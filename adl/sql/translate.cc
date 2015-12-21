#include <sql/trans2C.h>
#include <sql/ntsql_irtree.h>
#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include "util.h"
#include "list.h"

struct Cx {
  list_t *trues;
  list_t *falses;
  T_stm stm;
};

typedef enum {
  Tr_ex,
  Tr_nx,
  Tr_cx
} tr_t;

struct Tr_exp_ {
  tr_t kind;
  union {
    T_exp ex;
    T_stm nx;
    struct Cx *cx;
  } u;
};

int trans2C(S_table venv, S_table tenv, A_exp e, char *filename)
{
  int rc=0;
  FILE *out;

  nt_obj_t *out = makeStream(O_NUM, filename, 0);

  if (out == (nt_obj_t*)0) {
    rc = ERR_OPEN_FILE;
    displayErr(ERR_OPEN_FILE, filename);
    goto exit;
  }

  ntIndentPrintf(out, 0, "#include <sys/types.h>");
  ntIndentPrintf(out, 0, "\n#include <stdio.h>");
  ntIndentPrintf(out, 0, "\n#include <db.h>");

  struct expty exp;
  list_t *decl = e->u.let.decs->list;

  for (i=0; i<decl->length; i++)
    {
      A_dec d = (A_dec)getNthElementList(decl, i);
      rc = transDec(venv, tenv, d, out, 0);
      if (rc) {
	displayErr(ERR_HISTORY, 50, "trans2C", "transDec");
	goto exit;
      }
    }

  ntIndentPrintf(out, 0, "\nvoid main()\n{");
  ntIndentPrintf(out, 1, "\nint rc;");

  rc = transExp(venv, tenv, a->u.let.body, 0, out, 1);

  if (rc) {
    displayErr(ERR_HISTORY, 100, "trans2C", "transExp");
    goto exit;
  }

  ntIndentPrintf(out, 0, "\nexit:");
  ntIndentPrintf(out, 1, "\nexit(rc);");
  ntIndentPrintf(out, 0, "\n}");

 exit:
  if (out) fclose(out);
  return rc;
}

