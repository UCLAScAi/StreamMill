#ifndef NTSQL_SEMANT_H
#define NTSQL_SEMANT_H

#include <absyn.h>
#include <symbol.h>
#include <types.h>
#include <trans2C.h>
#include "list.h"
#include "adllib.h"
#include <stmt.h>

#include <vector>
//#include <hash_map.h>
#include <ext/hash_map>

using namespace std;
using namespace __gnu_cxx;

/************************************************************************/
/* mode of insert/update                                                */
/************************************************************************/
#define USE_STDOUT 1
#define USE_TEMP   2
#define USE_DIRECT 3

typedef struct expty* T_expty;
struct expty {
  struct timeval* tv;
  Tr_exp exp;
  Ty_ty ty;
  int size;
};

/************************************************************************/
/* Sql_sem: global values during compilation                            */
/************************************************************************/
typedef struct sqlsem *Sql_sem;


hash_map<const char*, void*, hash<const char*>, eqstrTab>* getInMemTables();

struct sqlsem {
  T_expty predec;
  T_expty preinit;
  T_expty preclean;
  T_expty afterpreclean; /*for ESL aggrs and adhoc queries to be able to delete
			   tables, the table might be used multiple times thus 
			   cannot put it in preclean, because other cursors
			   may be open and the table may be deleted*/
  int in_func;			/* inside UDA, UDF */
  char* func_name;           /* function name, for oldest */
  T_expty global;		/* at module level. for example, when
                                   a sort node is compiled, it
                                   generates a cmp function which must
                                   be defined at module level*/
  A_sqlopr cur_sqlopr;		/* the current oper that is being compiled */
  A_sqlopr top_sqlopr;		/* the outmost oper  */
  int cur_sqlopr_flag;		/* the current sql opr is a SCALAR=1, EXISTS=2, NOTEXISTS=3 */
  int update_mode;		/* USE_DIRECT or USE_TEMP */
  int noerrmsg;			/* for test-compile */
};

void show(S_symbol sym, void* binding);


Sql_sem SqlSem(void);
void SqlSem_Delete(Sql_sem sql);
void addSqlSemFirstEntryDec(Sql_sem sql, void *f);
void addSqlSemIndexDec(Sql_sem sql, void *f);
err_t addSqlSemCursorDec(Sql_sem sql, S_table venv, A_qun qun, char* tabLoc=NULL);
void mergeSqlSem(Sql_sem sql1, Sql_sem sql2);


Ty_ty checkType(S_table tenv, S_symbol sym);

/* constructor */
T_expty expTy(Tr_exp exp, Ty_ty ty);
T_expty expTy(Tr_exp exp, struct timeval* tv, Ty_ty ty);
T_expty expTy_Seq(T_expty seq, T_expty e);
T_expty expTy_Seq(T_expty seq, char *buf);
void expTy_Delete(T_expty e);

err_t
transDec(S_table venv, S_table tenv,
         A_dec d,               // IN: a declaration
         T_expty &edec,         // declaration. e.g. "DB *tab;"
         T_expty &einit,        // initialization. e.g. open a table
         T_expty &eclean,       // clean up. e.g. close a table
         int inaggr,            /* IN: inaggr= 1 if we are processing
                                   local variables of an aggregate
                                   routine*/
         T_expty &adec,         /* defer aggregate till end of current
                                   struct - only needed when inaggr =1*/
         vector<void*> aggregates, /* for nested aggregates */
         A_list reflist /* OUT: contains the attribute
                                   attributes whose types are not declared */
);

err_t
transAggrDec(S_table venv,
             S_table tenv,
             A_dec d,
             T_expty &edec,
             T_expty &einit,
             T_expty &eclean,
             int inaggr,
             vector<void*> aggregates,
	     int default_win = 0 //signifies that the aggregate is 
	                         //default window interpretaion, so can be replaced
             );

err_t transExp(S_table venv, S_table tenv, A_exp a, Sql_sem sql,
               T_expty &dec, T_expty &exe, vector<void*> aggregates,
               char* target_handle=NULL, cStmt* cstmt=NULL
               );

