/*
** absyn.c - Abstract Syntax Functions. Most functions create an
** instance of an abstract syntax rule.  
*/

#include "util.h"
#include <sql/list.h>
#include <sql/symbol.h> /* symbol table data structures */
#include <sql/absyn.h>  /* abstract syntax data structures */
#include <sql/const.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

extern "C"{
#include <dbug/dbug.h>
}
#include <types.h>

void setUserName(const char* s) {
  __userName__[0] = '\0';
  strcpy(__userName__, s);
}

char* getUserName() {
  return __userName__;
}

void getUserNameie(char* un) {
  if(strcmp(__userName__, "__user__") != 0) {
    sprintf(un, "%s__", __userName__);
  }
  else {
    sprintf(un, "");
  }
}

void setModelName(const char* s) {
  __modelName__[0] = '\0';
  if(strcmp(s, "") != 0) {
    sprintf(__modelName__, "%s_", s);
  }
}

char* getModelName() {
  return __modelName__;
}

char* getJustName(char* fName) {
  char* localName = strdup(fName);
  char* underScore = strchr(localName, '_');
  if(underScore != NULL && underScore[1] == '_') {
    return underScore + 2;
  }
  if(underScore == NULL) {
    underScore = strchr(localName, '$');
    if(underScore != NULL)
      return underScore + 1;
  }

  return localName;
}

/*
void getModelPrepend(char* pre) {

  if(strcmp(getUserName(), "__user__") == 0) {
    sprintf(pre, getModelName());
    return;
  }
  sprintf(pre, "%s__%s", getUserName(), getModelName());
  return;
}
*/

void cpModelId(char* modelId) {
  sprintf(modelId, "%s", getModelName());
}

char *strlwr(char *s)
{
   if (s != NULL)
   {
      char *p;

      for (p = s; *p; ++p)
         *p = tolower(*p);
   }
   return s;
}

A_list copyAList(A_list list)
{
  register int	i;
  A_list new_list = (A_list)0;

  if (list) {
    new_list = (A_list )ntMalloc(sizeof(*new_list));

    new_list->pos = list->pos;
    new_list->type = list->type;
    new_list->pp = list->pp;
    new_list->ppt = list->ppt;

    new_list->elements = (nt_obj_t **)ntMalloc(list->size*sizeof(nt_obj_t*));
    new_list->size = list->size;
    new_list->length = list->length;
    new_list->collect_type = list->collect_type;

    for (i = 0; i < new_list->length; i++) {
      switch (new_list->type) {
      case A_EXP:
	new_list->elements[i] = (nt_obj_t*)copyExp((A_exp)list->elements[i]);
	((A_exp)(new_list->elements[i]))->pp = (void*)new_list;
	break;
	/*
	  case A_EFIELD:
	  new_list->elements[i] = (nt_obj_t*)copyEfield((A_efield)list->elements[i]);
	  ((A_efield)(new_list->elements[i]))->pp = (void*)new_list;
	  break;
	*/
      case A_FIELD:
	new_list->elements[i] = (nt_obj_t*)copyField((A_field)list->elements[i]);
	((A_field)(new_list->elements[i]))->pp = (void*)new_list;
        break;
      case A_TY:
	new_list->elements[i] = (nt_obj_t*)copyTy((Ty_ty)list->elements[i]);
	((Ty_ty)(new_list->elements[i]))->pp = (void*)new_list;
        break;
      case A_DEC:
	new_list->elements[i] = (nt_obj_t*)copyDec((A_dec)list->elements[i]);
	((A_dec)(new_list->elements[i]))->pp = (void*)new_list;
	break;
      case A_SELECT_ITEM:
	new_list->elements[i] = (nt_obj_t*)copySelectItem((A_selectitem)list->elements[i]);
	((A_selectitem)(new_list->elements[i]))->pp = (void*)new_list;	
	break;
      case A_QUN:
	new_list->elements[i] = (nt_obj_t*)copyQun((A_qun)list->elements[i]);
	((A_qun)(new_list->elements[i]))->pp = (void*)new_list;	
	break;
      case A_MODELITEM:
	new_list->elements[i] = (nt_obj_t*)copyModelItem((A_modelitem)list->elements[i]);
	((A_modelitem)(new_list->elements[i]))->pp = (void*)new_list;	
	break;
      case A_FLOW:
	new_list->elements[i] = (nt_obj_t*)copyFlow((A_flow)list->elements[i]);
	((A_flow)(new_list->elements[i]))->pp = (void*)new_list;	
	break;
      case A_SYMBOL:
	new_list->elements[i] = list->elements[i];
	break;
      case A_LIST:
	new_list->elements[i] = (nt_obj_t*)copyAList((A_list)list->elements[i]);
	((A_list)(new_list->elements[i]))->pp = (void*)new_list;	
	break;
      case A_ORDERITEM:
      case A_TABLE_COLUMN:
      default:
	printf("unknown type in copyList\n");
	exit(1);
      }
    }
  }
  return new_list;
}

void appendAList(A_list l, void *obj)
{
  appendElementList(l, (nt_obj_t*)obj);
  switch(l->type) {
  case A_EXP:
    ((A_exp)obj)->pp = l;
    ((A_exp)obj)->ppt = A_LIST;
    break;
  case A_DEC:
    ((A_dec)obj)->pp = l;
    ((A_dec)obj)->ppt = A_LIST;
    break;
    /*
      case A_EFIELD:
      ((A_efield)obj)->pp = l;
      ((A_efield)obj)->ppt = A_LIST;
      break;
    */
  case A_FIELD:
    ((A_field)obj)->pp = l;
    ((A_field)obj)->ppt = A_LIST;
    break;
  case A_TY:
    ((A_ty)obj)->pp = l;
    ((A_ty)obj)->ppt = A_LIST;
    break;
  case A_SYMBOL:
    break;
  case A_MODELITEM:
    ((A_modelitem)obj)->pp = l;
    ((A_modelitem)obj)->ppt = A_LIST;
    break;
  case A_FLOW:
    ((A_flow)obj)->pp = l;
    ((A_flow)obj)->ppt = A_LIST;
    break;
  case A_LIST:
    ((A_list)obj)->pp = l;
    ((A_list)obj)->ppt = A_LIST;
    break;
  case A_SELECT_ITEM:
    ((A_selectitem)obj)->pp = l;
    ((A_selectitem)obj)->ppt = A_LIST;
    break;
  case A_ORDERITEM:
    ((A_orderitem)obj)->pp = l;
    ((A_orderitem)obj)->ppt = A_LIST;
    break;
  case A_QUN:
    ((A_qun)obj)->pp = (void*)l;
    ((A_qun)obj)->ppt = A_LIST;
    break;
  case A_VAR:
    ((A_var)obj)->pp = (void*)l;
    ((A_var)obj)->ppt = A_LIST;
    break;
  default:
    fprintf(stderr, "unkown type in appendAList()\n");
    exit(1);
  }
}


/* REF */
A_ref A_Ref(A_pos pos, A_var var, S_symbol col)
{
  A_ref r = (A_ref)ntMalloc(sizeof(*r));
  r->kind = A_varRef;
  r->u.var = var;
  r->col = col;
  return r;
}
A_ref A_Ref(A_pos pos, A_ref ref, S_symbol col)
{
  S_symbol oidsym = S_Symbol("oid");
  A_ref r = ref;

  if (col == oidsym) {
    /* "ref->OID" is the same thing as "ref" */
  } else if (ref->col == oidsym) {
    /* replace the "OID" in "var->OID" with current "col" */
    r->col = col;
  } else {
    r = (A_ref)ntMalloc(sizeof(*r));
    r->kind = A_refRef;
    r->u.ref = ref;
    r->col = col;
  }
  return r;
}

A_exp A_RefExp(A_pos, A_ref ref)
{
  S_symbol oidsym = S_Symbol("oid");
  A_exp p;

  if (ref->col == oidsym) {
    /* "var->OID" is just "var" */
    p = A_VarExp(ref->pos, ref->u.var);
  } else {
    p = (A_exp)ntMalloc(sizeof(*p));

    p->pp = (void*)0;
    p->kind = A_refExp;
    p->u.ref = ref;
  }
  return p;
}
A_ref copyRef(A_ref r)
{
  switch(r->kind) {
  case A_varRef:
    return A_Ref(r->pos, r->u.var, r->col);
  case A_refRef:
    return A_Ref(r->pos, r->u.ref, r->col);
  }
}
int equalRef(A_ref ref1, A_ref ref2)
{
  int result = 0;
  
  if (ref1->kind == ref2->kind &&
      ref1->col == ref2->col) 
    {
      switch (ref1->kind) {
      case A_refRef:
	result = equalRef(ref1->u.ref, ref2->u.ref);
	break;
      case A_varRef:
	result = equalVar(ref1->u.var, ref2->u.var);
	break;
      }
    }

  return result;
}
void displayRef(A_ref ref)
{
  switch (ref->kind) {
  case A_refRef:
    displayRef(ref->u.ref);
    break;
  case A_varRef:
    displayVar(ref->u.var);
    break;
  }
  fprintf(stderr, "->%s", S_name(ref->col));
}

