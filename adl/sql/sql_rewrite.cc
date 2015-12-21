#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include <sql/const.h>
#include <sql/sql_rewrite.h>
#include <stdio.h>
extern "C" {
#include <dbug/dbug.h>
}
#include "util.h"
#include <sql/list.h>

#include <SMLog.h>

#include <vector>
using namespace std;

err_t expandSelectList(S_table venv, A_exp a);
err_t predicatePushUp(S_table venv, A_exp a);
err_t checkLocalExp(S_table venv,
		A_exp exp,
		A_sqlopr a,
		int *result);

/*
 * Function rewriteQuery() is the major routine to carry out any 
 * necessary SQL rewriting.
 */


/*
 * create a select item for expr and append it into head expr list 
 */
int appendExp2HxpList(A_list hxp_list, A_exp expr)
{
	SMLog::SMLOG(10, "Entering appendExp2HxpList");
	A_selectitem si;
	char alias[80];
	int i;

	for (i=0; i<hxp_list->length; i++) {
		si = (A_selectitem) getNthElementList(hxp_list, i);

		if (equalExp(expr, si->u.s.exp)) {
			if (expr->kind == A_callExp &&
					expr->u.call.member != si->u.s.exp->u.call.member) {
				expr->u.call.shared |= SHARED_INVOCATION;
			} else {
				//	expr->u.call.shared = 0;
				break;
			}
		}
	}

	if (i>=hxp_list->length) {
		sprintf(alias, "a_%d", hxp_list->length);
		si = A_SelectItem(expr->pos, expr, new_Symbol(alias));
		appendAList(hxp_list, (void*)si);
	}

	return i;
}
/*
 * if expr contain aggregate return true 
 */
	static err_t 
exprContainAggr(S_table venv, A_exp e, int *resultp, int *hasnonwinaggr)
{
	SMLog::SMLOG(10, "Entering exprContainAggr");
	err_t rc = ERR_NONE;
	int has_aggr = 0;

	*resultp = 0;

	switch (e->kind) {
		case A_varExp:
		case A_refExp:
		case A_nilExp:
		case A_intExp:
		case A_realExp:
		case A_stringExp:
		case A_timestampExp:
			break;
		case A_callExp:
			{
				SMLog::SMLOG(12, "\tTrying to call function3 %s ", S_name(e->u.call.func));
				E_enventry x = (E_enventry)S_look(venv, e->u.call.func);

				if(!e->u.call.win) *hasnonwinaggr = 1;

				if (x) {
					if (e->u.call.args) {
						A_list arglist = e->u.call.args;
						for (int i=0; i<arglist->length; i++) {
							A_exp a_exp = (A_exp)getNthElementList(arglist, i);
							if ((rc = exprContainAggr(venv, a_exp, &has_aggr, hasnonwinaggr)))
								goto exit;
							if (has_aggr) *resultp = 1;
						}
					}
					if (x->kind == E_aggrEntry) {
						if (*resultp==1) {
							rc = ERR_AGGREGATE_CASCADING;
							EM_error(e->pos, rc, __LINE__, __FILE__, S_name(e->u.call.func));
							goto exit;
						}
						*resultp = 1;
					}
				} else {
					rc = ERR_UNDEFINED_FUNCTION;
					EM_error (e->pos, rc, __LINE__, __FILE__, S_name(e->u.call.func));
					goto exit;
				}
			}
			break;
		case A_opExp:
			if (e->u.op.left) {
				if ((rc = exprContainAggr(venv, e->u.op.left, resultp, hasnonwinaggr)))
					goto exit;
			}
			if (*resultp==0 && e->u.op.right) {
				if ((rc = exprContainAggr(venv, e->u.op.right, resultp, hasnonwinaggr)))
					goto exit;
			}
			break;
		case A_sqloprExp:
		case A_selectExp:
			/* we do not sink into sub query */
			break;
		default:
			rc = ERR_SYNTAX;
			EM_error(e->pos, rc, __LINE__, __FILE__, "aggregate");
			displayExp(e);
			goto exit;
	}
exit:
	return rc;
}

/*
 * Check if all non-aggregate hxp are group by columns 
 */
err_t checkNonAggregateHxp(A_exp gb)
{
	SMLog::SMLOG(10, "Entering checkNonAggregateHxp");
	err_t rc = ERR_NONE;

	int i, j;
	A_list hxplist = gb->u.sqlopr->hxp_list;
	A_list prdlist = gb->u.sqlopr->prd_list;
	int prdlist_length = (prdlist)? prdlist->length : 0;

	for (i=0; i<hxplist->length; i++) {
		A_selectitem e = (A_selectitem)getNthElementList(hxplist, i);
		A_exp hxp;

		if (e->kind != SIMPLE_ITEM) {
			rc = ERR_NTSQL_INTERNAL;
			EM_error(0, rc, __LINE__, __FILE__, "unexpected non SIMPLE_ITEM hxp");
			goto exit;
		}
		hxp = e->u.s.exp;
		if (hxp->kind == A_varExp) {
			/* this hxp must be a group by column */
			for (j=0; j<prdlist_length; j++) {
				A_exp prd = (A_exp)getNthElementList(prdlist, j);
				if (equalExp(prd, hxp)) {
					break;
				}
			}
			if (!isESL() && j>=prdlist_length) {
				rc = ERR_GROUPBY;
				EM_error(e->pos, rc, __LINE__, __FILE__);
				goto exit;
			}
		} 
	}

exit:
	return rc;
};
/*
 * checkAggr() checks the usage of aggregates in a SELECT statement
 * resultp is set if aggregate is found
 */
	static err_t 
