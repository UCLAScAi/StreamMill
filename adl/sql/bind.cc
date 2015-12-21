#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include <sql/symbol.h>
#include <sql/trans2C.h>
#include <sql/io.h>
#include <sql/const.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "util.h"
//#include <iostream>

extern char* cOper[];
/* an expression is bound if:
 * 
 * 1) it is a constant
 * 2) it is a formal parameter of a UDF/UDA
 * 3) it is from a QUN that precedes the current QUN in an OPR.
 */
err_t isExpBound(S_table venv, S_table tenv, 
		 A_exp a,	// the exp to be tested
		 A_sqlopr opr,	// the opr it belongs to
		 A_qun curqun,	// the qun it belongs to
		 Sql_sem sql, 
		 Tr_exp &exe)
{
  err_t rc = ERR_NONE;
  T_expty result;

  exe = (Tr_exp)0;
  switch (a->kind) {
  case A_varExp: 
    {
      A_qun outqun;
      int fieldidx, i;

      rc = transVar(venv, tenv, a->u.var, sql, &outqun, &fieldidx, result);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "isExpBound", "transVar");
	goto exit;
      }

      if (!outqun) {
	/* this variable does not belong to any QUN;
	   it is a formal parameter */
	exe = result->exp;
	goto exit;
      } 

      for (i=0; i<opr->jtl_list->length; i++) {
	A_qun q = (A_qun)getNthElementList(opr->jtl_list, i);
	if (q == curqun) {
	  /* the variable belongs to a QUN that comes after the QUN we
             are compiling, thus it is not bound */
	  goto exit;
	}
	if (q == outqun) {
	  /* we haven't met "curqun" yet, so "outqun" comes before us,
             thus the variable is bound */
	  exe = result->exp;
	  goto exit;
	}
      }
    }
    break;
  case A_intExp: 
    result = expTy(Tr_Int(a->u.intt), Ty_Int());
    exe = result->exp;
    break;
  case A_stringExp: 



    result = expTy(Tr_QuoteString(a->u.string), Ty_String());
    exe = result->exp;
    break;

  case A_timestampExp:
    result = expTy(Tr_Timestamp(a->u.timestamp), Ty_Timestamp());
    exe = result->exp;
    break;
  case A_opExp:
    {

      A_oper oper = a->u.op.oper;
      switch (oper) {
      case A_plusOp:
      case A_minusOp:
      case A_timesOp:
      case A_divideOp:
      case A_modOp:
	{
	  Tr_exp lexe, rexe;
	  char buf[MAX_STR_LEN];

	  // check left hand side 
	  rc = isExpBound(venv, tenv, a->u.op.left, opr, curqun, sql, lexe);
	  if (rc) {
	    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "isExpBound", "isExpBound");
	    goto exit;
	  }
	  if (!lexe) {
	    goto exit;
	  }
	  // check right hand side
	  rc = isExpBound(venv, tenv, a->u.op.right, opr, curqun, sql, rexe);
	  if (rc) {
	    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "isExpBound", "isExpBound");
	    goto exit;
	  }
	  if (!rexe) {
	    Tr_Delete(lexe);
	    goto exit;
	  }

	  // now, both the left hand side and the right hand side are bound
	  sprintf(buf, "(%s %s %s)", lexe, cOper[(int)oper], rexe);
	  exe = Tr_String(buf);
	}
	break;
      default:
	break;
      }
    }
    break;
  default:
    break;
  }
 exit:
  return rc;
}

/*
 * Detect:
 * 1) the qunidx-th QUN of a SELECT OPR is a NAME_QUN

 * 2) all the keys of its table are bound by a constant value, a
 * formal parameter, or a QUN that precedes the current QUN in the
 * FROM clause 
 */