/* VAR */
/*
A_lval_spec A_FieldVarSpec(A_pos pos, S_symbol sym, A_lval_spec spec)
{
  A_lval_spec s = (A_lval_spec)ntMalloc(sizeof(*s));
  s->pos = pos;
  s->u.sym = sym;
  s->spec = spec;
  return s;
}
A_lval_spec A_SubscriptVarSpec(A_pos pos, A_exp exp, A_lval_spec spec)
{
  A_lval_spec s = (A_lval_spec)ntMalloc(sizeof(*s));
  s->pos = pos;
  s->u.exp = exp;
  s->spec = spec;
  return s;
}


A_var A_Var(A_var var, A_lval_spec s)
{
  A_var v;

  if (!s) {
    v = var;
  } else {
    if (s->kind == A_field_var_spec) {
      v = A_Var(A_FieldVar(s->pos, var, s->u.sym), s->spec);
    } else if (s->kind == A_subscript_var_spec) {
      v = A_Var(A_SubscriptVar(s->pos, var, s->u.exp), s->spec);
      s->u.exp->pp = (void*)v;
      s->u.exp->ppt = A_VAR;
    }
    var->pp = (void*)v;
    var->ppt = A_VAR;
  }

  v->pp = (void*)0;
  return v;
}
*/

A_var copyVar(A_var var)
{
  switch(var->kind) {
  case A_simpleVar:
    return A_SimpleVar(var->pos, var->u.simple);
  case A_refVar:
    return A_RefVar(var->pos, var->u.ref);
  case A_fieldVar:
    return A_FieldVar(var->pos, copyVar(var->u.field.var), var->u.field.sym);
  case A_subscriptVar:
    return A_SubscriptVar(var->pos, 
			  copyVar(var->u.subscript.var),
			  copyExp(var->u.subscript.exp));
  }
}

A_var A_RefVar(A_pos pos, A_ref ref)
{
  A_var p = (A_var)ntMalloc(sizeof(*p));
  p->kind=A_refVar;
  p->pos=pos;
  p->u.ref=ref;

  return p;
}
A_var A_SimpleVar(A_pos pos, S_symbol sym)
{
  A_var p = (A_var)ntMalloc(sizeof(*p));
  p->kind=A_simpleVar;
  p->pos=pos;
  p->u.simple=sym;

  return p;
}

A_var A_FieldVar(A_pos pos, A_var var, S_symbol sym)
{
  A_var p = (A_var)ntMalloc(sizeof(*p));
  p->kind=A_fieldVar;
  p->pos=pos;
  p->u.field.var=var;
  p->u.field.sym=sym;

  setParent(var, p, A_VAR);
  return p;
}

A_var A_SubscriptVar(A_pos pos, A_var var, A_exp exp)
{
  A_var p = (A_var)ntMalloc(sizeof(*p));
  p->kind=A_subscriptVar;
  p->pos=pos;
  p->u.subscript.var=var;
  p->u.subscript.exp=exp;

  setParent(var, p, A_VAR);
  setParent(exp, p, A_VAR);

  return p;
}

S_symbol getVarNameSuffix(A_var var)
{
  switch (var->kind) {
  case A_simpleVar:
    return var->u.simple;
  case A_fieldVar:
    return var->u.field.sym;
  case A_subscriptVar:
    return 0;
  }
}

A_exp A_VarExp(A_pos pos, A_var var)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_varExp;
  p->pos=pos;
  p->u.var=var;

  setParent(var, p, A_EXP);

  return p;
}

A_exp A_NilExp(A_pos pos)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_nilExp;
  p->pos=pos;
  return p;
}

A_exp A_IntExp(A_pos pos, int i)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_intExp;
  p->pos=pos;
  p->u.intt=i;
  return p;
}

A_exp A_RealExp(A_pos pos, double r)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_realExp;
  p->pos=pos;
  p->u.realt=r;
  return p;
}

A_exp A_StringExp(A_pos pos, char *s)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_stringExp;
  p->pos=pos;
  p->u.string=s;
  return p;
}

A_exp A_TimestampExp(A_pos pos, char* timestamp)
{
  struct timeval* t;
  struct tm* tm = (struct tm*)ntMalloc(sizeof(struct tm));

  if(tm == NULL)
    return A_StringExp(pos, timestamp);

  t = (struct timeval*)ntMalloc(sizeof(struct timeval));

  char* ret = strptime(timestamp, TIMESTAMP_FORMAT, tm);

  if(ret == NULL)
  {
    return A_StringExp(pos, timestamp);
  }

  t->tv_sec = mktime(tm);
  t->tv_usec = (long)0;

  /*For daylight savings */
  if(tm->tm_isdst > 0)
    t->tv_sec = t->tv_sec - 3600;

  free(tm);
  
  if(ret[0] == '.')
  {
    ret ++;
    if(strlen(ret) > 6)
    {
      ret[6] = '\0';
      t->tv_usec = atol(ret);
    }
    else if(strlen(ret) == 6)
      t->tv_usec = atol(ret);
    else
    {
      t->tv_usec = (long)(atol(ret) * pow(10, (6-strlen(ret))));
    }
  }

  return A_TimestampExp(pos, t);
}

A_exp A_TimestampExp(A_pos pos, struct timeval* t)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_timestampExp;
  p->pos=pos;
  p->u.timestamp=t;
  return p;
}

/* Abstract syntax for func/aggr invocation. In the case that
 * func/aggr returns complex data type, "member" is a field name in
 * the record type. */
A_exp A_CallExp(A_pos pos, S_symbol func, A_list args, S_symbol member,
                char* init)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_callExp;
  p->pos=pos;
  p->u.call.func=func;
  p->u.call.args=args;
  p->u.call.member=member;
  p->u.call.shared=0;
  p->u.call.win = (A_win)0;
  p->u.call.init = init;
  setParent(args, p, A_EXP);
  return p;
}

A_exp A_OpExp(A_pos pos, A_oper oper, A_exp left, A_exp right)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_opExp;
  p->pos=pos;
  p->u.op.oper=oper;

  setParent(left, p, A_EXP);
  setParent(right, p, A_EXP);

  p->u.op.left=left;
  p->u.op.right=right;
  return p;
}

A_exp A_RecordExp(A_pos pos, S_symbol typ, A_list fields)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_recordExp;
  p->pos=pos;
  p->u.record.typ=typ;
  p->u.record.fields=fields;

  setParent(fields, p, A_EXP);

  return p;
}

A_exp A_SeqExp(A_pos pos, A_list seq)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_seqExp;
  p->pos=pos;
  p->u.seq=seq;

  setParent(seq, p, A_EXP);

  return p;
}

A_exp A_AssignExp(A_pos pos, A_var var, A_exp exp)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_assignExp;
  p->pos=pos;
  p->u.assign.var=var;
  p->u.assign.exp=exp;

  setParent(var, p, A_EXP);
  setParent(exp, p, A_EXP);

  return p;
}

A_exp A_IfExp(A_pos pos, A_exp test, A_exp then, A_exp elsee)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_ifExp;
  p->pos=pos;
  p->u.iff.test=test;
  p->u.iff.then=then;
  p->u.iff.elsee=elsee;
  return p;
}

A_exp A_WhileExp(A_pos pos, A_exp test, A_exp body)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_whileExp;
  p->pos=pos;
  p->u.whilee.test=test;
  p->u.whilee.body=body;
  return p;
}

A_exp A_ForExp(A_pos pos, S_symbol var, A_exp lo, A_exp hi, A_exp body)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_forExp;
  p->pos=pos;
  p->u.forr.var=var;
  p->u.forr.lo=lo;
  p->u.forr.hi=hi;
  p->u.forr.body=body;
  p->u.forr.escape=TRUE;
  return p;
}

A_exp A_BreakExp(A_pos pos)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_breakExp;
  p->pos=pos;
  return p;
}

A_exp A_LetExp(A_pos pos, A_list decs, A_exp body)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_letExp;
  p->pos=pos;
  p->u.let.decs=decs;
  p->u.let.body=body;

  setParent(decs, p, A_EXP);
  setParent(body, p, A_EXP);

  return p;
}

