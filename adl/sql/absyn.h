#ifndef NTSQL_ABSYN_H
#define NTSQL_ABSYN_H

#include "symbol.h"
#include <err.h>
#include <abstypes.h>
#include <list.h>
#include "const.h"
#include <time.h>
#include <vector>
#include <string>

using namespace std;

static char __userName__[1024];
void setUserName(const char*s="__user__");
char* getUserName();
void getUserNameie(char*);

static char __preModelName__[1024];
static char __modelName__[1024];
void setModelName(const char*s="__model__");
char* getModelName();
char* getJustName(char* fName);
void cpModelId(char* modelId);
//void getModelPrepend(char* pre);

typedef enum{
  tk_external=0,
    tk_internal,
    tk_none,
    }timekey_t;

/* pointer to parent */
#define setParent(child, parent, parent_type) \
{ if (child) { \
     (child)->pp = (void*)(parent); \
     (child)->ppt = parent_type; \
  } \
}

/* Type Definitions */

typedef struct A_var_ *A_var;
typedef struct A_ref_ *A_ref;
typedef struct A_lval_spec_ *A_lval_spec;
typedef struct A_exp_ *A_exp;
typedef struct A_dec_ *A_dec;
typedef struct A_index_ *A_index;
typedef struct A_fieldExp_ *A_fieldExp;
typedef struct A_ty_ *A_ty;


typedef struct A_field_ *A_field;
/* typedef struct A_efield_ *A_efield; */
typedef struct A_fundec_ *A_fundec;
typedef struct A_aggrdec_ *A_aggrdec;
typedef struct A_namety_ *A_namety;


/* SQL */
typedef struct A_withitem_ *A_withitem;
typedef struct A_selectitem_ *A_selectitem;
typedef struct A_qun_ *A_qun;
typedef struct A_win_ *A_win;
typedef struct A_range_ *A_range;
typedef struct A_slide_ *A_slide;
typedef struct A_tablecolumn_ *A_tablecolumn;
typedef struct A_orderitem_ *A_orderitem;
typedef struct A_sqlopr_ *A_sqlopr;
typedef struct A_modelitem_ *A_modelitem;
typedef struct A_flow_ *A_flow;

#if 0
struct A_list_
{
  A_pos pos;
  adl_t type;
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */
  list_t *list;
};
#endif

/* operator */
typedef enum 
  {
    A_plusOp,      
    A_minusOp,     
    A_timesOp,     
    A_divideOp,    
    A_modOp,       		/* % */
    A_pluseqOp,
    A_minuseqOp,
    A_eqOp,        
    A_neqOp,       
    A_ltOp,        
    A_leOp,        
    A_gtOp,        
    A_geOp,        
    A_andOp,       
    A_orOp,        
    A_likeOp,      
    A_nlikeOp,     
    A_inOp,        
    A_ninOp,       
    A_existOp,     
    A_notexistOp,  
    A_isnullOp,    
    A_isnnullOp    
  } A_oper;

typedef enum
  {
    A_simple,
    A_window
  } aggr_t;

typedef enum
  {
    A_immidiate = 0,
    A_defferred
  } viewmode_t;


/* variable */
struct A_lval_spec_
{
  absyn_t kind;
  A_pos pos;
  A_lval_spec spec;
  union {
    S_symbol sym;
    A_exp exp;
  } u;
};

struct A_var_
{
  absyn_t kind;
  A_pos pos;

  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */

  union {
    S_symbol simple;
    A_ref ref;			/* ref var */
    struct {
      A_var var;
      S_symbol sym;
    } field;
    struct {
      A_var var;
      A_exp exp;
    } subscript;
  } u;
};

struct A_ref_
{
  absyn_t kind;
  A_pos pos;

  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */

  union {
    A_ref ref;
    A_var var;
  } u;
  S_symbol col;
};


/* exp */
#define SHARED_INVOCATION 1

