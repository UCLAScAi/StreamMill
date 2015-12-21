#include <sql/adl_sys.h>
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

#include <vector>
using namespace std;


extern A_list global_functions;
extern system_t *ntsys;
extern err_t searchRecordField(A_list fields,
			       S_symbol fname, 
			       int *idx, // field index
			       int *off, // offset of the field in the record
			       Ty_field &result // OUTPUT
			       );

/******************************************************************************
 * Optimization regarding OID:
 *
 *  1.  UPDATE table SET a=a+...
 *      WHERE OID = expr AND b=;
 *
 *      UPDATE table SET a=a+...
 *      WHERE OID IN (SELECT refb FROM tab2) AND b=...;
 *
 *      ---- We rewrite it to:
 *
 *	SELECT t.field0->a=t.field0->a+...
 *      FROM (VALUES(expr)) AS t
 *      WHERE t.field0->b = ... ;     --- expr is not a subquery
 *
 *      ---- or
 *
 *	SELECT refb->a=refb->a+...
 *      FROM tab2        
 *      WHERE refb->b=...             --- expr is a subquery
 *
 *  2.  SELECT ...
 *      FROM t1, t2, ...
 *      WHERE t1.OID = expr;
 *      ---- Note: t1 is not used inside expr ----
 *
 *  3.  ---- Note: another possibility: OID IN expr ----
 *      SELECT ...
 *      FROM (expr) AS t1, t2, ...
 *
 *  Since 3) is already supported, and 2) is a special case of 3), we
 *  only implement 1) here.
 ********************************************************************************/

int isOIDVar(A_exp a, S_symbol tabname)
{
  int oidvarp = 0;

  if (a->kind == A_varExp) {
    S_symbol oid = S_Symbol("oid");

    if (a->u.var->kind == A_simpleVar &&
	a->u.var->u.simple == oid)
      oidvarp = 1;
    else if (a->u.var->kind == A_fieldVar &&
	     a->u.var->u.field.var->u.simple == tabname &&
	     a->u.var->u.field.sym == oid)
      oidvarp = 1;
  }
  return oidvarp;
}

