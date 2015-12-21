#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <sql/adl_obj.h>
#include "io.h"
#include "list.h"
#include "err.h"
#include <sql/adl_sys.h>
#include <dbug/dbug.h>
#include <sql/env.h>

static system_t the_sys;

system_t *ntsys = &the_sys;	/*(system_t*)0;*/

jmp_buf env;

static char* sys_tables_name[3] = {
  ".nt_tables",			/* TABLE_DEF */
  ".nt_udfs",			/* UDF_DEF */
  ".nt_udf_aggrs"		/* UDF_AGGR_DEF */
};
/************************************************************
                SINGLA HANDLING
************************************************************/
static void signal_handler(int sig) 
{
  switch (sig) {
  case SIGINT:
    longjmp(env, sig);
    break;			/* break never reached */
//   case SIGALRM:			
//     longjmp(env, sig);
//     break;			/* break never reached */
  default:
    exit(sig);
  }
}
void display(nt_obj_t *obj)
{
  displayObj(ntsys->err, obj);
}
/************************************************************
                SYSTEM STARTUP/SHUTDOWN
************************************************************/
void ntSysInit(char *pdir, char *pname)
{
  int i;
  ntsys->permanent = 1;

  /*  ntsys->in  = newNumObj(0);
  ntsys->out = newNumObj(1);
  ntsys->err = newNumObj(2);
  */
  ntsys->in  = newNumObj((int)stdin);
  ntsys->out = newNumObj((int)stdout);
  ntsys->err = newNumObj((int)stderr);

  ntsys->verbose = 0;
  ntsys->db2 = 0;
  ntsys->program_name = pname;
  ntsys->program_dir = pdir;

  ntsys->noerrmsg = 0;

  /* loading system table  */
  ntsys->sys_tables[SHARED_OBJ] = A_List();
  for (i=0; i<3; i++) {
    nt_obj_t *stream = makeStream(O_NUM, sys_tables_name[i], 
				  BINARY_STREAM | INPUT_STREAM);
    if (stream) {
      ntsys->sys_tables[i]=readList(stream);
      closeStream(stream);
    } else
      ntsys->sys_tables[i]=A_List();
  }
  clearList(ntsys->sys_tables[SHARED_OBJ]);
  ntsys->sys_tables[ACTIVE_DL] = A_List();

  /* initialize symbol module */
  SymInit();
  /* initialize env module */
  EnvInit();
  /* initialize memory management module */
  ntsys->memory=(char*)ntMalloc(NTSQL_MEMORY_POOL);
  ntsys->freespace=ntsys->memory;

  //  ntsys->permanent = 0;
}

void writeSystemTable(int which)
{
  nt_obj_t *stream = makeStream(O_NUM, sys_tables_name[which], BINARY_STREAM);
  clearList(ntsys->sys_tables[SHARED_OBJ]);
  displayList(stream, ntsys->sys_tables[which], "");

  closeStream(stream);
}
void ntSysQuit(void)
{
  int i;

  ntsys->permanent=1;

  /* close active dynamic library */
  /*
  for (i=0; i<ntsys->sys_tables[ACTIVE_DL]->length; i++) {
    pair_t *pair = (pair_t*)getNthElementList(ntsys->sys_tables[ACTIVE_DL], i);
    if (dlclose((void*)pair->value) !=0 ) {
      displayErr(ERR_IO, dlerror());
    }
    deletePair(pair);
  }

  clearList(ntsys->sys_tables[ACTIVE_DL]); 
  */
  clearList(ntsys->sys_tables[SHARED_OBJ]);
  for (i=0; i<SYS_TABLE_NUM; i++)
    deleteList(ntsys->sys_tables[i]);

  /* close io */
  deleteObj(ntsys->err);
  deleteObj(ntsys->out);
  deleteObj(ntsys->in);

  /* shutdown memory management */
  ntFree(ntsys->memory);

  ntsys->permanent=0;

  /*  ntFree(ntsys);*/
}

#if 0
/************************************************************
  System Table MAINTAINENCE
************************************************************/
nt_obj_t *getSysTableDef(int which, char *name, int *pos)
{
  int i;
  list_t *list = ntsys->sys_tables[which];
  char *aname;

  for (i=0; i<list->length; i++) {
    nt_obj_t *t = getNthElementList(list, i);
    switch (which) {
    case UDF_DEF:
      aname = UdfDefObj(t)->name;
      break;
    case TABLE_DEF:
      aname = TableDefObj(t)->name;
      break;
    case UDF_AGGR_DEF:
      aname = UdfAggrDefObj(t)->name;
      break;
    case ACTIVE_DL:
      aname = ((pair_t*)t)->name;
      break;
    }
    if (strcmp(aname, name)==0) {
      *pos = i;
      return t;
    }
  }
  return (nt_obj_t*)0;
}
/************************************************************
                UDF MAINTAINENCE
************************************************************/
udf_def_t *getUdfDef(char *name, int *pos)
{
  nt_obj_t *obj = getSysTableDef(UDF_DEF, name, pos);

  return (obj)? UdfDefObj(obj):(udf_def_t*)0;
}
void 
createUdfDef(char *name, nt_obj_t *type_list,
	     nt_obj_t* return_type, 
	     char *routine)
{
  int pos;
  udf_def_t *udf;
  nt_obj_t *new; 

  if (getUdfAggrDef(name, &pos) || getUdfDef(name, &pos)) {
    displayErr(ERR_CREATE_EXIST_FUNCTION, name);
    return;
  }

  if ((udf = newUdfDef(name, type_list, return_type, routine))) {
    new = newObj(O_UDF_DEF, (char*)udf);

    ntsys->permanent=1;
    appendElementList(ntsys->sys_tables[UDF_DEF], copyObj(new));
    ntsys->permanent=0;

    writeSystemTable(UDF_DEF);
  }
}
/************************************************************
                UDF_AGGR MAINTAINENCE
************************************************************/
int monotonicUdfAggrDefP(nt_obj_t *obj)
{
  return (UdfAggrDefObj(obj)->routines[AGGR_TERMINATE] == (nt_obj_t*)0)? 1:0;
}