err_t transCreateviewExp(S_table venv, S_table tenv, A_exp a, Sql_sem sql,
               T_expty &dec, T_expty &exe, vector<void*> aggregates
               );

err_t transCreatestreamExp(S_table venv, S_table tenv, A_exp a, Sql_sem sql,
               T_expty &dec, T_expty &exe, vector<void*> aggregates, 
               cStmt* cstmt);

err_t transSeqDec(S_table venv, S_table tenv, A_list decl,
                  T_expty &dec, T_expty &exe, T_expty &clean,
                  int inaggr, T_expty &adec, vector<void*> aggregates);


err_t transGBOpr(S_table venv, S_table tenv,
                 A_sqlopr a, Sql_sem sql,
                 T_expty &dec, T_expty &exe, char *name,
                 vector<void*> aggregates,
	       vector<string> &srcs);

err_t transSqlQuery(S_table venv,
                    S_table tenv,
                    A_sqlopr a,
                    Sql_sem sql,
                    T_expty &dec,
                    T_expty &exe,
                    char *name,
                    vector<void*> aggregates,
		    vector<string> &srcs,         // source buffers' name
                    char* target_handle,
                    cStmt*& cstmt
                    );

err_t
transSelOpr(S_table venv,
            S_table tenv,
            A_sqlopr a,
            Sql_sem sql,
            T_expty &dec,
            T_expty &exe,
            char *name,
            vector<void*> aggregates,
	    vector<string> &srcs,         // source buffers' name	    
            cStmt*& cstmt
);

err_t assignFuncBtreeKey(S_table venv, E_enventry te, char* buf);

err_t transOrderOpr(S_table venv, S_table tenv, A_sqlopr a, Sql_sem sql,
                    T_expty &dec, T_expty &exe, char *name, vector<void*> aggregates, vector<string> &srcs);

err_t transUDFDec(S_table venv, S_table tenv, A_dec d,
                  T_expty &edec, T_expty &einit, T_expty &eclean, vector<void*> aggregates);

err_t compileArgs(S_table venv,         /* variable env */
                  S_table tenv,         /* type env */
                  Sql_sem sql,          /* sql semantics */
                  A_list argdecs,
                  A_list formals,
                  T_expty &exe,
                  vector<void*> aggregates);    // result

err_t transBaseQun(S_table venv,
                   S_table tenv,
                   A_qun q,
                   Sql_sem sql,
                   T_expty &dec,        // OUT
                   T_expty &exe,        // OUT
                   char *name,
                   Tr_exp *bndp,        // IN
                   Tr_exp *bndpUpper,   // IN
                   Tr_exp *bndpLower,   // IN
                   int flag,
                   vector<void*> aggregates,
		   vector<string> &srcs,         // source buffers' name
                   char* target_handle,
                   cStmt*& cstmt
                   );
err_t transFunQun(S_table venv,
                  S_table tenv,
                  A_qun q,
                  Sql_sem sql,
                  T_expty &dec, // OUT
                  T_expty &exe, // OUT
                  vector<void*> aggregates
                  );

err_t transWinQun(S_table venv,
                  S_table tenv,
                  A_qun q,
                  Sql_sem sql,
                  T_expty &dec, // OUT
                  T_expty &exe, // OUT
                  vector<void*> aggregates
                  );

err_t transQun(S_table venv,
               S_table tenv,
               A_qun q,
               Sql_sem sql,
               T_expty &dec,
               T_expty &exe,
               char *name,
               Tr_exp *bndp,
               Tr_exp *bndpUpper,
               Tr_exp *bndpLower,
               int flag,
               vector<void*> aggregates,
		   vector<string> &srcs,         // source buffers' name
               char* target_handle,
               cStmt*& cstmt
	       );

err_t
transBuiltInFunc(S_table venv,          /* variable env */
                 S_table tenv,          /* type env */
                 A_exp a,               /* abstract syntacs */
                 Sql_sem sql,           /* sql semantics */
                 T_expty &dec,          /* OUTPUT: declartion part */
                 T_expty &exe,          /* OUTPUT: executable part */
                 vector<void*> aggregates
                 );