struct A_exp_
{
  absyn_t kind;
  A_pos pos;
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */
  union {
    A_var var;
    A_ref ref;
    /* nil; - needs only the pos */
    int intt;
    double realt;
    char * string;
    struct timeval* timestamp;
    struct {
      S_symbol func; 
      A_list args;
      S_symbol member;		/* in the case that func/aggr returns
                                   complex data type, "member" is a
                                   field name in the record type */
      int shared;		/* SHARED_INVOCATION: Another invocation of the same
				   aggregate exists in the head. This
				   invocation will not do the
				   aggregation, but instead, retrieve
				   the result of the other invocation.
				*/
      A_win win;
      char* init;
    } call;
    struct {
      A_oper oper; 
      A_exp left; 
      A_exp right;
    } op;
    struct {
      S_symbol typ; 
      A_list fields;
    } record;
    A_list seq;
    struct {
      A_var var; 
      A_exp exp;
    } assign;
    struct {
      A_exp test;
      A_exp then;
      A_exp elsee;
    } iff; /* elsee is optional */
    struct {
      A_exp test;
      A_exp body;
    } whilee;
    struct {
      S_symbol var; 
      A_exp lo,hi,body; 
      int escape;
    } forr;
    /* breakk; - need only the pos */
    struct {
      A_list decs; 
      A_exp body;
    } let;
    struct {
      S_symbol typ; 
      A_exp size, init;
    } array;
    struct {
      int distinct;
      A_list hxp_list;
      A_list join_table_list;
      A_list wr_prd_list;
      A_list group_by_list;
      A_list order_by_list;
      A_list hv_prd_list;
    } select;
    struct {
      S_symbol modelName;
      S_symbol task;
      S_symbol source;
      A_win win;
      A_list params;
    } runtask;
    struct {
      S_symbol name;
      A_exp query;
      A_list keydecs;		/* declaration of keys */
      int keyPos[MAX_NUM_KEY];         /* positions of keys in the column list */
      viewmode_t mode;
    } createview;
    struct {
      S_symbol name;
      A_exp query;
      S_symbol timekey;
    } createstream;
    A_sqlopr sqlopr;
  } u;
};

typedef enum {
  TAB_LOCAL,
  TAB_GLOBAL,
  TAB_MEMORY,
  TAB_WINDOW,
  TAB_EXPIRED,
} tabscope_t;

/*
  typedef enum {
  TAB_BTREE,
  TAB_RTREE
  } tabindex_t;
*/


typedef enum {
  INDEX_BTREE=1,
  INDEX_HASH,
  INDEX_RECNO,
  INDEX_QUEUE,
  INDEX_RTREE,
  INDEX_STREAM       // we treat stream as a special index, in order to reuse Atlas codes in ESL
} tabindex_t;

struct A_flow_
{
  A_pos pos;
  void *pp;                     /* points to its parent structure */
  adl_t ppt;                    /* type of its parent structure */

  S_symbol name;
  A_list statements;
};

struct A_modelitem_
{
  A_pos pos;
  void *pp;                     /* points to its parent structure */
  adl_t ppt;                    /* type of its parent structure */

  S_symbol name;
  S_symbol uda;
  int window;
  A_list paramtables;
  A_list allowedparams;
}
;

struct A_dec_
{
  absyn_t kind;
  A_pos pos;
  S_symbol name;
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */

  union {
    struct {
      S_symbol copyModel;
      int window;
      A_list sharedtables;
      A_list modelitems;
      A_list flows;
    } modeltype;
    struct {
      A_list params;
      A_ty result; 
      A_list decs;
      A_exp body;
    } fun;
    struct {
      S_symbol actual;
      A_list params; 
      A_ty result; 
      S_symbol externlib;
      int size;
    } ext;
    struct {
      aggr_t type;
      A_dec window;
      A_list params;
      A_ty result;
      A_list decs;
      A_exp init;
      A_exp iterate;
      A_exp expire;
      A_exp terminate;
      int create_windowed;
      int is_c_aggr;
      char* c_global_decs;
      char* c_decs;
      char* c_init;
      char* c_iter;
      char* c_expire;
      char* c_term;
    } aggr;
    A_list type;
    /* escape may change after the initial declaration */
    struct {
      S_symbol typ; 
      A_exp init; 
      int escape;
    } var;
    struct {
      S_symbol table;
      S_symbol rawname;
    } dynamic;
    struct {
      A_ty ty; 
      A_exp init; 
      tabscope_t scope;		/* memory,local,global */
      S_symbol source;		/* source (file name, e.g.) for global table */
      A_index index;
      int oid_enabled;		/* OID */
      int escape;
      int isView;
    } tabvar;
    struct {
      A_ty ty; 
      A_exp init; 
      S_symbol source;		/* source (file name, e.g.) for global table */
      S_symbol target;		/* source (file name, e.g.) for global table */
      S_symbol timekey;         // time key 
      timekey_t tk;             // time key type
      int escape;
      int isBuiltin;
      int port;
    } streamvar;
  } u;
};