A_exp A_ArrayExp(A_pos pos, S_symbol typ, A_exp size, A_exp init)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_arrayExp;
  p->pos=pos;
  p->u.array.typ=typ;
  p->u.array.size=size;
  p->u.array.init=init;
  return p;
}


A_exp A_CreateviewExp(A_pos pos, S_symbol name, A_exp query, 
		      A_list keydecs, viewmode_t mode)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind = A_createviewExp;
  p->pos = pos;
  p->u.createview.name = name;
  p->u.createview.query = query;
  p->u.createview.keydecs = keydecs;
  p->u.createview.mode = mode;
  return p;
}

A_exp A_CreatestreamExp(A_pos pos, S_symbol name, A_exp query, S_symbol timekey)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind = A_createstreamExp;
  p->pos = pos;
  p->u.createstream.name = name;
  p->u.createstream.query = query;
  p->u.createstream.timekey = timekey;
  return p;
}

/*A_dec A_FunctionDec(A_pos pos, A_list function)
{
  A_dec p = (A_dec)ntMalloc(sizeof(*p));
  p->kind=A_functionDec;
  p->pos=pos;
  p->u.function=function;
  return p;
}

A_dec A_AggregateDec(A_pos pos, A_list aggregate)
{
  A_dec p = (A_dec)ntMalloc(sizeof(*p));
  p->kind=A_aggregateDec;
  p->pos=pos;
  p->u.aggregate=aggregate;
  return p;
}*/

A_dec A_VarDec(A_pos pos, S_symbol var, S_symbol typ, A_exp init)
{
  A_dec p = (A_dec)ntMalloc(sizeof(*p));
  p->kind=A_varDec;
  p->pos=pos;
  p->name=var;
  p->u.var.typ=typ;
  p->u.var.init=init;
  p->u.var.escape=TRUE;

  setParent(init, p, A_DEC);

  return p;
}
A_dec copyVarDec(A_dec d)
{
  return A_VarDec(d->pos, d->name, d->u.var.typ, copyExp(d->u.var.init));
}
A_dec A_TypeDec(A_pos pos, A_list type)
{
  A_dec p = (A_dec)ntMalloc(sizeof(*p));
  p->kind=A_typeDec;
  p->pos=pos;
  p->u.type=type;

  setParent(type, p, A_DEC);

  return p;
}
A_dec copyTypeDec(A_dec d)
{
  printf("copyTypeDec not implemented\n");
  exit(1);
}
A_ty A_NameTy(A_pos pos, S_symbol name)
{
  A_ty p = (A_ty)ntMalloc(sizeof(*p));
  p->kind=A_nameTy;
  p->pos=pos;
  p->u.name=name;
  return p;
}
A_ty A_RecordTy(A_pos pos, A_list record)
{
  A_ty p = (A_ty)ntMalloc(sizeof(*p));
  p->kind=A_recordTy;
  p->pos=pos;
  p->u.record=record;
  
  setParent(record, p, A_TY);

  return p;
}
A_ty A_ArrayTy(A_pos pos, S_symbol array)
{
  A_ty p = (A_ty)ntMalloc(sizeof(*p));
  p->kind=A_arrayTy;
  p->pos=pos;
  p->u.array=array;
  return p;
}
A_ty copyTy(A_ty t)
{
  A_ty n;
  switch(t->kind) {
  case A_nameTy:
    n = A_NameTy(t->pos, t->u.name);
    break;
  case A_recordTy:
    printf("copyTy of A_recordTy type not implemented\n");
    exit(1);
    break;
  case A_arrayTy:
    n = A_ArrayTy(t->pos, t->u.array);
    break;
  }
  return n;
}

A_field A_Field(A_pos pos, S_symbol name, S_symbol typ, int size)
{
  A_field p = (A_field)ntMalloc(sizeof(*p));
  p->pos=pos;
  p->size=size;
  p->name=name;
  p->typ=typ;
  p->escape=TRUE;
  return p;
}

A_dec A_Fundec(A_pos pos, S_symbol name, 
	       A_list params, A_ty result,
	       A_list decs, A_exp body)
{
  A_dec p = (A_dec)ntMalloc(sizeof(*p));
  p->kind = A_functionDec;
  p->pos = pos;
  p->name = name;
  p->u.fun.params=params;
  p->u.fun.result=result;
  p->u.fun.decs = decs;
  p->u.fun.body=body;
  return p;
}

A_dec copyFunctionDec(A_dec d)
{
  return A_Fundec(d->pos, d->name, 
		  copyAList(d->u.fun.params),
		  copyTy(d->u.fun.result),
		  copyAList(d->u.aggr.decs),
		  copyExp(d->u.fun.body));
}
A_dec A_Externdec(A_pos pos, 
		  S_symbol name, 
		  A_list params, 
		  A_ty result,
		  S_symbol externlib,
                  int size,
		  S_symbol actual)
{
  A_dec p = (A_dec)ntMalloc(sizeof(*p));
  p->kind = A_externDec;
  p->pos = pos;
  p->name = name;
  p->u.ext.params=params;
  p->u.ext.result=result;
  p->u.ext.externlib=externlib;
  p->u.ext.size=size;
  if(actual)
    p->u.ext.actual=actual;
  else
    p->u.ext.actual = name;

  setParent(params, p, A_DEC);
  setParent(result, p, A_DEC);

  return p;
}
A_dec copyExternDec(A_dec d)
{
  return A_Externdec(d->pos,
		     d->name,
		     copyAList(d->u.ext.params),
		     copyTy(d->u.ext.result),
		     d->u.ext.externlib,
                     d->u.ext.size);
}
A_dec A_Aggrdec(A_pos pos, aggr_t type, S_symbol name, A_list params, 
		A_ty result,
		A_list decs,
		A_exp init,
		A_exp iterate,
		A_exp expire,
		A_exp terminate,
                A_dec window)
{
  A_dec p = (A_dec)ntMalloc(sizeof(*p));
  p->kind = A_aggregateDec;
  p->pos = pos;
  p->name = name;
  p->u.aggr.type = type;
  p->u.aggr.params=params;
  p->u.aggr.result=result;
  p->u.aggr.decs = decs;
  p->u.aggr.is_c_aggr = 0;

  p->u.aggr.terminate = terminate;
  p->u.aggr.window = window;
  if(type != A_window) p->u.aggr.create_windowed = 1;
  if (iterate==(A_exp)1) {
    p->u.aggr.iterate = copyExp(terminate);
  } else {
    p->u.aggr.iterate = iterate;
  }

  if (init==(A_exp)1) {
    p->u.aggr.init = iterate; //copyExp(iterate);
  } else {
    p->u.aggr.init = init;
  }

  p->u.aggr.expire = expire;

  setParent(params, p, A_DEC);
  setParent(p->u.aggr.init, p, A_DEC);
  setParent(p->u.aggr.iterate, p, A_DEC);
  setParent(p->u.aggr.terminate, p, A_DEC);

  return p;
}

A_dec A_Aggrdec(A_pos pos, aggr_t type, S_symbol name, A_list params, 
		A_ty result,
                char* c_global_decs,
		A_list decs,
                char* c_decs,
		char* init,
		char* iterate,
		char* expire,
		char* terminate,
                A_dec window)
{
  A_dec p = (A_dec)ntMalloc(sizeof(*p));
  p->kind = A_aggregateDec;
  p->pos = pos;
  p->name = name;
  p->u.aggr.type = type;
  p->u.aggr.params=params;
  p->u.aggr.result=result;
  p->u.aggr.decs = decs;
  p->u.aggr.is_c_aggr = 1;

  if(c_global_decs) {
    p->u.aggr.c_global_decs = strdup(c_global_decs);
  }
  else {
    p->u.aggr.c_global_decs = c_global_decs;
  }

  if(c_decs) {
    p->u.aggr.c_decs = strdup(c_decs);
  }
  else {
    p->u.aggr.c_decs = c_decs;
  }

  p->u.aggr.window = window;
  if(type != A_window) p->u.aggr.create_windowed = 1;

  if (init==(char*)1) {
    init = iterate;
  }

  if(init != NULL) {
    p->u.aggr.init=(A_exp)2;
    p->u.aggr.c_init = init;
  }
  else {
    p->u.aggr.init=(A_exp)1;
  }

  if(iterate == NULL) {
    p->u.aggr.c_iter = iterate;
  }
  else {
    p->u.aggr.iterate=(A_exp)1;
    p->u.aggr.c_iter = strdup(iterate);
  }

  if(expire == NULL) {
    p->u.aggr.c_expire = expire;
  }
  else {
    p->u.aggr.expire=(A_exp)1;
    p->u.aggr.c_expire = strdup(expire);
  }

  if(terminate == NULL) {
    p->u.aggr.c_term = terminate;
  }
  else {
    p->u.aggr.terminate=(A_exp)1;
    p->u.aggr.c_term = strdup(terminate);
  }

  return p;
}