/* if var is defined in x, replace it with pointer->var */
err_t replaceVar(S_table venv, E_enventry x, A_var var, A_var pointer, A_ref &result)
{
  err_t rc = ERR_NONE;
  E_enventry te;

  switch (var->kind) {
  case A_fieldVar:
    {
      A_var t = var->u.field.var;
      te = (E_enventry)S_look(venv, t->u.simple);
      if (te==x) {
	/* replace */
	result = A_Ref(var->pos, pointer, var->u.field.sym);
      }
    }
    break;
  case A_simpleVar:
    {
      Ty_field f;

      te = (E_enventry)S_look(venv, var->u.simple);
      if (!te) {
	/* check if it belongs to table x */
	rc = searchRecordField(x->u.var.ty->u.record, 
			       var->u.simple, 
			       (int*)0,
			       (int*)0,	// offset
			       f);
	if (rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceVar", "searchRecordField");
	  goto exit;
	}
	if (f) {
	  /* replace */
	  result = A_Ref(var->pos, pointer, var->u.simple);
	}
      }
    }
    break;
  }
 exit:
  return rc;
}
err_t replaceOID(S_table venv, A_var refvar, E_enventry x, A_exp exp)
{
  err_t rc = ERR_NONE;
  int i;

  switch(exp->kind) {
  case A_sqloprExp:
    {
      A_sqlopr a = exp->u.sqlopr;
      for (i=0; i<a->hxp_list->length; i++) {
	A_exp item;
	if (a->kind == A_SQL_UPDATE) {
	  item = (A_exp) getNthElementList(a->hxp_list, i);
	} else {
	  A_selectitem arg = (A_selectitem) getNthElementList(a->hxp_list, i);
	  if (arg->kind != SIMPLE_ITEM) {
	    rc = ERR_NTSQL_INTERNAL;
	    EM_error(0, rc, __LINE__, __FILE__, "not implemented expr type in replaceOID()");
	    goto exit;
	  }
	  item = arg->u.s.exp;
	}
	rc = replaceOID(venv, refvar, x, item);
	if (rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceOID", "replaceOID");
	  goto exit;
	}
      }

      for (i=0; i<a->prd_list->length; i++) {
	A_exp prd = (A_exp)getNthElementList(a->prd_list, i);
	rc = replaceOID(venv, refvar, x, prd);
	if (rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceOID", "replaceOID");
	  goto exit;
	}
      }
    }
    break;
  case A_assignExp:
    {
      A_ref aref = (A_ref)0;

      /* left hand side */
      rc = replaceVar(venv, x, exp->u.assign.var, refvar, aref);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceOID", "replaceVar");
	goto exit;
      }
      exp->u.assign.var = A_RefVar(exp->u.assign.var->pos, aref);

      /* right hand side */
      rc = replaceOID(venv, refvar, x, exp->u.assign.exp);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceOID", "replaceOID");
	goto exit;
      }
    }
    break;
  case A_opExp:
    {
      rc = replaceOID(venv, refvar, x, exp->u.op.left);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceOID", "replaceOID");
	goto exit;
      }
      if (exp->u.op.right) {
	rc = replaceOID(venv, refvar, x, exp->u.op.right);
	if (rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceOID", "replaceOID");
	  goto exit;
	}
      }
    }
    break;
  case A_varExp:
    {
      A_ref aref = (A_ref)0;

      rc = replaceVar(venv, x, exp->u.var, refvar, aref);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceOID", "replaceVar");
	goto exit;
      }
      if (aref) {
	exp->kind = A_refExp;
	exp->u.ref = aref;
      }
    }
    break;
  case A_callExp:
    {
      int i;
      
      if (exp->u.call.args) {
	for (i=0; i<exp->u.call.args->length; i++) {
	  A_exp arg_exp = (A_exp) getNthElementList(exp->u.call.args, i);
	  rc = replaceOID(venv, refvar, x, arg_exp);
	  if (rc) {
	    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceOID", "replaceOID");
	    goto exit;
	  }
	}
      }
    }
    break;
  case A_refExp:
    {
      /* get the first var */
      A_ref aref = (A_ref)0;
      A_ref ref = exp->u.ref;
      while (ref->kind == A_refRef) {
	ref = ref->u.ref;
      }
      rc = replaceVar(venv, x, ref->u.var, refvar, aref);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "replaceOID", "replaceVar");
	goto exit;
      }
      if (aref) {
	ref->kind = A_refRef;
	ref->u.ref = aref;
      }
    }
    break;
  case A_intExp:
  case A_realExp:
  case A_stringExp:
  case A_timestampExp:
  case A_nilExp:
    /* no action */
    break;
  default:
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "replaceOID");
    goto exit;
  }
 exit:
  return rc;
}
err_t rewriteUpdate(S_table venv, S_table tenv, A_exp exp, vector<void*> aggregates)
{
  err_t rc = ERR_NONE;
  A_qun q;
  E_enventry x;
  int i;
  int replaced;
  A_sqlopr a = exp->u.sqlopr;
  A_exp oid_exp;

  if (!a->jtl_list || a->jtl_list->length ==0 ||
      !a->prd_list || a->prd_list->length == 0)
    goto exit;

  q = (A_qun)getNthElementList(a->jtl_list, 0);

  if (q->kind != QUN_NAME)
    goto exit;

  x = (E_enventry)S_look(venv, q->u.table.name);
  if(x && x->kind == E_dynamicEntry) {
    x = (E_enventry)S_look(venv, x->u.dynamic.table);
  }
  if (!x || x->kind != E_varEntry) {
    rc = ERR_UNDEFINED_VARIABLE;
    EM_error(q->pos, rc, __LINE__, __FILE__, S_name(q->u.table.name));
    goto exit;
  }

  if (x->u.var.scope != TAB_MEMORY)
    goto exit;

  for (i=0; i<a->prd_list->length; i++) {
    A_exp prd = (A_exp) getNthElementList(a->prd_list, i);
    oid_exp = (A_exp)0;

    if (prd->kind == A_opExp && 
	(prd->u.op.oper == A_eqOp || prd->u.op.oper == A_inOp)) 
      {
	A_exp var_exp;
	
	if (isOIDVar(prd->u.op.left, q->u.table.name)) {
	  oid_exp = prd->u.op.right;
	} else if (isOIDVar(prd->u.op.right, q->u.table.name)) {
	  oid_exp = prd->u.op.left;
	}

	if (oid_exp && oid_exp->kind != A_nilExp) {
	  /* try to compile oid_exp without the info of the table being updated */
	  T_expty subdec;
	  T_expty subexe;
	  Sql_sem subsql = SqlSem();
	  int oldlen = global_functions->length;
	  ntsys->noerrmsg = 1;
	  rc = transExp(venv, tenv, oid_exp, subsql, subdec, subexe, aggregates);
	  ntsys->noerrmsg = 0;
	  if (global_functions->length>oldlen) {
	    removeNthElementList(global_functions,oldlen);
	  }
	  SqlSem_Delete(subsql);
	  if (subdec) expTy_Delete(subdec);
	  if (subexe) expTy_Delete(subexe);

	  if (!rc) {
	    /* We found it! oid_exp can be computed independently. */
	    removeNthElementList(a->prd_list, i);
	    break;
	  }
	}
      }
  }

  if (oid_exp) {
    switch(oid_exp->kind) {
    case A_sqloprExp: 
      {
	A_sqlopr oid_sel = oid_exp->u.sqlopr;
	A_selectitem arg;
	A_exp refexp;

	/* OID = (SELECT ...) : can only have one hxp in the subquery */
	if (oid_sel->hxp_list->length > 1) {
	  goto exit;
	}

	/* it must be a variable */
	arg = (A_selectitem)removeNthElementList(oid_sel->hxp_list, 0);
	refexp = arg->u.s.exp;
	if (refexp->kind != A_varExp) {
	  goto exit;
	}

	/* add OID in exp */
	rc = replaceOID(venv, refexp->u.var, x, exp);
	if (rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteUpdate", "replaceOID");
	  goto exit;
	}
	a->kind = A_SQL_SEL;

	/* memory leak of a->jtl_list ignored :( */
	a->jtl_list = oid_sel->jtl_list;
	oid_sel->jtl_list = (A_list)0;
	if (oid_sel->prd_list) {
	  appendListList(a->prd_list, oid_sel->prd_list);
	}
	
	for (i=0; i<a->hxp_list->length; i++) {
	  A_exp item = (A_exp)getNthElementList(a->hxp_list, i);
	  A_selectitem sitem = A_SelectItem(item->pos, item, 0);
	  overwriteElementList(a->hxp_list, i, (nt_obj_t*)sitem);
	}
      }
      break;
    case A_opExp:
    case A_intExp:
    case A_realExp:
    case A_stringExp:
    case A_timestampExp:
      break;
    case A_varExp:
      break;
    default:
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__, "unexpected expr type in rewriteUpdate()");
      goto exit;
    }
  }
 exit:
  return rc;
}



