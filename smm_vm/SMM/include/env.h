#ifndef NTSQL_ENV_H
#define NTSQL_ENV_H

#include <symbol.h>
#include <types.h>
#include <absyn.h>
#include "list.h"

typedef enum {
  E_dynamicEntry,
  E_modelTypeEntry,
  E_varEntry,
  E_funEntry,
  E_aggrEntry,
  E_extEntry,
  E_streamEntry
} entry_t;

typedef struct E_enventry_ *E_enventry;


#define AGGR_INIT		1
#define AGGR_ITERATE		2
#define AGGR_TERMINATE		4
#define AGGR_EXPIRE		8
#define AGGR_INIT_ITERATE	16

#define AGGR_BUILTIN_SUM    1
#define AGGR_BUILTIN_COUNT  2
#define AGGR_BUILTIN_MIN    3
#define AGGR_BUILTIN_MAX    4
#define AGGR_BUILTIN_MINR    5
#define AGGR_BUILTIN_MAXR    6
#define AGGR_BUILTIN_SUMR    7
#define AGGR_BUILTIN_COUNTR  8
#define AGGR_BUILTIN_AVG    9
#define AGGR_BUILTIN_XA    10 
#define AGGR_BUILTIN_VAR  13

#define FUN_BUILTIN_XE     11 
#define FUN_BUILTIN_ATTR   12

/* Names:
 *
 * A table in the FROM clause, for example, could have the following forms:
 * 
 * 1) tab AS t  
 *
 *    The data passed along the QUN will be stored in a structure, and
 *    the name of the structure is stored in "var.sname". 
 *
 *    The handle of physical storage (BERKELEY DB, HASH, ..) is
 *    "tab". If it is a local table declared inside an aggregate
 *    routine, then its handle is stored in "var.iname".
 * 
 *    The file name of the physical storage is "_adl_db_%s", where %s
 *    is the var.iname or var.name if there is no internal name for
 *    this entry. However, if the variable is a local variable of an
 *    aggregate routine, the file name is: "._%d_%s", where %d is the
 *    runtime value of aggregate status, and %s is var.name.
 *
 *    The name of the cursor over the physical storage is "t".  
 * 
 * 2) (query) AS t
 *
 *    The data passed along the QUN will be stored in a structure, and
 *    the name of the structure is stored in "var.sname". 
 * 
 *    The name of the cursor over the physical storage is "t".  */
#define TAB_INAME 0
#define TAB_SNAME 1
struct E_enventry_
{
  entry_t kind;
  S_symbol key;                 /* to enable nested aggregates*/
  union {
    struct {
      S_symbol sname;		/* the name of the structure passed
                                   along a QUN */
      S_symbol source;		/* file name, e.g. */
      S_symbol target;		/* file name, e.g. */
      timekey_t tk;             // timekey flag
      S_symbol timekey;         // timekey
      Ty_ty ty;
      int size;
      int isBuiltin;            // determines if the stream uses builtin iomod
      int port;                 // the port for the builtin iomod
      S_symbol orig;            /* Once a qun is translated, we create another 
				   env var with its alias.  For join statements
				   we need to retrieve the name of the original
				   stream. So we put it here, in transQun QUN_NAME*/
    } stream;			
    struct {
      A_list sharedtables;
      A_list modelitems;
      A_list flows;
    } modeltype;
    struct {
      S_symbol table;
      S_symbol rawname;
    } dynamic;
    struct {
      S_symbol sname;		/* the name of the structure passed
                                   along a QUN */
      S_symbol iname;		/* internal name. For example, a table
				   "tab" declared in an aggregate
				   routine is renamed to "state->tab" */
      S_symbol source;		/* file name, e.g. */
      tabscope_t scope;		/* if var is a table */
      A_index index;		/* if var is a table */
      int haskey;		/* if var is a table */
      Ty_ty ty;
      int size;
      int inaggr;		/* if var is a local variable in an
                                   aggregate routine */
      int firstKey;             /* if var is a table, first key position in ty*/
      int isStdout;             /* for buffers that emulating Stdout */
      int isBuffer;
    } var;			
    struct {
      A_list formals;
      Ty_ty result;
      int builtin;		/* min/max/sum/avg/count */
      int aggr_routines;	/* a bit OR of INIT/ITERATE/TERMINATE */
      int varied_args;		/* varied number of args */
      int inaggr;               /* if inner aggregate */
      int implicit;
      int window;		/* must be called with a window  */
      int default_win;          /* the aggr definition is derived from base version*/
    } fun;
    struct {
      A_list formals;
      Ty_ty result;
      int handle;		/* the identifier of the function */
      int size;
      S_symbol actual;
      S_symbol externlib;
      int varied_args;
    } ext;
  } u;
};

E_enventry E_ModelTypeEntry(S_symbol key, A_list sharedtables, 
                            A_list modelitems, A_list flows);
E_enventry E_DynamicEntry(S_symbol key, S_symbol dynamic, S_symbol rawname);
E_enventry E_VarEntry(S_symbol key, Ty_ty ty, int size,
		      tabscope_t scope = TAB_LOCAL,
		      A_index index = (A_index)0,
		      int haskey = 0, int isStdout = 0, int isBuffer = 0);
E_enventry E_StreamEntry(S_symbol name, 
			 Ty_ty ty, int size,
			 S_symbol source=0,
			 S_symbol target=0,
			 timekey_t tk = tk_none,
                         S_symbol timekey = 0,
			 int isBuuiltin = 0,
			 int port = -1);
E_enventry E_FunEntry(S_symbol key, A_list formals, Ty_ty result, int varied_args=0);


E_enventry E_ExtEntry(S_symbol key, A_list formals, Ty_ty result, 
		      S_symbol actual, S_symbol externlib, int size,
                      int varied_args=0);

err_t setVarName(S_table venv, S_symbol sym, int usage, S_symbol name);

/* aggr_routines is a bitmap ORing of the following values:
   AGGR_INIT, AGGR_ITERATE, AGGR_TERMINATE
*/
E_enventry E_AggrEntry(S_symbol key,
                       A_list formals, 
		       Ty_ty result, 
		       int aggr_routines,
                       int inaggr,
                       int implicit,
		       int builtin=0,
		       int default_win=0);

void EnvInit(void);

#endif /* NTSQL_ENV_H */


/* EnvCpy: Copy variable env */
void EnvCpy(E_enventry te, E_enventry x);

void displayFunEntry(E_enventry e);