A_dec copyAggregateDec(A_dec d)
{
  if(d->u.aggr.is_c_aggr == 0)
    return A_Aggrdec(d->pos,
                   d->u.aggr.type,
		   d->name,
		   copyAList(d->u.aggr.params),
		   copyTy(d->u.aggr.result),
		   copyAList(d->u.aggr.decs),
		   copyExp(d->u.aggr.init),
		   copyExp(d->u.aggr.iterate),
		   copyExp(d->u.aggr.expire),
		   copyExp(d->u.aggr.terminate),
                   copyDec(d->u.aggr.window));
  else
    return A_Aggrdec(d->pos,
                   d->u.aggr.type,
		   d->name,
		   copyAList(d->u.aggr.params),
		   copyTy(d->u.aggr.result),
		   d->u.aggr.c_global_decs,
		   copyAList(d->u.aggr.decs),
		   d->u.aggr.c_decs,
		   d->u.aggr.c_init,
		   d->u.aggr.c_iter,
		   d->u.aggr.c_expire,
		   d->u.aggr.c_term,
                   copyDec(d->u.aggr.window));
}

A_namety A_Namety(S_symbol name, A_ty ty)
{
  A_namety p = (A_namety)ntMalloc(sizeof(*p));
  p->name=name;
  p->ty=ty;
  return p;
}


/*
A_efield A_Efield(S_symbol name, A_exp exp)
{
  A_efield p = (A_efield)ntMalloc(sizeof(*p));
  p->name=name;
  p->exp=exp;
  return p;
}
A_efield copyEfield(A_efield efield)
{
  return A_Efield(efield->name, copyExp(efield->exp));
}
*/
/* SQL */
A_exp 
A_SqlVarExp(A_pos pos, S_symbol id1, S_symbol id2)
{
  A_exp e;
  
  if (id1)
    e = A_VarExp(pos, A_FieldVar(pos,
				 A_SimpleVar(pos, id1),
				 id2));
  else 
    e = A_VarExp(pos,
		 A_SimpleVar(pos, id2));

  return e;
}
A_selectitem A_SelectItem(A_pos pos, A_exp exp, S_symbol id)
{
  A_selectitem s = (A_selectitem)ntMalloc(sizeof(*s));
  s->kind = SIMPLE_ITEM;
  s->pos = pos;

  setParent(exp, s, A_SELECT_ITEM);

  s->u.s.exp = exp;
  s->u.s.alias =id; 

  return s;
}
A_selectitem A_SelectItemStar(A_pos pos, S_symbol id)
{
  A_selectitem s = (A_selectitem)ntMalloc(sizeof(*s));
  s->kind = STAR_ITEM;
  s->pos = pos;
  s->u.table = id;
  return s;
}
A_selectitem A_SelectItemComplex(A_pos pos, A_exp e, A_list alist)
{
  A_selectitem s = (A_selectitem)ntMalloc(sizeof(*s));
  s->kind = COMPLEX_ITEM;
  s->pos = pos;
  s->u.c.exp = e;
  s->u.c.aliaslist = alist;

  setParent(e, s, A_SELECT_ITEM);

  return s;
}
A_selectitem copySelectItem(A_selectitem si)
{
  A_selectitem n;
  switch (si->kind) {
  case SIMPLE_ITEM:
    n = A_SelectItem(si->pos, copyExp(si->u.s.exp), si->u.s.alias);
    break;
  case COMPLEX_ITEM:
    n = A_SelectItemComplex(si->pos, 
			    copyExp(si->u.c.exp),
			    copyAList(si->u.c.aliaslist));
    break;
  case STAR_ITEM:
    n = A_SelectItemStar(si->pos, si->u.table);
    break;
  }
  return n;
}

A_exp A_SetOpr(A_pos pos,
	       int set_oper,
	       int distinct,
	       A_list operand_list)
{
  return (A_exp)0;
}

A_exp A_SqlLetExp(A_pos pos,
		  A_list decs,
		  A_exp query)
{
  return (A_exp)0;
}

/*************************************************************
                             UDA & WIN
*************************************************************/
A_win A_Win(A_pos pos,
	    A_list partition_list,
	    A_list orderby_list,
	    A_range r,
	    A_slide s)
{
  A_win w = (A_win)ntMalloc(sizeof(*w));
  w->pos = pos;
  w->partition_list = partition_list;
  w->order_by_list = orderby_list;
  w->range = r;
  w->slide = s;
  return w;
}
A_slide A_Slide(A_pos pos,
		int size,
		slide_t type){
  A_slide s = (A_slide)ntMalloc(sizeof(*s));
  s->pos = pos;
  s->size = size;
  s->type = type;
  return s;
}
A_slide copySlide(A_slide s)
{
  if(!s) return NULL;
  return A_Slide(s->pos, s->size, s->type);
}

A_win copyWin(A_win w)
{
    if(!w) return NULL;
    return A_Win(w->pos, 
	    copyAList(w->partition_list),
	    copyAList(w->order_by_list),
	    copyRange(w->range),
            copySlide(w->slide));
}
void A_SetCallExpWindow(A_exp exp, A_win win)
{
  exp->u.call.win = win;
}
A_range A_Range(A_pos pos, int size, range_t type)
{
  A_range r = (A_range)ntMalloc(sizeof(*r));
  r->pos = pos;
  r->size = size;
  r->type = type;
  return r;
}
A_range copyRange(A_range r)
{
  if(!r) return NULL;
  return A_Range(r->pos, r->size, r->type);
}
/*************************************************************
                             QUN
*************************************************************/
A_qun A_NameQun(A_pos pos,
		S_symbol name,
		S_symbol alias,
                bool dynamic,
                S_symbol rawname)
{
  A_qun q = (A_qun)ntMalloc(sizeof(*q));
  //  q->win = NULL;
  q->kind = QUN_NAME;
  q->pos = pos;
  q->alias = alias;
  q->u.table.name = name;
  q->u.table.dynamic = dynamic;
  q->u.table.rawname = rawname;
  return q;
}

A_qun A_FunctionQun(A_pos pos,
		    S_symbol name,
		    A_list args,
		    S_symbol alias)
{
  A_qun q = (A_qun)ntMalloc(sizeof(*q));
  //  q->win = NULL;
  q->kind = QUN_FUNCTION;
  q->pos = pos;
  q->alias = alias;
  q->u.function.name = name;
  q->u.function.args = args;

  setParent(args, q, A_QUN);

  return q;
}

A_qun A_WindowQun(A_pos pos,
		  S_symbol name,
		  A_range range,
		  S_symbol respectTo,
		  S_symbol alias)
{
  A_qun q = (A_qun)ntMalloc(sizeof(*q));
  //  q->win = NULL;
  q->kind = QUN_WINDOW;
  q->pos = pos;
  q->alias = alias;
  q->u.window.name = name;
  q->u.window.range = range;
  q->u.window.respectTo = respectTo;

  // setParent(range, q, A_QUN); not quite sure if we need this for window

  return q;
}
A_qun A_QueryQun(A_pos pos,
		 S_symbol alias,
		 A_exp query)
{
  A_qun q = (A_qun)ntMalloc(sizeof(*q));
  //  q->win = NULL;
  q->kind = QUN_QUERY;
  q->pos = pos;
  q->alias = alias;
  q->u.query = query;

  setParent(query, q, A_QUN);

  return q;
}
A_qun copyQun(A_qun q)
{
  A_qun n;
  switch (q->kind) {
  case QUN_NAME:
    n = A_NameQun(q->pos, q->u.table.name, q->alias, q->u.table.dynamic);
    break;
  case QUN_FUNCTION:
    n = A_FunctionQun(q->pos, q->u.function.name, 
		      copyAList(q->u.function.args),
		      q->alias);
    break;
  case QUN_WINDOW:
    n = A_WindowQun(q->pos, q->u.window.name,
		    copyRange(q->u.window.range),
		    q->u.window.respectTo, q->alias);
    break;
  case QUN_QUERY:
    n = A_QueryQun(q->pos, 
		   q->alias,
		   copyExp(q->u.query));
    break;
  }
  //  A_SetQunWindow(n, copyWin(q->win));
  return n;
}
/*************************************************************

*************************************************************/
A_tablecolumn A_TableColumn(A_pos pos,
			    A_field field, 
			    int notnull,
			    int primarykey)
{
  return (A_tablecolumn)0;
}
A_exp A_CaseExp(A_pos pos, A_exp exp, A_list whenlist, A_exp elsee)
{
  return (A_exp)0;
}
A_orderitem A_OrderItem(A_pos pos, A_exp exp, int order)
{
  A_orderitem oi = (A_orderitem)ntMalloc(sizeof(*oi));
  oi->dir = order;
  oi->exp = exp;
  oi->pos = pos;

  setParent(exp, oi, A_ORDERITEM);

  return oi;
}