struct A_index_
{
  A_pos pos;
  tabindex_t kind;		/* B+tree, HASH, RECNO,
                                   QUEUE, ... currently not in use,
				   RTree
				*/
  A_list keydecs;		/* declaration of keys */
  // ??? REPLACE keyPos with a list
  int keyPos[MAX_NUM_KEY];         /* positions of keys in the column list */
  
  S_symbol func;
  int len;
};

struct A_fieldExp_
{
  S_symbol name;
  char *exp;
};

struct A_ty_
{
  absyn_t kind;
  A_pos pos;
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */
  union {
    S_symbol name;
    A_list record;
    S_symbol array;
  } u;
};

/***************************************************************/
/*                     SQL                                     */
/***************************************************************/
typedef enum {
  A_SQL_SEL,
  A_SQL_GB,
  A_SQL_UNION,
  A_SQL_EXCEPT,
  A_SQL_INTERSECT,
  A_SQL_INSERT,
  A_SQL_DELETE,
  A_SQL_UPDATE,
  A_SQL_LOAD,
  A_SQL_ORDER
} sqlopr_t;

/* the GB_EXISTS_FLAG flag is set if a GB opr is immediately inside an
   EXISTS/NOT EXISTS operator */
#define GB_EXISTS_FLAG (0x2)

struct A_sqlopr_
{
  sqlopr_t kind;
  int distinct;
  A_list hxp_list;
  A_list jtl_list;
  A_list prd_list;
  A_pos pos;
};

struct A_withitem_
{
  A_pos pos;
};

struct A_orderitem_ 
{
  A_pos pos;
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */
  A_exp exp;
  int dir;
};

typedef enum selectitem_t {
  SIMPLE_ITEM,
  STAR_ITEM,
  COMPLEX_ITEM
};

struct A_selectitem_
{
  selectitem_t kind;
  A_pos pos;
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */

  union {
    struct {
      A_exp exp;
      S_symbol alias;
    } s;			/* simple item */
    struct {
      A_exp exp;
      A_list aliaslist;
    } c;			/* complex item */
    S_symbol table;		/* "table.*" */
  } u;
};

/* QUN represents a "node" in query graph
 * It's basically what gets returned to the
 * parent node
 */
typedef enum {
  QUN_NAME,
  QUN_FUNCTION,
  QUN_QUERY,
  QUN_EXPIRED,
  QUN_WINDOW
} qun_t;

typedef enum {
  TIME_RANGE,
  COUNT_RANGE
} range_t;
typedef enum {
  TIME_SLIDE = 1,
  COUNT_SLIDE = 2,
  HEARTBEAT_TIME_SLIDE = 3
} slide_t;

struct A_range_
{
  A_pos pos;
  int size;
  range_t type;
};
struct A_slide_
{
  A_pos pos;
  int size;
  slide_t type;
};
struct A_win_
{
  A_pos pos;
  A_list order_by_list;
  A_list partition_list;
  A_range range;
  A_slide slide;
};
 
struct A_qun_
{
  A_pos pos;
  qun_t kind;
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */
  S_symbol alias;
  union {
    struct {
      S_symbol name;
      bool dynamic;
      S_symbol rawname;
    } table;
    struct {
      S_symbol name;
      A_list args;
    } function;
    struct {
      S_symbol name;
      A_range range;
      S_symbol respectTo;
    } window;
    A_exp query;
  } u;
  A_sqlopr ppnode;			/* points to its parent node */
  //  A_win win;
};

struct A_tablecolumn_
{
  A_pos pos;
};

/* Linked lists and nodes of lists */

struct A_field_
{
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */

  A_pos pos; 
  S_symbol name, typ; 
  int size;
  int escape;
};

#define isRefType(f) ((f)->size==-1)

/*
  struct A_fundec_
  {
  A_pos pos;
  S_symbol name; 
  A_list params; 
  S_symbol result; 
  A_exp body;
  };
  struct A_aggrdec_
  {
  A_pos pos;
  S_symbol name;
  A_list params;
  S_symbol result;
  A_list decs;
  A_exp init;
  A_exp iterate;
  A_exp produce;
  A_exp terminate;
  };
*/
struct A_namety_
{
  S_symbol name; 
  A_ty ty;
};

/*
  struct A_efield_ 
  {
  S_symbol name; 
  A_exp exp;
  };
*/

/* Function Prototypes */


char *strlwr(char *s);
A_exp A_RefExp(A_pos pos, A_ref ref);
A_ref A_Ref(A_pos pos, A_ref ref, S_symbol col);
A_ref A_Ref(A_pos pos, A_var var, S_symbol col);
A_ref copyRef(A_ref ref);