err_t isQunBound(S_table venv, 
		 S_table tenv, 
		 A_sqlopr a, 
		 int qunidx, 
		 Sql_sem sql, 
		 Tr_exp *&exep,
		 Tr_exp *&exepUpper,
		 Tr_exp *&exepLower,
		 int &flag
)
{
  err_t rc = ERR_NONE;
  int i,j;
  A_qun qun;
  E_enventry x; 
  int boundp;
  Ty_field f;
  A_list fieldlist;
  int index = 1;

  exep = (Tr_exp*)0;
  exepUpper = (Tr_exp*)0;
  exepLower = (Tr_exp*)0;
  flag = BOUND_NONE;
  if (!a->jtl_list || a->jtl_list->length == 0 ||
      !a->prd_list || a->prd_list->length == 0) {
    goto exit;
  }

  qun = (A_qun)getNthElementList(a->jtl_list, qunidx);

  if (qun->kind != QUN_NAME) {
    goto exit;
  }

  x = (E_enventry)S_look(venv, qun->u.table.name);
  if(x && x->kind == E_dynamicEntry) {
    x = (E_enventry)S_look(venv, x->u.dynamic.table);
  }

  if (!x || x->kind != E_varEntry) {
    rc = ERR_UNDEFINED_VARIABLE;
    EM_error(qun->pos, rc, __LINE__, __FILE__, S_name(qun->u.table.name));
    goto exit;
  }
  
  if (x->u.var.ty->kind != Ty_record) {
    rc = ERR_TUPLE_TYPE_REQUIRED;
    EM_error(qun->pos, rc, __LINE__, __FILE__, S_name(qun->alias));
    goto exit;
  }
  
  if (x->u.var.haskey == 0) {
    goto exit;
  }

  fieldlist = x->u.var.ty->u.record; 
  if(x->u.var.index != (A_index)0) 
    index = x->u.var.index->kind;
  
  
  /* exep is an arrays that stores for each key of the table, the
     value it is bound to */
  exep = (Tr_exp*)ntMalloc(sizeof (Tr_exp) * fieldlist->length);
  exepUpper = (Tr_exp*)ntMalloc(sizeof (Tr_exp) * fieldlist->length);
  exepLower = (Tr_exp*)ntMalloc(sizeof (Tr_exp) * fieldlist->length);
  memset(exep, 0, sizeof(Tr_exp) * fieldlist->length);
  memset(exepUpper, 0, sizeof(Tr_exp) * fieldlist->length);
  memset(exepLower, 0, sizeof(Tr_exp) * fieldlist->length);
  for (j=0; j<a->prd_list->length; j++) {
    A_exp prd = (A_exp)getNthElementList(a->prd_list, j);
    A_exp binding_exp;

    if (prd->kind == A_opExp) {
      A_oper oper = prd->u.op.oper;

      if (oper == A_eqOp || oper == A_ltOp || oper == A_leOp || oper == A_geOp ||
	  oper == A_gtOp) {
	// is the current qun on any side of the operator?
	A_qun outqun = (A_qun)0;
	int fieldidx;
	T_expty result;

	if (prd->u.op.left->kind == A_varExp) {
	  rc = transVar(venv, tenv, 
			prd->u.op.left->u.var, 
			sql, &outqun, &fieldidx, result);
	  if (rc) {
	    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "isQunBound", "transVar");
	    goto exit;
	  }
	  binding_exp = prd->u.op.right;
	}
	if (outqun != qun && prd->u.op.right->kind == A_varExp) {
	  rc = transVar(venv, tenv, 
			prd->u.op.right->u.var, 
			sql, &outqun, &fieldidx, result);
	  if (rc) {
	    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "isQunBound", "transVar");
	    goto exit;
	  }
	  binding_exp = prd->u.op.left;
	}


	switch (oper) {
	case A_ltOp:
	case A_leOp:
	  // the current QUN is on one side of the oper,  
	  if (outqun == qun) {
	    f = (Ty_field)getNthElementList(fieldlist, fieldidx);
	    if (f->iskey) {
	      rc = isExpBound(venv, tenv,
			      binding_exp,
			      a,
			      qun,
			      sql,
			      exepUpper[fieldidx]);
	      if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "isQunBound", "isExpBound");
		goto exit;
	      }
	      
	    }
	  }
	  
	  break;
	case A_gtOp:
	case A_geOp:
	  // the current QUN is on one side of the oper,  
	  if (outqun == qun) {
	    f = (Ty_field)getNthElementList(fieldlist, fieldidx);
	    if (f->iskey) {
	      rc = isExpBound(venv, tenv,
			      binding_exp,
			      a,
			      qun,
			      sql,
			      exepLower[fieldidx]);
	      if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "isQunBound", "isExpBound");
		goto exit;
	      }
	      
	    }
	  }

	  break;
	case A_eqOp:
	  // the current QUN is on one side of the oper,  
	  if (outqun == qun) {
	    f = (Ty_field)getNthElementList(fieldlist, fieldidx);
	    if (f->iskey) {
	      rc = isExpBound(venv, tenv,
			      binding_exp,
			      a,
			      qun,
			      sql,
			      exep[fieldidx]);
	      memcpy(&exepUpper[fieldidx], &exep[fieldidx], sizeof(exep[fieldidx]));
	      memcpy(&exepLower[fieldidx], &exep[fieldidx], sizeof(exep[fieldidx]));
	      if (rc) {
		EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "isQunBound", "isExpBound");
		goto exit;
	      }
	      
	    }
	  }
	  break;
	}
      }
    }
  } // end of for()  


  switch (index){
  case INDEX_BTREE:
    /* determine if all key bounded by equality condition */
    boundp = 1;
    for (i=0; i<fieldlist->length; i++) {
      f = (Ty_field) getNthElementList(fieldlist, i);
      if (f->iskey && exep[i]==(Tr_exp)0) {
	boundp = 0;
	break;
      }
    }
    if (boundp == 1) {
      flag = BOUND_ALL;
      goto exit;
    }

    /* Partial key bound?
       For simplicity, we only consider the first key 
    */
    
    boundp = 1;
    i = x->u.var.firstKey;
    f = (Ty_field) getNthElementList(fieldlist, i);
    if (!f->iskey) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "isQunBound", "getFirstKey");
      goto exit;
    }
    if (exep[i]==(Tr_exp)0 && 
	exepUpper[i] == (Tr_exp)0 && 
	exepLower[i] == (Tr_exp)0){ // no binding
      boundp = 0;
	}
    else{ // use binding, 
	  /* Since we only consider the first key, 
	     we need to erase other keys' binding 
	  */
      flag = BOUND_FIRST;
      if (exep[i] == (Tr_exp)0){ // don't have equality binding
	ntFree(exep);
	exep = (Tr_exp*)0;
	if (exepUpper[i] == (Tr_exp)0){
	  ntFree(exepUpper);
	  exepUpper = (Tr_exp*)0;
	}
	if (exepLower[i] == (Tr_exp)0){
	  ntFree(exepLower);
	  exepLower = (Tr_exp*)0;
	}
      }
      else{ // has equality binding, we don't need inequality binding
	ntFree(exepUpper);
	ntFree(exepLower);
	exepUpper = (Tr_exp*)0;
	exepLower = (Tr_exp*)0;
      }
      
    }
    
    break;
  case INDEX_RTREE:
    /* Different from BTREE, we only need to check if at least one of 
       RTREE's keys is bound */
    boundp=0;
    for (i=0;i<fieldlist->length;i++){
      f = (Ty_field) getNthElementList(fieldlist, i);
      if (f->iskey && (exepLower[i]!=(Tr_exp)0 || exepUpper[i] != (Tr_exp)0)) {
	boundp = 1;
	flag = BOUND_RTREE;
	break;
      }
     
    }
  default:
    break;
  } // end switch
 exit:
  if (flag == BOUND_NONE) {
    ntFree(exep);
    ntFree(exepUpper);
    ntFree(exepLower);
    exep = (Tr_exp*)0;
    exepUpper = (Tr_exp*)0;
    exepLower = (Tr_exp*)0;
  }
  return rc;
}
