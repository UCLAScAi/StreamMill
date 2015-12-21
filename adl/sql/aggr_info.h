#ifndef _AGGR_INFO_H
#define _AGGR_INFO_H

#include <adl_sys.h>
#include <semant.h>
#include <types.h>
#include <env.h>
#include <err.h>
#include <symbol.h>
#include <trans2C.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "list.h"
#include "util.h"


#define AGGR_INFO_LEN 10240

typedef struct aggr_info_ *Aggr_info_t;

struct aggr_info_ {
  A_exp aggr;
  int builtin;
  ty_t builtin_type;
  char argbuf[AGGR_INFO_LEN];
  char expire_argbuf[AGGR_INFO_LEN];
  char extDeallocBuf[AGGR_INFO_LEN];
  char extDeallocBuf_expire[AGGR_INFO_LEN];
  char *winkey_code;
  int winkey_size;
  char *windata_code;
  int windata_size;
  char *expire_code;
};

Aggr_info_t AggrInfo(A_exp aggr);
void DeleteAggrInfo(Aggr_info_t aggr_info);

err_t
transAggrArgs(S_table venv,
              S_table tenv,
              Sql_sem sql,
              Aggr_info_t aggr_info,
              char *win,
              vector<void*> aggregates
              );



#endif