checkAggr(S_table tenv, A_exp sel, int *resultp, int *hasnonwinaggr)
{
	SMLog::SMLOG(10, "Entering checkAggr");
	A_list gblist = sel->u.select.group_by_list;
	A_list hxplist = sel->u.select.hxp_list;
	A_list hvcond = sel->u.select.hv_prd_list;
	A_list wrcond = sel->u.select.wr_prd_list;
	err_t rc = ERR_NONE;
	int result;
	int i;

	*resultp = 0;

	/* check group-by column */
	if (gblist && gblist->length>0) {
		*resultp = 1;
		/* gb list can not have aggr */
		for (i=0; i<gblist->length; i++) {
			A_exp gb = (A_exp)getNthElementList(gblist, i);
			if ((rc = exprContainAggr(tenv, gb, &result, hasnonwinaggr))) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkAggr", "exprcontainAggr");
				goto exit;
			}
			if (result == 1) {
				rc = ERR_AGGREGATE_GROUPBY;
				EM_error(gb->pos, rc, __LINE__, __FILE__);
				goto exit;
			}
		}
	}

	/* check having cond */
	for (i=0; hvcond && i<hvcond->length; i++) {
		A_exp prd = (A_exp)getNthElementList(hvcond, i);
		if ( (rc = exprContainAggr(tenv, prd, &result, hasnonwinaggr)) ) {
			EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkAggr", "exprcontainAggr");
			goto exit;
		}
		if (result==1)
			*resultp = 1;
	}

	/* check where cond : no aggr allowed */
	for (i=0; wrcond && i<wrcond->length; i++) {
		A_exp prd = (A_exp)getNthElementList(wrcond, i);
		if ( (rc = exprContainAggr(tenv, prd, &result, hasnonwinaggr)) ) {
			EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkAggr", "exprcontainAggr");
			goto exit;
		}
		if (result==1) {
			rc = ERR_AGGREGATE_GROUPBY;
			EM_error(prd->pos, rc, __LINE__, __FILE__);
			goto exit;
		}
	}

	/* check head expression */
	if (hxplist && hxplist->length>0) {
		for (int i=0; i<hxplist->length; i++) {
			A_selectitem e = (A_selectitem)getNthElementList(hxplist, i);
			if (e->kind == SIMPLE_ITEM) {
				rc = exprContainAggr(tenv, e->u.s.exp, &result, hasnonwinaggr);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkAggr", "exprcontainAggr");
					goto exit;
				}
			}
			if (result==1)
				*resultp = 1;
		}
	}
exit:
	return rc;
} 

/* 
 * Check if the value of expr is deterministic in opr "sel".
 * It is deterministic, if
 *     o expr is a constant
 *     o expr is a vaiable passed in as a parameter
 *     o expr is a deterministic function and all its parameters 
 *       are deterministic.
 */
	static err_t 
irrelevantExpr(A_exp expr, A_exp sel, int *resultp)
{
	SMLog::SMLOG(10, "Entering irrelevantExpr");
	err_t rc = ERR_NONE;

	*resultp = 0;

	switch (expr->kind) {
		case A_callExp:
			break;
		case A_opExp:
			rc = irrelevantExpr(expr->u.op.left, sel, resultp);
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "irrelevantExpr", "irrelevantExpr");
				goto exit;
			}
			if (*resultp == 1) {
				rc = irrelevantExpr(expr->u.op.right, sel, resultp);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "irrelevantExpr", "irrelevantExpr");
					goto exit;
				}
			}
			break;
		case A_varExp: 
			break;
		case A_intExp: 
		case A_stringExp: 
		case A_timestampExp: 
		case A_realExp:
			*resultp = 1;
			break;
	}
exit:
	return rc;
}
/*
   given an expr, we extract its aggr sub-expressions

   for example, given 

   expr = (a*b)+aggr(b+c)+aggr(d+e) 
   gb_query_name = Q1
   sw_query_name = Q0

   we get:

   sw_hxp_list = a*b, b+c, d+e      
   gb_hxp_list = Q0.0, aggr(Q0.1), aggr(Q0.2)
   sh_hxp = Q1.0+Q1.1+Q1.2
   has_aggr = 1
 */