A_index A_Index(A_pos pos,
		tabindex_t kind,
		A_list keydecs,
		S_symbol func,
		int len) {
  A_index i = (A_index)ntMalloc(sizeof(*i));
  /* TAB_BTREE, TAB_RTREE */
  i->kind = kind;
  i->pos = pos;
  i->func = func;
  i->keydecs = keydecs;
  i->len = len;
 
  return i;
}

A_index copyAIndex(A_index a) {
  if(a == (A_index)0) {
    return (A_index)0;
  }
  A_index i = (A_index)ntMalloc(sizeof(*i));
  /* TAB_BTREE, TAB_RTREE */
  i->kind = a->kind;
  i->pos = a->pos;
  i->func = a->func;
  i->keydecs = copyAList(a->keydecs);
  i->len = a->len;

  return i;
}

A_fieldExp A_FieldExp(S_symbol name,
		      char* exp) {
  A_fieldExp f = (A_fieldExp)ntMalloc(sizeof(*f));
  f->name = name;
  f->exp = exp;
  return f;
}

A_flow A_Flow(A_pos pos,
              S_symbol name,
              A_list statements) {
  A_flow f = (A_flow)ntMalloc(sizeof(*f));
  f->pos = pos;
  f->name = name;
  f->statements = statements;

  setParent(statements, f, A_EXP);

  return f;
}

A_modelitem A_ModelItem(A_pos pos,
               S_symbol name,
               S_symbol uda,
               int window,
               A_list paramTables,
               A_list parameters) {
  A_modelitem m = (A_modelitem)ntMalloc(sizeof(*m));
  m->pos = pos;
  m->name = name;
  m->uda = uda;
  m->window = window;
  m->paramtables = paramTables;
  m->allowedparams = parameters;
  
  setParent(paramTables, m, A_SYMBOL);
  setParent(parameters, m, A_FIELD);

  return m;
}

A_dec A_ModelTypeDec(A_pos pos,
               S_symbol name,
               S_symbol copyModel) {
  A_dec d = (A_dec)ntMalloc(sizeof(*d));
  d->kind = A_modelTypeDec;
  d->pos = pos;
  d->name = name;
  d->u.modeltype.copyModel = copyModel;
 
  return d;
}

A_dec A_ModelTypeDec(A_pos pos,
               S_symbol name,
               A_list sharedTables,
               A_list modelItems,
               A_list flows) {
  A_dec d = (A_dec)ntMalloc(sizeof(*d));
  d->kind = A_modelTypeDec;
  d->pos = pos;
  d->name = name;
  d->u.modeltype.sharedtables = sharedTables;
  d->u.modeltype.modelitems = modelItems;
  d->u.modeltype.flows = flows;
  d->u.modeltype.copyModel = (S_symbol)0;
 
  setParent(sharedTables, d, A_SYMBOL);
  setParent(modelItems, d, A_MODELITEM);
  setParent(flows, d, A_FLOW);

  return d;
}

A_dec A_DynamicVarDec(A_pos pos,
                      S_symbol name,
                      S_symbol dynamic,
                      S_symbol rawname) {
  A_dec d = (A_dec)ntMalloc(sizeof(*d));
  d->kind = A_dynamicVarDec;
  d->pos = pos;
  d->name = name;
  d->u.dynamic.table = dynamic;
  d->u.dynamic.rawname = rawname;

  return d;
}

A_dec A_TabVarDec(A_pos pos, 
		  S_symbol tab, 
		  A_ty ty, 
		  A_index index,
		  int scope, 
		  A_exp init,
		  int isView)
{
  A_dec d = (A_dec)ntMalloc(sizeof(*d));
  d->kind = A_tabVarDec;
  d->pos = pos;
  d->name = tab;
  d->u.tabvar.ty = ty;
  d->u.tabvar.init = init;
  d->u.tabvar.isView = isView;

  d->u.tabvar.oid_enabled = 0;
  d->u.tabvar.source = (S_symbol)0;
  switch (scope) {
  case 0:			// LOCAL
    d->u.tabvar.scope = TAB_LOCAL;
    break;
  case 1:			// MEMORY
    d->u.tabvar.scope = TAB_MEMORY;
    break;
  case 2:			// MEMORY OID
    d->u.tabvar.scope = TAB_MEMORY;
    d->u.tabvar.oid_enabled = 1;
    break;
  default:			// SOURCE
    d->u.tabvar.scope = TAB_GLOBAL;
    d->u.tabvar.source = (S_symbol)scope;
    break;
  }

  d->u.tabvar.index = index;

  setParent(ty, d, A_DEC);
  //setParent(keydecs, d, A_DEC);
  setParent(init, d, A_DEC);

  return d;
}
A_dec A_StreamVarDec(A_pos pos, 
		     S_symbol tab, 
		     A_ty ty, 
		     S_symbol source, 
		     S_symbol target,
		     timekey_t tk,
		     S_symbol timekey,
		     A_exp init,
		     int port)
{
  DBUG_ENTER("A_StreamVarDec");
  A_dec d = (A_dec)ntMalloc(sizeof(*d));
  d->kind = A_streamVarDec;
  d->pos = pos;
  d->name = tab;
  d->u.streamvar.ty = ty;
  d->u.streamvar.init = init;

  d->u.streamvar.source = (S_symbol)(source);
  d->u.streamvar.target = (S_symbol)(target);
  d->u.streamvar.tk = tk;
  d->u.streamvar.timekey=timekey;

  if(port > 0) {
    d->u.streamvar.isBuiltin = 1;
    d->u.streamvar.port = port;
  }
  else {
    d->u.streamvar.isBuiltin = 0;
    d->u.streamvar.port = -1;
  }

  setParent(ty, d, A_DEC);
  setParent(init, d, A_DEC);

  DBUG_RETURN(d);
  return d;
}

A_flow copyFlow(A_flow f) {
  return A_Flow(f->pos, f->name, copyAList(f->statements));
}

A_modelitem copyModelItem(A_modelitem m) {
  return A_ModelItem(m->pos, m->name, m->uda, m->window,
                     copyAList(m->paramtables), copyAList(m->allowedparams));
}

A_dec copyTabVarDec(A_dec d)
{
  return A_TabVarDec(d->pos, d->name, copyTy(d->u.tabvar.ty),
		     copyAIndex(d->u.tabvar.index),
		     //copyAList(d->u.tabvar.keydecs),
		     (int)d->u.tabvar.scope,
		     copyExp(d->u.tabvar.init));
}
A_dec copyModelTypeDec(A_dec d) 
{
  return A_ModelTypeDec(d->pos, d->name, 
                        copyAList(d->u.modeltype.sharedtables), 
                        copyAList(d->u.modeltype.modelitems),
                        copyAList(d->u.modeltype.flows));
}
A_dec copyDec(A_dec d) 
{
  A_dec n;
  switch (d->kind) {
  case A_varDec:
    n = copyVarDec(d);
    break;
  case A_functionDec: 
    n = copyFunctionDec(d);
    break;
  case A_aggregateDec: 
    n = copyAggregateDec(d);
    break;
  case A_externDec:
    n = copyExternDec(d);
    break;
  case A_tabVarDec: 
    n = copyTabVarDec(d);
  case A_typeDec:
    n = copyTypeDec(d);
    break;
  }
  return n;
}

/* Function decomposeBoolExpr() breaks a boolexp into a conjunction
   list of exps. */
void decomposeBoolExpr(A_exp boolexp, 
		      A_list list)
{
  if (boolexp->kind == A_opExp &&
      boolexp->u.op.oper == A_andOp) {
    decomposeBoolExpr(boolexp->u.op.left, list);
    decomposeBoolExpr(boolexp->u.op.right, list);
  } else {
    appendAList(list, (void*)boolexp);
    /*    appendElementList(list, (nt_obj_t*)boolexp);*/
  }
}

A_exp A_Runtask(A_pos pos,
                S_symbol modelName,
                S_symbol task,
                S_symbol source,
                A_win win,
                A_list params) {
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_runtaskExp;
  p->pos=pos;

  p->u.runtask.modelName = modelName;
  p->u.runtask.task = task;
  p->u.runtask.source = source;
  p->u.runtask.win = win;
  p->u.runtask.params = params;

  return p;
}


