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
#include "aggr_info.h"

#include <vector>

extern "C" {
#include <dbug/dbug.h>
} 

using namespace std;
err_t genGetCode(T_expty ae, char *source, char *code, int &size) 
{
  err_t rc=ERR_NONE;
  char line[80];

  switch (ae->ty->kind) {
  case Ty_int:
  case Ty_ref:
    sprintf(line, "\nmemcpy(&(%s_expire), (char*)%s+%d, sizeof(int));",
	    ae->exp, source, size);
    strcat(code, line);
    size += sizeof(int);
    break;
  case Ty_real:
    sprintf(line, "\nmemcpy(&(%s_expire), (char*)%s+%d, sizeof(double));",
	    ae->exp,source, size);
    strcat(code, line);
    size += sizeof(double);
    break;
  case Ty_timestamp:
    sprintf(line, "\nmemcpy(&(%s_expire), (char*)%s+%d, sizeof(timeval));",
	    ae->exp,source, size);
    strcat(code, line);
    size += sizeof(timeval);
    break;
  case Ty_string:
    sprintf(line, "\nmemcpy(%s_expire, (char*)%s+%d, %d);",
	    ae->exp, source, size, ae->size);
    strcat(code, line);
    size += ae->size;
    break;
  case Ty_iext:
    sprintf(line, "\nmemcpy(&(%s_expire), (char*)%s+%d, sizeof(struct iExt_));",
	          ae->exp, source, size);
    strcat(code, line);
    size += sizeof(struct iExt_);
    break;
  case Ty_rext:
    sprintf(line, "\nmemcpy(&(%s_expire), (char*)%s+%d, sizeof(struct rExt_));",
	          ae->exp, source, size);
    strcat(code, line);
    size += sizeof(struct rExt_);
    break;
  case Ty_cext:
    sprintf(line, "\nmemcpy(&(%s_expire), (char*)%s+%d, sizeof(struct cExt_));",
	          ae->exp, source, size);
    strcat(code, line);
    size += sizeof(struct cExt_);
    break;
  case Ty_text:
    sprintf(line, "\nmemcpy(&(%s_expire), (char*)%s+%d, sizeof(struct tExt_));",
	          ae->exp, source, size);
    strcat(code, line);
    size += sizeof(struct tExt_);
    break;
  default:
    rc = ERR_DATATYPE;
    EM_error(0, rc, __LINE__, __FILE__, "genGetCode()");
  }
 exit:
  return rc;
}

err_t genExtDeallocCode(T_expty ae, char* code, char* code_expire) {
  err_t rc=ERR_NONE;
  char line[280];

  switch (ae->ty->kind) {
    case Ty_iext:
      sprintf(line, "\n_deleteiext(%s);", ae->exp);
      strcat(code, line);
      sprintf(line, "\n//printf(\"deletin %%d %%d\\n\", %s_expire.length, %s_expire.pt[1]);fflush(stdout);"
                    "\n_deleteiext(%s_expire);", ae->exp, ae->exp, ae->exp);
      strcat(code_expire, line);
      break;
  }

 exit:
  return rc;
}


err_t genComposeCode(T_expty ae, char *target, char *code, int &size) 
{
  err_t rc=ERR_NONE;
  char line[80];

  switch (ae->ty->kind) {
  case Ty_int:
  case Ty_ref:
    sprintf(line, "\nmemcpy(%s+%d, &(%s), sizeof(int));",
	    target, size, ae->exp);
    strcat(code, line);
    size += sizeof(int);
    break;
  case Ty_real:
    sprintf(line, "\nmemcpy(%s+%d, &(%s), sizeof(double));",
	    target, size, ae->exp);
    strcat(code, line);
    size += sizeof(double);
    break;
  case Ty_timestamp:
    sprintf(line, "\nmemcpy(%s+%d, &(%s), sizeof(timeval));",
	    target, size, ae->exp);
    strcat(code, line);
    size += sizeof(timeval);
    break;
  case Ty_iext:
    /* we are trying to just put the pointer */
    sprintf(line, "\nmemcpy(%s+%d, &(%s), sizeof(struct iExt_));",
	    target, size, ae->exp);
    strcat(code, line);
    size += sizeof(struct iExt_);
    break;
  case Ty_rext:
    /* we are trying to just put the pointer */
    sprintf(line, "\nmemcpy(%s+%d, &(%s), sizeof(struct rExt_));",
	    target, size, ae->exp);
    strcat(code, line);
    size += sizeof(struct rExt_);
    break;
  case Ty_cext:
    /* we are trying to just put the pointer */
    sprintf(line, "\nmemcpy(%s+%d, &(%s), sizeof(struct cExt_));",
	    target, size, ae->exp);
    strcat(code, line);
    size += sizeof(struct cExt_);
    break;
  case Ty_text:
    /* we are trying to just put the pointer */
    sprintf(line, "\nmemcpy(%s+%d, &(%s), sizeof(struct tExt_));",
	    target, size, ae->exp);
    strcat(code, line);
    size += sizeof(struct tExt_);
    break;
  case Ty_string:
    sprintf(line, "\nmemcpy(%s+%d, %s, %d);",
	    target, size, ae->exp, ae->size);
    strcat(code, line);
    size += ae->size;
    break;
  default:
    rc = ERR_DATATYPE;
    EM_error(0, rc, __LINE__, __FILE__, "transAggrArgs()");
  }
 exit:
  return rc;
}

