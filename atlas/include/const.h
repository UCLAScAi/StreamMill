#ifndef NTSQL_CONST_H
#define NTSQL_CONST_Hf



/***********************************************************************
                              NT-SQL CONSTANTS
***********************************************************************/
#define TABLE_NAME_LEN		    20
#define TABLE_COLUMN_NAME_LEN	    12
#define MAX_COLUMN_IN_TABLE	    20
#define FUNCTION_NAME_LEN	    24
#define LIBRARY_NAME_LEN	    24
#define MAX_FUNC_ARITY		    10
#define DEFAULT_NUM_DISPLAY_LEN	    10
#define MAX_ARITY_OF_STRUCT_DATA_TYPE	10

#define SQL_ASC	    0
#define SQL_DESC    1
#define SQL_UNION   1
#define SQL_INTERSECT 2
#define SQL_EXCEPT  3

#define COLUMN_NOT_NULL 1
#define COLUMN_PRIMARY_KEY 1

#define TRUE 1
#define FALSE 0

//#define SUCCESS 0
#define FAIL 1

//#define R_MAXINT 0x7fffffff         //RTREE Max Integer
//#define R_MININT 0x80000001         //RTREE Min Integer

#define MAX_NUM_KEY  255               //MAX # of keys
/*********************
  mode of key binding
 *********************/
#define BOUND_NONE   0
#define BOUND_ALL    1  //btree
#define BOUND_FIRST  2  //btree
#define BOUND_RTREE  3  

#define MAX_STR_LEN 40960  // max string buffer
#define LARGE_MAX_STR_LEN 819200  // max string buffer

#define TIMESTAMP_FORMAT "%I:%M:%S %p %D"  //"%d %b %y %H:%M:%S"


#define EXT_COUNT 200    // Max # of external functions that require string return buffer
//#define DBUG_OFF 
/**************************************************
ESL constants
**************************************************/
// Max identifier string size
#define MAX_NAME  256


//#define SM_KEY "/esl_shared_memory"
#define SM_SIZE 10240000 // in bytes

// Shared memory pool size (bytes) for DBTs
#define DBT_SM_SIZE 30720000

// Max # of buffers
#define MAX_BUF 500

// default max number of tuples to run for newly created drivers 
//(before exit to see if there is any user commands that needs to be handled)
//at some point too high number will hurt the interactivity of program - user command take longer to be responded when there are tuples in system.
#define DEFAULT_TUPLE_PER_PROMPT 100000

// internal timekey column name
#define ITIME_COLUMN "current_time" 

// timestamp size
#define TS_SZ sizeof(timestamp)

#define DCL_EXPIRE true

#define MAX_UID 1024

#endif
