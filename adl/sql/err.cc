/*
 * errormsg.c - functions used in all phases of the compiler to give
 *              error messages about the Tiger program.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sql/adl_sys.h>
#include "util.h"
#include "err.h"
#include "io.h"
#include "const.h"
extern system_t *ntsys;

int anyErrors= 0;

static char * fileName = (char*)0;

static int lineNum = 1;

int EM_tokPos=0;

FILE *yyin;

typedef struct intList {int i; struct intList *rest;} *IntList;

/************************************************************
                ERROR MESSAGE
************************************************************/
static char *err_msg[]= {
  "no",				/* ERR_NO_ERR */  

  /* folloing message sorted by macro name*/
  
  "type INT or REAL is required for builtin aggregate", /* ERR_AGGREGATE_BUILTIN_TYPE */ 
  "Cannot cascade aggregate '%s'", /* ERR_AGGREGATE_CASCADING */  
  "Aggregate not allowed in group by", /* ERR_AGGREGATE_GROUPBY */  
  "Berkeley DB %s: %s",		/* ERR_BERKELEY_DB */
  "Buffer operation: %s %s",       /* ERR_BUFFER */
  "Aggr(Func) '%s' returns complex data structure.", /* ERR_COMPLEX_DATA */
  "Aggr(Func) '%s' does not return complex data structure.",  /* ERR_COMPLEX_FUNC_EXPECTED */
  "Function '%s' already exists.", /* ERR_CREATE_EXIST_FUNCTION*/
  "Table '%s' already exists.",	/* ERR_CREATE_EXIST_TABLE */  
  "Invalid create stream statement: %s", /* ERR_CREATE_STREAM */
  "Invalid create view statement: %s", /* ERR_CREATE_VIEW */
  "Incorrect data type for operator '%s'.", /* ERR_DATATYPE */  
  "Incompatible dbt field: %s.", /* ERR_DBT_INCOMPATIBLE */
  "Divide by zero.",		/*  ERR_DIV_ZERO */
  "Duplicate column name '%s' in line %d.",		/*  ERR_DUPLICATE_COLUMN_NAME */
  "Duplicate field name in record type: %s.",		/*  ERR_DUPLICATE_FIELD_NAME */
  "Duplicate name in FROM clause: %s.",		/*  ERR_DUPLICATE_JOIN_NAME */
  "Aggr(Func) call should have an alias in subquery",	/* ERR_FUNCTION_ALIAS_MISSING */
  "Function '%s' does not exist", /* ERR_FUNCTION_NOT_EXIST */
  "Non-aggregate select item does not appear in GROUP-BY clause", /* ERR_GROUPBY */  
  "%s calls %s",/* ERR_HISTORY */ 
  "Ident '%s' undefined",	/* ERR_IDENT_UNDEFINED */  
  "Id '%s' is undefined.",	/* ERR_ID_UNDEFINED */  
  "Type Mismatch. %s",		/* ERR_INCOMPATIBLE_TYPE */ 
  "Stream '%s' does not match the expected type.", /*ERR_INCOMPATIBLE_STREAMS */
  "Incompatible time key of stream %s", /* ERR_INCOMPATIBLE_TIMEKEY */
  "Could not create memory table %s", /*  ERR_INMEM_TABLE */
  "Invalid argument for arithmetic operation",	/* ERR_INVALID_ARG */  
  "Invalid command '%s'.",	/* ERR_INVALID_COMMAND */  
  "Invalid delete function use", /* ERR_INVALID_DELETE_FUNC_USE */
  "Invalid external name '%s'. Format is 'lib!func'.", /* ERR_INVALID_EXTERNAL_NAME */
  "Invalid index specification: %s", /*ERR_INVALID_INDEX_SPEC */
  "Invalid use of oldest: %s.", /*ERR_INVALID_OLDEST_USE */
  "Invalid param table %s. The first parameter table should have exactly two columns and both should be chars.", /* ERR_INVALID_PARAM_TABLE */
  "Invalid use of partition_by: %s.", /*ERR_INVALID_PARTITION_USE */
  "Invalid port number, which must be >1024 and <=65536", /*ERR_INVALID_PORT_NUM */
  "Invalid table scope, table %s scope must be %s.", /*ERR_INVALID_SCOPE*/
  "Invalid statement in flow, only create stream, run task, and insert into stmts allowed. The flow should also have exactly one insert into OUTSTREAM statement.", /* ERR_INVALID_STMT_IN_FLOW */
  "Invalid use of table function %s", /*ERR_INVALID_TABLE_FUNCTION_USE */
  "IO error: %s.",		/* ERR_IO */  
  "Join error: %s",             /* ERR_JOIN_STREAM_TABLE */
  "Key '%s' already exists.",	/* ERR_KEY_EXISTS */
  "Key '%s' is not a table column.", /* ERR_KEY_NOT_FOUND */ 
  "Key type mismatch.",		/* ERR_KEY_REF_MATCH */
  "NTSql internal error: %s.",	/* ERR_NTSQL_INTERNAL */  
  "",				/* ERR_NON_BUILTIN_FUNC */ 
  "'->%s' can only be used after a reference type.",    /* ERR_NON_REFERENCE_TYPE */
  "Scalar query required.",	/* ERR_NON_SCALAR */
  "OID function is only allowed as a SELECT ITEM.", /* ERR_OID_ARITHMETIC */ 
  "OID function is only allowed in INSERT statements.", /* ERR_OID_INSERTION */ 
  "Table '%s' does not have reference column.", /* ERR_OID_NONEXIST */ 
  "Unable to open file '%s'.",	/* ERR_OPEN_FILE */  
  "Out of Memory in '%s'.",	/* ERR_OUT_OF_MEMORY */  
  "Panic. '%s'.",		/* ERR_PANIC */
  "Redeclaration of '%s'.",	/* ERR_REDECLARATION */ 
  "Table '%s' using reference type is not an in-memory table", /* ERR_REF_MEMORY_ONLY */
  "The No. %d column of table '%s' is not a reference type column", /* ERR_REF_TYPE_MISMATCH */
  "Only 2-dimension R-Tree supported in table '%s'", /* ERR_RTREE_DIMENSION */
  "R-Tree can not index in-memory table '%s'", /* ERR_RTREE_MEMORY */
  "Only integer type columns can be used as indexed columns for RTREE (table '%s')", /* ERR_RTREE_KEYTYPE */
  "Square root of a negative number: %s", /* ERR_SQRT_NEGATIVE */  
  "Stream operation: %s",       /* ERR_STREAM */  
  "Stream %s target specification not valid, expecting 'machine_name:port'", /* ERR_STREAM_TARGET_SPEC */
  "Processing Sub-query: %s",	/* ERR_SUBQUERY */
  "Syntax error: %s",		/* ERR_SYNTAX */
  "Table '%s' does not exist",	/* ERR_TABLE_NOT_EXIST */  
  "Invalid use of target stream: %s", /* ERR_TARGET_STREAM_USE */
  "Timestamp operation: %s", /* ERR_TIMESTAMP */
  "Support for %s is not implemented yet",	/* ERR_TO_BE_IMPLEMENTED */ 
  "Tuple type requried: %s",	/* ERR_TUPLE_TYPE_REQUIRED */
  "Undefined field: '%s'",	/* ERR_UNDEFINED_FIELD */  
  "Undefined function: '%s', you may also get this error if you are trying to use a windowed aggregate without specifying a window.",	/* ERR_UNDEFINED_FUNCTION */  
  "Referencing undefined table: '%s'",	/* ERR_UNDEFINED_REF */  
  "Undefined type: '%s'",	/* ERR_UNDEFINED_TYPE */  
  "Undefined variable: %s",	/* ERR_UNDEFINED_VARIABLE */  
  "Unexpected err at: '%s'",	/* ERR_UNEXPECTED */  
  "Cardinality of %s is not equal", /* ERR_UNMATCH_CARDINALITY */  
  "Type mismatch of table declaration and initialization.", /* ERR_UNMATCH_INIT_TYPE */
  "Type mismatch of UNION operands.", /* ERR_UNMATCH_UNION_TYPE */  
  "Object type '%s' is not supported in switch-case statement in '%s'.", /* ERR_UNSUPPORTED_CASE_TYPE */
  "Windows tables are only allowed on streams, thus they cannot be user in adl.", /* ERR_WINDOW_NOT_ALLOWED_ON_TABLE */
  "Window OVER (...) not present in calling UDA '%s'.", /* ERR_WINDOW_UDA_REQUIRED */
  "Window use invalid. %s", /* ERR_WINDOW_USE_INVALID */
  "Wrong number of arguments calling function '%s'.", /* ERR_WRONG_NUMBER_OF_ARGS */
}; 

