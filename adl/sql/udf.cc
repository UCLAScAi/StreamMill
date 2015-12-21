#include <sql/adl_sys.h>
#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include <sql/symbol.h>
#include <sql/trans2C.h>
#include <sql/io.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "util.h"

#include <string>
#include <vector>
using namespace std;

extern system_t *ntsys;

err_t transExtUdfQun(S_table venv, 
		     S_table tenv, 
		     A_qun q, 
		     E_enventry x, // env entry of UDF
		     T_expty argbuf,
		     Sql_sem sql, 
		     T_expty &dec, // OUT
		     T_expty &exe // OUT
		     )
{
  err_t rc = ERR_NONE;
  char buf[MAX_STR_LEN];
  E_enventry  te;
  char qunname[80];

  addSqlSemFirstEntryDec(sql, (void*)q);

  // put the qun into symbol table
  te = E_VarEntry(q->alias, x->u.ext.result, 0); 
  getQunName(q, qunname);
  te->u.var.sname = new_Symbol(qunname);
  S_enter(venv, q->alias, te);

  /* first check if builtin table function vert ?? */
  if(x->key == S_Symbol("vert")) {
    // invoke the table function
    sprintf(buf, 
   	    "\nrc = _verticalize(first_entry_%d, (void*)(&%s), ", 
	    UID(q), qunname);
    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, argbuf);
    exe = expTy_Seq(exe, ");");
  }
  else if(x->key == S_Symbol("buildiext")) {
    // invoke the table function
    if(argbuf->ty != Ty_Int() && argbuf->ty != Ty_Real()) {
        rc = ERR_INCOMPATIBLE_TYPE;
        EM_error(0, rc, __LINE__, __FILE__);
        goto exit;
    }
    sprintf(buf,
            "\nrc = _buildIExt(first_entry_%d, (void*)(&%s), ",
            UID(q), qunname);
    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, argbuf);
    exe = expTy_Seq(exe, ");");
  }
  else if(x->key == S_Symbol("iextvert")) {
    // invoke the table function
    if(argbuf->ty != Ty_IExt()) {
        rc = ERR_INCOMPATIBLE_TYPE;
        EM_error(0, rc, __LINE__, __FILE__);
        goto exit;
    }
    sprintf(buf, 
   	    "\nrc = _iExtVert(first_entry_%d, (void*)(&%s), ", 
	    UID(q), qunname);
    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, argbuf);
    exe = expTy_Seq(exe, ");");
  }
  else if(x->key == S_Symbol("rextvert")) {
    // invoke the table function
    if(argbuf->ty != Ty_RExt()) {
        rc = ERR_INCOMPATIBLE_TYPE;
        EM_error(0, rc, __LINE__, __FILE__);
        goto exit;
    }
    sprintf(buf, 
   	    "\nrc = _rExtVert(first_entry_%d, (void*)(&%s), ", 
	    UID(q), qunname);
    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, argbuf);
    exe = expTy_Seq(exe, ");");
  }
  else if(x->key == S_Symbol("cextvert")) {
    // invoke the table function
    if(argbuf->ty != Ty_CExt()) {
        rc = ERR_INCOMPATIBLE_TYPE;
        EM_error(0, rc, __LINE__, __FILE__);
        goto exit;
    }
    sprintf(buf, 
   	    "\nrc = _cExtVert(first_entry_%d, (void*)(&%s), ", 
	    UID(q), qunname);
    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, argbuf);
    exe = expTy_Seq(exe, ");");
  }
  else if(x->key == S_Symbol("textvert")) {
    // invoke the table function
    if(argbuf->ty != Ty_TExt()) {
        rc = ERR_INCOMPATIBLE_TYPE;
        EM_error(0, rc, __LINE__, __FILE__);
        goto exit;
    }
    sprintf(buf, 
   	    "\nrc = _tExtVert(first_entry_%d, (void*)(&%s), ", 
	    UID(q), qunname);
    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, argbuf);
    exe = expTy_Seq(exe, ");");
  }
  else {
    // invoke the table function
    sprintf(buf, 
   	    "\nrc = (*_adl_func_%d)(first_entry_%d, (void*)(&%s), ", 
	    UID(x), UID(q), qunname);
    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, argbuf);
    exe = expTy_Seq(exe, ");");
  }

  sprintf(buf, "\nif (rc==0) {"
	  "\nfirst_entry_%d = 0;"
	  ,UID(q));
  exe = expTy_Seq(exe, buf);

	  // reset cursor
  sprintf(buf, "\n} else if (rc == DB_NOTFOUND) {"
	  "\nfirst_entry_%d = 1;"
	  , UID(q));
  exe = expTy_Seq(exe, buf);

  if(isESL()) {
    sprintf(buf, "\n} else {"
	    "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: External Table Function\");"
	    "\nreturn s_failure;"
	    "\n}",
	    getUserName(), getQueryName());
  }
  else if(isESLAggr()) {
    sprintf(buf, "\n} else {"
	    "adlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: External Table Function\");"
	    "\nreturn;"
	    "\n}",
	    getUserName(), getAggrName());
  }
  else {
    sprintf(buf, "\n} else {"
	    "\nadlabort(rc, \"External Table Function\");"
	    "\n}");
  }
  exe = expTy_Seq(exe, buf);

  exe->ty = x->u.ext.result;
  if (exe->ty->kind == Ty_string)
    exe->size = x->u.ext.size;


