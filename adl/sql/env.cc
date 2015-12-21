#include <sql/env.h>
#include <sql/adl_sys.h>
#include <sql/symbol.h>
#include <sql/table.h>
#include "list.h"
#include <stdio.h>
extern "C" {
#include <dbug/dbug.h>
}

S_table E_base_tenv;
S_table E_base_venv;

void EnvInit(void)
{
  A_list params;
  A_list t_fields;  

  E_base_venv = TAB_empty();
  E_base_tenv = TAB_empty();
  E_enventry te;

  /* string, int */
  S_enter(E_base_tenv, S_Symbol("char"), Ty_String());
  S_enter(E_base_tenv, S_Symbol("int"), Ty_Int());
  S_enter(E_base_tenv, S_Symbol("real"), Ty_Real());
  S_enter(E_base_tenv, S_Symbol("timestamp"), Ty_Timestamp());
  S_enter(E_base_tenv, S_Symbol("iext"), Ty_IExt());
  S_enter(E_base_tenv, S_Symbol("rext"), Ty_RExt());
  S_enter(E_base_tenv, S_Symbol("cext"), Ty_CExt());
  S_enter(E_base_tenv, S_Symbol("text"), Ty_TExt());

  /* stdout */
  S_enter(E_base_venv, S_Symbol("stdout"), E_VarEntry(S_Symbol("stdout"), Ty_Nil(), 0));

  /* SQLCODE is set to 1 if the previous SQL statement is successful. */
  te = E_VarEntry(S_Symbol("SQLCODE"), Ty_Int(), 0);
  te->u.var.iname = new_Symbol("_adl_sqlcode");
  S_enter(E_base_venv, S_Symbol("SQLCODE"), te);

  /* SQLRECID is the runtime recursive id for recursive aggregate
     routines. This variable is provided for debugging purpose. */
  te = E_VarEntry(S_Symbol("SQLRECID"), Ty_Int(), 0);
  te->u.var.iname = new_Symbol("_rec_id");
  S_enter(E_base_venv, S_Symbol("SQLRECID"), te);

  /* built-in aggregates */
  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("sum"), E_AggrEntry(S_Symbol("sum"), params, Ty_Int(), 0, 0, 1, 
						    AGGR_BUILTIN_SUM));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("avg"), E_AggrEntry(S_Symbol("avg"), params, Ty_Int(), 0, 0, 1,
						    AGGR_BUILTIN_AVG));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("count"), E_AggrEntry(S_Symbol("count"), params, Ty_Int(), 0, 0, 1,
						      AGGR_BUILTIN_COUNT));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("min"), E_AggrEntry(S_Symbol("min"), params, Ty_Int(), 0, 0, 1,
						    AGGR_BUILTIN_MIN));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("minr"), E_AggrEntry(S_Symbol("minr"), params, Ty_Real(), 0, 0, 1,
						    AGGR_BUILTIN_MINR));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("max"), E_AggrEntry(S_Symbol("max"), params, Ty_Int(), 0, 0, 1,
						    AGGR_BUILTIN_MAX));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("maxr"), E_AggrEntry(S_Symbol("maxr"), params, Ty_Real(), 0, 0, 1,
						    AGGR_BUILTIN_MAXR));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("sumr"), E_AggrEntry(S_Symbol("sumr"), params, Ty_Real(), 0, 0, 1,
						    AGGR_BUILTIN_SUMR));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("countr"), E_AggrEntry(S_Symbol("countr"), params, Ty_Real(), 0, 0, 1,
						    AGGR_BUILTIN_COUNTR));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_String());
  S_enter(E_base_venv, S_Symbol("XMLAgg"), E_AggrEntry(S_Symbol("XMLAgg"),params, Ty_String(), 0,0,1,
						    AGGR_BUILTIN_XA));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("var"), E_AggrEntry(S_Symbol("var"), params, Ty_Real(), 0, 0, 1,
  						    AGGR_BUILTIN_VAR));
  /* print, flush, getchar, ord, chr, size, substring, concat, not, exit */

  /* OID function. It has varied arguments. */
  /* S_enter(E_base_venv, S_Symbol("oid"), E_FunEntry(S_Symbol("oid"), (A_list )0, Ty_Int(), 1)); */

  /* vert function. It has varied arguments. */
  t_fields = A_List(0, A_FIELD);  
  appendElementList(t_fields,
             (nt_obj_t*)Ty_Field(new_Symbol("id"), Ty_Int(), sizeof(int)));
  appendElementList(t_fields,  
             (nt_obj_t*)Ty_Field(new_Symbol("col"), Ty_Int(), sizeof(int)));
  appendElementList(t_fields,
             (nt_obj_t*)Ty_Field(new_Symbol("val"), Ty_Real(), sizeof(double)));
  appendElementList(t_fields,  
             (nt_obj_t*)Ty_Field(new_Symbol("lbl"), Ty_Int(), sizeof(int)));
  S_enter(E_base_venv, S_Symbol("vert"), E_ExtEntry(S_Symbol("vert"), (A_list )0, Ty_Record(t_fields), (S_symbol)0, (S_symbol)0, 0, 1));

  /*fetchtbl table function, the return type is varied*/ 
  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_String());
  appendElementList(params, (nt_obj_t*)Ty_String());
  //t_fields dummy for testing only
  t_fields = A_List(0, A_FIELD);  
  appendElementList(t_fields,
             (nt_obj_t*)Ty_Field(new_Symbol("id"), Ty_Int(), sizeof(int)));
  S_enter(E_base_venv, S_Symbol("fetchtbl"), E_ExtEntry(S_Symbol("fetchtbl"), params, Ty_Record(t_fields) /*(Ty_ty)0*/, (S_symbol)0, (S_symbol)0, 0, 1));


  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_IExt());
  t_fields = A_List(0, A_FIELD);  
  appendElementList(t_fields,  
             (nt_obj_t*)Ty_Field(new_Symbol("col"), Ty_Int(), sizeof(int)));
  appendElementList(t_fields,
             (nt_obj_t*)Ty_Field(new_Symbol("val"), Ty_Int(), sizeof(int)));
  S_enter(E_base_venv, S_Symbol("iextvert"), E_ExtEntry(S_Symbol("iextvert"), params, Ty_Record(t_fields), (S_symbol)0, (S_symbol)0, 0, 1));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  t_fields = A_List(0, A_FIELD);  
  appendElementList(t_fields,  
             (nt_obj_t*)Ty_Field(new_Symbol("tid"), Ty_Int(), sizeof(int)));
  appendElementList(t_fields,
             (nt_obj_t*)Ty_Field(new_Symbol("val"), Ty_IExt(), sizeof(int)));
  S_enter(E_base_venv, S_Symbol("buildiext"), E_ExtEntry(S_Symbol("buildiext"), params, Ty_Record(t_fields), (S_symbol)0, (S_symbol)0, 0, 1));

  /* newiext function. It has varied arguments. */
  S_enter(E_base_venv, S_Symbol("newiext"), E_FunEntry(S_Symbol("newiext"),(A_list )0, Ty_IExt(), 1));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_IExt());
  S_enter(E_base_venv, S_Symbol("deleteiext"), E_FunEntry(S_Symbol("deleteiext"), params, Ty_Int()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_RExt());
  t_fields = A_List(0, A_FIELD);  
  appendElementList(t_fields,  
             (nt_obj_t*)Ty_Field(new_Symbol("col"), Ty_Int(), sizeof(int)));
  appendElementList(t_fields,
             (nt_obj_t*)Ty_Field(new_Symbol("val"), Ty_Real(), sizeof(int)));
  S_enter(E_base_venv, S_Symbol("rextvert"), E_ExtEntry(S_Symbol("rextvert"), params, Ty_Record(t_fields), (S_symbol)0, (S_symbol)0, 0, 1));

  /* newiext function. It has varied arguments. */
  S_enter(E_base_venv, S_Symbol("newrext"), E_FunEntry(S_Symbol("newrext"),(A_list )0, Ty_RExt(), 1));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_RExt());
  S_enter(E_base_venv, S_Symbol("deleterext"), E_FunEntry(S_Symbol("deleterext"), params, Ty_Int()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_CExt());
  t_fields = A_List(0, A_FIELD);  
  //NOTE: cextvert always returns string of length 100, 
          // not sure how to make this dynamic
  appendElementList(t_fields,  
             (nt_obj_t*)Ty_Field(new_Symbol("col"), Ty_Int(), sizeof(int)));
  appendElementList(t_fields,
             (nt_obj_t*)Ty_Field(new_Symbol("val"), Ty_String(), 100));
  S_enter(E_base_venv, S_Symbol("cextvert"), E_ExtEntry(S_Symbol("cextvert"), params, Ty_Record(t_fields), (S_symbol)0, (S_symbol)0, 0, 1));

  /* newiext function. It has varied arguments. */
  S_enter(E_base_venv, S_Symbol("newcext"), E_FunEntry(S_Symbol("newcext"),(A_list )0, Ty_CExt(), 1));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_CExt());
  S_enter(E_base_venv, S_Symbol("deletecext"), E_FunEntry(S_Symbol("deletecext"), params, Ty_Int()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_TExt());
  t_fields = A_List(0, A_FIELD);  
  appendElementList(t_fields,  
             (nt_obj_t*)Ty_Field(new_Symbol("col"), Ty_Int(), sizeof(int)));
  appendElementList(t_fields,
             (nt_obj_t*)Ty_Field(new_Symbol("val"), Ty_Timestamp(), sizeof(timestamp)));
  S_enter(E_base_venv, S_Symbol("textvert"), E_ExtEntry(S_Symbol("textvert"), params, Ty_Record(t_fields), (S_symbol)0, (S_symbol)0, 0, 1));

  /* newiext function. It has varied arguments. */
  S_enter(E_base_venv, S_Symbol("newtext"), E_FunEntry(S_Symbol("newtext"),(A_list )0, Ty_TExt(), 1));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_TExt());
  S_enter(E_base_venv, S_Symbol("deletetext"), E_FunEntry(S_Symbol("deletetext"), params, Ty_Int()));


  S_enter(E_base_venv, S_Symbol("XMLElement"), E_FunEntry(S_Symbol("XMLElement"),(A_list )0, Ty_String(), 1));

  S_enter(E_base_venv, S_Symbol("XMLAttributes"), E_FunEntry(S_Symbol("XMLAttributes"),(A_list )0, Ty_String(), 1));


  S_enter(E_base_venv, S_Symbol("oldest"), E_FunEntry(S_Symbol("oldest"),(A_list )0, (Ty_ty)0, 1));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_String());
  S_enter(E_base_venv, S_Symbol("print"), E_FunEntry(S_Symbol("print"), params, (Ty_ty)0));


  /* In AXL, srand() always return 0 */
  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("srand"), E_FunEntry(S_Symbol("srand"), params, Ty_Int()));

  S_enter(E_base_venv, S_Symbol("timeofday"), E_FunEntry(S_Symbol("timeofday"), (A_list )0, Ty_Timestamp()));  

  //S_enter(E_base_venv, S_Symbol("TUPLE_ID"), E_FunEntry(S_Symbol("TUPLE_ID"), (A_list )0, Ty_Int()));  

  S_enter(E_base_venv, S_Symbol("rand"), E_FunEntry(S_Symbol("rand"), (A_list )0, Ty_Real()));  

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("ceil"), E_FunEntry(S_Symbol("ceil"), params, Ty_Int()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("pow"), E_FunEntry(S_Symbol("pow"), params, Ty_Real()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("log"), E_FunEntry(S_Symbol("log"), params, Ty_Real()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("inttostring"), E_FunEntry(S_Symbol("inttostring"), params, Ty_String()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("realtostring"), E_FunEntry(S_Symbol("realtostring"), params, Ty_String()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_String());
  S_enter(E_base_venv, S_Symbol("stringtoint"), E_FunEntry(S_Symbol("stringtoint"), params, Ty_Int()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_String());
  S_enter(E_base_venv, S_Symbol("stringtoreal"), E_FunEntry(S_Symbol("stringtoreal"), params, Ty_Real()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("inttoreal"), E_FunEntry(S_Symbol("inttoreal"), params, Ty_Real()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Real());
  S_enter(E_base_venv, S_Symbol("realtoint"), E_FunEntry(S_Symbol("realtoint"), params, Ty_Int()));


  S_enter(E_base_venv, S_Symbol("flush"), E_FunEntry(S_Symbol("flush"), (A_list )0, (Ty_ty)0));  

  S_enter(E_base_venv, S_Symbol("getchar"), E_FunEntry(S_Symbol("getchar"), (A_list )0, Ty_String()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_String());
  S_enter(E_base_venv, S_Symbol("ord"), E_FunEntry(S_Symbol("ord"), params, Ty_Int()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("char"), E_FunEntry(S_Symbol("char"), params, Ty_String()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_String());
  S_enter(E_base_venv, S_Symbol("size"), E_FunEntry(S_Symbol("size"), params, Ty_Int()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_String());
  appendElementList(params, (nt_obj_t*)Ty_Int());
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("substring"), E_FunEntry(S_Symbol("substring"), params, Ty_String()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_String());
  appendElementList(params, (nt_obj_t*)Ty_String());
  S_enter(E_base_venv, S_Symbol("concat"), E_FunEntry(S_Symbol("concat"), params, Ty_String()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("gettimeval"), E_FunEntry(S_Symbol("gettimeval"), params, Ty_String()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("timetostring"), E_FunEntry(S_Symbol("timetostring"), params, Ty_String()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("not"), E_FunEntry(S_Symbol("not"), params, Ty_Int()));

  params = A_List();
  appendElementList(params, (nt_obj_t*)Ty_Int());
  S_enter(E_base_venv, S_Symbol("sqrt"), E_FunEntry(S_Symbol("sqrt"), params, Ty_Real()));


  S_enter(E_base_venv, S_Symbol("exit"), E_FunEntry(S_Symbol("exit"), (A_list )0, (Ty_ty)0));
}

E_enventry E_ModelTypeEntry(S_symbol key, A_list sharedtables, 
                            A_list modelitems, A_list flows)
{
  E_enventry e = (E_enventry)ntMalloc(sizeof(*e));
  e->kind = E_modelTypeEntry;
  e->key = key;

  e->u.modeltype.sharedtables = sharedtables;
  e->u.modeltype.modelitems = modelitems;
  e->u.modeltype.flows = flows;

  return e;
}

E_enventry E_DynamicEntry(S_symbol key, S_symbol name, S_symbol rawname)
{
  E_enventry e = (E_enventry)ntMalloc(sizeof(*e));
  e->kind = E_dynamicEntry;
  e->key = key;

  e->u.dynamic.table = name;
  e->u.dynamic.rawname = rawname;

  return e;
}

E_enventry E_VarEntry(S_symbol key, Ty_ty ty, int size, tabscope_t scope, 
		      A_index index, int haskey, int isStdout, int isBuffer)
{
  E_enventry e = (E_enventry)ntMalloc(sizeof(*e));
  e->kind = E_varEntry;
  e->key = key;

  e->u.var.ty = ty;
  e->u.var.size = size;
  e->u.var.scope = scope;
  e->u.var.source = (S_symbol)0;
  e->u.var.index = index;	
  e->u.var.haskey = haskey;
  e->u.var.iname = (S_symbol)0;
  e->u.var.sname = (S_symbol)0;
  e->u.var.inaggr = 0;
  e->u.var.firstKey = 0;
  e->u.var.isStdout = isStdout;
  e->u.var.isBuffer = isBuffer;
  return e;
}

E_enventry E_StreamEntry(S_symbol name, 
			 Ty_ty ty, 
			 int size,
			 S_symbol source,
			 S_symbol target,
			 timekey_t tk,
                         S_symbol timekey,
			 int isBuiltin,
			 int port)
{
  DBUG_ENTER("E_StreamEntry");
  E_enventry e = (E_enventry)ntMalloc(sizeof(*e));
  e->kind = E_streamEntry;
  
  e->u.stream.ty = ty;
  e->u.stream.size = size;
  e->u.stream.source = source;
  e->u.stream.target = target;
  e->u.stream.tk = tk;
  e->u.stream.timekey = timekey;

  e->u.stream.sname = (S_symbol)name;
  e->u.stream.isBuiltin = isBuiltin;
  e->u.stream.port = port;
  e->u.stream.orig = (S_symbol)name;
  DBUG_RETURN(e);
  return e;
}

E_enventry E_FunEntry(S_symbol key, A_list formals, Ty_ty result, int varied_args)
{
  E_enventry e = (E_enventry)ntMalloc(sizeof(*e));
  e->kind = E_funEntry;
  e->key = key;
  e->u.fun.formals = formals;
  e->u.fun.result = result;
  e->u.fun.aggr_routines = 0;
  e->u.fun.varied_args = varied_args;
  e->u.fun.inaggr = 0;
  e->u.fun.window = 0;
  return e;
}
E_enventry E_ExtEntry(S_symbol key, A_list formals, Ty_ty result, 
		      S_symbol actual, S_symbol externlib, int size,
                      int varied_args)
{
  E_enventry e = (E_enventry)ntMalloc(sizeof(*e));
  e->kind = E_extEntry;
  e->key = key;
  e->u.ext.formals = formals;
  e->u.ext.result = result;
  e->u.ext.size = size;
  e->u.ext.actual = actual;
  e->u.ext.externlib = externlib;
  e->u.ext.varied_args = varied_args;
  return e;
}
	E_enventry E_AggrEntry(S_symbol key,
                       A_list formals, 
		       Ty_ty result, 
		       int aggr_routines, 
                       int inaggr,
                       int implicit,
		       int builtin,
		       int default_win)
{
  E_enventry e = (E_enventry)ntMalloc(sizeof(*e));
  e->kind = E_aggrEntry;
  e->key = key;
  e->u.fun.formals = formals;
  e->u.fun.result = result;
  e->u.fun.aggr_routines = aggr_routines;
  e->u.fun.inaggr = inaggr;
  e->u.fun.implicit = implicit;
  e->u.fun.builtin = builtin;
  e->u.fun.window = 0;
  e->u.fun.default_win = default_win;
  return e;
}
/* 
 * setVarName() sets the internal name/structure name of a variable.
 *
 * See comments in env.h 
 */
err_t setVarName(S_table venv, S_symbol sym, int usage, S_symbol name)
{
  err_t rc = ERR_NONE;
  E_enventry te = (E_enventry)S_look(venv, sym);

  if (!te || te->kind != E_varEntry || te->u.var.ty->kind != Ty_record) {
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "setInternalName");
    goto exit;
  }

  if (usage == TAB_INAME) 
    te->u.var.iname = name;
  if (usage == TAB_SNAME) 
    te->u.var.sname = name;
 exit:
  return rc;
}

/* EnvCpy: Copy variable env */
void EnvCpy(E_enventry te, E_enventry x){
  switch (x->kind){
  case E_varEntry:
      te->key = x->key;
      te->u.var.iname = x->u.var.iname;
      te->u.var.inaggr = x->u.var.inaggr;
      te->u.var.scope = x->u.var.scope;
      te->u.var.source = x->u.var.source;
      te->u.var.index = x->u.var.index;
      te->u.var.haskey = x->u.var.haskey;
      te->u.var.firstKey = x->u.var.firstKey;
      te->u.var.isBuffer = x->u.var.isBuffer;
      break;
  case E_streamEntry:
    te->u.stream.sname = x->u.stream.sname;
    te->u.stream.source = x->u.stream.source;
    te->u.stream.target = x->u.stream.target;
    te->u.stream.ty = x->u.stream.ty;
    te->u.stream.size = x->u.stream.size;
  }

}
void displayFunEntry(E_enventry e){
  fprintf(stderr, "(");

      if (!A_ListEmpty(e->u.fun.formals)) {
	for (int i=0; i<e->u.fun.formals->length; i++) {
	  A_exp se = (A_exp)getNthElementList(e->u.fun.formals, i);
	  displayExp(se);
	  if (i<e->u.fun.formals->length-1) 
	    fprintf(stderr, ", ");
	}
      }
      fprintf(stderr, ")");

  if (e->u.fun.varied_args)
    fprintf(stderr, "(varied_args)");
  if (e->u.fun.builtin)
    fprintf(stderr, "(builtin)");
  fprintf(stderr,"\n");
};
