#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include <sql/const.h>
#include <stdio.h>
extern "C" {
#include <dbug/dbug.h>
}
#include "util.h"
#include "list.h"

err_t expandSelectList(S_table venv, A_exp sel);

/*
 * create a select item for expr and append it into head expr list 
 */
int appendExp2HxpList(list_t *hxp_list, A_exp expr)
{
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
	//expr->u.call.shared = 0;
	break;
      }
    }
  }
  
  if (i>=hxp_list->length) {
    sprintf(alias, "a_%d", hxp_list->length);
    si = A_SelectItem(expr->pos, expr, new_Symbol(alias));
    appendElementList(hxp_list, (nt_obj_t*)si);
  }

  return i;
}
/*
 * if expr contain aggregate return true 
 */
static err_t 
exprContainAggr(S_table venv, A_exp e, int *resultp)
{
  err_t rc = ERR_NONE;
  int has_aggr = 0;

  *resultp = 0;

  switch (e->kind) {
  case A_varExp:
  case A_nilExp:
  case A_intExp:
  case A_stringExp:
  case A_timestampExp:
    break;
  case A_callExp:
    {
      E_enventry x = (E_enventry)S_look(venv, e->u.call.func);

      if (x) {
	if (e->u.call.args) {
	  list_t *arglist = e->u.call.args->list;
	  for (int i=0; i<arglist->length; i++) {
	    A_exp a_exp = (A_exp)getNthElementList(arglist, i);
	    if ((rc = exprContainAggr(venv, a_exp, &has_aggr)))
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
      if ((rc = exprContainAggr(venv, e->u.op.left, resultp)))
	goto exit;
    }
    if (*resultp==0 && e->u.op.right) {
      if ((rc = exprContainAggr(venv, e->u.op.right, resultp)))
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
  err_t rc = ERR_NONE;

  int i, j;
  list_t *hxplist = gb->u.sqlopr->hxp_list;
  list_t *prdlist = gb->u.sqlopr->prd_list;

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
      for (j=0; j<prdlist->length; j++) {
	A_exp prd = (A_exp)getNthElementList(prdlist, j);
	if (equalExp(prd, hxp)) {
	  break;
	}
      }
      if (j>=prdlist->length) {
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
checkAggr(S_table tenv, A_exp sel, int *resultp)
{
  A_list gblist = sel->u.select.group_by_list;
  A_list hxplist = sel->u.select.hxp_list;
  list_t* hvcond = sel->u.select.hv_prd_list;
  list_t* wrcond = sel->u.select.wr_prd_list;
  err_t rc = ERR_NONE;
  int result;
  int i;

  *resultp = 0;

  /* check group-by column */
  if (gblist && gblist->list && gblist->list->length>0) {
    *resultp = 1;
    /* gb list can not have aggr */
    for (i=0; i<gblist->list->length; i++) {
      A_exp gb = (A_exp)getNthElementList(gblist->list, i);
      if ((rc = exprContainAggr(tenv, gb, &result))) {
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
  for (i=0; i<hvcond->length; i++) {
    A_exp prd = (A_exp)getNthElementList(hvcond, i);
    if ( (rc = exprContainAggr(tenv, prd, &result)) ) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkAggr", "exprcontainAggr");
      goto exit;
    }
    if (result==1)
      *resultp = 1;
  }

  /* check where cond : no aggr allowed */
  for (i=0; i<wrcond->length; i++) {
    A_exp prd = (A_exp)getNthElementList(wrcond, i);
    if ( (rc = exprContainAggr(tenv, prd, &result)) ) {
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
  if (hxplist && hxplist->list) {
    for (int i=0; i<hxplist->list->length; i++) {
      A_selectitem e = (A_selectitem)getNthElementList(hxplist->list, i);
      if (e->kind == SIMPLE_ITEM) {
	rc = exprContainAggr(tenv, e->u.s.exp, &result);
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
  case A_timestampExp:
  case A_stringExp: 
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
	       list_t *gb_hxp_list,    /* IO: head exprs in GB box */
	       char *gb_query_name,    /* IN: name of GB query */
	       list_t *sw_hxp_list,    /* IO: head exprs in SW box */
	       char *sw_query_name,    /* IN: name of SW query */
	       A_exp &sh_hxp,	    /* OUT: rewritten expr */
	       int *has_aggr	    /* OUT: found aggr? */
	       )
{
  err_t rc = ERR_NONE;
  char ident[80];
  A_selectitem si;
  int hxpidx;

  if ((rc = exprContainAggr(venv, expr, has_aggr))) {
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
	if (sh_hxp) sh_hxp = expr;
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
	  appendElementList(gb_hxp_list, (nt_obj_t*)gb_hxp);
	} else {
	  hxpidx = appendExp2HxpList(gb_hxp_list, gb_hxp);

	  sprintf(ident, "a_%d", hxpidx);
	  sh_hxp = A_SqlVarExp(expr->pos, 
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
	      list_t *args = expr->u.call.args->list;
	      list_t *newargs = newList();
	      int startpos = sw_hxp_list->length;

	      for (i=0; i<args->length; i++) {
		A_exp arg = (A_exp)getNthElementList(args, i);
		A_exp newarg;

		hxpidx = appendExp2HxpList(sw_hxp_list, arg);

		sprintf(ident, "a_%d", hxpidx);
		newarg = A_SqlVarExp(arg->pos,
				     new_Symbol(sw_query_name),
				     new_Symbol(ident));
		appendElementList(newargs, (nt_obj_t*)newarg);
	      }

	      clearList(args);
	      deleteList(args);
	      expr->u.call.args->list = newargs;
	    }

	    hxpidx = appendExp2HxpList(gb_hxp_list, expr);
	    sprintf(ident, "a_%d", hxpidx);
	    sh_hxp = A_SqlVarExp(expr->pos,
				 new_Symbol(gb_query_name),
				 new_Symbol(ident));
	  } else {
	    /* ..., f(x+y,aggr(z)+3), ... ==>

	       SH: ..., f(GB.c, GB.d+3), ...
	       GB: ..., SW.a c, aggr(SW.b) d, ...
	       SW: ..., x+y a, z b, ...

	       aggregtes appear in the arguments
	    */
	    if (expr->u.call.args) {
	      list_t *args = expr->u.call.args->list;

	      for (i=0; i<args->length; i++) 
		{
		  A_exp arg = (A_exp)getNthElementList(args, i);
		  A_exp new_arg = (A_exp)0;
		  rc = filterAggrExpr(venv, arg, gb_hxp_list, gb_query_name,
				      sw_hxp_list, sw_query_name,    
				      new_arg, has_aggr);	    
		  if (rc) {
		    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "filterAggrExpr", "filterAggrExpr");
		    goto exit;
		  }
		
		  overwriteElementList(args, i, (nt_obj_t*)new_arg);
		}
	    }
	    sh_hxp = expr;
	  }
	}
	break;
      case A_opExp:
	{
	  A_exp newl, newr;
	  rc = filterAggrExpr(venv, expr->u.op.left, gb_hxp_list, 
			      gb_query_name,
			      sw_hxp_list, sw_query_name, newl, has_aggr);
	  if (rc) {
	    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "filterAggrExpr", "filterAggrExpr");
	    goto exit;
	  }

	  rc = filterAggrExpr(venv, expr->u.op.right, gb_hxp_list, 
			      gb_query_name,
			      sw_hxp_list, sw_query_name, newr, has_aggr);
	  if (rc) {
	    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "filterAggrExpr", "filterAggrExpr");
	    goto exit;
	  }

	  expr->u.op.left = newl;
	  expr->u.op.right = newr;
	  sh_hxp = expr;
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
  giving an SQL statement:

       SELECT .. FROM .. WHERE .. GROUP BY .. HAVING .. ORDER BY ..

  construct a query graph composed of SEL/GB nodes.
 */
err_t constructSelOp(S_table venv, A_exp sel, A_exp &result)
{
  A_exp sw, gb, sh, od;
  list_t *alist;
  A_exp ele;
  A_exp sh_hxp;
  int i;
  int hasaggr;			// flag:  aggregate is used in SELECT
  A_list odlist = sel->u.select.order_by_list;
  A_list gblist = sel->u.select.group_by_list;
  A_list hxplist;
  list_t *hvcond = sel->u.select.hv_prd_list;
  list_t *wrcond = sel->u.select.wr_prd_list;
  A_qun qun;
  err_t rc = ERR_NONE;

  rc = expandSelectList(venv, sel); 
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "expandSelectList");
    goto exit;
  }
  
  hxplist = sel->u.select.hxp_list;

  rc = checkAggr(venv, sel, &hasaggr);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "checkAggr");
    goto exit;
  }

  if (hasaggr)
    {
      DBUG_EXECUTE("gbconvert", displayExp(sel););

      char *sw_name = getUniqueName("Q");
      char *gb_name = getUniqueName("Q");
      list_t *gb_hxp_list = newList();
      list_t *gb_cnd_list = newList();
      list_t *sh_hxp_list = newList();
      list_t *sh_cnd_list = newList();
      list_t *sw_hxp_list = newList();

      list_t *jtl1 = newList();
      list_t *jtl2 = newList();

      /* construct SEL-GB-SEL graph */
      /* decompose hxp_list */
      for (i=0; i<hxplist->list->length; i++) 
	{
	  A_selectitem si = (A_selectitem) getNthElementList(hxplist->list, i);
	  A_selectitem ti; 

	  if (si->kind == SIMPLE_ITEM) {
	    ele = si->u.s.exp;
	    rc = filterAggrExpr(venv, ele, gb_hxp_list, gb_name, 
				sw_hxp_list, sw_name, sh_hxp, &hasaggr);

	    if (rc) {
	      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "filterAggrExpr");
	      goto exit;
	    }

	    ti = A_SelectItem(si->pos, sh_hxp, si->u.s.alias);
	    appendElementList(sh_hxp_list, (nt_obj_t*)ti);
	  }
	}
      /* decompose gblist */
      for (i=0; gblist && gblist->list && i<gblist->list->length; i++) 
	{
	  A_exp dummy = (A_exp)0;
	  ele = (A_exp)getNthElementList(gblist->list, i);
	  rc = filterAggrExpr(venv, ele, 
			      gb_cnd_list,/* pass in group-by list
                                             instead */
			      sw_name,
			      sw_hxp_list, 
			      sw_name, 
			      dummy, /* The expr is in the group-by
					column, and will be put into
					the group-by list.*/
			      &hasaggr);
	  if (rc) {
	    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "filterAggrExpr");
	    goto exit;
	  }
	}

      /* decompose hvcond */
      for (i=0; hvcond && i<hvcond->length; i++) {
	ele = (A_exp)getNthElementList(hvcond, i);
	rc = filterAggrExpr(venv, ele, gb_hxp_list, gb_name,
			    sw_hxp_list, sw_name, sh_hxp, &hasaggr);
	if (rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "filterAggrExpr");
	  goto exit;
	}
	appendElementList(sh_cnd_list, (nt_obj_t*)sh_hxp);
      }
      
      /* construct SW box */
      if (sw_hxp_list->length == 0) {
	A_exp dummy = A_IntExp(sel->pos, 1);
	A_selectitem si = A_SelectItem(sel->pos, dummy, (S_symbol)0); 

	appendElementList(sw_hxp_list, (nt_obj_t*)si);
      }
      sw = A_SqlOprExp(sel->pos,
		       A_SQL_SEL,
		       0,	// distinct
		       sw_hxp_list,
		       sel->u.select.join_table_list->list,
		       wrcond);

      /* construct GB box */
      qun = A_QueryQun(sel->pos,
		       new_Symbol(sw_name),
		       sw);
      appendElementList(jtl1, (nt_obj_t*)qun);

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
      appendElementList(jtl2, (nt_obj_t*)qun);

      sh = A_SqlOprExp(sel->pos,
		       A_SQL_SEL,
		       sel->u.select.distinct,
		       sh_hxp_list,
		       jtl2,
		       sh_cnd_list);

      DBUG_EXECUTE("gbconvert", displayExp(sh););

      /* check if all non-aggregate hxp are group by columns */
      rc = checkNonAggregateHxp(gb);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "constructSelOp", "checkNonAggregateHxp");
	goto exit;
      }
    } /* has aggr */
  else 
    { 
      /* no aggr */
      sh = A_SqlOprExp(sel->pos,
		       A_SQL_SEL,
		       sel->u.select.distinct,
		       hxplist->list,
		       getList(sel->u.select.join_table_list),
		       wrcond);
      if (odlist) {
	/* order-by */
	list_t *od_hxp_list = newList();
	list_t *od_cnd_list = newList(); // order by columns
	list_t *od_jtl_list = newList();
	char *sh_name = getUniqueName("Q");
	char ident[5];

	/* construct QUN */
	qun = A_QueryQun(sel->pos,
			 new_Symbol(sh_name),
			 sh);
	appendElementList(od_jtl_list, (nt_obj_t*)qun);

	/* copy the hxp list of sh to od */
	for (i=0; i<hxplist->list->length; i++) {
	  A_selectitem si = (A_selectitem) getNthElementList(hxplist->list, i);
	  S_symbol alias = si->u.s.alias;
	  if (!alias) {
	    sprintf(ident, "%d", i);
	    alias = new_Symbol(ident);
	  }
	  A_exp od_hxp = A_SqlVarExp(si->pos,
				     new_Symbol(sh_name),
				     alias);
	  si = A_SelectItem(si->pos, od_hxp, (S_symbol)0);
	  appendElementList(od_hxp_list, (nt_obj_t*)si);
	}
	/* rename the order by */
	for (i=0; i<odlist->list->length; i++) {
	  A_exp oditem = (A_exp)getNthElementList(odlist->list, i);
	  /* is this order-by column in the hxp-list ? */
	  int hxpidx = appendExp2HxpList(hxplist->list, oditem);
	  sprintf(ident, "a_%d", hxpidx);
	  odhxp = A_SqlVarExp(expr->pos,
			      new_Symbol(sh_name),
			      new_Symbol(ident));
	  appendElementList(od_cnd_list, (nt_obj_t*)odhxp);
	}
	
	sh = A_SqlOprExp(sel->pos,
			 A_SQL_ORDER,
			 0,	// distinct
			 od_hxp_list,
			 od_jtl_list,
			 od_cnd_list
			 );
      } 
    }

  result = sh;
 exit:
  return rc;
}

/*
 *
 */
err_t expandSelectList(S_table venv, A_exp sel)
{
  err_t rc = ERR_NONE;
  A_list hxp_list = sel->u.select.hxp_list;
  A_selectitem si, nsi;
  E_enventry x;
  int i,j,k,processed;
  list_t *newselectlist = (list_t*)0;

  for (i=0; i<hxp_list->list->length; i++) {

    si = (A_selectitem) getNthElementList(hxp_list->list, i);
    processed = 0;

    switch(si->kind) {
    case SIMPLE_ITEM: 
      {
	A_exp e = si->u.s.exp;
	if (e->kind == A_callExp && e->u.call.member == (S_symbol)0) {

	  x = (E_enventry)S_look(venv, e->u.call.func);
	  if (!x) {
	    rc = ERR_UNDEFINED_FUNCTION;
	    EM_error (e->pos, rc, __LINE__, __FILE__, S_name(e->u.call.func));
	    goto exit;
	  }

	  if ((x->kind == E_funEntry || x->kind == E_aggrEntry) &&
	      x->u.fun.result->kind==Ty_record) {

	    list_t *list = x->u.fun.result->u.record;
	    for (j=0; j<list->length; j++) {
	      Ty_field f = (Ty_field)getNthElementList(list, j);
	      A_exp newe = copyExp(e);

	      newe->u.call.member = f->name;

	      if (j>0) e->u.call.shared |= SHARED_INVOCATION;
	      nsi = A_SelectItem(si->pos, newe, (S_symbol)0);

	      if (!newselectlist) {
		newselectlist = newList();
		for (k=0;k<i;k++) {
		  nt_obj_t *prev = getNthElementList(hxp_list->list, k);
		  appendElementList(newselectlist, prev);
		}
	      }

	      appendElementList(newselectlist, (nt_obj_t*)nsi);
	      processed = 1;
	    }
	  }
	}
      }
      break;
    case COMPLEX_ITEM:
      {
	A_exp e = si->u.c.exp;

	x = (E_enventry)S_look(venv, e->u.call.func);
	if (!x || (x->kind != E_funEntry && x->kind != E_aggrEntry)) {
	  rc = ERR_UNDEFINED_FUNCTION;
	  EM_error (e->pos, rc, __LINE__, __FILE__, S_name(e->u.call.func));
	  goto exit;
	}
	if (x->u.fun.result->kind!=Ty_record) {
	  rc = ERR_COMPLEX_FUNC_EXPECTED;
	  EM_error (e->pos, rc, __LINE__, __FILE__, S_name(e->u.call.func));
	  goto exit;
	}
	list_t *list = x->u.fun.result->u.record;
	if (list->length != si->u.c.aliaslist->list->length) {
	  rc = ERR_UNMATCH_CARDINALITY;
	  EM_error(e->pos, rc, __LINE__, __FILE__, S_name(e->u.call.func));
	  goto exit;
	}
	for (j=0; j<list->length; j++) {
	  Ty_field f = (Ty_field)getNthElementList(list, j);
	  S_symbol alias = (S_symbol)getNthElementList(si->u.c.aliaslist->list, j);
	  A_exp newe = copyExp(e);

	  newe->u.call.member = f->name;
	  if (j>0) e->u.call.shared |= SHARED_INVOCATION;

	  nsi = A_SelectItem(si->pos, newe, alias);

	  if (!newselectlist) {
	    newselectlist = newList();
	    for (k=0;k<i;k++) {
	      nt_obj_t *prev = getNthElementList(hxp_list->list, k);
	      appendElementList(newselectlist, prev);
	    }
	  }

	  appendElementList(newselectlist, (nt_obj_t*)nsi);
	  processed = 1;
	}
      }
      break;
    case STAR_ITEM:
      break;
    }

    if (!processed && newselectlist) 
      appendElementList(newselectlist, (nt_obj_t*)si);
  }

  if (newselectlist)
    sel->u.select.hxp_list = A_List(hxp_list->pos, newselectlist, A_SELECT_ITEM);

 exit:
  return rc;
}