A_exp A_Select(A_pos pos,
	       int distinct,
	       A_list select_list,
	       A_list join_table_list,
	       A_list where_cond_list,
	       A_list group_by_list,
	       A_list having_cond_list,
	       A_list order_by_list) {
 A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_selectExp;
  p->pos=pos;

  p->u.select.distinct = distinct;
  p->u.select.hxp_list = select_list;
  p->u.select.join_table_list = join_table_list;
  p->u.select.group_by_list = group_by_list;
  p->u.select.order_by_list = order_by_list;
  p->u.select.wr_prd_list = where_cond_list;
  p->u.select.hv_prd_list = having_cond_list;

  setParent(select_list, p, A_EXP);
  setParent(join_table_list, p, A_EXP);
  setParent(where_cond_list, p, A_EXP);
  setParent(group_by_list, p, A_EXP);
  setParent(having_cond_list, p, A_EXP);
  setParent(order_by_list, p, A_EXP);

  return p;
}

A_exp A_Select(A_pos pos,
	       int distinct,
	       A_list select_list,
	       A_list join_table_list,
	       A_exp where_cond,
	       A_list group_by_list,
	       A_exp having_cond,
	       A_list order_by_list)
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  p->kind=A_selectExp;
  p->pos=pos;

  p->u.select.distinct = distinct;
  p->u.select.hxp_list = select_list;
  p->u.select.join_table_list = join_table_list;
  p->u.select.group_by_list = group_by_list;
  p->u.select.order_by_list = order_by_list;
  p->u.select.wr_prd_list = p->u.select.hv_prd_list = (A_list)0;

  if (where_cond) {
    p->u.select.wr_prd_list = A_List(where_cond->pos, A_EXP);
    decomposeBoolExpr(where_cond, p->u.select.wr_prd_list);
  }
  if (having_cond) {
    p->u.select.hv_prd_list = A_List(having_cond->pos, A_EXP);
    decomposeBoolExpr(having_cond, p->u.select.hv_prd_list);
  }

  setParent(select_list, p, A_EXP);
  setParent(join_table_list, p, A_EXP);
  setParent(where_cond, p, A_EXP);
  setParent(group_by_list, p, A_EXP);
  setParent(having_cond, p, A_EXP);
  setParent(order_by_list, p, A_EXP);

  return p;
}

int isValuesSqlOpr(A_exp p)
{
  /* return 1 if it is a VALUES() construct */
  int isValues = 0, i;
  if (p->kind == A_sqloprExp) {
    if (p->u.sqlopr->kind == A_SQL_SEL) {
      if (p->u.sqlopr->jtl_list==(A_list)0)
	isValues = 1;
    }
    if (p->u.sqlopr->kind == A_SQL_UNION) {
      A_list lst = p->u.sqlopr->jtl_list;
      isValues = 1;
      for (i=0; i<lst->length; i++) {
	A_qun p = (A_qun)getNthElementList(lst, i);
	if (!(p->kind == QUN_QUERY && isValuesSqlOpr(p->u.query)))
	  isValues = 0;
      }
    }
  }
  return (isValues);
}
A_exp A_SqlOprExp(A_pos pos,
		  sqlopr_t kind,
		  int distinct,
		  A_list hxp_list,
		  A_list jtl_list,
		  A_list prd_list
		  )
{
  A_exp p = (A_exp)ntMalloc(sizeof(*p));
  A_sqlopr o = (A_sqlopr)ntMalloc(sizeof(*o));

  p->pos = o->pos = pos;
  p->kind=A_sqloprExp;
  p->u.sqlopr = o;

  o->kind = kind;
  o->distinct = distinct;
  o->hxp_list = hxp_list;
  o->jtl_list = jtl_list;
  o->prd_list = prd_list;

  setParent(hxp_list, p, A_EXP);
  setParent(jtl_list, p, A_EXP);
  setParent(prd_list, p, A_EXP);

  return p;
}

/*************************************************************/
/*                EQUAL                                      */
/*************************************************************/
int equalList(A_list list1, A_list list2)
{
	if (A_ListEmpty(list1) && A_ListEmpty(list2)) return 1;
	if (A_ListEmpty(list1) || A_ListEmpty(list2) || (list1->length != list2->length)) return 0;
	for (int i = 0; i < list1->length; ++i) {
		if (!equalExp((A_exp)getNthElementList(list1, i), (A_exp)getNthElementList(list2, i))) return 0;
	}
	return 1;
}

int equalWin(A_win win1, A_win win2)
{
	if (win1 == win2 ) return 1;
	if (win1 != (A_win)0 && win2 != (A_win)0) {
		if (!equalList(win1->order_by_list, win2->order_by_list)) return 0;
		if (!equalList(win1->partition_list, win2->partition_list)) return 0;
		if (win1->range->size != win2->range->size || win1->range->type != win2->range->type) return 0;
		if (win1->slide->size != win2->slide->size || win1->slide->type != win2->slide->type) return 0;
	}
	return 1;
}

int compareTimeStamps(timeval* t1, timeval* t2)
{
  if(t1->tv_sec == t2->tv_sec && t1->tv_usec == t2->tv_usec)
    return 1;
  return 0;
}

int equalExp(A_exp exp1, A_exp exp2)
{
  int result = 0;

  if (exp1== exp2 || exp1 == (A_exp)0 && exp2 == (A_exp)0) {
    result = 1;
    goto exit;
  }

  if (exp1 == (A_exp)0 ||
      exp2 == (A_exp)0 ||
      exp1->kind != exp2->kind) {
    result = 0;
    goto exit;
  }

  switch (exp1->kind) {
  case A_refExp:
    result = equalRef(exp1->u.ref, exp2->u.ref);
    break;
  case A_varExp:
    result = equalVar(exp1->u.var, exp2->u.var);
    break;
  case A_nilExp:
    result = 1;
    break;
  case A_intExp:
    result = (exp1->u.intt == exp2->u.intt);
    break;
  case A_stringExp:
    result = (strcmp(exp1->u.string, exp2->u.string)==0);
    break;
  case A_timestampExp:
    result = compareTimeStamps(exp1->u.timestamp, exp2->u.timestamp);
    break;
  case A_callExp:
    {
      result = 0;
      if (exp1->u.call.func != exp2->u.call.func) {
    	  goto exit;
      }
      result = equalList(exp1->u.call.args, exp2->u.call.args);
      if (result == 0) {
    	  goto exit;
      }
      result = equalWin(exp1->u.call.win, exp2->u.call.win);
    }
    break;
  case A_opExp:
    result =  
      (exp1->u.op.oper == exp2->u.op.oper) &&
      equalExp(exp1->u.op.left, exp2->u.op.left) &&
      equalExp(exp1->u.op.left, exp2->u.op.left);
    break;
  case A_recordExp:
    break;
  case A_seqExp:
    break;
  case A_assignExp:
    break;
  case A_ifExp:
    break;
  case A_whileExp:
    break;
  case A_forExp:
    break;
  case A_breakExp:
    break;
  case A_letExp:
    break;
  case A_arrayExp:
    break;
  case A_selectExp:
    break;
  case A_sqloprExp:
    break;
  }
 exit:
  return result;
}
int equalVar(A_var var1, A_var var2)
{
  int result = 0;

  if (var1->kind == var2->kind) {

    switch (var1->kind) {
    case A_simpleVar:
      result = (var1->u.simple == var2->u.simple);
      break;
    case A_fieldVar:
      result = 
	equalVar(var1->u.field.var, var2->u.field.var) &&
	(var1->u.field.sym == var2->u.field.sym);
      break;
    case A_subscriptVar:
      break;
    }
  }

  return result;
}
/*************************************************************/
/*                DISPLAY                                    */
/*************************************************************/
char *doper[] = {
  " + ", /*  A_plusOp */
  " - ", /*  A_minusOp */
  " * ", /*  A_timesOp */
  " / ", /*  A_divideOp */
  " % ", /*  A_modOp */
  " == ", /*  A_eqOp */
  " != ", /*  A_neqOp */
  " < ", /*  A_ltOp */
  " <= ", /*  A_leOp */
  " > ", /*  A_gtOp */
  " >= ", /*  A_geOp */
  " && ", /*  A_andOp */
  " || ", /*  A_orOp */
  " LIKE ", /*  A_likeOp */
  " NOT LIKE ", /*  A_nlikeOp */
  " IN ", /*  A_inOp */
  " NOT IN ", /*  A_ninOp */
  " EXIST ", /*  A_existOp */
  " IS NULL ", /*  A_isnullOp */
  " IS NOT NULL " /*  A_isnnullOp */
};

void displayTableDecList(A_list decs)
{
  for(int i = 0; i< decs->length; i++)
  {
    A_dec dec = (A_dec)getNthElementList(decs, i);
    if(dec->kind == A_tabVarDec)
    {
      A_list fields = dec->u.tabvar.ty->u.record;
    }
  }
}