A_var A_Var(A_var var, A_lval_spec s);
A_var copyVar(A_var var);

A_var A_SimpleVar(A_pos pos, S_symbol sym);
A_var A_RefVar(A_pos pos, A_ref ref);
A_var A_FieldVar(A_pos pos, A_var var, S_symbol sym);
A_var A_SubscriptVar(A_pos pos, A_var var, A_exp exp);
S_symbol getVarNameSuffix(A_var var);
A_lval_spec A_FieldVarSpec(A_pos pos, S_symbol sym, A_lval_spec spec);
A_lval_spec A_SubscriptVarSpec(A_pos pos, A_exp exp, A_lval_spec spec);

void displayExp(A_exp e);
void displaySqlOpr(A_sqlopr s);
void displayVar(A_var var);
void displayRef(A_ref ref);
void displaySelectItem(A_selectitem si);
void displayQun(A_qun qun);
int equalExp(A_exp exp1, A_exp exp2);
int equalVar(A_var var1, A_var var2);
int equalWin(A_win win1, A_win win2);
int equalList(A_list list1, A_list list2);

A_exp copyExp(A_exp exp);
A_field copyField(A_field field);
A_exp A_VarExp(A_pos pos, A_var var);
A_exp A_NilExp(A_pos pos);
A_exp A_IntExp(A_pos pos, int i);
A_exp A_RealExp(A_pos pos, double i);
A_exp A_StringExp(A_pos pos, char *s);
A_exp A_TimestampExp(A_pos pos, char* timestamp);
A_exp A_TimestampExp(A_pos pos, timeval* t);
A_exp A_CallExp(A_pos pos, S_symbol func, A_list args, S_symbol member,
                char* init = (char*)0);
A_exp A_OpExp(A_pos pos, A_oper oper, A_exp left, A_exp right);
A_exp A_RecordExp(A_pos pos, S_symbol typ, A_list fields);
A_exp A_SeqExp(A_pos pos, A_list seq);
A_exp A_AssignExp(A_pos pos, A_var var, A_exp exp);
A_exp A_IfExp(A_pos pos, A_exp test, A_exp then, A_exp elsee);
A_exp A_CaseExp(A_pos pos, A_exp test, A_list condlist, A_exp elsee);
A_exp A_WhileExp(A_pos pos, A_exp test, A_exp body);
A_exp A_ForExp(A_pos pos, S_symbol var, A_exp lo, A_exp hi, A_exp body);
A_exp A_BreakExp(A_pos pos);
A_exp A_LetExp(A_pos pos, A_list decs, A_exp body);
A_exp A_ArrayExp(A_pos pos, S_symbol typ, A_exp size, A_exp init);
A_exp A_CreateviewExp(A_pos pos, S_symbol name, A_exp query, A_list keydecs, viewmode_t mode);
A_exp A_CreatestreamExp(A_pos pos, S_symbol name, A_exp query, S_symbol timekey);

/*************************************************************/
/*                 declarations                              */
/*************************************************************/

A_modelitem A_ModelItem(A_pos pos,
               S_symbol name,
               S_symbol uda,
               int window,
               A_list paramTables,
               A_list parameters);

A_flow A_Flow(A_pos pos,
             S_symbol name,
             A_list statements);

A_modelitem copyModelItem(A_modelitem m);
A_flow copyFlow(A_flow f);

A_dec copyDec(A_dec dec);
A_dec A_ModelTypeDec(A_pos pos, 
	       S_symbol name, 
	       A_list sharedTables, 
	       A_list modelItems,
               A_list flows);
A_dec A_ModelTypeDec(A_pos pos, 
	       S_symbol name, 
               S_symbol copyModel);
A_dec A_VarDec(A_pos pos, 
	       S_symbol var, 
	       S_symbol typ, 
	       A_exp init);
A_dec A_DynamicVarDec(A_pos pos,
                      S_symbol name,
                      S_symbol dynamic,
                      S_symbol rawname);
A_dec A_TabVarDec(A_pos pos, 
		  S_symbol tab, 
		  A_ty ty, 
		  A_index index,
		  int scope, 
		  A_exp init,
		  int isView=0);
A_dec A_StreamVarDec(A_pos pos, 
		     S_symbol tab, 
		     A_ty ty, 
		     S_symbol source, 
		     S_symbol target,
		     timekey_t tk,
		     S_symbol timekey,
		     A_exp init,
		     int port=-1);
A_dec A_TypeDec(A_pos pos, A_list type);

