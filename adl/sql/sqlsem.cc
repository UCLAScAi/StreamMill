#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include <sql/symbol.h>
#include <sql/trans2C.h>
#include <sql/io.h>
#include <stdio.h>
#include <string.h>
// #include <dlfcn.h>
#include "list.h"
#include "util.h"

/************************************************************************/
/* Sql_sem: predeclarations during compilation                          */
/************************************************************************/
Sql_sem SqlSem(void)
{
  Sql_sem s = (Sql_sem)ntMalloc(sizeof(*s));

  s->predec = (T_expty)0;
  s->preinit = (T_expty)0;
  s->preclean = (T_expty)0;
  s->afterpreclean = (T_expty)0;
  s->global = (T_expty)0;
//   s->invocations = A_List(0,A_EXP);
  s->in_func = 0;
  s->cur_sqlopr = (A_sqlopr)0;
  s->top_sqlopr = (A_sqlopr)0;

  s->cur_sqlopr_flag = 0;	/* the current sql opr is a SCALAR=1, EXISTS=2, NOTEXISTS=3 */

  s->update_mode = USE_DIRECT;	// assume key is not modified in UPDATE

  return s;
}

void SqlSem_Delete(Sql_sem sql)
{
  if (sql->predec) 
    expTy_Delete(sql->predec);
  if (sql->preinit)
    expTy_Delete(sql->preinit);
  if (sql->preclean)
    expTy_Delete(sql->preclean);
  if(sql->afterpreclean)
    expTy_Delete(sql->afterpreclean);
  if (sql->global)
    expTy_Delete(sql->global);
//   clearList(sql->invocations);
//   deleteList(sql->invocations);
  ntFree(sql);
}

void addSqlSemFirstEntryDec(Sql_sem sql, void *f)
{
  char buf[MAX_STR_LEN];
  int i = UID(f);
  //  printf("%d\n", i);
  //  if (!isFirstEntryDec(i)){ // first time
  //    setFirstEntryFlag(i);
    sprintf(buf, "\nint first_entry_%d = 1;", i);
    //    printf("--%d--\n", i);
    sql->predec = expTy_Seq(sql->predec, buf);
    //  }

}

void addSqlSemIndexDec(Sql_sem sql, void *f)
{
  char buf[MAX_STR_LEN];

  sprintf(buf, "\nint index_%d = 0;", UID(f));
  sql->predec = expTy_Seq(sql->predec, buf);
}