void displayExp(A_exp e)
{
  int i;

  switch (e->kind) {
  case A_refExp:
    displayRef(e->u.ref);
    break;
  case A_varExp:
    displayVar(e->u.var);
    break;
  case A_nilExp:
    fprintf(stderr, "null");
    break;
  case A_intExp:
    fprintf(stderr, "%d", e->u.intt);
    break;
  case A_realExp:
    fprintf(stderr, "%f", e->u.realt);
    break;
  case A_stringExp:
    fprintf(stderr, "%s", e->u.string);
    break;
  case A_timestampExp:
  {
    struct tm *tm;
    char date[30];
    tm = localtime(&e->u.timestamp->tv_sec);
    strftime(date, 30, TIMESTAMP_FORMAT, tm);
    fprintf(stderr, "%s", date);
  }
    break;
  case A_callExp:
    {
      fprintf(stderr, "%s(", S_name(e->u.call.func));

      if (!A_ListEmpty(e->u.call.args)) {
	for (i=0; i<e->u.call.args->length; i++) {
	  A_exp se = (A_exp)getNthElementList(e->u.call.args, i);
	  displayExp(se);
	  if (i<e->u.call.args->length-1) 
	    fprintf(stderr, ", ");
	}
      }
      fprintf(stderr, ")");

      if(e->u.call.win) {
        fprintf(stderr, "OVER (");
        if(e->u.call.win->range->type == COUNT_RANGE) {
          fprintf(stderr, "ROWS %d PRECEDING", e->u.call.win->range->size);
        }
        else {
          fprintf(stderr, "RANGE %d PRECEDING", e->u.call.win->range->size);
        }
        if(e->u.call.win->slide) {
          fprintf(stderr, " SLIDE %d)", e->u.call.win->slide->size);
        }
        else {
          fprintf(stderr, ")");
        }
      }

      if (e->u.call.member) {
	fprintf(stderr, "->%s", S_name(e->u.call.member));
      }
    }
    break;
  case A_opExp:
    if (e->u.op.oper == A_existOp || e->u.op.oper == A_notexistOp) {
      /* prefix */
      fprintf(stderr, "%s", doper[(int)e->u.op.oper]);
      displayExp(e->u.op.left);
    } else if (e->u.op.oper == A_isnullOp || e->u.op.oper == A_isnnullOp) {
      /* postfix */
      displayExp(e->u.op.left);
      fprintf(stderr, "%s", doper[(int)e->u.op.oper]);
    } else {
      /* infix */
      displayExp(e->u.op.left);
      fprintf(stderr, "%s", doper[(int)e->u.op.oper]);
      displayExp(e->u.op.right);
    }
    break;
  case A_recordExp:
    break;
  case A_seqExp:
    {
      if (!A_ListEmpty(e->u.seq)) {
	for (i=0; i<e->u.seq->length; i++) {
	  A_exp seqe = (A_exp)getNthElementList(e->u.seq, i);
	  displayExp(seqe);
	}
      }
    }
    break;
  case A_assignExp:
    {
      displayVar(e->u.assign.var);
      fprintf(stderr, " = ");
      displayExp(e->u.assign.exp);
    }
    break;
  case A_ifExp:
    break;
  case A_whileExp:
    break;
  case A_forExp:
    break;
  case A_breakExp:
    break;
  case A_letExp:
    {
      displayTableDecList(e->u.let.decs);
      displayExp(e->u.let.body);
    }
    break;
  case A_arrayExp:
    break;
  case A_selectExp:
    {
      A_list alist;

      if (A_ListEmpty(e->u.select.join_table_list))
	fprintf(stderr, "VALUES(");
      else 
	fprintf(stderr, "SELECT ");

      if (e->u.select.distinct) fprintf(stderr, "DISTINCT ");

      alist = e->u.select.hxp_list;
      for (i=0; i<alist->length; i++) {
	A_selectitem si = (A_selectitem) getNthElementList(alist, i);
	displaySelectItem(si);
	if (i<alist->length-1) 
	  fprintf(stderr, ", ");
      }
      if (A_ListEmpty(e->u.select.join_table_list))
	fprintf(stderr, ")\n");
      else {
	fprintf(stderr, "\nFROM ");
	alist = e->u.select.join_table_list;
	for (i=0; i<alist->length; i++) {
	  A_qun q = (A_qun) getNthElementList(alist, i);
	  displayQun(q);
	  if (i<alist->length-1) 
	    fprintf(stderr, ", ");
	}
      }
      alist = e->u.select.wr_prd_list;
      if (alist && alist->length>0) {
	fprintf(stderr, "\nWHERE ");
	for (i=0; i<alist->length; i++) {
	  A_exp conde = (A_exp) getNthElementList(alist, i);
	  displayExp(conde);
	  if (i<alist->length-1) 
	    fprintf(stderr, " AND ");
	}
      }
      if (!A_ListEmpty(e->u.select.group_by_list)) {
	alist = e->u.select.group_by_list;
	fprintf(stderr, "\nGROUP BY ");
	for (i=0; i<alist->length; i++) {
	  A_exp conde = (A_exp) getNthElementList(alist, i);
	  displayExp(conde);
	  if (i<alist->length-1) 
	    fprintf(stderr, ", ");
	}
      }
      alist = e->u.select.hv_prd_list;
      if (alist && alist->length>0) {
	fprintf(stderr, "\nHAVING ");
	for (i=0; i<alist->length; i++) {
	  A_exp conde = (A_exp) getNthElementList(alist, i);
	  displayExp(conde);
	  if (i<alist->length-1) 
	    fprintf(stderr, " AND ");
	}
      }
      alist = e->u.select.order_by_list;
      if (alist && alist->length>0) {
	fprintf(stderr, "\nORDER BY ");
	for (i=0; i<alist->length; i++) {
	  A_orderitem oi = (A_orderitem) getNthElementList(alist, i);
	  displayExp(oi->exp);
	  fprintf(stderr, (oi->dir)? " DSC":" ASC");
	  if (i<alist->length-1) 
	    fprintf(stderr, ", ");
	}
      }
    }
    fprintf(stderr, "\n");
    break;
  case A_sqloprExp:
    displaySqlOpr(e->u.sqlopr);
    break;
  case A_runtaskExp:
    if(e->u.runtask.modelName)
      fprintf(stderr, "run %s.%s on %s\n", S_name(e->u.runtask.modelName), 
                 S_name(e->u.runtask.task), S_name(e->u.runtask.source));
    else 
      fprintf(stderr, "run %s on %s\n",  
                 S_name(e->u.runtask.task), S_name(e->u.runtask.source));
    break;
  case A_createstreamExp:
      fprintf(stderr, "create stream %s ", S_name(e->u.createstream.name));
      displayExp(e->u.createstream.query);
    break;
  default:
    printf("Unkown Exp type!\n");
  }
}

A_field copyField(A_field field) {
  return A_Field(field->pos, field->name, field->typ, field->size);
}

