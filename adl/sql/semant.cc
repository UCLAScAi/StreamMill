#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include <sql/symbol.h>
#include <sql/trans2C.h>
#include <sql/io.h>
#include <sql/sql_rewrite.h>
#include <sql/aggr_info.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
// #include <dlfcn.h>

extern "C" {
#include <dbug/dbug.h>
#include <im_db.h>
}
#include "util.h"

#include <vector>
#include <string>

using namespace std;
#include <iostream>
#include <buffer.h>
#include <driver.h>
#include <stmt.h>
#include <querySchdl.h>
#include <ios/ios.h>
#include <esl/querySchdl.h>
#include <SMLog.h>
#include <list.h>
#include <vector>
//using namespace ESL;


extern A_list global_functions;

static hash_map<const char*, void*, std::hash<const char*> , eqstrTab>* inMemTables;

hash_map<const char*, void*, std::hash<const char*> , eqstrTab>* getInMemTables() {
  if (inMemTables == NULL) {
    SMLog::SMLOG(12, "\tgetInMemTables: inMemTables is null");
    inMemTables
        = new hash_map<const char*, void*, std::hash<const char*> , eqstrTab> ;
  }
  return inMemTables;
}

// Call S_dump(venv, show);
void show(S_symbol sym, void* binding) {
  printf("In show %s\n", S_name(sym));
}

Ty_ty checkType(S_table tenv, S_symbol sym, absyn_t decType) {
  //  SMLog::SMLOG(10, "Entering checkType");
  Ty_ty ty = (Ty_ty) S_look(tenv, sym);
  char *name = S_name(sym);
  if (strcasecmp(name, "iext") == 0) {
    //if(decType != A_aggregateDec) {
    //  return (Ty_ty)0;
    // }
    //else {
    ty = (Ty_ty) S_look(tenv, S_Symbol("iext"));
    //}
  } else if (strcasecmp(name, "rext") == 0) {
    if (decType != A_aggregateDec) {
      return (Ty_ty) 0;
    } else {
      ty = (Ty_ty) S_look(tenv, S_Symbol("rext"));
    }
  } else if (strcasecmp(name, "cext") == 0) {
    if (decType != A_aggregateDec) {
      return (Ty_ty) 0;
    } else {
      ty = (Ty_ty) S_look(tenv, S_Symbol("cext"));
    }
  } else if (strcasecmp(name, "text") == 0) {
    if (decType != A_aggregateDec) {
      return (Ty_ty) 0;
    } else {
      ty = (Ty_ty) S_look(tenv, S_Symbol("text"));
    }
  }

  /* for builtin types, i.e., int, real, char, we ignore up/lower cases */
  if (!ty) {

    if (strcasecmp(name, "int") == 0)
      ty = (Ty_ty) S_look(tenv, S_Symbol("int"));
    else if (strcasecmp(name, "real") == 0)
      ty = (Ty_ty) S_look(tenv, S_Symbol("real"));
    else if (strcasecmp(name, "char") == 0)
      ty = (Ty_ty) S_look(tenv, S_Symbol("char"));
    else if (strcasecmp(name, "timestamp") == 0)
      ty = (Ty_ty) S_look(tenv, S_Symbol("timestamp"));
  }

  return ty;
}

T_expty expTy(Tr_exp exp, Ty_ty ty) {
  T_expty e;

  //    e = (T_expty)ntMalloc(sizeof(*e));
  e = (T_expty) malloc(sizeof(*e));
  e->tv = NULL;
  e->exp = exp;
  e->ty = ty;
  e->size = 0;

  return (e);
}

T_expty expTy(Tr_exp exp, struct timeval* tv, Ty_ty ty) {
  T_expty e;

  //    e = (T_expty)ntMalloc(sizeof(*e));
  e = (T_expty) malloc(sizeof(*e));
  e->tv = tv;
  e->exp = exp;
  e->ty = ty;
  e->size = 0;

  return (e);
}

void expTy_Delete(T_expty e) {
  if (e->exp)
    Tr_Delete(e->exp);
  //    ntFree(e);
  free(e);
}
T_expty expTy_Seq(T_expty seq, T_expty e) {
  if (seq == (T_expty) 0) {
    seq = e;
  } else if (e != (T_expty) 0) {
    seq->exp = Tr_Seq(seq->exp, e->exp);
    //    Tr_Delete(e->exp);
    //    e->exp = (Tr_exp)0;
    expTy_Delete(e);

    seq->ty = e->ty;
  }
  return seq;
}
T_expty expTy_Seq(T_expty seq, char *buf) {
  if (seq == (T_expty) 0) {
    seq = expTy(Tr_String(buf), Ty_Nil());
  } else {
    //    seq->exp = Tr_Seq(seq->exp, Tr_String(buf));
    seq->exp = Tr_Seq(seq->exp, buf);
    seq->ty = Ty_Nil();
  }

  return seq;
}

void getQunName(A_qun qun, char *buf) {
  sprintf(buf, "%s_%d_%d", (qun->alias) ? S_name(qun->alias) : "",
      UID(qun->ppnode), UID(qun));
}

/* searchRecordField() search a list of fields and return the field
 whose name is fname. It also returns i) the field index, and ii)
 the offset of the field in the key/data buffer.  */
err_t searchRecordField(A_list fields, S_symbol fname, int *idx, // field index
    int *off, // offset of the field in the record
    Ty_field &result // OUTPUT
) {
  SMLog::SMLOG(10, "Entering searchRecordField");
  err_t rc = ERR_NONE;

  int i;
  int offset[2] = { 0, 0 }; // offset into the key/data buffer

  DBUG_ENTER("searchRecordField");

  result = (Ty_field) 0;

  for (i = 0; i < fields->length; i++) {
    Ty_field f = (Ty_field) getNthElementList(fields, i);

    if (fname == f->name) {
      if (off) {
        if (f->iskey)
          *off = offset[1];
        *off = offset[0];
      }
      if (idx)
        *idx = i;
      result = f;
      break;
    }

    if (off) {
      if (f->ty == Ty_String()) {
        if (f->iskey)
          offset[1] += f->size;
        offset[0] += f->size;
      } else {
        if (f->iskey)
          offset[1] += getStorageSize(f->ty);
        offset[0] += getStorageSize(f->ty);
      }
    }
  }
  exit: DBUG_RETURN (rc);
}

/* 
 * compile a variable 
 */
err_t transVar(S_table venv, S_table tenv, A_var v, Sql_sem sql, A_qun *outqun, // OUT: the qun this var belongs to
    int *fieldidx, // OUT: the index of the field in the QUN
    T_expty &result) {
  SMLog::SMLOG(10, "Entering transVar");
  err_t rc = ERR_NONE;
  int i, j;
  char buf[MAX_STR_LEN];
  char qunname[80];
  char *name;
  DBUG_ENTER("transVar");

  A_sqlopr a = sql->cur_sqlopr;
  Ty_field f;

  E_enventry te;
  A_qun qun;

  result = (T_expty) 0;
  if (outqun)
    *outqun = (A_qun) 0;

  switch (v->kind) {
  case A_simpleVar: {
    te = (E_enventry) S_look(venv, v->u.simple);

    if (te && // it is in the symbol table
        te->kind == E_varEntry && // it is a var
        te->u.var.ty->kind != Ty_record) // it is not of tuple type
    {
      /* this variable is passed in as a parameter in a UDA */
      if (te->u.var.iname)
        name = S_name(te->u.var.iname);
      else
        name = S_name(v->u.simple);

      result = expTy(Tr_String(name), te->u.var.ty);
      result->size = te->u.var.size;
    } else if (te && te->kind == E_streamEntry) {
      EM_error(0, ERR_TO_BE_IMPLEMENTED, __LINE__, __FILE__,
          "transVar for E_streamEntry");
    } else {
      /* this variable is a table column, we need to find the
       table to which it belongs */
      int found = 0;
      for (i = 0; a->jtl_list && i < a->jtl_list->length; i++) {
        qun = (A_qun) getNthElementList(a->jtl_list, i);
        te = (E_enventry) S_look(venv, qun->alias);
        if (te) {
          if (te->kind == E_varEntry && te->u.var.ty->kind == Ty_record) {
            rc = searchRecordField(te->u.var.ty->u.record, v->u.simple,
                fieldidx, (int*) 0, // offset
                f);
            if (rc) {
              EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transVar",
                  "searchRecordField");
              goto exit;
            }
            if (f != (Ty_field) 0) {
              if (outqun)
                *outqun = qun;
              found = 1;
              break;
            }
          } else if (te->kind == E_streamEntry && te->u.stream.ty->kind
              == Ty_record) {
            rc = searchRecordField(te->u.stream.ty->u.record, v->u.simple,
                fieldidx, (int*) 0, // offset
                f);
            if (rc) {
              EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transVar",
                  "searchRecordField");
              goto exit;
            }
            if (f != (Ty_field) 0) {
              if (outqun)
                *outqun = qun;
              found = 1;
              break;
            }

          }
        }
      }
      if (!found) {
        rc = ERR_UNDEFINED_VARIABLE;
        EM_error(v->pos, rc, __LINE__, __FILE__, S_name(v->u.simple));
        goto exit;
      }
      getQunName(qun, qunname);
      sprintf(buf, "%s.%s", qunname, S_name(f->name));
      if (f->name == S_Symbol("oid")) {
        result = expTy(Tr_String(buf), Ty_Ref(te->u.var.ty) /* convert a record type to a ref type */
        );
        result->size = te->u.var.size;
      } else {
        result = expTy(Tr_String(buf), f->ty);
        result->size = f->size;
      }
    }
  }
    break;
  case A_fieldVar: {
    A_var t = v->u.field.var;

    te = (E_enventry) S_look(venv, t->u.simple);
    if (!te || (te->kind != E_varEntry && te->kind != E_streamEntry)) {
      rc = ERR_UNDEFINED_VARIABLE;
      EM_error(t->pos, rc, __LINE__, __FILE__, S_name(t->u.simple));
      goto exit;
    }

    if (te->kind == E_varEntry) {
      if (te->u.var.ty->kind != Ty_record) {
        rc = ERR_TUPLE_TYPE_REQUIRED;
        EM_error(t->pos, rc, __LINE__, __FILE__, S_name(t->u.simple));
        goto exit;
      }

      /* the name of the structure */
      if (te->u.var.sname)
        name = S_name(te->u.var.sname);
      else
        name = S_name(t->u.simple);

      /* which qun does this var belong to? */
      if (outqun != (A_qun*) 0) {
        for (i = 0; i < a->jtl_list->length; i++) {
          qun = (A_qun) getNthElementList(a->jtl_list, i);
          if (qun->alias == t->u.simple) {
            *outqun = qun;
            break;
          }
        }
      }

      rc = searchRecordField(te->u.var.ty->u.record, v->u.field.sym, fieldidx,
          (int *) 0, f);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transVar",
            "searchRecordField");
        goto exit;
      }

      if (f == (Ty_field) 0) {
        rc = ERR_UNDEFINED_FIELD;
        EM_error(t->pos, rc, __LINE__, __FILE__, S_name(v->u.field.sym));
        goto exit;
      }

      sprintf(buf, "%s.%s", name, S_name(f->name));

      if (v->u.field.sym == S_Symbol("oid")) {
        result = expTy(Tr_String(buf), Ty_Ref(te->u.var.ty) /* convert a record type to a ref type */
        );
      } else {
        result = expTy(Tr_String(buf), f->ty);
        if (f->ty->kind == Ty_string) {
          result->size = f->size;
        }
      }
    } else if (te->kind == E_streamEntry) {
      if (te->u.stream.ty->kind != Ty_record) {
        rc = ERR_TUPLE_TYPE_REQUIRED;
        EM_error(t->pos, rc, __LINE__, __FILE__, S_name(t->u.simple));
        goto exit;
      }

      /* the name of the structure */
      if (te->u.stream.sname)
        name = S_name(te->u.stream.sname);
      else
        name = S_name(t->u.simple);

      /* which qun does this var belong to? */
      if (outqun != (A_qun*) 0) {
        for (i = 0; i < a->jtl_list->length; i++) {
          qun = (A_qun) getNthElementList(a->jtl_list, i);
          if (qun->alias == t->u.simple) {
            *outqun = qun;
            break;
          }
        }
      }

      rc = searchRecordField(te->u.stream.ty->u.record, v->u.field.sym,
          fieldidx, (int *) 0, f);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transVar",
            "searchRecordField");
        goto exit;
      }

      if (f == (Ty_field) 0) {
        rc = ERR_UNDEFINED_FIELD;
        EM_error(t->pos, rc, __LINE__, __FILE__, S_name(v->u.field.sym));
        goto exit;
      }

      sprintf(buf, "%s.%s", name, S_name(f->name));

      if (v->u.field.sym == S_Symbol("oid")) {
        result = expTy(Tr_String(buf), Ty_Ref(te->u.var.ty) /* convert a record type to a ref type */
        );
      } else {
        result = expTy(Tr_String(buf), f->ty);
        if (f->ty->kind == Ty_string) {
          result->size = f->size;
        }
      }
    }
  }
    break;
  case A_subscriptVar:
  case A_refVar:
  default:
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "transVar()");
    goto exit;
  }
  exit: DBUG_RETURN (rc);
}

err_t transFields(S_table venv, S_table tenv, A_list src, A_list fields,
    A_list reflist, /* OUT: list of reference-type
     attributes whose types are not
     yet declared */
    int *foundref, /* OUT: ref type is used in the fields */
    absyn_t decType) {
  SMLog::SMLOG(10, "Entering transFields");
  err_t rc = ERR_NONE;
  Ty_ty ty;

  DBUG_ENTER("transFields");

  for (int i = 0; i < src->length; i++) {
    A_field fi = (A_field) getNthElementList(src, i);
    Ty_field f;
    int size = fi->size;

    for (int k = 0; k < i; k++) {
      A_field fk = (A_field) getNthElementList(src, k);
      if (fi->name == fk->name) {
        rc = ERR_DUPLICATE_FIELD_NAME;
        EM_error(fi->pos, rc, __LINE__, __FILE__, S_name(fk->name));
        goto exit;
      }
    }

    if (isRefType(fi)) {
      E_enventry te;
      /* This is a reference type. */
      if (foundref)
        *foundref = 1;

      /* We look for the referenced table in the venv table */
      te = (E_enventry) S_look(venv, fi->typ);

      if (!te || te->kind != E_varEntry) {
        /* Referenced table is not declared. However, it may be
         declared later in the same declaration list; or it is a
         recursive declaration. We set the type to NULL here. Its
         real type will be filled in when the entire list is
         compiled. */
        f = Ty_Field(fi->name, fi->typ, (Ty_ty) 0);

        if (!reflist) {
          rc = ERR_UNEXPECTED;
          EM_error(fi->pos, rc, __LINE__, __FILE__,
              "reference type not supported here");
          goto exit;
        }
        appendElementList(reflist, (nt_obj_t*) f);
        appendElementList(reflist, (nt_obj_t*) fi->pos); // for error report
      } else {
        /* reloading */
        f = Ty_Field(fi->name, fi->typ, Ty_Ref(te->u.var.ty));
      }

    } else {

      ty = checkType(tenv, fi->typ, decType);
      if (!ty) {
        rc = ERR_UNDEFINED_TYPE;
        EM_error(fi->pos, rc, __LINE__, __FILE__, S_name(fi->typ));
        goto exit;
      }
      /* set default data size */
      if (size <= 0) {
        size = getDisplaySize(ty);
        /*
         if (ty == Ty_Int()) size = 10;
         if (ty == Ty_Real()) size = 10;
         if (ty == Ty_String()) size = 20;
         if (ty == Ty_Record()) size = 10;
         */
      }

      f = Ty_Field(fi->name, ty, size);
    }
    appendElementList(fields, (nt_obj_t*) f);
  }

  exit: DBUG_RETURN(rc);
}

err_t transTyRefType(S_table venv, A_ty a, Ty_ty ty) {
  SMLog::SMLOG(10, "Entering transTyRefType");
  err_t rc = ERR_NONE;
  DBUG_ENTER("transTyRefTypes");

  if (a->kind == A_recordTy) {

    A_list src = a->u.record;
    A_list record = ty->u.record;
    Ty_field f;

    for (int i = 0; i < src->length; i++) {
      A_field fi = (A_field) getNthElementList(src, i);

      if (isRefType(fi)) {
        E_enventry te;
        Ty_field f;
        /* looking for the table that is being referenced */
        te = (E_enventry) S_look(venv, fi->typ);
        if (!te || te->kind != E_varEntry) {
          rc = ERR_UNDEFINED_REF;
          EM_error(fi->pos, rc, __LINE__, __FILE__, S_name(fi->typ));
          goto exit;
        }
        /* reference field is of record type */
        f = Ty_Field(fi->name, fi->typ, Ty_Ref(te->u.var.ty));
        overwriteElementList(record, i, (nt_obj_t*) f);
      }
    }
  }
  exit: DBUG_RETURN(rc);
}

err_t transTy(S_table venv, S_table tenv, A_ty a, Ty_ty &ty, A_list reflist,
    int *foundref, absyn_t decType) {
  err_t rc = ERR_NONE;

  DBUG_ENTER("transTy");

  switch (a->kind) {
  case A_nameTy: {
    ty = checkType(tenv, a->u.name, decType);

    if (!ty) {
      rc = ERR_UNDEFINED_TYPE;
      EM_error(a->pos, rc, __LINE__, __FILE__, S_name(a->u.name));
      goto exit;
    }
  }
    break;
  case A_recordTy: {
    A_list fields = A_List(0, A_FIELD);
    rc = transFields(venv, tenv, a->u.record, fields, reflist, foundref,
        decType);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transTy", "transFields");
      goto exit;
    }
    ty = Ty_Record(fields);
  }
    break;
  case A_arrayTy:
  default:
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "transTy()");
    goto exit;
  }
  exit: DBUG_RETURN(rc);
}

err_t transSeqExp(S_table venv, S_table tenv, A_exp a, Sql_sem sql,
    T_expty &dec, T_expty &exe, vector<void*> aggregates) {
  SMLog::SMLOG(10, "Entering transSeqExp");
  err_t rc = ERR_NONE;
  A_list a_list;

  DBUG_ENTER("transSeqExp");

  dec = exe = (T_expty) 0;

  if (A_ListEmpty(a->u.seq))
    goto exit;

  a_list = a->u.seq;

  for (int i = 0; i < a_list->length; i++) {
    T_expty curdec, curexe;

    A_exp member = (A_exp) getNthElementList(a_list, i);

    rc = transExp(venv, tenv, member, sql, curdec, curexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSeqExp", "transExp");
      goto exit;
    }
    dec = expTy_Seq(dec, curdec);
    exe = expTy_Seq(exe, curexe);
  }

  exit: DBUG_RETURN(rc);
}

char* cOper[] = { "+", /* A_plusOp,      */
"-", /* A_minusOp,     */
"*", /* A_timesOp,     */
"/", /* A_divideOp,    */
"%", /* A_modOp,       */
"+=", /* A_pluseqOp,    */
"-=", /* A_minuseqOp    */
"==", /* A_eqOp,        */
"!=", /* A_neqOp,       */
"<", /* A_ltOp,        */
"<=", /* A_leOp,        */
">", /* A_gtOp,        */
">=", /* A_geOp,        */
"&&", /* A_andOp,       */
"||", /* A_orOp,	*/
"", /* A_likeOp,      */
"", /* A_nlikeOp,     */
"", /* A_inOp,        */
"", /* A_ninOp,       */
"", /* A_existOp,     */
"", /* A_notexistOp,  */
"==", /* A_isnullOp,    */
"!=" /* A_isnnullOp    */
};

#define isRefCmpType(x) (x->kind==Ty_int || x->kind==Ty_nil || x->kind==Ty_ref)
#define isNumCmpType(x) (x->kind==Ty_int || x->kind==Ty_real)

err_t doCmp(A_pos pos, A_oper oper, T_expty left, T_expty right,
    T_expty &result) {
  SMLog::SMLOG(10, "Entering doCmp oper: %i,\n\t left: %s, \n\tright: %s",
      oper, left->exp, right->exp);
  err_t rc = ERR_NONE;
  char s[MAX_STR_LEN];

  if (isRefCmpType(left->ty) && isRefCmpType(left->ty)
      || isNumCmpType(left->ty) && isNumCmpType(left->ty)) {
    /* numerical operands or ref(pointer) comparisons */
    sprintf(s, "(%s %s %s)", left->exp, cOper[(int) oper], right->exp);
    result = expTy(Tr_String(s), Ty_Int());
  } else if (left->ty->kind == Ty_string && right->ty->kind == Ty_string) {
    /* string operands */
    sprintf(s, "(strcmp(%s, %s) %s 0)", left->exp, right->exp,
        cOper[(int) oper]);
    result = expTy(Tr_String(s), Ty_Int());
  } else if (left->ty->kind == Ty_timestamp && right->ty->kind == Ty_timestamp) {
    /* timestamp operands */
    if (left->tv == NULL && right->tv == NULL)
      sprintf(s, "(timeval_cmp(%s, %s) %s 0)", left->exp, right->exp,
          cOper[(int) oper]);
    else if (left->tv == NULL && right->tv != NULL)
      sprintf(s, "(timeval_cmp(%s, A_Timeval(%d, %d)) %s 0)", left->exp,
          right->tv->tv_sec, right->tv->tv_usec, cOper[(int) oper]);
    else if (left->tv != NULL && right->tv == NULL)
      sprintf(s, "(timeval_cmp(A_Timeval(%d, %d), %s) %s 0)", left->tv->tv_sec,
          left->tv->tv_usec, right->exp, cOper[(int) oper]);
    else if (left->tv == NULL && right->tv != NULL)
      sprintf(s, "(timeval_cmp(A_Timeval(%d, %d), A_Timeval(%d, %d)) %s 0)",
          left->tv->tv_sec, left->tv->tv_usec, right->tv->tv_sec,
          right->tv->tv_usec, cOper[(int) oper]);
    result = expTy(Tr_String(s), Ty_Int());
  } else {
    rc = ERR_INVALID_ARG;
    EM_error(pos, rc, __LINE__, __FILE__);
    goto exit;
  }

  exit: return rc;
}

err_t doNullCmp(A_pos pos, A_oper oper, T_expty exp, T_expty &result) {
  SMLog::SMLOG(10, "Entering doNullCmp");
  err_t rc = ERR_NONE;
  char s[MAX_STR_LEN];

  if (exp->ty == Ty_String()) {
    sprintf(s, "(%s == _ADL_NULL_STR)", exp->exp, cOper[(int) oper]);
  } else {
    sprintf(s, "((int)(%s) %s _ADL_NULL)", exp->exp, cOper[(int) oper]);
  }

  result = expTy(Tr_String(s), Ty_Int());

  exit: return rc;
}

err_t doArith(A_pos pos, A_oper oper, T_expty left, T_expty right,
    T_expty &result) {
  SMLog::SMLOG(10, "Entering doArith oper: %i,\n\t left: %s, \n\tright: %s",
      oper, left->exp, right->exp);
  err_t rc = ERR_NONE;
  char s[MAX_STR_LEN];
  ty_t leftty, rightty;

  leftty = left->ty->kind;
  rightty = (right) ? right->ty->kind : leftty;

  if (!((leftty == Ty_int || leftty == Ty_real || leftty == Ty_timestamp)
      && (rightty == Ty_int || rightty == Ty_real || rightty == Ty_timestamp))) {
    /* numerical type expected */
    rc = ERR_INVALID_ARG;
    EM_error(pos, rc, __LINE__, __FILE__);
    goto exit;
  }

  if ((leftty != Ty_int || rightty != Ty_int) && oper == A_modOp) {
    /* only integer type allowed for modular operator */
    rc = ERR_INVALID_ARG;
    EM_error(pos, rc, __LINE__, __FILE__);
    goto exit;
  }

  if ((leftty == Ty_timestamp || rightty == Ty_timestamp) && (oper != A_plusOp
      && oper != A_minusOp)) {
    /* timestamp type only allowed in plus and minus opers */
    rc = ERR_INVALID_ARG;
    EM_error(pos, rc, __LINE__, __FILE__);
    goto exit;
  }

  if ((leftty == Ty_timestamp && rightty == Ty_timestamp) && (oper == A_plusOp)) {
    /* two timestamps cannot be added */
    rc = ERR_INVALID_ARG;
    EM_error(pos, rc, __LINE__, __FILE__);
    goto exit;
  }

  if (right) {
    if (leftty == Ty_timestamp && rightty == Ty_timestamp && oper == A_minusOp) {
      if (left->tv == NULL && right->tv == NULL) {
        sprintf(s, "timeval_subtract(%s, %s)", left->exp, right->exp);
      } else if (left->tv == NULL && right->tv != NULL) {
        sprintf(s, "timeval_subtract(%s, A_Timeval(%d, %d))", left->exp,
            right->tv->tv_sec, right->tv->tv_usec);
      } else if (left->tv != NULL && right->tv == NULL) {
        sprintf(s, "timeval_subtract(A_Timeval(%d, %d), %s)", left->tv->tv_sec,
            left->tv->tv_usec, right->exp);
      } else if (left->tv != NULL && right->tv != NULL) {
        sprintf(s, "timeval_subtract(A_Timeval(%d, %d), A_timeval(%d, %d))",
            left->tv->tv_sec, left->tv->tv_usec, right->tv->tv_sec,
            right->tv->tv_usec);
      }
      result = expTy(Tr_String(s), Ty_Real());
    } else if (leftty == Ty_timestamp && (rightty == Ty_int || rightty
        == Ty_real)) {
      if (left->tv == NULL) {
        if (oper == A_plusOp)
          sprintf(s, "timeval_add(%s, %s)", left->exp, right->exp);
        else if (oper == A_minusOp)
          sprintf(s, "timeval_subtract(%s, %s)", left->exp, right->exp);
      } else {
        if (oper == A_plusOp)
          sprintf(s, "timeval_add(A_Timeval(%d, %d), %s)", left->tv->tv_sec,
              left->tv->tv_usec, right->exp);
        else if (oper == A_minusOp)
          sprintf(s, "timeval_subtract(A_Timeval(%d, %d), %s)",
              left->tv->tv_sec, left->tv->tv_usec, right->exp);
      }
      result = expTy(Tr_String(s), Ty_Timestamp());
    } else if (rightty == Ty_timestamp && (leftty == Ty_int || leftty
        == Ty_real)) {
      if (right->tv == NULL) {
        if (oper == A_plusOp)
          sprintf(s, "timeval_add(%s, %s)", right->exp, left->exp);
        else if (oper == A_minusOp)
          sprintf(s, "timeval_subtract(%s, %s)", left->exp, right->exp);
      } else {
        if (oper == A_plusOp)
          sprintf(s, "timeval_add(A_Timeval(%d, %d), %s)", right->tv->tv_sec,
              right->tv->tv_usec, left->exp);
        else if (oper == A_minusOp)
          sprintf(s, "timeval_subtract(%s, A_Timeval(%d, %d))", left->exp,
              right->tv->tv_sec, right->tv->tv_usec);
      }
      result = expTy(Tr_String(s), Ty_Timestamp());
    } else {
      sprintf(s, "((%s) %s %s)", left->exp, cOper[(int) oper], right->exp);
    }
  } else {
    sprintf(s, "((%s) %s)", left->exp, (oper == A_minuseqOp) ? "--" : "++");
  }

  if (leftty == Ty_timestamp || rightty == Ty_timestamp)
    ;
  else if (leftty == Ty_real || rightty == Ty_real)
    result = expTy(Tr_String(s), Ty_Real());
  else
    result = expTy(Tr_String(s), Ty_Int());

  exit: return rc;
}

/*
 *  Compile a QUN (possible with a window construct)
 *
 *
 next_q:
 getTuple from qun;
 if (found tuple && win) {
 apply win.pred on tuple;
 if pred fails
 goto next_q;
 }
 put tuple into a logic window
 the tuple and the logic window will be used by subsequent code

 */
err_t transQun(S_table venv, S_table tenv, A_qun q, Sql_sem sql,
    T_expty &dec, // OUT
    T_expty &exe, // OUT
    char *name,
    Tr_exp *bndp, // IN: binding equality
    Tr_exp *bndpUpper, // IN: binding upper bound
    Tr_exp *bndpLower, // IN: binding lower bound
    int flag, // IN: binding flag
    vector<void*> aggregates, vector<string> &srcs, char* target_handle,
    cStmt*& cstmt) {
  SMLog::SMLOG(10, "Entering transQun");
  err_t rc = ERR_NONE;
  int i;
  T_expty exebase = (T_expty) 0;

  rc = transBaseQun(venv, tenv, q, sql, dec, exebase, name, bndp, bndpUpper,
      bndpLower, flag, aggregates, srcs, target_handle, cstmt);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transQun", "transBaseQun");
    goto exit;
  }

  exe = exebase;

#if 0
  if (!q->win) {
    exe = exebase;
  } else {
    char buf[MAX_STR_LEN];
    Ty_ty ty = exebase->ty;

    sprintf(buf, "\nnext_%d:  /* begin of Window Construct %d */"
        "\nwin_t win_%d;"
        , UID(q), UID(q), UID(q));
    exe = expTy_Seq(exe, buf);

    sprintf(buf, "\nif (first_entry_%d == DB_FIRST) {"
        "\n/* initialize window */"
        "\n}"
        , UID(q));
    exe = expTy_Seq(exe, buf);

    exe = expTy_Seq(exe, exebase);
    exe = expTy_Seq(exe, "\nif (rc != DB_NOTFOUND) {"
        "\n/* apply preds in the window */");

    if (!A_ListEmpty(q->win->prd_list)) {
      for (i=0; i<q->win->prd_list->length; i++) {
        T_expty d,e;
        A_exp prd = (A_exp)getNthElementList(q->win->prd_list, i);
        rc = transExp(venv, tenv, prd, sql, d, e);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transQun", "transExp");
          goto exit;
        }
        sprintf(buf, "\n if (!(%s)) {"
            "\nrc = DB_NOTFOUND;"
            "\ngoto next_%d;"
            "\n}"
            , e->exp, UID(q));
        exe = expTy_Seq(exe, buf);
      }
    }
    // put tuple into a logic window
    sprintf(buf, "\n} else {"
        "\n/* destroy window */"
        "\n}/* end of Window construct %d */", UID(q));
    exe = expTy_Seq(exe, buf);
    exe->ty = ty;
  }
#endif
  exit: return rc;
}

/* 
 * compile a QUN under a node. 
 *
 * a QUN can be a stream, a table, a sub query, or a table function
 *
 * If QUN is a table, then for each key of the table, bndp stores the
 * value the key is bound to.
 */
err_t transBaseQun(S_table venv, S_table tenv, A_qun q, Sql_sem sql,
    T_expty &dec, // OUT
    T_expty &exe, // OUT
    char *name, Tr_exp *bndp, // IN, binding equality
    Tr_exp *bndpUpper, //IN: binding upper bound
    Tr_exp *bndpLower, // IN: binding lower bound
    int flag, vector<void*> aggregates, vector<string> &srcs, // source buffers name
    char* target_handle, cStmt*& cstmt) {
  SMLog::SMLOG(10, "Entering transBaseQun");
  err_t rc = ERR_NONE;
  char buf[MAX_STR_LEN], qunname[80];
  char first_entry_name[MAX_STR_LEN], cursor_name[MAX_STR_LEN];
  E_enventry te;

  DBUG_ENTER("transBaseQun");

  dec = exe = (T_expty) 0;

  //printf("qun kind %d\n", q->kind);fflush(stdout);

  switch (q->kind) {
  case QUN_NAME: {
    //printf("qun type name\n");
    E_enventry x = (E_enventry) S_look(venv, q->u.table.name);
    if (x && x->kind == E_streamEntry) { // stream QUN
      DBUG_PRINT("info", ("stream QUN: %s\n",
              S_name(q->u.table.name)));

      getQunName(q, qunname);
      te = E_StreamEntry(q->alias, x->u.stream.ty, x->u.stream.size);
      EnvCpy(te, x);
      te->u.stream.sname = new_Symbol(qunname);
      te->u.stream.orig = x->u.stream.sname;
      S_enter(venv, q->alias, te);

      rc = transStreamGet2C(x, buf);
      exe = expTy_Seq(exe, buf);
      // make assignments
      rc = assignKeyData2C(x->u.stream.ty, qunname, buf, INDEX_STREAM);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transBaseQun",
            "assignKeyData2C");
        goto exit;
      }

      exe = expTy_Seq(exe, buf);
      exe->ty = x->u.stream.ty;
      srcs.push_back(q->u.table.name->name);
      goto exit;
    }
    if (x && x->kind == E_dynamicEntry) {
      x = (E_enventry) S_look(venv, x->u.dynamic.table);
    }
    if (!x || x->kind != E_varEntry) {
      rc = ERR_UNDEFINED_VARIABLE;
      EM_error(q->pos, rc, __LINE__, __FILE__, S_name(q->u.table.name));
      goto exit;
    }
    // other QUN
    DBUG_PRINT("info", ("table QUN: %s\n",
            S_name(q->u.table.name)));

    rc = addSqlSemCursorDec(sql, venv, q);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transQun",
          "addSqlSemCursorDec");
      goto exit;
    }
    // leave this one because it is for a table
    addSqlSemFirstEntryDec(sql, (void*) q);

    getQunName(q, qunname);

    te = E_VarEntry(q->alias, x->u.var.ty, x->u.var.size);
    EnvCpy(te, x); // Richard 2003/5
    te->u.var.sname = new_Symbol(qunname);
    S_enter(venv, q->alias, te);

    // get cursor
    sprintf(first_entry_name, "first_entry_%d", UID(q));
    sprintf(cursor_name, "%s_%d", S_name(q->alias), UID(q->ppnode));
    rc = transCursorGet2C(venv, first_entry_name, cursor_name, x, //->u.var.ty,
        bndp, bndpUpper, bndpLower, flag, buf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transQun",
          "transCursorGet2C");
      goto exit;
    }
    exe = expTy_Seq(exe, buf);

    sprintf(buf, "\nif (rc==0) {"
      "\nfirst_entry_%d = 0;", UID(q));

    exe = expTy_Seq(exe, buf);

    // make assignments
    getQunName(q, qunname);
    tabindex_t index = (tabindex_t) 1;
    if (x->u.var.index != (A_index) 0) {
      index = x->u.var.index->kind;
    }
    rc = assignKeyData2C(x->u.var.ty, qunname, buf, index);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transQun",
          "assignKeyData2C");
      goto exit;
    }

    exe = expTy_Seq(exe, buf);

    // reset cursor
    sprintf(buf, "\n} else if (rc == DB_NOTFOUND) {"
      "\n%s = 1;", first_entry_name);
    exe = expTy_Seq(exe, buf);
    if (isESL()) {
      sprintf(
          buf,
          "\n} else {"
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DBC->c_get() or IM_RELC->c_get()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          buf,
          "\n} else {"
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DBC->c_get() or IM_RELC->c_get()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(buf,
          "\n} else adlabort(rc, \"DBC->c_get() or IM_RELC->c_get()\");");
    }
    exe = expTy_Seq(exe, buf);
    exe->ty = x->u.var.ty;
  }
    break;
  case QUN_FUNCTION: {
    //printf("qun type func\n");
    /* UDF (table function) as QUN. */
    rc = transFunQun(venv, tenv, q, sql, dec, exe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transQun", "transFunQun");
      goto exit;
    }
  }
    break;
  case QUN_WINDOW: {
    //printf("qun type wind\n");
    if (!isESL() || (isESL() && sql->in_func == 1)) {
      rc = ERR_WINDOW_NOT_ALLOWED_ON_TABLE;
      EM_error(q->pos, rc, __LINE__, __FILE__);
      goto exit;
    }
    rc = transWinQun(venv, tenv, q, sql, dec, exe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transQun", "transWinQun");
      goto exit;
    }
  }
    break;
  case QUN_QUERY: {
    //printf("qun type query\n");
    A_exp query = q->u.query;

    /*      if (query->kind == A_selectExp) {
     A_exp nquery;

     rc = constructSelOp(venv, query, nquery);
     if (rc) {
     EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transQun", "constructSelOp");
     goto exit;
     }

     query = nquery;
     }
     */

    if (query->kind != A_sqloprExp) {
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__, "transQun()");
      goto exit;
    }

    /*printf("-- calling transSqlQuery\n");
     displayExp(query);
     printf("\n-- done\n");*/
    rc = transSqlQuery(venv, tenv, query->u.sqlopr, sql, dec, exe, name,
        aggregates, srcs, target_handle, cstmt);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transQun", "transSelOpr");
      goto exit;
    }
    //printf("back from transSQLQuery\n");

    /* I believe Richard wanted to comment this if for values in ESL, which
     does not seem right,
     but it gives problems in union -- Hetal */
    //if(!isESL() || isESLTest())  //Richard: for values() query on ESL
    if (isESLAggr() || !isESL() || query->u.sqlopr->kind != A_SQL_SEL)
      addSqlSemFirstEntryDec(sql, (void*) query->u.sqlopr);

    /* if qun does not have an alias, it is not referenced outside
     the node, and we don't add it to symbol table */
    if (q->alias) {
      te = E_VarEntry(q->alias, exe->ty, exe->size);
      getQunName(q, qunname);
      te->u.var.sname = new_Symbol(qunname);

      S_enter(venv, q->alias, te);
    }
  }
    break;
  }
  exit:
  //printf("done with qun\n");
  DBUG_RETURN(rc);
}

err_t transWinUpdate2C(S_table venv, S_table tenv, A_qun q, Sql_sem sql,
    T_expty &dec, // OUT
    T_expty &exe, // OUT
    vector<void*> aggregates) {
  SMLog::SMLOG(10, "Entering transWinUpdate2C");
  err_t rc = ERR_NONE;
  char qunname[80];
  E_enventry y;
  T_expty adec, aexe;
  char buf[MAX_STR_LEN];
  A_qun newQun;

  DBUG_ENTER("transWinUpdate2C");

  y = (E_enventry) S_look(venv, q->u.window.name);
  if (!y || y->kind != E_streamEntry) {
    rc = ERR_JOIN_STREAM_TABLE;
    EM_error(q->pos, rc, __LINE__, __FILE__,
        "Windows can only be defined on registered stream.");
    goto exit;
  }

  newQun = A_NameQun(0, q->u.window.name, q->alias);

  getQunName(newQun, qunname);

  sprintf(
      buf,
      "\nextern \"C\" int %s_%d_wm(bufferMngr *bm, int freeVars = 0, buffer* backBuf = (buffer*)NULL);"
        "\nextern \"C\" int %s_%d_wm(bufferMngr *bm, int freeVars, buffer* backBuf)"
        "\n{"
        "\nint rc = 0;"
        "\nDBT key, data, windata;"
        "\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];"
        "\nstruct timeval atime;"
        "\nwindowBuf *%s_%d_winbuf;"
        "\ndata.data = datadata;"
        "\nkey.data = keydata;", queryName, UID(q->ppnode), queryName,
      UID(q->ppnode), S_name(q->u.window.name), UID(q->ppnode));
  sql->global = expTy_Seq(sql->global, buf);

  //instead call transQun and use ifs for declarations
  sprintf(buf, "\nbuffer *%s = bm->lookup(\"%s\");"
    "\nif (!%s || ((rc=%s->get(&data, &atime, &key)) == DB_NOTFOUND)) {"
    "\nrc = s_no_input;"
    "\ngoto doClose;"
    "\n}", S_name(y->u.stream.sname), S_name(y->u.stream.sname), S_name(
      y->u.stream.sname), S_name(y->u.stream.sname));
  sql->global = expTy_Seq(sql->global, buf);

  //First make sure winbuf is initialized
  sprintf(buf, "\n%s_%d_winbuf = (windowBuf*)bm->lookup(\"%s_%d_winbuf\");"
    "\nif(!%s_%d_winbuf) {", S_name(q->u.window.name), UID(q->ppnode),
      queryName, UID(q->ppnode), S_name(q->u.window.name), UID(q->ppnode));
  sql->global = expTy_Seq(sql->global, buf);
  if (isESL()) {
    sprintf(
        buf,
        "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: windowBuffer %s_%d_winbuf not found\");"
          "\nreturn s_failure;"
          "\n}", getUserName(), getQueryName(), queryName, UID(q->ppnode));
  } else if (isESLAggr()) {
    sprintf(
        buf,
        "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: windowBuffer %s_%d_winbuf not found\");"
          "\nreturn;"
          "\n}", getUserName(), getAggrName(), queryName, UID(q->ppnode));
  } else {
    sprintf(buf, "\nadlabort(rc, \"windowBuffer %s_%d_winbuf not found\");"
      "\n}", queryName, UID(q->ppnode));
  }
  sql->global = expTy_Seq(sql->global, buf);

  if (y->u.stream.tk == tk_none) { //for latent timestamps put the current sys time in atime
    sprintf(buf, "\nstruct timezone tz;"
      "\nmemcpy(&atime, 0, sizeof(atime));"
      "\nmemcpy(&tz, 0, sizeof(tz));"
      "\ngettimeofday(&atime, &tz);");
    sql->global = expTy_Seq(sql->global, buf);
  }

  //instead use transInsert2C
  if (q->u.window.range->type == TIME_RANGE) {
    sprintf(buf, "\n%s_%d_winbuf->updateTupleID(&atime);", S_name(
        q->u.window.name), UID(q->ppnode));
    sql->global = expTy_Seq(sql->global, buf);
  } else {
    sprintf(buf, "\n%s_%d_winbuf->updateTupleID();"
      "\n%s_%d_winbuf->setCurrentTimestamp(&atime);", S_name(q->u.window.name),
        UID(q->ppnode), S_name(q->u.window.name), UID(q->ppnode));
    sql->global = expTy_Seq(sql->global, buf);
  }

  sprintf(buf, "\nif((rc = %s_%d_winbuf->put(&data))!=0) {", S_name(
      q->u.window.name), UID(q->ppnode));
  sql->global = expTy_Seq(sql->global, buf);
  if (isESL()) {
    sprintf(
        buf,
        "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: winbuf->put()\");"
          "\nreturn s_failure;"
          "\n}", getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        buf,
        "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: winbuf->put()\");"
          "\nreturn;"
          "\n", getUserName(), getAggrName());
  } else {
    sprintf(buf, "\nadlabort(rc, \"winbuf->put()\");"
      "\n");
  }
  sql->global = expTy_Seq(sql->global, buf);

  sql->global = expTy_Seq(sql->global, "\ndoClose:");
  sql->global = expTy_Seq(sql->global,
      "\nexit:\nrc=s_success; \nreturn rc;\n};");

  exit: DBUG_RETURN(rc);
}

err_t transWinQun(S_table venv, S_table tenv, A_qun q, Sql_sem sql,
    T_expty &dec, // OUT
    T_expty &exe, // OUT
    vector<void*> aggregates) {
  SMLog::SMLOG(10, "Entering transWinQun");
  err_t rc = ERR_NONE;
  E_enventry x;
  E_enventry y;
  char buf[MAX_STR_LEN];
  char qunname[80];
  char first_entry_name[MAX_STR_LEN];
  char cursor_name[MAX_STR_LEN];
  E_enventry te;
  bufferMngr *bm = bufferMngr::getInstance();
  char bufferName[80];

  DBUG_ENTER("transWinQun");

  // check if the stream is defined
  x = (E_enventry) S_look(venv, q->u.window.name);
  if (!x || x->kind != E_streamEntry) {
    rc = ERR_JOIN_STREAM_TABLE;
    EM_error(q->pos, rc, __LINE__, __FILE__,
        "Windows can only be defined on registered stream.");
    goto exit;
  }

  y = (E_enventry) S_look(venv, q->u.window.respectTo);
  if (!y || y->kind != E_streamEntry) {
    rc = ERR_JOIN_STREAM_TABLE;
    EM_error(q->pos, rc, __LINE__, __FILE__,
        "Defined window must be respect to a registered stream.");
    goto exit;
  }

  rc = transWinUpdate2C(venv, tenv, q, sql, dec, exe, aggregates);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transWinQun",
        "transWinUpdate2C");
    goto exit;
  }

  //Instead create a windowedBuf class very similar to winbuf, plus
  // it will extend from buffer class so we can put it in the BufferMngr
  // that way driver can also work with it for back tracking on joins
  //  q->u.window.range->size,	    (q->u.window.range->type == TIME_RANGE)?	      "_ADL_WIN_TIME":"_ADL_WIN_ROW");
  sprintf(bufferName, "%s_%d_winbuf", queryName, UID(q->ppnode));
  bm->create(bufferName, WINBUF, q->u.window.range->size,
      (q->u.window.range->type == TIME_RANGE) ? _ADL_WIN_TIME : _ADL_WIN_ROW);

  sprintf(buf, "\nwindowBuf* %s_%d_winbuf;"
    "\n%s_%d_winbuf = (windowBuf*)bm->lookup(\"%s_%d_winbuf\");"
    "\nif(!%s_%d_winbuf) {", S_name(q->u.window.name), UID(q->ppnode), S_name(
      q->u.window.name), UID(q->ppnode), queryName, UID(q->ppnode), S_name(
      q->u.window.name), UID(q->ppnode));
  sql->predec = expTy_Seq(sql->predec, buf);

  if (isESL()) {
    sprintf(
        buf,
        "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: windowBuffer %s_%d_winbuf not found\");"
          "\nreturn s_failure;"
          "\n}", getUserName(), getQueryName(), queryName, UID(q->ppnode));
  } else if (isESLAggr()) {
    sprintf(
        buf,
        "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: windowBuffer %s_%d_winbuf not found\");"
          "\nreturn;"
          "\n}", getUserName(), getAggrName(), queryName, UID(q->ppnode));
  } else {
    sprintf(buf, "\nadlabort(rc, \"windowBuffer %s_%d_winbuf not found\");"
      "\n}", queryName, UID(q->ppnode));
  }
  sql->predec = expTy_Seq(sql->predec, buf);

  //translate rest of the qun,
  //Some how need to figure out the latest time stamp of this qun
  // and the qun it is joined with, not quite clear how
  // first check if x == y if so then nothing to do keep going
  //  else find current timestamp of y and compare with x's current timestamp
  //      if x.timestamp < y.timestamp then
  //           set back buffer
  //           return unable to process tuple
  //      else  continue
  if (x != y) {
    sprintf(
        buf,
        "\nstruct timeval tv_%d;"
          "\nstruct timeval tv_%d;"
          "\nif(bm->get(\"%s\", &tv_%d) == DB_NOTFOUND) {"
          "\nrc = s_no_input;"
          "\ngoto doClose;"
          "\n}"
          "\nif(bm->lookup(\"%s\") == NULL) {"
          "\nrc = s_no_input;"
          "\ngoto doClose;"
          "\n}"
          "\nmemcpy(&tv_%d, bm->lookup(\"%s\")->mant, sizeof(timeval));"
          "\nif(timeval_cmp(tv_%d, tv_%d) < 0) {"
          "\n//%s_%d_winbuf->setCurrentTimestamp(&tv_%d); //streams may have external timestamp, so don't do this"
          "\nbackBuf = bm->lookup(\"%s_%d_winbuf\");"
          "\nrc = s_no_input;"
          "\ngoto doClose;"
          "\n}", UID(q), UID(y), S_name(y->u.stream.orig), UID(y), S_name(
            q->u.window.name), UID(q), S_name(q->u.window.name), UID(q),
        UID(y), S_name(q->u.window.name), UID(q->ppnode), UID(y), queryName,
        UID(q->ppnode));
    exe = expTy_Seq(exe, buf);
  }

  //May be we should be updating the timestamp here
  // with timestamp of the tuple in join stream (tv_UID(y))
  sprintf(buf, "\nwhile(%s_%d_winbuf->hasExpired()) {"
    "\n%s_%d_winbuf->pop();"
    "\n}", S_name(q->u.window.name), UID(q->ppnode), S_name(q->u.window.name),
      UID(q->ppnode));
  exe = expTy_Seq(exe, buf);

  rc = addSqlSemCursorDec(sql, venv, q);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transWinQun",
        "addSelSemCursor");
    goto exit;
  }

  getQunName(q, qunname);
  te = E_VarEntry(x->u.stream.sname, x->u.stream.ty, x->u.stream.size);
  te->u.var.sname = new_Symbol(qunname);
  S_enter(venv, q->alias, te);

  addSqlSemFirstEntryDec(sql, (void*) q);

  sprintf(first_entry_name, "first_entry_%d", UID(q));
  sprintf(cursor_name, "%s_%d_winbufc", S_name(q->alias), UID(q->ppnode));

  rc = transCursorGet2C(venv, first_entry_name, cursor_name, te, (Tr_exp*) 0,
      (Tr_exp*) 0, (Tr_exp*) 0, BOUND_NONE, buf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transWinQun",
        "transCursorGet2C");
    goto exit;
  }
  exe = expTy_Seq(exe, buf);

  sprintf(buf, "\nif (rc==0) {"
    "\nfirst_entry_%d = 0;", UID(q));

  exe = expTy_Seq(exe, buf);

  // make assignments
  getQunName(q, qunname);
  rc = assignKeyData2C(x->u.stream.ty, qunname, buf, INDEX_BTREE);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transWinQun",
        "assignKeyData2C");
    goto exit;
  }

  exe = expTy_Seq(exe, buf);

  // reset cursor
  sprintf(buf, "\n} else if (rc == DB_NOTFOUND) {"
    "\n%s = 1;", first_entry_name);
  exe = expTy_Seq(exe, buf);

  if (isESL()) {
    sprintf(
        buf,
        "\n} else {"
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DBC->c_get() or IM_RELC->c_get()\");"
          "\nreturn s_failure;"
          "\n}", getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        buf,
        "\n} else {"
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DBC->c_get() or IM_RELC->c_get()\");"
          "\nreturn;"
          "\n}", getUserName(), getAggrName());
  } else {
    sprintf(buf, "\n} else adlabort(rc, \"DBC->c_get() or IM_RELC->c_get()\");");
  }
  exe = expTy_Seq(exe, buf);

  exe->ty = x->u.stream.ty;

  exit: DBUG_RETURN(rc);
}

/*
 1) open all the cursors that are used in a sql statement;
 2) initialize all node_index to 0
 3) initialize all first_entry to 1
 */
void sqlStatementGlobalInit(S_table venv, Sql_sem sql, T_expty &dec,
    T_expty &exe) {
  SMLog::SMLOG(10, "Entering sqlStatementGlobalInit");
  //    int i;
  //    char cursorname[128];
  //    char buf[MAX_STR_LEN];

  /*exe = expTy_Seq(exe, sql->predec);*/

  dec = expTy_Seq(dec, sql->predec);
  sql->predec = (T_expty) 0;

  exe = expTy_Seq(exe, sql->preinit);
  sql->preinit = (T_expty) 0;

  /*
   for (i=0; i<sql->cursor_list->length; i++) {
   A_qun qun = (A_qun)getNthElementList(sql->cursor_list, i);
   E_enventry x = (E_enventry)S_look(venv, qun->u.name);
   char *name;

   if (x && x->kind == E_varEntry && x->u.var.internalname)
   name = S_name(x->u.var.internalname);
   else
   name = S_name(qun->u.name);

   sprintf(cursorname, "%s_%d", S_name(qun->alias), UID(qun->ppnode));

   transCursorDec2C(cursorname, buf);
   dec = expTy_Seq(dec, buf);

   transCursorInit2C(name, cursorname, buf);
   exe = expTy_Seq(exe, buf);
   }
   for (i=0; i<sql->index_list->length; i++) {
   sprintf(buf, "\nint index_%d = 0;",
   UID(getNthElementList(sql->index_list, i)));
   dec = expTy_Seq(dec, buf);
   }
   for (i=0; i<sql->first_entry_list->length; i++) {
   sprintf(buf, "\nint first_entry_%d = 1;",
   UID(getNthElementList(sql->first_entry_list, i)));
   dec = expTy_Seq(dec, buf);
   }
   */
}

/* close all the cursors used in a sql statement */
void sqlStatementGlobalCleanUp(Sql_sem sql, T_expty &exe) {
  SMLog::SMLOG(10, "Entering sqlStatementGlobalCleanUp ");
  /*  int i;
   char buf[MAX_STR_LEN], cursorname[128];

   for (i=0; i<sql->cursor_list->length; i++) {
   A_qun qun = (A_qun)getNthElementList(sql->cursor_list, i);

   sprintf(cursorname, "%s_%d", S_name(qun->alias), UID(qun->ppnode));
   transCursorClose2C(cursorname, buf);
   exe = expTy_Seq(exe, buf);
   }
   //  exe = expTy_Seq(exe, "\n}");
   */
  exe = expTy_Seq(exe, sql->preclean);
  sql->preclean = (T_expty) 0;
  if (sql->afterpreclean) {
    exe = expTy_Seq(exe, sql->afterpreclean);
    sql->afterpreclean = (T_expty) 0;
  }
}

/*
 * generate C code for SEL, UNION, GB, EXCEPT, INTERSECT 
 * return type is declared.
 */
err_t transSqlQuery(S_table venv, S_table tenv, A_sqlopr a, Sql_sem sql,
    T_expty &dec, T_expty &exe, char *name, vector<void*> aggregates, vector<
        string> &srcs, char* target_handle, cStmt*& cstmt)

{
  SMLog::SMLOG(10, "Entering transSqlQuery");
  err_t rc = ERR_NONE;
  char buf[MAX_STR_LEN];
  int i, j;
  A_list retfields;
  bufferMngr *bm = bufferMngr::getInstance();

  DBUG_ENTER("transSqlQuery");

  //printf("in transSqlQuery %d\n", a->kind);

  dec = exe = (T_expty) 0;

  switch (a->kind) {
  case A_SQL_ORDER: {
    rc = transOrderOpr(venv, tenv, a, sql, dec, exe, name, aggregates, srcs);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
          "transOrderOpr");
      goto exit;
    }
    rc = declareQun2C(name, exe->ty, buf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
          "declareQun2C");
      goto exit;
    }
    dec = expTy_Seq(dec, buf);
  }
    break;
  case A_SQL_SEL: {
    rc = transSelOpr(venv, tenv, a, sql, dec, exe, name, aggregates, srcs,
        cstmt);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
          "transSelOpr");
      goto exit;
    }
    rc = declareQun2C(name, exe->ty, buf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
          "declareQun2C");
      goto exit;
    }
    dec = expTy_Seq(dec, buf);
  }
    break;
  case A_SQL_UNION: {
    if (isESL()) {
      char oldQueryName[1024];
      char tempQueryName[1024];
      char branchname[128];
      T_expty qdec, qexe;
      T_expty qdec1, qexe1;
      Ty_ty first_ty = (Ty_ty) 0;
      qdec = qexe = (T_expty) 0;
      qdec1 = qexe1 = (T_expty) 0;

      memset(oldQueryName, '\0', 1024);
      memset(tempQueryName, '\0', 1024);

      sprintf(oldQueryName, "%s", queryName);

      for (i = 0; i < a->jtl_list->length; i++) {
        A_qun qun = (A_qun) getNthElementList(a->jtl_list, i);
        if (qun->kind == QUN_QUERY) {
          sprintf(tempQueryName, "%s%d", oldQueryName, i);
          setQueryName(tempQueryName);

          sprintf(branchname, "unionqun_%d", UID(qun));

          /*the following call is to get the right return types only
           the actual translation is down below */
          rc = transQun(venv, tenv, qun, sql, qdec, qexe, branchname,
              (Tr_exp*) 0, (Tr_exp*) 0, (Tr_exp*) 0, BOUND_NONE, aggregates,
              srcs, target_handle, cstmt);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
                "transQun");
            goto exit;
          }

          /*change qun->u.query to have srcs buffer
           I think we can change it to "insert into srcs buf"
           but could this occur in some other situation
           also how do we change it in to an insert */
          S_symbol tar = S_Symbol(queryName);
          buffer* subStmtBo = bm->lookup(queryName);
          if (!subStmtBo) {
            bm->create(queryName);
          }
          E_enventry x = (E_enventry) S_look(venv, S_Symbol(target_handle));
          if (strcmp(target_handle, "stdout") == 0 || (x && x->kind
              == E_varEntry && x->u.var.isStdout == 1)) {
            S_enter(venv, tar, E_VarEntry(tar, Ty_Nil(), 0, TAB_LOCAL,
                (A_index) 0, 0, 1));
          } else {
            if (!x || x->kind != E_streamEntry) {
              rc = ERR_UNDEFINED_VARIABLE;
              EM_error(qun->pos, rc, __LINE__, __FILE__, target_handle);
              goto exit;
            }

            S_enter(venv, tar, E_StreamEntry(tar, x->u.stream.ty,
                x->u.stream.size, x->u.stream.source, x->u.stream.target,
                x->u.stream.tk, x->u.stream.timekey, x->u.stream.isBuiltin,
                x->u.stream.port));
          }

          A_qun target = A_NameQun(qun->pos, tar, tar);
          A_qun source = A_QueryQun(qun->pos, qun->alias, qun->u.query);
          A_list jtl_li = A_List(0, A_QUN);
          appendAList(jtl_li, (void*) target);
          appendAList(jtl_li, (void*) source);
          A_exp tempSqlOpr = A_SqlOprExp(qun->pos, A_SQL_INSERT, 0, (A_list) 0,
              jtl_li, (A_list) 0);

          rc = transExp(venv, tenv, tempSqlOpr, sql, qdec1, qexe1, aggregates,
              target_handle, cstmt);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
                "transExp");
            goto exit;
          }

          if (!subStmtBo && (qun->u.query->kind == A_sqloprExp
              && qun->u.query->u.sqlopr->kind == A_SQL_UNION)) {
            bm->kill(queryName);
          }
          if (i == 0) {
            first_ty = qexe->ty;
          } else if (!((strcmp(target_handle, "stdout") == 0) || (x && x->kind
              == E_varEntry && x->u.var.isStdout == 1)) && !makeCompatibleType(
              first_ty, qexe->ty)) {
            rc = ERR_UNMATCH_UNION_TYPE;
            EM_error(qun->pos, rc, __LINE__, __FILE__);
            goto exit;
          }

          if (qdec1)
            exe = expTy_Seq(exe, qdec1);
          exe = expTy_Seq(exe, qexe1);
        }
      }
      setQueryName(oldQueryName);

      /* return type of UNION */
      retfields = A_List(0, A_FIELD);
      for (j = 0; j < first_ty->u.record->length; j++) {
        char field[128];
        Ty_field f, n;
        sprintf(field, "field_%d", j);

        f = (Ty_field) getNthElementList(first_ty->u.record, j);
        n = Ty_Field(new_Symbol(field), f->ty, f->size);
        appendElementList(retfields, (nt_obj_t*) n);
      }

      exe->ty = Ty_Record(retfields);
      rc = declareQun2C(name, exe->ty, buf);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
            "declareQun2C");
        goto exit;
      }
      dec = expTy_Seq(dec, buf);
    }
    /*
     if (where_cond) {
     do {
     if (first_entry==1) index = 0;
     switch (index) {
     case 0:
     [qun0]
     if (rc==0) assignValue;
     break;
     case 1:
     [qun1]
     if (rc==0) assignValue;
     break;
     case 2:
     [qun2]
     if (rc==0) assignValue;
     break;
     }
     if (rc == DB_NOTFOUND) index++;
     } while (rc == DB_NOTFOUND && index < 3);
     if (rc == DB_NOTFOUND)
     first_entry = 1;
     else
     first_entry = 0;
     }
     */
    else //!isESL()
    {
      char branchname[128], abuf[MAX_STR_LEN];
      T_expty qdec, qexe;
      Ty_ty first_ty = (Ty_ty) 0;
      qdec = qexe = (T_expty) 0;

      //        appendElementList(sql->index_list, (nt_obj_t*)a);
      addSqlSemIndexDec(sql, (void*) a);

      if (a->prd_list && a->prd_list->length > 0) {
        for (i = 0; i < a->prd_list->length; i++) {
          T_expty d, e;
          A_exp prd = (A_exp) getNthElementList(a->prd_list, i);
          rc = transExp(venv, tenv, prd, sql, d, e, aggregates);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
                "transExp");
            goto exit;
          }
          sprintf(buf, "\nif (!(%s)) {"
            "\nrc = DB_NOTFOUND;"
            "\ngoto next_%d;"
            "\n}", e->exp, UID(a));
          exe = expTy_Seq(exe, buf);
        }
      }

      sprintf(abuf, "\nif (first_entry_%d==1) index_%d = 0;"
        "\ndo {"
        "\nswitch (index_%d) {", UID(a), UID(a), UID(a));

      exe = expTy_Seq(exe, abuf);

      for (i = 0; i < a->jtl_list->length; i++) {
        A_qun qun = (A_qun) getNthElementList(a->jtl_list, i);
        sprintf(branchname, "unionqun_%d", UID(qun));

        rc = transQun(venv, tenv, qun, sql, qdec, qexe, branchname,
            (Tr_exp*) 0, (Tr_exp*) 0, (Tr_exp*) 0, BOUND_NONE, aggregates,
            srcs, NULL, cstmt);

        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
              "transQun");
          goto exit;
        }

        if (i == 0) {
          first_ty = qexe->ty;
        } else if (!makeCompatibleType(first_ty, qexe->ty)) {
          rc = ERR_UNMATCH_UNION_TYPE;
          EM_error(qun->pos, rc, __LINE__, __FILE__);
          goto exit;
        }
        sprintf(abuf, "\ncase %d:\n{", i);
        exe = expTy_Seq(exe, abuf);

        //sprintf(abuf, "\nprintf(\"case2 %d\\n\");fflush(stdout);", i);
        //exe = expTy_Seq(exe, abuf);
        /* make assignments */
        *buf = '\0';
        for (j = 0; j < qexe->ty->u.record->length; j++) {
          Ty_field f = (Ty_field) getNthElementList(qexe->ty->u.record, j);

          if (f->ty->kind == Ty_string) {
            sprintf(abuf, "\nmemcpy(%s.field_%d, %s.%s, %d);"
              "\n%s.field_%d[%d] = 0;", name, j, branchname, S_name(f->name),
                f->size, name, j, f->size);//string length
          } else if (f->ty->kind == Ty_timestamp) {
            sprintf(abuf, "\nmemcpy(&(%s.field_%d), &(%s.%s), %d);", name, j,
                branchname, S_name(f->name), sizeof(struct timeval));
          } else {
            sprintf(abuf, "\n%s.field_%d = %s.%s;", name, j, branchname,
                S_name(f->name));
          }
          strcat(buf, abuf);
        }

        if (qdec)
          exe = expTy_Seq(exe, qdec);
        exe = expTy_Seq(exe, qexe);
        exe = expTy_Seq(exe, "\nif (rc==0) {");
        exe = expTy_Seq(exe, buf);
        exe = expTy_Seq(exe, "\n}\n}\nbreak;");
      }
      sprintf(buf, "\n}/* end of switch */"
        "\nif (rc == DB_NOTFOUND) index_%d++;"
        "\n} while (rc == DB_NOTFOUND && index_%d < %d);"
        "\nnext_%d:"
        "\nif (rc == DB_NOTFOUND) {\nfirst_entry_%d = 1;\n}"
        "\nelse {\nfirst_entry_%d = 0;\n}", UID(a), UID(a),
          a->jtl_list->length, UID(a), UID(a), UID(a));
      exe = expTy_Seq(exe, buf);

      /* return type of UNION */
      retfields = A_List(0, A_FIELD);
      for (j = 0; j < first_ty->u.record->length; j++) {
        char field[128];
        Ty_field f, n;
        sprintf(field, "field_%d", j);

        f = (Ty_field) getNthElementList(first_ty->u.record, j);
        n = Ty_Field(new_Symbol(field), f->ty, f->size);
        appendElementList(retfields, (nt_obj_t*) n);
      }

      exe->ty = Ty_Record(retfields);
      rc = declareQun2C(name, exe->ty, buf);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
            "declareQun2C");
        goto exit;
      }
      dec = expTy_Seq(dec, buf);
    }
  }
    break;
  case A_SQL_GB: {
    //printf("calling transGBOpr\n");
    rc = transGBOpr(venv, tenv, a, sql, dec, exe, name, aggregates, srcs);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
          "transGBOpr");
      goto exit;
    }
    rc = declareQun2C(name, exe->ty, buf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
          "declareQun2C");
      goto exit;
    }
    dec = expTy_Seq(dec, buf);
  }
    break;
  case A_SQL_EXCEPT:
  case A_SQL_INTERSECT:
    break;
  }
  exit: DBUG_RETURN(rc);
}

/*
 * Function findSourceInQun() tests if a table is used in a qun.  This
 * is just a makeshift. In the future, we need to have a function for
 * more general cases, namely, finding an object within another
 * object.  */
err_t findSourceInQun(S_symbol source, A_qun qun, int *result) {
  SMLog::SMLOG(10, "Entering findSourceInQun");
  err_t rc = ERR_NONE;

  *result = 0;

  switch (qun->kind) {
  case QUN_NAME:
    if (qun->u.table.name == source)
      *result = 1;
    break;
  case QUN_FUNCTION:
    break;
  case QUN_QUERY: {
    A_exp a = qun->u.query;
    int i;

    if (a->kind == A_sqloprExp) {
      A_sqlopr opr = a->u.sqlopr;

      switch (opr->kind) {
      case A_SQL_SEL:
      case A_SQL_GB:
      case A_SQL_UNION:
        for (i = 0; opr->jtl_list && i < opr->jtl_list->length; i++) {
          A_qun q = (A_qun) getNthElementList(opr->jtl_list, i);
          rc = findSourceInQun(source, q, result);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "findSourceInQun",
                "findSourceInQun");
            goto exit;
          }
        }
        break;
      case A_SQL_EXCEPT:
      case A_SQL_INTERSECT:
        rc = ERR_NTSQL_INTERNAL;
        EM_error(0, rc, __LINE__, __FILE__,
            "type not implemented in findSourceInQun()");
        goto exit;
      }
    } else {
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__,
          "unexpected type in findSourceInQun()");
      goto exit;
    }
  }
    break;
  }
  exit: return rc;
}

int isQunJoin(A_qun qun) {
  SMLog::SMLOG(10, "Entering isQunJoin");
  //return 1 if join, return 0 if union, return 2 if other
  if (qun->kind == QUN_WINDOW) {
    return 1;
  }
  if (qun->kind != QUN_QUERY)
    return 2;

  A_exp query = qun->u.query;
  A_sqlopr sqlopr = query->u.sqlopr;

  if (sqlopr->kind == A_SQL_UNION)
    return 0;

  if (sqlopr->kind == A_SQL_SEL) {
    if (sqlopr->jtl_list->length > 1)
      return 1;
  }

  return 2;
}

void getSubStmtName(char* oldQueryName, int index, int size, char* dest) {
  int numberOfZeros = 0;

  sprintf(dest, "%s", oldQueryName);
  numberOfZeros = size - index - 1;

  for (int i = 0; i < numberOfZeros; i++) {
    dest = strcat(dest, "0");
  }

  if (index != 0) {
    dest = strcat(dest, "1");
  }
}

err_t transSqlStatement(S_table venv, S_table tenv, Sql_sem sql, A_sqlopr s,
    T_expty &dec, T_expty &exe, vector<void*> aggregates, vector<string> &srcs, //source buffer names, used in ESL
    char* target_handle, cStmt* cstmt = NULL) {
  SMLog::SMLOG(10, "Entering transSqlStatement");
  int i;
  tabscope_t kind; /* memory, local, berkeley db */
  char buf[MAX_STR_LEN], target_name[128];
  err_t rc = ERR_NONE;
  T_expty adec, aexe;
  bufferMngr *bm = bufferMngr::getInstance();
  DrvMgr *dm = DrvMgr::getInstance();
  querySchdl *qs = querySchdl::getInstance();

  DBUG_ENTER("transSqlStatement");

  sql->top_sqlopr = s;

  S_beginScope(venv);
  S_beginScope(tenv);

  switch (s->kind) {
  case A_SQL_LOAD: {
    SMLog::SMLOG(12, "\tA_SQL_LOAD");
    exe = expTy_Seq((T_expty) 0, "\nrc = 0;"
      "\n_adl_cursqlcode = 1 /* ASSUME GUITY */;");

    A_qun source = (A_qun) getNthElementList(s->jtl_list, 0);
    A_qun target = (A_qun) getNthElementList(s->jtl_list, 1);

    char *filename = S_name(source->u.table.name);
    char *name;
    E_enventry x;

    x = (E_enventry) S_look(venv, target->u.table.name);
    if (!x || x->kind != E_varEntry) {
      rc = ERR_UNDEFINED_VARIABLE;
      EM_error(target->pos, rc, __LINE__, __FILE__,
          S_name(target->u.table.name));
      goto exit;
    }

    if (x->u.var.iname)
      name = S_name(x->u.var.iname);
    else
      name = S_name(target->u.table.name);

    rc = transLoad2C(venv, name, /* handle name */
    S_name(target->u.table.name), /* database name */
    filename, /* source data */
    x,
    //		       x->u.var.ty,		/* tuple type */
        buf
    //		       x->u.var.scope,		/* memory, local, berkeleydb */
        //		       x->u.var.dbtype
        );
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
          "transLoad2C");
      goto exit;
    }
    exe = expTy_Seq(exe, buf);
  }
    break;
  case A_SQL_INSERT: {
    SMLog::SMLOG(12, "\tA_SQL_INSERT");
    A_qun target = (A_qun) getNthElementList(s->jtl_list, 0);
    A_qun source = (A_qun) getNthElementList(s->jtl_list, 1);

    int mode;
    int dynamic = 0;
    S_symbol rawname;

    /* target */
    E_enventry x = (E_enventry) 0; // target env

    Ty_ty ty = 0;
    E_enventry newx;

    x = (E_enventry) S_look(venv, target->u.table.name);
    if (target->u.table.name == S_Symbol("stdout") || (x && x->kind
        == E_varEntry && x->u.var.isStdout == 1)) {
      mode = USE_STDOUT;
      target_handle = S_name(target->u.table.name);
      x = (E_enventry) 0;
    } else {
      int found;
      if (x && x->kind == E_dynamicEntry) {
        rawname = x->u.dynamic.rawname;
        x = (E_enventry) S_look(venv, x->u.dynamic.table);
        dynamic = 1;
      }
      if (!x || (x->kind != E_varEntry && x->kind != E_streamEntry)) {
        rc = ERR_UNDEFINED_VARIABLE;
        EM_error(target->pos, rc, __LINE__, __FILE__, S_name(
            target->u.table.name));
        goto exit;
      }
      ty = x->kind == E_varEntry ? x->u.var.ty : x->u.stream.ty;

      // check if source is in target
      rc = findSourceInQun(target->u.table.name, source, &found);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
            "transQun");
        goto exit;
      }

      mode = (found) ? USE_TEMP : USE_DIRECT;
      if (x->kind == E_streamEntry && found) {
        rc = ERR_STREAM;
        EM_error(0, ERR_STREAM, __LINE__, __FILE__,
            "insert into the same stream");
        goto exit;
      }

      if (isAdHoc() || isESLAggr()) {
        //special initialization for adhoc insert into values
        char dbname[80];
        if (x && x->kind == E_varEntry && x->u.var.source) {
          sprintf(dbname, "\"./%s\"", S_name(x->u.var.source));
          tabindex_t index = (tabindex_t) 1;
          if (x->u.var.index != (A_index) 0)
            index = x->u.var.index->kind;
          rc = transTabInit2C(S_name(target->u.table.name), dbname,
              x->u.var.haskey, buf, x->u.var.scope, index, Ty_nil,
              x->u.var.inaggr, x->u.var.isBuffer, target->u.table.name);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
                "transTabInit2C");
            goto exit;
          }
          /*sql->preinit = expTy_Seq(sql->preinit, "\nif(");
           sql->preinit = expTy_Seq(sql->preinit, S_name(target->u.name));
           sql->preinit = expTy_Seq(sql->preinit, "==NULL) {");*/
          sql->preinit = expTy_Seq(sql->preinit, buf);
          //sql->preinit = expTy_Seq(sql->preinit, "\n//printf(\"got here\\n\");");
          //sql->preinit = expTy_Seq(sql->preinit, "\n}");

          /*This deletes the table before its cursors*/
          transTabClose2C(S_name(target->u.table.name), S_name(
              target->u.table.name), buf, x->u.var.scope, index);
          sql->afterpreclean = expTy_Seq(sql->afterpreclean, buf);
        } else if (x && x->kind == E_varEntry && !x->u.var.inaggr
            && x->u.var.scope == TAB_MEMORY && dynamic == 0) {
          tabindex_t index = (tabindex_t) 1;
          if (x->u.var.index != (A_index) 0)
            index = x->u.var.index->kind;
          char* name1;
          if (x->u.var.iname)
            name1 = S_name(x->u.var.iname);
          else
            name1 = S_name(target->u.table.name);

          sprintf(dbname, "\"_adl_db_%s\"", name1);

          rc = transTabInit2C(S_name(target->u.table.name), dbname,
              x->u.var.haskey, buf, x->u.var.scope, index, Ty_nil,
              x->u.var.inaggr, x->u.var.isBuffer, target->u.table.name);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
                "transTabInit2C");
            goto exit;
          }
          sql->preinit = expTy_Seq(sql->preinit, buf);
        }

      }

      /* here we should look at the target qun, if it has dynamic then
       use inMemTables... */
      if (x->kind == E_streamEntry) {
        target_handle = S_name(x->u.stream.sname);
      } else if (x->kind == E_varEntry && x->u.var.iname)
        target_handle = S_name(x->u.var.iname);
      else if (target_handle == NULL) {
        if (dynamic == 1) {
          sprintf(buf, "\nchar tabName_%d[200];", UID(target));
          sql->preinit = expTy_Seq(sql->preinit, buf);
          sprintf(buf, "\nsprintf(tabName_%d, \"%%s%s\", _modelId);",
              UID(target), S_name(rawname));
          sql->preinit = expTy_Seq(sql->preinit, buf);
          target_handle = (char*) malloc(200);
          sprintf(target_handle,
              "((IM_REL*)inMemTables->operator[](tabName_%d))", UID(target));
        } else
          target_handle = S_name(target->u.table.name);
      }
    }

    /* source */
    sprintf(target_name, "insert_%d", UID(s));
    rc = transQun(venv, tenv, source, sql, adec, aexe, target_name,
        (Tr_exp*) 0, (Tr_exp*) 0, (Tr_exp*) 0, BOUND_NONE, aggregates, srcs,
        target_handle, cstmt);

    if ((isESL() && sql->in_func != 1) && source->kind == QUN_QUERY
        && source->u.query->u.sqlopr->kind == A_SQL_SEL && adec) {
      sql->predec = expTy_Seq(sql->predec, adec->exp);
    }

    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
          "transQun");
      goto exit;
    }

    if (mode != USE_STDOUT && !extendType(aexe->ty, ty)) {
      rc = ERR_INCOMPATIBLE_TYPE;
      EM_error(s->pos, rc, __LINE__, __FILE__);
      goto exit;
    }

    if (!isESL() || (isESL() && sql->in_func == 1)) {
      exe = expTy_Seq((T_expty) 0, "\nrc = 0;"
        "\n_adl_cursqlcode = 1 /* ASSUME GUITY */;");

      rc = declareQun2C(target_name, aexe->ty, buf);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
            "declareQun2C");
        goto exit;
      }

      dec = expTy_Seq(dec, buf);
    }

    sqlStatementGlobalInit(venv, sql, dec, exe);

    if (x) {
      if (ty->kind != Ty_record || aexe->ty->kind != Ty_record
          || getNumOfExplictFields(ty->u.record) != aexe->ty->u.record->length) {
        rc = ERR_UNMATCH_CARDINALITY;
        EM_error(0, rc, __LINE__, __FILE__, "SQL insert operation");
      }

      for (i = 0; i < aexe->ty->u.record->length; i++) {
        Ty_field srcf, tgtf;

        srcf = (Ty_field) getNthElementList(aexe->ty->u.record, i);
        tgtf = (Ty_field) getNthElementList(ty->u.record, i);

        /* TO DO: check compatibility of aexe->ty and ty */

        srcf->iskey = tgtf->iskey;
        srcf->isTimekey = tgtf->isTimekey;
      }
      if (x->kind == E_streamEntry) {
        newx = E_StreamEntry(S_symbol(""), aexe->ty, aexe->size,

        x->u.stream.source, x->u.stream.target, x->u.stream.tk,
            x->u.stream.timekey);
      } else {
        newx = E_VarEntry(S_symbol(""), aexe->ty, aexe->size, x->u.var.scope,
            x->u.var.index, x->u.var.haskey, x->u.var.isStdout,
            x->u.var.isBuffer);
        newx->u.var.source = x->u.var.source;
        newx->u.var.index = x->u.var.index;
      }
    } else {
      /* construct a pseudo x and pass it into transInsert2C */
      if (isESL()) {
        newx = E_StreamEntry(S_symbol(""), aexe->ty, aexe->size);
      } else
        newx = E_VarEntry(S_symbol(""), aexe->ty, aexe->size);

    }
#if 0
    /* check if x is indeed pointing to somewhere
     * if not, assign kind to something that won't affect anything
     * if yes, kind will tell whether the table is local, imdb, or bdb
     */
    //kind = (x)? x->u.var.scope:(tabscope_t)100;
#endif

    if (isESL() && target->u.table.name != S_Symbol("stdout") && mode
        == USE_STDOUT)
      mode = USE_DIRECT;
    rc = transInsert2C(venv, target, target_name, target_handle, newx, mode,
        buf, isESL() ? queryName : "", sql->in_func);
    //			 aexe->ty, buf, mode, kind, x->u.var.dbtype);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
          "insert2C");
      goto exit;
    }

    bool oneTuple = isESL() && sql->in_func != 1;

    /*
     if (srcs.size() == 0)  // if no src, this is a values() query
     oneTuple = false;
     else {
     // set outTuple=false if the source is a relation
     E_enventry src_env = (E_enventry)0;   // source env
     src_env = (E_enventry)S_look(venv, S_Symbol(srcs.front()));
     if (src_env && src_env->kind != E_streamEntry)
     oneTuple = false;
     }
     */
    if (oneTuple) {
      // ESL query: insert only one tuple
      if (srcs.size() <= 1) {
        char str_vars[100];
        str_vars[0] = '\0';

        sprintf(str_vars, "\nint case0_visited = 0;"
          "\nint freed_gbstatus = 0;"
          "\nint input_count = 0;"
          "\nint output_count = 0;");

        exe = expTy_Seq(exe, str_vars);
        exe = expTy_Seq(exe, "\nwhile (rc==0 && !freed_gbstatus) {");

        /* read from source relation */
        exe = expTy_Seq(exe, aexe);

        exe = expTy_Seq(exe, "\nif (rc ==0 && slide_out == 1) {"
          "\n_adl_cursqlcode = 0; /* SUCCESS */ "
          "\n/* INSERT STARTS */");

        if (isESL())
          exe = expTy_Seq(exe, "\noutput_count++;");

        /* store new tuple into temporary relation */
        exe = expTy_Seq(exe, buf);

        // TODO(nlaptev): Verify that by removing else slide_out = 1
        // we are not breaking anything. The reason why the else was removed
        // is because this was one of the reasons why extra outputs were generated.
        //        exe = expTy_Seq(exe, "\n/* INSERT ENDS */"
        //          "\n} else"
        //          "\nslide_out = 1;");
        exe = expTy_Seq(exe, "\n/* INSERT ENDS */"
          "\n}");

        exe = expTy_Seq(exe, "\n} /* while (rc==0) */");
      }

      int isJoin = isQunJoin(source); //1 is JOIN, 0 is UNION, 2 is neither
      //printf("isJoin %d %s\n", isJoin, queryName);

      if (srcs.size() == 1) {
        buffer *bi = bm->lookup(srcs.front().c_str());
        buffer * bo;
        if (mode == USE_STDOUT) {
          string stdoutName = "stdout_";
          stdoutName += queryName;
          bm->create(stdoutName, SHARED);
          bo = bm->lookup(stdoutName);
          if (bo) {
            buffer* ioBuf = bm->lookup("_ioBuffer");
            ioBuf->put(ADD_STDOUT_COMMAND, bo->name, getUserName());
          }
        } else
          bo = bm->lookup(strdup(target_handle));
        if (!bi) {
          rc = ERR_BUFFER;
          EM_error(0, ERR_BUFFER, __LINE__, __FILE__,
              "input buffer not found: ", srcs.front().c_str());
          goto exit;
        } else if (!bo) {
          rc = ERR_BUFFER;
          EM_error(0, ERR_BUFFER, __LINE__, __FILE__,
              "output buffer not found: ", (mode == USE_STDOUT ? "STDOUT"
                  : target_handle));
          goto exit;
        }
        if (isJoin != 1) {
          if (cstmt != NULL && strcmp("temp", cstmt->name) != 0) {
            cstmt->addSubStmt(new nStmt(queryName, bi, bo));
            cstmt->set_out_buf(bo);
            qs->s = cstmt;
          } else {
            qs->s = new nStmt(queryName, bi, bo);
          }
        } else { //for join case create a join statement, add all substatements
          // substatments include the main join + a statement for each
          // QUN_WINDOW, becaue this statement does the updating.
          jStmt* mainStmt = new jStmt(queryName, bi, bo, true);
          //printf("creating join stmt %s\n", queryName);
          //it is know that source is of QUN_QUERY kind and the query is SEL kind
          A_list jtl = source->u.query->u.sqlopr->jtl_list;
          int length = jtl->length;
          for (int i = 0; i < length; i++) {
            A_qun q = (A_qun) getNthElementList(jtl, i);
            if (q->kind == QUN_WINDOW) {
              buffer * inBuf;
              buffer * outBuf;
              char qunStmtName[80];
              char qunStmtInBuf[80];
              char qunStmtOutBuf[80];
              sprintf(qunStmtName, "%s_%d_wm", queryName, UID(q->ppnode));
              sprintf(qunStmtInBuf, "%s", S_name(q->u.window.name));
              sprintf(qunStmtOutBuf, "%s_%d_winbuf", queryName, UID(q->ppnode));
              inBuf = bm->lookup(qunStmtInBuf);
              outBuf = bm->lookup(qunStmtOutBuf);
              if (!inBuf) {
                rc = ERR_BUFFER;
                EM_error(0, ERR_BUFFER, __LINE__, __FILE__,
                    "input buffer not found: ", qunStmtInBuf);
                goto exit;
              } else if (!outBuf) {
                rc = ERR_BUFFER;
                EM_error(0, ERR_BUFFER, __LINE__, __FILE__,
                    "output buffer not found: ", qunStmtOutBuf);
                goto exit;
              }
              mainStmt->addSubStmt(new nStmt(qunStmtName, inBuf, outBuf));
              /*determine if the join is synchronized or not */
              E_enventry x = (E_enventry) S_look(venv, q->u.window.name);
              E_enventry y = (E_enventry) S_look(venv, q->u.window.respectTo);
              if (x != y) {
                //commenting out temporarily to compile with older version
                //mainStmt->mantOp = true;
              }

              //printf("adding substmt %s\n", qunStmtName);
            }
          }
          if (cstmt != NULL && strcmp("temp", cstmt->name) != 0) {
            cstmt->addSubStmt(mainStmt);
            cstmt->set_out_buf(bo);
            qs->s = cstmt;
          } else {
            qs->s = mainStmt;
          }
        }
      } else if (srcs.size() >= 2 && isJoin == 0) { //unions
        //create continuous query
        char* biName;
        int size = srcs.size();
        buffer *bi = NULL;
        buffer *bo = NULL;
        buffer *subStmtBo = NULL;
        int isTime = 0;
        E_enventry x;
        char oldQueryName[1024];
        char tempQueryName[1024];
        utStmt* ut;
        utlStmt* utl;

        /* read from source relation */
        exe = expTy_Seq(exe, aexe);

        memset(oldQueryName, '\0', 1024);
        memset(tempQueryName, '\0', 1024);

        sprintf(oldQueryName, "%s", queryName);

        bo = bm->lookup(target_handle);
        if (!bo) {
          rc = ERR_BUFFER;
          EM_error(0, ERR_BUFFER, __LINE__, __FILE__,
              "output buffer not found: ", (mode == USE_STDOUT ? "STDOUT"
                  : target_handle));
        }

        for (int i = 0; i < size; i++) {
          getSubStmtName(oldQueryName, i, size, tempQueryName);
          subStmtBo = bm->lookup(tempQueryName);
          if (!subStmtBo) {
            rc = ERR_NTSQL_INTERNAL;
            EM_error(target->pos, rc, __LINE__, __FILE__,
                "Buffer for the subQuery must be defined");
            goto exit;
          }

          biName = strdup(srcs.operator[](i).c_str());
          if (!((x = (E_enventry) S_look(venv, S_Symbol(biName))) && x->kind
              == E_streamEntry)) {
            rc = ERR_UNDEFINED_VARIABLE;
            EM_error(target->pos, rc, __LINE__, __FILE__, biName);
            goto exit;
          }
          if (i == 0 && x->u.stream.timekey != (S_symbol) 0) {
            if (mode == USE_STDOUT) {
              string stdoutName = "stdout_";
              stdoutName += queryName;
              bm->create(stdoutName, SHARED);
              bo = bm->lookup(stdoutName);
              if (bo) {
                buffer* ioBuf = bm->lookup("_ioBuffer");
                ioBuf->put(ADD_STDOUT_COMMAND, bo->name, getUserName());
              }
            }

            ut = new utStmt(oldQueryName, bo, true);
            isTime = 1;
          } else if (i == 0 && x->u.stream.timekey == (S_symbol) 0) {
            if (mode == USE_STDOUT) {
              string stdoutName = "stdout_";
              stdoutName += queryName;
              bm->create(stdoutName, SHARED);
              bo = bm->lookup(stdoutName);
              if (bo) {
                buffer* ioBuf = bm->lookup("_ioBuffer");
                ioBuf->put(ADD_STDOUT_COMMAND, bo->name, getUserName());
              }
            }

            utl = new utlStmt(oldQueryName, bo, true);
          } else if (i != 0 && x->u.stream.timekey != (S_symbol) 0 && isTime
              == 0) {
            //incompatible type of srcs
            rc = ERR_INCOMPATIBLE_TIMEKEY;
            EM_error(target->pos, rc, __LINE__, __FILE__, biName);
            goto exit;
          } else if (i != 0 && x->u.stream.timekey == (S_symbol) 0 && isTime
              == 1) {
            //incompatible type of srcs
            rc = ERR_INCOMPATIBLE_TIMEKEY;
            EM_error(target->pos, rc, __LINE__, __FILE__, biName);
            goto exit;
          }
          bi = bm->lookup(biName);
          if (!bi) {
            rc = ERR_BUFFER;
            EM_error(0, ERR_BUFFER, __LINE__, __FILE__,
                "input buffer not found: ", biName);
          }
          if (isTime == 1) {
            ut->addSubStmt(new nStmt(tempQueryName, bi, subStmtBo));
          } else if (isTime == 0) {
            utl->addSubStmt(new nStmt(tempQueryName, bi, subStmtBo));
          }
        } // end for
        if (cstmt == NULL || strcmp("temp", cstmt->name) == 0) {
          if (isTime == 1)
            qs->s = ut;
          else if (isTime == 0)
            qs->s = utl;
          //printf("we are here instead %d\n", cstmt);
        } else {
          //addSubStmt will remove subStmts of union statement from the coll
          // stmt
          if (isTime == 1)
            cstmt->addSubStmt(ut);
          else if (isTime == 0)
            cstmt->addSubStmt(utl);
          qs->s = cstmt;
          fflush(stdout);
        }
      } //end else if(UNION)
    } // end if (): ESL query: insert only one tuple
    //else if(!isESL() || (isESL() && sql->in_func == 1))
    else
    // Atlas query: use while loop to insert all tuples
    {
      exe = expTy_Seq(exe, "\nwhile (rc==0) {");

      /* read from source relation */
      exe = expTy_Seq(exe, aexe);

      exe = expTy_Seq(exe, "\nif (rc ==0 && slide_out == 1) {"
        "\n_adl_cursqlcode = 0; /* SUCCESS */ "
        "\n/* INSERT STARTS */");

      if (isESL())
        exe = expTy_Seq(exe, "\noutput_count++;");

      /* store new tuple into temporary relation */
      exe = expTy_Seq(exe, buf);
      // TODO(nlaptev): Verify that by removing else slide_out = 1
      // we are not breaking anything. The reason why the else was removed
      // is because this was one of the reasons why extra outputs were generated.
      //        exe = expTy_Seq(exe, "\n/* INSERT ENDS */"
      //          "\n} else"
      //          "\nslide_out = 1;");
      exe = expTy_Seq(exe, "\n/* INSERT ENDS */"
        "\n}");

      exe = expTy_Seq(exe, "\n} /* while (rc==0) */");

      if (mode == USE_TEMP) {
        /* move all new tuple from temporary relation into target table */
        sprintf(buf, "\nmvTemp(%d, _rec_id, %s);", UID(target), target_handle);
        exe = expTy_Seq(exe, buf);
      } // end if
    } // end else

    sqlStatementGlobalCleanUp(sql, exe);
  }
    break;
  case A_SQL_UPDATE: {
    SMLog::SMLOG(12, "\tA_SQL_UPDATE");
    exe = expTy_Seq((T_expty) 0, "\nrc = 0;"
      "\n_adl_cursqlcode = 1 /* ASSUME GUITY */;");

    A_qun q = (A_qun) getNthElementList(s->jtl_list, 0);
    E_enventry x = (E_enventry) S_look(venv, q->u.table.name);
    if (x && x->kind == E_streamEntry) {
      EM_error(0, ERR_TO_BE_IMPLEMENTED, __LINE__, __FILE__,
          "Update on streams");
    }

    if (isAdHoc() || isESLAggr()) {
      //special initialization for adhoc update values
      char dbname[80];
      if (x && x->kind == E_varEntry && x->u.var.source) {
        sprintf(dbname, "\"./%s\"", S_name(x->u.var.source));
        tabindex_t index = (tabindex_t) 1;
        if (x->u.var.index != (A_index) 0)
          index = x->u.var.index->kind;
        rc = transTabInit2C(S_name(q->u.table.name), dbname, x->u.var.haskey,
            buf, x->u.var.scope, index, Ty_nil, x->u.var.inaggr,
            x->u.var.isBuffer, q->u.table.name);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
              "transTabInit2C");
          goto exit;
        }
        /*sql->preinit = expTy_Seq(sql->preinit, "\nif(");
         sql->preinit = expTy_Seq(sql->preinit, S_name(q->u.name));
         sql->preinit = expTy_Seq(sql->preinit, "==NULL) {");*/
        sql->preinit = expTy_Seq(sql->preinit, buf);
        //sql->preinit = expTy_Seq(sql->preinit, "\n//printf(\"got here\\n\");");
        //sql->preinit = expTy_Seq(sql->preinit, "\n}");

        /*This deletes the table before its cursors*/
        transTabClose2C(S_name(q->u.table.name), S_name(q->u.table.name), buf,
            x->u.var.scope, index);
        sql->afterpreclean = expTy_Seq(sql->afterpreclean, buf);

      } else if (x && x->kind == E_varEntry && !x->u.var.inaggr
          && x->u.var.scope == TAB_MEMORY) {
        tabindex_t index = (tabindex_t) 1;
        if (x->u.var.index != (A_index) 0)
          index = x->u.var.index->kind;

        char* name1;
        if (x->u.var.iname)
          name1 = S_name(x->u.var.iname);
        else
          name1 = S_name(q->u.table.name);

        sprintf(dbname, "\"_adl_db_%s\"", name1);

        rc = transTabInit2C(S_name(q->u.table.name), dbname, x->u.var.haskey,
            buf, x->u.var.scope, index, Ty_nil, x->u.var.inaggr,
            x->u.var.isBuffer, q->u.table.name);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
              "transTabInit2C");
          goto exit;
        }
        sql->preinit = expTy_Seq(sql->preinit, buf);
      }
    }

    char *name;

    /*

     if (findSourceInQun(q->u.name, source)) {
     sql->update_mode = USE_TEMP;
     } else {

     To Do: We need to detect whether target is used in the body of
     the UPDATE statement. Right now, assume it is not.
     }
     */
    sprintf(target_name, "top_%d", UID(s));

    rc = transSelOpr(venv, tenv, s, sql, adec, aexe, target_name, aggregates,
        srcs, cstmt);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
          "transSelOpr");
      goto exit;
    }

    if (!isESL() || isESLTest())
      addSqlSemFirstEntryDec(sql, (void*) s);

    sqlStatementGlobalInit(venv, sql, dec, exe);
    exe = expTy_Seq(exe, "\nwhile (rc==0) {");
    exe = expTy_Seq(exe, aexe);
    exe = expTy_Seq(exe, "\nif (rc ==0) {"
      "\n_adl_cursqlcode = 0; /* SUCCESS */"
      "\n}");
    exe = expTy_Seq(exe, "\n} /* while (rc==0) */");

    // add the updated tuples into the relation
    if (x && x->kind == E_varEntry && x->u.var.iname)
      name = S_name(x->u.var.iname);
    else
      name = S_name(q->u.table.name);

    if (sql->update_mode == USE_TEMP) {
      sprintf(buf, "\nmvTemp(%d, _rec_id, %s);", UID(s), name);
      exe = expTy_Seq(exe, buf);
      sql->update_mode == USE_DIRECT;
    }

    sql->update_mode = USE_DIRECT;
    sqlStatementGlobalCleanUp(sql, exe);
  }
    break;
  case A_SQL_DELETE: {
    SMLog::SMLOG(12, "\tA_SQL_DELETE");
    int dynamic = 0;
    S_symbol rawname;
    exe = expTy_Seq((T_expty) 0, "\nrc = 0;"
      "\n_adl_cursqlcode = 1 /* ASSUME GUITY */;");

    sprintf(target_name, "top_%d", UID(s));

    if (!s->prd_list || s->prd_list->length == 0) {
      /* There is no WHERE clause for the DELETE statement. The
       entire table is to be removed. */
      A_qun q = (A_qun) getNthElementList(s->jtl_list, 0);
      char dbname[80], handlename[200], *name;

      E_enventry x = (E_enventry) S_look(venv, q->u.table.name);
      if (x && x->kind == E_streamEntry) {
        EM_error(0, ERR_TO_BE_IMPLEMENTED, __LINE__, __FILE__,
            "DELETE on streams");
      }

      if (x && x->kind == E_dynamicEntry) {
        rawname = x->u.dynamic.rawname;
        dynamic = 1;
        x = (E_enventry) S_look(venv, x->u.dynamic.table);
      }

      if (!x || x->kind != E_varEntry || x->u.var.ty->kind != Ty_record) {
        rc = ERR_UNDEFINED_VARIABLE;
        EM_error(q->pos, rc, __LINE__, __FILE__, S_name(q->u.table.name));
        goto exit;
      }

      if (x->u.var.iname)
        name = S_name(x->u.var.iname);
      else
        name = S_name(q->u.table.name);
      strcpy(handlename, name);
      if (!x->u.var.iname && dynamic == 1) {
        sprintf(buf, "\nchar tabName_%d[200];"
          "\nIM_REL* tmpTab_%d;", UID(q), UID(q));
        exe = expTy_Seq(exe, buf);
        sprintf(buf, "\nsprintf(tabName_%d, \"%%s%s\", _modelId);", UID(q),
            S_name(rawname));
        exe = expTy_Seq(exe, buf);
        sprintf(handlename, "((IM_REL*)inMemTables->operator[](tabName_%d))",
            UID(q));
      }

      /*
       if(isAdHoc() || isESLAggr()) {
       //special initialization for adhoc insert into values
       if(x && x->kind == E_varEntry && x->u.var.source) {
       char dbname[80];
       sprintf(dbname, "\"./%s\"", S_name(x->u.var.source));
       rc = transTabInit2C(handlename, dbname, x->u.var.haskey,
       buf, x->u.var.scope, x->u.var.index,
       Ty_nil, x->u.var.inaggr, x->u.var.isBuffer, S_Symbol(name));
       if(rc) {
       EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
       "transSqlStatement", "transTabInit2C");
       goto exit;
       }
       //sql->preinit = expTy_Seq(sql->preinit, "\nif(");
       //sql->preinit = expTy_Seq(sql->preinit, handlename);
       //sql->preinit = expTy_Seq(sql->preinit, "==NULL) {");
       sql->preinit = expTy_Seq(sql->preinit, buf);
       //sql->preinit = expTy_Seq(sql->preinit, "\n//printf(\"got here\\n\");");
       //sql->preinit = expTy_Seq(sql->preinit, "\n}");

       transTabClose2C(handlename, handlename,
       buf, x->u.var.scope, x->u.var.index);
       sql->afterpreclean = expTy_Seq(sql->afterpreclean, buf);

       }
       else if(x && x->kind == E_varEntry && !x->u.var.inaggr
       && x->u.var.scope==TAB_MEMORY) {
       char* name1;
       if (x->u.var.iname)
       name1 = S_name(x->u.var.iname);
       else
       name1 = S_name(q->u.name);

       sprintf(dbname, "\"_adl_db_%s\"", name1);

       rc = transTabInit2C(S_name(target->u.name), dbname, x->u.var.haskey,
       buf, x->u.var.scope, x->u.var.index, Ty_nil,
       x->u.var.inaggr, x->u.var.isBuffer,
       S_Symbol(name1));
       if(rc) {
       EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
       "transSqlStatement", "transTabInit2C");
       goto exit;
       }
       sql->preinit = expTy_Seq(sql->preinit, buf);
       }

       }*/

      if (x->u.var.inaggr) {
        /* this is a local variable defined inside an aggregate routine */
        sprintf(buf, "\nsprintf(_adl_dbname, \"._%%d_%s\", status);", name);
        exe = expTy_Seq(exe, buf);
        strcpy(dbname, "_adl_dbname");
      } else if (x->u.var.source != (S_symbol) 0) {
        sprintf(dbname, "\"%s\"", S_name(x->u.var.source));
      } else {
        sprintf(dbname, "\"_adl_db_%s\"", name);
      }

      //if memory table and in ESL or dynamic table
      if ((x->u.var.scope == TAB_MEMORY && (isAdHoc())) || (!x->u.var.iname
          && dynamic == 1)) {
        if (!x->u.var.iname && dynamic == 1) {
          sprintf(buf,
              "\ntmpTab_%d = (IM_REL*)inMemTables->operator[](tabName_%d);"
                "\nIM_RELC *temp;"
                "\nint tfirst = 1;"
                "\nif((rc = tmpTab_%d->cursor(tmpTab_%d, &temp, 0)) != 0) {",
              UID(q), UID(q), UID(q), UID(q)
          );
        } else {
          sprintf(buf, "\n%s = (IM_REL*)inMemTables->operator[](\"%s\");"
            "\nIM_RELC *temp;"
            "\nint tfirst = 1;"
            "\nif((rc = %s->cursor(%s, &temp, 0)) != 0) {", name, name, name,
              name);
        }
        exe = expTy_Seq(exe, buf);

        if (isESL()) {
          sprintf(
              buf,
              "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->cursor\");"
                "\nreturn s_failure;"
                "\n}", getUserName(), getQueryName());
        } else if (isESLAggr()) {
          sprintf(
              buf,
              "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->cursor\");"
                "\nreturn;"
                "\n}", getUserName(), getAggrName());
        } else {
          sprintf(buf, "\nadlabort(rc, \"IM_REL->cursor\");"
            "\n}");
        }
        exe = expTy_Seq(exe, buf);

        sprintf(buf, "\nwhile(rc==0) {"
          "\nrc = temp->c_get(temp, &key, &data, (tfirst==1)?DB_FIRST:DB_NEXT);"
          "\ntfirst = 0;"
          "\nif(rc==0) {"
          "\n/*DELETE STARTS*/"
          "\nif ((rc = temp->c_del(temp, 0)) != 0) {");
        exe = expTy_Seq(exe, buf);
        if (isESL()) {
          sprintf(
              buf,
              "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_RELC->c_del() or DBC->c_del()\");"
                "\nreturn s_failure;"
                "\n}", getUserName(), getQueryName());
        } else if (isESLAggr()) {
          sprintf(
              buf,
              "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_RELC->c_del() or DBC->c_del()\");"
                "\nreturn;"
                "\n}", getUserName(), getAggrName());
        } else {
          sprintf(buf, "\nadlabort(rc, \"IM_RELC->c_del() or DBC->c_del()\");"
            "\n}");
        }
        exe = expTy_Seq(exe, buf);

        sprintf(buf, "\n}"
          "\n/*DELETE ENDS*/"
          "\n}"
          "\nif (temp && (rc = temp->c_close(temp)) != 0) {");
        exe = expTy_Seq(exe, buf);
        if (isESL()) {
          sprintf(
              buf,
              "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_RELC->c_close() or DBC->c_close()\");"
                "\nreturn s_failure;"
                "\n}", getUserName(), getQueryName());
        } else if (isESLAggr()) {
          sprintf(
              buf,
              "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_RELC->c_close() or DBC->c_close()\");"
                "\nreturn;"
                "\n}", getUserName(), getAggrName());
        } else {
          sprintf(buf,
              "\nadlabort(rc, \"IM_RELC->c_close() or DBC->c_close()\");"
                "\n}");
        }
        exe = expTy_Seq(exe, buf);
      } else {
        /* close db with TAB_LOCAL flag will erase the db */
        tabindex_t index = (tabindex_t) 1;
        if (x->u.var.index != (A_index) 0)
          index = x->u.var.index->kind;
        rc = transTabClose2C(handlename, dbname, buf, TAB_LOCAL, index);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
              "transTabClose2C");
          goto exit;
        }
        exe = expTy_Seq(exe, buf);
#if 0
        rc = transTabRemove2C(handlename, dbname, buf, TAB_LOCAL);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement", "transTabRemove2C");
          goto exit;
        }
        exe = expTy_Seq(exe, buf);
#endif
        /* reopen an empty table */
        rc = transTabInit2C(handlename, dbname, x->u.var.haskey, buf,
            x->u.var.scope, index, Ty_nil, x->u.var.inaggr, x->u.var.isBuffer,
            S_Symbol(name));
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
              "transTabInit2C");
          goto exit;
        }
        exe = expTy_Seq(exe, buf);
      }
    } else {
      rc = transSelOpr(venv, tenv, s, sql, adec, aexe, target_name, aggregates,
          srcs, cstmt);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
            "transSelOpr");
        goto exit;
      }

      //  	appendElementList(sql->first_entry_list, (nt_obj_t*)s);
      if (!isESL() || isESLTest())
        addSqlSemFirstEntryDec(sql, (void*) s);

      sqlStatementGlobalInit(venv, sql, dec, exe);

      exe = expTy_Seq(exe, "\nwhile (rc==0) {");
      exe = expTy_Seq(exe, aexe);
      exe = expTy_Seq(exe, "\nif (rc ==0) {"
        "\n_adl_cursqlcode = 0; /* SUCCESS */"
        "\n}");
      exe = expTy_Seq(exe, "\n} /* while (rc==0) */");

      sqlStatementGlobalCleanUp(sql, exe);
    }
  }
    break;
  case A_SQL_SEL:
  case A_SQL_GB:
  case A_SQL_UNION:
  case A_SQL_EXCEPT:
  case A_SQL_INTERSECT:
  case A_SQL_ORDER: {
    SMLog::SMLOG(12, "\t kind (%i) other A_SQL ", s->kind);
    exe = expTy_Seq((T_expty) 0, "\nrc = 0;"
      "\n_adl_cursqlcode = 1 /* ASSUME GUITY */;");

    A_qun q = (A_qun) getNthElementList(s->jtl_list, 0);
    E_enventry x = (E_enventry) S_look(venv, q->u.table.name);
    sprintf(target_name, "top_%d", UID(s));

    rc = transSqlQuery(venv, tenv, s, sql, adec, aexe, target_name, aggregates,
        srcs, NULL, cstmt);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
          "transSelOpr");
      goto exit;
    }
    /*      rc = declareQun2C(target_name, aexe->ty, buf);
     if (rc) {
     EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement", "declareQun2C");
     goto exit;
     }
     dec = expTy_Seq(dec, buf);*/

    //        appendElementList(sql->first_entry_list, (nt_obj_t*)s);
    if (!isESL() || isESLTest())
      addSqlSemFirstEntryDec(sql, (void*) s);

    sqlStatementGlobalInit(venv, sql, dec, exe);
    dec = expTy_Seq(dec, adec);
    if (isESL()) {
      exe = expTy_Seq(exe, "");
      exe = expTy_Seq(exe, aexe);
      exe = expTy_Seq(exe, "\nif (rc ==0) {"
        "\n_adl_cursqlcode = 0; /* SUCCESS */"
        "\n}");
      exe = expTy_Seq(exe, "");
    } else {
      exe = expTy_Seq(exe, "\nwhile (rc==0) {");
      exe = expTy_Seq(exe, aexe);
      exe = expTy_Seq(exe, "\nif (rc ==0) {"
        "\n_adl_cursqlcode = 0; /* SUCCESS */"
        "\n}");
      exe = expTy_Seq(exe, "\n} /* while (rc==0) */");
    }
    sqlStatementGlobalCleanUp(sql, exe);
  }
    break;
  }

  if (!isESL() || (isESL() && srcs.size() <= 1))
    exe = expTy_Seq(exe, "\n_adl_sqlcode = _adl_cursqlcode;");

  S_endScope(tenv);
  S_endScope(venv);

  sql->top_sqlopr = (A_sqlopr) 0;

  exit: DBUG_RETURN(rc);
}

/*
 * transOrderOpr() generate C++ code for: ORDER Opr
 */
err_t transOrderOpr(S_table venv, S_table tenv, A_sqlopr a, Sql_sem sql,
    T_expty &dec, T_expty &exe, char *name, vector<void*> aggregates, vector<
        string> &srcs) {
  SMLog::SMLOG(10, "Entering transOrderOpr");
  err_t rc = ERR_NONE;
  int i, j;
  char buf[MAX_STR_LEN], cmpbuf[MAX_STR_LEN], qunname[80];
  A_qun qun;
  T_expty subdec, subexe;
  A_list v_fields, t_fields;
  cStmt* cstmt = new cStmt("temp");

  /* A tuple get from the underlying node is put into a "sort buffer",
   such that the first n bytes are from the order-by columns */

  int offset = 0; /* the current offset into the sort buffer */
  int ordersize; /* number of bytes from the order-by columns */
  int hxoffset[32]; /* hxoffset[i] is the offset of the i-th hxp in the
   sort buffer;*/
  int gboffset[32]; /* gboffset[i] is the offset of the i-th
   sort-by column in the sort buffer */

  DBUG_ENTER("transOrderOpr");

  /* pseudo code:

   if (index == 0) {
   [initialize buffer used for sorting];
   do {
   rc = getTuple();
   if (rc ==0 )
   [store the tuple away];
   }  while (rc==0);
   [external sort];
   index++;
   }
   rc = [get sorted tuples one by one];
   if (rc==DB_NOTFOUND) {
   [free the buffer used for sorting];
   index = 0;
   }

   */

  dec = exe = (T_expty) 0;

  /* there's only one QUN under SORT node */
  qun = (A_qun) getNthElementList(a->jtl_list, 0);
  qun->ppnode = a;

  getQunName(qun, qunname);
  rc = transQun(venv, tenv, qun, sql, subdec, subexe, qunname, (Tr_exp*) 0,
      (Tr_exp*) 0, (Tr_exp*) 0, BOUND_NONE, aggregates, srcs, NULL, cstmt);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transOrderOpr", "transQun");
    goto exit;
  }

  /* declare qun under the current node */
  declareQun2C(qunname, subexe->ty, buf);
  exe = expTy_Seq(exe, buf);

  /* declare the index variable */
  addSqlSemIndexDec(sql, (void*) a);

  sprintf(buf, "\nchar _adl_order_buf_[MAX_STR_LEN];"
    "\nif (index_%d ==0) {"
    "\n/* init sort buffer */"
    "\nadl_sort_init();"
    "\ndo {"
    "\n/* get source tuple from qun */", UID(a));

  exe = expTy_Seq(exe, buf);

  /* get source tuple from qun */
  exe = expTy_Seq(exe, subexe);

  sprintf(buf, "\nif (rc==0) {"
    "\n/* make assignments */");
  exe = expTy_Seq(exe, buf);

  /* we also need to generate a cmp function */
  sprintf(cmpbuf, "\nint ordercmp_%d(const void *s1, const void *s2)"
    "\n{"
    "\nint ia, ib;"
    "\ndouble ra, rb;"
    "\nstruct timeval *ta, *tb;"
    "\nint rc;", UID(a));
  sql->global = expTy_Seq(sql->global, cmpbuf);

  /* for ORDER node, order by columns are in the predicate list */
  for (i = 0; i < a->prd_list->length; i++) {
    A_orderitem oi = (A_orderitem) getNthElementList(a->prd_list, i);

    T_expty d, e;
    rc = transExp(venv, tenv, oi->exp, sql, d, e, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transOrderOpr", "transExp");
      goto exit;
    }
    gboffset[i] = offset;

    switch (e->ty->kind) {
    case Ty_string:
      sprintf(buf, "\nmemcpy(_adl_order_buf_+%d, %s, %d);", //TODO: string size
          offset, e->exp, e->size);
      sprintf(cmpbuf, "\nrc = memcmp((char*)s1+%d, (char*)s2+%d, %d);"
        "\nif (rc !=0 ) return %crc;", offset, offset, e->size, (oi->dir) ? '-'
          : ' ');
      offset += e->size;
      if (offset % 4 != 0)
        offset += (4 - offset % 4); // alignment
      break;
    case Ty_int:
      sprintf(buf, "\nmemcpy(_adl_order_buf_+%d, &%s, sizeof(int));", offset,
          e->exp, e->size);
      sprintf(cmpbuf, "\nia = *(int*)((char*)s1+%d);"
        "\nib = *(int*)((char*)s2+%d);"
        "\nif (ia != ib) return %c(ia>ib);", offset, offset, (oi->dir) ? '-'
          : ' ');
      offset += sizeof(int);
      break;
    case Ty_real:
      sprintf(buf, "\nmemcpy(_adl_order_buf_+%d, &%s, sizeof(double));",
          offset, e->exp, e->size);
      sprintf(cmpbuf, "\nia = *(double*)((char*)s1+%d);"
        "\nib = *(double*)((char*)s2+%d);"
        "\nif (ia != ib) return %c(ia>ib);", offset, offset, (oi->dir) ? '-'
          : ' ');
      offset += sizeof(double);
      break;
    case Ty_timestamp:
      sprintf(buf,
          "\nmemcpy(_adl_order_buf_+%d, &%s, sizeof(struct timeval));", offset,
          e->exp, e->size);
      sprintf(
          cmpbuf,
          "\nta = (struct timeval*)((char*)s1+%d);"
            "\ntb = (struct timeval*)((char*)s2+%d);"
            "\nif (timeval_cmp(*ta, *tb) != 0) return %c(timeval_cmp(*ta,*tb) > 0);",
          offset, offset, (oi->dir) ? '-' : ' ');
      offset += sizeof(struct timeval);
      break;
    default:
      rc = ERR_INVALID_ARG;
      EM_error(0, rc, __LINE__, __FILE__);
      goto exit;
    }
    exe = expTy_Seq(exe, buf);
    sql->global = expTy_Seq(sql->global, cmpbuf);
  }
  ordersize = offset;

  /* end of cmp function */
  sql->global = expTy_Seq(sql->global, "\nreturn 0;"
    "\n}");

  /* some order-by columns are in hxp_list, some are not */
  t_fields = A_List(0, A_FIELD);
  for (j = 0; j < a->hxp_list->length; j++) {
    A_selectitem arg = (A_selectitem) getNthElementList(a->hxp_list, j);
    A_exp argexp = arg->u.s.exp;
    T_expty d, e;
    Ty_field curfield;

    rc = transExp(venv, tenv, argexp, sql, d, e, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transOrderOpr", "transExp");
      goto exit;
    }

    /* is this hxp also an order-by column? */
    for (i = 0; i < a->prd_list->length; i++) {
      A_orderitem oi = (A_orderitem) getNthElementList(a->prd_list, i);
      if (equalExp(oi->exp, argexp)) {
        break;
      }
    }
    if (i >= a->prd_list->length) {
      /* not found in the order by columns */
      hxoffset[j] = offset;
      switch (e->ty->kind) {
      case Ty_string:
        sprintf(buf,
            "\nmemcpy(_adl_order_buf_+%d, %s, %d);"//TODO: string size
              "\n_adl_order_buf_[%d+%d]=0;", offset, e->exp, e->size, offset,
            e->size);
        offset += e->size;
        if (offset % 4 != 0)
          offset += (4 - offset % 4); // alignment
        break;
      case Ty_int:
        sprintf(buf, "\nmemcpy(_adl_order_buf_+%d, &%s, sizeof(int));", offset,
            e->exp);
        offset += sizeof(int);
        break;
      case Ty_real:
        sprintf(buf, "\nmemcpy(_adl_order_buf_+%d, &%s, sizeof(double));",
            offset, e->exp);
        offset += sizeof(double);
        break;
      case Ty_timestamp:
        sprintf(buf,
            "\nmemcpy(_adl_order_buf_+%d, &(%s), sizeof(struct timeval));",
            offset, e->exp);
        offset += sizeof(struct timeval);
        break;
      }
      exe = expTy_Seq(exe, buf);
    } else {
      hxoffset[j] = gboffset[i];
    }
    if (arg->u.s.alias == (S_symbol) 0) {
      char field[20];
      sprintf(field, "field_%d", j);
      curfield = Ty_Field(new_Symbol(field), e->ty, e->size);
    } else {
      curfield = Ty_Field(arg->u.s.alias, e->ty, e->size);
    }
    appendElementList(t_fields, (nt_obj_t*) curfield);
  }

  sprintf(buf, "\n/* store tuple away */"
    "\nadl_sort_put(_adl_order_buf_, %d);"
    "\n}"
    "\n} while (rc==0);", offset);
  exe = expTy_Seq(exe, buf);

  sprintf(buf, "\n/*external sort*/"
    "\nadl_sort_sort(ordercmp_%d);"
    "\nindex_%d++;"
    "\n} /* end of index==0 */", UID(a), UID(a));
  exe = expTy_Seq(exe, buf);

  sprintf(buf, "\n/* get sorted tuple */"
    "\nrc=adl_sort_get(_adl_order_buf_);"
    "\n/* extract columns from sort buf */"
    "\nif (rc==0) {");
  exe = expTy_Seq(exe, buf);

  /* make assignments to hxp expression */
  for (j = 0; j < t_fields->length; j++) {
    Ty_field f = (Ty_field) getNthElementList(t_fields, j);

    switch (f->ty->kind) {
    case Ty_string:
      sprintf(buf,
          "\nmemcpy(%s.%s, _adl_order_buf_+%d, %d);"//TODO: string size
            "\n%s.%s[%d]=0;", name, S_name(f->name), hxoffset[j], f->size,
          name, S_name(f->name), f->size);
      break;
    case Ty_int:
      sprintf(buf, "\nmemcpy(&%s.%s, _adl_order_buf_+%d, %d);", name, S_name(
          f->name), hxoffset[j], sizeof(int));
      break;
    case Ty_real:
      sprintf(buf, "\nmemcpy(&%s.%s, _adl_order_buf_+%d, %d);", name, S_name(
          f->name), hxoffset[j], sizeof(double));
      break;
    case Ty_timestamp:
      sprintf(buf, "\nmemcpy(&(%s.%s), _adl_order_buf_+%d, %d);", name, S_name(
          f->name), hxoffset[j], sizeof(struct timeval));
      break;
    }
    exe = expTy_Seq(exe, buf);
  }

  sprintf(buf, "\n} else /*rc == DB_NOTFOUND*/ {"
    "\n/* free the buffer used for sorting */"
    "\nadl_sort_cleanup();"
    "\nindex_%d = 0;"
    "\n}", UID(a));
  exe = expTy_Seq(exe, buf);

  /* set the return type of GB opr */
  exe->ty = Ty_Record(t_fields);

  exit: DBUG_RETURN(rc);
}

/*******************************************************************************/
/*                                                                             */
/*              INITIALIZE .... INITIALIZE                                     */
/*                                                                             */
/*******************************************************************************/
err_t transInitExp(A_exp a, T_expty &exe);
err_t transInitSqlQuery(A_sqlopr a, T_expty &exe);
err_t transInitQun(A_qun q, T_expty &exe);
err_t transInitSelOpr(A_sqlopr a, T_expty &exe);

err_t transInitExp(A_exp a, T_expty &exe) {
  SMLog::SMLOG(10, "Entering transInitExp");
  err_t rc = ERR_NONE;

  exe = (T_expty) 0;

  switch (a->kind) {
  case A_refExp:
  case A_varExp:
  case A_callExp:
  case A_recordExp:
  case A_assignExp:
  case A_ifExp:
  case A_whileExp:
  case A_forExp:
  case A_breakExp:
  case A_arrayExp:
  case A_nilExp:
  case A_intExp:
  case A_realExp:
  case A_stringExp:
  case A_timestampExp:
    /* nothing happens */
    break;
  case A_sqloprExp: {
    char buf[MAX_STR_LEN];
    T_expty lexe;

    sprintf(buf, "\nfirst_entry_%d = 1;", UID(a->u.sqlopr));
    exe = expTy_Seq(exe, buf);
    rc = transInitSelOpr(a->u.sqlopr, lexe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitExp",
          "transInitSelOpr");
      goto exit;
    }
    if (lexe)
      exe = expTy_Seq(exe, lexe);
  }
    break;
  case A_seqExp: {
    T_expty lexe;

    if (a->u.seq) {
      for (int i = 0; i < a->u.seq->length; i++) {
        A_exp member = (A_exp) getNthElementList(a->u.seq, i);
        rc = transInitExp(member, lexe);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitExp",
              "transInitExp");
          goto exit;
        }
        if (lexe)
          exe = expTy_Seq(exe, lexe);
      }
    }
  }
    break;
  case A_opExp: {
    T_expty lexe;

    rc = transInitExp(a->u.op.left, lexe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitExp",
          "transInitExp");
      goto exit;
    }
    if (lexe)
      exe = expTy_Seq(exe, lexe);

    if (a->u.op.right) {
      rc = transInitExp(a->u.op.right, lexe);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitExp",
            "transInitExp");
        goto exit;
      }
      if (lexe)
        exe = expTy_Seq(exe, lexe);
    }
  }
    break;
  case A_letExp:
  case A_selectExp:
    /* should not happen here, all SELECT has been re-written  */
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "transInitExp");
    goto exit;
  }
  exit: return rc;
}

err_t transInitSqlQuery(A_sqlopr a, T_expty &exe) {
  SMLog::SMLOG(10, "Entering transInitSqlQuery");
  err_t rc = ERR_NONE;
  T_expty lexe;

  exe = (T_expty) 0;

  switch (a->kind) {
  case A_SQL_SEL:
    rc = transInitSelOpr(a, exe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitSqlQuery",
          "transInitSelOpr");
      goto exit;
    }
    break;
  case A_SQL_GB:
    /*
     TO DO: for group-by node, we need to clean up its hash table for group-by columns.
     Right now, we only clean up the gb node that is directly beneath
     the EXISTS OP, this is not enough

     rc = transInitGBOpr(a, exe);
     if (rc) {
     EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitSqlQuery", "transInitGBOpr");
     goto exit;
     }
     */
    break;
  case A_SQL_UNION: {
    int i;
    for (i = 0; i < a->jtl_list->length; i++) {
      A_qun qun = (A_qun) getNthElementList(a->jtl_list, i);
      rc = transInitQun(qun, lexe);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitSqlQuery",
            "transInitQun");
        goto exit;
      }
      if (lexe)
        exe = expTy_Seq(exe, lexe);
    }
  }
    break;
  case A_SQL_EXCEPT:
  case A_SQL_INTERSECT:
  case A_SQL_ORDER:
    break;
  }
  exit: return rc;
}

err_t transInitQun(A_qun q, T_expty &exe) {
  SMLog::SMLOG(10, "Entering transInitQun");
  err_t rc = ERR_NONE;

  exe = (T_expty) 0;

  switch (q->kind) {
  case QUN_NAME: {
    char buf[MAX_STR_LEN];

    if (!isESL())
      sprintf(buf, "\nfirst_entry_%d = 1;", UID(q));
    else
      sprintf(buf, "\n");
    exe = expTy_Seq(exe, buf);
  }
    break;
  case QUN_WINDOW: {
    char buf[MAX_STR_LEN];

    if (!isESL())
      sprintf(buf, "\nfirst_entry_%d = 1;", UID(q));
    else
      sprintf(buf, "\n");
    exe = expTy_Seq(exe, buf);
  }
    break;
  case QUN_FUNCTION:
    /* nothing happens */
    break;
  case QUN_QUERY:
    rc = transInitSqlQuery(q->u.query->u.sqlopr, exe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitQun",
          "transInitSqlQuery");
      goto exit;
    }
    break;
  }
  exit: return rc;
}

err_t transInitSelOpr(A_sqlopr a, T_expty &exe) {
  err_t rc = ERR_NONE;
  int i;
  T_expty lexe;

  exe = (T_expty) 0;

  if (a->jtl_list) {
    for (i = 0; i < a->jtl_list->length; i++) {
      A_qun q = (A_qun) getNthElementList(a->jtl_list, i);
      rc = transInitQun(q, lexe);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitSelOpr",
            "transInitQun");
        goto exit;
      }
      if (lexe)
        exe = expTy_Seq(exe, lexe);
    }
  }

  if (a->prd_list) {
    for (i = 0; i < a->prd_list->length; i++) {
      A_exp prd = (A_exp) getNthElementList(a->prd_list, i);
      rc = transInitExp(prd, lexe);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInitSelOpr",
            "transInitQun");
        goto exit;
      }
      if (lexe)
        exe = expTy_Seq(exe, lexe);
    }
  }

  exit: return rc;
}
/*************************************************************************************/
/***********END OF INITIALIZE*********************************************************/
/*************************************************************************************/
/*
 * If a var is local, it means that it can not be computed
 * independently without the local context.
 **/
err_t checkLocalVar(S_table venv, A_var v, A_sqlopr a, int *result) {
  SMLog::SMLOG(10, "Entering checkLocalVar");
  err_t rc = ERR_NONE;
  int i;
  A_qun qun;

  *result = 0;

  switch (v->kind) {
  case A_fieldVar: {
    S_symbol sym = v->u.field.var->u.simple;
    for (i = 0; a->jtl_list && i < a->jtl_list->length; i++) {
      qun = (A_qun) getNthElementList(a->jtl_list, i);
      if (sym == qun->alias) {
        *result = 1;
        break;
      }
    }
  }
  case A_simpleVar: {
    /* For QUN_FUNCTION or QUN_QUERY, aliases are mandatory. So we
     handle QUN_NAME only. */
    for (i = 0; a->jtl_list && i < a->jtl_list->length; i++) {
      qun = (A_qun) getNthElementList(a->jtl_list, i);
      if (qun->kind == QUN_NAME) {
        E_enventry x = (E_enventry) S_look(venv, qun->u.table.name);
        Ty_field field;

        if (x && x->kind == E_dynamicEntry) {
          x = (E_enventry) S_look(venv, x->u.dynamic.table);
        }

        if (!x || (x->kind != E_varEntry && x->kind != E_streamEntry)) {
          rc = ERR_UNDEFINED_VARIABLE;
          EM_error(qun->pos, rc, __LINE__, __FILE__, S_name(qun->u.table.name));
          goto exit;
        }
        Ty_ty ty = x->kind == E_streamEntry ? x->u.stream.ty : x->u.var.ty;
        rc = searchRecordField(ty->u.record, v->u.simple, (int*) 0, (int*) 0,
            field);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkLocalVar",
              "searchRecordField");
          goto exit;
        }
        if (field) {
          *result = 1;
        }
      }
    }
  }
    break;
  case A_subscriptVar:
    /* not used */
    break;
  }
  exit: return rc;
}

/*
 * A *local* expression is an expression that can not be computed
 * outside its local context. If an expression is not local, it can be
 * computed a priori.
 **/
err_t checkLocalExp(S_table venv, A_exp exp, A_sqlopr a, int *result) {
  SMLog::SMLOG(10, "Entering checkLocalExp");
  err_t rc = ERR_NONE;
  *result = 0;

  switch (exp->kind) {
  case A_refExp: {
    /* get the first var */
    A_ref ref = exp->u.ref;
    while (ref->kind == A_refRef) {
      ref = ref->u.ref;
    }
    rc = checkLocalVar(venv, ref->u.var, a, result);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkLocalExp",
          "checkLocalExp");
      goto exit;
    }
  }
    break;
  case A_callExp: {
    int i;

    if (exp->u.call.args) {
      for (i = 0; *result == 0 && i < exp->u.call.args->length; i++) {
        A_exp arg_exp = (A_exp) getNthElementList(exp->u.call.args, i);
        rc = checkLocalExp(venv, arg_exp, a, result);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkLocalExp",
              "checkLocalExp");
          goto exit;
        }
      }
    }
  }
    break;
  case A_varExp:
    rc = checkLocalVar(venv, exp->u.var, a, result);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkLocalExp",
          "checkLocalVar");
      goto exit;
    }
    break;
  case A_sqloprExp:
  case A_selectExp:
    *result = 1; // not really, but just keep it simple
    break;
  case A_opExp:
    rc = checkLocalExp(venv, exp->u.op.left, a, result);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkLocalExp",
          "checkLocalExp");
      goto exit;
    }
    if (*result == 0 && exp->u.op.right) {
      rc = checkLocalExp(venv, exp->u.op.right, a, result);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "checkLocalExp",
            "checkLocalExp");
        goto exit;
      }
    }
    break;
  case A_nilExp:
  case A_intExp:
  case A_realExp:
  case A_stringExp:
  case A_timestampExp:
    break;
  default:
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "unexpected type in checkLocalExp()");
    goto exit;
  }
  exit: return rc;
}
/* 
 ********************************************************************************
 * transSelOpr() generate C++ code for:
 *        SEL opr, UPDATE opr
 ********************************************************************************
 */
err_t transSelOpr(S_table venv, S_table tenv, A_sqlopr a, Sql_sem sql,
    T_expty &dec, T_expty &exe, char *name, vector<void*> aggregates, vector<
        string> &srcs, //source buffers' names, used in ESL
    cStmt*& cstmt) {
  SMLog::SMLOG(10, "Entering transSelOpr, name: %s", name);
  err_t rc = ERR_NONE;

  A_list v_fields, t_fields;
  T_expty subdec[16];
  T_expty subexe[16];
  Sql_sem subsql[16];
  char buf[MAX_STR_LEN];
  char qunname[80];
  int i, j;
  int cur_sqlopr_flag;
  int precomputed[80], prep = 0;
  char prebuf[MAX_STR_LEN];

  DBUG_ENTER("transSelOpr");

  dec = exe = (T_expty) 0;

  S_beginScope(venv);
  S_beginScope(tenv);

  sql->cur_sqlopr = a;

  cur_sqlopr_flag = sql->cur_sqlopr_flag;
  sql->cur_sqlopr_flag = 0;

  if (cur_sqlopr_flag == 2 /* EXISTS */|| cur_sqlopr_flag == 3 /* NON_EXISTS */) {
    // exists or nonexists
    // check if this is really a GB (SEL-GB-SEL)
    // operator. We set the GB_EXISTS_FLAG for the GB operator, so that
    // it will clean up the hash table as soon as one tuple is
    // returned.
    if (a->jtl_list && a->jtl_list->length == 1) {
      A_qun q = (A_qun) getNthElementList(a->jtl_list, 0);
      if (q->kind == QUN_QUERY) {
        A_sqlopr gb = q->u.query->u.sqlopr;
        if (gb->kind == A_SQL_GB) {
          gb->distinct |= GB_EXISTS_FLAG; // overloading
        }
      }
    }
  }

  //printf("in selopr\n");

  /* compute predicates (those that can be pre-computed*/
  if (a->prd_list) {

    memset(precomputed, 0, sizeof(int) * 80);
    *prebuf = '\0';

    for (i = 0; i < a->prd_list->length; i++) {
      A_exp prd = (A_exp) getNthElementList(a->prd_list, i);
      int localp;

      rc = checkLocalExp(venv, prd, a, &localp);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
            "checkLocalExp");
        goto exit;
      }

      if (!localp) {
        T_expty d, e;
        rc = transExp(venv, tenv, prd, sql, d, e, aggregates);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
              "transExp");
          goto exit;
        }

        if (prep != 0)
          strcat(prebuf, " && ");

        strcat(prebuf, e->exp);

        precomputed[i] = 1;
        prep = 1;
      }
    }
    if (prep) {
      sprintf(buf, "\n/* precomputed predicates */"
        "\nif (! (%s) ) {"
        "\nrc = DB_NOTFOUND;"
        "\n} else {", prebuf);
      exe = expTy_Seq(exe, buf);
    }
  }

  /* subnodes */
  if (a->jtl_list && a->jtl_list->length > 0) {

    /*
     * STEP 1. Compile all the QUNS under the current node assumming
     * all of them are unbound.  This step is necessary because we
     * need the type information of each QUN to decide whether a
     * certain QUN is bound or not.
     */

    /* add cursor/view definition into enviroment */
    for (i = 0; i < a->jtl_list->length; i++) {
      A_qun q = (A_qun) getNthElementList(a->jtl_list, i);

      if (isESL() && i != 0) {
        if (q->kind == QUN_NAME) {
          //S_dump(venv, show);
          E_enventry te = (E_enventry) S_look(venv, q->u.table.name);
          if (!te) {
            char uName[256];
            char nName[256];
            getUserNameie(uName);
            sprintf(nName, "%s%s", uName, S_name(q->u.table.name));
            q->u.table.name = S_Symbol(nName);
            te = (E_enventry) S_look(venv, q->u.table.name);
            //printf("we here looking %s %d\n", nName, te);
          }
          if (!te || te->kind == E_streamEntry) {
            rc = ERR_JOIN_STREAM_TABLE;
            EM_error(
                0,
                rc,
                __LINE__,
                __FILE__,
                "Stream can only be the first table in FROM clause, when joining with table(s).");
            goto exit;
          }
        }
      }
      if (q->kind == QUN_NAME) {
        E_enventry ee = (E_enventry) S_look(venv, q->u.table.name);
        if (ee && ee->kind == E_streamEntry && ee->u.stream.target != NULL) {
          rc = ERR_TARGET_STREAM_USE;
          EM_error(q->pos, rc, __LINE__, __FILE__, S_name(q->u.table.name));
          goto exit;
        }
      }

      /*if (isESL() && i >=1){
       rc = ERR_TO_BE_IMPLEMENTED;
       EM_error(0, ERR_TO_BE_IMPLEMENTED, __LINE__, __FILE__, "join on streams/tables");
       goto exit;
       }*/

      // set parent
      q->ppnode = a;

      // detect duplicates
      for (j = 0; j < i; j++) {
        A_qun p = (A_qun) getNthElementList(a->jtl_list, j);
        if (q->alias == p->alias) {
          rc = ERR_DUPLICATE_JOIN_NAME;
          EM_error(p->pos, rc, __LINE__, __FILE__, S_name(p->alias));
          goto exit;
        }
      }

      getQunName(q, qunname);

      // compile the i-th QUN, assuming it is not bound
      subsql[i] = SqlSem();
      subsql[i]->cur_sqlopr = sql->cur_sqlopr;
      subsql[i]->top_sqlopr = sql->top_sqlopr;

      /*printf("A---- calling transQun\n");
       displayQun(q);
       printf("\nA---- done\n");*/
      rc = transQun(venv, tenv, q, subsql[i], subdec[i], subexe[i], qunname,
          (Tr_exp*) 0, (Tr_exp*) 0, (Tr_exp*) 0, BOUND_NONE, aggregates, srcs,
          NULL, cstmt);
      //printf("A---- backfrom transQun\n");

      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr", "transQun");
        goto exit;
      }
      if (isESL() && q->kind == QUN_QUERY && subdec[i]) {
        dec = expTy_Seq(dec, subdec[i]->exp);
      }
      if (sql->in_func == 0 && isESL()) {
        //TODO:here we just over-wrote preclean, this is ok as long as we
        // don't put anything else in preclean, which is the case.
        sql->preclean = expTy_Seq(NULL, "\ndoClose:");
      }
      if (isESL() && subsql[i]->afterpreclean) {
        sql->preclean = expTy_Seq(sql->preclean, subsql[i]->afterpreclean->exp);
      }
      if (isESL() && (q->kind == QUN_NAME || q->kind == QUN_WINDOW)
          && subsql[i] && subsql[i]->predec && subsql[i]->preinit) {
        dec = expTy_Seq(dec, subsql[i]->predec->exp);
        dec = expTy_Seq(dec, subsql[i]->preinit->exp);
      }
      if (isESL() && q->kind == QUN_WINDOW) {
        sql->global = expTy_Seq(sql->global, subsql[i]->global);
      }
    }
    if (!isESL()) {
      /*
       * STEP 2. Detect if any QUN is bound. If a QUN is bound, we need
       * to recompile the QUN.
       */

      for (i = 0; i < a->jtl_list->length; i++) {
        A_qun q = (A_qun) getNthElementList(a->jtl_list, i);

        Tr_exp *exep;
        Tr_exp *exepUpper;
        Tr_exp *exepLower;

        // are all/partial keys in the current QUN bound?
        int flag;
        rc
            = isQunBound(venv, tenv, a, i, sql, exep, exepUpper, exepLower,
                flag);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
              "transQun");
          goto exit;
        }

        // if all/partial keys are bound, we need to re-compile the QUN
        if (exep != (Tr_exp*) 0 || exepUpper != (Tr_exp*) 0 || exepLower
            != (Tr_exp*) 0) {
          if (subdec[i])
            expTy_Delete(subdec[i]);
          if (subexe[i])
            expTy_Delete(subexe[i]);

          getQunName(q, qunname);

          SqlSem_Delete(subsql[i]);
          subsql[i] = SqlSem();
          subsql[i]->cur_sqlopr = sql->cur_sqlopr;
          subsql[i]->top_sqlopr = sql->top_sqlopr;

          rc = transQun(venv, tenv, q, subsql[i], subdec[i], subexe[i],
              qunname, exep, exepUpper, exepLower, flag, aggregates, srcs,
              NULL, cstmt);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
                "transQun");
            goto exit;
          }
        }

        mergeSqlSem(sql, subsql[i]);
      } // end for, step 2
    } // end if not isESL()

    /* declare all qun under the current node */
    for (i = 0; i < a->jtl_list->length; i++) {
      A_qun q = (A_qun) getNthElementList(a->jtl_list, i);

      getQunName(q, qunname);
      declareQun2C(qunname, subexe[i]->ty, buf);
      exe = expTy_Seq(exe, buf);
    }

    /* compute Sub Queries */
    sprintf(buf, "\nnext_%d:", UID(a));
    exe = expTy_Seq(exe, buf);

    if (a->jtl_list->length == 1) {
      exe = expTy_Seq(exe, subexe[0]);

      /*
       * for scalar query we shall succeed once and only once
       */
      if (cur_sqlopr_flag == 1 /* SCALAR */) {
        if (!isESL())
          sprintf(buf, "\nfirst_entry_%d = (rc)? 1:0;", UID(a));
        else
          sprintf(buf, "\n");
        exe = expTy_Seq(exe, buf);
      }

      exe = expTy_Seq(exe, "\nif (rc==0) {");
      //&& slide_out == 1) { );
      //slide should still work, this caused a bug, so commented

      /*
       * For SQL UPDATE, if there is subquery in WHERE clause, we need
       * to keep a copy of the key/data pair of the target table
       * because variables "key" and "data" might be used in the
       * subquery.

       * To Do: test if there's subquery in WHERE clause.
       * Right now, we keep a copy as long as there's a WHERE clause
       */
      if (a->kind == A_SQL_UPDATE && a->prd_list && a->prd_list->length > 0) {
        exe = expTy_Seq(exe, "\ntmpStore(&key, &data);");
      }
    } else {

      addSqlSemIndexDec(sql, (void*) a);

      sprintf(buf, "\nwhile (index_%d>=0 && index_%d < %d) { ", UID(a), UID(a),
          a->jtl_list->length);
      exe = expTy_Seq(exe, buf);

      sprintf(buf, "\nswitch(index_%d) {", UID(a));
      exe = expTy_Seq(exe, buf);

      for (i = 0; i < a->jtl_list->length; i++) {
        A_qun qun = (A_qun) getNthElementList(a->jtl_list, i);

        sprintf(buf, "\ncase %d:\n{", i);
        exe = expTy_Seq(exe, buf);
        //sprintf(buf, "\nprintf(\"case1 %d\\n\");fflush(stdout);", i);
        //exe = expTy_Seq(exe, buf);

        exe = expTy_Seq(exe, subexe[i]);
        exe = expTy_Seq(exe, "\n}\nbreak;");
      }
      sprintf(buf, "\n} /*switch */"
        "\nif (rc==0) {"
        "\nindex_%d++;"
        "\n} else if (rc==DB_NOTFOUND) {"
        "\nindex_%d--;"
        "\n}"
        "\n} /* while */", UID(a), UID(a));
      exe = expTy_Seq(exe, buf);

      /* for scalar query we shall succeed once and only once */
      if (cur_sqlopr_flag == 1 /* SCALAR */) {

        //sprintf(buf,
        //"\nif (first_entry_%d && rc !=0 ||"
        //"\n    !first_entry_%d && rc != DB_NOTFOUND) {"
        //"\nfprintf(stderr, \"ERR: scalar result required in sub query at line %d\\n\");"
        //"\nexit(1);"
        //"\n}"
        //"\nfirst_entry_%d = (rc)? 1:0;"
        //	, UID(a), UID(a), a->pos, UID(a));
        sprintf(buf, "\nif (first_entry_%d && rc !=0 ||"
          "\n    !first_entry_%d && rc != DB_NOTFOUND) {", UID(a), UID(a));
        exe = expTy_Seq(exe, buf);

        if (isESL()) {
          sprintf(
              buf,
              "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"Error in query %s: scalar result required in sub query at line %d\");"
                "\nreturn s_failure;"
                "\n}"
                "\nfirst_entry_%d = (rc)? 1:0;", getUserName(), getQueryName(),
              a->pos, UID(a));
        } else if (isESLAggr()) {
          sprintf(
              buf,
              "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"Error in Aggregate %s: scalar result required in sub query at line %d\");"
                "\nreturn;"
                "\n}"
                "\nfirst_entry_%d = (rc)? 1:0;", getUserName(), getAggrName(),
              a->pos, UID(a));
        } else {
          sprintf(
              buf,
              "\nfprintf(stderr, \"ERR: scalar result required in sub query at line %d\\n\");"
                "\nexit(1);"
                "\n}"
                "\nfirst_entry_%d = (rc)? 1:0;", a->pos, UID(a));
        }
        exe = expTy_Seq(exe, buf);
      }

      /* Now we are out of the while roop.  If we were successful,
       i.e., we reached the last subgoal and the index was
       incremented, we need to decrease the index, so the next
       getTuple will churn the last subgoal again. If we were not
       successful, then we have backtracked beyond the first
       subgoal, so we need to reset index to 0. */

      sprintf(buf, "\nif (rc!=0) {"
        "\nindex_%d++;    /* set index to the first subgoal */"
        "\n} else {"
        "\nindex_%d--;    /* set index to the last subgoal */", UID(a), UID(a));

      exe = expTy_Seq(exe, buf);
    }

  } else {
    /* jtl_list->length == 0 */

    /* this is a VALUES() construct */
    sprintf(buf, "\nnext_%d:"
      "\nrc = (first_entry_%d)? 0:DB_NOTFOUND;"
      "\nif (rc == DB_NOTFOUND) first_entry_%d=1;"
      "\nelse {"
      "\nfirst_entry_%d=0;", UID(a), UID(a), UID(a), UID(a));
    exe = expTy_Seq(exe, buf);
  }

  /* compute predicates */
  if (a->prd_list) {
    for (i = 0; i < a->prd_list->length; i++) {
      T_expty d, e;
      A_exp prd;

      if (precomputed[i])
        continue;

      prd = (A_exp) getNthElementList(a->prd_list, i);
      rc = transExp(venv, tenv, prd, sql, d, e, aggregates);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr", "transExp");
        goto exit;
      }

      if (d) {
        /* subquery in predicate */
        exe = expTy_Seq(exe, d);
      }
      if (isESL() && a->jtl_list->length == 1) {
        sprintf(buf,
            "\nrc = 0;          /* subquery could've overwritten rc */"
              "\nif (!(%s)) {"
              "\nrc = s_no_output;"
              "\ngoto doClose;"
              "\n}", e->exp, UID(a));
      } else {
        sprintf(buf,
            "\nrc = 0;          /* subquery could've overwritten rc */"
              "\nif (!(%s)) {"
              "\ngoto next_%d;"
              "\n}", e->exp, UID(a));
      }
      exe = expTy_Seq(exe, buf);
    }
  }

  if (a->kind == A_SQL_UPDATE) {
    A_qun qun = (A_qun) getNthElementList(a->jtl_list, 0);
    A_var target;
    A_exp source;
    T_expty s_dec, s_exp;
    Ty_field f;
    E_enventry te;
    int offset;
    char cursor_name[128];
    int key_updated = 0; // whether key is updated.
    char rtree_update_buf[MAX_STR_LEN]; // POINT<->RECTANGLE in rtree
    rtree_update_buf[0] = '\0';
    char argbuf[1024];
    char temp[80];
    argbuf[0] = '\0';
    char fName[80];

    sprintf(cursor_name, "%s_%d", S_name(qun->alias), UID(qun->ppnode));
    exe = expTy_Seq(exe, "\n/*UPDATE STARTS*/");
    exe = expTy_Seq(exe, "\nif (key.data == (char*)0) {"
      "\n/* key may not be initialized if r_key is in use */"
      "\nkey.data = keydata;"
      "\n}");

    /* recover the stored key/data pair of the target table */
    if (a->prd_list && a->prd_list->length > 0) {
      exe = expTy_Seq(exe, "\ntmpRecover(&key, &data);");
    }

    if (!((te = (E_enventry) S_look(venv, qun->alias)) && te->kind
        == E_varEntry && te->u.var.ty->kind == Ty_record)) {
      rc = ERR_UNDEFINED_VARIABLE;
      EM_error(target->pos, rc, __LINE__, __FILE__, S_name(target->u.simple));
      goto exit;
    }

    for (i = 0; i < a->hxp_list->length; i++) {
      A_exp setitem = (A_exp) getNthElementList(a->hxp_list, i);
      if (setitem->kind != A_assignExp) {
        rc = ERR_NTSQL_INTERNAL;
        EM_error(0, rc, __LINE__, __FILE__, "SQL UPDATE");
        goto exit;
      }
      target = setitem->u.assign.var;
      source = setitem->u.assign.exp;

      rc = transExp(venv, tenv, source, sql, s_dec, s_exp, aggregates);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr", "transExp");
        goto exit;
      }

      if (s_dec != (T_expty) 0) {
        /* We have (scalar) sub-queries in UPDATE.  We need another
         copy of KEY/DATA pair because the current pair is used as
         lvalue in the assigment.
         */
        char buf[MAX_STR_LEN];
        sprintf(buf, "\n /* SUBQUERY IN UPDATE STARTS */"
          "\nkdPush(&key,&data);"
        // 		"\nDBT oldkey, olddata;"
        // 		"\nmemcpy(&oldkey, &key, sizeof(key));"
        // 		"\nmemcpy(&olddata, &data, sizeof(data));"
        );
        //	cWrite(out, "\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];");
        exe = expTy_Seq(exe, buf);

        exe = expTy_Seq(exe, s_dec);

        sprintf(buf, "\nkdPop(&key, &data);"
        // 		"\nmemcpy(&key, &oldkey, sizeof(key));"
            // 		"\nmemcpy(&data, &olddata, sizeof(data));"
              "\n/* SUBQUERY IN UPDATE ENDS */");
        exe = expTy_Seq(exe, buf);
      }

      switch (target->kind) {
      case A_simpleVar: {
        rc = searchRecordField(te->u.var.ty->u.record, target->u.simple,
            (int*) 0, &offset, f);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
              "searchRecordField");
          goto exit;
        }

        if (f == (Ty_field) 0) {
          rc = ERR_UNDEFINED_VARIABLE;
          EM_error(target->pos, rc, __LINE__, __FILE__,
              S_name(target->u.simple));
          goto exit;
        }

        if (s_exp->ty != f->ty && !extendType(s_exp->ty, f->ty)) {
          rc = ERR_DATATYPE;
          EM_error(target->pos, rc, __LINE__, __FILE__, "UPDATE assignment");
          goto exit;
        }

        switch (f->ty->kind) {
        case Ty_int:
          if (f->iskey) {
            key_updated++;
            if (te->u.var.index != (A_index) 0 && te->u.var.index->kind
                == INDEX_BTREE && te->u.var.index->func != (S_symbol) 0) {
              sprintf(buf, "\nint field_%d = %s;", i, s_exp->exp);
              exe = expTy_Seq(exe, buf);
              sprintf(temp, "field_%d", i);
              if (argbuf[0] != '\0') {
                strcat(argbuf, " ,");
              }
              strcat(argbuf, temp);
            } else {
              sprintf(buf, "\n*(int*)((char*)key.data+%d) = %s;", offset,
                  s_exp->exp);
              exe = expTy_Seq(exe, buf);
            }
            sprintf(buf, "\n*(int*)((char*)data.data+%d) = %s;", offset,
                s_exp->exp);
            if (te->u.var.index != (A_index) 0 && te->u.var.index->kind
                == INDEX_RTREE) {
              sprintf(rtree_update_buf,
                  "%s\n*(int*)((char*)key.data+%d) = %s;", rtree_update_buf,
                  offset + 8, s_exp->exp);
            }
          } else {
            sprintf(buf, "\n*(int*)((char*)data.data+%d) = %s;", offset,
                s_exp->exp);
          }
          break;
        case Ty_string:
          if (f->iskey) {
            key_updated++;
            if (te->u.var.index != (A_index) 0 && te->u.var.index->kind
                == INDEX_BTREE && te->u.var.index->func != (S_symbol) 0) {
              sprintf(buf, "\nchar field_%d[%d];"
                "\nmemcpy(field_%d, %s, %d);", i, f->size, i, s_exp->exp,
                  f->size);
              exe = expTy_Seq(exe, buf);
              sprintf(temp, "field_%d", i);
              if (argbuf[0] != '\0') {
                strcat(argbuf, " ,");
              }
              strcat(argbuf, temp);
            } else {
              sprintf(buf, "\nmemcpy((char*)key.data+%d, %s, %d);", offset,
                  s_exp->exp, f->size);
              exe = expTy_Seq(exe, buf);
            }
            sprintf(buf, "\nmemcpy((char*)data.data+%d, %s, %d);", offset,
                s_exp->exp, f->size);
          } else {
            sprintf(buf, "\nmemcpy((char*)data.data+%d, %s, %d);", offset,
                s_exp->exp, f->size);
          }
          break;
        case Ty_real:
          if (f->iskey) {
            key_updated++;
            if (te->u.var.index != (A_index) 0 && te->u.var.index->kind
                == INDEX_BTREE && te->u.var.index->func != (S_symbol) 0) {
              sprintf(buf, "\ndouble field_%d = %s;", i, s_exp->exp);
              exe = expTy_Seq(exe, buf);
              sprintf(temp, "field_%d", i);
              if (argbuf[0] != '\0') {
                strcat(argbuf, " ,");
              }
              strcat(argbuf, temp);
            } else {
              sprintf(buf, "\n*(double*)((char*)key.data+%d) = %s;", offset,
                  s_exp->exp);
              exe = expTy_Seq(exe, buf);
            }
            sprintf(buf, "\n*(double*)((char*)data.data+%d) = %s;", offset,
                s_exp->exp);
          } else {
            sprintf(buf, "\n*(double*)((char*)data.data+%d) = %s;", offset,
                s_exp->exp);
          }
          break;
        case Ty_timestamp:
          if (f->iskey) {
            key_updated++;
            if (s_exp->tv != NULL) {
              if (te->u.var.index != (A_index) 0 && te->u.var.index->kind
                  == INDEX_BTREE && te->u.var.index->func != (S_symbol) 0) {
                sprintf(buf, "\ntimestamp field_%d;"
                  "\nfield_%d.tv_sec = %d;"
                  "\nfield_%d.tv_usec = %d;", i, i, s_exp->tv->tv_sec, i,
                    s_exp->tv->tv_usec);
                exe = expTy_Seq(exe, buf);
                sprintf(temp, "field_%d", i);
                if (argbuf[0] != '\0') {
                  strcat(argbuf, " ,");
                }
                strcat(argbuf, temp);
              } else {
                sprintf(buf,
                    "\nmemcpy((char*)key.data+%d, &A_Timeval(%d, %d), %d);",
                    offset, s_exp->tv->tv_sec, s_exp->tv->tv_usec,
                    sizeof(struct timeval));
                exe = expTy_Seq(exe, buf);
              }
              sprintf(buf,
                  "\nmemcpy((char*)data.data+%d, &A_Timeval(%d, %d), %d);",
                  offset, s_exp->tv->tv_sec, s_exp->tv->tv_usec,
                  sizeof(struct timeval));
            } else {
              if (te->u.var.index != (A_index) 0 && te->u.var.index->kind
                  == INDEX_BTREE && te->u.var.index->func != (S_symbol) 0) {
                sprintf(buf, "\ntimestamp field_%d;"
                  "\nmemcpy(&field_%d, &%s, sizeof(timestamp));",
                //"\nfield_%d.tv_sec = %s.tv_sec;"
                    //"\nfield_%d.tv_usec = %s.tv_usec;",
                    i, i, s_exp->exp/*, i, s_exp->exp*/);
                exe = expTy_Seq(exe, buf);
                sprintf(temp, "field_%d", i);
                if (argbuf[0] != '\0') {
                  strcat(argbuf, " ,");
                }
                strcat(argbuf, temp);
              } else {
                sprintf(buf, "\nmemcpy((char*)key.data+%d, &%s, %d);", offset,
                    s_exp->exp, sizeof(struct timeval));
                exe = expTy_Seq(exe, buf);
              }
              sprintf(buf, "\nmemcpy((char*)data.data+%d, &%s, %d);", offset,
                  s_exp->exp, sizeof(struct timeval));
            }
          } else {
            if (s_exp->tv != NULL) {
              sprintf(buf,
                  "\nmemcpy((char*)data.data+%d, &A_Timeval(%d, %d), %d);",
                  offset, s_exp->tv->tv_sec, s_exp->tv->tv_usec,
                  sizeof(struct timeval));
            } else
              sprintf(buf, "\nmemcpy((char*)data.data+%d, &%s, %d);", offset,
                  s_exp->exp, sizeof(struct timeval));
          }
          break;
        default:
          rc = ERR_DATATYPE;
          EM_error(target->pos, rc, __LINE__, __FILE__,
              "type not supported in UPDATE");
          goto exit;
        } /* end of switch (f->ty->kind) */
        exe = expTy_Seq(exe, buf);
      }
        break;
      case A_refVar: {
        T_expty t_dec, t_exp;

        rc = transRef(venv, tenv, target->u.ref, sql, t_dec, t_exp);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
              "transRef");
          goto exit;
        }
        if (s_exp->ty != t_exp->ty && !extendType(s_exp->ty, t_exp->ty)) {
          rc = ERR_DATATYPE;
          EM_error(target->pos, rc, __LINE__, __FILE__, "UPDATE assignment");
          goto exit;
        }
        sprintf(buf, "\n%s = %s;", t_exp->exp, s_exp->exp);
        exe = expTy_Seq(exe, buf);
      }
        break;
      default:
        rc = ERR_NTSQL_INTERNAL;
        EM_error(0, rc, __LINE__, __FILE__, "transSelOpr()");
        goto exit;
      }
    } /* end of for */

    if (key_updated > 0) {
      if (te->u.var.index != (A_index) 0 && te->u.var.index->kind
          == INDEX_RTREE && key_updated == 2) {
        exe = expTy_Seq(exe, rtree_update_buf);
      }

      //can't find the correct env entry thus do this in-place
      /*if(te->u.var.index != (A_index)0
       && te->u.var.index->kind == INDEX_BTREE
       && te->u.var.index->func != (S_symbol)0) {
       rc = assignFuncBtreeKey(venv, te, buf);
       if (rc) {
       EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr", "assignFuncBtreeKey");
       goto exit;
       }
       exe = expTy_Seq(exe, buf);
       }*/
      if (te->u.var.index != (A_index) 0 && te->u.var.index->kind
          == INDEX_BTREE && te->u.var.index->func != (S_symbol) 0) {
        E_enventry x = (E_enventry) S_look(venv, te->u.var.index->func);
        char callbuf[1024];
        char retType[80];

        if (!x && !x->kind == E_extEntry) {
          rc = ERR_INVALID_INDEX_SPEC;
          EM_error(0, rc, __LINE__, __FILE__, "External func not found.");
          goto exit;
        }

        switch (x->u.ext.result->kind) {
        case Ty_int:
          sprintf(retType, "int");
          break;
        case Ty_real:
          sprintf(retType, "double");
          break;
        case Ty_string:
          sprintf(retType, "char*");
          break;
        case Ty_timestamp:
          sprintf(retType, "struct timeval");
          break;
        default:
          rc = ERR_DATATYPE;
          EM_error(0, rc, __LINE__, __FILE__, "type not supported in UPDATE");
          goto exit;
        }
        sprintf(buf, "\nif(_adl_func_%d == NULL) { /*1*/"
          "\n_adl_func_%d = (%s(*)(%s))_adl_dlm(\"%s\", \"%s\"); //h1"
          "\n}", UID(x), UID(x), retType, argbuf, S_name(x->u.ext.externlib),
            S_name(x->u.ext.actual));
        exe = expTy_Seq(exe, buf);

        if (isESL()) {
          sprintf(
              buf,
              "\nif(_adl_func_%d == NULL) { /*2*/"
                "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"Error in query %s: external func not found\");"
                "\nreturn s_failure;"
                "\n}", UID(x), getUserName(), getQueryName());
        } else if (isESLAggr()) {
          sprintf(
              buf,
              "\nif(_adl_func_%d == NULL) {/*3*/"
                "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"Error in Aggregate %s: external func not found\");"
                "\nreturn;"
                "\n}", UID(x), getUserName(), getAggrName());
        } else {
          sprintf(buf, "\nif(_adl_func_%d == NULL) { /*4*/"
            "\nfprintf(stderr, \"ERR: external func not found\\n\");"
            "\nexit(1);"
            "\n}", UID(x));
        }
        exe = expTy_Seq(exe, buf);

        sprintf(temp, "%d", te->u.var.index->len);
        switch (x->u.ext.result->kind) {
        case Ty_int:
          sprintf(callbuf, "\nint ret_%d = (int)((*_adl_func_%d)(%s));",
              UID(x), UID(x), argbuf);
          sprintf(retType, "sizeof(int)");
          sprintf(fName, "&ret_%d", UID(x));
          break;
        case Ty_real:
          sprintf(callbuf, "\ndouble ret_%d = (double)((*_adl_func_%d)(%s));",
              UID(x), UID(x), argbuf);
          sprintf(retType, "sizeof(double)");
          sprintf(fName, "&ret_%d", UID(x));
          break;
        case Ty_string:
          sprintf(callbuf, "\nchar* ret_%d = (char*)((*_adl_func_%d)(%s));",
              UID(x), UID(x), argbuf);
          sprintf(retType, "%d", x->u.ext.size);
          sprintf(fName, "ret_%d", UID(x));
          break;
        case Ty_timestamp:
          sprintf(callbuf,
              "\ntimestamp ret_%d = (timestamp)((*_adl_func_%d)(%s));", UID(x),
              UID(x), argbuf);
          sprintf(retType, "sizeof(timestamp)");
          sprintf(fName, "&ret_%d", UID(x));
          break;
        case Ty_iext:
          sprintf(callbuf,
              "\nstruct iExt_ ret_%d = (struct iExt_)((*_adl_func_%d)(%s));",
              UID(x), UID(x), argbuf);
          sprintf(retType, "sizeof(struct iExt_)");
          sprintf(fName, "ret_%d", UID(x));
          break;
        case Ty_rext:
          sprintf(callbuf,
              "\nstruct rExt_ ret_%d = (struct rExt_)((*_adl_func_%d)(%s));",
              UID(x), UID(x), argbuf);
          sprintf(retType, "sizeof(struct rExt_)");
          sprintf(fName, "ret_%d", UID(x));
          break;
        case Ty_cext:
          sprintf(callbuf,
              "\nstruct cExt_ ret_%d = (struct cExt_)((*_adl_func_%d)(%s));",
              UID(x), UID(x), argbuf);
          sprintf(retType, "sizeof(struct cExt_)");
          sprintf(fName, "ret_%d", UID(x));
          break;
        case Ty_text:
          sprintf(callbuf,
              "\nstruct tExt_ ret_%d = (struct tExt_)((*_adl_func_%d)(%s));",
              UID(x), UID(x), argbuf);
          sprintf(retType, "sizeof(struct tExt_)");
          sprintf(fName, "ret_%d", UID(x));
          break;
        default:
          rc = ERR_DATATYPE;
          EM_error(0, rc, __LINE__, __FILE__, "type not supported in UPDATE");
          goto exit;
        }
        sprintf(buf, "%s\nmemcpy((char*)key.data, %s, %s);", callbuf, fName,
            (te->u.var.index->len > 0) ? temp : retType);
        exe = expTy_Seq(exe, buf);
      }

      // store the updated tuple to a temporary relation
      sprintf(buf, "\ninsertTemp(%d, _rec_id, &key, &data);", UID(a));
      exe = expTy_Seq(exe, buf);

      // delete the current tuple
      rc = transCursorDelete2C(cursor_name, buf);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
            "transCursorUpdate2C");
        goto exit;
      }
      // We will insert the new tuple after the entire UPDATE is
      // finished. This is done in transSqlStatement().
      sql->update_mode = USE_TEMP;
    } else {
      rc = transCursorPut2C(cursor_name, buf);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
            "transCursorUpdate2C");
        goto exit;
      }
    }

    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, "\n/*UPDATE ENDS*/");

  } else if (a->kind == A_SQL_DELETE) {
    ///////////// DELETE /////////////////
    A_qun q = (A_qun) getNthElementList(a->jtl_list, 0);
    char cursor_name[128];
    E_enventry x;

    /* just for the purpose of passing in te->u.var.scope
     * to transCursorDelete2C
     */
    /* x = (E_enventry)S_look(venv, q->alias); */
    sprintf(cursor_name, "%s_%d", S_name(q->alias), UID(q->ppnode));
    if (q->kind == QUN_NAME && strcmp(S_name(q->u.table.name), "inwindow") == 0) {
      //here we can look up inwindow
      //and go through the try to dealloc the iExts
      x = (E_enventry) S_look(venv, S_Symbol("inwindow"));
      if (!x || x->kind != E_varEntry) {
        rc = ERR_NTSQL_INTERNAL;
        EM_error(a->pos, rc, __LINE__, __FILE__, "inwindow must exist");
        goto exit;
      }
      if (x->u.var.ty->kind != Ty_record) {
        rc = ERR_NTSQL_INTERNAL;
        EM_error(a->pos, rc, __LINE__, __FILE__,
            "inwindow must have record type");
        goto exit;
      }

      //get the value at current tuple
      sprintf(buf, "\nmemset(&key, 0, sizeof(key));"
        "\nmemset(&data, 0, sizeof(data));"
        "\nrc = %s->c_get(%s, &key, &data, DB_CURRENT);", cursor_name,
          cursor_name);
      exe = expTy_Seq(exe, buf);
      if (isESL()) {
        sprintf(
            buf,
            "\nif(rc == DB_NOTFOUND) {"
              "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DBC->c_get()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            buf,
            "\nif(rc == DB_NOTFOUND) {"
              "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DBC->c_get()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(buf, "\nif(rc == DB_NOTFOUND) adlabort(rc, \"DBC->c_get()\");");
      }
      exe = expTy_Seq(exe, buf);

      sprintf(buf, "\nstruct iExt_ tmpIext_%d;"
        "\nstruct rExt_ tmpRext_%d;"
        "\nstruct cExt_ tmpCext_%d;"
        "\nstruct tExt_ tmpText_%d;", UID(q), UID(q), UID(q), UID(q));
      exe = expTy_Seq(exe, buf);
      int offset = 0;
      for (int i = 0; i < x->u.var.ty->u.record->length; i++) {
        Ty_field fi = (Ty_field) getNthElementList(x->u.var.ty->u.record, i);
        if (fi->ty == Ty_IExt()) {
          sprintf(
              buf,
              "\nmemcpy(&tmpIext_%d, (char*)data.data+%d, sizeof(struct iExt_));"
                "\n//printf(\"here deletin (%%d, %%d)\\n\", tmpIext_%d.length, tmpIext_%d.pt[1]);fflush(stdout);"
                "\n_deleteiext(tmpIext_%d);", UID(q), offset, UID(q), UID(q),
              UID(q));
          exe = expTy_Seq(exe, buf);
          offset += sizeof(struct iExt_);
        } else if (fi->ty == Ty_RExt()) {
          sprintf(
              buf,
              "\nmemcpy(&tmpRext_%d, (char*)data.data+%d, sizeof(struct rExt_));"
                "\n//printf(\"here deletin (%%d, %%d)\\n\", tmpRext_%d.length, tmpRext_%d.pt[1]);fflush(stdout);"
                "\n_deleterext(tmpRext_%d);", UID(q), offset, UID(q), UID(q),
              UID(q));
          exe = expTy_Seq(exe, buf);
          offset += sizeof(struct rExt_);
        }
        if (fi->ty == Ty_CExt()) {
          sprintf(
              buf,
              "\nmemcpy(&tmpCext_%d, (char*)data.data+%d, sizeof(struct cExt_));"
                "\n//printf(\"here deletin (%%d, %%d)\\n\", tmpCext_%d.length, tmpCext_%d.pt[1]);fflush(stdout);"
                "\n_deletecext(tmpCext_%d);", UID(q), offset, UID(q), UID(q),
              UID(q));
          exe = expTy_Seq(exe, buf);
          offset += sizeof(struct cExt_);
        }
        if (fi->ty == Ty_TExt()) {
          sprintf(
              buf,
              "\nmemcpy(&tmpText_%d, (char*)data.data+%d, sizeof(struct tExt_));"
                "\n//printf(\"here deletin (%%d, %%d)\\n\", tmpText_%d.length, tmpText_%d.pt[1]);fflush(stdout);"
                "\n_deletetext(tmpText_%d);", UID(q), offset, UID(q), UID(q),
              UID(q));
          exe = expTy_Seq(exe, buf);
          offset += sizeof(struct tExt_);
        } else if (fi->ty == Ty_Int()) {
          offset += sizeof(int);
        } else if (fi->ty == Ty_Real()) {
          offset += sizeof(double);
        } else if (fi->ty == Ty_Timestamp()) {
          offset += sizeof(timestamp);
        } else if (fi->ty == Ty_String()) {
          offset += fi->size;
        }
      }

      //if we have a plist then mark idth tuple as deleted
      sprintf(buf, "\nif(plist != NULL) {"
        "\nint tmpid = 0;"
        "\nmemcpy(&tmpid, ((char*)data.data)+status->win->plidbegin, sizeof(int));"
        "\n(*plist)[tmpid]->deleted = true;"
        "\n}");
      exe = expTy_Seq(exe, buf);
    }
    rc = transCursorDelete2C(cursor_name, buf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
          "transCursorDelete2C");
      goto exit;
    }
    exe = expTy_Seq(exe, "\n/*DELETE STARTS*/");
    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, "\n/*DELETE ENDS*/");
  } else if (cur_sqlopr_flag == 2 || cur_sqlopr_flag == 3 /*EXISTS/NOT EXISTS*/) {
    /* there's no head expression, really */
    char field[20];
    Ty_field curfield;
    t_fields = A_List(0, A_FIELD);

    sprintf(field, "exists");
    curfield = Ty_Field(new_Symbol(field), Ty_Int(), sizeof(int));
    appendElementList(t_fields, (nt_obj_t*) curfield);
  } else {
    /* compute Head Expr */
    t_fields = A_List(0, A_FIELD);
    for (i = 0; i < a->hxp_list->length; i++) {
      A_selectitem arg = (A_selectitem) getNthElementList(a->hxp_list, i);
      Ty_field curfield;
      S_symbol varsym;

      if (arg->kind == SIMPLE_ITEM) {
        T_expty d, e;
        rc = transExp(venv, tenv, arg->u.s.exp, sql, d, e, aggregates);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
              "transExp");
          goto exit;
        }

        if (d != (T_expty) 0) {
          /* there are (scalar) subqueries in head expr */
          exe = expTy_Seq(exe, d);
        }

        // make assignments AND create return structure
        //
        // The SelOpr will have a return structure:
        // struct {
        //       type field_0;
        //	 type field_1;
        // }
        // Here, field_0, field_1 are names of the head expression.
        // If user provides alias, then we use alias as names.
        // Otherwise, we try as much as possible to use the variable name.
        // For instance, query:
        //	SELECT id, salary
        //	FROM emp;
        // will have a return structure
        // struct {
        //	int id;
        //	int salary;
        // }
        // The above approach is used only if there's no duplicates.
        //
        if (arg->u.s.alias != (S_symbol) 0) {
          /* if the hxp has an alias, use the alias as its field name */
          curfield = Ty_Field(arg->u.s.alias, e->ty, e->size);
          if (curfield->ty->kind == Ty_string) {
            sprintf(buf, "\nmemcpy(%s.%s, %s, %d);"
              "\n%s.%s[%d]=0;", name, S_name(arg->u.s.alias), e->exp,
                curfield->size, name, S_name(arg->u.s.alias), curfield->size);//string_length
          } else if (curfield->ty->kind == Ty_timestamp) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(timestamp));",
            //"\n%s.%s.tv_sec =  %s.tv_sec;"
                //"\n%s.%s.tv_usec = %s.tv_usec;",
                //name, S_name(arg->u.s.alias), e->exp,
                name, S_name(arg->u.s.alias), e->exp);
          } else if (curfield->ty->kind == Ty_iext) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct iExt_));",
                name, S_name(arg->u.s.alias), e->exp);
          } else if (curfield->ty->kind == Ty_rext) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct rExt_));",
                name, S_name(arg->u.s.alias), e->exp);
          } else if (curfield->ty->kind == Ty_cext) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct cExt_));",
                name, S_name(arg->u.s.alias), e->exp);
          } else if (curfield->ty->kind == Ty_text) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct tExt_));",
                name, S_name(arg->u.s.alias), e->exp);
          } else {
            sprintf(buf, "\n%s.%s = %s;", name, S_name(arg->u.s.alias), e->exp);
          }
        } else if (
        // 		   (!a->jtl_list ||
        // 		     a->jtl_list->length<=1) &&
        arg->u.s.exp->kind == A_varExp && (varsym = getVarNameSuffix(
            arg->u.s.exp->u.var)) && isFieldNameP(t_fields, varsym) == 0) {
          /* use the name (last field name, if it's a field
           variable, and it has not already been used  */

          curfield = Ty_Field(varsym, e->ty, e->size);
          if (curfield->ty->kind == Ty_string) {
            sprintf(buf, "\nmemcpy(%s.%s, %s, %d);"
              "\n%s.%s[%d]=0;", name, S_name(varsym), e->exp, curfield->size,
                name, S_name(varsym), curfield->size);//string length
          } else if (curfield->ty->kind == Ty_timestamp) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(timestamp));",
            //"\n%s.%s.tv_sec =  %s.tv_sec;"
                //"\n%s.%s.tv_usec = %s.tv_usec;",
                //name, S_name(varsym), e->exp,
                name, S_name(varsym), e->exp);
          } else if (curfield->ty->kind == Ty_iext) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct iExt_));",
            //"\n%s.%s.tv_sec =  %s.tv_sec;"
                //"\n%s.%s.tv_usec = %s.tv_usec;",
                //name, S_name(varsym), e->exp,
                name, S_name(varsym), e->exp);
          } else if (curfield->ty->kind == Ty_rext) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct rExt_));",
                name, S_name(varsym), e->exp);
          } else if (curfield->ty->kind == Ty_cext) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct cExt_));",
                name, S_name(varsym), e->exp);
          } else if (curfield->ty->kind == Ty_text) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct tExt_));",
                name, S_name(varsym), e->exp);
          } else {
            sprintf(buf, "\n%s.%s = %s;", name, S_name(varsym), e->exp);
          }
        } else {
          /* last resort: use "field_%d" as its field name */
          char field[20];
          sprintf(field, "field_%d", i);
          curfield = Ty_Field(new_Symbol(field), e->ty, e->size);
          if (curfield->ty->kind == Ty_string) {
            sprintf(buf,
                "\nmemcpy(%s.%s, %s, %d);" //memcpy4
                  "\n%s.%s[%d]=0;", name, field, e->exp, curfield->size, name,
                field, curfield->size);
          } else if (curfield->ty->kind == Ty_timestamp) {
            if (e->tv == NULL)
              sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(timestamp));",
              //"\n%s.%s.tv_sec = %s.tv_sec;"
                  //"\n%s.%s.tv_usec = %s.tv_usec;",
                  //name, field, e->exp,
                  name, field, e->exp);
            else
              sprintf(buf, "\n%s.%s.tv_sec =  %d;"
                "\n%s.%s.tv_usec = %d;", name, field, e->tv->tv_sec, name,
                  field, e->tv->tv_usec);
          } else if (curfield->ty->kind == Ty_iext) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct iExt_));",
            //"\n%s.%s.tv_sec = %s.tv_sec;"
                //"\n%s.%s.tv_usec = %s.tv_usec;",
                //name, field, e->exp,
                name, field, e->exp);
          } else if (curfield->ty->kind == Ty_rext) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct rExt_));",
                name, field, e->exp);
          } else if (curfield->ty->kind == Ty_cext) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct cExt_));",
                name, field, e->exp);
          } else if (curfield->ty->kind == Ty_text) {
            sprintf(buf, "\nmemcpy(&(%s.%s), &%s, sizeof(struct tExt_));",
                name, field, e->exp);
          } else {
            sprintf(buf, "\n%s.%s = %s;", name, field, e->exp);//string length
          }
        }

        appendElementList(t_fields, (nt_obj_t*) curfield);
        exe = expTy_Seq(exe, buf);
      }
    }
  }

  if (cur_sqlopr_flag == 1 /* SCALAR */) {
    /* we need to go back and try again to see if it returns more than
     one tuple */
    sprintf(buf, "\nif (%s_cnt++ ==0) goto next_%d; /* scalar opr */", name,
        UID(a));
    exe = expTy_Seq(exe, buf);
  }

  if (isESL())
    exe = expTy_Seq(exe, "\n//output_count++;"
      "\n} /* if (rc == 0) */");
  else
    exe = expTy_Seq(exe, "\n} /* if (rc == 0) */");

  /*
   * for scalar query we shall succeed once and only once
   */
  if (cur_sqlopr_flag == 1 /* SCALAR */) {
    //sprintf(buf,
    //    "\nif (%s_cnt == 0) {"
    //    "\nfprintf(stderr, \"ERR: scalar subquery returns no tuple at line %d.\\n\");"
    //    "\nexit(1);"
    //    "\n}",
    //    name, a->pos);
    sprintf(buf, "\nif (%s_cnt == 0) {", name);
    exe = expTy_Seq(exe, buf);

    if (isESL()) {
      sprintf(
          buf,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"Error in query %s: scalar subquery returns no tuple at line %d.\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName(), a->pos);
    } else if (isESLAggr()) {
      sprintf(
          buf,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"Error in Aggregate %s: scalar subquery returns no tuple at line %d.\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName(), a->pos);
    } else {
      sprintf(buf,
          "\nfprintf(stderr, \"ERR: scalar subquery returns no tuple at line %d.\\n\");"
            "\nexit(1);"
            "\n}", a->pos);
    }
    exe = expTy_Seq(exe, buf);

    //sprintf(buf, "\nelse if (%s_cnt >  1) {"
    //    "\nfprintf(stderr, \"ERR: scalar subquery returns more than one tuple at line %d.\\n\");"
    //    "\nexit(1);"
    //    "\n}"
    //    "\nrc = 0;"
    //    , name, a->pos);
    sprintf(buf, "\nelse if (%s_cnt >  1) {", name, a->pos);
    exe = expTy_Seq(exe, buf);

    if (isESL()) {
      sprintf(
          buf,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"Error in query %s: scalar subquery returns more than one tuple at line %d.\");"
            "\nreturn s_failure;"
            "\n}"
            "\nrc = 0;", getUserName(), getQueryName(), a->pos);
    } else if (isESLAggr()) {
      sprintf(
          buf,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"Error in Aggregate %s: scalar subquery returns more than one tuple at line %d.\");"
            "\nreturn;"
            "\n}"
            "\nrc = 0;", getUserName(), getAggrName(), a->pos);
    } else {
      sprintf(
          buf,
          "\nfprintf(stderr, \"ERR: scalar subquery returns more than one tuple at line %d.\\n\");"
            "\nexit(1);"
            "\n}"
            "\nrc = 0;", a->pos);
    }
    exe = expTy_Seq(exe, buf);
  }

  if (cur_sqlopr_flag == 2 || cur_sqlopr_flag == 3 /* EXIST/NOTEXIST*/) {
    sprintf(buf, "\n%s.exists=%s;", name, (cur_sqlopr_flag == 2) ? "(rc==0)"
        : "(rc==DB_NOTFOUND)");
    exe = expTy_Seq(exe, buf);
    A_qun qun = (A_qun) getNthElementList(a->jtl_list, 0);
    //setSqlOprFlag(oper, sql, a->u.op.left); // EXIST,NOT-EXIST,SCALAR
    if ((qun->kind == QUN_NAME || qun->kind == QUN_FUNCTION) && a->kind
        == A_SQL_SEL) {
      sprintf(buf, "\nfirst_entry_%d = 1;", UID(qun));
      exe = expTy_Seq(exe, buf);
    }
  }

  if (cur_sqlopr_flag == 2 /* EXIST */) {
    /*
     * Very offen, an EXISTS query returns TRUE before it finishes
     * scanning the tables. So when it does, we need to initialize all
     * the tables used within the EXISTS query, so that the next entry
     * will start from the beginning of the tables.
     */

    T_expty lexe;
    rc = transInitSelOpr(a, lexe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSelOpr",
          "transInitSelOpr");
      goto exit;
    }
    sprintf(buf, "\nif (rc == 0) {");
    exe = expTy_Seq(exe, buf);
    exe = expTy_Seq(exe, lexe);
    exe = expTy_Seq(exe, "\n}");
  }

  if (prep) {
    exe = expTy_Seq(exe, "\n} /* end of precomputed predicates */");
  }

  if (a->kind == A_SQL_SEL)
    exe->ty = Ty_Record(t_fields);

  S_endScope(tenv);
  S_endScope(venv);

  exit:
  //printf("done with selopr\n");
  DBUG_RETURN(rc);
}

//Assumes that the var is var type and has a index specified
err_t assignFuncBtreeKey(S_table venv, E_enventry te, char *buf) {
  SMLog::SMLOG(10, "Entering assignFuncBtreeKey");
  err_t rc = ERR_NONE;
  char linebuf[MAX_STR_LEN];
  int offset;
  Ty_field f;
  A_list keydecs = te->u.var.index->keydecs;
  char argbuf[1024];
  char callbuf[2048];
  char retType[80];
  argbuf[0] = '\0';
  char fName[80];

  E_enventry x = (E_enventry) S_look(venv, te->u.var.index->func);

  if (!x && x->kind != E_extEntry) {
    rc = ERR_INVALID_INDEX_SPEC;
    EM_error(0, rc, __LINE__, __FILE__, "External func not found.");
    goto exit;
  }

  offset = 0;
  char temp[32];
  for (int i = 0; i < keydecs->length; i++) {
    S_symbol skey = (S_symbol) getNthElementList(keydecs, i);
    rc = searchRecordField(te->u.var.ty->u.record, skey, (int*) 0, &offset, f);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "assignFuncBtreeKey",
          "searchRecordField");
      goto exit;
    }

    if (f == (Ty_field) 0) {
      rc = ERR_UNDEFINED_VARIABLE;
      EM_error(0, rc, __LINE__, __FILE__, S_name(skey));
      goto exit;
    }
    //get value from data part convert to right type and pass to external func
    if (i != 0)
      strcat(argbuf, ", ");
    switch (f->ty->kind) {
    case Ty_int:
      sprintf(linebuf, "\nint field_%d;"
        "\nmemcpy(&field_%d, ((char*)data.data+%d), sizeof(int));", i, i,
          offset);
      strcat(buf, linebuf);
      sprintf(temp, "field_%d", i);
      strcat(argbuf, temp);
      break;
    case Ty_real:
      sprintf(linebuf, "\ndouble field_%d;"
        "\nmemcpy(&field_%d, ((char*)data.data+%d), sizeof(double));", i, i,
          offset);
      strcat(buf, linebuf);
      sprintf(temp, "field_%d", i);
      strcat(argbuf, temp);
      break;
    case Ty_string:
      sprintf(linebuf, "\nchar field_%d[%d];"
        "\nmemcpy(field_%d, ((char*)data.data+%d), %d);", i, f->size, i,
          offset, f->size);
      strcat(buf, linebuf);
      sprintf(temp, "field_%d", i);
      strcat(argbuf, temp);
      break;
    case Ty_timestamp:
      sprintf(linebuf, "\ntimestamp field_%d;"
        "\nmemcpy(&field_%d, ((char*)data.data+%d), sizeof(timestamp));", i, i,
          offset);
      strcat(buf, linebuf);
      sprintf(temp, "field_%d", i);
      strcat(argbuf, temp);
      break;
    default:
      rc = ERR_DATATYPE;
      EM_error(0, rc, __LINE__, __FILE__, "type not supported in UPDATE");
      goto exit;
    }
  }

  switch (x->u.ext.result->kind) {
  case Ty_int:
    sprintf(retType, "int");
    break;
  case Ty_real:
    sprintf(retType, "double");
    break;
  case Ty_string:
    sprintf(retType, "char*");
    break;
  case Ty_timestamp:
    sprintf(retType, "struct timeval");
    break;
  default:
    rc = ERR_DATATYPE;
    EM_error(0, rc, __LINE__, __FILE__, "type not supported in UPDATE");
    goto exit;
  }

  sprintf(linebuf, "\nif(_adl_func_%d == NULL) { /*5*/"
    "\n_adl_func_%d = (%s(*)(%s))_adl_dlm(\"%s\", \"%s\"); //h2"
    "\n}", UID(x), UID(x), retType, argbuf, S_name(x->u.ext.externlib), S_name(
      x->u.ext.actual));
  strcat(buf, linebuf);

  if (isESL()) {
    sprintf(
        linebuf,
        "\nif(_adl_func_%d == NULL) {"
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"Error in query %s: external func not found\");"
          "\nreturn s_failure;"
          "\n}", UID(x), getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        linebuf,
        "\nif(_adl_func_%d == NULL) {"
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"Error in Aggregate %s: external func not found\");"
          "\nreturn;"
          "\n}", UID(x), getUserName(), getAggrName());
  } else {
    sprintf(linebuf, "\nif(_adl_func_%d == NULL) {"
      "\nfprintf(stderr, \"ERR: external func not found\\n\");"
      "\nexit(1);"
      "\n}", UID(x));
  }
  strcat(buf, linebuf);

  sprintf(temp, "%d", te->u.var.index->len);
  switch (x->u.ext.result->kind) {
  case Ty_int:
    sprintf(callbuf, "\nint ret_%d = (int)((*_adl_func_%d)(%s));", UID(x),
        UID(x), argbuf);
    sprintf(retType, "sizeof(int)");
    sprintf(fName, "&ret_%d", UID(x));
    break;
  case Ty_real:
    sprintf(callbuf, "\ndouble ret_%d = (double)((*_adl_func_%d)(%s));",
        UID(x), UID(x), argbuf);
    sprintf(retType, "sizeof(double)");
    sprintf(fName, "&ret_%d", UID(x));
    break;
  case Ty_string:
    sprintf(callbuf, "\nchar* ret_%d = (char*)((*_adl_func_%d)(%s));", UID(x),
        UID(x), argbuf);
    sprintf(retType, "%d", x->u.ext.size);
    sprintf(fName, "ret_%d", UID(x));
    break;
  case Ty_timestamp:
    sprintf(callbuf, "\ntimestamp ret_%d = (timestamp)((*_adl_func_%d)(%s));",
        UID(x), UID(x), argbuf);
    sprintf(retType, "sizeof(timestamp)");
    sprintf(fName, "&ret_%d", UID(x));
    break;
  default:
    rc = ERR_DATATYPE;
    EM_error(0, rc, __LINE__, __FILE__, "type not supported in UPDATE");
    goto exit;
  }
  sprintf(linebuf, "%s\nmemcpy(key.data, %s, %s);", callbuf, fName,
      (te->u.var.index->len > 0) ? temp : retType);
  strcat(buf, linebuf);

  exit: return rc;
}

/* recursiveSqlDec() searches a query tree recursively and declares
 all the tables and cursors used in the query tree */
err_t recursiveSqlDec(S_table venv, A_sqlopr s, A_list db_list,
    A_list cursor_list, A_list qun_list, A_list index_list) {
  SMLog::SMLOG(10, "Entering recursiveSqlDec");
  err_t rc = ERR_NONE;
  int i, j;

  if (!s || !s->jtl_list)
    goto exit;

  // define index
  if (s->jtl_list->length > 1) {
    appendElementList(index_list, (nt_obj_t*) s);
  }

  for (i = 0; i < s->jtl_list->length; i++) {
    A_qun q = (A_qun) getNthElementList(s->jtl_list, i);

    // detect duplicates
    for (j = 0; j < i; j++) {
      A_qun p = (A_qun) getNthElementList(s->jtl_list, j);
      if (q->alias == p->alias) {
        rc = ERR_DUPLICATE_JOIN_NAME;
        EM_error(p->pos, rc, __LINE__, __FILE__, S_name(p->alias));
        goto exit;
      }
    }

    switch (q->kind) {
    case QUN_NAME: {
      E_enventry x = (E_enventry) S_look(venv, q->u.table.name);
      if (!x || x->kind != E_varEntry) {
        rc = ERR_UNDEFINED_VARIABLE;
        EM_error(q->pos, rc, __LINE__, __FILE__, S_name(q->u.table.name));
        goto exit;
      }
      // define db
      if (!memberPList(db_list, (nt_obj_t*) q->u.table.name))
        appendElementList(db_list, (nt_obj_t*) q->u.table.name);
      // define cursor
      appendElementList(cursor_list, (nt_obj_t*) q);
      // define qun
      appendElementList(qun_list, (nt_obj_t*) q);
    }
      break;
    case QUN_FUNCTION:
      break;
    case QUN_WINDOW:
      break;
    case QUN_QUERY: {
      A_exp a = q->u.query;
      A_exp ns;

      if (a->kind == A_sqloprExp) {
        ns = a;
        /*
         rewriting is done in sql_rewrite.cc

         } else if (a->kind == A_selectExp) {
         rc = constructSelOp(venv, a, ns);
         if (rc) {
         EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "recursiveSqlDec", "constructSelOp");
         goto exit;
         }
         q->u.query = ns;
         */
      } else {
        rc = ERR_NTSQL_INTERNAL;
        EM_error(0, rc, __LINE__, __FILE__, "recursiveSqlDec()");
        goto exit;
      }

      rc = recursiveSqlDec(venv, ns->u.sqlopr, db_list, cursor_list, qun_list,
          index_list);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "recursiveSqlDec",
            "recursiveSqlDec");
        goto exit;
      }
      // define qun
      appendElementList(qun_list, (nt_obj_t*) q);
    }
      break;
    }
  }
  exit: return rc;
}

err_t transInsOpr(S_table venv, S_table tenv, A_sqlopr a, T_expty &result) {
  err_t rc = ERR_NONE;

  exit: return rc;
}

/* compile the arguments of a function calling */
err_t compileArgs(S_table venv, /* variable env */
S_table tenv, /* type env */
Sql_sem sql, /* sql semantics */
A_list argdecs, A_list formals, T_expty &exe, vector<void*> aggregates) // result
{
  SMLog::SMLOG(10, "Entering compileArgs");
  err_t rc = ERR_NONE;
  int i;

  // compile arguments
  exe = (T_expty) 0;

  for (i = 0; i < formals->length; i++) {
    A_exp a_exp = (A_exp) getNthElementList(formals, i);
    T_expty ad, ae;

    rc = transExp(venv, tenv, a_exp, sql, ad, ae, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "compileArgs", "transExp");
      goto exit;
    }

    /* TO DO : check type compatibility */

    if (i > 0) {
      exe = expTy_Seq(exe, ", ");
    }
    exe = expTy_Seq(exe, ae);
  }

  exit: return rc;
}

static void setSqlOprFlag(A_oper oper, Sql_sem sql, A_exp a) {
  if (a && (a->kind == A_sqloprExp || a->kind == A_selectExp)) {
    if (oper == A_existOp) {
      /* in case there are sub-queries embeded inside the expr */
      sql->cur_sqlopr_flag = 2; /* EXISTS */
    } else if (oper == A_notexistOp) {
      sql->cur_sqlopr_flag = 3; /* EXISTS */
    } else {
      sql->cur_sqlopr_flag = 1; /* SCALAR */
    }
  }
}

err_t transRef(S_table venv, S_table tenv, A_ref ref, Sql_sem sql,
    T_expty &dec, T_expty &exe) {
  SMLog::SMLOG(10, "Entering transRef");
  err_t rc = ERR_NONE;
  char buf[MAX_STR_LEN], typebuf[64], source[MAX_STR_LEN], *sourcep;
  Ty_field f;
  int idx, off;

  DBUG_ENTER("transRef");

  switch (ref->kind) {
  case A_refRef:
    rc = transRef(venv, tenv, ref->u.ref, sql, dec, exe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transRef", "transRef");
      goto exit;
    }
    sprintf(source, "(TLL*)(%s)", exe->exp);
    sourcep = source;
    break;
  case A_varRef:
    rc = transVar(venv, tenv, ref->u.var, sql, (A_qun*) 0, (int*) 0, exe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transRef", "transVar");
      goto exit;
    }

    sprintf(source, "(TLL*)(%s)", exe->exp);
    sourcep = source;

    break;
  }

  if (exe->ty->kind != Ty_ref && exe->ty->kind != Ty_record) {
    rc = ERR_NON_REFERENCE_TYPE;
    EM_error(ref->pos, rc, __LINE__, __FILE__, S_name(ref->col));
    goto exit;
  }

  rc = searchRecordField(exe->ty->u.record, ref->col, &idx, &off, f);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transRef",
        "searchRecordField");
    goto exit;
  }

  if (f == (Ty_field) 0) {
    rc = ERR_UNDEFINED_FIELD;
    EM_error(ref->pos, rc, __LINE__, __FILE__, S_name(ref->col));
    goto exit;
  }

  rc = Dereference2C(f->ty, typebuf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transRef", "Dereference2C");
    goto exit;
  }

  //sprintf(buf, "%s((char*)((%s)->%s.data)+%d)",
  //	  typebuf, sourcep, (f->iskey)? "key":"data", off+4);
  sprintf(buf, "%s((char*)((%s)->%s.data)+%d)", typebuf, sourcep, "data", off
      + 4);
  expTy_Delete(exe);

  /* we allocate a new buf */
  exe = expTy(Tr_String(buf), f->ty);
  if (f->ty == Ty_String())
    exe->size = f->size;

  exit: DBUG_RETURN(rc);
}

void prepareSchemaString(Ty_ty ty, char *message) {
  SMLog::SMLOG(10, "Entering prepareSchemaString");
  //ty->kind should be record
  A_list fields = ty->u.record;

  for (int i = 0; i < fields->length; i++) {
    Ty_field fi = (Ty_field) getNthElementList(fields, i);

    strcat(message, S_name(fi->name));
    strcat(message, " ");
    switch (fi->ty->kind) {
    case Ty_int:
      strcat(message, "int");
      break;
    case Ty_real:
      strcat(message, "real");
      break;
    case Ty_string:
      char type[10];
      sprintf(type, "char(%d)", fi->size);
      strcat(message, type);
      break;
    case Ty_timestamp:
      strcat(message, "timestamp");
      break;
    case Ty_iext:
      strcat(message, "iext");
      break;
    case Ty_rext:
      strcat(message, "rext");
      break;
    case Ty_cext:
      strcat(message, "cext");
      break;
    case Ty_text:
      strcat(message, "text");
      break;
    default:
      break;
    }
    if (i != fields->length - 1)
      strcat(message, ", ");
  }
}

void prepareWindowMessage(S_symbol name, Ty_ty ty, A_list keydecs,
    char* message) {
  SMLog::SMLOG(10, "Entering prepareWindowMessage");
  char schema[4096];

  memset(schema, 0, 4096);
  sprintf(message, "WINDOW %s(", S_name(name));
  prepareSchemaString(ty, schema);
  strcat(message, schema);
  strcat(message, ")");
  if (!A_ListEmpty(keydecs)) {
    strcat(message, " hash(");
    for (int i = 0; i < keydecs->length; i++) {
      if (i != 0) {
        strcat(message, ", ");
      }
      strcat(message, S_name((S_symbol) getNthElementList(keydecs, i)));
    }
    strcat(message, ");");
  } else {
    strcat(message, ";");
  }
}

void prepareStreamMessage(S_symbol name, Ty_ty ty, S_symbol key, char* message) {
  SMLog::SMLOG(10, "Entering prepareStreamMessage");
  char schema[4096];

  sprintf(message, "STREAM %s(", S_name(name));
  memset(schema, 0, 4096);
  prepareSchemaString(ty, schema);
  strcat(message, schema);
  strcat(message, ")");
  if (key != NULL) {
    strcat(message, " ORDER BY ");
    strcat(message, S_name(key));
  }
  strcat(message, ";");
  //printf("message %s\n", message);
}

err_t assignParameter(char* buf, E_enventry tbl, A_var v, T_expty s_exp,
    S_symbol tblName) {
  SMLog::SMLOG(10, "Entering assignParameter");
  err_t rc = ERR_NONE;
  char linebuf[2048];
  int length1;
  int length2;
  length1 = ((Ty_field) getNthElementList(tbl->u.var.ty->u.record, 0))->size;
  length2 = ((Ty_field) getNthElementList(tbl->u.var.ty->u.record, 1))->size;

  sprintf(linebuf, "\nIM_RELC *pc_%d;"
    "\nint pfirst_entry_%d = 1;"
    "\nint updated_%d = 0;"
    "\nchar pkey_%d[%d];"
    "\nchar val_%d[%d];"
    "\nchar pTableName_%d[256];"
    "\n"
    "\nmemset(val_%d, 0, %d);", UID(v), UID(v), UID(v), UID(v), length1 + 1,
      UID(v), length2 + 1, UID(v), UID(v), length1);
  strcat(buf, linebuf);

  switch (s_exp->ty->kind) {
  case Ty_int: {
    sprintf(linebuf, "\nsprintf(val_%d, \"%%d\", %s);", UID(v), s_exp->exp);
  }
    break;
  case Ty_real: {
    sprintf(linebuf, "\nsprintf(val_%d, \"%%f\", %s);", UID(v), s_exp->exp);
  }
    break;
  case Ty_string: {
    sprintf(linebuf, "\nsprintf(val_%d, %s);", UID(v), s_exp->exp);
  }
    break;
  case Ty_timestamp: {
    sprintf(linebuf, "\nstruct timeval tval_%d = %s;"
      "\nsprintf(val_%d, \"%%d.%%d\", tval_%d.tv_sec, tval_%d.tv_usec);",
        UID(v), s_exp->exp, UID(v), UID(v), UID(v));
  }
    break;
  default: {
    rc = ERR_DATATYPE;
    EM_error(0, rc, __LINE__, __FILE__, "type not supported as func param");
    goto exit;
  }
  }
  strcat(buf, linebuf);

  sprintf(linebuf, "\nsprintf(pTableName_%d, \"%%s%s\", getModelId());",
      UID(v), S_name(tblName));
  strcat(buf, linebuf);

  sprintf(
      linebuf,
      "\nif ((rc = ((IM_REL*)inMemTables->operator[](pTableName_%d))->cursor(((IM_REL*)inMemTables->operator[](pTableName_%d)), &pc_%d, 0)) != 0) {",
      UID(v), UID(v), UID(v));
  strcat(buf, linebuf);

  if (isESL()) {
    sprintf(
        linebuf,
        "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->cursor()\");"
          "\nreturn s_failure;"
          "\n}", getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        linebuf,
        "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->cursor()\");"
          "\nreturn;"
          "\n}", getUserName(), getAggrName());
  } else {
    sprintf(linebuf, "\nadlabort(rc, \"IM_REL->cursor()\");"
      "\n}");
  }
  strcat(buf, linebuf);

  sprintf(linebuf, "\nwhile(rc == 0 && updated_%d == 0) {"
    "\nmemset(&pkey, 0, sizeof(pkey));"
    "\nmemset(&pdata, 0, sizeof(pdata));", UID(v));
  strcat(buf, linebuf);

  sprintf(
      linebuf,
      "\nrc = pc_%d->c_get(pc_%d, &pkey, &pdata, (pfirst_entry_%d)?DB_FIRST:DB_NEXT);",
      UID(v), UID(v), UID(v));
  strcat(buf, linebuf);

  //look up the first and the second columns and their lengths
  sprintf(linebuf, "\nif (rc==0) {"
    "\npfirst_entry_%d = 0;"
    "\nmemcpy(pkey_%d, (char*)pdata.data+0, %d);"
    "\npkey_%d[%d] = \'\\0\';", UID(v), UID(v), length1, UID(v), length1);
  strcat(buf, linebuf);

  sprintf(linebuf, "\nif(strcmp(\"%s\", pkey_%d) == 0) {"
    "\nupdated_%d = 1;"
    "\nmemcpy((char*)pdata.data+%d, val_%d, %d);"
    "\nif ((rc = pc_%d->c_put(pc_%d, &pkey, &pdata, DB_CURRENT)) != 0) {",
      S_name(v->u.simple), UID(v), UID(v), length1, UID(v), length2, UID(v),
      UID(v));
  strcat(buf, linebuf);

  if (isESL()) {
    sprintf(
        linebuf,
        "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_RELC->c_put() or DBC->c_put()\");"
          "\nreturn s_failure;"
          "\n}", getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        linebuf,
        "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_RELC->c_put() or DBC->c_put()\");"
          "\nreturn;"
          "\n}", getUserName(), getAggrName());
  } else {
    sprintf(linebuf, "\nadlabort(rc, \"IM_RELC->c_put() or DBC->c_put()\");"
      "\n}");
  }
  strcat(buf, linebuf);

  sprintf(linebuf, "\n} /* if (strcmp) */"
    "\n} /* if (rc == 0) */"
    "\n} /* while (rc==0) */");
  strcat(buf, linebuf);

  //next we close the above cursor and if updated_%d == 0, we do the insert
  sprintf(linebuf, "\nif (pc_%d && (rc = pc_%d->c_close(pc_%d)) != 0) {",
      UID(v), UID(v), UID(v));
  strcat(buf, linebuf);

  if (isESL()) {
    sprintf(
        linebuf,
        "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_RELC->c_close() or DBC->c_close()\");"
          "\nreturn s_failure;"
          "\n}", getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        linebuf,
        "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_RELC->c_close() or DBC->c_close()\");"
          "\nreturn;"
          "\n}", getUserName(), getAggrName());
  } else {
    sprintf(linebuf,
        "\nadlabort(rc, \"IM_RELC->c_close() or DBC->c_close()\");"
          "\n}");
  }
  strcat(buf, linebuf);

  sprintf(
      linebuf,
      "\nif(updated_%d == 0) {"
        "\nmemset(&pkey, 0, sizeof(pkey));"
        "\nmemset(&pdata, 0, sizeof(pdata));"
        "\npdata.data = pdatadata;"
        "\npkey.data = pkeydata;"
        "\nmemcpy((char*)pdata.data, \"%s\", %d);"
        "\nmemcpy((char*)pdata.data+%d, val_%d, %d);"
        "\npdata.size = %d;"
        "\npkey.size = 0;"
        "\nif ((rc = ((IM_REL*)inMemTables->operator[](\"%s\"))->put(((IM_REL*)inMemTables->operator[](\"%s\")), &pkey, &pdata, DB_APPEND)) != 0) {",
      UID(v), S_name(v->u.simple), length1, length1, UID(v), length2, length1
          + length2, S_name(tbl->key), S_name(tbl->key));
  strcat(buf, linebuf);

  if (isESL()) {
    sprintf(
        linebuf,
        "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->put()\");"
          "\nreturn s_failure;"
          "\n}", getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        linebuf,
        "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->put()\");"
          "\nreturn;"
          "\n}", getUserName(), getAggrName());
  } else {
    sprintf(linebuf, "\nadlabort(rc, \"IM_REL->put()\");"
      "\n}");
  }
  strcat(buf, linebuf);

  sprintf(linebuf, "\n} /* if(updated == 0) */");
  strcat(buf, linebuf);
  exit: return rc;
}

err_t renameItemsInFlowStatement(A_exp a, A_exp statement, char* target,
    char* pre, A_exp& newExp, int &isFinalStmt) {
  SMLog::SMLOG(10, "Entering renameItemsInFlowStatement");
  err_t rc = ERR_NONE;
  char renameSource[256];
  char* unScores = NULL;

  DBUG_ENTER("renameItemsInFlowStatement");

  if (statement->kind == A_runtaskExp) {
    if (statement->u.runtask.source == S_Symbol("INSTREAM")) {
      statement->u.runtask.source = a->u.runtask.source;
    } else {
      unScores = strstr(S_name(statement->u.runtask.source), "__");
      if (unScores == NULL)
        unScores = strstr(S_name(statement->u.runtask.source), "$");
      sprintf(renameSource, "%s%s", pre, unScores == NULL ? S_name(
          statement->u.runtask.source) : unScores + 2);
      statement->u.runtask.source = S_Symbol(strdup(renameSource));
    }
    statement->u.runtask.modelName = a->u.runtask.modelName;
  } else if (statement->kind == A_selectExp) {
    A_list jtl = statement->u.select.join_table_list;
    for (int j = 0; j < jtl->length; j++) {
      A_qun qun = (A_qun) getNthElementList(jtl, j);
      if (qun->kind == QUN_NAME) {
        if (qun->u.table.name == S_Symbol("INSTREAM")) {
          qun->u.table.name = a->u.runtask.source;
        } else {
          unScores = strstr(S_name(qun->u.table.name), "__");
          if (unScores == NULL)
            unScores = strstr(S_name(qun->u.table.name), "$");
          sprintf(renameSource, "%s%s", pre, unScores == NULL ? S_name(
              qun->u.table.name) : unScores + 2);
          qun->u.table.name = S_Symbol(strdup(renameSource));
        }
      } else {
        rc = ERR_CREATE_STREAM;
        EM_error(
            a->pos,
            rc,
            __LINE__,
            __FILE__,
            "Create stream only allowed with select and run task queries, and from clause can only have tables or streams.");
        goto exit;
      }
    }
  } else if (statement->kind == A_createstreamExp) {
    //need to rename the stream
    A_exp csquery = statement->u.createstream.query;
    unScores = strstr(S_name(statement->u.createstream.name), "__");
    if (unScores == NULL)
      unScores = strstr(S_name(statement->u.createstream.name), "$");
    sprintf(renameSource, "%s%s", pre, unScores == NULL ? S_name(
        statement->u.createstream.name) : unScores + 2);
    statement->u.createstream.name = S_Symbol(strdup(renameSource));
    //printf("here %s %s\n", renameSource, S_name(statement->u.createstream.name));

    rc = renameItemsInFlowStatement(a, csquery, target, pre, newExp,
        isFinalStmt);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
          "renameItemsInFlowStatement", "renameItemsInFlowStatement");
      goto exit;
    }
  } else if (statement->kind == A_sqloprExp
      && statement->u.sqlopr->jtl_list->length == 2) {
    A_qun q0 = (A_qun) getNthElementList(statement->u.sqlopr->jtl_list, 0);
    A_qun q1 = (A_qun) getNthElementList(statement->u.sqlopr->jtl_list, 1);

    if (statement->u.sqlopr->kind == A_SQL_INSERT) {
      if (q0->kind == QUN_NAME && q0->u.table.name == S_Symbol("OUTSTREAM")
          && newExp == NULL) {
        //This is the statement which will be returned back, since
        // it is insert into OUTSTREAM, so we indicate accordingly
        //newExp = csquery;
        //indicate to calling fun using ...
        isFinalStmt = 1;
      }
      if (q0->kind == QUN_NAME) {
        if (q0->u.table.name == S_Symbol("OUTSTREAM") || q0->u.table.name
            == S_Symbol("stdout")) {
          if (target != NULL)
            q0->u.table.name = S_Symbol(strdup(target));
          else
            q0->u.table.name = S_Symbol("stdout");
        } else {
          unScores = strstr(S_name(q0->u.table.name), "__");
          if (unScores == NULL)
            unScores = strstr(S_name(q0->u.table.name), "$");
          sprintf(renameSource, "%s%s", pre, unScores == NULL ? S_name(
              q0->u.table.name) : unScores + 2);
          q0->u.table.name = S_Symbol(strdup(renameSource));
        }
      }
      if (q1->kind == QUN_QUERY) {
        A_exp csquery = q1->u.query;
        rc = renameItemsInFlowStatement(a, csquery, target, pre, newExp,
            isFinalStmt);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
              "renameItemsInFlowStatement", "renameItemsInFlowStatement");
          goto exit;
        }
      }
      if (q1->kind != QUN_QUERY || q0->kind != QUN_NAME) {
        rc = ERR_INVALID_STMT_IN_FLOW;
        EM_error(a->pos, rc, __LINE__, __FILE__);
        goto exit;
      }
    } else if (statement->u.sqlopr->kind == A_SQL_UNION) {
      if (q0->kind == QUN_QUERY && q1->kind == QUN_QUERY) {
        rc = renameItemsInFlowStatement(a, q0->u.query, target, pre, newExp,
            isFinalStmt);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
              "renameItemsInFlowStatement", "renameItemsInFlowStatement");
          goto exit;
        }
        rc = renameItemsInFlowStatement(a, q1->u.query, target, pre, newExp,
            isFinalStmt);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
              "renameItemsInFlowStatement", "renameItemsInFlowStatement");
          goto exit;
        }
      } else {
        rc = ERR_INVALID_STMT_IN_FLOW;
        EM_error(a->pos, rc, __LINE__, __FILE__);
        goto exit;
      }
    } else {
      rc = ERR_INVALID_STMT_IN_FLOW;
      EM_error(a->pos, rc, __LINE__, __FILE__);
      goto exit;
    }
  } else {
    rc = ERR_INVALID_STMT_IN_FLOW;
    EM_error(a->pos, rc, __LINE__, __FILE__);
    goto exit;
  }

  exit: DBUG_RETURN(rc);
}

/* Returns the select query equivalent to the runtask query, in newExp param.
 * In case of flows, it returns the first "insert into OUTSTREAM" query or
 * the last statement in the flow.
 */
err_t rewriteRuntaskExp(S_table venv, /* variable env */
S_table tenv, /* type env */
A_exp a, /* abstract syntacs */
Sql_sem sql, /* sql semantics */
T_expty &dec, A_exp &newExp, vector<void*> aggregates, char* target,
    cStmt *&cstmt) {
  SMLog::SMLOG(10, "Entering rewriteRuntaskExp");
  err_t rc = ERR_NONE;
  int length;
  bool found;
  bool foundFlow;
  E_enventry x;
  E_enventry s;
  E_enventry y, z;
  A_modelitem mi;
  A_flow flow;
  Ty_ty sourceTy;
  A_list args;
  A_list from_list;
  A_list select_list;
  A_exp callExp;
  A_list modelitems;
  A_list sharedTables;
  S_symbol sharedTbl;
  char sharedTblName[200];
  char modelId[200];
  char paramInit[MAX_STR_LEN];
  paramInit[0] = '\0';
  A_list params;
  newExp = NULL;

  DBUG_ENTER("transRuntaskExp");

  if (!a->u.runtask.modelName) {
    rc = ERR_SYNTAX;
    EM_error(a->pos, rc, __LINE__, __FILE__,
        "Run task statement outside flow declaration must specify model name");
    goto exit;
  }
  //check that the modeltype exists and the task within the model type exists
  x = (E_enventry) S_look(venv, a->u.runtask.modelName);
  if (!x || x->kind != E_modelTypeEntry) {
    rc = ERR_UNDEFINED_VARIABLE;
    EM_error(a->pos, rc, __LINE__, __FILE__, S_name(a->u.runtask.modelName));
    goto exit;
  }
  modelitems = x->u.modeltype.modelitems;
  length = modelitems->length;
  found = false;
  foundFlow = false;

  for (int i = 0; i < length && !found; i++) {
    mi = (A_modelitem) getNthElementList(modelitems, i);
    if (mi->name == a->u.runtask.task) {
      found = true;
    }
  }

  if (x->u.modeltype.flows)
    length = x->u.modeltype.flows->length;
  else
    length = 0;
  for (int i = 0; i < length && !found && !foundFlow; i++) {
    flow = (A_flow) getNthElementList(x->u.modeltype.flows, i);
    if (flow->name == a->u.runtask.task) {
      foundFlow = true;
    }
  }
  if (!found && !foundFlow) {
    rc = ERR_UNDEFINED_VARIABLE;
    EM_error(a->pos, rc, __LINE__, __FILE__, S_name(a->u.runtask.task));
    goto exit;
  }

  //make sure that the source exists and is of type table or stream
  s = (E_enventry) S_look(venv, a->u.runtask.source);
  if (!s || (s->kind != E_varEntry && s->kind != E_streamEntry)) {
    rc = ERR_UNDEFINED_VARIABLE;
    EM_error(a->pos, rc, __LINE__, __FILE__, S_name(a->u.runtask.source));
    goto exit;
  }

  //set modelId
  setModelName(S_name(a->u.runtask.modelName));

  if (found) {
    //here we have found the task and mi is pointing to it
    //here make sure that the window type of the model item and the window type
    // of the task are compatible
    if ((mi->window == 1 && a->u.runtask.win == (A_win) 0) || (mi->window == 0
        && a->u.runtask.win != (A_win) 0)) {
      rc = ERR_WINDOW_USE_INVALID;
      EM_error(a->pos, rc, __LINE__, __FILE__,
          "Windowed aggregate being called without a window or the other way around.");
      goto exit;
    }

    //write the query to use the aggregate with all attributes of the source
    if (s->kind == E_streamEntry) {
      sourceTy = s->u.stream.ty;
    } else {
      sourceTy = s->u.var.ty;
    }

    //TODO: put the parameters in the partable, update if none then insert
    //we should get the first paramtable from inMemTables,
    //and try to do an update if that doesn't work, do an insert
    params = a->u.runtask.params;
    if (params != 0) {
      sprintf(paramInit, "\nDBT pkey, pdata;"
        "\nchar pkeydata[MAX_STR_LEN], pdatadata[MAX_STR_LEN];");
      for (int i = 0; i < params->length; i++) {
        T_expty s_exp;
        T_expty s_dec;
        A_exp pe = (A_exp) getNthElementList(params, i);
        A_var ve = pe->u.assign.var;
        A_exp pae = pe->u.assign.exp;
        S_symbol paramTblName =
            (S_symbol) getNthElementList(mi->paramtables, 0);
        char uName[256];
        char upTblName[256];

        getUserNameie(uName);
        sprintf(upTblName, "%s%s", uName, S_name(paramTblName));
        E_enventry paramTbl = (E_enventry) S_look(venv, S_Symbol(upTblName));

        rc = transExp(venv, tenv, pae, sql, s_dec, s_exp, aggregates);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transRuntaskExp",
              "transExp");
          goto exit;
        }

        rc = assignParameter(paramInit, paramTbl, ve, s_exp, paramTblName);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transRuntaskExp",
              "assignParameter");
          goto exit;
        }
      }
    }

    //go through the ty and make the uda call exp,
    args = A_List(a->pos, A_EXP);
    length = sourceTy->u.record->length;
    for (int i = 0; i < length; i++) {
      Ty_field f = (Ty_field) getNthElementList(sourceTy->u.record, i);
      if (f->name != S_Symbol("oid"))
        appendAList(args, A_VarExp(a->pos, A_SimpleVar(a->pos, f->name)));
    }
    callExp = A_CallExp(a->pos, mi->uda, args, (S_symbol) 0, paramInit);
    if (a->u.runtask.win != 0) {
      A_SetCallExpWindow(callExp, a->u.runtask.win);
    }
    //we created the call above, just need to create a simple select here
    select_list = A_List(a->pos, A_SELECT_ITEM);
    appendAList(select_list, A_SelectItem(a->pos, callExp, (S_symbol) 0));
    from_list = A_List(a->pos, A_QUN);
    appendAList(from_list, A_NameQun(a->pos, a->u.runtask.source,
        a->u.runtask.source));
    newExp = A_Select(a->pos, 0, select_list, from_list, (A_exp) 0, (A_list) 0,
        (A_exp) 0, (A_list) 0);

  } else if (foundFlow) {
    T_expty adec = (T_expty) 0;
    T_expty aexe = (T_expty) 0;
    char pre[256];
    char oldQueryName[256];
    bufferMngr *bm = bufferMngr::getInstance();

    sprintf(oldQueryName, "%s", queryName);
    if (strcmp(getUserName(), "__user__") == 0) {
      sprintf(pre, getModelName());
    } else {
      sprintf(pre, "%s__%s", getUserName(), getJustName(getModelName()));
    }
    //printf("here pre %s\n", pre);
    A_list statements = flow->statements;
    length = statements->length;
    if ((cstmt == NULL || strcmp("temp", cstmt->name) == 0) && isESL()) {
      cstmt = new cStmt(oldQueryName);
      cstmt->set_in_buf(bm->lookup(S_name(a->u.runtask.source)));
      if (target != NULL)
        cstmt->set_out_buf(bm->lookup(target));
    }
    /* may be add instream to env, same as the source */
    int seenFinalStmt = 0;
    int isFinalStmt = 0;
    for (int i = 0; i < length; i++) {
      isFinalStmt = 0;
      sprintf(queryName, "%s_%d", oldQueryName, i);
      A_exp statement = (A_exp) getNthElementList(statements, i);
      rc = renameItemsInFlowStatement(a, statement, target, pre, newExp,
          isFinalStmt);
      //displayExp(statement);fflush(stderr);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
            "renameItemsInFlowStatement", "rewriteRuntaskExp");
        goto exit;
      }
      if (statement->kind == A_runtaskExp) {
        //rewrite this run task first
        A_exp nexp;
        rc = rewriteRuntaskExp(venv, tenv, statement, sql, dec, nexp,
            aggregates, NULL, cstmt);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteRuntaskExp",
              "rewriteRuntaskExp");
          goto exit;
        }
        A_list jtl = A_List(a->pos, A_QUN);

        appendAList(jtl, A_NameQun(a->pos, S_Symbol("stdout"), S_Symbol(
            "stdout")));
        appendAList(jtl, (void*) A_QueryQun(a->pos, 0, nexp));

        A_exp newOpr = A_SqlOprExp(a->pos, A_SQL_INSERT, 0, (A_list) 0, jtl,
            (A_list) 0);
        adec = (T_expty) 0;
        aexe = (T_expty) 0;
        rc = transExp(venv, tenv, newOpr, sql, adec, aexe, aggregates, NULL,
            cstmt);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteRuntaskExp",
              "transExp");
          goto exit;
        }
        sql->global = expTy_Seq(sql->global, adec);
        sql->global = expTy_Seq(sql->global, aexe);
      } else if (statement->kind == A_createstreamExp) {
        //printf("here %s %s\n", S_name(statement->u.createstream.name));
        adec = (T_expty) 0;
        aexe = (T_expty) 0;
        rc = transCreatestreamExp(venv, tenv, statement, sql, adec, aexe,
            aggregates, cstmt);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteRuntaskExp",
              "transCreateStreamExp");
          goto exit;
        }
        sql->global = expTy_Seq(sql->global, adec);
        sql->global = expTy_Seq(sql->global, aexe);
      } else if (statement->kind == A_sqloprExp
          && statement->u.sqlopr->jtl_list->length == 2) {
        A_qun q1 = (A_qun) getNthElementList(statement->u.sqlopr->jtl_list, 1);
        if (q1->kind == QUN_QUERY) {
          A_exp csquery = q1->u.query;
          if (isFinalStmt == 1 && seenFinalStmt == 0) {
            seenFinalStmt = 1;
            if (csquery->kind == A_runtaskExp) {
              A_exp nexp;
              rc = rewriteRuntaskExp(venv, tenv, csquery, sql, dec, nexp,
                  aggregates, NULL, cstmt);
              if (rc) {
                EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
                    "rewriteRuntaskExp", "rewriteRuntaskExp");
                goto exit;
              }
              newExp = nexp;
            } else {
              newExp = csquery;
            }
          }
        } else if (isFinalStmt == 1 && seenFinalStmt == 1) {
          rc = ERR_INVALID_STMT_IN_FLOW;
          EM_error(a->pos, rc, __LINE__, __FILE__);
          goto exit;
        }
        if (isFinalStmt == 0) {
          if (statement->u.sqlopr->kind = A_SQL_INSERT) {
            A_qun q0 = (A_qun) getNthElementList(statement->u.sqlopr->jtl_list,
                0);
            A_qun q1 = (A_qun) getNthElementList(statement->u.sqlopr->jtl_list,
                1);
            if (q1->kind == QUN_QUERY) {
              A_exp csquery = q1->u.query;
              if (csquery->kind == A_runtaskExp) {
                A_exp nexp;
                rc = rewriteRuntaskExp(venv, tenv, csquery, sql, dec, nexp,
                    aggregates, NULL, cstmt);
                if (rc) {
                  EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
                      "rewriteRuntaskExp", "rewriteRuntaskExp");
                  goto exit;
                }

                //source and target
                A_sqlopr nsqlopr = statement->u.sqlopr;
                A_list jtList = A_List(a->pos, A_QUN);
                appendAList(jtList, q0);
                appendAList(jtList, A_QueryQun(a->pos, 0, nexp));
                nsqlopr->jtl_list = jtList;

                adec = (T_expty) 0;
                aexe = (T_expty) 0;
                rc = transExp(venv, tenv, statement, sql, adec, aexe,
                    aggregates, NULL, cstmt);
                if (rc) {
                  EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
                      "rewriteRuntaskExp", "transExp");
                  goto exit;
                }
                sql->global = expTy_Seq(sql->global, adec);
                sql->global = expTy_Seq(sql->global, aexe);
              } else {
                adec = (T_expty) 0;
                aexe = (T_expty) 0;
                rc = transExp(venv, tenv, statement, sql, adec, aexe,
                    aggregates, NULL, cstmt);
                if (rc) {
                  EM_error(0, ERR_HISTORY, __LINE__, __FILE__,
                      "rewriteRuntaskExp", "transExp");
                  goto exit;
                }
                sql->global = expTy_Seq(sql->global, adec);
                sql->global = expTy_Seq(sql->global, aexe);
              }
            }
          } else if (statement->u.sqlopr->kind = A_SQL_UNION) {
            adec = (T_expty) 0;
            aexe = (T_expty) 0;
            rc = transExp(venv, tenv, statement, sql, adec, aexe, aggregates,
                NULL, cstmt);
            if (rc) {
              EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "rewriteRuntaskExp",
                  "transExp");
              goto exit;
            }
            sql->global = expTy_Seq(sql->global, adec);
            sql->global = expTy_Seq(sql->global, aexe);
          }
        }
      } else {
        rc = ERR_INVALID_STMT_IN_FLOW;
        EM_error(a->pos, rc, __LINE__, __FILE__);
        goto exit;
      }
    }
    sprintf(queryName, "%s", oldQueryName);
  }
  if (newExp == NULL) {
    rc = ERR_INVALID_STMT_IN_FLOW;
    EM_error(a->pos, rc, __LINE__, __FILE__);
    goto exit;
  }
  exit: DBUG_RETURN(rc);
}

err_t getKeyName(S_symbol alias, S_symbol key, A_list hxp_list, char* keyName) {
  SMLog::SMLOG(10, "Entering getKeyName");
  err_t rc = ERR_NONE;
  int length = 0;

  DBUG_ENTER("getKeyName");

  length = hxp_list->length;

  for (int i = 0; i < length; i++) {
    A_selectitem si = (A_selectitem) getNthElementList(hxp_list, i);
    switch (si->kind) {
    case SIMPLE_ITEM: {
      A_exp e = si->u.s.exp;
      if (e->kind == A_varExp) {
        A_var var = e->u.var;
        switch (var->kind) {
        case A_simpleVar:
          if (var->u.simple == key) {
            if (si->u.s.alias != NULL)
              sprintf(keyName, "%s", S_name(si->u.s.alias));
            else
              sprintf(keyName, S_name(key));
            goto exit;
          }
          break;
        case A_fieldVar:
          if (var->u.field.sym == alias && var->u.field.var->u.simple == key) {
            if (si->u.s.alias != NULL)
              sprintf(keyName, "%s", S_name(si->u.s.alias));
            else
              sprintf(keyName, S_name(key));
            goto exit;
          }
          break;
        default:
          break;
        }
      }
    }
      break;
    default:
      /*rc = ERR_CREATE_STREAM;
       EM_error(0, rc, __LINE__, __FILE__, "Create stream only allows simple items in select list.");
       goto exit;
       */
      break;
    }
  }

  exit: DBUG_RETURN(rc);
}

err_t verifyKeyExists(S_symbol key, Ty_ty ty) {
  SMLog::SMLOG(10, "Entering verifyKeyExists");
  err_t rc = ERR_NONE;
  int length = 0;

  DBUG_ENTER("verifyKeyExists");
  length = ty->u.record->length;
  for (int i = 0; i < length; i++) {
    Ty_field f = (Ty_field) getNthElementList(ty->u.record, i);
    if (strcmp(S_name(f->name), S_name(key)) == 0 && f->ty->kind
        == Ty_timestamp) {
      goto exit;
    }
  }

  rc = ERR_CREATE_STREAM;
  EM_error(0, rc, __LINE__, __FILE__,
      "Invalid Order by value used in create stream.");
  exit: DBUG_RETURN(rc);
}

err_t renameTypeNames(S_table venv, A_exp query, T_expty& e) {
  SMLog::SMLOG(10, "Entering renameTypeNames");
  err_t rc = ERR_NONE;
  int j = 0;
  for (int i = 0; i < query->u.select.hxp_list->length; i++) {
    A_selectitem arg = (A_selectitem) getNthElementList(
        query->u.select.hxp_list, i);
    if (arg->kind == SIMPLE_ITEM) {
      Ty_field fi = (Ty_field) getNthElementList(e->ty->u.record, j);
      if (arg->u.s.alias != (S_symbol) 0) {
        fi->name = S_Symbol(strdup(S_name(arg->u.s.alias)));
      } else if (arg->u.s.exp->kind == A_varExp) {
        A_var var = arg->u.s.exp->u.var;
        switch (var->kind) {
        case A_simpleVar: {
          fi->name = S_Symbol(strdup(S_name(var->u.simple)));
        }
          break;
        case A_fieldVar: {
          fi->name = S_Symbol(strdup(S_name(var->u.field.sym)));
        }
          break;
        default:
          break;
        }
      } else if (arg->u.s.exp->kind == A_callExp) {
        E_enventry x = (E_enventry) S_look(venv, arg->u.s.exp->u.call.func);
        if (!x || x->kind != E_aggrEntry) {
          rc = ERR_UNDEFINED_FUNCTION;
          EM_error(query->pos, rc, __LINE__, __FILE__, S_name(
              arg->u.s.exp->u.call.func));
          return rc;
        }
        Ty_ty result = x->u.fun.result;
        if (result->kind == Ty_record) {
          A_list rel = result->u.record;
          int length = rel->length;
          for (int n = 0; n < length; n++) {
            Ty_field t = (Ty_field) getNthElementList(rel, n);
            fi->name = S_Symbol(strdup(S_name(t->name)));
            fi = (Ty_field) getNthElementList(e->ty->u.record, j + n + 1);
          }
          j = j + length - 1;
        }
      }
      j++;
    } else if (arg->kind == COMPLEX_ITEM) {
      //loop through alias list, keep incrementing j, no renaming
      for (int k = 0; k < arg->u.c.aliaslist->length; k++) {
        j++;
      }
    } else if (arg->kind == STAR_ITEM) {
      rc = ERR_CREATE_STREAM;
      EM_error(query->pos, rc, __LINE__, __FILE__,
          "Cannot use Star in Createstream declaration.");
      return rc;
    }
  }
  return rc;
}

err_t transCreatestreamExp(S_table venv, /* variable env */
S_table tenv, /* type env */
A_exp a, /* abstract syntacs */
Sql_sem sql, /* sql semantics */
T_expty &dec, /* OUTPUT: declartion part */
T_expty &exe, /* OUTPUT: executable part */
vector<void*> aggregates, cStmt* cstmt) {
  SMLog::SMLOG(10, "Entering transCreatestreamExp");
  err_t rc = ERR_NONE;
  E_enventry x;
  E_enventry y;
  A_exp query;
  A_exp newQuery;
  A_exp na;
  A_exp select;
  A_exp newExp;
  A_qun q;
  A_list jtList;
  A_list jtl;
  T_expty d = (T_expty) 0;
  T_expty e = (T_expty) 0;
  T_expty adec = (T_expty) 0;
  T_expty aexe = (T_expty) 0;
  Sql_sem asql = SqlSem();
  bufferMngr *bm = bufferMngr::getInstance();
  buffer * ioBuf;
  vector<string> srcs;
  char declare[4096];
  timekey_t tk;
  char selectedKey[200];
  selectedKey[0] = 0;

  DBUG_ENTER("transCreatestreamExp");

  x = (E_enventry) S_look(venv, a->u.createstream.name);
  if (x) {
    rc = ERR_REDECLARATION;
    EM_error(a->pos, rc, __LINE__, __FILE__, S_name(a->u.createstream.name));
    goto exit;
  }

  query = a->u.createstream.query;
  if (query->kind == A_runtaskExp) {
    A_exp newExp;
    rc = rewriteRuntaskExp(venv, tenv, query, sql, dec, newExp, aggregates,
        NULL, cstmt);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreatestreamExp",
          "rewriteRuntaskExp");
      goto exit;
    }
    a->u.createstream.query = newExp;
    query = a->u.createstream.query;
  } else if (query->kind == A_sqloprExp) {
    if (query->u.sqlopr->kind == A_SQL_UNION && query->u.sqlopr->jtl_list
        && query->u.sqlopr->jtl_list->length > 0) {
      A_qun qun = (A_qun) getNthElementList(query->u.sqlopr->jtl_list, 0);
      if (qun->kind == QUN_QUERY) {
        query = qun->u.query;
      } else {
        rc = ERR_CREATE_STREAM;
        EM_error(
            a->pos,
            rc,
            __LINE__,
            __FILE__,
            "Create stream only allowed with union statements apart from slect and run statements.");
        goto exit;
      }
    } else {
      rc = ERR_CREATE_STREAM;
      EM_error(
          a->pos,
          rc,
          __LINE__,
          __FILE__,
          "Create stream only allowed with union statements apart from slect and run statements.");
      goto exit;
    }
  }

  if (query->kind != A_selectExp) {
    rc = ERR_CREATE_STREAM;
    EM_error(a->pos, rc, __LINE__, __FILE__,
        "Create stream only allowed with select statements.");
    goto exit;
  }

  if (query->u.select.join_table_list->length < 1) {
    rc = ERR_CREATE_STREAM;
    EM_error(a->pos, rc, __LINE__, __FILE__,
        "Create stream requires a stream in the from list of select.");
    goto exit;
  }

  q = (A_qun) getNthElementList(query->u.select.join_table_list, 0);
  if (q->kind != QUN_NAME) {
    rc = ERR_CREATE_STREAM;
    EM_error(a->pos, rc, __LINE__, __FILE__,
        "Create stream requires the from list to begin with a stream.");
    goto exit;
  }

  y = (E_enventry) S_look(venv, q->u.table.name);
  if (!y || y->kind != E_streamEntry) {
    rc = ERR_UNDEFINED_VARIABLE;
    EM_error(a->pos, rc, __LINE__, __FILE__, S_name(q->u.table.name));
    goto exit;
  }
  /*newQuery = A_SqlOprExp(query->pos,
   A_SQL_SEL,
   query->u.select.distinct,
   query->u.select.hxp_list,
   query->u.select.join_table_list,
   query->u.select.wr_prd_list
   );*/

  newQuery = copyExp(query);

  rc = rewriteQuery(venv, tenv, newQuery, na, aggregates);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreatestreamExp",
        "rewriteQuery");
    goto exit;
  }

  //call transSelOpr here to find out the ty
  rc = transSelOpr(venv, tenv, na->u.sqlopr, asql, d, e, "temp", aggregates,
      srcs, cstmt);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreatestreamExp",
        "transSelOpr");
    goto exit;
  }

  rc = renameTypeNames(venv, query, e);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreatestreamExp",
        "renameTypeNames");
    goto exit;
  }

  if (a->u.createstream.timekey == (S_symbol) NULL) {
    if (y->u.stream.tk == tk_none)
      tk = tk_none;
    else if (y->u.stream.tk == tk_internal || y->u.stream.tk == tk_external) {
      S_symbol key = y->u.stream.timekey;
      rc = getKeyName(q->alias, key, query->u.select.hxp_list, selectedKey);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreatestreamExp",
            "getKeyName");
        goto exit;
      }
      if (selectedKey[0] == 0) {
        tk = tk_none;
      } else {
        tk = y->u.stream.tk;
      }
    }
  } else {
    if (strcmp(S_name(a->u.createstream.timekey), ITIME_COLUMN) == 0) {
      tk = tk_internal;
      sprintf(selectedKey, "%s", ITIME_COLUMN);

      //add a timestamp to e->ty;
      Ty_field keyField = Ty_Field(S_Symbol(ITIME_COLUMN), Ty_Timestamp(),
          sizeof(struct timeval));
      //keyField->isTimekey = 1;
      appendElementList(e->ty->u.record, (nt_obj_t*) keyField);

      //also add callExp(timeofday) to the query->hxp_list
      A_list list = A_List(query->pos, A_EXP);
      A_selectitem si = A_SelectItem(query->pos, A_CallExp(query->pos,
          S_Symbol("timeofday"), list, (S_symbol) 0), S_Symbol(ITIME_COLUMN));
      appendAList(query->u.select.hxp_list, si);
    } else {
      tk = tk_external;
      sprintf(selectedKey, "%s", S_name(a->u.createstream.timekey));
      rc = verifyKeyExists(a->u.createstream.timekey, e->ty);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreatestreamExp",
            "verifyKeyExists");
        goto exit;
      }
    }
  }

  for (int j = 0; j < e->ty->u.record->length; j++) {
    Ty_field fi = (Ty_field) getNthElementList(e->ty->u.record, j);
    //printf("field name %s\n", S_name(fi->name));
    if (strcmp(S_name(fi->name), selectedKey) == 0) {
      fi->isTimekey = 1;
      break;
    }
  }

  //create a stream Entry and enter it in venv
  if (tk != tk_none) {
    x = E_StreamEntry(a->u.createstream.name, e->ty, 0, 0, 0, tk, S_Symbol(
        selectedKey));
  } else
    x = E_StreamEntry(a->u.createstream.name, e->ty, 0, 0, 0, tk);
  S_enter(venv, a->u.createstream.name, x);

  bm->create(S_name(a->u.createstream.name), SHARED);

  //send message to ioBuff suggesting that we have a new stream
  char message[4096];
  prepareStreamMessage(a->u.createstream.name, x->u.stream.ty, ((selectedKey[0]
      == 0) ? NULL : S_Symbol(selectedKey)), message);
  ioBuf = bm->lookup("_ioBuffer");
  ioBuf->put(ADD_STREAM_DEC, S_name(a->u.createstream.name), message);

  //create an insert into the stream (newQuery) and translate it
  if (a->u.createstream.query->kind == A_sqloprExp) {
    query = a->u.createstream.query;
  }
  jtList = A_List(a->pos, A_QUN);
  appendAList(jtList, A_NameQun(a->pos, a->u.createstream.name,
      a->u.createstream.name));
  appendAList(jtList, A_QueryQun(query->pos, 0, query));

  newExp = A_SqlOprExp(a->pos, A_SQL_INSERT, query->u.select.distinct,
      query->u.select.hxp_list, jtList, query->u.select.wr_prd_list);
  //displayExp(newExp);fflush(stderr);
  rc = transExp(venv, tenv, newExp, sql, adec, aexe, aggregates, NULL, cstmt);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreatestreamExp",
        "transExp");
    goto exit;
  }

  dec = expTy_Seq(dec, adec);
  dec = expTy_Seq(dec, aexe);

  exit: DBUG_RETURN(rc);
}

err_t transCreateviewExp(S_table venv, /* variable env */
S_table tenv, /* type env */
A_exp a, /* abstract syntacs */
Sql_sem sql, /* sql semantics */
T_expty &dec, /* OUTPUT: declartion part */
T_expty &exe, /* OUTPUT: executable part */
vector<void*> aggregates) {
  SMLog::SMLOG(10, "Entering transCreateviewExp");
  err_t rc = ERR_NONE;
  E_enventry x;
  E_enventry y;
  A_exp query;
  A_exp newQuery;
  A_exp na;
  A_exp select;
  A_exp newExp;
  A_qun q;
  A_qun newQun;
  A_list newQunList;
  A_list jtList;
  T_expty d = (T_expty) 0;
  T_expty e = (T_expty) 0;
  T_expty adec = (T_expty) 0;
  T_expty aexe = (T_expty) 0;
  Sql_sem asql = SqlSem();
  bufferMngr *bm = bufferMngr::getInstance();
  buffer * ioBuf;
  vector<string> srcs;
  char declare[4096];
  cStmt* cstmt = new cStmt("temp");

  DBUG_ENTER("transCreateviewExp");

  x = (E_enventry) S_look(venv, a->u.createview.name);
  if (x) {
    rc = ERR_REDECLARATION;
    EM_error(a->pos, rc, __LINE__, __FILE__, S_name(a->u.createview.name));
    goto exit;
  }

  query = a->u.createview.query;
  if (query->kind != A_selectExp) {
    rc = ERR_CREATE_VIEW;
    EM_error(a->pos, rc, __LINE__, __FILE__,
        "table transformer is only allowed with select statements.");
    goto exit;
  }

  if (query->u.select.join_table_list->length != 1) {
    rc = ERR_CREATE_VIEW;
    EM_error(a->pos, rc, __LINE__, __FILE__,
        "table transformer is only allowed with simple select stmts without any joins.");
    goto exit;
  }

  q = (A_qun) getNthElementList(query->u.select.join_table_list, 0);
  if (q->kind != QUN_WINDOW) {
    rc = ERR_CREATE_VIEW;
    EM_error(
        a->pos,
        rc,
        __LINE__,
        __FILE__,
        "table transformer is only allowed with simple select stmts which have window defined on a stream.");
    goto exit;
  }

  y = (E_enventry) S_look(venv, q->u.window.name);
  if (!y || y->kind != E_streamEntry) {
    rc = ERR_UNDEFINED_VARIABLE;
    EM_error(a->pos, rc, __LINE__, __FILE__, S_name(q->u.window.name));
    goto exit;
  }

  if (q->u.window.name != q->u.window.respectTo) {
    rc = ERR_CREATE_VIEW;
    EM_error(
        a->pos,
        rc,
        __LINE__,
        __FILE__,
        "table transformer is only allowed with simple select stmts which have window defined on a stream respect to itself.");
    goto exit;
  }

  newQun = A_NameQun(a->pos, q->u.window.name, q->alias);
  newQunList = A_List(a->pos, A_QUN);

  appendAList(newQunList, (void*) newQun);
  newQuery = A_SqlOprExp(query->pos, A_SQL_SEL, query->u.select.distinct,
      query->u.select.hxp_list, newQunList, query->u.select.wr_prd_list);

  //call transSelOpr here to find out the ty
  rc = transSelOpr(venv, tenv, newQuery->u.sqlopr, asql, d, e, "temp",
      aggregates, srcs, cstmt);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreateviewExp",
        "transSelOpr");
    goto exit;
  }

  //create a var Entry and enter it in venv, the var should have isBuf = 1
  x = E_VarEntry(a->u.createview.name, e->ty, 0, TAB_MEMORY, (A_index) 0, 0, 0,
      1);
  /* check index type */
  /*rc = checkTabIndexCreateview(a, e->ty, x);
   if (rc) {
   EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreateviewExp", "checkTabIndexCreateview");
   goto exit;
   }
   x->u.var.firstKey = a->u.createview.keyPos[0];*/

  S_enter(venv, a->u.createview.name, x);

  //create a windowBuf with range of first qun in a->u.createview.query
  bm->create(S_name(a->u.createview.name), WINBUF, q->u.window.range->size,
      (q->u.window.range->type == TIME_RANGE) ? _ADL_WIN_TIME : _ADL_WIN_ROW);

  //send message to ioBuff suggesting that a buffer was added (for deletion)
  char message[4096];
  prepareWindowMessage(a->u.createview.name, x->u.var.ty,
      a->u.createview.keydecs, message);
  ioBuf = bm->lookup("_ioBuffer");
  ioBuf->put(ADD_TABLE_DEC, S_name(a->u.createview.name), message);

  //create an insert into the var (newQuery) and translate it
  jtList = A_List(a->pos, A_QUN);
  appendAList(jtList, A_NameQun(a->pos, a->u.createview.name,
      a->u.createview.name));
  appendAList(jtList, A_QueryQun(query->pos, 0, A_Select(query->pos,
      query->u.select.distinct, query->u.select.hxp_list, newQunList,
      query->u.select.wr_prd_list, query->u.select.group_by_list,
      query->u.select.hv_prd_list, query->u.select.order_by_list)));
  newExp = A_SqlOprExp(a->pos, A_SQL_INSERT, query->u.select.distinct,
      query->u.select.hxp_list, jtList, query->u.select.wr_prd_list);
  rc = transExp(venv, tenv, newExp, sql, adec, aexe, aggregates);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCreateviewExp",
        "transExp");
    goto exit;
  }

  sprintf(declare, "\nwindowBuf *%s=NULL;", S_name(a->u.createview.name));
  sql->global = expTy_Seq(sql->global, declare);

  dec = expTy_Seq(dec, adec);
  dec = expTy_Seq(dec, aexe);

  exit: DBUG_RETURN(rc);
}

err_t transExp(S_table venv, /* variable env */
S_table tenv, /* type env */
A_exp a, /* abstract syntacs */
Sql_sem sql, /* sql semantics */
T_expty &dec, /* OUTPUT: declartion part */
T_expty &exe, /* OUTPUT: executable part */
vector<void*> aggregates, char* target_handle, cStmt* cstmt) {
  SMLog::SMLOG(10, "Entering transExp target_handle: %s", target_handle);
  err_t rc = ERR_NONE;
  int i;
  char buf[MAX_STR_LEN];

  DBUG_ENTER("transExp");

  dec = (T_expty) 0;
  exe = (T_expty) 0;

  /*fprintf(stderr, "here %d\n", a->kind);
   displayExp(a);
   fprintf(stderr, "\nDone\n\n");
   */

  switch (a->kind) {
  case A_refExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_refExp");
    rc = transRef(venv, tenv, a->u.ref, sql, dec, exe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transRef");
      goto exit;
    }
    if (exe->ty->kind == Ty_record) {
      T_expty e = expTy(Tr_String("(int)"), exe->ty);
      exe = expTy_Seq(e, exe);
    }
  }
    break;
  case A_varExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_varExp");
    rc = transVar(venv, tenv, a->u.var, sql, (A_qun*) 0, (int*) 0, exe);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transVar");
      goto exit;
    }
  }
    break;
  case A_callExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_callExp");
    E_enventry x;
    A_list a_arglist;
    int n_args;
    T_expty argbuf = (T_expty) 0;

    if (a->u.call.args) {
      a_arglist = a->u.call.args;
      n_args = a_arglist->length;
    } else {
      n_args = 0;
    }

    // check if function is defined
    SMLog::SMLOG(10, "\tTrying to call function %s", S_name(a->u.call.func));
    x = (E_enventry) S_look(venv, a->u.call.func);
    if (!x) {
      rc = ERR_UNDEFINED_FUNCTION;
      EM_error(a->pos, rc, __LINE__, __FILE__, S_name(a->u.call.func));
      goto exit;
    }

    if ((x->kind == E_funEntry && !x->u.fun.varied_args) || (x->kind
        == E_extEntry && !x->u.ext.varied_args)) {
      int n_formals = (x->u.fun.formals) ? x->u.fun.formals->length : 0;
      if (n_formals != n_args) {
        rc = ERR_WRONG_NUMBER_OF_ARGS;
        EM_error(a->pos, rc, __LINE__, __FILE__, S_name(a->u.call.func));
        goto exit;
      }
    }

    switch (x->kind) {
    case E_aggrEntry: {
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__, "aggrEntry found in transExp");
      goto exit;
    }
      break;
    case E_funEntry: {
      rc = transBuiltInFunc(venv, tenv, a, sql, dec, exe, aggregates);
      if (rc) {
        if (rc == ERR_NON_BUILTIN_FUNC) {
          if (n_args > 0) {
            rc = compileArgs(venv, tenv, sql, x->u.fun.formals, a_arglist,
                argbuf, aggregates);
            if (rc) {
              EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
                  "compileArgs");
              goto exit;
            }
          }
          /* this is a scalar UDF in SQL, to do! */
          rc = ERR_TO_BE_IMPLEMENTED;
          EM_error(a->pos, rc, __LINE__, __FILE__, "scalar UDF in SQL");
          goto exit;

          sprintf(buf, "%s(", S_name(a->u.call.func));

          exe = expTy_Seq(exe, buf);
          exe = expTy_Seq(exe, argbuf);
          exe = expTy_Seq(exe, ")");

          exe->ty = x->u.ext.result;
          if (exe->ty->kind == Ty_string)
            exe->size = x->u.ext.size;
        } else {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
              "transBuiltInFunc");
          goto exit;
        }
      }
    }
      break;
    case E_extEntry: {
      if (x->u.ext.result->kind == Ty_record) {
        rc = ERR_INVALID_TABLE_FUNCTION_USE;
        EM_error(0, rc, __LINE__, __FILE__, S_name(x->key));
        goto exit;
      }
      if (n_args > 0) {
        rc = compileArgs(venv, tenv, sql, x->u.ext.formals, a_arglist, argbuf,
            aggregates);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
              "compileArgs");
          goto exit;
        }
      }

      char tmpbuf[128];
      char argbufTmp[2048];
      argbufTmp[0] = '\0';
      for (i = 0; i < x->u.ext.formals->length; i++) {
        Ty_field f = (Ty_field) getNthElementList(x->u.ext.formals, i);
        rc = Ty2C(f->ty, tmpbuf);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "Ty2C");
          goto exit;
        }
        if (i > 0) {
          strcat(argbufTmp, ", ");
        }
        strcat(argbufTmp, tmpbuf);
      }

      if (isESL() || isAdHoc() || isESLAggr()) {
        char code[1024];
        char retType[80];
        switch (x->u.ext.result->kind) {
        case Ty_int:
          sprintf(retType, "int");
          break;
        case Ty_real:
          sprintf(retType, "double");
          break;
        case Ty_string:
          sprintf(retType, "char*");
          break;
        case Ty_timestamp:
          sprintf(retType, "struct timeval");
          break;
        default:
          rc = ERR_DATATYPE;
          EM_error(0, rc, __LINE__, __FILE__, "type not supported in UPDATE");
          goto exit;
        }
        sprintf(code, "\nif(_adl_func_%d == NULL) {"
          "\n_adl_func_%d = (%s(*)(%s))_adl_dlm(\"%s\", \"%s\"); //h3"
          "\n}", UID(x), UID(x), retType, argbufTmp,
            S_name(x->u.ext.externlib), S_name(x->u.ext.actual));
        dec = expTy_Seq(dec, code);
      }

      sprintf(buf, "(*_adl_func_%d)(", UID(x));

      exe = expTy_Seq(exe, buf);
      exe = expTy_Seq(exe, argbuf);
      exe = expTy_Seq(exe, ")");

      exe->ty = x->u.ext.result;
      if (exe->ty->kind == Ty_string)
        exe->size = x->u.ext.size;
    }
      break;
    }
  }
    break;
  case A_recordExp:
    break;
  case A_seqExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_seqExp");
    rc = transSeqExp(venv, tenv, a, sql, dec, exe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transSeqExp");
      goto exit;
    }
  }
    break;
  case A_assignExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_assignExp");
    /* assign occurs in 2 places:
     1. UPDATE SET ...
     2. SELECT hxp ... ---for in memory table only

     Here is for case 2.
     */
    T_expty dec, s_exp, t_exp;

    if (a->u.assign.var->kind != A_refVar) {
      /* only A_refVar, which is rewritten by rewriteUpdate(), is allowed */
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__, "assignment in SELECT");
      goto exit;
    }
    rc = transRef(venv, tenv, a->u.assign.var->u.ref, sql, dec, t_exp);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transRef");
      goto exit;
    }
    rc = transExp(venv, tenv, a->u.assign.exp, sql, dec, s_exp, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transRef");
      goto exit;
    }
    if (s_exp->ty != t_exp->ty && !extendType(s_exp->ty, t_exp->ty)) {
      rc = ERR_DATATYPE;
      EM_error(a->pos, rc, __LINE__, __FILE__, "UPDATE assignment");
      goto exit;
    }
    sprintf(buf, "\n%s = %s;", t_exp->exp, s_exp->exp);
    exe = expTy_Seq(exe, buf);
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
  case A_letExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_letExp");
    T_expty curdec, curexe, curcln;
    T_expty clean = (T_expty) 0;
    T_expty adec = (T_expty) 0;

    if (!A_ListEmpty(a->u.let.decs)) {
      A_list dec_list = a->u.let.decs;
      S_beginScope(venv);
      S_beginScope(tenv);

      rc = transSeqDec(venv, tenv, dec_list, dec, exe, clean, 0, adec,
          aggregates);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transSeqDec");
        goto exit;
      }
      /*	for (i=0; i<dec_list->length; i++)
       {
       A_dec d = (A_dec)getNthElementList(dec_list, i);
       rc = transDec(venv, tenv, d, curdec, curexe, curcln);
       if (rc) {
       EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transDec");
       goto exit;
       }
       dec = expTy_Seq(dec, curdec);
       exe = expTy_Seq(exe, curexe);
       clean = expTy_Seq(clean, curcln);
       }
       */
    }

    if (a->u.let.body) {
      rc = transExp(venv, tenv, a->u.let.body, sql, curdec, curexe, aggregates);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transExp");
        goto exit;
      }
      dec = expTy_Seq(dec, curdec);
      exe = expTy_Seq(exe, curexe);
    }

    if (!A_ListEmpty(a->u.let.decs)) {
      exe = expTy_Seq(exe, clean);
      S_endScope(tenv);
      S_endScope(venv);
    }
  }
    break;
  case A_arrayExp:
    break;
  case A_sqloprExp: {
    SMLog::SMLOG(12, "\tabstract syntax kind is A_sqloprExp");
    T_expty d = (T_expty) 0;
    T_expty e = (T_expty) 0;
    vector<string> srcs;
    bufferMngr *bm = bufferMngr::getInstance();
    //cStmt* cstmt = NULL;

    if ((isESL() && sql->in_func != 1) || sql->top_sqlopr == (A_sqlopr) 0) {
      A_exp na;

      SMLog::SMLOG(12, "\tabstract syntax kind is A_sqloprExp1");
      //i'd say here we check if runtask, then we should do something diff
      //simple call the rewriteRuntask method, then put the new exp
      // in a's list, jtl list has target and the source 0 and 1, i believe
      //displayExp(a);
      if (a->u.sqlopr->kind == A_SQL_INSERT) {
        A_list jtl = a->u.sqlopr->jtl_list;
        if (jtl->length == 2) {
          A_qun q0 = (A_qun) getNthElementList(jtl, 0);
          A_qun q1 = (A_qun) getNthElementList(jtl, 1);
          if (q0->kind == QUN_NAME && q1->kind == QUN_QUERY
              && q1->u.query->kind == A_runtaskExp) {
            A_exp newExp;
            if ((cstmt == NULL || strcmp("temp", cstmt->name) == 0) && isESL())
              cstmt = new cStmt(queryName);
            rc = rewriteRuntaskExp(venv, tenv, q1->u.query, sql, dec, newExp,
                aggregates, S_name(q0->u.table.name), cstmt);
            if (rc) {
              EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
                  "rewriteRuntaskExp");
              goto exit;
            }
            q1->u.query = newExp;
          }
        }
      }

      if (a->u.sqlopr->kind == A_SQL_INSERT && a->u.sqlopr->jtl_list->length
          == 2) {
        SMLog::SMLOG(12, "\tabstract syntax kind is A_sqloprExp2");
        A_sqlopr s = a->u.sqlopr;
        int aggrCnt = 0;
        A_exp aggrExp;
        int paneBasedOpt = 0;
        A_qun source = (A_qun) getNthElementList(s->jtl_list, 1);
        A_qun fqun = NULL;
        int otherExpCnt = 0;
        //A_qun origSrc = copyQun(source);

        if (source->kind == QUN_QUERY && source->u.query->kind == A_selectExp) {
          //displayQun(source);
          A_exp query = source->u.query;
          if (query->u.select.join_table_list != NULL)
            fqun
                = (A_qun) getNthElementList(query->u.select.join_table_list, 0);
          if (query->u.select.hxp_list != NULL) {
            for (int i = 0; i < query->u.select.hxp_list->length; i++) {
              A_selectitem hxp = (A_selectitem) getNthElementList(
                  query->u.select.hxp_list, i);
              if (hxp->u.s.exp && hxp->u.s.exp->kind == A_callExp) {
                aggrCnt++;
                aggrExp = hxp->u.s.exp;
              } else {
                otherExpCnt++;
              }
            }
            if (fqun && fqun->kind == QUN_NAME && aggrCnt == 1
                && aggrExp->u.call.win != NULL && aggrExp->u.call.win->slide
                != NULL && aggrExp->u.call.win->slide->size > 1
                && aggrExp->u.call.win->partition_list == (A_list) 0
                && otherExpCnt == 0) {
              A_win w = aggrExp->u.call.win;
              double wsize = w->range->size;
              if (wsize > w->slide->size && ceil(wsize / w->slide->size)
                  == floor(wsize / w->slide->size)) {
                //pane based optimization
                paneBasedOpt = 1;
              }
            }
          }
        }
        // TODO(nlaptev): Fix this, when paneBasedOpt is set to true, then we have an error when compiling with adlc. Need to figure out why the code generated, incorrectly generates the column variables.

        if (paneBasedOpt == 1 && 0) {

          //printf("here paneBasedOpt %d\n", paneBasedOpt);
          //create a tmp table/stream for intermediate results
          //first we figure out the schema of the qun, by compling the
          // select query
          T_expty di, ei;
          E_enventry x;
          char newObjNm[100];
          A_exp na;

          A_exp query = copyExp(source->u.query);
          if (query->u.select.hxp_list != NULL) {
            for (int i = 0; i < query->u.select.hxp_list->length; i++) {
              A_selectitem hxp = (A_selectitem) getNthElementList(
                  query->u.select.hxp_list, i);
              A_exp aexp;
              aexp = hxp->u.s.exp;
              if (aexp->kind == A_callExp && aexp->u.call.win != NULL
                  && aexp->u.call.win->slide != NULL
                  && aexp->u.call.win->slide->size > 1) {
                A_win w = aexp->u.call.win;
                w->range->size = w->slide->size;
                w->slide = NULL;
              }
            }
          }
          rc = rewriteQuery(venv, tenv, query, na, aggregates);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
                "rewriteQuery");
            goto exit;
          }

          //call transSelOpr here to find out the ty
          rc = transSelOpr(venv, tenv, na->u.sqlopr, sql, di, ei, "temp",
              aggregates, srcs, cstmt);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
                "transSelOpr");
            goto exit;
          }
          //now ei has the type
          rc = renameTypeNames(venv, query, ei);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
                "renameTypeNames");
            goto exit;
          }

          sprintf(newObjNm, "%stmpPane_%d", queryName, UID(query));

          if (isESL()) {
            x = E_StreamEntry(S_Symbol(newObjNm), ei->ty, 0, 0, 0, tk_none);
            bm->create(newObjNm, SHARED);
            buffer *b = bm->lookup("_ioBuffer");
            b->put(ADDED_BUFFER, newObjNm);

            //send message to ioBuff suggesting that we have a new stream
            //char message[4096];
            //prepareStreamMessage(S_Symbol(newObjNm), x->u.stream.ty,
            //  ((selectedKey[0] == 0)?NULL:S_Symbol(selectedKey)), message);
            //ioBuf = bm->lookup("_ioBuffer");
            //ioBuf->put(ADD_STREAM_DEC, S_name(a->u.createstream.name), message);
          } else {
            //output IMREL newObj to ATLaS code
            char buf[MAX_STR_LEN];
            sprintf(buf, "\nIM_REL *%s;", newObjNm);
            sql->global = expTy_Seq(sql->global, buf);

            sprintf(buf,
                "\nif ((rc=im_rel_create(&%s, NULL, IM_LINKEDLIST, 0)) != 0) {"
                  "\nadlabort(rc, \"im_rel_create()\");"
                  "\n}"
                  "\nif ((rc = %s->open(%s, \"_adl_db_%s\", 0)) != 0) {"
                  "\nadlabort(rc, \"open()\");"
                  "\n}"
                  "\nif (inMemTables->count(\"%s\") == 0) {"
                  "\ninMemTables->operator[](strdup(\"%s\")) = %s;"
                  "\n}", newObjNm, newObjNm, newObjNm, newObjNm, newObjNm,
                newObjNm, newObjNm);
            sql->preinit = expTy_Seq(sql->preinit, buf);

            x = E_VarEntry(S_Symbol(newObjNm), ei->ty, 0, TAB_MEMORY,
                (A_index) 0, 0, 0, 0);
          }
          S_enter(venv, S_Symbol(newObjNm), x);

          //we write another query, which will be the fisrt query that also
          // calls the window version with source same as orig query
          // target of the new table/stream, window size of orig slide size
          // copy the original query and make changes as needed
          A_qun sourceCopy = copyQun(source);
          //modify sourceCopy here
          query = sourceCopy->u.query;
          if (query->u.select.hxp_list != NULL) {
            for (int i = 0; i < query->u.select.hxp_list->length; i++) {
              A_selectitem hxp = (A_selectitem) getNthElementList(
                  query->u.select.hxp_list, i);
              A_exp aexp;
              aexp = hxp->u.s.exp;
              if (aexp->kind == A_callExp && aexp->u.call.win != NULL
                  && aexp->u.call.win->slide != NULL
                  && aexp->u.call.win->slide->size > 1) {
                A_win w = aexp->u.call.win;
                w->range->size = w->slide->size;
                //w->slide->size = size;
                //w->slide = NULL;
              }
            }
          }
          A_list jtList = A_List(s->pos, A_QUN);
          appendAList(jtList, A_NameQun(s->pos, S_Symbol(newObjNm), S_Symbol(
              newObjNm)));
          appendAList(jtList, A_QueryQun(query->pos, 0, query));

          A_exp newExp = A_SqlOprExp(s->pos, A_SQL_INSERT, 0, (A_list) 0,
              jtList, (A_list) 0);
          char oldQueryName[1024];

          if ((cstmt == NULL || strcmp("temp", cstmt->name) == 0) && isESL()) {
            sprintf(oldQueryName, "%s", queryName);
            sprintf(queryName, "%s_opt", oldQueryName);
            //creating a statement of type collect, we will add another
            // statement to this collection later
            cstmt = new cStmt(oldQueryName, new nStmt(queryName, bm->lookup(
                S_name(fqun->u.table.name)), bm->lookup(newObjNm)));
            cstmt->set_in_buf(bm->lookup(S_name(fqun->u.table.name)));
          } else if (isESL()) {
            cstmt->addSubStmt(new nStmt(queryName, bm->lookup(S_name(
                fqun->u.table.name)), bm->lookup(newObjNm)));
          }

          di = (T_expty) 0;
          ei = (T_expty) 0;
          //displayExp(newExp);

          rc = transExp(venv, tenv, newExp, sql, di, ei, aggregates);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlStatement",
                "transExp");
            goto exit;
          }
          //TODO: this should go in sql->global
          sql->global = expTy_Seq(sql->global, di);
          sql->global = expTy_Seq(sql->global, ei);
          if (isESL()) {
            sprintf(queryName, "%s", oldQueryName);
          }

          // change this original query
          // i am thinkin make this orig query the second query that calls the
          // windowed version with wsize/slidesize window size
          // the source for this query will be the new stream/table,
          // target same as orig query
          query = source->u.query;
          if (query->u.select.hxp_list != NULL) {
            for (int i = 0; i < query->u.select.hxp_list->length; i++) {
              A_selectitem hxp = (A_selectitem) getNthElementList(
                  query->u.select.hxp_list, i);
              A_exp aexp;
              aexp = hxp->u.s.exp;
              if (aexp->kind == A_callExp && aexp->u.call.win != NULL
                  && aexp->u.call.win->slide != NULL
                  && aexp->u.call.win->slide->size > 1) {
                A_win w = aexp->u.call.win;
                w->range->type = COUNT_RANGE;
                int size = w->range->size / w->slide->size;
                w->range->size = size;
                w->slide = NULL;
              }
            }
          }
          //assign query->jtl to be the list of the new table/stream
          //jtList = A_List(s->pos, A_QUN);
          overwriteElementList(query->u.select.join_table_list, 0,
              (nt_obj_t*) A_NameQun(s->pos, S_Symbol(newObjNm), S_Symbol(
                  newObjNm)));
          //query->u.select.join_table_list = jtList;
          //appendAList(jtList, A_QueryQun(s->pos, 0, query));

          //in ESL, at this point I just want to call transExp again and exit
          if (isESL()) {
            rc = transExp(venv, tenv, a, sql, dec, exe, aggregates,
                target_handle, cstmt);
            if (rc) {
              EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
                  "transExp");
              goto exit;
            }
            goto exit;
          }
        }
      }

      rc = rewriteQuery(venv, tenv, a, na, aggregates);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "rewriteQuery");
        goto exit;
      }

      if (sql->in_func) {
        /* inside  UDF/UDA, we do not break statements into functions */

        rc = transSqlStatement(venv, tenv, sql, na->u.sqlopr, dec, exe,
            aggregates, srcs, target_handle);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
              "transSqlStatement");
          goto exit;
        }
        //printf("here back\n");

      } else {
        /* each sql statement is implemented by separate funciton */
        rc = transSqlStatement(venv, tenv, sql, na->u.sqlopr, d, e, aggregates,
            srcs, target_handle, cstmt);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
              "transSqlStatement");
          goto exit;
        }
        char temp[2048];
        char heartbeat_buffer[500];
        if (isAdHoc()) {
          sprintf(
              temp,
              "extern \"C\" int adhoc_%d(bufferMngr *bm, int freeVars = 0,"
                "\n\tbuffer* backBuf = (buffer*)NULL, "
                "\n\thash_map<const char*, void*, std::hash<const char*>, eqstrTab>* inMemTables=NULL);"
                "\nextern \"C\" int adhoc_%d(bufferMngr *bm, int freeVars, "
                "\n\tbuffer* backBuf, "
                "\n\thash_map<const char*, void*, std::hash<const char*>, eqstrTab>* inMemTables)",
              getAdHocNum(), getAdHocNum());
          setAdHocNum(getAdHocNum() + 1);
        } else if (strlen(queryName) == 0) {
          sprintf(temp, "int _adl_statement_%d()", UID(a));
        } else {
        	if (isESL()) {
        	  sprintf(heartbeat_buffer, "\nint heart_beat_init = 0;"
              "\nbool FLAGS_enable_heartbeat = false;"
              "\nstruct args {"
              "\nbufferMngr *bm;"
              "\nint freeVars;"
              "\nbuffer* backBuf;"
              "\nhash_map<const char*, void*, std::hash<const char*>, eqstrTab>* inMemTables;"
              "\n};"
              "\nvoid *HeartBeat(void* arguments);");
        	} else {
        	  sprintf(heartbeat_buffer, "");	
        	}
          sprintf(
            temp, "%s"
            "\nextern \"C\" int %s(bufferMngr *bm, int freeVars = 0,"
            "buffer* backBuf = (buffer*)NULL, "
            "\n\thash_map<const char*, void*, std::hash<const char*>, eqstrTab>* inMemTables = NULL);"
            "\nextern \"C\" int %s(bufferMngr *bm, int freeVars, buffer* backBuf, "
            "\t\nhash_map<const char*, void*, std::hash<const char*>, eqstrTab>* inMemTables)",
              heartbeat_buffer, queryName, queryName); 
        }

        int window_size = 1;

        if (!isESL() || (isESL() && srcs.size() <= 1)) {
        	if (isESL()) {
        	sprintf(heartbeat_buffer, "\nif (FLAGS_enable_heartbeat && heart_beat_init == 0) {"
            "\nprintf(\"Initializing heartbeat\\n\");"
            "\nfflush(stdout);"
            "\npthread_t thread;"
            "\nheart_beat_init = 1;"
            "\nstruct args *myargs = (struct args*)malloc(sizeof(struct args));"
            "\nmyargs->bm = bm;"
            "\nmyargs->freeVars = freeVars;"
            "\nmyargs->backBuf = backBuf;"
            "\nmyargs->inMemTables = inMemTables;"
            "\n"
            "\nint rc = 0;"
            "\nrc = pthread_create(&thread, NULL, HeartBeat, (void*)myargs);"
            "\nif (rc){"
            "\nprintf(\"ERROR; return creating pthread.\\n\", rc);"
            "\nfflush(stdout);"
            "\nexit(1);"
            "\n}"
            "\n}");
        fflush(stdout);
        	} else {
        	  sprintf(heartbeat_buffer, "");	
        	}
          sprintf(buf,
            "\n%s\n{"
            "\n%s"
            "\nint rc = 0;"
            // "\nint _adl_sqlcode, _adl_cursqlcode;"
              "\nDBT key, data, windata;"
              "\nstruct timeval atime;"
              "\nRect r_key;"
              "\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];"
              "\nchar _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;"
              "\nint _rec_id=0; /* recursive id */"
              "\nchar _adl_dbname[MAX_STR_LEN];"
              "\nint slide_out = 1;"
              "\nwinbuf* rwinbuf = NULL;"
              "\nint rlast_out = 0;"
              "\nstatic int last_out = 0;"
              "\nstatic bool iterate = false;"
              "\nstatic bool init = true;"
              "\nchar _timeexpkey[MAX_STR_LEN];"
              "\nchar *timeexpkey=_timeexpkey;", temp, heartbeat_buffer);
          dec = expTy_Seq(dec, buf);
        }
        dec = expTy_Seq(dec, d);
        dec = expTy_Seq(dec, e);

        if (!isESL() || (isESL() && srcs.size() <= 1)) {
          dec = expTy_Seq(dec, "\nexit:\nreturn rc;\n};");
         if (isESL()) {
          sprintf(buf,
            "\nvoid *HeartBeat(void* arguments) {"
            "\n struct args* myargs = (struct args*)arguments;"
            "\n while(1) {"
            "\n sleep(%d);"
            "\nprintf(\"heartbeating\\n\");"
            "\n%s(myargs->bm, myargs->freeVars, myargs->backBuf, myargs->inMemTables);"
            "\n}\n}", window_size, queryName);

          dec = expTy_Seq(dec, buf);
         }

          appendElementList(global_functions, (nt_obj_t*) UID(a));
        }
      }
    } else {
      char rname[80];
      Ty_field rfield;
      A_sqlopr cur_sqlopr = sql->cur_sqlopr;

      S_beginScope(venv);
      S_beginScope(tenv);

      /* scalar select oper */
      if (sql->cur_sqlopr_flag == 1
      /*scalar query required */&& a->u.sqlopr->hxp_list->length != 1) {
        rc = ERR_NON_SCALAR;
        EM_error(a->pos, rc, __LINE__, __FILE__);
        goto exit;
      }

      if (!isESL() || isESLTest())
        addSqlSemFirstEntryDec(sql, (void*) a->u.sqlopr);

      sql->cur_sqlopr = a->u.sqlopr;
      sprintf(rname, "embed_%d", UID(a));
      vector<string> srcs;
      rc = transSelOpr(venv, tenv, a->u.sqlopr, sql, d, e, rname, aggregates,
          srcs, cstmt);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transSelOpr");
        goto exit;
      }

      sql->cur_sqlopr = cur_sqlopr;

      rc = declareQun2C(rname, e->ty, buf);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSqlQuery",
            "declareQun2C");
        goto exit;
      }
      sprintf(rname, "\nint embed_%d_cnt = 0;", UID(a));
      d = expTy_Seq(d, rname);

      /* construct the scalar result */
      rfield = (Ty_field) getNthElementList(e->ty->u.record, 0);
      sprintf(rname, "embed_%d.%s", UID(a), S_name(rfield->name));
      exe = expTy_Seq(exe, rname);
      exe->ty = rfield->ty;

      dec = expTy_Seq(dec, buf);
      dec = expTy_Seq(dec, d);
      dec = expTy_Seq(dec, e);

      S_endScope(tenv);
      S_endScope(venv);
    }
  }
    break;
  case A_selectExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_selectExp");
    A_exp na;

    rc = rewriteQuery(venv, tenv, a, na, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "rewriteQuery");
      goto exit;
    }

    rc = transExp(venv, tenv, na, sql, dec, exe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transExp");
      goto exit;
    }
  }
    break;
  case A_createviewExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_createviewExp");
    rc = transCreateviewExp(venv, tenv, a, sql, dec, exe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
          "transCreateviewExp");
      goto exit;
    }
  }
    break;
  case A_runtaskExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_runtaskExp");
    A_exp newExp;
    A_exp newOpr;
    if ((cstmt == NULL || strcmp("temp", cstmt->name) == 0) && isESL())
      cstmt = new cStmt(queryName);
    rc = rewriteRuntaskExp(venv, tenv, a, sql, dec, newExp, aggregates, NULL,
        cstmt);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
          "rewriteRuntaskExp");
      goto exit;
    }
    //may be we should just make an sqlopr out of it, let's give it a try
    A_list jtl = A_List(a->pos, A_QUN);

    appendAList(jtl, A_NameQun(a->pos, S_Symbol("stdout"), S_Symbol("stdout")));
    appendAList(jtl, (void*) A_QueryQun(a->pos, 0, newExp));

    newOpr = A_SqlOprExp(a->pos, A_SQL_INSERT, 0, (A_list) 0, jtl, (A_list) 0);
    rc = transExp(venv, tenv, newOpr, sql, dec, exe, aggregates, NULL, cstmt);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transExp");
      goto exit;
    }
    cstmt->print();
    fflush(stderr);
  }
    break;
  case A_createstreamExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_createstreamExp");
    rc = transCreatestreamExp(venv, tenv, a, sql, dec, exe, aggregates, cstmt);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp",
          "transCreatestreamExp");
      goto exit;
    }
  }
    break;
  case A_nilExp:
    exe = expTy(Tr_Int(0), Ty_Nil());
    break;
  case A_intExp:
    exe = expTy(Tr_Int(a->u.intt), Ty_Int());
    break;
  case A_realExp:
    exe = expTy(Tr_Real(a->u.realt), Ty_Real());
    break;
  case A_timestampExp:
    exe = expTy(Tr_Timestamp(a->u.timestamp), a->u.timestamp, Ty_Timestamp());
    break;
  case A_stringExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_stringExp");
    exe = expTy(Tr_QuoteString(a->u.string), Ty_String());
    exe->size = strlen(a->u.string) + 1;
  }
    break;
  case A_opExp: {
    SMLog::SMLOG(12, "abstract syntax kind is A_opExp");
    A_oper oper = a->u.op.oper;
    T_expty ldec, lexe, rdec, rexe = (T_expty) 0;

    setSqlOprFlag(oper, sql, a->u.op.left); // EXIST,NOT-EXIST,SCALAR
    rc = transExp(venv, tenv, a->u.op.left, sql, ldec, lexe, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transExp");
      goto exit;
    }
    dec = expTy_Seq(dec, ldec);

    if (a->u.op.right) {
      setSqlOprFlag(oper, sql, a->u.op.right); // EXIST,NOT-EXIST,SCALAR
      rc = transExp(venv, tenv, a->u.op.right, sql, rdec, rexe, aggregates);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "transExp");
        goto exit;
      }
      dec = expTy_Seq(dec, rdec);
    }

    switch (oper) {
    case A_eqOp:
    case A_neqOp:
    case A_ltOp:
    case A_leOp:
    case A_gtOp:
    case A_geOp:
      //should we compare types for left and right here???
      //I think so
      if (!makeCompatibleType(lexe->ty, rexe->ty)) {
        rc = ERR_INCOMPATIBLE_TYPE;
        EM_error(a->pos, rc, __LINE__, __FILE__);
        goto exit;
      }
      rc = doCmp(a->pos, oper, lexe, rexe, exe);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "doCmp");
        goto exit;
      }
      break;
    case A_plusOp:
    case A_minusOp:
    case A_timesOp:
    case A_divideOp:
    case A_modOp:
    case A_minuseqOp:
    case A_pluseqOp:
      rc = doArith(a->pos, oper, lexe, rexe, exe);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "doArith");
        goto exit;
      }
      break;
    case A_likeOp:
    case A_nlikeOp:
      break;
    case A_andOp:
    case A_orOp: {
      sprintf(buf, "((%s) %s (%s))", lexe->exp,
          (oper == A_andOp) ? "&&" : "||", rexe->exp);
      exe = expTy_Seq(exe, buf);
      exe->ty = Ty_Int();
    }
      break;
    case A_existOp:
    case A_notexistOp: {
      exe = expTy_Seq(exe, lexe);
    }
      break;
    case A_isnullOp:
    case A_isnnullOp: {
      rc = doNullCmp(a->pos, oper, lexe, exe);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transExp", "doNullCmp");
        goto exit;
      }
    }
      break;
    case A_inOp:
    case A_ninOp:
    default:
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__, "transExp");
      goto exit;
    }
    break;
  }
  }
  exit: SMLog::SMLOG(10, ">Exiting transExp: %i", rc);
  DBUG_RETURN(rc);
}

err_t transSeqDec(S_table venv, S_table tenv, A_list decl, T_expty &edec,
    T_expty &einit, T_expty &eclean, int inaggr, /* IN: if we are processing local
     variables of an aggregate routine*/
    T_expty &adec, /* defer aggregate till end of current
     struct - only needed when inaggr =1*/
    vector<void*> aggregates) {
  SMLog::SMLOG(10, "Entering transSeqDec");
  err_t rc = ERR_NONE;
  int i, j;
  T_expty ed, ei, ec;
  A_list reflist = A_List(0, A_EXP);

  DBUG_ENTER("transSeqDec");

  edec = einit = eclean = (T_expty) 0;

  for (i = 0; i < decl->length; i++) {
    A_dec d = (A_dec) getNthElementList(decl, i);
    SMLog::SMLOG(12, "\twindow_aggr_name: %s", S_name(d->name));
    for (j = 0; j < i; j++) {
      A_dec p = (A_dec) getNthElementList(decl, j);

      //check for duplicate names
      if (d->name == p->name) {
        rc = ERR_REDECLARATION;
        EM_error(p->pos, rc, __LINE__, __FILE__, S_name(d->name));
        goto exit;
      }

      //The two ifs below check if there is a windowed version of
      // an aggregate defined, if so change create_windowed to 0

      //printf("-checking %s\n", S_name(d->name));
      if (p->kind == A_aggregateDec && p->u.aggr.type != A_window) {
        char window_aggr_name[1024];
        sprintf(window_aggr_name, "%s_window", strdup(S_name(p->name)));
        //printf("checking %s %s\n", window_aggr_name, S_name(d->name));

        if (S_Symbol(window_aggr_name) == d->name) {
          //printf("doing set %s\n", S_name(p->name));
          p->u.aggr.create_windowed = 0;
        }
      }

      //printf("-checking %s\n", S_name(p->name));
      if (d->kind == A_aggregateDec && d->u.aggr.type != A_window) {
        char window_aggr_name[1024];
        sprintf(window_aggr_name, "%s_window", S_name(d->name));
        //printf("checking %s %s\n", window_aggr_name, S_name(p->name));

        if (S_Symbol(window_aggr_name) == p->name) {
          //printf("doing set1\n", S_name(d->name));
          d->u.aggr.create_windowed = 0;
        }
      }
    }
  }

  for (i = 0; i < decl->length; i++) {
    A_dec d = (A_dec) getNthElementList(decl, i);
    rc = transDec(venv, tenv, d, ed, ei, ec, inaggr, adec, aggregates, reflist);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSeqDec", "transDec");
      goto exit;
    }
    edec = expTy_Seq(edec, ed);
    einit = expTy_Seq(einit, ei);
    eclean = expTy_Seq(eclean, ec);

  }

  for (i = 0; i < reflist->length; i += 2) {
    Ty_field f = (Ty_field) getNthElementList(reflist, i);
    int pos = (int) getNthElementList(reflist, i + 1);
    E_enventry te = (E_enventry) S_look(venv, f->ref);

    if (!te || (te->kind != E_varEntry && te->kind != E_streamEntry)) {
      rc = ERR_UNDEFINED_REF;
      EM_error(pos, rc, __LINE__, __FILE__, S_name(f->ref));
      goto exit;
    }

    f->ty = te->u.var.ty;
  }

  clearList(reflist);
  deleteList(reflist);

  exit:

  SMLog::SMLOG(10, ">Exiting  transSeqDec: %i", rc);
  DBUG_RETURN(rc);
}

/* MORE added to allow index over functions
 * check the following:
 *	1) only 1 key declaration allowed
 *	2) key name must be a column name
 *	3) for RTree index
 *		3.1)	# of keys =2(point) or =4 (rectangle)
 *		3.2)    each key must be inteter type
 *
 * and if no key is defined then we use RECNO index
 */
err_t checkTabIndex(S_table venv, A_dec d, // the tablevar declaration
    Ty_ty ty, // column types
    E_enventry var // the tablevar
) {
  SMLog::SMLOG(10, "Entering checkTabIndex");
  err_t rc = ERR_NONE;
  A_list keydec_list, tuple_list;
  int i, j, nKeyColumns = 0;

  // key declarations
  if (d->u.tabvar.index != (A_index) 0 && !A_ListEmpty(
      d->u.tabvar.index->keydecs)) {

    /*
     if (d->u.tabvar.keydecs->length>1) {
     rc = ERR_TO_BE_IMPLEMENTED;
     EM_error(d->u.tabvar.keydecs->pos, rc, __LINE__, __FILE__,
     "multiple keys in table declaration");
     goto exit;
     }


     keydec_list = (A_list)getNthElementList(d->u.tabvar.keydecs, 0);
     */

    keydec_list = d->u.tabvar.index->keydecs;
    int iNumKey = 0;

    tuple_list = ty->u.record;

    nKeyColumns = keydec_list->length;
    for (i = 0; i < nKeyColumns; i++) {
      Ty_field fi;
      S_symbol skey = (S_symbol) getNthElementList(keydec_list, i);
      for (j = 0; j < tuple_list->length; j++) {
        fi = (Ty_field) getNthElementList(tuple_list, j);
        if (fi->name == skey) {
          fi->iskey = 1;
          d->u.tabvar.index->keyPos[iNumKey] = j; // append the key position into keyPos list ???
          iNumKey++;
          break;
        }
      }
      if (j >= tuple_list->length) {
        rc = ERR_KEY_NOT_FOUND;
        EM_error(d->u.tabvar.index->keydecs->pos, rc, __LINE__, __FILE__,
            S_name(skey));
        goto exit;
      }

      if (fi->ty->kind != Ty_int && var->u.var.index->kind == INDEX_RTREE) {
        /* as of now, we only support integer type RTREE */
        rc = ERR_RTREE_KEYTYPE;
        EM_error(d->pos, rc, __LINE__, __FILE__, S_name(d->name));
        goto exit;
      }
    }
    if (d->u.tabvar.index->func != (S_symbol) 0) {
      E_enventry eFun = (E_enventry) S_look(venv, d->u.tabvar.index->func);
      if (!eFun || eFun->kind != E_extEntry || eFun->u.ext.formals->length
          != nKeyColumns || (eFun->u.ext.result->kind != Ty_int
          && eFun->u.ext.result->kind != Ty_real && eFun->u.ext.result->kind
          != Ty_string && eFun->u.ext.result->kind != Ty_timestamp)) {
        rc = ERR_INVALID_INDEX_SPEC;
        EM_error(d->pos, rc, __LINE__, __FILE__,
            "1) External function for in dex not specified or"
              " 2) Number of arguments to external function different from specified or"
              " 3) Result type of the external function is not a scalar");
        goto exit;
      }
      for (i = 0; i < nKeyColumns; i++) {
        Ty_field fi;
        S_symbol skey = (S_symbol) getNthElementList(keydec_list, i);
        Ty_field argFi = (Ty_field) getNthElementList(eFun->u.ext.formals, i);
        for (j = 0; j < tuple_list->length; j++) {
          fi = (Ty_field) getNthElementList(tuple_list, j);
          if (fi->name == skey) {
            if (fi->ty != argFi->ty) {
              char error[80];
              sprintf(error, "Incompatible argument type at index %d", i);
              rc = ERR_INVALID_INDEX_SPEC;
              EM_error(d->pos, rc, __LINE__, __FILE__, error);
              goto exit;
            }
          }
        }
      }
    }
  }

  if (nKeyColumns != 2 && nKeyColumns != 4 && var->u.var.index
      && var->u.var.index->kind == INDEX_RTREE) {
    /* as of now, we only support 2-dimensional index */
    rc = ERR_RTREE_DIMENSION;
    EM_error(d->pos, rc, __LINE__, __FILE__, S_name(d->name));
    goto exit;
  }

  if (nKeyColumns == 0) {
    /* no key defined by user, we set the first column as key */
    /* Ty_field keyf = (Ty_field)getNthElementList(ty->u.record, 0);
     keyf->iskey = 1;
     */

    /* no key defined by user, we use RECNO table of Berkeley DB (instead of B+-tree) */
    //var->u.var.dbtype = DB_RECNO;
    //var->u.var.index = INDEX_RECNO;  //Hetal -- I don't think, we need this
  }

  exit: return rc;
}

/* the above function for create views */
/*err_t
 checkTabIndexCreateview(A_exp e,		// the tablevar declaration
 Ty_ty ty,		// column types
 E_enventry var	// the tablevar
 )
 {
 err_t rc = ERR_NONE;
 A_list keydec_list, tuple_list;
 int i, j, nKeyColumns=0;

 // key declarations
 if (!A_ListEmpty(e->u.createview.keydecs)) {

 keydec_list = e->u.createview.keydecs;
 int iNumKey = 0;

 //rtree for create view not supported
 if (var->u.var.index == INDEX_RTREE) {
 rc = ERR_RTREE_MEMORY;
 EM_error(e->pos, rc, __LINE__, __FILE__, S_name(e->u.createview.name));
 goto exit;
 }

 tuple_list = ty->u.record;

 nKeyColumns = keydec_list->length;
 for (i=0; i<nKeyColumns; i++) {
 Ty_field fi;
 S_symbol skey = (S_symbol)getNthElementList(keydec_list, i);
 for (j=0; j<tuple_list->length; j++) {
 fi = (Ty_field)getNthElementList(tuple_list, j);
 if (fi->name == skey) {
 fi->iskey = 1;
 e->u.createview.keyPos[iNumKey] = j; // append the key position into keyPos list ???
 iNumKey++;
 break;
 }
 }
 if (j>=tuple_list->length) {
 rc = ERR_KEY_NOT_FOUND;
 EM_error(e->u.createview.keydecs->pos, rc, __LINE__, __FILE__, S_name(skey));
 goto exit;
 }

 //Unneeded because rtree for create view not supported
 }
 }

 if (nKeyColumns == 0) {
 var->u.var.index = INDEX_RECNO;
 }

 exit:
 return rc;
 }*/
/* 
 * check the Time Key:
 *	1) only 1 key declaration allowed
 *	2) key name must be a column name
 */
err_t checkTimeKey(A_dec d, // the streamvar declaration
    Ty_ty ty, // column types
    E_enventry var // the streamvar
) {
  SMLog::SMLOG(10, "Entering checkTimeKey");
  err_t rc = ERR_NONE;
  A_list tuple_list;
  int i, j, nKeyColumns = 0;

  // key declarations
  if (d->u.streamvar.timekey) {
    tuple_list = ty->u.record;

    Ty_field fi;
    S_symbol skey = d->u.streamvar.timekey;
    for (j = 0; j < tuple_list->length; j++) {
      fi = (Ty_field) getNthElementList(tuple_list, j);
      if (fi->name == skey) {
        fi->isTimekey = 1;
        break;
      }
    }

    if (skey != (S_symbol) NULL && j >= tuple_list->length) {
      rc = ERR_UNDEFINED_FIELD;
      EM_error(d->pos, rc, __LINE__, __FILE__, S_name(skey));
      goto exit;
    }

    if (j >= tuple_list->length) {
      // internal timekey
      appendElementList(tuple_list, (nt_obj_t*) A_Field(0, skey, S_Symbol(
          "TIMESTAMP")));
      goto exit;
    }
  }
  exit: return rc;
}

char* getVarNameFromIname(S_symbol iname, S_symbol parent) {
  char* ciname = S_name(iname);
  char* val = strstr(ciname, "status->");
  if (val == NULL)
    return ciname;
  else {
    val = val + 8;
    char ret[50];
    sprintf(ret, "%s_Inst->%s", S_name(parent), val);
    return strdup(ret);
  }
}

char* getINameFromVarName(S_symbol varname, S_symbol parent) {
  SMLog::SMLOG(10, "Entering getINameFromVarName");
  char* cvarname = S_name(varname);
  char* cparent = S_name(parent);
  char tomatch[50];
  sprintf(tomatch, "%s_Inst->", cparent);
  char* val = strstr(cvarname, tomatch);
  if (val == NULL)
    return cvarname;
  else {
    val = val + strlen(tomatch);
    char ret[50];
    sprintf(ret, "status->%s", val);
    return strdup(ret);
  }
}

S_symbol getESLType(Ty_ty ty) {
  switch (ty->kind) {
  case Ty_int:
    return S_Symbol("int");
    break;
  case Ty_real:
    return S_Symbol("real");
    break;
  case Ty_string:
    return S_Symbol("char");
    break;
  case Ty_timestamp:
    return S_Symbol("timestamp");
    break;
  case Ty_iext:
    return S_Symbol("iExt");
    break;
  case Ty_rext:
    return S_Symbol("rExt");
    break;
  case Ty_cext:
    return S_Symbol("cExt");
    break;
  case Ty_text:
    return S_Symbol("tExt");
    break;
  default:
    return S_Symbol("int");
  }
}

vector<void*> S_endScopeval(S_table);

err_t checkModelTables(S_table venv, S_table tenv, A_list tables,
    char* modelId, A_dec d, // IN: a declaration
    T_expty &edec, // declaration. e.g. "DB *tab;"
    T_expty &einit, // initialization. e.g. open a table
    T_expty &eclean, // clean up. e.g. close a table
    int inaggr, /* IN: inaggr= 1 if we are processing
     local variables of an aggregate
     routine*/
    T_expty &adec, /* defer aggregate till end of current
     struct - only needed when inaggr =1*/
    vector<void*> aggregates, /* for nested aggregates */
    A_list reflist /* OUT: contains the attribute
 attributes whose types are not declared */
) {
  SMLog::SMLOG(10, "Entering checkModelTables");
  err_t rc = ERR_NONE;
  E_enventry te, y;
  T_expty ed, ei, ec;
  A_dec dec;
  char uName[256];
  char usName[256];
  char sharedTblName1[512];
  char* sharedTblName;

  getUserNameie(uName);

  DBUG_ENTER("checkModelTables");
  for (int j = 0; j < tables->length; j++) {
    /* here we make sure that the orig table exists
     and check if the specific is defined. If specific is not defined
     we create it. */
    S_symbol s = (S_symbol) getNthElementList(tables, j);
    sprintf(usName, "%s%s", uName, S_name(s));
    te = (E_enventry) S_look(venv, S_Symbol(usName));
    if (!te) {
      rc = ERR_IDENT_UNDEFINED;
      EM_error(d->pos, rc, __LINE__, __FILE__, usName);
      goto exit;
    }
    sprintf(sharedTblName1, "%s%s", modelId, S_name(s));
    sharedTblName = strdup(sharedTblName1);
    y = (E_enventry) S_look(venv, S_Symbol(sharedTblName));
    if (!y) {
      y = E_VarEntry(S_Symbol(sharedTblName), te->u.var.ty, 0, TAB_MEMORY,
          copyAIndex(te->u.var.index),
          (te->u.var.index == (A_index) 0) ? 0 : 1, 0, 0);
      //create the tab var
      A_list f_types = te->u.var.ty->u.record;
      A_list a_types = A_List(d->pos, A_FIELD);
      int l = f_types->length;
      for (int k = 0; k < l; k++) {
        Ty_field t = (Ty_field) getNthElementList(f_types, k);
        appendAList(a_types, A_Field(d->pos, t->name, getESLType(t->ty),
            t->size));
      }
      dec = A_TabVarDec(d->pos, S_Symbol(sharedTblName), A_RecordTy(d->pos,
          a_types), copyAIndex(te->u.var.index), TAB_MEMORY, (A_exp) 0);
      rc = transDec(venv, tenv, dec, ed, ei, ec, inaggr, adec, aggregates,
          reflist);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transSeqDec", "transDec");
        goto exit;
      }
      edec = expTy_Seq(edec, ed);
      einit = expTy_Seq(einit, ei);
      eclean = expTy_Seq(eclean, ec);
    }
  }
  exit: DBUG_RETURN(rc);
  return rc;
}

err_t transDec(S_table venv, S_table tenv, A_dec d, // IN: a declaration
    T_expty &edec, // declaration. e.g. "DB *tab;"
    T_expty &einit, // initialization. e.g. open a table
    T_expty &eclean, // clean up. e.g. close a table
    int inaggr, /* IN: inaggr= 1 if we are processing
     local variables of an aggregate
     routine*/
    T_expty &adec, /* defer aggregate till end of current
     struct - only needed when inaggr =1*/
    vector<void*> aggregates, /* for nested aggregates */
    A_list reflist /* OUT: contains the attribute
 attributes whose types are not declared */

) {
  SMLog::SMLOG(10, "Entering transDec kind -> %i, inaggr -> %i", d->kind,
      inaggr);
  char code[MAX_STR_LEN];
  code[0] = '\0';
  err_t rc = ERR_NONE;
  int i;
  bufferMngr* bm = bufferMngr::getInstance();

  DBUG_ENTER("transDec");

  edec = einit = eclean = (T_expty) 0;

  switch (d->kind) {
  case A_aggregateDec: {
    SMLog::SMLOG(12, "\tkind -> A_aggregateDec");
    if (inaggr == 1) {
      vector<void*> localVars = S_endScopeval(venv);
      int size = localVars.size();
      S_beginScope(venv);
      S_symbol curAggregate = (S_symbol) aggregates.back();
      for (int i = 0; i < size; i++) {
        E_enventry e = (E_enventry) localVars.operator[](i);
        if (e->kind == E_varEntry && e->u.var.iname != NULL) {
          e->u.var.iname = S_Symbol(getVarNameFromIname(e->u.var.iname,
              curAggregate));
          S_enter(venv, e->key, e);
        } else {
          S_enter(venv, e->key, e);
        }
      }
      rc = transAggrDec(venv, tenv, d, adec, einit, eclean, inaggr, aggregates);
      //printf("back from transAggr\n");
      localVars = S_endScopeval(venv);
      size = localVars.size();
      S_beginScope(venv);
      for (int i = 0; i < size; i++) {
        E_enventry e = (E_enventry) localVars.operator[](i);
        if (e->kind == E_varEntry && e->u.var.iname != NULL) {
          e->u.var.iname = S_Symbol(getINameFromVarName(e->u.var.iname,
              curAggregate));
          S_enter(venv, e->key, e);
        } else {
          S_enter(venv, e->key, e);
        }
      }
    } else if (inaggr != 1) {
      if (isESL() || isAdHoc()) {
        //in ESL we first call the aggregate by itself
        //as if it were the only thing in ADL file,
        //so we will have the windowed version
        // through the else below, for base aggrs that is.
        //I believe then we only need to change the call
        //to the aggr, if the aggr is win, then check in the
        //env entry if it is default then look for it
        //in the base aggr file as opposed to the
        //file with the name of this windowed aggr
        //changes in semant_aggr.cc
        //but I think that to get the default window in the
        //environment we need to do the stuff in else here
        T_expty einit1, eclean1;
        einit1 = eclean1 = (T_expty) 0;
        rc = transAggrDec(venv, tenv, d, edec, einit1, eclean1, inaggr,
            aggregates);

        //so here we go
        if (d->u.aggr.type != A_window && d->u.aggr.create_windowed) {
          //create the windowed aggregate here, defualt if none exists
          char winAggr[1024];
          sprintf(winAggr, "%s_window", S_name(d->name));
          E_enventry w = (E_enventry) S_look(venv, S_Symbol(winAggr));
          if (!w && isESL()) {
            //in adhoc this creates problems, but it is not required
            //define Dec then just call transAggrDec, hopefully that will do it
            /* following create aggregate as follows, only for non isESL case
             WINDOW AGGREGATE orig_name_window (func_agrs):func_result{
             INITIALIZE:ITERATE: {
             insert into return select orig_name(func_args) from inwindow;
             }
             }
             */
            //preparing select list
            A_list wasl = A_List(d->pos, A_SELECT_ITEM);
            A_list wa_func_argl = A_List(d->pos, A_EXP);
            A_list func_params = d->u.aggr.params;
            for (int i = 0; i < func_params->length; i++) {
              A_field fi = (A_field) getNthElementList(func_params, i);
              appendAList(wa_func_argl, (void*) A_SqlVarExp(d->pos, S_Symbol(
                  "w"), fi->name));
            }
            A_exp wa_func_exp = A_CallExp(d->pos, d->name, wa_func_argl,
                (S_symbol) 0);

            //preparing qun_list, from list
            A_list waql = A_List(d->pos, A_QUN);
            appendAList(waql, A_NameQun(d->pos, S_Symbol("inwindow"), S_Symbol(
                "w")));

            //preparing select statement
            A_exp waq = A_Select(d->pos, 0, wasl, waql, (A_list) 0, (A_list) 0,
                (A_list) 0, (A_list) 0);
            /*displayExp((A_exp)getNthElementList(wasl, 0));
             printf("\n");
             displayExp(waq);
             printf("\n");*/

            //preparing insert into retrurn select...
            A_list jtl = A_List(d->pos, A_QUN);
            S_symbol std = S_Symbol("return");
            appendAList(jtl, (void*) A_NameQun(d->pos, std, std));
            appendAList(jtl, (void*) A_QueryQun(d->pos, 0, waq));

            //puting stuff in iterate
            A_exp wa_iterate = A_SqlOprExp(d->pos, A_SQL_INSERT, 0, (A_list) 0,
                jtl, (A_list) 0);

            A_dec wa = A_Aggrdec(d->pos, A_window, S_Symbol(winAggr),
                d->u.aggr.params, d->u.aggr.result, (A_list) 0, (A_exp) 1,
                wa_iterate, (A_exp) 0, (A_exp) 0, (A_dec) 0);
            setParent(wa, wa_func_exp, A_DEC);
            setParent(wa, wa_func_argl, A_DEC);
            setParent(wa, wasl, A_DEC);

            //printf("calling transAggrDec of window\n");
            rc = transAggrDec(venv, tenv, wa, edec, einit1, eclean1, inaggr,
                aggregates, 1);
          }
        }
      } else {
        //printf("calling trans Aggr for simple\n");
        if ((!isESLAggr() || inaggr || (isESLAggr() && (strlen(S_name(d->name))
            >= strlen(getAggrName())) && (strncmp(getAggrName(),
            S_name(d->name), strlen(getAggrName())) == 0)))) {

          rc = transAggrDec(venv, tenv, d, edec, einit, eclean, inaggr,
              aggregates);
          if (rc) {
            EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec",
                "transAggrDec");
            goto exit;
          }
          //printf("here %s %d\n", S_name(d->name), d->u.aggr.create_windowed);
          if (d->u.aggr.type != A_window && d->u.aggr.create_windowed) {
            //create the windowed aggregate here, defualt if none exists
            char winAggr[1024];
            sprintf(winAggr, "%s_window", S_name(d->name));
            E_enventry w = (E_enventry) S_look(venv, S_Symbol(winAggr));
            if (!w) {
              //define Dec then just call transAggrDec, hopefully that will do it
              /* following create aggregate as follows, only for non isESL case
               WINDOW AGGREGATE orig_name_window (func_agrs):func_result{
               INITIALIZE:ITERATE: {
               insert into return select orig_name(func_args) from inwindow;
               }
               }
               */
              //preparing select list
              A_list wasl = A_List(d->pos, A_SELECT_ITEM);
              A_list wa_func_argl = A_List(d->pos, A_EXP);
              A_list func_params = d->u.aggr.params;
              if (func_params) {
                for (int i = 0; i < func_params->length; i++) {
                  A_field fi = (A_field) getNthElementList(func_params, i);
                  appendAList(wa_func_argl, (void*) A_SqlVarExp(d->pos,
                      S_Symbol("w"), fi->name));
                }
              }
              A_exp wa_func_exp = A_CallExp(d->pos, d->name, wa_func_argl,
                  (S_symbol) 0);
              appendAList(wasl, A_SelectItem(d->pos, wa_func_exp, (S_symbol) 0));

              //preparing qun_list, from list
              A_list waql = A_List(d->pos, A_QUN);
              appendAList(waql, A_NameQun(d->pos, S_Symbol("inwindow"),
                  S_Symbol("w")));

              //preparing select statement
              A_exp waq = A_Select(d->pos, 0, wasl, waql, (A_list) 0,
                  (A_list) 0, (A_list) 0, (A_list) 0);
              /*displayExp((A_exp)getNthElementList(wasl, 0));
               printf("\n");
               displayExp(waq);
               printf("\n");*/

              //preparing insert into retrurn select...
              A_list jtl = A_List(d->pos, A_QUN);
              S_symbol std = S_Symbol("return");
              appendAList(jtl, (void*) A_NameQun(d->pos, std, std));
              appendAList(jtl, (void*) A_QueryQun(d->pos, 0, waq));

              //puting stuff in iterate
              A_exp wa_iterate = A_SqlOprExp(d->pos, A_SQL_INSERT, 0,
                  (A_list) 0, jtl, (A_list) 0);

              A_dec wa = A_Aggrdec(d->pos, A_window, S_Symbol(winAggr),
                  d->u.aggr.params, d->u.aggr.result, (A_list) 0, (A_exp) 1,
                  wa_iterate, (A_exp) 0, (A_exp) 0, (A_dec) 0);
              setParent(wa, wa_func_exp, A_DEC);
              setParent(wa, wa_func_argl, A_DEC);
              setParent(wa, wasl, A_DEC);

              //printf("calling transAggrDec of window\n");
              rc = transAggrDec(venv, tenv, wa, edec, einit, eclean, inaggr,
                  aggregates, 1);
            }
          }
        }
      }
    }
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transAggrDec");
      goto exit;
    }
  }
    break;
  case A_externDec: {
    Ty_ty resultTy;
    char tmpbuf[128];
    char retType[80];
    char argbuf[2048];
    argbuf[0] = '\0';
    A_list formalTys = A_List(0, A_UNKNOWN);
    E_enventry ext_entry;

    /* return type of external function */
    rc = transTy(venv, tenv, d->u.ext.result, resultTy, (A_list) 0, (int*) 0,
        A_tabVarDec);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transTy");
      goto exit;
    }

    /* arguments of external function */
    if (d->u.ext.params) {
      rc = transFields(venv, tenv, d->u.ext.params, formalTys, (A_list) 0,
          (int*) 0, A_externDec); /* , (A_list)0, (int*)0); */
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transFields");
        goto exit;
      }
    }

    /* enter external function into symbol table */

    ext_entry = E_ExtEntry(d->name, formalTys, resultTy, d->u.ext.actual,
        d->u.ext.externlib, d->u.ext.size);
    S_enter(venv, d->name, ext_entry);

    /* an example:
     #include <dlfcn.h>
     int main(int argc, char **argv) {
     void *handle = dlopen ("/lib/libm.so", RTLD_LAZY);
     double (*cosine)(double) = dlsym(handle, "cos");
     printf ("%f\n", (*cosine)(2.0));
     dlclose(handle);
     }
     */

    /* code for delcaration */
    rc = Ty2C(resultTy, tmpbuf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "Ty2C");
      goto exit;
    }
    sprintf(code, "\n%s (*_adl_func_%d)(", tmpbuf, UID(ext_entry));
    sprintf(retType, "%s", tmpbuf);

    /* external table function has two more parameters:

     int first_entry
     void *return_stru
     */
    if (resultTy->kind == Ty_record) {
      /* this is a table function */
      strcat(code, (formalTys->length > 0) ? "int, void *, " : "int, void *");
    }

    for (i = 0; i < formalTys->length; i++) {
      Ty_field f = (Ty_field) getNthElementList(formalTys, i);
      rc = Ty2C(f->ty, tmpbuf);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "Ty2C");
        goto exit;
      }
      if (i > 0) {
        strcat(code, ", ");
        strcat(argbuf, ", ");
      }
      strcat(code, tmpbuf);
      strcat(argbuf, tmpbuf);
    }
    strcat(code, ") = NULL;");
    edec = expTy_Seq(edec, code);

    /* code for initialization */
    sprintf(code,
        "\n_adl_func_%d = (%s(*)(%s%s))_adl_dlm(\"%s\", \"%s\"); //h4",
        UID(ext_entry), retType, argbuf,
        (resultTy->kind == Ty_record) ? ",void*, int" : "", S_name(
            d->u.ext.externlib), S_name(d->u.ext.actual));
    einit = expTy_Seq(einit, code);

    /* code for clean up */
  }
    break;
  case A_functionDec: {
    rc = transUDFDec(venv, tenv, d, edec, einit, eclean, aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transUDFDec");
      goto exit;
    }
  }
    break;
  case A_modelTypeDec: {
    E_enventry te, x;
    char modelId[512];
    A_list sharedTables;
    A_list modelItems;
    A_list flows;
    A_exp fse;
    int window;
    S_symbol firstParTable;
    E_enventry fpt, nstr;
    Ty_ty fptTy;
    Ty_field f;
    A_modelitem mi;
    A_flow flow;
    char uName[256];
    char usName[256];

    getUserNameie(uName);

    if (d->u.modeltype.copyModel != (S_symbol) 0) {
      x = (E_enventry) S_look(venv, d->u.modeltype.copyModel);
      //make sure the copyModel exists
      if (!x || x->kind != E_modelTypeEntry) {
        rc = ERR_IDENT_UNDEFINED;
        EM_error(d->pos, rc, __LINE__, __FILE__, S_name(
            d->u.modeltype.copyModel));
        goto exit;
      }

      sharedTables = x->u.modeltype.sharedtables;
      modelItems = x->u.modeltype.modelitems;
      flows = x->u.modeltype.flows;
    } else {
      sharedTables = d->u.modeltype.sharedtables;
      modelItems = d->u.modeltype.modelitems;
      flows = d->u.modeltype.flows;
    }

    sprintf(modelId, "%s_", S_name(d->name));
    //make sure that the share tables exist
    rc = checkModelTables(venv, tenv, sharedTables, modelId, d, edec, einit,
        eclean, inaggr, adec, aggregates, reflist);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec",
          "checkModelTables");
      goto exit;
    }

    //make sure that the aggregate exists
    for (int i = 0; i < modelItems->length; i++) {
      mi = (A_modelitem) getNthElementList(modelItems, i);

      /* not checkinmg for windowsize, i think we will use OLAP construct */
      /*
       //check if params includes windowsize
       bool windowsize = false;
       for(int j=0; j < mi->allowedparams->length && !windowsize; j++) {
       Ty_field f = (Ty_field)getNthElementList(mi->allowedparams, j);
       if(f->name == S_Symbol("windowsize")) {
       windowsize = true;
       }
       }
       if(!windowsize && (!te || te->kind != E_aggrEntry)) {
       rc = ERR_IDENT_UNDEFINED;
       EM_error(d->pos, rc, __LINE__, __FILE__, S_name(mi->uda));
       goto exit;
       }
       */

      //check the aggregate exists, windowed or base depending on options
      if (mi->window == 0) {
        te = (E_enventry) S_look(venv, mi->uda);
        /*if(!te || te->kind != E_aggrEntry) {
         rc = ERR_IDENT_UNDEFINED;
         EM_error(d->pos, rc, __LINE__, __FILE__, S_name(mi->uda));
         goto exit;
         }*/
      } else {
        char udaName[100];
        sprintf(udaName, "%s_window", S_name(mi->uda));
        te = (E_enventry) S_look(venv, S_Symbol(udaName));
        if (!te || te->kind != E_aggrEntry) {
          rc = ERR_IDENT_UNDEFINED;
          EM_error(d->pos, rc, __LINE__, __FILE__, udaName);
          goto exit;
        }
      }

      //make sure that the param tables exist
      rc = checkModelTables(venv, tenv, mi->paramtables, modelId, d, edec,
          einit, eclean, inaggr, adec, aggregates, reflist);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec",
            "checkModelTables");
        goto exit;
      }

      //we should make sure that the first two columns in the first partable
      // are both chars, which is where the params will be put
      firstParTable = (S_symbol) getNthElementList(mi->paramtables, 0);
      sprintf(usName, "%s%s", uName, S_name(firstParTable));
      fpt = (E_enventry) S_look(venv, S_Symbol(usName));
      fptTy = fpt->u.var.ty;
      if (fptTy->u.record->length != 3) { //3 cause 2 + OID
        rc = ERR_INVALID_PARAM_TABLE;
        EM_error(d->pos, rc, __LINE__, __FILE__, usName);
        goto exit;
      }
      for (int k = 0; k < 2; k++) {
        f = (Ty_field) getNthElementList(fptTy->u.record, k);
        if (f->ty->kind != Ty_string) {
          rc = ERR_INVALID_PARAM_TABLE;
          EM_error(d->pos, rc, __LINE__, __FILE__, usName);
          goto exit;
        }
      }
    }

    //validate flows
    for (int i = 0; flows != (A_list) 0 && i < flows->length; i++) {
      flow = (A_flow) getNthElementList(flows, i);
      for (int j = 0; j < flow->statements->length; j++) {
        fse = (A_exp) getNthElementList(flow->statements, j);
        //check fse type, create stream or runtask
        if (fse->kind == A_createstreamExp) {
          /*
           A_exp query = fse->u.createstream.query;
           bool found = false;
           if(query->kind == A_runtaskExp) {
           //if the source for create stream is a runtask then we look up
           for(int i = 0; i < modelItems->length && !found; i++) {
           mi = (A_modelitem)getNthElementList(modelItems, i);
           if(mi->name == query->u.runtask.task) {
           found = true;
           }
           }
           if(!found) {
           rc = ERR_UNDEFINED_VARIABLE;
           EM_error(flow->pos, rc, __LINE__, __FILE__, S_name(query->u.runtask.task));
           goto exit;
           }

           //lookup the uda for mi, the return type for this uda is the
           // schema for the create stream
           if(mi->window == 0) {
           te = (E_enventry)S_look(venv, mi->uda);
           }
           else {
           char udaName[100];
           sprintf(udaName, "%s_window", S_name(mi->uda));
           te = (E_enventry)S_look(venv, S_Symbol(udaName));
           }
           //te can't be NULL, since we checked just a little bit earlier
           Ty_ty result = te->u.fun.result;
           nstr = E_StreamEntry(fse->u.createstream.name, te->u.fun.result,
           0, 0, 0, tk_none, (S_symbol)NULL);
           S_enter(venv, fse->u.createstream.name, nstr);
           }
           //need some fixin here
           else if(query->kind == A_selectExp) {
           T_expty di=(T_expty)0;
           T_expty ei=(T_expty)0;
           Sql_sem asql = SqlSem();
           A_exp na;
           vector<string> srcs;
           A_exp  newQuery = copyExp(query);
           cStmt* cstmt = new cStmt("temp");

           rc = rewriteQuery(venv, tenv, newQuery, na, aggregates);
           if (rc) {
           EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "rewriteQuery");
           goto exit;
           }

           //call transSelOpr here to find out the ty
           rc = transSelOpr(venv, tenv, na->u.sqlopr, asql, di, ei, "temp", aggregates, srcs, cstmt);
           if (rc) {
           EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transSelOpr");
           goto exit;
           }

           rc = renameTypeNames(venv, query, ei);
           if (rc) {
           EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "renameTypeNames");
           goto exit;
           }

           //create a stream Entry and enter it in venv
           nstr = E_StreamEntry(fse->u.createstream.name, ei->ty, 0, 0, 0,
           tk_none, (S_symbol)NULL);
           S_enter(venv, fse->u.createstream.name, nstr);
           }
           else if(query->kind == A_sqloprExp) {
           if(query->u.sqlopr->kind == A_SQL_UNION
           && query->u.sqlopr->jtl_list
           && query->u.sqlopr->jtl_list->length > 0) {
           A_qun qun = (A_qun)getNthElementList(query->u.sqlopr->jtl_list, 0);
           if(qun->kind == QUN_QUERY) {
           query = qun->u.query;
           }
           else {
           rc = ERR_CREATE_STREAM;
           EM_error(flow->pos, rc, __LINE__, __FILE__, "Create stream only allowed with union statements apart from slect and run statements.");
           goto exit;
           }
           }
           else {
           rc = ERR_CREATE_STREAM;
           EM_error(flow->pos, rc, __LINE__, __FILE__, "Create stream only allowed with union statements apart from slect and run statements.");
           goto exit;
           }

           //if source is something else, we compile
           //only issue is that if this other statement uses INSTREAM,
           //or OUSTREAM we have an issue
           T_expty di=(T_expty)0;
           T_expty ei=(T_expty)0;
           Sql_sem asql = SqlSem();
           A_exp na;
           vector<string> srcs;
           A_exp  newQuery = copyExp(query);
           cStmt* cstmt = new cStmt("temp");

           rc = rewriteQuery(venv, tenv, newQuery, na, aggregates);
           if (rc) {
           EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "rewriteQuery");
           goto exit;
           }

           //call transSelOpr here to find out the ty
           rc = transSelOpr(venv, tenv, na->u.sqlopr, asql, di, ei, "temp", aggregates, srcs, cstmt);
           if (rc) {
           EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transSelOpr");
           goto exit;
           }

           rc = renameTypeNames(venv, query, ei);
           if (rc) {
           EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "renameTypeNames");
           goto exit;
           }

           //create a stream Entry and enter it in venv
           nstr = E_StreamEntry(fse->u.createstream.name, ei->ty, 0, 0, 0,
           tk_none, (S_symbol)NULL);
           S_enter(venv, fse->u.createstream.name, nstr);
           }
           else {
           rc = ERR_CREATE_STREAM;
           EM_error(flow->pos, rc, __LINE__, __FILE__, "Create stream only allowed with select statements.");
           goto exit;
           }
           */
        } else if (fse->kind == A_runtaskExp || fse->kind == A_sqloprExp) {
          //the above allows run task and insert into OUTSTREAM statements
          //also allows unions, etc.
          //do nothing
        } else {
          rc = ERR_TO_BE_IMPLEMENTED;
          EM_error(flow->pos, rc, __LINE__, __FILE__,
              "statements other than create stream and runtask"
                " in flow definition");
          goto exit;
        }
      }
    }

    //now that everything checks out, we need to enter this model
    //in the environment;
    S_enter(venv, d->name, E_ModelTypeEntry(d->name, sharedTables, modelItems,
        flows));
  }
    break;
  case A_tabVarDec: {
    Ty_ty ty;
    char dbname[80], handlename[80];
    E_enventry var;
    int foundref = 0; // is any of the columns of ref type?
    int nKeyColumns = 0;
    IM_REL* inMem;
    A_index index = d->u.tabvar.index;

    if (d->u.tabvar.isView == 1) {
      //create a var Entry and enter it in venv, the var should have isBuf = 1
      //compile tuple type first
      rc = transTy(venv, tenv, d->u.tabvar.ty, ty, reflist, &foundref,
          A_tabVarDec);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transTy");
        goto exit;
      }

      if (d->u.tabvar.index != (A_index) 0) {
        //need to take care of index TODO
        var = E_VarEntry(d->name, ty, 0, TAB_MEMORY, copyAIndex(
            d->u.tabvar.index), (index == (A_index) 0) ? 0 : 1, 0, 1);
        /* check index type */
        rc = checkTabIndex(venv, d, ty, var);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec",
              "checkTabIndex");
          goto exit;
        }
        var->u.var.firstKey = d->u.tabvar.index->keyPos[0];
      } else {
        var = E_VarEntry(d->name, ty, 0, TAB_MEMORY, (A_index) 0, (index
            == (A_index) 0) ? 0 : 1, 0, 1);
      }

      S_enter(venv, d->name, var);

      char buf[4096];
      sprintf(buf, "\nwindowBuf *%s;", S_name(d->name));
      edec = expTy_Seq(edec, buf);

      sprintf(buf, "\n%s = bm->lookup(\"%%s\");"
        "\nif(!%s) {", S_name(d->name), S_name(d->name), S_name(d->name));
      einit = expTy_Seq(einit, buf);
      if (isESL()) {
        sprintf(
            buf,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: bm->lookup() windowBuf not found\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            buf,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: bm->lookup() windowBuf not found\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(buf, "\nadlabort(rc, \"bm->lookup() windowBuf not found\");"
          "\n}");
      }
      einit = expTy_Seq(einit, buf);

    } else {
      char dbFile[256];
      // compile tuple type
      rc = transTy(venv, tenv, d->u.tabvar.ty, ty, reflist, &foundref,
          A_tabVarDec);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transTy");
        goto exit;
      }

      if (d->u.tabvar.scope == TAB_LOCAL && (isESL() || isAdHoc()
          || isESLAggr()) && !inaggr) {

        //in ESL we only support memory and source table thus we change this to mem
        d->u.tabvar.scope = TAB_MEMORY;
      }

      if (d->u.tabvar.scope == TAB_MEMORY) {
        /* We need to expand the ty structure to include one special
         field: OID.  OID of the current tuple is always returned by
         cursor->get() as the last field in the data area of the
         (key,data) pair.*/
        A_list fields = ty->u.record;

        //check if the last is already OID, don't add -- for tables that are
        // created from other tables -- ModelType ...
        Ty_field last_field = (Ty_field) getNthElementList(fields,
            fields->length - 1);
        if (last_field->name != S_Symbol("oid")) {
          Ty_field oid_field = Ty_Field(S_Symbol("oid"), Ty_REF(), sizeof(int));

          oid_field->implicit = 1; /* this is a field that is not
           declared explicitly by the user */
          appendElementList(fields, (nt_obj_t*) oid_field);
        }

        if (index != (A_index) 0 && d->u.tabvar.index->kind == INDEX_RTREE) {
          /* RTREE is disk-resident index */
          rc = ERR_RTREE_MEMORY;
          EM_error(d->pos, rc, __LINE__, __FILE__, S_name(d->name));
          goto exit;
        }

      } else if (foundref == 1) {
        /* only in-memory table allows ref type */
        rc = ERR_REF_MEMORY_ONLY;
        EM_error(d->pos, rc, __LINE__, __FILE__, S_name(d->name));
        goto exit;
      }

      var = E_VarEntry(d->name, ty, 0, d->u.tabvar.scope, copyAIndex(
          d->u.tabvar.index), (d->u.tabvar.index == (A_index) 0) ? 0 : 1);
      var->u.var.source = d->u.tabvar.source;

      if (d->u.tabvar.source != (S_symbol) 0) {
        sprintf(dbname, "\"%s\"", S_name(d->u.tabvar.source));
        sprintf(dbFile, "%s", S_name(d->u.tabvar.source));
      } else {
        sprintf(dbname, "\"_adl_db_%s\"", S_name(d->name));
        sprintf(dbFile, "_adl_db_%s", S_name(d->name));
      }

      if ((isESL() || isAdHoc() || isESLAggr()) && d->u.tabvar.scope
          == TAB_MEMORY && !inaggr) {
        //create the in_mem_table and put in the hash
        int rt;
        if ((rt = im_rel_create(&inMem, NULL, IM_LINKEDLIST,
            d->u.tabvar.index ? IM_REL_INDEXED : 0)) != 0) {
          rc = ERR_INMEM_TABLE;
          EM_error(d->pos, rc, __LINE__, __FILE__, S_name(d->name));
          goto exit;
        }

        if ((rt = inMem->open(inMem, dbFile, 0)) != 0) {
          rc = ERR_INMEM_TABLE;
          EM_error(d->pos, rc, __LINE__, __FILE__, S_name(d->name));
          goto exit;
        }
        //printf("we here putting %s\n", S_name(d->name));fflush(stdout);
        if (getInMemTables()->count(S_name(d->name)) == 0) {
          getInMemTables()->operator[](strdup(S_name(d->name))) = inMem;
        }
      }

      if (isESL() && !inaggr) {
        bufferMngr* bm = bufferMngr::getInstance();
        buffer *b = bm->lookup("_ioBuffer");
        //printf("get here\n");
        //fflush(stdout);
        b->put(ADDED_BUFFER, S_name(d->name));
        int index = 1;
        if (d->u.tabvar.index != (A_index) 0)
          index = d->u.tabvar.index->kind;
        if (index == INDEX_BTREE) {
          //_ADL_WIN_ROW is just a default parameter, it does not mean anything
          bm->create(S_name(d->name), BUF_BTREE, 0, _ADL_WIN_ROW, dbFile);
        } else {
          bm->create(S_name(d->name), BUF_RECNO, 0, _ADL_WIN_ROW, dbFile);
        }
      }

      if (inaggr) {
        char iname[80];
        /* local variables of aggregate routine are stored in a structure */
        sprintf(iname, "status->%s", S_name(d->name));
        var->u.var.iname = new_Symbol(iname);
        var->u.var.inaggr = 1;
      }
      /* check index type */
      rc = checkTabIndex(venv, d, ty, var);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec",
            "checkTabIndex");
        goto exit;
      }
      if (index != (A_index) 0)
        var->u.var.firstKey = d->u.tabvar.index->keyPos[0];
      else
        var->u.var.firstKey = 0; // not completely sure, may be we need 0
      S_enter(venv, d->name, var);

      /* fill in reference types */
      rc = transTyRefType(venv, d->u.tabvar.ty, ty);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec",
            "transTyRefType");
        goto exit;
      }

      strcpy(handlename, S_name(d->name));

      /*printf ("\nTEST!!!! kind = %d\n", var->u.var.scope);*/
      transTabDec2C(handlename, code, var);

      edec = expTy_Seq(edec, code);

      if (inaggr) {
        /* the table vars are local tables in an aggregate. we delay opening local tables */
        d->u.tabvar.index = var->u.var.index;
      } else {
        /* get first key's data type, used in SET BTREE Comprison function */
        ty_t keyType = Ty_int;
        if (var->u.var.index != 0 && var->u.var.index->kind == INDEX_BTREE) {
          /* get key type */
          A_list fields = var->u.var.ty->u.record;
          for (int i = 0; i < fields->length; i++) {
            Ty_field f = (Ty_field) getNthElementList(fields, i);
            if (f->iskey) {
              keyType = f->ty->kind;
              break;
            }
          }
        }
        tabindex_t index = (tabindex_t) 1;
        if (var->u.var.index != (A_index) 0)
          index = var->u.var.index->kind;
        transTabInit2C(handlename, dbname, var->u.var.haskey, code,
            var->u.var.scope, index, keyType, inaggr, var->u.var.isBuffer,
            d->name);

        einit = expTy_Seq(einit, code);
      }

      /* put table in the global array of memory tables */
      if (d->u.tabvar.scope == TAB_MEMORY && !inaggr && !(isESL()
          || isESLAggr() || isAdHoc())) {
        sprintf(code, "\nif (inMemTables->count(\"%s\") == 0) {"
          "\ninMemTables->operator[](strdup(\"%s\")) = %s;"
          "\n}", handlename, handlename, handlename);
        einit = expTy_Seq(einit, code);
      }

      if (d->u.tabvar.init) {
        A_qun target = A_NameQun(d->pos, d->name, d->name);
        A_qun source = A_QueryQun(d->u.tabvar.init->pos, 0, d->u.tabvar.init);
        A_list jtl = A_List(0, A_QUN);
        A_exp initopr;
        T_expty curdec, curexe;

        Sql_sem sql = SqlSem();
        if (inaggr)
          sql->in_func = 1;

        appendElementList(jtl, (nt_obj_t*) target);
        appendElementList(jtl, (nt_obj_t*) source);

        initopr = A_SqlOprExp(d->pos, A_SQL_INSERT, 0, (A_list) 0, jtl,
            (A_list) 0);

        rc = transExp(venv, tenv, initopr, sql, curdec, curexe, aggregates);
        if (rc) {
          EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transExp");
          goto exit;
        }
        /*
         For UDA/UDF, we do not realize each sql statement with a C function.
         */
        if (sql->in_func == 1) {
          einit = expTy_Seq(einit, curdec);
        } else {
          edec = expTy_Seq(edec, curdec);
        }
        einit = expTy_Seq(einit, curexe);

        SqlSem_Delete(sql);
      }

      tabindex_t index = (tabindex_t) 1;
      if (var->u.var.index != (A_index) 0)
        index = var->u.var.index->kind;
      transTabClose2C(handlename, dbname, code, var->u.var.scope, index);
      eclean = expTy_Seq(eclean, code);
    }
  }
    break;
  case A_dynamicVarDec: {
    //make sure the dynamic table exists and make the entry
    E_enventry te = (E_enventry) S_look(venv, d->u.dynamic.table);
    if (!te || te->kind != E_varEntry) {
      rc = ERR_UNDEFINED_VARIABLE;
      EM_error(d->pos, rc, __LINE__, __FILE__, S_name(d->u.dynamic.table));
      goto exit;
    }

    S_enter(venv, d->name, E_DynamicEntry(d->name, d->u.dynamic.table,
        d->u.dynamic.rawname));
  }
    break;
  case A_streamVarDec: {
    if (!isESL()) {
      EM_error(0, ERR_SYNTAX, __LINE__, __FILE__,
          "Stream declaration not allowed in atlas, only allowed in StreamMill");
      goto exit;
    }
    Ty_ty ty;
    char handlename[80];
    E_enventry var;
    E_enventry outOfOrder;
    int foundref = 0; // is any of the columns of ref type?
    int nKeyColumns = 0;

    // compile tuple type
    rc = transTy(venv, tenv, d->u.streamvar.ty, ty, reflist, &foundref,
        A_streamVarDec);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "transTy");
      goto exit;
    }
    if (foundref == 1) {
      /* only in-memory table allows ref type */
      rc = ERR_REF_MEMORY_ONLY;
      EM_error(d->pos, rc, __LINE__, __FILE__, S_name(d->name));
      goto exit;
    }

    //printf("here stream, timekey type %s, %d\n", S_name(d->name), d->u.streamvar.tk);
    var = E_StreamEntry(d->name, ty, 0, d->u.streamvar.source,
        d->u.streamvar.target, d->u.streamvar.tk, d->u.streamvar.timekey,
        d->u.streamvar.isBuiltin, d->u.streamvar.port);

    if (d->u.streamvar.tk == tk_external) {
      char ooStream[256];
      buffer * ooBuf;

      sprintf(ooStream, "%s_outOfOrder", S_name(d->name));
      ooBuf = bm->lookup(ooStream);
      if (!ooBuf) {
        bm->create(ooStream, SHARED);
      }

      outOfOrder = E_StreamEntry(S_Symbol(ooStream), ty, 0,
          d->u.streamvar.source, d->u.streamvar.target, tk_none);

      S_enter(venv, S_Symbol(ooStream), outOfOrder);
    }
    if (inaggr) {
      EM_error(0, ERR_SYNTAX, __LINE__, __FILE__,
          "Stream declaration not allowed in aggregate");
      goto exit;
    }
    S_enter(venv, d->name, var);

    /* check time key */
    rc = checkTimeKey(d, ty, var);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec", "checkTimeKey");
      goto exit;
    }
    strcpy(handlename, S_name(d->name));
    rc = transStreamDec2C(venv, handlename, code, var);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transDec",
          "transStreamDec2C");
      goto exit;
    }

    edec = expTy_Seq(edec, code);

    strcpy(code, "");
    einit = expTy_Seq(einit, code);

    //transStreamClose2C(handlename, code);
    eclean = expTy_Seq(eclean, "");

    if (d->u.streamvar.target != NULL) {
      char *target = strdup(S_name(d->u.streamvar.target));
      char *machine = strsep(&target, ":");
      char *port = target;
      int portNum = 0;
      int code = ADD_TARGET_STREAM;

      if (port)
        portNum = atoi(port);

      if (port == NULL || machine == NULL || portNum < 1024 || portNum > 65536) {
        rc = ERR_STREAM_TARGET_SPEC;
        EM_error(0, rc, __LINE__, __FILE__, S_name(d->name));
        goto exit;
      }

      //send message to IOS to add buffer as outBuf
      int size = sizeof(int);
      size = size + strlen(S_name(d->name)) + 1;
      size = size + strlen(machine);
      size = size + sizeof(int);
      size = size + strlen(getUserName()) + 1;

      cDBT cdbt(size);
      memcpy(cdbt.data, &code, sizeof(int));
      strcpy(cdbt.data + sizeof(int), S_name(d->name));
      strcpy(cdbt.data + sizeof(int) + strlen(S_name(d->name)) + 1, machine);
      memcpy((char*) cdbt.data + sizeof(int) + strlen(S_name(d->name)) + 1
          + strlen(machine) + 1, &portNum, sizeof(int));
      strcpy(cdbt.data + sizeof(int) + strlen(S_name(d->name)) + 1 + strlen(
          machine) + 1 + sizeof(int), getUserName());
      bufferMngr *bm = bufferMngr::getInstance();
      buffer* ioBuf = bm->lookup("_ioBuffer");
      ioBuf->put(&cdbt);
    }

  }
    break;
  case A_varDec: {
    //struct expty e = transExp(venv, tenv, d->u.var.init);

    //      S_enter(venv, d->u.var.var, E_VarEntry(d->u.var.var, e.ty, 0));
  }
    break;
  case A_typeDec: {
    /*      struct expty e = transExp(venv, tenv, d->u.var.init);
     A_namety nt = (A_namety)getNthElementList(d->u.type, 0);
     S_enter(tenv, nt->name, transTy(tenv, nt->ty));
     */
  }
    break;
  }
  exit:
  //printf("done with seqdec\n");
  DBUG_RETURN(rc);
  return rc;
}

bool setQueryName(const char* s) {
  if (!strcmp(s, "__test__"))
    return strlen(queryName) != 0;
  else {
    strcpy(queryName, s);
    resetFirstEntryFlag();
  }
}

char* getQueryName() {
  return queryName;
}

void setIsESLAggr(int value) {
  isESLAgg = value;
}

int isESLAggr() {
  return isESLAgg;
}

void setAggrName(const char* name) {
  strcpy(eslAggrName, name);
}

char* getAggrName() {
  return eslAggrName;
}

void setAdHoc(int value) {
  adHoc = value;
}

int isAdHoc() {
  return adHoc;
}

void setAdHocNum(int value) {
  adHocNum = value;
}

int getAdHocNum() {
  return adHocNum;
}

void resetFirstEntryFlag() {
  memset(firstEntryFlag, 0, sizeof(firstEntryFlag));
}
bool isFirstEntryDec(int i) {
  return firstEntryFlag[i];
}

void setFirstEntryFlag(int i) {
  firstEntryFlag[i] = true;
}
void setESLTestFlag(bool flag) {
  ESLTestFlag = flag;
}
bool isESLTest() {
  return ESLTestFlag;
}
;