udf_aggr_def_t* getUdaDef(char *name, int *pos)
{
  nt_obj_t *obj = getSysTableDef(UDF_AGGR_DEF, name, pos);

  return (obj)? UdfAggrDefObj(obj):(udf_aggr_def_t*)0;
}
void 
createAggrFunction(char *aggr_name, nt_obj_t *type_list,
		   nt_obj_t *return_type, nt_obj_t *state_type_list,
		   int ordered,
		   nt_obj_t **routines)
{
  int pos;
  udf_aggr_def_t *aggr;
  nt_obj_t *new; 

  if (getUdfAggrDef(aggr_name, &pos) || getUdfDef(aggr_name, &pos)) {
    displayErr(ERR_CREATE_EXIST_FUNCTION, aggr_name);
    return;
  }

  aggr = newUdfAggrDef(aggr_name, type_list, 
		       return_type, state_type_list,
		       ordered, routines);
  new = newObj(O_UDF_AGGR_DEF, (char*)aggr);

  ntsys->permanent=1;
  clearList(ntsys->sys_tables[SHARED_OBJ]);
  appendElementList(ntsys->sys_tables[UDF_AGGR_DEF], copyObj(new));
  ntsys->permanent=0;

  writeSystemTable(UDF_AGGR_DEF);

}
void dropFunction(char *name)
{
  int pos;
  nt_obj_t *old;
  
  if (getUdfAggrDef(name, &pos)) {
    old = removeNthElementList(ntsys->sys_tables[UDF_AGGR_DEF], pos);
    writeSystemTable(UDF_AGGR_DEF);
    ntPrintf(ntsys->out, "\nUser Defined Function '%s' Removed\n\n", name);
  }
  else if (getUdfDef(name, &pos)) {
    old = removeNthElementList(ntsys->sys_tables[UDF_DEF], pos);
    writeSystemTable(UDF_DEF);
    ntPrintf(ntsys->out, "\nUser Defined Aggregate '%s' Removed\n\n", name);
  } else {
    displayErr(ERR_FUNCTION_NOT_EXIST, name);
    return;
  }

  ntsys->permanent=1;
  deleteObj(old);
  ntsys->permanent=0;
}
/************************************************************
                RELATION MAINTAINENCE
************************************************************/
table_def_t *getTableDef(char *name)
{
  int pos;
  nt_obj_t *obj = getSysTableDef(TABLE_DEF, name, &pos);

  return (obj)? TableDefObj(obj):(table_def_t*)0;
}
DB *openTableDB(table_def_t *t)
{
  DB_INFO dbinfo;

  if (!t->dbp) {
    /* Initialize the database. */
    memset(&dbinfo, 0, sizeof(dbinfo));
    dbinfo.db_pagesize = 1024;		/* Page size: 1K. */
    dbinfo.flags = (t->has_primary_key)? 0 : DB_DUP;

    if ((errno = db_open(t->name, DB_BTREE, DB_CREATE, 
			 0664, NULL, &dbinfo, &t->dbp))!=0) {
      displayErr(ERR_BERKELEY_DB, t->name, strerror(errno));
      t->dbp = (DB *)0;
    }
  }
  return t->dbp;
}
/*
heap_t *getTableHeap(table_def_t *t)
{
  if (t->heap ||
      (t->heap = openHeap(t->name, t->length)) ||
      (t->heap = createHeap(t->name, t->length, NULL)))
  return t->heap;
}*/
int createTable(table_def_t *table)
{
  nt_obj_t *new;

  if (getTableDef(table->name)) {
    displayErr(ERR_CREATE_EXIST_TABLE, ltrim(table->name));
    return 0;
  }
  if (unlink(table->name)<0 && errno!=ENOENT) {
    displayErr(ERR_IO, strerror(errno));
    return 0;
  }

  new = newObj(O_TABLE_DEF, (char*)table);

  ntsys->permanent=1;
  appendElementList(ntsys->sys_tables[TABLE_DEF], copyObj(new));
  ntsys->permanent=0;

  writeSystemTable(TABLE_DEF);
  return 1;
}
int dropTable(char *name)
{
  int i;
  list_t *list = ntsys->sys_tables[TABLE_DEF];

  for (i=0; i<list->length; i++) {
    table_def_t *t = TableDefObj(getNthElementList(list, i));
    if (strcmp(t->name, name)==0) {
      nt_obj_t *old = removeNthElementList(list, i);

      ntsys->permanent=1;
      deleteObj(old);
      ntsys->permanent=0;

      writeSystemTable(TABLE_DEF);
      if (unlink(name)<0) {
	displayErr(ERR_IO, strerror(errno));
	return 0;
      }
      return 1;
    }
  }
  
  displayErr(ERR_TABLE_NOT_EXIST, name);
  return 0;
}
#endif