A_dec A_Fundec(A_pos pos, 
	       S_symbol name, 
	       A_list params, 
	       A_ty result,
	       A_list decs,
	       A_exp body);
A_dec A_Aggrdec(A_pos pos, aggr_t type, S_symbol name, A_list params, 
		A_ty result,
		A_list decs,
		A_exp init,
		A_exp iterate,
		A_exp expire,
		A_exp terminate, A_dec window);
A_dec A_Aggrdec(A_pos pos, aggr_t type, S_symbol name, A_list params, 
		A_ty result,
		char* c_global_decs,
                A_list decs,
                char* c_decs,
		char* init,
		char* iterate,
		char* expire,
		char* terminate, A_dec window);
A_dec A_Externdec(A_pos pos, 
		  S_symbol name, 
		  A_list params, 
		  A_ty result,
		  S_symbol externlib,
                  int size = 0,
		  S_symbol actual = (S_symbol)0);

A_index A_Index(A_pos pos,
		tabindex_t kind,
		A_list keydecs,
		S_symbol func = (S_symbol)0,
		int len=-1);

A_index copyAIndex(A_index a);
		
A_fieldExp A_FieldExo(S_symbol name,
		      char* exp);

A_ty A_NameTy(A_pos pos, S_symbol name);
A_ty A_RecordTy(A_pos pos, A_list record);
A_ty A_ArrayTy(A_pos pos, S_symbol array);
A_field A_Field(A_pos pos, S_symbol name, S_symbol typ, int size = 0);

A_namety A_Namety(S_symbol name, A_ty ty);
/*
  A_efield A_Efield(S_symbol name, A_exp exp);
  A_efield copyEfield(A_efield efield);
*/

/* SQL */
A_exp A_SqlVarExp(A_pos pos, S_symbol id1, S_symbol id2);
A_orderitem A_OrderItem(A_pos pos, A_exp exp, int order);
A_selectitem A_SelectItem(A_pos pos, A_exp exp, S_symbol id);
A_selectitem A_SelectItemStar(A_pos pos, S_symbol id);
A_selectitem A_SelectItemComplex(A_pos, A_exp exp, A_list list);
A_selectitem copySelectItem(A_selectitem si);

A_exp A_SqlOprExp(A_pos pos,
		  sqlopr_t kind,
		  int distinct,
		  A_list hxp_list,
		  A_list jtl_list,
		  A_list prd_list
		  );
int isValuesSqlOpr(A_exp p);
A_exp A_Runtask(A_pos pos,
                S_symbol modelName,
                S_symbol task,
                S_symbol source,
                A_win win,
                A_list params);
A_exp A_Select(A_pos pos,
	       int distinct,
	       A_list select_list,
	       A_list join_table_list,
	       A_exp where_cond,
	       A_list group_by_list,
	       A_exp having_cond,
	       A_list order_by_list
	       );
A_exp A_Select(A_pos pos,
	       int distinct,
	       A_list select_list,
	       A_list join_table_list,
	       A_list where_cond_list,
	       A_list group_by_list,
	       A_list having_cond_list,
	       A_list order_by_list
	       );
A_exp A_SqlLetExp(A_pos pos,
		  A_list decs,
		  A_exp query);

A_qun A_NameQun(A_pos pos,
		S_symbol name,
		S_symbol alias,
                bool dynamic=false,
                S_symbol rawname=(S_symbol)0);
A_qun A_FunctionQun(A_pos pos,
		    S_symbol name,
		    A_list args,
		    S_symbol alias);
A_qun A_WindowQun(A_pos pos,
		  S_symbol name,
		  A_range range,
		  S_symbol respectTo,
		  S_symbol alias);
A_qun A_QueryQun(A_pos pos,
		 S_symbol name,
		 A_exp query);
A_qun copyQun(A_qun q);
A_win A_Win(A_pos pos, 
	    A_list partition_list,
	    A_list orderby_list,
	    A_range r,
	    A_slide s);
A_win copyWin(A_win w);
void A_SetCallExpWindow(A_exp exp, 
			A_win win);
A_range A_Range(A_pos pos,
		int size,
		range_t type);
A_slide A_Slide(A_pos pos,
		int size,
		slide_t type);
A_range copyRange(A_range r);
A_slide copySlide(A_slide s);
A_tablecolumn A_TableColumn(A_pos pos,
			    A_field field, 
			    int notnull,
			    int primarykey);
A_orderitem A_OrderItem(A_pos pos, A_exp exp, int order);

void decomposeBoolExpr(A_exp boolexp, A_list list);

#endif