exit:
  return rc;
}

/*
 * Implementation of the UDF table function.
 *
 * local data:
 *          struct udf_local_data {
 *                 [ LOCAL tables ]
 *                 DB *return;
 *          };
 * udf:
 *          udf(struct udf_local_data *p, arg0, arg1, ..., argN) 
 *          {
 *                 [ init tables in p ]
 *                 // recursion?
 *                 struct udf_local_data *q;
 *                 udf(q, ...);
 *                 ...
 *          }
 *          Note: Local tables and the return table are initialized at the begining of the udf. 
 *          Note: Local tables are cleaned up at the end of the udf.
 *          Note: Return table are cleaned up by the caller.
 * caller: 
 *          struct udf_local_data *p;
 *          udf(p, arg0, arg1, ..., argN);
 *          do {
 *                rc = p->return->get_tuple();
 *          } while (rc==0);
 *
 * Note: If the table function is one of several QUNs under a node, it
 *          is called everytime it is accessed (first_entry=1). In the
 *          future, we want to optimize it so that the content of the
 *          return table is retained as long as the args are the same
 *          from the previous calling to save the execution.
 * */

err_t transUdfQun(S_table venv, 
		  S_table tenv, 
		  A_qun q, 
		  E_enventry x, // env entry of UDF
		  T_expty udf_args,
		  Sql_sem sql, 
		  T_expty &dec,	// OUT
		  T_expty &exe)	// OUT
{
  err_t rc = ERR_NONE;
  char buf[40960];
  char first_entry_name[80];
  char cursor_name[80];
  char qunname[80];
  char tmp[8000];
  char handlename[80];
  char *udf_name;
  E_enventry te;

  udf_name = S_name(q->u.function.name);

  addSqlSemFirstEntryDec(sql, (void*)q);

  // put the qun into symbol table
  te = E_VarEntry(q->alias, x->u.ext.result, 0); 
  getQunName(q, qunname);
  te->u.var.sname = new_Symbol(qunname);
  S_enter(venv, q->alias, te);

  /* delcare new status */
  sprintf(handlename, "%s_udf_%d", udf_name, UID(q));
  sprintf(buf, 
	  "\nstruct %s_status *%s;"
	  , udf_name, handlename);
  sql->predec = expTy_Seq(sql->predec, buf);

//    sprintf(first_entry_name, "%s_udf_%d->retc_first_entry", 
//  	  udf_name, UID(q));
  sprintf(first_entry_name, "first_entry_%d", UID(q));
  sprintf(cursor_name, "%s->retc", handlename);

  /* call udf */
  if (udf_args) {
    sprintf(tmp, ", %s", udf_args->exp);
  } else {
    *tmp = '\0';
  }
  sprintf(buf, 
	  "\nif (%s == 1) {"
	  "\n%s = (struct %s_status *)malloc(sizeof(struct %s_status));"
	  "\n%s_udf(%s%s);"
	  "\n}"
	  , first_entry_name
	  , handlename, udf_name, udf_name
	  , udf_name, handlename, tmp);
  exe = expTy_Seq(exe, buf);
  
  /* get tuple from "return" table of the UDF */
  rc = transCursorGet2C(venv,
			first_entry_name, 
			cursor_name, 
			(E_enventry)0,
			(Tr_exp*)0, // binding
			(Tr_exp*)0, // binding upper bound
			(Tr_exp*)0, // binding lower bound
			BOUND_NONE, // no binding
			buf);

  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transUdfQun", "transCursorGet2C");
    goto exit;
  }
  exe = expTy_Seq(exe, buf);

  sprintf(buf, "\nif (rc==0) {"
	  "\nfirst_entry_%d = 0;"
	  ,UID(q));
  exe = expTy_Seq(exe, buf);

  // make assignments 
  getQunName(q, qunname);
  rc = assignKeyData2C(x->u.fun.result, qunname, buf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transUdfQun", "assignKeyData2C");
    goto exit;
  }

  exe = expTy_Seq(exe, buf);

  // reset cursor
  sprintf(buf, "\n} else if (rc == DB_NOTFOUND) {"
	  "\n%s = 1;",
	  first_entry_name);
  exe = expTy_Seq(exe, buf);
  
  
  // close "return" table
  sprintf(tmp, "%s->ret", handlename);
  rc = transTabClose2C(tmp, 
		       (char*)0,
		       buf, 
		       TAB_MEMORY);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transUdfQun", "transTabClose2C");
    goto exit;
  }
  exe = expTy_Seq(exe, buf);

  // dealloc status structure
  sprintf(buf, 
	  "\nfree(%s);"
	  "\nrc = DB_NOTFOUND; /* reset rc which has been over written */"
	  , handlename);
  exe = expTy_Seq(exe, buf);

  //sprintf(buf, "\n} else adlabort(rc, \"DBC->c_get() or IM_RELC->c_get()\");"
  //	  , first_entry_name);
  if(isESL()) {
    sprintf(buf, "\n} else {"
	    "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DBC->c_get() or IM_RELC->c_get()\");"
	    "\nreturn s_failure;"
	    "\n}",
	    getUserName(), getQueryName());
  }
  else if(isESLAggr()) {
    sprintf(buf, "\n} else {"
	    "adlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DBC->c_get() or IM_RELC->c_get()\");"
	    "\nreturn;"
	    "\n}",
	    getUserName(), getAggrName());
  }
  else {
    sprintf(buf, "\n} else {"
	    "\nadlabort(rc, \"DBC->c_get() or IM_RELC->c_get()\");"
	    "\n}");
  }
  exe = expTy_Seq(exe, buf);
  exe->ty = x->u.fun.result;

 exit:
  return rc;
}