static IntList intList(int i, IntList rest) 
{
  IntList l= (IntList)ntMalloc(sizeof *l);
  l->i=i; l->rest=rest;
  return l;
}

static IntList linePos=NULL;

void EM_newline(void)
{lineNum++;
 linePos = intList(EM_tokPos, linePos);
}

void EM_error(err_t type, int line, char *file, ...){
  va_list ap;
  EM_error(0, type, line, file, ap);
}

void EM_error(int pos, err_t type, int line, char *file, ...)
{
  va_list ap;
  char buf[MAX_STR_LEN];
  IntList lines = linePos; 
  int num=lineNum;
 
  if (ntsys->noerrmsg) return;

  anyErrors=1;
  while (lines && lines->i >= pos) 
    {lines=lines->rest; num--;}

  if (fileName) 
    //ntPrintf(ntsys->err,"%s:",fileName);
    fprintf(stderr,"%s:",fileName);
  if (lines) 
    //ntPrintf(ntsys->err,"%d.%d: ", num, pos-lines->i);
    fprintf(stderr,"%d.%d: ", num, pos-lines->i);
  va_start(ap, file);

  vsprintf(buf, err_msg[type], ap);

  if (pos>0) {
    //ntPrintf(ntsys->err, "Err : [%d@%s] %s at line %d\n", line, file, buf, pos);
    fprintf(stderr, "Err : [%d@%s] %s at line %d\n", line, file, buf, pos);
  } else {
    //    ntPrintf(ntsys->err, "Err : [%d@%s] %s\n", line, file, buf);
    fprintf(stderr, "Err : [%d@%s] %s\n", line, file, buf);
  }

  va_end(ap);
}

void EM_reset(char * fname)
{
  anyErrors=0; fileName=fname; lineNum=1;
  linePos=intList(0,NULL);
  yyin = fopen(fname,"r");
  if (!yyin) {EM_error(0,ERR_OPEN_FILE, __LINE__, __FILE__, __FILE__, fname); exit(1);}
}

/*
void displayErr(err_t type, ...)
{
  char buf[MAX_STR_LEN];
  va_list ap;
  va_start(ap, type);

  vsprintf(buf, err_msg[type], ap);
  ntPrintf(ntsys->err, "Err: %s\n", buf);
  va_end(ap);
}
*/