static err_t 
filterAggrExpr(S_table venv, 
		A_exp expr,	    /* IN: expr */
		A_list gb_hxp_list,    /* IO: head exprs in GB box */
		char *gb_query_name,    /* IN: name of GB query */
		A_list sw_hxp_list,    /* IO: head exprs in SW box */
		char *sw_query_name,    /* IN: name of SW query */
		A_exp *sh_hxp,	    /* OUT: rewritten expr */
		int *has_aggr,	    /* OUT: found aggr? */
		int *has_nonwin_aggr
		)
{
	SMLog::SMLOG(10, "Entering filterAggrExpr");
	err_t rc = ERR_NONE;
	char ident[80];
	A_selectitem si;
	int hxpidx;

	if ((rc = exprContainAggr(venv, expr, has_aggr, has_nonwin_aggr))) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "filterAggrExpr", "exprContainAggr");
		goto exit;
	}

	if (*has_aggr == 0)
	{
		int irrelevant;
		rc = irrelevantExpr(expr, (A_exp)0, &irrelevant);
		if (rc) {
			EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "filterAggrExpr", "irrelevantExpr");
			goto exit;
		}
		if (irrelevant) {
			if (sh_hxp) *sh_hxp = expr;
		} else {
			/* x+y  ==>

SH: GB.b
GB: SW.a b
SW: x+y a
			 */
			A_exp gb_hxp;

			hxpidx = appendExp2HxpList(sw_hxp_list, expr);

			sprintf(ident, "a_%d", hxpidx);
			gb_hxp = A_SqlVarExp(expr->pos, 
					new_Symbol(sw_query_name), 
					new_Symbol(ident));

			if (!sh_hxp) {
				appendAList(gb_hxp_list, (void*)gb_hxp);
			} else {
				hxpidx = appendExp2HxpList(gb_hxp_list, gb_hxp);

				sprintf(ident, "a_%d", hxpidx);
				*sh_hxp = A_SqlVarExp(expr->pos, 
						new_Symbol(gb_query_name), 
						new_Symbol(ident));
			}
		}
	}
	else 
	{
		switch (expr->kind) {
			case A_callExp:
				{
					E_enventry x = (E_enventry)S_look(venv, expr->u.call.func);
					int i;

					if (x->kind == E_aggrEntry) {
						/* ..., aggr(x+1,y+2,z+3), ... ==>

SH: ..., GB.d, ...
GB: ..., aggr(SW.a, SW.b, SW.c) d, ...
SW: ..., x+1 a, y+2 b, z+3 c, ...

no aggregte should appear in the arguments
						 */
						if (expr->u.call.args) {
							A_list args = expr->u.call.args;
							A_list newargs = A_List(args->pos, args->type);
							int startpos = sw_hxp_list->length;

							for (i=0; i<args->length; i++) {
								A_exp arg = (A_exp)getNthElementList(args, i);
								A_exp newarg;

								hxpidx = appendExp2HxpList(sw_hxp_list, arg);

								sprintf(ident, "a_%d", hxpidx);
								newarg = A_SqlVarExp(arg->pos,
										new_Symbol(sw_query_name),
										new_Symbol(ident));
								appendAList(newargs, (void*)newarg);
							}

							clearList(args);
							deleteList(args);
							expr->u.call.args = newargs;
						}

						hxpidx = appendExp2HxpList(gb_hxp_list, expr);
						if (sh_hxp) {
							sprintf(ident, "a_%d", hxpidx);
							*sh_hxp = A_SqlVarExp(expr->pos,
									new_Symbol(gb_query_name),
									new_Symbol(ident));
						}
					} else {
						/* ..., f(x+y,aggr(z)+3), ... ==>

SH: ..., f(GB.c, GB.d+3), ...
GB: ..., SW.a c, aggr(SW.b) d, ...
SW: ..., x+y a, z b, ...

aggregtes appear in the arguments
						 */
						if (expr->u.call.args) {
							A_list args = expr->u.call.args;

							for (i=0; i<args->length; i++) 
							{
								A_exp arg = (A_exp)getNthElementList(args, i);
								A_exp new_arg;
								rc = filterAggrExpr(venv, arg, gb_hxp_list, gb_query_name,
										sw_hxp_list, sw_query_name,    
										&new_arg, has_aggr, has_nonwin_aggr);	    
								if (rc) {
									EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "filterAggrExpr", "filterAggrExpr");
									goto exit;
								}

								overwriteElementList(args, i, (nt_obj_t*)new_arg);
							}
						}
						if (sh_hxp) *sh_hxp = expr;
					}
				}
				break;
			case A_opExp:
				{
					A_exp newl, newr;
					rc = filterAggrExpr(venv, expr->u.op.left, gb_hxp_list, 
							gb_query_name,
							sw_hxp_list, sw_query_name, &newl, has_aggr,
							has_nonwin_aggr);
					if (rc) {
						EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "filterAggrExpr", "filterAggrExpr");
						goto exit;
					}

					rc = filterAggrExpr(venv, expr->u.op.right, gb_hxp_list, 
							gb_query_name,
							sw_hxp_list, sw_query_name, &newr, has_aggr,
							has_nonwin_aggr);
					if (rc) {
						EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "filterAggrExpr", "filterAggrExpr");
						goto exit;
					}

					expr->u.op.left = newl;
					expr->u.op.right = newr;
					if (sh_hxp) *sh_hxp = expr;
				}
				break;
			default:
				break;
		}
	}
exit:
	return rc;
}

/*
 * Function rewriteExp() performs SQL rewriting on an expression.
 */
err_t rewriteExp(S_table venv, S_table tenv, A_sqlopr opr, A_exp exp, A_exp &nexp, vector<void*> aggregates)
{
	SMLog::SMLOG(10, "Entering rewriteExp");
	err_t rc = ERR_NONE;
	switch (exp->kind) {
		case A_nilExp:
		case A_varExp:
		case A_intExp:
		case A_stringExp:
		case A_timestampExp:
		case A_callExp:
		case A_refExp:
		case A_realExp:
			nexp = exp;
			break;
		case A_opExp:
			{
				A_exp nleft, nright;
				rc = rewriteExp(venv, tenv, opr, exp->u.op.left, nleft, aggregates);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteExp", "rewriteExp");
					goto exit;	
				}
				exp->u.op.left = nleft;

				if (exp->u.op.right) {
					rc = rewriteExp(venv, tenv, opr, exp->u.op.right, nright, aggregates);
					if (rc) {
						EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteExp", "rewriteExp");
						goto exit;	
					}
					exp->u.op.right = nright;
				} 

				nexp = exp;
			}
			break;
		case A_selectExp:
		case A_sqloprExp:
			rc = rewriteQuery(venv, tenv, exp, nexp, aggregates);
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteExp", "rewriteQuery");
				goto exit;	
			}
			/*
			   record subquery in the subquery_list
			   appendElementList(opr->subquery_list, (nt_obj_t*)nexp);
			 */
			break;
		default:
			rc = ERR_NTSQL_INTERNAL;
			EM_error(0, rc, __LINE__, __FILE__, "unexpected expr type in rewriteExp()");
			goto exit;
	}
exit:
	return rc;
}

err_t establishLinks(A_exp a)
{
	err_t rc = ERR_NONE;

exit: 
	return rc;
}