err_t 
addSqlSemCursorDec(Sql_sem sql, S_table venv, A_qun qun, char* tabLoc)
{
  err_t rc = ERR_NONE;
  char name[80];
  char cursorname[80];
  char buf[MAX_STR_LEN];
  E_enventry x;

  if(qun->kind == QUN_WINDOW) {
    sprintf(name, "%s_%d_winbuf->get_im_rel()", S_name(qun->u.window.name), 
	    UID(qun->ppnode));
    sprintf(cursorname, "%s_%d_winbufc", S_name(qun->alias), UID(qun->ppnode));
    
    rc = transCursorDec2C(cursorname, buf, TAB_MEMORY, INDEX_BTREE);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, 
	       "addSqlSemCursorDec", "transCursorDec2C");
      goto exit;
    }
    
    sql->predec = expTy_Seq(sql->predec, buf);
    
    rc = transCursorInit2C(name, cursorname, buf, TAB_MEMORY, INDEX_BTREE);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, 
	       "addSqlSemCursorDec", "transCursorInit2C");
      goto exit;
    }
    sql->preinit = expTy_Seq(sql->preinit, buf);
  }
  else {
    int dynamic = 0;
    S_symbol rawname;

    x = (E_enventry)S_look(venv, qun->u.table.name);
    if(x && x->kind == E_dynamicEntry) {
      rawname = x->u.dynamic.rawname;
      dynamic = 1;
      x = (E_enventry)S_look(venv, x->u.dynamic.table);
    }

    if(tabLoc != NULL) {
      sprintf(name, "%s", tabLoc);
    }
    else {
      if (x && x->kind == E_varEntry && x->u.var.iname) {
        sprintf(name, "%s", S_name(x->u.var.iname));
      }
      else {
        if(dynamic == 1) {
          sprintf(buf, "\nchar tabName_%d[200];", UID(qun));
          sql->predec = expTy_Seq(sql->predec, buf);
          sprintf(buf, "\nsprintf(tabName_%d, \"%%s%s\", _modelId);",
                       UID(qun), S_name(rawname));
          sql->preinit = expTy_Seq(sql->preinit, buf);
          sprintf(name, "((IM_REL*)inMemTables->operator[](tabName_%d))", 
                        UID(qun));
        }
        else
          sprintf(name, "%s", S_name(qun->u.table.name));
      }
    }
    
    sprintf(cursorname, "%s_%d", S_name(qun->alias), UID(qun->ppnode));

    tabindex_t index = (tabindex_t)1;
    if(x->u.var.index != (A_index)0) 
      index = x->u.var.index->kind;
    rc = transCursorDec2C(cursorname, buf, x->u.var.scope, index);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, 
	       "addSqlSemCursorDec", "transCursorDec2C");
      goto exit;
    }
    
    sql->predec = expTy_Seq(sql->predec, buf);
    
    
    // Checks to make sure that the table we are using is
    // initialized in C, if not then initialize it.
    //this is needed for ESL, adHoc queries and aggr def in ESL

    if((isESL()||isAdHoc()||isESLAggr())) { 
      char dbname[80];
      if(x && x->kind == E_varEntry && x->u.var.source) {
	sprintf(dbname, "\"./%s\"", S_name(x->u.var.source));
	rc = transTabInit2C(name, dbname, x->u.var.haskey, buf, 
			    x->u.var.scope, index, Ty_nil, 
	                    x->u.var.inaggr, x->u.var.isBuffer, S_Symbol(name));
	if(rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, 
		   "addSqlSemCursorDec", "transTabInit2C");
	  goto exit;
	}
	/*sql->preinit = expTy_Seq(sql->preinit, "\nif(");
	  sql->preinit = expTy_Seq(sql->preinit, name);
	  sql->preinit = expTy_Seq(sql->preinit, "==NULL) {");*/
	sql->preinit = expTy_Seq(sql->preinit, buf);
	//sql->preinit = expTy_Seq(sql->preinit, "\nprintf(\"got here\\n\");\nfflush(stdout);");
	//sql->preinit = expTy_Seq(sql->preinit, "\n}");
      }
      else if(x && x->kind == E_varEntry && !x->u.var.inaggr
	      && x->u.var.scope == TAB_MEMORY && dynamic == 0) {
	char* name1;
	if (x->u.var.iname) 
	  name1 = S_name(x->u.var.iname);
	else
	  name1 = S_name(qun->u.table.name);
	    
	sprintf(dbname, "\"_adl_db_%s\"", name1);
	
	rc = transTabInit2C(S_name(qun->u.table.name), dbname, x->u.var.haskey, 
			    buf, x->u.var.scope, index, Ty_nil, 
			    x->u.var.inaggr, x->u.var.isBuffer, S_Symbol(name1));
	if(rc) {
	  EM_error(0, ERR_HISTORY, __LINE__, __FILE__, 
		   "transSqlStatement", "transTabInit2C");
	  goto exit;
	}
	sql->preinit = expTy_Seq(sql->preinit, buf);
      }
    }
    rc = transCursorInit2C(name, cursorname, buf, x->u.var.scope, 
			   index, x->u.var.isBuffer);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, 
	       "addSqlSemCursorDec", "transCursorInit2C");
      goto exit;
    }
    sql->preinit = expTy_Seq(sql->preinit, buf);
  }
  
  rc = transCursorClose2C(cursorname, buf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, 
	     "addSqlSemCursorDec", "transCursorClose2C");
      goto exit;
  }
  sql->preclean = expTy_Seq(sql->preclean, buf);

  /*Not (isESL()||isAdHoc()), because for adhoc we may have multiple instances
  of the same table thus, we don't wnt to delete the table before all cursors are 
  deleted - YYY right now tables are not deleted at all becuase there isn't anyway 
  to tell, if this is the last cursor for the table or not. Having said that
  it could potentially also be a problem in ESL*/
  if(qun->kind != QUN_WINDOW && (isESL()||isAdHoc()||isESLAggr()) 
     && x && x->kind == E_varEntry && x->u.var.source) {
    tabindex_t index = (tabindex_t)1;
    if(x->u.var.index != (A_index)0) 
      index = x->u.var.index->kind;
    //sql->preclean = expTy_Seq(sql->preclean, "\n//printf(\"got here clean\\n\");\nfflush(stdout);");
    transTabClose2C(name, name, buf, x->u.var.scope, index);
    sql->preclean = expTy_Seq(sql->preclean, buf);
  }
  
  exit:
  return rc;
}


void mergeSqlSem(Sql_sem sql1, Sql_sem sql2)
{
  sql1->predec = expTy_Seq(sql1->predec, sql2->predec);
  sql1->preinit = expTy_Seq(sql1->preinit, sql2->preinit);
  sql1->preclean = expTy_Seq(sql1->preclean, sql2->preclean);

  sql2->predec = (T_expty)0;
  sql2->preinit = (T_expty)0;
  sql2->preclean = (T_expty)0;

  SqlSem_Delete(sql2);
}