A_exp copyExp(A_exp e)
{
  int i;
  A_exp p = (A_exp)0;

  if (e) {
    switch (e->kind) {
    case A_refExp:
      p = A_RefExp(e->pos, copyRef(e->u.ref));
      break;
    case A_varExp:
      p = A_VarExp(e->pos, copyVar(e->u.var));
      break;
    case A_nilExp:
      p = A_NilExp(e->pos);
      break;
    case A_intExp:
      p = A_IntExp(e->pos, e->u.intt);
      break;
    case A_realExp:
      p = A_RealExp(e->pos, e->u.realt);
      break;
    case A_stringExp:
      p = A_StringExp(e->pos, copyStr(e->u.string));
      break;
    case A_timestampExp:
      p = A_TimestampExp(e->pos, copyTimeval(e->u.timestamp));
      break;
    case A_callExp:
      p = A_CallExp(e->pos, e->u.call.func, 
		    copyAList(e->u.call.args), e->u.call.member, 
                    e->u.call.init);
      p->u.call.win = copyWin(e->u.call.win);
      break;
    case A_opExp:
      p = A_OpExp(e->pos, e->u.op.oper, 
		  copyExp(e->u.op.left), 
		  copyExp(e->u.op.right));
      break;
    case A_recordExp:
      p = A_RecordExp(e->pos, 
		      e->u.record.typ, 
		      copyAList(e->u.record.fields));
      break;
    case A_seqExp:
      p = A_SeqExp(e->pos, copyAList(e->u.seq));
      break;
    case A_assignExp:
      p = A_AssignExp(e->pos, copyVar(e->u.assign.var), copyExp(e->u.assign.exp));
      break;
    case A_ifExp:
      p = A_IfExp(e->pos, copyExp(e->u.iff.test),
		  copyExp(e->u.iff.then),
		  copyExp(e->u.iff.elsee));
      break;
    case A_whileExp:
      p = A_WhileExp(e->pos, copyExp(e->u.whilee.test), copyExp(e->u.whilee.body));
      break;
    case A_forExp:
      p = A_ForExp(e->pos, e->u.forr.var, 
		   copyExp(e->u.forr.lo),
		   copyExp(e->u.forr.hi),
		   copyExp(e->u.forr.body));
      break;
    case A_breakExp:
      p = A_BreakExp(e->pos);
      break;
    case A_letExp:
      p = A_LetExp(e->pos, 
		   copyAList(e->u.let.decs), 
		   copyExp(e->u.let.body));
      break;
    case A_arrayExp:
      p = A_ArrayExp(e->pos, e->u.array.typ, 
		     copyExp(e->u.array.size), 
		     copyExp(e->u.array.init));
      break;
    case A_selectExp:
      {
	p = (A_exp)ntMalloc(sizeof(*p));

	p->kind=A_selectExp;
	p->pos=e->pos;

	p->u.select.distinct = e->u.select.distinct;
	p->u.select.hxp_list = copyAList(e->u.select.hxp_list);
	p->u.select.join_table_list = copyAList(e->u.select.join_table_list);
	p->u.select.group_by_list = copyAList(e->u.select.group_by_list);
	p->u.select.order_by_list = copyAList(e->u.select.order_by_list);
	p->u.select.wr_prd_list = copyAList(e->u.select.wr_prd_list);
	p->u.select.hv_prd_list = copyAList(e->u.select.hv_prd_list);
      }
      break;
    case A_sqloprExp:
      {
	A_list jtl_list = copyAList(e->u.sqlopr->jtl_list);
	A_list hxp_list = copyAList(e->u.sqlopr->hxp_list);
	A_list prd_list = copyAList(e->u.sqlopr->prd_list);
	/*
	list_t *hxp_list;
	list_t *jtl_list;
	if (e->u.sqlopr->kind == A_SQL_UPDATE) {
	  hxp_list = copyTypeList(e->u.sqlopr->hxp_list, A_EXP);	  
	} else {
	  hxp_list = copyTypeList(e->u.sqlopr->hxp_list, A_SELECT_ITEM);
	}

	jtl_list = copyTypeList(e->u.sqlopr->jtl_list, A_QUN);
	*/
	
	p = A_SqlOprExp(e->pos, e->u.sqlopr->kind,
			e->u.sqlopr->distinct,
			hxp_list,
			jtl_list,
			prd_list);
      }
      break;
    }
  }
  return p;
}

void displaySqlOpr(A_sqlopr sql)
{
  int i;

  switch (sql->kind) {
  case A_SQL_ORDER:
  case A_SQL_SEL:
  case A_SQL_GB:
    if (A_ListEmpty(sql->jtl_list))
      fprintf(stderr, "VALUES(");
    else 
      fprintf(stderr, "SELECT ");

    if (sql->distinct) fprintf(stderr, "DISTINCT ");

    for (i=0; i<sql->hxp_list->length; i++) {
      A_selectitem si = (A_selectitem) getNthElementList(sql->hxp_list, i);
      displaySelectItem(si);
      if (i<sql->hxp_list->length-1) 
	fprintf(stderr, ", ");
    }
    if (A_ListEmpty(sql->jtl_list)) 
      fprintf(stderr, ")\n");
    else {
      fprintf(stderr, "\nFROM ");
      for (i=0; i<sql->jtl_list->length; i++) {
	A_qun q = (A_qun) getNthElementList(sql->jtl_list, i);
	displayQun(q);
	if (i<sql->jtl_list->length-1) 
	  fprintf(stderr, ", ");
      }
    }
    if (!A_ListEmpty(sql->prd_list)) {
      if (sql->kind == A_SQL_ORDER) {
	fprintf(stderr, "\nORDER BY ");

	for (i=0; i<sql->prd_list->length; i++) {
	  A_orderitem e = (A_orderitem) getNthElementList(sql->prd_list, i);
	  displayExp(e->exp);
	  fprintf(stderr, (e->dir)? " DSC":" ASC");
	  if (i<sql->prd_list->length-1) 
	    fprintf(stderr, ", ");
	}
      } else {
	if (sql->kind == A_SQL_GB) 
	  fprintf(stderr, "\nGROUP BY ");
	else 
	  fprintf(stderr, "\nWHERE ");

	for (i=0; i<sql->prd_list->length; i++) {
	  A_exp e = (A_exp) getNthElementList(sql->prd_list, i);
	  displayExp(e);
	  if (i<sql->prd_list->length-1) 
	    fprintf(stderr, " AND ");
	}
      }
      fprintf(stderr, "\n");
    }
    break;
  case A_SQL_UNION:
    break;
  case A_SQL_EXCEPT:
    break;
  case A_SQL_INTERSECT:
    break;
  case A_SQL_INSERT:
    {
      A_qun q = (A_qun) getNthElementList(sql->jtl_list, 0);
      A_qun source = (A_qun) getNthElementList(sql->jtl_list, 1);

      fprintf(stderr, "INSERT INTO %s (%d)", S_name(q->u.table.name),
                                             q->u.table.dynamic);
      displayExp(source->u.query);
      fprintf(stderr, ";\n");
    }
    break;
  case A_SQL_DELETE:
    {
      A_qun q = (A_qun) getNthElementList(sql->jtl_list, 0);

      fprintf(stderr, "DELETE FROM %s (%d)", S_name(q->u.table.name),
                                             q->u.table.dynamic);

      if (sql->prd_list && sql->prd_list->length>0) {
	fprintf(stderr, "\nWHERE ");
	for (i=0; i<sql->prd_list->length; i++) {
	  A_exp e = (A_exp) getNthElementList(sql->prd_list, i);
	  displayExp(e);
	  if (i<sql->prd_list->length-1) 
	    fprintf(stderr, " AND ");
	}
      }
      fprintf(stderr, ";\n");
    }
    break;
  case A_SQL_UPDATE:
    {
      A_qun q = (A_qun) getNthElementList(sql->jtl_list, 0);

      fprintf(stderr, "UPDATE %s (%d) SET ", S_name(q->u.table.name),
                                             q->u.table.dynamic);

      for (i=0; i<sql->hxp_list->length; i++) {
	A_exp setitem = (A_exp)getNthElementList(sql->hxp_list, i);
	displayExp(setitem);
	if (i<sql->hxp_list->length-1)
	  fprintf(stderr, ", ");
      }

      if (sql->prd_list && sql->prd_list->length>0) {
	fprintf(stderr, "\nWHERE ");
	for (i=0; i<sql->prd_list->length; i++) {
	  A_exp e = (A_exp) getNthElementList(sql->prd_list, i);
	  displayExp(e);
	  if (i<sql->prd_list->length-1) 
	    fprintf(stderr, " AND ");
	}
      }
      fprintf(stderr, ";\n");
    }
    break;
  }
}

void displaySelectItem(A_selectitem si)
{
  switch (si->kind) {
  case SIMPLE_ITEM:
    displayExp(si->u.s.exp);
    if (si->u.s.alias) 
      fprintf(stderr, " %s", S_name(si->u.s.alias));
    break;
  case STAR_ITEM:
    if (si->u.table) 
      fprintf(stderr, "%s.*", S_name(si->u.table)); 
    else
      fprintf(stderr, "*.*");
    break;
  }
}
void displayVar(A_var var)
{
  switch (var->kind) {
  case A_simpleVar:
    fprintf(stderr, "%s", S_name(var->u.simple));
    break;
  case A_fieldVar:
    displayVar(var->u.field.var);
    fprintf(stderr, ".%s", S_name(var->u.field.sym));
    break;
  case A_refVar:
    displayRef(var->u.ref);
    break;
  case A_subscriptVar:
    break;
  }
}
void displayQun(A_qun qun)
{
  switch (qun->kind) {
  case QUN_NAME:
    fprintf(stderr, "%s(%d)", S_name(qun->u.table.name), qun->u.table.name);
    break;
  case QUN_FUNCTION:
    break;
  case QUN_WINDOW:
    fprintf(stderr, "TABLE(%s OVER(omitted(RANGE/ROW ...) %s)) AS %s\n",
	    S_name(qun->u.window.name), S_name(qun->u.window.respectTo),
	    S_name(qun->alias));
    break;
  case QUN_QUERY:
    fprintf(stderr, " (");
    displayExp(qun->u.query);
    fprintf(stderr, ")");
    break;
  }
  if (qun->alias)
    fprintf(stderr, " %s", S_name(qun->alias));
}