/*
 * Function rewriteQuery() is the major routine to carry out any 
 * necessary SQL rewriting
 *
 * Currently, the following rewriting is implemented:
 *
 *  o stack SQL opr
 *  o expand SELECT hxp which returns a complex data structure
 *
 * A lot of useful rewritings can be added in the future, for example,
 * predicate push down.  
 *
 * --Haixun Wang
 */
err_t rewriteQuery(S_table venv, S_table tenv, A_exp a, A_exp &result, vector<void*> aggregates)
{
	SMLog::SMLOG(10, "Entering rewriteQuery");
	err_t rc = ERR_NONE;

	rc = staticRewrite(venv, tenv, a, result, aggregates);	// rewriting within a node
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteQuery", "staticRewrite");
		goto exit;
	}

#if 0
	rc = establishLinks(a);
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteQuery", "establishLinks");
		goto exit;
	}

	rc = dynamicRewrite(venv, a, result);		// rewriting requires traversal
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteQuery", "dynamicRewrite");
		goto exit;
	}
#endif

exit: 
	return rc;
}


err_t staticRewrite(S_table venv, S_table tenv, A_exp a, A_exp &result, vector<void*> aggregates)
{
	SMLog::SMLOG(10, "Entering staticRewrite");
	err_t rc = ERR_NONE;

	A_sqlopr opr;
	int i;

	SMLog::SMLOG(10, "\tstaticRewrite: PHASE 1");
	/* PHASE 1 */
	switch(a->kind) {
		case A_sqloprExp:
			SMLog::SMLOG(10, "\tstaticRewrite: A_sqloprExp");
			rc = expandSelectList(venv, a); 
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "staticRewrite", "expandSelectList");
				goto exit;
			}

			result = a;
			break;
		case A_selectExp:
			{
				SMLog::SMLOG(10, "\tstaticRewrite: A_selectExp");
				rc = expandSelectList(venv, a);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "staticRewrite", "constructSelOp");
					goto exit;
				}

				rc = constructSelOp(venv, a, result);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "staticRewrite", "constructSelOp");
					goto exit;	
				}
			}
			break;
		default:
			rc = ERR_NTSQL_INTERNAL;
			EM_error(0, rc, __LINE__, __FILE__, "unexpected expr type in staticRewrite()");
			goto exit;
	}

	SMLog::SMLOG(10, "\tstaticRewrite: rewrite sub nodes recursively ");
	/* rewrite sub nodes recursively */
	opr = result->u.sqlopr;
	for (i=0; opr->jtl_list && i < opr->jtl_list->length; i++) {
		A_qun q = (A_qun)getNthElementList(opr->jtl_list, i);
		if (q->kind == QUN_QUERY) {
			A_exp nq;
			rc = rewriteQuery(venv, tenv, q->u.query, nq, aggregates);
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteQuery", "rewriteQuery");
				goto exit;
			}
			q->u.query = nq;
		}
	}

	/* check if duplicate column name exists in subquery */
	if (a->kind == A_selectExp)
	{
		A_list tblList = a->u.select.join_table_list;
		A_selectitem item1, item2;
		S_symbol symbol1, symbol2;
		for (int j = 0; tblList && j < tblList->length; ++j)
		{
			A_qun tblQun = (A_qun)getNthElementList(tblList, j);
			if (tblQun->kind == QUN_QUERY) // is a subquery
			{
				A_list colList;
				switch (tblQun->u.query->kind)
				{
					case A_selectExp:
						colList = tblQun->u.query->u.select.hxp_list;
						break;
					case A_sqloprExp:
						colList = tblQun->u.query->u.sqlopr->hxp_list;
						break;
					default:
						continue;
				}
				for (int itr1 = 0; itr1 < colList->length; ++itr1)
				{
					item1 = (A_selectitem) getNthElementList(colList, itr1);
					if (item1->u.s.exp->kind == A_varExp)
					{
						if (item1->u.s.exp->u.var->kind == A_simpleVar) {
							symbol1 = item1->u.s.exp->u.var->u.simple;
						} else if (item1->u.s.exp->u.var->kind == A_fieldVar) {
							symbol1 = item1->u.s.exp->u.var->u.field.sym;
						} else if (item1->u.s.alias) {
							symbol1 = item1->u.s.alias;
						} else continue;
					} else if (item1->u.s.alias) {
						symbol1 = item1->u.s.alias;
					} else continue;
					for (int itr2 = itr1 + 1; itr2 < colList->length; ++itr2)
					{
						item2 = (A_selectitem) getNthElementList(colList, itr2);
						if (item2->u.s.exp->kind == A_varExp)
						{
							if (item2->u.s.exp->u.var->kind == A_simpleVar) {
								symbol2 = item2->u.s.exp->u.var->u.simple;
							} else if (item1->u.s.exp->u.var->kind == A_fieldVar) {
								symbol2 = item2->u.s.exp->u.var->u.field.sym;
							} else if (item2->u.s.alias) {
								symbol2 = item2->u.s.alias;
							} else continue;
						} else if (item2->u.s.alias) {
							symbol2 = item2->u.s.alias;
						} else continue;
						if (strcmp(symbol1->name, symbol2->name) == 0)
						{
							rc = ERR_DUPLICATE_COLUMN_NAME;
							EM_error(0, rc, __LINE__, __FILE__, symbol1->name, item1->pos);
							goto exit;
						}
					}
				}
			}
		}
	}

	if (opr->kind == A_SQL_SEL || opr->kind == A_SQL_UPDATE || opr->kind == A_SQL_DELETE) {
		// process the subquery in the WHERE condition
		for (i=0; opr->prd_list && i<opr->prd_list->length; i++) {
			A_exp exp = (A_exp)getNthElementList(opr->prd_list, i);
			A_exp nexp;

			rc = rewriteExp(venv, tenv, opr, exp, nexp, aggregates);
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteQuery", "rewriteExp");
				goto exit;
			}
			if (exp != nexp) {
				overwriteElementList(opr->prd_list, i, (nt_obj_t*)nexp);
			}
		}
	}

	if (opr->kind == A_SQL_UPDATE) {
		rc = rewriteUpdate(venv, tenv, result, aggregates);
		if (rc) {
			EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteQuery", "rewriteUpdate");
			goto exit;
		}
	}