int transTabDec2C(nt_obj_t *stream, int indent, char *varname)
{
  char *name = getUniqueName(varname);
  int rc = 0;

  ntIndentPrintf(stream, indent, "
  DB *%s;

  if ((rc = db_create(&%s, NULL, 0)) != 0) {
    fprintf(stderr, \"db_create: \%s\n\", db_strerror(rc));
    exit (1);
  }
  if ((rc = %s->open(%s,  %s, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
    dbp->err(%s, rc, \"\%s\", %s);
    goto exit;
  }", name, name, name, name, name, name, name);
  return rc;
}


/* fill in all the label-holes in patch list 'plist' with the lable 'label' */  
static void doPatch(list_t *plist, Temp_label label)
{
  int i;
  for (i=0; i<plist->length; i++) {
    Temp_label *a = (Temp_label*)getNthElementList(plist, i);
    *a = label;
  }
}

static list_t *joinPatch(list_t *first, list_t *second)
{
}

Tr_exp Tr_Ex(T_exp ex)
{
  Tr_exp e = (Tr_exp)ntMalloc(sizeof(*e));
  e->kind = Tr_ex;
  e->u.ex = ex;
  return e;
}     
 
Tr_exp Tr_Nx(T_stm nx)
{
  Tr_exp e = (Tr_exp)ntMalloc(sizeof(*e));
  e->kind = Tr_nx;
  e->u.nx = nx;
  return e;
}

Tr_exp Tr_Cx(list_t *trues, list_t *falses, T_stm stm)
{
  Tr_exp e = (Tr_exp)ntMalloc(sizeof(*e));
  e->kind = Tr_cx;

  e->u.cx = (struct Cx*)ntMalloc(sizeof(struct Cx));
  e->u.cx->trues = trues;
  e->u.cx->falses = falses;
  e->u.cx->stm = stm;
  return e;
}

static T_exp seq(int n, ...)
{
  va_list ap;
  T_exp s[16], exp;
  int i=0;

  va_start(ap, n);
  for (i=0; i<n; i++)
    s[i] = va_arg(ap, T_exp); 
  va_end(ap);

  exp = s[n-1];
  for (i=n-2; i>=0; i--)
    exp = T_Seq(s[i], exp);

  return exp;
}     

static T_exp unEx(Tr_exp e)
{
  switch (e->kind) {
  case Tr_ex:
    return e->u.ex;
  case Tr_cx:
    {
      Temp_temp r = Temp_newtemp();
      Temp_label t = Temp_newlabel();
      Temp_label f = Temp_newlabel();
      
      doPatch(e->u.cx->trues, t);
      doPatch(e->u.cx->falses, f);

      return T_Eseq(seq(5,
			T_Move(T_Temp(r), T_Const(1)),
			e->u.cx->stm,
			T_Label(f), 
			T_Move(T_Temp(r), T_Const(0)),
			T_Label(t)),
		    T_Fetch(T_Temp(r)));
    }
  case Tr_nx:
    return T_Eseq(e->u.nx, T_Const(0));
  }
}

static T_stm unNx(Tr_exp e)
{
  switch (e->kind) {
  case Tr_ux:
    return e->u.stm;
  case Tr_ex:
    return T_EXP(e->u.ex);
  case Tr_cx:
    {
      Temp_label l = Temp_newlabel();
      doPatch(e->u.cx->trues, l);
      doPatch(e->u.cx->falses, l);
      return seq(2,
		 e->u.cx->stm,
		 T_Label(l));
    }
  }
}

static struct Cx *unCx(Tr_exp e)
{
  switch (e->kind) {
  case Tr_ux:
    /* err */
    EM_impossible("unCx (Nx s)");
    return (struct Cx *)0;
  case Tr_ex:
    /*
      | unCx (Ex (T.CONST 0)) = (fn (t, f) => T.JUMP (T.NAME f, [f]))
      | unCx (Ex (T.CONST _)) = (fn (t, f) => T.JUMP (T.NAME t, [t]))
      | unCx (Ex e) = (fn (t, f) => T.CJUMP (T.EQ, e, T.CONST 0, f, t))
    */
    {
      T_exp exp = e->u.ex;
      struct Cx *cx = (struct Cx*)ntMalloc(sizeof (struct Cx));
      cx->trues = newList();
      cx->falses = newList();

      if (exp->kind == T_CONST) {
	if (exp->u.CONST == 0) {
	  Temp_label f = Temp_newlabel();
	  appendElementList(cx->falses, (nt_obj_t*)f);
	  appendElementList(jumplist, (nt_obj_t*)f);
	  cx->stm = T_Jump(T_Name(f), jumplist);
	} else {
	  Temp_label t = Temp_newlabel();
	  appendElementList(cx->trues, (nt_obj_t*)t);
	  appendElementList(jumplist, (nt_obj_t*)t);
	  cx->stm = T_Jump(T_Name(t), jumplist);
	} 
      } else {
	Temp_label t = Temp_newlabel();
	Temp_label f = Temp_newlabel();

	appendElementList(cx->trues, (nt_obj_t*)t);
	appendElementList(cx->falses, (nt_obj_t*)f);
	cx->stm = T_Cjump(T_eq, exp, T_Const(0), f, t);
      }
      return cx;
    }
  case Tr_cx:
    return e->u.cx;
}

static Tr_exp 
Tr_Cmp(int scalar, int ltsw, int mustNegate, T_expty left, T_expty right)
{
  T_relOp oper;

  if (ltsw == -1 && mustNegate == FALSE) {
    oper = T_eq;
  } 
  else if (ltsw == -1 && mustNegate == TRUE) {
    oper = T_ne;
  } 
  else if (ltsw == FALSE && mustNegate == FALSE) {
    oper = T_lt;
  } 
  else if (ltsw == FALSE && mustNegate == TRUE) {
    oper = T_gt;
  } 
  else if (ltsw == TRUE && mustNegate == FALSE) {
    oper = T_ge;
  } 
  else if (ltsw == TRUE && mustNegate == TRUE) {
    oper = T_le;
  } 
  //	Cx (fn (t, f) => T.CJUMP (oper, unEx left, unEx right, t, f))
  list_t *trues = newList();
  list_t *falses = newList();
  Temp_label t = Temp_newlabel();
  Temp_label f = Temp_newlabel();
  T_stm stm;

  appendElementList(trues, (nt_obj_t*)t);
  appendElementList(falses, (nt_obj_t*)f);

  if (scalar) {
    stm = T_Cjump(oper, unEx(left), unEx(right), f, t);
  } else {
    list_t *arg_list = newList(); 
    Temp_temp templ = Temp_newtemp();
    Temp_temp tempr = Temp_newtemp();

    appendElementList(arg_list, (nt_obj_t*)T_Fetch(templ));
    appendElementList(arg_list, (nt_obj_t*)T_Fetch(tempr));

    stm = seq(3,
	      T_Move(tmpl, unEx(left)),
	      T_Move(tmpr, unEx(right)),
	      T_Cjump(T_eq,
		      Fr_externalCall("strcmp", arg_list),
		      T_Const(0), f, t));
  }

  return Tr_Cx(trues, falses, stm);
} 

Tr_exp Tr_CmpScalar(int ltsw, int mustNegate, T_expty left, T_expty right)
{
  return Tr_Cmp(1, ltsw, mustNegate, left, right);
}
Tr_exp Tr_CmpString (int ltsw, int mustNegate, T_expty larg, T_expty rarg)
{
  return Tr_Cmp(0, ltsw, mustNegate, left, right);
}
Tr_exp Tr_Arith(T_binOp oper, Tr_exp left, Tr_exp right)
{
  return Tr_Ex(T_BINOP(oper, unEx(left), unEx(right)));
}