err_t 
transUDFDec(S_table venv, 
	    S_table tenv, 
	    A_dec d, 
	    T_expty &edec, 
	    T_expty &einit, 
	    T_expty &eclean,
            vector<void*> aggregates
	    )
{
  err_t rc = ERR_NONE;
  Ty_ty resultTy;
  char buf[MAX_STR_LEN];
  char argdef[MAX_STR_LEN];
  char internalname[128];
  char handlename[80];
  T_expty curdec, curexe;
  A_list formalTys = A_List();
  A_list declist = (A_list)0;
  T_expty lc_dec, lc_init, lc_clean;
  E_enventry te, x;
  int i;

  Sql_sem sql = SqlSem();
  sql->in_func = 1;		// inside UDF
  sql->func_name = strdup(S_name(d->name));

  /* return type of UDF (it is a record type) */
  rc = transTy(venv, tenv, d->u.fun.result, resultTy, (A_list)0, (int*)0, A_tabVarDec);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transUDFDec", "transTy");
    goto exit;
  }

  /* since resultTy is the type of the return table, we need to give
     it a key (duplicates allowed) */
  if (resultTy->u.record->length>0) {
    Ty_field f = (Ty_field)getNthElementList(resultTy->u.record, 0);
    f->iskey = 1;
  }

  /* arguments of UDF */
  *argdef = '\0';
  if (d->u.fun.params) {
    rc = transFields(venv, tenv, d->u.fun.params, formalTys, (A_list)0, (int*)0, A_functionDec); 
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transUDFDec", "transFields");
      goto exit;
    }
    /* c-type arguments list */
    rc = TyList2C(formalTys, argdef);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transUDFDec", "TyList2C");
      goto exit;
    }
  }

  /* add UDF into symbol table now (so that it can be called
     recursively inside the UDF.) */
  S_enter(venv, d->name, E_FunEntry(d->name, formalTys, resultTy, 0/*vairied_args*/));
  
  /* 
   * Add UDF arguments into symbol table.

   * UDF arguments are visible to local variables (during initialization).
   */
  S_beginScope(venv);
  S_beginScope(tenv);

  /* add formal args into symbol table */
  for (i=0; i<formalTys->length; i++) {
    Ty_field f = (Ty_field) getNthElementList(formalTys, i);
    S_enter(venv, f->name, E_VarEntry(f->name, f->ty, f->size));
  }

  /* local variables of this UDF */
  if (!A_ListEmpty(d->u.fun.decs)) {
    declist = d->u.fun.decs;
    T_expty adec = (T_expty)0;

    rc = transSeqDec(venv, tenv, declist, 
		     lc_dec, lc_init, lc_clean, 
		     1, adec, aggregates			    /* in aggr, aggr dec */
		     );
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transUDFDec", "transSeqDec");
      goto exit;
    }
  }

    /* "return" is the table to hold return values of the UDF */ 
  te = E_VarEntry(S_Symbol("return"), resultTy, 0);
  te->u.var.inaggr = 1;

  strcpy(internalname, "status->ret");
  te->u.var.iname = new_Symbol(internalname);

  /* Now that we are using IMDB for hash tables, we need to define
   * this flag in the symbol table
   */
  te->u.var.scope = TAB_MEMORY;
  S_enter(venv, S_Symbol("return"), te);

  /* declare the status structure */
  sprintf(buf, "\nstruct %s_status {", S_name(d->name));
  if (declist) strcat (buf, lc_dec->exp);
  strcat (buf, 
	  "\nIM_REL *ret;"
	  "\nIM_RELC *retc;"
	  "\nint retc_first_entry;"
	  "\n};"
	  );
  edec = expTy_Seq(edec, buf);

  /* Compile the body of the UDF */
  rc = transExp(venv, tenv, d->u.fun.body, sql, curdec, curexe, aggregates);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transUDFDec", "transExp");
    goto exit;
  }

  /* The UDF */
  if (*argdef) {
    sprintf(buf, 
	    "\nvoid %s_udf(struct %s_status *status, %s)"
	    , S_name(d->name), S_name(d->name), argdef);
  } else {
    sprintf(buf, 
	    "\nvoid %s_udf(struct %s_status *status)"
	    , S_name(d->name), S_name(d->name));
	    
  }
  edec = expTy_Seq(edec, buf);
  sprintf(buf, 
	  "\n{"
	  "\nint rc;"
	  "\nint _adl_sqlcode, _adl_cursqlcode;"
	  "\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
	  "\nint _rec_id=0; /* recursive id */"
 	  "\nDBT key, data, windata;"
	  "\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];"
	  "\nchar _adl_dbname[80];"
          "\nint slide_out = 1;"
	  , S_name(d->name), S_name(d->name), argdef);
  edec = expTy_Seq(edec, buf);

  if (ntsys->verbose) {
    sprintf(buf, "\nadltrace(\"%s_udf\", (int)status, 0);"
	    , S_name(d->name));
    edec = expTy_Seq(edec, buf);
  }

  /* open all local tables */
  for (i=0; declist && i<declist->length; i++) {
    A_dec lv = (A_dec)getNthElementList(declist, i);

    sprintf(handlename, "status->%s", S_name(lv->name));

    /* the dbname is encoded by status */
    sprintf(buf, "\nsprintf(_adl_dbname, \"._%%d_%s\", status);",
	    S_name(lv->name));
    edec = expTy_Seq(edec, buf);


    tabindex_t index = (tabindex_t)1;
    if(lv->u.tabvar.index != (A_index)0) 
      index = lv->u.tabvar.index->kind;
    transTabInit2C(handlename, 
		   "_adl_dbname", 
		   (lv->u.tabvar.index == (A_index)0)? 0:1,
		   buf,
		   lv->u.tabvar.scope,
		   index,
		   Ty_nil,
		   1,
                   lv->u.tabvar.isView,
                   lv->name); 
                    /* we don't need to set BTREE comparison function because
			       UDF is not BTREE table
			    */
    edec = expTy_Seq(edec, buf);
  }

  /* open "return" table */
  sprintf(buf, "\nsprintf(_adl_dbname, \"._%%d_ret\", status);");
  edec = expTy_Seq(edec, buf);

  /* call newly defined function for imdb */
  transTabInit2C_imdb("status->ret", 
		      "_adl_dbname", 
		      TAB_LOCAL, 
		      0, /* no key defined for "return" table, duplicates
			    allowed */
		      buf);
  edec = expTy_Seq(edec, buf);

  /* open cursor of "return" table */
  /* since we are dealing explicitly with return table
     * we can pass in the "TAB_MEMORY" flag here
     */
  transCursorInit2C("status->ret", "status->retc", buf, TAB_MEMORY);

  edec = expTy_Seq(edec, buf);

  if (curdec) edec = expTy_Seq(edec, curdec->exp);
  if (!A_ListEmpty(d->u.fun.decs)) {
    edec = expTy_Seq(edec, lc_init);
  }

  if (curexe) edec = expTy_Seq(edec, curexe->exp);

  edec = expTy_Seq(edec, "\nstatus->retc_first_entry=1;");

  if (ntsys->verbose) {
    sprintf(buf, "\nadltrace(\"%s_udf\", (int)status, 1);"
	    , S_name(d->name));
    edec = expTy_Seq(edec, buf);
  }

  /* close all local tables */
  for (i=0; declist && i<declist->length; i++) {
    A_dec lv = (A_dec)getNthElementList(declist, i);

    sprintf(handlename, "status->%s", S_name(lv->name));

    /* the dbname is encoded by status */
    sprintf(buf, "\nsprintf(_adl_dbname, \"._%%d_%s\", status);",
	    S_name(lv->name));
    edec = expTy_Seq(edec, buf);

    /* call S_look to get the scope (local/imdb/bdb) */
    x = (E_enventry)S_look(venv, lv->name);

    transTabClose2C(handlename, "_adl_dbname", buf, x->u.var.scope);
    edec = expTy_Seq(edec, buf);
  }

  edec = expTy_Seq(edec, "\nstatus->retc_first_entry=1;");

  if (ntsys->verbose) {
    sprintf(buf, "\nadltrace(\"%s_terminate\", (int)status, 1);"
	    , S_name(d->name));
    edec = expTy_Seq(edec, buf);
  }

  edec = expTy_Seq(edec, "\n}");

  /* delete aggr local variables from the symbol table */
  S_endScope(tenv);
  S_endScope(venv);

  SqlSem_Delete(sql);

 exit:
  return rc;
}