exit:
	return rc;
}


/*
   giving an SQL statement:

   SELECT .. FROM .. WHERE .. GROUP BY .. HAVING ..

   construct a query graph composed of SEL/GB nodes.
 */
err_t constructSelOp(S_table venv, A_exp sel, A_exp &result)
{
	SMLog::SMLOG(10, "Entering constructSelOp");
	A_exp sw, gb, sh;
	A_list alist;
	A_exp ele;
	A_exp sh_hxp;
	int i;
	int hasaggr;			// flag:  aggregate is used in SELECT
	int hasnonwinaggr = 0;
	A_list gblist = sel->u.select.group_by_list;
	A_list hxplist = sel->u.select.hxp_list;
	A_list odlist = sel->u.select.order_by_list;
	A_list hvcond = sel->u.select.hv_prd_list;
	A_list wrcond = sel->u.select.wr_prd_list;
	A_qun qun;
	err_t rc = ERR_NONE;

	/*  rc = expandSelectList(venv, sel); 
		if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "expandSelectList");
		goto exit;
		}
	 */


	rc = checkAggr(venv, sel, &hasaggr, &hasnonwinaggr);
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "checkAggr");
		goto exit;
	}

	if (hasaggr)
	{
		DBUG_EXECUTE("gbconvert", displayExp(sel););

		char *sw_name = getUniqueName("Q");
		char *gb_name = getUniqueName("Q");
		A_list gb_hxp_list = A_List(hxplist->pos, hxplist->type);
		A_list sh_hxp_list = A_List(hxplist->pos, hxplist->type);
		A_list sw_hxp_list = A_List(hxplist->pos, hxplist->type);
		A_list gb_cnd_list = (A_list)0;
		A_list sh_cnd_list = (A_list)0;

		A_list jtl1 = A_List(0, A_QUN);
		A_list jtl2 = A_List(0, A_QUN);

		/* construct SEL-GB-SEL graph */
		/* decompose hxp_list */
		for (i=0; i<hxplist->length; i++) 
		{
			A_selectitem si = (A_selectitem) getNthElementList(hxplist, i);
			A_selectitem ti; 

			if (si->kind == SIMPLE_ITEM) {
				ele = si->u.s.exp;
				rc = filterAggrExpr(venv, ele, gb_hxp_list, gb_name, 
						sw_hxp_list, sw_name, &sh_hxp, &hasaggr,
						&hasnonwinaggr);

				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "filterAggrExpr");
					goto exit;
				}

				ti = A_SelectItem(si->pos, sh_hxp, si->u.s.alias);
				appendAList(sh_hxp_list, (void*)ti);
			}
		}

		/* decompose gblist */
		if (gblist) {
			gb_cnd_list = A_List(gblist->pos, A_EXP);
			for (i=0; i<gblist->length; i++) {
				ele = (A_exp)getNthElementList(gblist, i);
				rc = filterAggrExpr(venv, ele, 
						gb_cnd_list,/* pass in group-by list
									   instead */
						sw_name,
						sw_hxp_list, 
						sw_name, 
						(A_exp*)0, /* The expr is in the group-by
									  column, and will be put into
									  the group-by list.*/
						&hasaggr,
						&hasnonwinaggr);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "filterAggrExpr");
					goto exit;
				}
			}
		}
		/* decompose hvcond */
		if (hvcond) {
			sh_cnd_list = A_List(hvcond->pos, A_EXP);
			for (i=0; i<hvcond->length; i++) {
				ele = (A_exp)getNthElementList(hvcond, i);
				rc = filterAggrExpr(venv, ele, gb_hxp_list, gb_name,
						sw_hxp_list, sw_name, &sh_hxp, &hasaggr,
						&hasnonwinaggr);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "filterAggrExpr");
					goto exit;
				}
				appendAList(sh_cnd_list, (void*)sh_hxp);
			}
		}
		/* construct SW box */
		if (sw_hxp_list->length == 0) {
			A_exp dummy = A_IntExp(sel->pos, 1);
			A_selectitem si = A_SelectItem(sel->pos, dummy, (S_symbol)0); 

			appendAList(sw_hxp_list, (void*)si);
		}
		sw = A_SqlOprExp(sel->pos,
				A_SQL_SEL,
				0,	// distinct
				sw_hxp_list,
				sel->u.select.join_table_list,
				wrcond);

		/* move predicates in wrcond that are not relevant (e.g., constant) to having-condition */ 
		if (wrcond) {
			i = 0;
			while (i<wrcond->length) {
				A_exp prd = (A_exp)getNthElementList(wrcond, i);
				int result;
				rc = checkLocalExp(venv, prd, sw->u.sqlopr, &result);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "checkLocalExp");
					goto exit;
				}
				if (!result) {
					removeNthElementList(wrcond, i);
					i--;
					if (!sh_cnd_list) {
						sh_cnd_list = A_List(wrcond->pos, A_EXP);
					}
					appendAList(sh_cnd_list, (void*)prd);
				}
				i++;
			}
		}

		/* construct GB box */
		qun = A_QueryQun(sel->pos,
				new_Symbol(sw_name),
				sw);
		appendAList(jtl1, (void*)qun);

		gb = A_SqlOprExp(sel->pos,
				A_SQL_GB,
				0,	// distinct
				gb_hxp_list,
				jtl1,
				gb_cnd_list);

		/* construct SH box */
		qun = A_QueryQun(sel->pos,
				new_Symbol(gb_name),
				gb);
		appendAList(jtl2, (void*)qun);

		sh = A_SqlOprExp(sel->pos,
				A_SQL_SEL,
				sel->u.select.distinct,
				sh_hxp_list,
				jtl2,
				sh_cnd_list);

		DBUG_EXECUTE("gbconvert", displayExp(sh););

		/* check if all non-aggregate hxp are group by columns */
		if(hasnonwinaggr) {
			rc = checkNonAggregateHxp(gb);
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "checkNonAggregateHxp");
				goto exit;
			}
		}

	} /* end of hasaggr */
	else 
	{
		/* no aggr */
		sh = A_SqlOprExp(sel->pos,
				A_SQL_SEL,
				sel->u.select.distinct,
				hxplist,
				sel->u.select.join_table_list,
				wrcond);
		if (odlist) {
			/* order-by */
			A_list od_hxp_list = A_List(hxplist->pos, hxplist->type);
			A_list od_cnd_list = A_List(0, A_ORDERITEM); // order by columns
			A_list od_jtl_list = A_List(0, A_QUN);
			char *sh_name = getUniqueName("Q");
			char ident[20];

			/* construct QUN */
			qun = A_QueryQun(sel->pos,
					new_Symbol(sh_name),
					sh);
			appendAList(od_jtl_list, (void*)qun);

			/* copy the hxp list of sh to od */
			for (i=0; i<hxplist->length; i++) {
				A_selectitem si = (A_selectitem) getNthElementList(hxplist, i);
				if (!si->u.s.alias) {
					sprintf(ident, "a_%d", i);
					si->u.s.alias = new_Symbol(ident);
				}
				A_exp od_hxp = A_SqlVarExp(si->pos,
						new_Symbol(sh_name),
						si->u.s.alias);
				si = A_SelectItem(si->pos, od_hxp, (S_symbol)0);
				appendAList(od_hxp_list, (void*)si);
			}
			/* rename the order by */
			for (i=0; i<odlist->length; i++) {
				A_orderitem oi = (A_orderitem)getNthElementList(odlist, i);
				A_exp exp = oi->exp;
				/* is this order-by column in the hxp-list ? */
				int hxpidx = appendExp2HxpList(hxplist, exp);
				A_selectitem si = (A_selectitem)getNthElementList(hxplist, hxpidx);
				if (!si->u.s.alias) {
					sprintf(ident, "a_%d", i);
					si->u.s.alias = new_Symbol(ident);
				}
				exp = A_SqlVarExp(exp->pos,
						new_Symbol(sh_name),
						si->u.s.alias);
				appendAList(od_cnd_list, (void*)A_OrderItem(oi->pos, exp, oi->dir));
			}

			sh = A_SqlOprExp(sel->pos,
					A_SQL_ORDER,
					0,	// distinct
					od_hxp_list,
					od_jtl_list,
					od_cnd_list
					);
			DBUG_EXECUTE("gbconvert", displayExp(sh););
		} 
	}

	result = sh;
