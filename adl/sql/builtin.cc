#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include <sql/symbol.h>
#include <sql/trans2C.h>
#include <sql/io.h>
#include <sql/sql_rewrite.h>
#include <SMLog.h>
#include <stdio.h>
#include <string.h>

/* Note: Many builtin functions in AXL are standard C library
 * functions. We can call those C functions directly. However in AXL,
 * each builtin function need to return a value, so there are no void
 * builtin functions. To port a C function whose return type is void,
 * we can simply use (voidfun(..),0) which will return an integer: 0.
 * */

#include <vector>
using namespace std;


extern "C" {
#include <dbug/dbug.h>
}

#include <sql/list.h>
#include "util.h"


err_t
transFuncOid(S_table venv,		/* variable env */
	     S_table tenv,		/* type env */
	     A_exp a,		/* abstract syntacs */
	     Sql_sem sql,		/* sql semantics */
	     T_expty &dec,		/* OUTPUT: declartion part */
	     T_expty &exe		/* OUTPUT: executable part */
	     )
{
  SMLog::SMLOG(10, "Entering transFuncOid");
  err_t rc = ERR_NONE;

  A_sqlopr insert_node = sql->top_sqlopr;
  A_qun target;
  E_enventry x, y;
  A_list hxp_list, column_list, arg_list;
  int col, i, n_keys;
  T_expty argdec[16], argexe[16];
  Ty_ty ty; 
  Ty_field ref_field;

  /* oid is a builtin function. It can have arbitrary number of
     parameters. These parameters form a key of a table. oid() returns
     the OID of the tuple that has the key in that table.  */


  /* The top node must be an INSERT node. The builtin
               function oid is only used in the following syntax:

	       INSERT INTO t VALUES(..., oid(...), ...)  */
  if (insert_node->kind != A_SQL_INSERT) {
    rc = ERR_OID_INSERTION;
    EM_error(0, rc, __LINE__, __FILE__, "");
    goto exit;
  }

  /* Values are inserted into a table, not the stdout */
  target = (A_qun)getNthElementList(insert_node->jtl_list, 0);
  if (target->u.table.name == S_Symbol("stdout")) {
    rc = ERR_OID_NONEXIST;
    EM_error(0, rc, __LINE__, __FILE__, "stdout");
    goto exit;
  }

  /* The table must have been defined. */
  x = (E_enventry)S_look(venv, target->u.table.name);
  if (!x || x->kind != E_varEntry) {
    rc = ERR_UNDEFINED_VARIABLE;
    EM_error(target->pos, rc, __LINE__, __FILE__, S_name(target->u.table.name));
    goto exit;
  }
	    
  /* which column? */
  if (a->ppt != A_SELECT_ITEM) {
    rc = ERR_OID_ARITHMETIC;
    EM_error(a->pos, rc, __LINE__, __FILE__);
    goto exit;
  }
  if (((A_selectitem)a->pp)->ppt != A_LIST) {
    rc = ERR_NTSQL_INTERNAL;
    EM_error(a->pos, rc, __LINE__, __FILE__, "oid");
    goto exit;
  }

  hxp_list = (A_list)(((A_selectitem)a->pp)->pp);
  for (col=0; col<hxp_list->length; col++) {
    A_selectitem hxp = (A_selectitem)getNthElementList(hxp_list, col);
    if (hxp==a->pp)
      break;
  }
  if (col>hxp_list->length) {
    rc = ERR_NTSQL_INTERNAL;
    EM_error(a->pos, rc, __LINE__, __FILE__, "oid");
    goto exit;
  }

  /* is this column a reference type? */
  ty = x->u.var.ty;
  if (ty->kind != Ty_record) {
    rc = ERR_TUPLE_TYPE_REQUIRED;
    EM_error(target->pos, rc, __LINE__, __FILE__, S_name(target->u.table.name));
    goto exit;
  }

  ref_field = (Ty_field)getNthElementList(ty->u.record, col);
  if (!ref_field || ref_field->ref == (S_symbol)0) {
    rc = ERR_REF_TYPE_MISMATCH;
    EM_error(target->pos, rc, __LINE__, __FILE__, col, S_name(target->u.table.name));
    goto exit;
  }
  
  /* do they have compatible keys? */
  y = (E_enventry)S_look(venv, ref_field->ref);
  if (!y || y->kind != E_varEntry || y->u.var.ty->kind != Ty_record) {
    rc = ERR_UNDEFINED_VARIABLE;
    EM_error(target->pos, rc, __LINE__, __FILE__, S_name(ref_field->ref));
    goto exit;
  }

  column_list = y->u.var.ty->u.record;
  arg_list = a->u.call.args;

  /* count the keys */
  n_keys=0;
  for (i=0; i<column_list->length; i++) {
    Ty_field field = (Ty_field)getNthElementList(column_list, i);
    if (field->iskey)
      n_keys++;
  }
  if (n_keys != arg_list->length) {
    rc = ERR_KEY_REF_MATCH;
    EM_error(a->pos, rc, __LINE__, __FILE__);
    goto exit;
  }
	
  n_keys=0;
  for (i=0; i<column_list->length; i++) {
    Ty_field field = (Ty_field)getNthElementList(column_list, i);

    if (field->iskey) {
      A_exp arg = (A_exp)getNthElementList(arg_list, n_keys);
      vector<void*> aggregates;
      rc = transExp(venv, tenv, arg, sql, argdec[n_keys], argexe[n_keys], aggregates);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transFuncOid", "transExp");
	goto exit;
      }
      if (argexe[n_keys]->ty->kind != field->ty->kind) {
	rc = ERR_KEY_REF_MATCH;
	EM_error(a->pos, rc, __LINE__, __FILE__);
	goto exit;
      }

      n_keys++;
    }
  }
  
 exit:
  return rc;
}