/* table function */
err_t transFunQun(S_table venv, 
		  S_table tenv, 
		  A_qun q, 
		  Sql_sem sql, 
		  T_expty &dec,	// OUT
		  T_expty &exe,	// OUT
                  vector<void*> aggregates
		  )
{
  err_t rc = ERR_NONE;
  E_enventry x;
  int n_args = 0;
  A_list a_arglist;
  T_expty argbuf;
  char field[1024];

  argbuf = (T_expty)0;

  // check if the function is defined
  x = (E_enventry) S_look(venv, q->u.function.name);
  if (!x) {
    rc = ERR_UNDEFINED_FUNCTION;
    EM_error (q->pos, rc, __LINE__, __FILE__, S_name(q->u.function.name));
    goto exit;
  }

  // aggregates are not allowed here
  if (x->kind == E_aggrEntry) {
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "aggregate used as QUN");
    goto exit;
  }

  if (q->u.function.args) {
    a_arglist = q->u.function.args;
    n_args = a_arglist->length;

    /* instead of compile args, if it is "vert" table fun then
       we need to compile each arg separatly, convert it to 8 bytes,
       and then put number of args and the 8-byte args
       together in the argbuf 
    */
    if(q->u.function.name == S_Symbol("vert")) {
      /* here take each argument, call transExp, 
         then check its type, convert to 8 bytes
         then the variable that contains it, put it in the argbuf 
         the first thing in the argbuf should be number of arguments
      */
      sprintf(field, "%d, ", a_arglist->length);
      argbuf = expTy_Seq(argbuf, field);

      for (int i=0; i<a_arglist->length; i++) {
	A_exp a_exp = (A_exp)getNthElementList(a_arglist, i);
	T_expty ad, ae;
	
	rc = transExp(venv, tenv, a_exp, sql, ad, ae, aggregates);
	if (rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transFunQun", "transExp");
	  goto exit;
	}
	if (i>0) {
	  argbuf = expTy_Seq(argbuf, ", ");
	}
        if(i == 0) { 
	  argbuf = expTy_Seq(argbuf, ae);	  
        }
        else {
	  switch(ae->ty->kind) {
	  case Ty_string:
	    sprintf(field, "\nchar* field_%d = %s;"
		    "\ndouble field8_%d;"
		    "\nmemcpy((void*)&field8_%d, field_%d, sizeof(double));",
		    i, ae->exp,
		    i, 
		    i, i,
		    i);
	    exe = expTy_Seq(exe, field);
	    
	    //use in the switch
	    sprintf(field, "field8_%d", i);
	    argbuf = expTy_Seq(argbuf, field);
	    break;
	  case Ty_int:
	    /*sprintf(field, "\nint field_%d = %s;"
		    "\ndouble field8_%d;"
		    "\nmemcpy((void*)&field8_%d, (void*)&field_%d, sizeof(int));",
		    i, ae->exp,
		    i, 
		    i, i);*/
	    sprintf(field, "\ndouble field8_%d = %s;",
		    i, ae->exp);

	    exe = expTy_Seq(exe, field);
	    //use in the switch
	    sprintf(field, "field8_%d", i);
	    argbuf = expTy_Seq(argbuf, field);
	    break;
	  case Ty_real:
	    argbuf = expTy_Seq(argbuf, ae);
	    break;
	  default:
	    rc = ERR_DATATYPE;
	    EM_error(0, rc, __LINE__, __FILE__, "vert");
	    goto exit;
	  }
	}
      }
    }
    else {
      rc = compileArgs(venv, tenv, sql, 
		       x->u.ext.formals, 
		       a_arglist,
		       argbuf, aggregates);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transFunQun", "compileArgs");
	goto exit;
      }
    }
  }
	
  switch (x->kind) {
  case E_funEntry:
    {
      rc = transUdfQun(venv, tenv, q, x, argbuf, sql, dec, exe);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transFunQun", "transUdfQun");
	goto exit;
      }
    }
    break;
  case E_extEntry:
    {
      if(q->u.function.name == S_Symbol("fetchtbl")) {
        char buf[MAX_STR_LEN], qunname[80];
        char first_entry_name[MAX_STR_LEN], cursor_name[MAX_STR_LEN];
        E_enventry te;

        //get the second argument
        //it should be name of a defined table
        if(a_arglist->length != 2) {
          rc = ERR_INVALID_TABLE_FUNCTION_USE;
          EM_error(q->pos, rc, __LINE__, __FILE__, "- fetchtbl table can be onlyinvoked inside an aggregate and must be called with 2 args");
          goto exit;
        } 
	A_exp a_exp0 = (A_exp)getNthElementList(a_arglist, 0);
	A_exp a_exp1 = (A_exp)getNthElementList(a_arglist, 1);
        //need the following error checking here
        //first arg should be a varchar(20) and second a constant string
        //also that we should be in an aggregate
        if(a_exp0->kind != A_varExp || a_exp0->u.var->kind != A_simpleVar
           || a_exp1->kind != A_stringExp) {
          rc = ERR_INVALID_TABLE_FUNCTION_USE;
          EM_error(q->pos, rc, __LINE__, __FILE__, "- the first argument to fetchtbl should be a variable and the second argument should be a string literal");
          goto exit;
        }

        //prepend this table name with the user name
        char tableName[200];
        if(strcmp(getUserName(), "__user__") == 0) 
          sprintf(tableName, "%s", a_exp1->u.string);
        else 
          sprintf(tableName, "%s__%s", getUserName(), a_exp1->u.string);
        E_enventry x = (E_enventry)S_look(venv, S_Symbol(tableName));
        //printf("ExtEntry table %s %d\n", tableName, x);
        if (!x || x->kind != E_varEntry) {
          rc = ERR_UNDEFINED_VARIABLE;
          EM_error(q->pos, rc, __LINE__, __FILE__, S_name(q->u.table.name));
          goto exit;
        }
        A_qun qun = A_NameQun(q->pos, S_Symbol(tableName), q->alias);
        char tabLoc[80];
        sprintf(tabLoc, "((IM_REL*)inMemTables->operator[](\"%s\"))", 
                                       S_name(a_exp0->u.var->u.simple));
        rc = addSqlSemCursorDec(sql, venv, qun, tabLoc);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transFunQun", "addSqlSemCursorDec");
          goto exit;
        }
        // leave this one because it is for a table
        addSqlSemFirstEntryDec(sql, (void*)qun);

        getQunName(q, qunname);
        te = E_VarEntry(q->alias, x->u.var.ty, x->u.var.size);
        EnvCpy(te, x); // Richard 2003/5
        te->u.var.sname = new_Symbol(qunname);
        S_enter(venv, q->alias, te);

        // get cursor
        sprintf(first_entry_name, "first_entry_%d", UID(qun));
        sprintf(cursor_name, "%s_%d", S_name(q->alias), UID(qun->ppnode));
        rc = transCursorGet2C(venv,
                              first_entry_name,
                              cursor_name,
                              x, //->u.var.ty,
                              (Tr_exp*)0,
                              (Tr_exp*)0,
                              (Tr_exp*)0,
                              BOUND_NONE,
                              buf);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transFunQun", "transCursorGet2C");
          goto exit;
        }
        exe = expTy_Seq(exe, buf);
        sprintf(buf, "\nif (rc==0) {"
              "\nfirst_entry_%d = 0;"
              ,UID(qun));

        exe = expTy_Seq(exe, buf);

        // make assignments
        //leave it as q here, cause that is the actual qun
        getQunName(q, qunname);
        tabindex_t index = (tabindex_t)1;
        if(x->u.var.index != (A_index)0) {
          index = x->u.var.index->kind;
        }
        rc = assignKeyData2C(x->u.var.ty, qunname, buf, index);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transFunQun", "assignKeyData2C");
          goto exit;
        }

        exe = expTy_Seq(exe, buf);
 
        // reset cursor
        sprintf(buf, "\n} else if (rc == DB_NOTFOUND) {"
                "\n%s = 1;"
                , first_entry_name);
        exe = expTy_Seq(exe, buf);
        if(isESL()) {
          sprintf(buf, "\n} else {"
                  "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DBC->c_get() or IM_RELC->c_get()\");"
                  "\nreturn s_failure;"
                  "\n}",
                  getUserName(), getQueryName());
        }
        else if(isESLAggr()) {
          sprintf(buf, "\n} else {"
                  "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DBC->c_get() or IM_RELC->c_get()\");"
                  "\nreturn;"
                  "\n}",
                  getUserName(), getAggrName());
        }
        else {
          sprintf(buf, "\n} else adlabort(rc, \"DBC->c_get() or IM_RELC->c_get()\");");
        }
        exe = expTy_Seq(exe, buf);
        exe->ty = x->u.var.ty;
      }
      else {
        rc = transExtUdfQun(venv, tenv, q, x, argbuf, sql, dec, exe);
        if (rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transFunQun", "transExtUdfQun");
	  goto exit;
        }
      }
    }
    break;
  }
 exit:
  return rc;
}