exit:
	return rc;
}

#if 0
err_t dynamicRewrite(S_table venv, A_exp a, A_exp &result)
{
	err_t rc = ERR_NONE;

	rc = predicatePushUp(venv, result);
	if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "staticRewrite", "predicatePushUp");
		goto exit;	
	}

exit:
	return rc;
}
/*
 * predicates (expr in WHERE conditions) are pushed up if they are not
 * relevant to the current node.
 *
 */
err_t predicatePushUp(S_table venv, A_exp a)
{
	err_t rc = ERR_NONE;
	int i;
	A_sqlopr opr;
	A_qun qun;

	if (a->kind != A_sqloprExp) {
		rc = ERR_NTSQL_INTERNAL;
		EM_error(0, rc, __LINE__, __FILE__, "unexpected expr type in predicatePushUp()");
		goto exit;
	}

	opr = a->u.sqlopr;
	if (opr->jtl_list) {
		int pos = opr->jtl_list->length;
		A_qun qun = (A_qun)getNthElementList(opr->jtl_list, pos);

		while (pos>=0) {

			rc = predicatePushUpAlongQun(qun, predlist);
			if (rc) {
				EM_error(0, ERR_HISTORY, __LINE__, __FILE__, 
						"predicatePushUp", "predicatePushUpAlongQun");
				goto exit;
			}

			pos--;

			if (pos>=0) {
				qun = (A_qun)getNthElementList(opr->jtl_list, pos);
				rc = predicatePushDownAlongQun(qun, predlist);
				if (rc) {
					EM_error(0, ERR_HISTORY, __LINE__, __FILE__, 
							"predicatePushUp", "predicatePushDownAlongQun");
					goto exit;
				}
			}
		}
	}
exit:
	return rc;
}

err_t predicatePushUpAlongQun(A_qun qun, list_t *list) 
{
	err_t rc;

	switch (qun->kind) {
		case QUN_NAME:
			break;
		case QUN_FUNCTION:
			break;
		case QUN_QUERY:
			break;
	}
exit:
	return rc;
}
#endif 
/*
 * Expand the hxp if it is a function call that returns a complex
 * data structure.
 */