err_t 
transAggrArgs(S_table venv, 
	      S_table tenv,
	      Sql_sem sql, 
	      Aggr_info_t aggr_info,
	      char *win_str,
              vector<void*> aggregates
	      )
{
  err_t rc=ERR_NONE;
  int nargs, j;
  char fname[80], line[80];
  A_exp aggr = aggr_info->aggr;
  A_list arglist = aggr->u.call.args;
  A_win win = aggr->u.call.win;
  T_expty dummy, ae;
  int ei=0;

  /* # of args */
  nargs = A_ListEmpty(arglist)? 0: arglist->length;

  /* aggr name */
  strcpy(fname, S_name(aggr->u.call.func));

  /* check # of args */
  E_enventry x = (E_enventry)S_look(venv, aggr->u.call.func);
  if ((x->u.fun.varied_args == 0)  // fixed # of args
      && nargs != x->u.fun.formals->length) {
    rc = ERR_WRONG_NUMBER_OF_ARGS;
    EM_error(aggr->pos, rc, __LINE__, __FILE__, fname);
    goto exit;
  }

  //printf("aggr info3 %d %d\n", aggr_info->builtin, x->u.fun.builtin);

  aggr_info->builtin = x->u.fun.builtin;
  if (aggr_info->builtin == AGGR_BUILTIN_XA)
      aggr_info->builtin_type = Ty_string;

  if (aggr_info->builtin == AGGR_BUILTIN_MINR)
      aggr_info->builtin_type = Ty_real;

  if (aggr_info->builtin == AGGR_BUILTIN_MAXR)
      aggr_info->builtin_type = Ty_real;

  if (aggr_info->builtin == AGGR_BUILTIN_SUMR)
      aggr_info->builtin_type = Ty_real;

  if (aggr_info->builtin == AGGR_BUILTIN_COUNTR)
      aggr_info->builtin_type = Ty_real;

  if (aggr_info->builtin == AGGR_BUILTIN_VAR)
      aggr_info->builtin_type = Ty_real;

  /* generate partition key */
  if (win && !A_ListEmpty(win->partition_list)) {

    aggr_info->winkey_size =0;
    *aggr_info->winkey_code = '\0';

    A_list gb_list = win->partition_list;
    for (j=0; j<gb_list->length; j++) {
      A_exp gb = (A_exp) getNthElementList(gb_list, j);
      rc = transExp(venv, tenv, gb, sql, dummy, ae, aggregates);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrArgs", "transExp");
	goto exit;
      }
      sprintf(line, "%s->keybuf", win_str);
      rc = genComposeCode(ae, line,
			  aggr_info->winkey_code, aggr_info->winkey_size);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrArgs", "genComposeCode");
	goto exit;
      }
    }

  }  

  /* generate argbuf */
  *aggr_info->argbuf       = '\0';
  *aggr_info->expire_argbuf       = '\0';
  *aggr_info->extDeallocBuf       = '\0';
  *aggr_info->extDeallocBuf_expire = '\0';

  if (win != (A_win)0) {
    *aggr_info->windata_code = '\0';
    *aggr_info->winkey_code = '\0';
    *aggr_info->expire_code = '\0';
  }
  aggr_info->windata_size  = 0;

  for (j=0; j<nargs; j++) {
    A_exp arg = (A_exp)getNthElementList(arglist, j);

    rc = transExp(venv, tenv, arg, sql, dummy, ae, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCallAggr", "transExp");
      goto exit;
    }
    /* TODO: check type */
	
    // append into argbuf
    strcat(aggr_info->argbuf, ", ");
    strcat(aggr_info->argbuf, ae->exp);

    //call a func that puts the deallocate code
    rc = genExtDeallocCode(ae, aggr_info->extDeallocBuf, aggr_info->extDeallocBuf_expire);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrArgs", "genExtDeallocCode");
	goto exit;
      }
    
    // append to expire argbuf
    strcat(aggr_info->expire_argbuf, ", ");
    strcat(aggr_info->expire_argbuf, ae->exp);
    strcat(aggr_info->expire_argbuf, "_expire");
    // construct _win_data
    if (win != (A_win)0) {
      sprintf(line, "%s", win_str);
      rc = genComposeCode(ae, line,
			  aggr_info->windata_code, aggr_info->windata_size);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrArgs", "genComposeCode");
	goto exit;
      }
      rc = genGetCode(ae, "windata.data",
			  aggr_info->expire_code, 
			  ei);
      if (rc) {
	EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transAggrArgs", "genComposeCode");
	goto exit;
      }
    }
  } // end for j
  if (win){
    memset(line, 0, 80);
    sprintf(line, "\nwindata.data = %s;"
	    "\nwindata.size = %d;"
	    , win_str, aggr_info->windata_size);
    strcat(aggr_info->windata_code,
	   line);
  }
  

 exit:
  return rc;
}

Aggr_info_t AggrInfo(A_exp aggr)
{
  Aggr_info_t aggr_info = (Aggr_info_t)malloc(sizeof(*aggr_info));

  aggr_info->aggr = aggr;
  aggr_info->builtin_type = Ty_int;

  if (aggr->u.call.win) {
    aggr_info->winkey_code = (char*)malloc(AGGR_INFO_LEN);
    aggr_info->windata_code = (char*)malloc(AGGR_INFO_LEN);
    aggr_info->expire_code = (char*)malloc(AGGR_INFO_LEN);
  } else {
    aggr_info->winkey_code = (char*)0;
    aggr_info->windata_code = (char*)0;
  }
  aggr_info->winkey_size = 0;
  aggr_info->windata_size = 0;
  return aggr_info;
}

void DeleteAggrInfo(Aggr_info_t aggr_info)
{
  if (aggr_info->aggr->u.call.win) {
    free(aggr_info->winkey_code);
    free(aggr_info->windata_code);
    free(aggr_info->expire_code);
  }
  free(aggr_info);
}