err_t
transBuiltInFunc(S_table venv,		/* variable env */
		 S_table tenv,		/* type env */
		 A_exp a,		/* abstract syntacs */
		 Sql_sem sql,		/* sql semantics */
		 T_expty &dec,		/* OUTPUT: declartion part */
		 T_expty &exe,		/* OUTPUT: executable part */
                 vector<void*> aggregates
		 )
{
  SMLog::SMLOG(10, "Entering transBuiltInFunc");
  err_t rc = ERR_NONE;
  A_list arg_list = a->u.call.args;
  T_expty argdec, argexe;

  dec = exe = (T_expty)0;

  //the following assertion should be guarranteed by syntax
  //if (bfid != OID && A_ListEmpty(arglist)) {
  //   goto exit;
  //}

  if (a->u.call.func == S_Symbol("rand")) {
    exe = expTy("(rand()/(RAND_MAX+1.0))", Ty_Real());
  } else if (a->u.call.func == S_Symbol("pow")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    argexe = expTy_Seq(argexe, ", (double)");

    T_expty argexe1;
    arg = (A_exp)getNthElementList(arg_list, 1);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe1, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }
    argexe = expTy_Seq(argexe, argexe1);

      // to do : check arg's type

    // Note: C function srand() returns void. To make things simple,
    // we use (srand(),0) so that calls of srand() in AXL always
    // return 0, an integer.
    exe = expTy_Seq(exe, "(pow((double)");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, "))");
    exe->ty = Ty_Real();
  } else if (a->u.call.func == S_Symbol("log")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    T_expty argexe1;
    arg = (A_exp)getNthElementList(arg_list, 1);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe1, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }
      // to do : check arg's type

    // Note: C function srand() returns void. To make things simple,
    // we use (srand(),0) so that calls of srand() in AXL always
    // return 0, an integer.
    exe = expTy_Seq(exe, "(log((double)");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, ")/log((double)");
    exe = expTy_Seq(exe, argexe1);
    exe = expTy_Seq(exe, "))");
    exe->ty = Ty_Real();
  } else if (a->u.call.func == S_Symbol("ceil")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    exe = expTy_Seq(exe, "(ceil((double)");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, "))");
    exe->ty = Ty_Int();
  } else if (a->u.call.func == S_Symbol("gettimeval")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    exe = expTy_Seq(exe, "getTimeval(");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, ")");
    exe->ty = Ty_Timestamp();
    exe->size = sizeof(char)*strlen(exe->exp);
  } else if (a->u.call.func == S_Symbol("timetostring")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    exe = expTy_Seq(exe, "__timetostring(");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, ")");
    exe->ty = Ty_String();
    exe->size = sizeof(char)*strlen(exe->exp);
  } else if (a->u.call.func == S_Symbol("inttostring")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    exe = expTy_Seq(exe, "(__intToString(");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, "))");
    exe->ty = Ty_String();
    exe->size = 100-1;
  } else if (a->u.call.func == S_Symbol("realtostring")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    exe = expTy_Seq(exe, "(__realToString(");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, "))");
    exe->ty = Ty_String();
    exe->size = 100-1;
  } else if (a->u.call.func == S_Symbol("stringtoint")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    exe = expTy_Seq(exe, "(__stringToInt(");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, "))");
    exe->ty = Ty_Int();
  } else if (a->u.call.func == S_Symbol("stringtoreal")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    exe = expTy_Seq(exe, "(__stringToReal(");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, "))");
    exe->ty = Ty_Real();
  } else if (a->u.call.func == S_Symbol("inttoreal")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    exe = expTy_Seq(exe, "((double)");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, ")");
    exe->ty = Ty_Real();
  } else if (a->u.call.func == S_Symbol("realtoint")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

    exe = expTy_Seq(exe, "((int)");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, ")");
    exe->ty = Ty_Int();
  } else if (a->u.call.func == S_Symbol("srand")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

      // to do : check arg's type

    // Note: C function srand() returns void. To make things simple,
    // we use (srand(),0) so that calls of srand() in AXL always
    // return 0, an integer.
    exe = expTy_Seq(exe, "(srand(");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, "), 0)");

  } else if (a->u.call.func == S_Symbol("sqrt")) {
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }

      // to do : check arg's type
    exe = expTy_Seq(exe, "sqrt(");
    exe = expTy_Seq(exe, "(double)(");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, "))");

      // return type
    exe->ty = Ty_Real();

  } else if (a->u.call.func == S_Symbol("timeofday")) {
    /* __timeofday() is implemented in adllib.c */
    exe = expTy("__timeofday()", Ty_Timestamp());
    exe->size = 20;
  }
  else if(a->u.call.func == S_Symbol("oldest")) {
    A_exp arg;
    A_var var;
    S_symbol table;
    E_enventry t;
    Ty_ty type = NULL;
    char buf[4096];
    int size = 0;

    //verify that we are in aggr
    if(sql->in_func != 1) {
      rc = ERR_INVALID_OLDEST_USE;
      EM_error(0, rc, __LINE__, __FILE__, "Oldest can only be used inside an aggregate");
      goto exit;
    }

    //verify only one param and it is table.field
    if(arg_list == NULL || arg_list->length != 1) {
      rc = ERR_INVALID_OLDEST_USE;
      EM_error(0, rc, __LINE__, __FILE__, "Oldest can only be called with one argument");
      goto exit;
    }

    //This check and the next check are redundant since they are generated by our
    // parser but just incase
    arg = (A_exp)getNthElementList(arg_list, 0);
    if(arg->kind != A_varExp || arg->u.var->kind != A_fieldVar) {
      rc = ERR_INVALID_OLDEST_USE;
      EM_error(0, rc, __LINE__, __FILE__, "Oldest can only be called with table.column");
      goto exit;
    }
    var = arg->u.var;
    table = var->u.field.var->u.simple;
    if(strcmp(S_name(table), "inwindow") != 0) {
      rc = ERR_INVALID_OLDEST_USE;
      EM_error(0, rc, __LINE__, __FILE__, "Oldest can only be called on inwindow table");
      goto exit;
    }


    //verify table exists and field in the table exists
    t = (E_enventry)S_look(venv, table);
    if(!t) {
      rc = ERR_INVALID_OLDEST_USE;
      EM_error(0, rc, __LINE__, __FILE__, "Oldest can only be called on inwindow table, which is only available in windowed aggregates");
      goto exit;
    }

    if(var->u.field.sym == (S_symbol)0 && t->u.var.ty->u.record->length == 1) {
      Ty_field fi = (Ty_field)getNthElementList(t->u.var.ty->u.record, 0);
      //printf("checking %s %s\n", S_name(fi->name), S_name(var->u.field.sym));
      var->u.field.sym = fi->name;
      type = fi->ty;
      size = fi->size;
    }
    else if(var->u.field.sym == (S_symbol)0) {
      rc = ERR_INVALID_OLDEST_USE;
      EM_error(0, rc, __LINE__, __FILE__, "Oldest used without argument, which is only alloed if the inwindow table only has one column");
      goto exit;
    }
    else {
      for(int i = 0; i < t->u.var.ty->u.record->length; i++) {
	Ty_field fi = (Ty_field)getNthElementList(t->u.var.ty->u.record, i);
	//printf("checking %s %s\n", S_name(fi->name), S_name(var->u.field.sym));
	if(strcmp(S_name(fi->name), S_name(var->u.field.sym)) == 0) {
	  type = fi->ty;
	  size = fi->size;
	  break;
	}
      }
    }
    if (type == (Ty_ty)0) {
      rc = ERR_UNDEFINED_FIELD;
      EM_error(a->pos, rc, __LINE__, __FILE__, S_name(var->u.field.sym));
      goto exit;
    }    

    //call getLastTuple with window and tuple
    //assign exe correctly
    sprintf(buf, "(getLastTuple_%s(window, &tuple, bm)->%s)", 
	    sql->func_name, S_name(var->u.field.sym)
	    );
    exe = expTy(buf, type);
    exe->size = size;
  }
  /*else if (a->u.call.func == S_Symbol("TUPLE_ID")) {
    exe = expTy("(status->win->getTupleID())", Ty_Int());
    exe->size = sizeof(int);
    }*/
  else if (a->u.call.func == S_Symbol("concat")) {
    int n = 2;			// to do!!
    exe = expTy_Seq(exe, "__strcat(");
    int size = 0;
    for (int i=0; i<n; i++) {
      A_exp arg = (A_exp)getNthElementList(arg_list, i);
      rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc",   "transExp");
	goto exit;
      }
      exe = expTy_Seq(exe, argexe);
      if (i<n-1) {
	exe = expTy_Seq(exe, ",");
      }
    }
    exe = expTy_Seq(exe, ")");
    exe->ty = Ty_String();
    exe->size = strlen(exe->exp)*sizeof(char); 
  } else if (a->u.call.func == S_Symbol("oid")) {

    rc = transFuncOid(venv, tenv, a, sql, dec, exe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transFuncOid");
      goto exit;
    } 
  } else if (a->u.call.func == S_Symbol("XMLElement")){ 
    int n=arg_list->length;
    exe = expTy_Seq(exe, "_XMLElement(");
    //    exe = expTy_Seq(exe, "\"name\",");
    for (int i = 0; i < n; i++){
      A_exp arg = (A_exp)getNthElementList(arg_list, i);
      rc = transExp(venv, tenv, arg, sql, argdec, argexe,aggregates);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
	goto exit;
      }
      exe = expTy_Seq(exe, argexe);
      exe = expTy_Seq(exe, ",");

    }
      exe = expTy_Seq(exe, "\"._end\")");
      exe->ty = Ty_String();
      exe->size = MAX_STR_LEN-1;
  } else if (a->u.call.func == S_Symbol("XMLAttributes")){ 
    int n=arg_list->length;
    exe = expTy_Seq(exe, "_XMLAttributes(");
    for (int i = 0; i < n; i++){
      A_exp arg = (A_exp)getNthElementList(arg_list, i);
      rc = transExp(venv, tenv, arg, sql, argdec, argexe,aggregates);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
	goto exit;
      }
      exe = expTy_Seq(exe, argexe);
      exe = expTy_Seq(exe, ",");

    }
      exe = expTy_Seq(exe, "\"._end\")");
      exe->ty = Ty_String();
      exe->size = MAX_STR_LEN-1;
  }
  else if (a->u.call.func == S_Symbol("newiext") 
           || a->u.call.func == S_Symbol("newrext")
           || a->u.call.func == S_Symbol("newcext")
           || a->u.call.func == S_Symbol("newtext")){ 
    int n=arg_list->length;
    char temp[80];
    sprintf(temp, "_%s(%d, ", S_name(a->u.call.func), n);
    exe = expTy_Seq(exe, temp);
    for (int i = 0; i < n; i++){
      A_exp arg = (A_exp)getNthElementList(arg_list, i);
      rc = transExp(venv, tenv, arg, sql, argdec, argexe,aggregates);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
	goto exit;
      }
      if((a->u.call.func == S_Symbol("newiext") && argexe->ty->kind != Ty_int)
         || (a->u.call.func == S_Symbol("newrext") && argexe->ty->kind != Ty_real)
         || (a->u.call.func == S_Symbol("newcext") && argexe->ty->kind != Ty_string)
         || (a->u.call.func == S_Symbol("newtext") && argexe->ty->kind != Ty_timestamp)) {
        rc = ERR_INCOMPATIBLE_TYPE;
        EM_error(0, rc, __LINE__, __FILE__);
        goto exit;
      }
      exe = expTy_Seq(exe, argexe);
      if(i!=n-1)
        exe = expTy_Seq(exe, ",");
    }
    exe = expTy_Seq(exe, ")");
    if(a->u.call.func == S_Symbol("newiext")) {
      exe->ty = Ty_IExt();
      exe->size = sizeof(struct iExt_);
    }
    else if(a->u.call.func == S_Symbol("newrext")) {
      exe->ty = Ty_RExt();
      exe->size = sizeof(struct rExt_);
    }
    else if(a->u.call.func == S_Symbol("newcext")) {
      exe->ty = Ty_CExt();
      exe->size = sizeof(struct cExt_);
    }
    else if(a->u.call.func == S_Symbol("newtext")) {
      exe->ty = Ty_TExt();
      exe->size = sizeof(struct tExt_);
    }
  }
  else if(a->u.call.func == S_Symbol("deleteiext")
          || a->u.call.func == S_Symbol("deleterext")
          || a->u.call.func == S_Symbol("deletecext")
          || a->u.call.func == S_Symbol("deletetext")) {
    //verify only one param and it is table.field
    if(arg_list == NULL || arg_list->length != 1) {
      rc = ERR_INVALID_DELETE_FUNC_USE;
      EM_error(a->pos, rc, __LINE__, __FILE__);
      goto exit;
    }
    A_exp arg = (A_exp)getNthElementList(arg_list, 0);
    rc = transExp(venv, tenv, arg, sql, argdec, argexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBuiltInFunc", "transExp");
      goto exit;
    }
    if((a->u.call.func == S_Symbol("deleteiext") && argexe->ty->kind != Ty_iext)
       || (a->u.call.func == S_Symbol("deleterext") && argexe->ty->kind != Ty_rext)
       || (a->u.call.func == S_Symbol("deletecext") && argexe->ty->kind != Ty_cext)
       || (a->u.call.func == S_Symbol("deletetext") && argexe->ty->kind != Ty_text))
         {
      rc = ERR_INVALID_DELETE_FUNC_USE;
      EM_error(a->pos, rc, __LINE__, __FILE__);
      goto exit;
    }
    exe = expTy_Seq(exe, "_");
    exe = expTy_Seq(exe, S_name(a->u.call.func));
    exe = expTy_Seq(exe, "(");
    exe = expTy_Seq(exe, argexe);
    exe = expTy_Seq(exe, ")");
    if(a->u.call.func == S_Symbol("deleteiext")) {
      exe->ty = Ty_Int();
      exe->size = 0;
    }
    else if(a->u.call.func == S_Symbol("deleterext")) {
      exe->ty = Ty_Real();
      exe->size = 0;
    }
    else if(a->u.call.func == S_Symbol("deletecext")) {
      exe->ty = Ty_String();
      exe->size = 0;
    }
    else if(a->u.call.func == S_Symbol("deletetext")) {
      exe->ty = Ty_Timestamp();
      exe->size = 0;
    }
  }
  else {
    rc = ERR_NON_BUILTIN_FUNC;
  }
  
    /*  
 BF_LENGTH:
    break;
  case BF_ABS:
    break;
  case BF_CASE:
    break; 
  case BF_LTRIM:
    break;
  case BF_MAX:
    break;
  case BF_MIN:
    break;
  case BF_RTRIM:
  break;
case BF_SUBSTR:
    break;
  case BF_TRIM:
  break;*/

 exit:
  return rc;
}