err_t expandSelectList(S_table venv, A_exp a)
{
	SMLog::SMLOG(10, "Entering expandSelectList");
	err_t rc = ERR_NONE;
	A_selectitem si, nsi;
	E_enventry x;
	int j,k, nextpos = 0;
	A_list alist;

	switch (a->kind) {
		case A_sqloprExp:
			alist = a->u.sqlopr->hxp_list;
			break;
		case A_selectExp:
			alist = a->u.select.hxp_list;
			break;
	}

	if (!alist)
		goto exit;

	nextpos = 0;
	while (nextpos < alist->length) {

		si = (A_selectitem) getNthElementList(alist, nextpos);
		nextpos++;

		switch(si->kind) {
			case SIMPLE_ITEM: 
				{
					A_exp e = si->u.s.exp;
					if (e->kind == A_callExp && e->u.call.member == (S_symbol)0) {

						SMLog::SMLOG(12, "\tTrying to call function1 %s ", S_name(e->u.call.func));
						x = (E_enventry)S_look(venv, e->u.call.func);
						if (!x) {
							rc = ERR_UNDEFINED_FUNCTION;
							EM_error (e->pos, rc, __LINE__, __FILE__, S_name(e->u.call.func));
							goto exit;
						}

						if ((x->kind == E_funEntry || x->kind == E_aggrEntry) &&
								x->u.fun.result &&
								x->u.fun.result->kind==Ty_record &&
								x->u.fun.result->u.record->length>1) {

							A_list list = x->u.fun.result->u.record;
							for (j=0; j<list->length; j++) {
								Ty_field f = (Ty_field)getNthElementList(list, j);
								A_exp newe = copyExp(e);

								newe->u.call.member = f->name;

								nsi = A_SelectItem(si->pos, newe, (S_symbol)0);

								if (j==0) {
									overwriteElementList(alist, nextpos-1, (nt_obj_t*)nsi);
								} else {
									e->u.call.shared |= SHARED_INVOCATION;
									insertElementList(alist, nextpos, (nt_obj_t*)nsi);
									nextpos++;
								}
							}
						}
					}
				}
				break;
			case COMPLEX_ITEM:
				{
					A_exp e = si->u.c.exp;

					SMLog::SMLOG(12, "\tTrying to call function2 %s ", S_name(e->u.call.func));
					x = (E_enventry)S_look(venv, e->u.call.func);
					if (!x || (x->kind != E_funEntry && x->kind != E_aggrEntry)) {
						rc = ERR_UNDEFINED_FUNCTION;
						EM_error (e->pos, rc, __LINE__, __FILE__, S_name(e->u.call.func));
						goto exit;
					}
					if (x->u.fun.result->kind!=Ty_record) {
						rc = ERR_COMPLEX_FUNC_EXPECTED;
						EM_error (e->pos, rc, __LINE__, __FILE__,S_name(e->u.call.func));
						goto exit;
					}
					A_list list = x->u.fun.result->u.record;
					if (list->length != si->u.c.aliaslist->length) {
						rc = ERR_UNMATCH_CARDINALITY;
						EM_error(e->pos, rc, __LINE__, __FILE__, S_name(e->u.call.func));
						goto exit;
					}
					for (j=0; j<list->length; j++) {
						Ty_field f = (Ty_field)getNthElementList(list, j);
						S_symbol alias = (S_symbol)getNthElementList(si->u.c.aliaslist, j);
						A_exp newe = copyExp(e);

						newe->u.call.member = f->name;

						nsi = A_SelectItem(si->pos, newe, alias);

						if (j==0) {
							overwriteElementList(alist, nextpos-1, (nt_obj_t*)nsi);
						} else {
							e->u.call.shared |= SHARED_INVOCATION;
							insertElementList(alist, nextpos, (nt_obj_t*)nsi);
							nextpos++;
						}
					}
				}
				break;
			case STAR_ITEM:
				{
					/*
					 * mohan yang, 11/14/11
					 *
					 * expand select *
					 * assume table A(int a, int b); table B(int a, int b);
					 *
					 * 1) select * from A; // from a single table
					 * => select A.a, A.b from A;
					 *
					 * 2) select * from A, B; // from join of multiple table
					 * => select A.a, A.b, B.a, B.b from A, B;
					 *
					 * 3) select A.*, B.b from A, B; // table.*
					 * => select A.a, A.b, B.b from A, B;
					 *
					 * 4) select * from (select a, b from A) as C; // from a subquery
					 * => select a, b from (select a, b from A) as C;
					 *
					 * 5) select * from (select * from A) as C; // recursive
					 * => select C.a, C.b from (select A.a, A.b from A) as C;
					 *
					 * 6) select * from (select * from A, B) as C; // duplicate column name 'a' and 'b', throw an error later
					 * => select C.a, C.b, C.a, C.b from (select A.a, A.b, B.a, B.b from A, B) as C;
					 *
					 * 7) select * from (select sum(a) as s from A) as C; // from a subquery with function/aggregate call (must have an alias)
					 * => select C.s from (select sum(a) as s from A) as C;
					 *
					 * TODO	support the following expansion
					 * 1) select * from (select sum(a) from A) as C; // from a subquery with function/aggregate call (but no alias)
					 * */
					bool aliasChecking = true;
					if (si->u.table) {
						SMLog::SMLOG(12, "\tTrying to expand select %s.*", S_name(si->u.table));
						bool found = false;
						A_list tbList = a->u.select.join_table_list;
						for (int j = 0; j < tbList->length && !found; ++j)
						{
							A_qun tblQun = (A_qun)getNthElementList(tbList, j);
							if (tblQun->kind == QUN_NAME){
								if (strcmp(si->u.table->name, tblQun->u.table.name->name) == 0 || strcmp(si->u.table->name, tblQun->alias->name) == 0)
								{
									found = true;
									void* envEntry = S_look(venv, tblQun->u.table.name);
									if (envEntry) {
										x = (E_enventry)envEntry;
										if (x->kind == E_varEntry)
										{
											A_list colList = x->u.var.ty->u.record;
											for (int itr = 0; itr < colList->length; ++itr)
											{
												Ty_field f = (Ty_field)getNthElementList(colList, itr);
												nsi = A_SelectItem(si->pos, A_SqlVarExp(si->pos, tblQun->alias, f->name), (S_symbol)0);
												if (itr == 0) {
													overwriteElementList(alist, nextpos - 1, (nt_obj_t*)nsi);
												} else {
													insertElementList(alist, nextpos, (nt_obj_t*)nsi);
													nextpos++;
												}
											}
										}
									} else {
										rc = ERR_TABLE_NOT_EXIST;
										EM_error(si->pos, rc, __LINE__, __FILE__, si->u.table->name);
										goto exit;
									}
								}
							} else if (tblQun->kind == QUN_QUERY) {
								expandSelectList(venv, tblQun->u.query);
								if (strcmp(si->u.table->name, tblQun->alias->name) == 0) {
									found = true;
									A_list colList = tblQun->u.query->u.select.hxp_list;
									for (int itr = 0; itr < colList->length; ++itr)
									{
										nsi = (A_selectitem) getNthElementList(colList, itr);
										aliasChecking = true;
										if (nsi->u.s.exp->kind == A_varExp) {
											if (nsi->u.s.exp->u.var->kind == A_simpleVar || nsi->u.s.exp->u.var->kind == A_refVar) {
												nsi = A_SelectItem(si->pos, nsi->u.s.exp, nsi->u.s.alias);
												aliasChecking = false;
											} else if (nsi->u.s.exp->u.var->kind == A_fieldVar) {
												nsi = A_SelectItem(si->pos, A_SqlVarExp(si->pos, tblQun->alias, nsi->u.s.exp->u.var->u.field.sym), nsi->u.s.alias);
												aliasChecking = false;
											}
										}
										if (aliasChecking) {
											if (nsi->u.s.alias) {
												nsi = A_SelectItem(si->pos, A_SqlVarExp(si->pos, tblQun->alias, nsi->u.s.alias), (S_symbol)0);
											} else {
												rc = ERR_FUNCTION_ALIAS_MISSING;
												EM_error(si->pos, rc, __LINE__, __FILE__);
												goto exit;
											}
										}
										if (itr == 0) {
											overwriteElementList(alist, nextpos - 1, (nt_obj_t*)nsi);
										} else {
											insertElementList(alist, nextpos, (nt_obj_t*)nsi);
											nextpos++;
										}
									}
								}
							}
						}
						if (!found)
						{
							rc = ERR_TABLE_NOT_EXIST;
							EM_error(si->pos, rc, __LINE__, __FILE__, si->u.table->name);
							goto exit;
						}
					} else {
						SMLog::SMLOG(12, "\tTrying to expand select *");
						A_list tblList = a->u.select.join_table_list;
						for (int j = 0; j < tblList->length; ++j)
						{
							A_qun tblQun = (A_qun)getNthElementList(tblList, j);
							if (tblQun->kind == QUN_NAME) {
								void* envEntry = S_look(venv, tblQun->u.table.name);
								if (envEntry) {
									x = (E_enventry)envEntry;
									if (x->kind == E_varEntry)
									{
										A_list colList = x->u.var.ty->u.record;
										for (int itr = 0; itr < colList->length; ++itr)
										{
											Ty_field f = (Ty_field)getNthElementList(colList, itr);
											nsi = A_SelectItem(si->pos, A_SqlVarExp(si->pos, tblQun->u.table.name, f->name), (S_symbol)0);
											if (j == 0 && itr == 0) {
												overwriteElementList(alist, nextpos - 1, (nt_obj_t*)nsi);
											} else {
												insertElementList(alist, nextpos, (nt_obj_t*)nsi);
												nextpos++;
											}
										}
									}
								} else {
									rc = ERR_TABLE_NOT_EXIST;
									EM_error(tblQun->pos, rc, __LINE__, __FILE__, tblQun->u.table.name->name);
									goto exit;
								}
							} else if (tblQun->kind == QUN_QUERY) {
								expandSelectList(venv, tblQun->u.query);
								A_list colList = tblQun->u.query->u.select.hxp_list;
								for (int itr = 0; itr < colList->length; ++itr)
								{
									nsi = (A_selectitem) getNthElementList(colList, itr);
									aliasChecking = true;
									if (nsi->u.s.exp->kind == A_varExp) {
										if (nsi->u.s.exp->u.var->kind == A_simpleVar || nsi->u.s.exp->u.var->kind == A_refVar) {
											nsi = A_SelectItem(si->pos, nsi->u.s.exp, nsi->u.s.alias);
											aliasChecking = false;
										} else if (nsi->u.s.exp->u.var->kind == A_fieldVar) {
											nsi = A_SelectItem(si->pos, A_SqlVarExp(si->pos, tblQun->alias, nsi->u.s.exp->u.var->u.field.sym), nsi->u.s.alias);
											aliasChecking = false;
										}
									}
									if (aliasChecking) {
										if (nsi->u.s.alias) {
											nsi = A_SelectItem(si->pos, A_SqlVarExp(si->pos, tblQun->alias, nsi->u.s.alias), (S_symbol)0);
										} else {
											rc = ERR_FUNCTION_ALIAS_MISSING;
											EM_error(si->pos, rc, __LINE__, __FILE__);
											goto exit;
										}
									}
									if (j == 0 && itr == 0) {
										overwriteElementList(alist, nextpos - 1, (nt_obj_t*)nsi);
									} else {
										insertElementList(alist, nextpos, (nt_obj_t*)nsi);
										nextpos++;
									}
								}
							}
						}
					}
				}
				break;
		}
	}

exit:
	SMLog::SMLOG(10, ">Exiting expandSelectList %i", rc);
	return rc;
}