err_t transQun(S_table venv, 
	       S_table tenv, 
	       A_qun q, 
	       Sql_sem sql, 
	       T_expty &dec, 
	       T_expty &exe, 
               char *name,
               Tr_exp *bndp,            // IN: binding equality
               Tr_exp *bndpUpper,       // IN: binding upper bound
               Tr_exp *bndpLower,       // IN: binding lower bound
               int flag,                 // IN: binding flag
               vector<void*> aggregates,
		   vector<string> &srcs         // source buffers' name
               );

err_t transVar(S_table venv, S_table tenv, 
	       A_var v, 
	       Sql_sem sql, 
	       A_qun *outqun, // OUT: the qun this var belongs to
	       int *fieldidx, // OUT: the index of the field in the QUN
	       T_expty &result);
err_t transTy(S_table venv, 
	      S_table tenv, 
	      A_ty a, 
	      Ty_ty &ty, 
	      A_list reflist = (A_list)0,
	      int *foundref = (int*)0,
              absyn_t decType = A_tabVarDec);
err_t transFields(S_table venv, S_table tenv, 
		  A_list alist, 
		  A_list fields, 
		  A_list reflist = (A_list)0,
		  int *foundref = (int*)0,
                  absyn_t decType = A_tabVarDec);
void getQunName(A_qun qun, char *qunname);
err_t transRef(S_table venv,
	       S_table tenv,
	       A_ref ref,
	       Sql_sem sql,
	       T_expty &dec,
	       T_expty &exe
	       );
err_t transQun(S_table venv, 
	       S_table tenv, 
	       A_qun q, 
	       Sql_sem sql, 
	       T_expty &dec, 
	       T_expty &exe, 
	       char *name,
	       Tr_exp *bndp,
	       Tr_exp *bndpUpper,
	       Tr_exp *bndpLower,
	       int flag,
	       vector<void*> aggregates,
	       vector<string> &srcs);
err_t isQunBound(S_table venv, 
		 S_table tenv, 
		 A_sqlopr a, 
		 int qunidx, 
		 Sql_sem sql, 
		 Tr_exp *&exep,
		 Tr_exp *&exepUpper,
		 Tr_exp *&bndpLower,
		 int &flag
);
err_t
checkTabIndex(S_table venv, 
	      A_dec d,		// the tablevar declaration
	      Ty_ty ty,		// column types
	      E_enventry var	// the tablevar
	      );

err_t
checkTabIndexCreateview(A_exp e,// create view expression
	      Ty_ty ty,		// column types
	      E_enventry var	// the tablevar
	      );

/* I think this is a redefinition, so ...
  err_t
  transAggrDec(S_table venv,
  S_table tenv,
  A_dec d,
  T_expty &edec,
  T_expty &einit,
  T_expty &eclean,
  int inaggr,
  std::vector<void*> aggregates
  );*/

void sqlStatementGlobalInit(S_table venv,
			    Sql_sem sql, 
			    T_expty &dec, 
			    T_expty &exe);


static char queryName[1024];
static int adHoc = 0;
static int adHocNum = 0;
static int isESLAgg = 0;
static char eslAggrName[1024];
// By default, setQueryName() returns true when it is in ESL environment, by testing a static variable queryName.
// If s is other than "__test__",  we set the queryName to s as function name, used in ESL.
// For Atlas we use empty string ("") for queryName
// We couldn't use "extern"+"static" global variable for this purpose, because by definition, static variable is used only in this file
void setAdHoc(int value);
int isAdHoc();
void setAdHocNum(int value);
int getAdHocNum();

void setIsESLAggr(int value);
int isESLAggr();

void setAggrName(const char* aggrName);
char* getAggrName();

bool setQueryName(const char*s="__test__");
char* getQueryName();

static bool firstEntryFlag[MAX_UID];
// to prevent first_entry redeclaration
bool isFirstEntryDec(int i);

void setFirstEntryFlag(int i);

void resetFirstEntryFlag();

static bool ESLTestFlag = false;
void setESLTestFlag(bool flag=true);
bool isESLTest();


#endif /* NTSQL_SEMANT_H */

