#ifndef SQL_SYSTEM_H
#define SQL_SYSTEM_H

#include "io.h"
#include "util.h"
#include "err.h"
#include <adl_obj.h>
#include "list.h"

/************************************************************
                SYSTEM CONSTANTS
************************************************************/
#define SYS_TABLE_NUM	5

#define TABLE_DEF	0
#define UDF_DEF		1
#define UDF_AGGR_DEF	2
#define ACTIVE_DL	3
#define SHARED_OBJ	4

#define NTSQL_MEMORY_POOL   1024*128
/************************************************************
                SYSTEM DATA STRUCTURE
************************************************************/
typedef struct system_s {
  char *program_dir, *program_name;
  nt_obj_t *err, *out, *in;
  A_list sys_tables[SYS_TABLE_NUM];
  int verbose;
  int db2;

  /* for test compile */
  int noerrmsg;

  /* memory management */
  int permanent;
  char *memory;
  char *freespace;
  
#ifdef SQL_AG
  char *cfile;
  int unfenced;
#endif
} system_t;

/************************************************************
                SYSTEM METHODS
************************************************************/
void ntSysInit(char *pdir, char *pname);
void ntSysQuit(void);
/*  void displayErr(err_t type, ...); */

/*table_def_t *getTableDef(char *name);
nt_obj_t *getUdaDef(char *name, int *pos);
nt_obj_t *getUdfDef(char *name, int *pos);
void createUdfDef(char *name, nt_obj_t *type_list,
		  nt_obj_t* return_type, 
		  char *routine);
void createAggrFunction(char *aggr_name, nt_obj_t *type_list,
			nt_obj_t *return_type, nt_obj_t *state_type_list,
			int ordered,
			nt_obj_t **routines);
heap_t *getTableHeap(table_def_t *td);
int createTable(table_def_t *t);
int dropTable(char *name);
DB *openTableDB(table_def_t *t);
*/
int containObjType(nt_obj_t *expr, int type);

#endif











