#include <sql/adl_sys.h>
#include <stdio.h>
#include <string.h>
#include <sql/trans2C.h>
#include <sql/semant.h>
#include <sql/types.h>
#include <sql/env.h>
#include <sql/err.h>
#include <sql/io.h>
#include <sql/const.h>
#include <stdio.h>
extern "C" {
#include <dbug/dbug.h>
}
#include "util.h"
#include <sql/list.h>
#include <vector>
#include <limits.h>
using namespace std;
#include <buffer.h>
#include <iostream>

#include <ios/ios.h>
#include <esl/querySchdl.h>
#include <SMLog.h>
using namespace ESL;
extern system_t *ntsys;

static int c_indent = 0;

A_list global_functions;

typedef enum {
} tr_exp_t;

struct Tr_exp_ {
  tr_exp_t kind;

  union {
    A_list rec;
    char *code;
  } u;
};

char *recs[] = { "x_ul", "y_ul", "x_lr", "y_lr" };
char *dataTypes[] = { "record", "null", "int", "double", "char*" };

Tr_exp Tr_Seq(Tr_exp seq, Tr_exp ele) {
  Tr_exp result;

  if (seq == (Tr_exp) 0) {
    result = ele;
  } else {
    int len = strlen(seq) + strlen(ele) + 1;
    //      result = (Tr_exp) ntRealloc(seq, len);
    result = (Tr_exp) realloc(seq, len);
    strcat(result, ele);
    //      Tr_Delete(ele);
  }
  return result;
}
void Tr_Delete(Tr_exp ele) {
  //    ntFree(ele);
  free(ele);
}
Tr_exp Tr_String(char *s) {
  Tr_exp e;

  //    e = (Tr_exp)ntMalloc(strlen(s)+1);
  e = (Tr_exp) malloc(strlen(s) + 1);
  strcpy((char*) e, s);

  return e;
}
Tr_exp Tr_QuoteString(char *s) {
  Tr_exp e;
  e = (Tr_exp) malloc(strlen(s) + 3);
  sprintf((char*) e, "\"%s\"", s);
  return e;
}

Tr_exp Tr_Int(int n) {
  char s[16];
  sprintf(s, "%d", n);
  return Tr_String(s);
}
Tr_exp Tr_Real(double r) {
  char s[16];
  sprintf(s, "%f", r);
  return Tr_String(s);
}
Tr_exp Tr_Timestamp(struct timeval* tv) {
  char date[40];
  sprintf(date, "A_Timeval(%d, %d)", tv->tv_sec, tv->tv_usec);
  return Tr_String(date);
}

void cWrite(nt_obj_t *stream, char *str) {
  char buf[2048], *p = buf, *s = str;
  int i;

  while (*s) {
    if (*s == '\n') {
      *p++ = *s++;
      for (i = 0; i < c_indent; i++) {
        *p++ = ' ';
        *p++ = ' ';
        *p++ = ' ';
      }
      if (*s == '}' && c_indent > 0)
        p -= 3;
    } else {
      if (*s == '{')
        c_indent++;
      if (*s == '}')
        c_indent--;
      *p++ = *s++;
    }

    if (p - buf >= 1024) {
      *p = '\0';
      ntWriteStr(stream, buf);
      p = buf;
    }
  }
  *p = '\0';
  ntWriteStr(stream, buf);
}

err_t trans2C(S_table venv, S_table tenv, A_exp e, char *filename) {
  SMLog::SMLOG(10, "Entering trans2C: filename-> %s", filename);
  err_t rc = ERR_NONE;
  int i;
  char buf[MAX_STR_LEN];
  char temp_buf[255];

  A_list dec_list;
  nt_obj_t *out = makeStream(O_NUM, filename, 0);
  T_expty dec0 = (T_expty) 0;
  T_expty dec1 = (T_expty) 0;
  T_expty exe0 = (T_expty) 0;
  T_expty exe1 = (T_expty) 0;
  T_expty cln = (T_expty) 0;

  exe1 = expTy((Tr_exp) 0, Ty_Nil());

  vector<void*> aggregates;
  Sql_sem sql = (Sql_sem) 0;

  if (out == (nt_obj_t*) 0) {
    rc = ERR_OPEN_FILE;
    EM_error(0, rc, __LINE__, __FILE__, filename);
    goto exit;
  }

  global_functions = A_List(0, A_EXP);

  /* compile the declaration part */
  SMLog::SMLOG(12, "\ttrans2C::Start to compile the declaration part");
  if (!A_ListEmpty(e->u.let.decs)) {
    dec_list = e->u.let.decs;
    T_expty adec = (T_expty) 0;

    rc
        = transSeqDec(venv, tenv, dec_list, dec0, exe0, cln, 0, adec,
            aggregates);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "trans2C", "transSeqDec");
      goto exit;
    }
  }

  /* compile the query part */
  SMLog::SMLOG(12, "\ttrans2C::Start to compile the query part");
  if (e->u.let.body) {
    sql = SqlSem();
    rc = transExp(venv, tenv, e->u.let.body, sql, dec1, exe1, aggregates);

    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "trans2C", "transExp");
      goto exit;
    }
  }

  SMLog::SMLOG(12, "\ttrans2C::writing include part to the file");
  /* generate code 1. head files and global variables */
  cWrite(out, "#include <math.h>");
  cWrite(out, "\n#include <sys/types.h>");
  cWrite(out, "\n#include <stdio.h>");
  cWrite(out, "\n#include <stdlib.h>");
  cWrite(out, "\n#include <math.h>");
  cWrite(out, "\n#include <db.h>");
  cWrite(out, "\n#include <unistd.h>");
  cWrite(out, "\n#include <string.h>");
  cWrite(out, "\n#include <dlfcn.h>");
  cWrite(out, "\n#include <rtree.h>");
  cWrite(out, "\nextern \"C\"{\n#include <im_db.h>");
  cWrite(out, "\n#include \"swimlib.h\"");
  cWrite(out, "\n#include \"fptree.h\"");
  cWrite(out, "\n#include \"memmgr.h\"");
  cWrite(out, "\n#include \"timeval.h\"");
  cWrite(out, "\n#include \"histmgr.h\"");
  cWrite(out, "\n#include <mcheck.h>\n}");
  cWrite(out, "\n#include \"adllib.h\"");
  cWrite(out, "\n#include <ext/hash_map>");
  cWrite(out, "\n#include <winbuf.h>");
  cWrite(out, "\n#include <windowBuf.h>");

  cWrite(out, "\nusing namespace ESL;\n");
  cWrite(out, "\nusing namespace __gnu_cxx;\n");

  if (isESL() || isAdHoc()) {
    cWrite(out, "\n//////////////////// ESL headers////////////////////");
    cWrite(out, "\n#include <stmt.h>");
    cWrite(out, "\n#include <buffer.h>");
  } else if (!isESLAggr()) {
    cWrite(
        out,
        "\nstatic hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables=new hash_map<const char*, void*, hash<const char*>, eqstrTab>;");
  }

  SMLog::SMLOG(12, "\ttrans2C::writing other definitions");
  cWrite(out, "\nextern int _ADL_NULL;");
  cWrite(out, "\nextern char* _ADL_NULL_STR;");

  sprintf(temp_buf, "\n#define MAX_STR_LEN %d\n", MAX_STR_LEN);
  cWrite(out, temp_buf);

  if (ntsys->verbose) {
    cWrite(out, "\n\nint adldebug = 1;");
  } else {
    cWrite(out, "\n\nint adldebug = 0;");
  }

  cWrite(out, "\nint _adl_sqlcode, _adl_cursqlcode;");

  /* generate code 2. dec of delaration part */
  if (dec0) {
    SMLog::SMLOG(12, "\ttrans2C::generate code 2. dec of delaration part");
    cWrite(out, "\n/**** Global Declarations ****/");
    cWrite(out, (char*) dec0->exp);
    expTy_Delete(dec0);
  }

  /* generate code 3. global modules of the query part */
  if (sql && sql->global) {
    SMLog::SMLOG(12,
        "\ttrans2C::generate code 3. global modules of the query part");
    cWrite(out, (char*) sql->global->exp);
  }

  /* generate code 6. declaration of query part */
  if (dec1) {
    SMLog::SMLOG(12, "\ttrans2C::generate code 6. declaration of query part ");
    cWrite(out, "\n/**** Query Declarations ****/");
    cWrite(out, (char*) dec1->exp);
    expTy_Delete(dec1);
  }

  /* generate code 7. execution of query part */
  if (exe1 != 0) {
    SMLog::SMLOG(12, "\ttrans2C::generate code 7. execution of query part");
    cWrite(out, "\n/**** Query Execution ****/");
    cWrite(out, (char*) exe1->exp);
    expTy_Delete(exe1);
  }

  /* generate code 4. main function and initialization of lib modules */
  /* Set BTREE Comparison function for int, real */

  // Do not generate main for ESL programs
  if (!isESL() && !isAdHoc()) {
    if (!A_ListEmpty(global_functions)) {
      SMLog::SMLOG(12, "\ttrans2C::creating main() function");
      cWrite(out, "\nint main()\n{");
      cWrite(out, "\nint rc;");
      cWrite(out, "\ntempdb_init();");
      cWrite(out, "\nhashgb_init();");
      cWrite(out, "\n_adl_dlm_init();");

      // generate code 5. initialization of declaration part
      if (exe0) {
        cWrite(out, "\n// Initialization of Declarations");
        cWrite(out, (char*) exe0->exp);
        expTy_Delete(exe0);
      }

      for (i = 0; i < global_functions->length; i++) {
        int id = (int) getNthElementList(global_functions, i);
        sprintf(buf, "\n_adl_statement_%d();", id);
        cWrite(out, buf);
      }

      if (sql)
        SqlSem_Delete(sql);

      // generate code 8. clean up of lib modules
      cWrite(out, "\nexit:");
      cWrite(out, "\ntempdb_delete();");
      cWrite(out, "\n_adl_dlm_delete();");
      if (cln) {
        cWrite(out, cln->exp);
        expTy_Delete(cln);
      }
      cWrite(out, "\nreturn(rc);");
      cWrite(out, "\n};\n");

    }
  } // end if not ESL
  clearList(global_functions);
  deleteList(global_functions);
  exit: if (out)
    closeStream(out);
  SMLog::SMLOG(10, ">Exiting trans2C");
  return rc;
}

err_t Dereference2C(Ty_ty f, char *refbuf) {
  SMLog::SMLOG(10, "Entering Dereference2C");
  err_t rc = ERR_NONE;

  switch (f->kind) {
  case Ty_int:
    strcpy(refbuf, "*(int*)");
    break;
  case Ty_real:
    strcpy(refbuf, "*(double*)");
    break;
  case Ty_string:
    strcpy(refbuf, "(char*)");
    break;
  case Ty_timestamp:
    strcpy(refbuf, "(struct timeval *)");
    break;
  case Ty_iext:
    strcpy(refbuf, "(struct iExt_ *)");
    break;
  case Ty_rext:
    strcpy(refbuf, "(struct rExt_ *)");
    break;
  case Ty_cext:
    strcpy(refbuf, "(struct cExt_ *)");
    break;
  case Ty_text:
    strcpy(refbuf, "(struct tExt_ *)");
    break;
  case Ty_ref:
    /* im_db type */
    strcpy(refbuf, "(int)*(TLL**)");
    break;
  case Ty_record:
  default:
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "Unsupported type in Dereference2C()");
    goto exit;
  }

  exit: return rc;
}
err_t Ty2C(Ty_ty f, char *cdef) {
  err_t rc = ERR_NONE;

  switch (f->kind) {
  case Ty_int:
    strcpy(cdef, "int");
    break;
  case Ty_real:
    strcpy(cdef, "double");
    break;
  case Ty_string:
    strcpy(cdef, "char*");
    break;
  case Ty_timestamp:
    strcpy(cdef, "struct timeval");
    break;
  case Ty_iext:
    strcpy(cdef, "struct iExt_");
    break;
  case Ty_rext:
    strcpy(cdef, "struct rExt_");
    break;
  case Ty_cext:
    strcpy(cdef, "struct cExt_");
    break;
  case Ty_text:
    strcpy(cdef, "struct tExt_");
    break;
  case Ty_record:
    /* table function : return error code 'rc' */
    strcpy(cdef, "int");
    break;
  case Ty_ref:
    strcpy(cdef, "int");
    break;
  default:
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "Unsupported type in Ty2C()");
    goto exit;
  }

  exit: return rc;
}

err_t TyName2C(Ty_field f, char *cdef) {
  err_t rc = ERR_NONE;

  rc = Ty2C(f->ty, cdef);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "TyName2C", "Ty2C");
    goto exit;
  }
  strcat(cdef, " ");
  strcat(cdef, S_name(f->name));

  exit: return rc;
}

err_t TyList2C(A_list tylist, char* cdef) {
  err_t rc = ERR_NONE;

  *cdef = '\0';
  for (int i = 0; i < tylist->length; i++) {
    char argdef[128];
    Ty_field f = (Ty_field) getNthElementList(tylist, i);
    rc = TyName2C(f, argdef);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "TyList2C", "TyName2C");
      goto exit;
    }
    if (i > 0)
      strcat(cdef, ", ");
    strcat(cdef, argdef);
  }

  exit: return rc;
}

err_t transCursorUpdate2C(char *name, char *buf) {
  err_t rc = ERR_NONE;

  /* no need to compare if it's imdb for now
   * since the API is exactly the same
   */
  /* if (kind == TAB_MEMORY) */

  //sprintf(buf, "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
  //	  "\nadlabort(rc, \"IM_RELC->c_put() or DBC->c_put()\");"
  //  "\n}"
  //  ,name, name);

  if (isESL()) {
    sprintf(
        buf,
        "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_RELC->c_put() or DBC->c_put()\");"
          "\nreturn s_failure;"
          "\n}", name, name, getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        buf,
        "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_RELC->c_put() or DBC->c_put()\");"
          "\nreturn;"
          "\n}", name, name, getUserName(), getAggrName());
  } else {
    sprintf(buf, "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
      "\nadlabort(rc, \"IM_RELC->c_put() or DBC->c_put()\");"
      "\n}", name, name);
  }

  /* else
   {
   sprintf(buf, "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
   "\nadlabort(rc, \"DBC->c_put()\");"
   "\n}"
   ,name, name);
   } */
  exit: return rc;
}

err_t transCursorDec2C(char *handlename, char *buf, tabscope_t kind,
    tabindex_t idx) {
  err_t rc = ERR_NONE;

  if (kind == TAB_WINDOW || kind == TAB_EXPIRED)
    sprintf(buf, "\nIM_RELC *%s;", handlename);
  else if (kind == TAB_MEMORY)
    sprintf(buf, "\nIM_RELC *%s;", handlename);
  else if (idx == INDEX_RTREE)
    sprintf(buf, "\nRTreeCursor *%s;", handlename);
  else
    sprintf(buf, "\nDBC *%s;", handlename);
  return rc;
}

err_t transCursorInit2C(char *handlename, char *cursorname, char *buf,
    tabscope_t kind, tabindex_t idx, int isView) {
  DBUG_ENTER("transCursorInit");
  err_t rc = ERR_NONE;
  char tmp[MAX_STR_LEN];

  if (isView == 1) {
    sprintf(buf, "\nif(%s == NULL) {"
      "\n%s = (windowBuf*)bm->lookup(\"%s\");"
      "\nif(%s == NULL) {", handlename, handlename, handlename, handlename);
    if (isESL()) {
      sprintf(
          tmp,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: bm->lookup(), windowBuf not found\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          tmp,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: bm->lookup(), windowBuf not found\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(tmp, "\nadlabort(rc, \"bm->lookup(), windowBuf not found\");"
        "\n}");
    }
    strcat(buf, tmp);

    sprintf(tmp, "\n}"
      "\nif((rc = %s->get_im_rel()->cursor(%s->get_im_rel(),&%s,0))!=0){",
        handlename, handlename, cursorname);
    strcat(buf, tmp);

    if (isESL()) {
      sprintf(
          tmp,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->cursor()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          tmp,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->cursor()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(tmp, "\nadlabort(rc, \"IM_REL->cursor()\");"
        "\n}");
    }
    strcat(buf, tmp);

  }
  /* see if it's using in-memory, R-Tree or berkeley db (B-Tree) table */
  else if (kind == TAB_WINDOW) {
    sprintf(buf, "\nif ((rc = window->cursor(window, &%s, 0)) != 0) {",
        cursorname);

    if (isESL()) {
      sprintf(
          tmp,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: WINDOW->cursor()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          tmp,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: WINDOW->cursor()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(tmp, "\nadlabort(rc, \"WINDOW->cursor()\");"
        "\n}");
    }
    strcat(buf, tmp);
  } else if (kind == TAB_EXPIRED) {
    sprintf(buf, "\nif ((rc = _adl_winCursor(status->win, &%s, 1)) != 0) {",
        cursorname);

    if (isESL()) {
      sprintf(
          tmp,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: WINDOW->cursor()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          tmp,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: WINDOW->cursor()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(tmp, "\nadlabort(rc, \"WINDOW->cursor()\");"
        "\n}");
    }
    strcat(buf, tmp);
  } else if (kind == TAB_MEMORY) {
    sprintf(buf, "\nif ((rc = %s->cursor(%s, &%s, 0)) != 0) {", handlename,
        handlename, cursorname);

    if (isESL()) {
      sprintf(
          tmp,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->cursor()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          tmp,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->cursor()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(tmp, "\nadlabort(rc, \"IM_REL->cursor()\");"
        "\n}");
    }
    strcat(buf, tmp);
  } else if (idx == INDEX_RTREE) {
    sprintf(buf, "\nif ((rc = %s->cursor(%s, &%s, 0)) != 0) {", handlename,
        handlename, cursorname);

    if (isESL()) {
      sprintf(
          tmp,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: RTREE->cursor()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          tmp,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: RTREE->cursor()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(tmp, "\nadlabort(rc, \"RTREE->cursor()\");"
        "\n}");
    }
    strcat(buf, tmp);
  } else {
    if (isESL()) {
      sprintf(buf, "\n//%s = bm->lookup((char*)\"%s\");"
        "\nif ((rc = %s->cursor(%s, NULL, &%s, 0)) != 0) {", handlename,
          handlename, handlename, handlename, cursorname);

      if (isESL()) {
        sprintf(
            tmp,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DB->cursor()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            tmp,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DB->cursor()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(tmp, "\nadlabort(rc, \"DB->cursor()\");"
          "\n}");
      }
      strcat(buf, tmp);
    } else {
      sprintf(buf, "\nif ((rc = %s->cursor(%s, NULL, &%s, 0)) != 0) {",
          handlename, handlename, cursorname);

      if (isESL()) {
        sprintf(
            tmp,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DB->cursor()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            tmp,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DB->cursor()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(tmp, "\nadlabort(rc, \"DB->cursor()\");"
          "\n}");
      }
      strcat(buf, tmp);
    }
  }
  DBUG_RETURN(rc);
  return rc;
}

//extern int ESL::NUM_TUPS_PER_CALL;

err_t transStreamGet2C(E_enventry x, char* buf) {
  SMLog::SMLOG(10, "Entering transStreamGet2C");
  DBUG_ENTER("transStreamGet2C");
  char bufName[1024];
  strcpy(bufName, S_name(x->u.stream.sname));
  sprintf(buf, "\nif (input_count == %d) {"
    "\nif (output_count == 0) {"
    "\nrc = s_no_output;"
    "\ngoto doClose;"
    "\n}"
    "\nrc = (stmt_rc)output_count;"
    "\ngoto doClose;"
    "\n}"
    "\nelse {"
    "\ncase0_visited = 1;"
    "\n}"
    "\ndata.data = datadata;"
    "\nkey.data = keydata;"
    "\nbuffer *%s = bm->lookup(\"%s\");"
    "\nif (%s && input_count>0 && %s->bufSize() > 1) {"
    "\n%s->pop();"
    "\n}"
    "\nelse if(%s && input_count>0) { /*if size <= 1 then exit for now*/"
    "\nrc = (input_count>0)?(stmt_rc)output_count:s_no_input;"
    "\ngoto doClose;"
    "\n}"
    "\nif (!%s || ((rc=%s->get(&data, &atime, &key)) == DB_NOTFOUND)){"
    "\nif (FLAGS_enable_heartbeat) {"
    "\natime.tv_sec = 0;"
    "\nrc = 0;"
    "\n} else {"
    "\nrc = (input_count>0)?(stmt_rc)output_count:s_no_input;"
    "\ngoto doClose;"
    "\n}"
    "\n}"
    "\ninput_count++;", 1, //NUM_TUPS_PER_CALL,
      bufName, bufName, bufName, bufName, bufName, bufName, bufName, bufName);
  DBUG_RETURN(ERR_NONE);
  return ERR_NONE;
}
;

err_t transCursorGet2C(S_table venv, char *first_entry_name, char *cursorname,
    E_enventry x, Tr_exp *bndp, // bindings of the keys, for equality (e.g. a=1)
    Tr_exp *bndpUpper, // bindings of the keys, for upper bound (e.g. a < 1)
    Tr_exp *bndpLower, // bindings of the keys, for lower bound (e.g. a > 1)
    int flag, // binding flag
    char *buf // OUT
) {
  SMLog::SMLOG(10,
      "Entering transCursorGet2C first_entry_name: %s, cursorname: %s",
      first_entry_name, cursorname);
  err_t rc = ERR_NONE;
  A_list fields;
  int i, offset;
  char linebuf[MAX_STR_LEN];

  if (bndp == (Tr_exp*) 0 && bndpUpper == (Tr_exp*) 0 && bndpLower
      == (Tr_exp*) 0) {
    // no binding
    if (x && x->u.var.index && x->u.var.index->kind == INDEX_RTREE) {
      sprintf(buf, "\nmemset(&key, 0, sizeof(key));"
        "\nmemset(&data, 0, sizeof(data));"
        "\nkey.size = sizeof(Rect);"
        "\nkey.data = &r_key;"
        "\nrc = %s->c_get(%s, &key, &data, (%s)? DB_FIRST:DB_NEXT);",
          cursorname, cursorname, first_entry_name);
    } else {
      sprintf(buf, "\nmemset(&key, 0, sizeof(key));"
        "\nmemset(&data, 0, sizeof(data));"
        "\nrc = %s->c_get(%s, &key, &data, (%s)? DB_FIRST:DB_NEXT);",
          cursorname, cursorname, first_entry_name);
    }

  } else {
    // use binding
    Ty_ty ty = x->u.var.ty;
    tabindex_t idx = (x->u.var.index == (A_index) 0) ? (tabindex_t) 1
        : x->u.var.index->kind;
    int firstKey = x->u.var.firstKey;

    if (ty->kind != Ty_record) {
      rc = ERR_TUPLE_TYPE_REQUIRED;
      EM_error(0, rc, __LINE__, __FILE__, "transCursorGet2C");
      goto exit;
    }

    /* if (kind == TAB_MEMORY)
     sprintf(buf,
     "\nmemset(&key, 0, sizeof(key));"
     "\nmemset(&data, 0, sizeof(data));"
     "\nkey.data = keydata;"
     , first_entry_name);
     else*/
    fields = ty->u.record;

    int keycnt = 0;
    for (i = 0; i < fields->length; i++) {
      Ty_field f = (Ty_field) getNthElementList(fields, i);
      if (f->iskey) {
        keycnt++;
      }
    }

    switch (idx) {
    case INDEX_RTREE:
      sprintf(buf, "\nint flag%s= (%s)? DB_SET_RANGE : DB_NEXT_RANGE;"
        "\nmemset(&key, 0, sizeof(key));"
        "\nmemset(&data, 0, sizeof(data));"
        "\nkey.data = keydata;"
        "\ndata.data = datadata;", first_entry_name, first_entry_name);

      break;
    default: // BTREE
      sprintf(buf, "\nmemset(&key, 0, sizeof(key));"
        "\nmemset(&data, 0, sizeof(data));"
        "\nmemset(keydata, 0, sizeof(keydata));"
        "\nkey.data = keydata;"
        "\ndata.data = datadata;");
      if (x->u.var.scope == TAB_MEMORY || (x->u.var.index
          && x->u.var.index->func != (S_symbol) 0)) {
        //Hetal: index on memory tables
        //only used if bndp specified (exact value)
        if (flag == BOUND_ALL) /* equality bound all keys */
          sprintf(linebuf, "\nint flag%s= (%s)? DB_SET : DB_NEXT_DUP;",
              first_entry_name, first_entry_name);
        else
          sprintf(linebuf, "\nint flag%s= (%s)? DB_FIRST : DB_NEXT;",
              first_entry_name, first_entry_name);
        strcat(buf, linebuf);
      } else if (flag == BOUND_ALL) {
        sprintf(linebuf, "\nint flag%s= (%s)? DB_SET : DB_NEXT_DUP;",
            first_entry_name, first_entry_name);
        strcat(buf, linebuf);
      } else { //bound_first key
        if (bndpLower != (Tr_exp*) 0 || bndp != (Tr_exp*) 0) /* we have lower bound, use DB_SET_RANGE
         to locate the first tuple
         use DB_NEXT to locate subsequent tuples
         */
          sprintf(linebuf, "\nint flag%s= (%s)? DB_SET_RANGE : DB_NEXT;",
              first_entry_name, first_entry_name);
        else
          /* Both bndp and bndpLower are 0, we only have upper bound */
          sprintf(linebuf, "\nint flag%s= (%s)? DB_FIRST : DB_NEXT;",
              first_entry_name, first_entry_name);
        strcat(buf, linebuf);
      }
      break;
    }// end switch idx


    offset = 0;
    int rkeyIdx = 0; /* used to assign values to rtree key (r_key), and check
     whether rtree key is point or rectangle */
    int keyType = 0;
    for (i = 0; i < fields->length; i++) {
      Ty_field f = (Ty_field) getNthElementList(fields, i);
      if (f->iskey) {
        if (x && x->u.var.index && x->u.var.index->kind == INDEX_RTREE) {
          /* Richard 2003/2:
           Right now we only support integer index
           */
          switch (f->ty->kind) {
          case Ty_int:
            switch (rkeyIdx) {
            case 0:
              if (bndpLower[i] == (Tr_exp) 0) { // x no lower bound
                sprintf(linebuf, "\nr_key.x_ul = %ld;", INT_MIN);
              } else {
                sprintf(linebuf, "\nr_key.x_ul = %s;", bndpLower[i]);
              }
              strcat(buf, linebuf);
              /* In case 0 and 1, we also need to take care of the other point
               of the rectangle (which is case 3 and 4),
               in case that index is point instead of Rect
               */

              if (bndpUpper[i] == (Tr_exp) 0) { // x no upper bound
                sprintf(linebuf, "\nr_key.x_lr = %ld;", INT_MAX);
              } else {
                sprintf(linebuf, "\nr_key.x_lr = %s;", bndpUpper[i]);
              }

              break;
            case 1:
              /* In case 0 and 1, we also need to take care of the other point
               of the rectangle (which is case 3 and 4),
               in case that index is point instead of Rect
               */
              if (bndpLower[i] == (Tr_exp) 0) { // y no lower bound
                sprintf(linebuf, "\nr_key.y_lr = %ld;", INT_MIN);
              } else {
                sprintf(linebuf, "\nr_key.y_lr = %s;", bndpLower[i]);
              }
              strcat(buf, linebuf);
              if (bndpUpper[i] == (Tr_exp) 0) { // y no upper bound
                sprintf(linebuf, "\nr_key.y_ul = %ld;", INT_MAX);
              } else {
                sprintf(linebuf, "\nr_key.y_ul = %s;", bndpUpper[i]);
              }

              break;
            case 2:
              if (bndpUpper[i] == (Tr_exp) 0) // x_lr no upper bound
                sprintf(linebuf, "\nr_key.x_lr = %ld;", INT_MAX);
              else
                sprintf(linebuf, "\nr_key.x_lr = %s;", bndpUpper[i]);

              break;
            case 3:
              if (bndpLower[i] == (Tr_exp) 0) // y_lr no lower bound
                sprintf(linebuf, "\nr_key.y_lr = %ld;", INT_MIN);
              else
                sprintf(linebuf, "\nr_key.y_lr = %s;", bndpLower[i]);

              break;
            default:
              rc = ERR_NTSQL_INTERNAL;
              EM_error(0, rc, __LINE__, __FILE__,
                  "transCursorGet2C:RTREE key index");
              goto exit;
              break;
            } // end switch rkeyIdx
            rkeyIdx++;
            break;
          default:
            rc = ERR_NTSQL_INTERNAL;
            EM_error(0, rc, __LINE__, __FILE__,
                "transCursorGet2C:RTREE index:Only support interger");
            goto exit;
          } // end switch
        } else { // BTREE
          if (flag == BOUND_ALL) { // equality bound all key
            switch (f->ty->kind) {
            case Ty_real:
              if (x->u.var.index && x->u.var.index->func != (S_symbol) 0) {
                //put data in data.data temporariliy
                //we use this later to construct actual key with ext function
                //no need to do offset, already done
                sprintf(linebuf, "\n*(double*)((char*)data.data+%d) = %s;",
                    offset, bndp[i]);
              } else {
                sprintf(linebuf, "\n*(double*)((char*)key.data+%d) = %s;",
                    offset, bndp[i]);
                offset += sizeof(double);
              }
              break;
            case Ty_int:
            case Ty_ref:
              if (x->u.var.index && x->u.var.index->func != (S_symbol) 0) {
                //put data in data.data temporariliy
                //we use this later to construct actual key with ext function
                //no need to do offset, already done
                sprintf(linebuf, "\n*(int*)((char*)data.data+%d) = %s;",
                    offset, bndp[i]);
              } else {
                sprintf(linebuf, "\n*(int*)((char*)key.data+%d) = %s;", offset,
                    bndp[i]);
                offset += sizeof(int);
              }
              break;
            case Ty_string:
              if (x->u.var.index && x->u.var.index->func != (S_symbol) 0) {
                //put data in data.data temporariliy
                //we use this later to construct actual key with ext function
                //no need to do offset, already done
                sprintf(linebuf, "\nmemset((char*)data.data+%d, 0, %d);"
                  "\nmemcpy((char*)data.data+%d, %s, %d);", offset, f->size,
                    offset, bndp[i],
                    //bndp[i]
                    f->size);//string length
              } else {
                sprintf(linebuf, "\nmemset((char*)key.data+%d, 0, %d);"
                  "\nmemcpy((char*)key.data+%d, %s, %d);", offset, f->size,
                    offset, bndp[i],
                    //bndp[i]
                    f->size);//string length
                offset += f->size;
              }
              break;
            case Ty_timestamp:
              if (x->u.var.index && x->u.var.index->func != (S_symbol) 0) {
                //put data in data.data temporariliy
                //we use this later to construct actual key with ext function
                //no need to do offset, already done
                sprintf(linebuf, "\nmemset((char*)data.data+%d, 0, %d);"
                  "\nmemcpy((char*)data.data+%d, &%s, %d);", offset,
                    sizeof(struct timeval), offset, bndp[i],
                    sizeof(struct timeval));
              } else {
                sprintf(linebuf, "\nmemset((char*)key.data+%d, 0, %d);"
                  "\nmemcpy((char*)key.data+%d, &%s, %d);", offset,
                    sizeof(struct timeval), offset, bndp[i],
                    sizeof(struct timeval));
                offset += sizeof(struct timeval);
              }
              break;
            default:
              rc = ERR_NTSQL_INTERNAL;
              EM_error(0, rc, __LINE__, __FILE__, "transCursorGet2C");
              goto exit;
            }
          }
          //Hetal: if memory table of using ext func index then
          //BOUND_FIRST key is not usuable
          else if (!(x->u.var.scope == TAB_MEMORY || (x->u.var.index
              && x->u.var.index->func != (S_symbol) 0))) { //BOUND_FIRST key
            f = (Ty_field) getNthElementList(fields, firstKey);
            keyType = f->ty->kind;
            if (bndp == (Tr_exp*) 0 && bndpLower == (Tr_exp*) 0)
              break; //no lower bound
            switch (keyType) {
            case Ty_real:
              sprintf(linebuf, "\n*(double*)((char*)key.data) = %s;", (bndp
                  != (Tr_exp*) 0) ? bndp[firstKey] : bndpLower[firstKey]);
              offset += sizeof(double);
              break;
            case Ty_int:
            case Ty_ref:
              sprintf(linebuf, "\n*(int*)((char*)key.data) = %s;", (bndp
                  != (Tr_exp*) 0) ? bndp[firstKey] : bndpLower[firstKey]);
              offset += sizeof(int);
              break;
            case Ty_string:
              sprintf(linebuf, "\nmemset((char*)key.data, 0, %d);"
                "\nmemcpy((char*)key.data, %s, %d);", f->size, (bndp
                  != (Tr_exp*) 0) ? bndp[firstKey] : bndpLower[firstKey],
              //(bndp != (Tr_exp*)0) ? bndp[firstKey]: bndpLower[firstKey]
                  f->size);//string length
              offset += f->size;
              break;
            case Ty_timestamp:
              sprintf(linebuf, "\nmemset((char*)key.data, 0, %d);"
                "\nmemcpy((char*)key.data, &%s, %d);", sizeof(struct timeval),
                  (bndp != (Tr_exp*) 0) ? bndp[firstKey] : bndpLower[firstKey],
                  //(bndp != (Tr_exp*)0) ? bndp[firstKey]: bndpLower[firstKey]
                  sizeof(struct timeval));
              offset += sizeof(struct timeval);
              break;
            default:
              rc = ERR_NTSQL_INTERNAL;
              EM_error(0, rc, __LINE__, __FILE__, "transCursorGet2C");
              goto exit;
            }// end switch
            strcat(buf, linebuf);
            break; // we only consider the first key
          } // end else bound_first key
        }
        strcat(buf, linebuf);
      }
      //Hetal: in case when ext func is used for btree key, put data in data first
      //thus offsetting here
      if (x->u.var.index && x->u.var.index->func != (S_symbol) 0 && flag
          == BOUND_ALL) {
        switch (f->ty->kind) {
        case Ty_real:
          offset += sizeof(double);
          break;
        case Ty_int:
        case Ty_ref:
          offset += sizeof(int);
          break;
        case Ty_string:
          offset += f->size;
          break;
        case Ty_timestamp:
          offset += sizeof(struct timeval);
          break;
        default:
          rc = ERR_NTSQL_INTERNAL;
          EM_error(0, rc, __LINE__, __FILE__, "transCursorGet2C");
          goto exit;
        }
      }
    } // end for


    if (x->u.var.index && x->u.var.index->func != (S_symbol) 0 && flag
        == BOUND_ALL) {
      rc = assignFuncBtreeKey(venv, x, buf);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transCursorGet2C",
            "assignFuncBtreeKey");
        goto exit;
      }
    }

    if (idx == INDEX_RTREE) {
      /* RTREE with key binding */
      /* Richard 2003/2:
       */
      sprintf(linebuf, "\nkey.size = sizeof(Rect);"
        "\nkey.data = &r_key;"
        "\nrc = %s->c_get(%s, &key, &data, flag%s);", cursorname, cursorname,
          first_entry_name);

      strcat(buf, linebuf);
    } else { // BTREE
      if (x->u.var.index && x->u.var.index->func != (S_symbol) 0 && flag
          == BOUND_ALL) {
        if (x->u.var.index->func != (S_symbol) 0 && x->u.var.index->len > 0) {
          offset = x->u.var.index->len;
        } else if (x->u.var.index->func != (S_symbol) 0) {
          //look up func and assign its size to be equal to the key size
          E_enventry f = (E_enventry) S_look(venv, x->u.var.index->func);
          if (!f || f->kind != E_extEntry) {
            rc = ERR_INVALID_INDEX_SPEC;
            EM_error(0, rc, __LINE__, __FILE__, "External function not found");
            goto exit;
          }
          switch (f->u.ext.result->kind) {
          case Ty_int:
            offset = sizeof(int);
            break;
          case Ty_real:
            offset = sizeof(double);
            break;
          case Ty_timestamp:
            offset = sizeof(timestamp);
            break;
          case Ty_string:
            offset = f->u.ext.size;
            break;
          default:
            offset = sizeof(int); //should not happen
            break;
          }
        }
      }
      sprintf(linebuf, "\nkey.size = %d;"
        "\nrc = %s->c_get(%s, &key, &data, flag%s);", offset, cursorname,
          cursorname, first_entry_name);

      strcat(buf, linebuf);
      if (flag == BOUND_FIRST && (bndpUpper != (Tr_exp*) 0 || bndp
          != (Tr_exp*) 0) && x->u.var.scope != TAB_MEMORY) { // use upper bound to stop early
        sprintf(linebuf, "\nif (rc==0) {/* use upper bound to stop early */");
        strcat(buf, linebuf);
        switch (keyType) {
        case Ty_real:
          sprintf(linebuf, "\nif (*(double*)((char*)key.data) > %s){", (bndp
              != (Tr_exp*) 0) ? bndp[firstKey] : bndpUpper[firstKey]);
          break;
        case Ty_timestamp:
          sprintf(linebuf,
              "\n\tif (timeval_cmp((struct timeval *)key.data, %s) > 0){",
              (bndp != (Tr_exp*) 0) ? bndp[firstKey] : bndpUpper[firstKey]);
          break;
        case Ty_int:
        case Ty_ref:
          sprintf(linebuf, "\n\tif (*(int*)((char*)key.data) > %s){", (bndp
              != (Tr_exp*) 0) ? bndp[firstKey] : bndpUpper[firstKey]);
          break;
        case Ty_string:
          sprintf(linebuf, "\n\tif (strncmp((char*)key.data, %s, %d) > 0){",
              (bndp != (Tr_exp*) 0) ? bndp[firstKey] : bndpUpper[firstKey],
              offset);
          break;
        default:
          rc = ERR_NTSQL_INTERNAL;
          EM_error(0, rc, __LINE__, __FILE__, "transCursorGet2C");
          goto exit;
        } // end switch
        strcat(buf, linebuf);
        sprintf(linebuf, "\nrc=DB_NOTFOUND;"
          "\n}"
          "\n}");

        strcat(buf, linebuf);
      } // end else (BTREE)
    }
  }

  exit: return rc;
}
err_t transCursorDelete2C(char *cursorname, char *buf) {
  SMLog::SMLOG(10, "Entering transCursorDelete2C cursorname: %s", cursorname);
  err_t rc = ERR_NONE;

  /* no need to compare if it's imdb for now
   * since the API is exactly the same
   */
  /* if (kind == TAB_MEMORY) */
  //sprintf(buf, "\nif ((rc = %s->c_del(%s, 0)) != 0) {"
  //	  "\nadlabort(rc, \"IM_RELC->c_del() or DBC->c_del()\");"
  //  "\n}",
  //	  cursorname, cursorname);
  if (isESL()) {
    sprintf(
        buf,
        "\nif ((rc = %s->c_del(%s, 0)) != 0) {"
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_RELC->c_del() or DBC->c_del()\");"
          "\nreturn s_failure;"
          "\n}", cursorname, cursorname, getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        buf,
        "\nif ((rc = %s->c_del(%s, 0)) != 0) {"
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_RELC->c_del() or DBC->c_del()\");"
          "\nreturn;"
          "\n}", cursorname, cursorname, getUserName(), getAggrName());
  } else {
    sprintf(buf, "\nif ((rc = %s->c_del(%s, 0)) != 0) {"
      "\nadlabort(rc, \"IM_RELC->c_del() or DBC->c_del()\");"
      "\n}", cursorname, cursorname);
  }

  /* else
   sprintf(buf, "\nif ((rc = %s->c_del(%s, 0)) != 0) {"
   "\nadlabort(rc, \"DBC->c_del()\");"
   "\n}",
   cursorname, cursorname); */

  exit: return rc;
}

/*
 err_t transStreamPut2C(char* stName,
 char *buf){
 sprintf(buf,"\nif (%s->put(d) !=0){"
 "\nadlabort(rc, \"%s->put()\");"
 "\n}"
 ,stName, stName);
 return ERR_NONE;

 }
 */
err_t transCursorPut2C(char *cursorname, char *buf) {
  SMLog::SMLOG(10, "Entering transCursorPut2C cursorname: %s", cursorname);
  err_t rc = ERR_NONE;

  /* no need to compare if it's imdb for now
   * since the API is exactly the same
   */
  /* if (kind == TAB_MEMORY) */
  //sprintf(buf, "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
  //	  "\nadlabort(rc, \"IM_RELC->c_put() or DBC->c_put()\");"
  //  "\n}",
  //  cursorname, cursorname);

  if (isESL()) {
    sprintf(
        buf,
        "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_RELC->c_put() or DBC->c_put()\");"
          "\nreturn s_failure;"
          "\n}", cursorname, cursorname, getUserName(), getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        buf,
        "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_RELC->c_put() or DBC->c_put()\");"
          "\nreturn;"
          "\n}", cursorname, cursorname, getUserName(), getAggrName());
  } else {
    sprintf(buf, "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
      "\nadlabort(rc, \"IM_RELC->c_put() or DBC->c_put()\");"
      "\n}", cursorname, cursorname);
  }

  /* else
   sprintf(buf, "\nif ((rc = %s->c_put(%s, &key, &data, DB_CURRENT)) != 0) {"
   "\nadlabort(rc, \"DBC->c_put()\");"
   "\n}",
   cursorname, cursorname); */

  exit: return rc;
}
err_t transCursorClose2C(char *cursorname, char *buf) {
  SMLog::SMLOG(10, "Entering transCursorClose2C cursorname: %s", cursorname);
  err_t rc = ERR_NONE;

  /* no need to see if it's imdb for now
   * since the API is exactly the same
   */
  /* if (kind == TAB_MEMORY) */
  //sprintf(buf, "\nif (%s && (rc = %s->c_close(%s)) != 0) {"
  //	  "\nadlabort(rc, \"IM_RELC->c_close() or DBC->c_close()\");"
  //  "\n}",
  //  cursorname, cursorname, cursorname);
  if (isESL()) {
    sprintf(
        buf,
        "\nif (%s && (rc = %s->c_close(%s)) != 0) {"
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_RELC->c_close() or DBC->c_close()\");"
          "\nreturn s_failure;"
          "\n}", cursorname, cursorname, cursorname, getUserName(),
        getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        buf,
        "\nif (%s && (rc = %s->c_close(%s)) != 0) {"
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_RELC->c_close() or DBC->c_close()\");"
          "\nreturn;"
          "\n}", cursorname, cursorname, cursorname, getUserName(),
        getAggrName());
  } else {
    sprintf(buf, "\nif (%s && (rc = %s->c_close(%s)) != 0) {"
      "\nadlabort(rc, \"IM_RELC->c_close() or DBC->c_close()\");"
      "\n}", cursorname, cursorname, cursorname);
  }

  /* else
   sprintf(buf, "\nif ((rc = %s->c_close(%s)) != 0) {"
   "\nadlabort(rc, \"DBC->c_close()\");"
   "\n}",
   cursorname, cursorname); */
  exit: return rc;
}

err_t transTabIomod2C(E_enventry var, E_enventry te, char* name) {
  SMLog::SMLOG(10, "Entering transTabIomod2C name: %s", name);
  err_t rc = ERR_NONE;
  char fileName[80];
  FILE* fdesc;

  Ty_ty destTy = var->u.stream.ty;
  A_list destFields = destTy->u.record;
  Ty_ty srcTy = te->u.var.ty;
  A_list srcFields = srcTy->u.record;

  if (destFields->length != srcFields->length) {
    rc = ERR_INCOMPATIBLE_TYPE;
    EM_error(0, rc, __LINE__, __FILE__);
    return rc;
  }

  for (int i = 0; i < destFields->length; i++) {
    Ty_field df = (Ty_field) getNthElementList(destFields, i);
    Ty_field sf = (Ty_field) getNthElementList(srcFields, i);

    if (df->ty->kind != sf->ty->kind) {
      rc = ERR_INCOMPATIBLE_TYPE;
      EM_error(0, rc, __LINE__, __FILE__,
          "Types must match exactly w/o conversion.");
      return rc;
    }

    //we want to check length for chars
    if (df->ty->kind == Ty_string && df->size != sf->size) {
      rc = ERR_INCOMPATIBLE_TYPE;
      EM_error(0, rc, __LINE__, __FILE__,
          "Length of char fields must also match.");
      return rc;
    }

    //we also want to check iskey, which should be false,
    if (sf->iskey) {
      rc = ERR_INCOMPATIBLE_TYPE;
      EM_error(0, rc, __LINE__, __FILE__, "Keys not allowed in source table.");
      return rc;
    }
  }

  char *iomodFileTemplate = "#include <sys/types.h>"
    "\n#include <math.h>"
    "\n#include <sys/types.h>"
    "\n#include <stdio.h>"
    "\n#include <stdlib.h>"
    "\n#include <math.h>"
    "\n#include <db.h>"
    "\n#include <unistd.h>"
    "\n#include <string.h>"
    "\n#include <dlfcn.h>"
    "\n#include <rtree.h>"
    "\nextern \"C\"{"
    "\n#include <im_db.h>"
    "\n#include <swimlib.h>"
    "\n#include <fptree.h>"
    "\n#include <memmgr.h>"
    "\n#include <timeval.h>"
    "\n#include <histmgr.h>"
    "\n#include <mcheck.h>"
    "\n}"
    "\n#include \"adllib.h\""
    "\n#include <ext/hash_map>"
    "\nusing namespace ESL;"
    "\nusing namespace __gnu_cxx;"
    "\n\nextern \"C\" int getTuple(buffer* dest, bufferMngr* bm);"
    "\nextern \"C\" int closeConnection();"
    "\nint first = 1;"
    "\nint getTuple(buffer* dest, bufferMngr* bm) {"
    "\nif(!first) return 2; //No data"
    "\nfirst = 0;"
    "\nint done = 0;"
    "\nint rc = 0;"
    "\nint first_entry=1;"
    "\nDBT key, data;"
    "\nchar keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];"
    "\nDB *tab;"
    "\nDBC *tabc;"
    "\nif ((rc = db_create(&tab, NULL, 0)) != 0) {"
    "\nreturn -1;"
    "\n}"
    "\nif ((rc = tab->set_pagesize(tab, 2048)) != 0) {"
    "\nreturn -1;"
    "\n}"
    "\nif ((rc = tab->set_flags(tab, DB_DUP)) != 0) {"
    "\nreturn -1;"
    "\n}"
    "\nif ((rc = tab->open(tab, \"%s\", NULL, DB_BTREE, DB_CREATE, 0644)) != 0) {"
    "\nreturn -1;"
    "\n}"
    "\nif ((rc = tab->cursor(tab, NULL, &tabc, 0)) != 0) {"
    "\nreturn -1;"
    "\n}"
    "\nwhile(!done) {"
    "\nmemset(&key, 0, sizeof(key));"
    "\nmemset(&data, 0, sizeof(data));"
    "\nrc = tabc->c_get(tabc, &key, &data, (first_entry)?DB_FIRST:DB_NEXT);"
    "\nif(rc == 0) {"
    "\nfirst_entry = 0;"
    "\n%s;" //value assignments, if any
      "\n}"
      "\nelse if (rc == DB_NOTFOUND) {"
      "\ndone = 1;"
      "\n}"
      "\nif(rc == 0) {"
      "\nstruct timeval tmpAtime;"
      "\ntmpAtime.tv_sec = 0;"
      "\ntmpAtime.tv_usec = 0;"
      "\ndest->put(&data, &tmpAtime, &key);"
      "\n}"
      "\n} /* ends while */"
      "\nif (tabc && (rc = tabc->c_close(tabc)) != 0) {"
      "\nreturn 0;"
      "\n}"
      "\nif (tab && ((rc = tab->close(tab, 0)) != 0)) {"
      "\nreturn 0;"
      "\n}"
      "\ntab = NULL;"
      "\nreturn 0;"
      "\n}"
      "\nint closeConnection() {"
      "\nfirst = 1;"
      "\nreturn 0;"
      "\n}";

  //S_name(te->u.var.name), "");

  sprintf(fileName, "../exe/%s.cc", name);

  //printf("we got here2 %s\n", name);fflush(stdout);
  fdesc = fopen(fileName, "w");

  if (fdesc == NULL) {
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "transTabIomod2C");
    goto exit;
  }

  fprintf(fdesc, iomodFileTemplate, S_name(te->u.var.source), "");
  fclose(fdesc);

  exit: return rc;
}

//For streams, store keys in both key.data and data.data, don't have to worry
//about external functions on key though, because no real keys
err_t transCSVIomod2C(char* name, E_enventry var) {
  SMLog::SMLOG(10, "Entering transCSVIomod2C name: %s", name);
  err_t rc = ERR_NONE;
  char fileName[80];
  char
      *iomodFileTemplate = "#include <sys/types.h>"
        "\n#include <dlfcn.h>"
        "\n#include <sys/socket.h>"
        "\n#include <netinet/in.h>"
        "\n#include <netinet/tcp.h>"
        "\n#include <arpa/inet.h>"
        "\n#include <unistd.h>"
        "\n#include <fcntl.h>"
        "\n#include <sys/param.h>"
        "\n#include <stdio.h>"
        "\n#include <stdlib.h>"
        "\n#include <string.h>"
        "\n#include <errno.h>"
        "\n#include <sys/time.h>"
        "\n#include <dbt.h>"
        "\n#include <buffer.h>"
        "\n#include <unordered_map>"
        // "\n#include <ext/hash_map>"
          "\nusing namespace __gnu_cxx;"
          "\n#include <adllib.h>"
          "\nusing namespace ESL;"
          "\n#define MAX_BUFFER 65536"
          "\n\nextern \"C\" int getTuple(buffer* dest, bufferMngr* bm);"
          "\nextern \"C\" int closeConnection();"
          "\nint fdesc = -1;"
          "\nint listensockfd = -1;"
          "\nstruct timeval extTime;"
          "\nint outOfOrder = 0;"
          "\nstruct sockaddr_in listensock;"
	  "\n\nint DEBUG = 0;"
          "\n\nint init() {"
          "\nint p;                /*general purpose    */"
          "\nint op;"
          "\nlistensock.sin_family=AF_INET;"
          "\nlistensock.sin_port=htons((unsigned short)%d);"
          "\nlistensock.sin_addr.s_addr=INADDR_ANY;"
          "\n\n/* create socket, terminate in case of failure */"
          "\nif((listensockfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {"
          "\nperror(\"Error calling socket()\");"
          "\nreturn -1;"
          "\n}"
          "\nif(bind(listensockfd, (struct sockaddr *)&listensock, sizeof(listensock))) {"
          "\nperror(\"Error calling bind()\");"
          "\nreturn -1;"
          "\n}"
          "\n\n/* make socket listening for connections, terminate in case of failure */"
          "\nif(listen(listensockfd, 1)) {"
          "\nperror(\"Error calling listen()\");"
          "\nreturn -1;"
          "\n}"
          "\nop = fcntl(listensockfd, F_GETFL, 0);"
          "\nif (op != -1) {"
          "\nop |= O_NONBLOCK;"
          "\nfcntl(listensockfd, F_SETFL, op);"
          "\n}"
          "\n\np=sizeof(listensock);"
          "\nif((fdesc=accept(listensockfd, (struct sockaddr *)&listensock, (socklen_t *)&p)) == -1) {"
          "\nreturn 1;"
          "\n}"
          "\nop = fcntl(fdesc, F_GETFL, 0);"
          "\nif (op != -1) {"
          "\nop |= O_NONBLOCK;"
          "\nfcntl(fdesc, F_SETFL, op);"
          "\n}"
          "\nreturn 0;"
          "\n}"
          "\n\nint tryAccept() {"
          "\nint p, op;"
          "\n\np = sizeof(listensock);"
          "\nif((fdesc=accept(listensockfd, (struct sockaddr *)&listensock, (socklen_t *)&p)) == -1) {"
          "\nreturn 1;"
          "\n}"
          "\n\nop = fcntl(fdesc, F_GETFL, 0);"
          "\nif (op != -1) {"
          "\nop |= O_NONBLOCK;"
          "\nfcntl(fdesc, F_SETFL, op);"
          "\n}"
          "\nreturn 0;"
          "\n}"
          "\n bool FileExists( const char* FileName )"
          "\n {"
          "\n FILE* fp = NULL;"
          "\n fp = fopen( FileName, \"rb\" );"
          "\n if( fp != NULL )"
          "\n {"
          "\n   fclose( fp );"
          "\n   return true;"
          "\n }"
          "\n return false;"
          "\n }"
          "\n"
          "\n  void BucketizeStream(unordered_map<string, string>& bucket, char* buf) {"
          "\n    int tmp_length = 0;"
          "\n	      bool retrieved_stream_name = false;"
          "\n	      string tmp_name = \"\";"
          "\n	      string tmp_input = \"\";"
          "\n	      for (int i = 0; i < strlen(buf); i++) {"
          "\n	        if (!retrieved_stream_name) {"
          "\n	          if (buf[i] == ',') {"
          "\n	            tmp_length = 0;"
          "\n	            retrieved_stream_name = true;"
          "\n	            tmp_input = \"\";"
          "\n	          } else {"
          "\n	            tmp_name += buf[i];"
          "\n	            tmp_length++;"
          "\n	          }"
          "\n	        } else {"
          "\n	          tmp_input += buf[i];"
          "\n	        }"
          "\n"
          "\n	        if (buf[i] == '\\n') {"
          "\n	          retrieved_stream_name = false;"
          "\n	          bucket[tmp_name] += tmp_input + '\\n';"
          "\n	          tmp_name = \"\";"
          "\n	        }"
          "\n	      }"
          "\n	      if (tmp_name != \"\") {"
          "\n	        bucket[tmp_name] += tmp_input + '\\n';"
          "\n	      }"
          "\n  }"
          "\n"
          "\n  int getTuple(buffer* dest, bufferMngr* bm) {"
          "\n	      char buf[MAX_BUFFER];"
          "\n	      int rc;"
          "\n"
          "\n	      if (fdesc < 0 && listensockfd < 0) {"
          "\n	        rc = init();"
          "\n	      }"
          "\n	      if (rc == -1) {"
          "\n	        return -1;"
          "\n	      } // Error establishing connection"
          "\n"
          "\n	      if (fdesc < 0) {"
          "\n	        tryAccept();"
          "\n	      }"
          "\n"
          "\n	      if (fdesc < 0) {"
          "\n	        return 2; //No data"
          "\n	      }"
          "\n"
          "\n	      int olen = read(fdesc, buf, sizeof(buf));"
          "\n	      unordered_map<string, string> bucket;"
          "\n	      BucketizeStream(bucket, buf);"
          "\n	      fflush(stdout);"
          "\n"
          "\n	      if (olen == -1 && errno == EAGAIN) {"
          "\n	        return 2; // No data"
          "\n	      }"
          "\n	      if (olen == 0) {"
          "\n	        return 1; // Connection Closed"
          "\n	      }"
          "\n"
          "\n	      buf[olen] = 0;"
          "\n"
          "\n	      void (*putDataInBuffer)(char*, buffer*, bufferMngr*);"
          "\n"
          "\n	      for (unordered_map<string, string>::iterator it = bucket.begin(); it"
          "\n	          != bucket.end(); ++it) {"
          "\n	        char parser_lib[100];"
          "\n       if (DEBUG) {\nprintf(\"Getting parser_lib\\n\");"
          "\n       fflush(stdout);\n}"
          "\n	        sprintf(parser_lib, \"../exe/%%s.so\", it->first.c_str());"
          "\n       if (DEBUG) {\nprintf(\"Parser lib is %%s\\n\", parser_lib);"
          "\n       fflush(stdout);\n}"
          "\n       if (!FileExists(parser_lib)) {"
          "\n         printf(\"Warning: library %%s does not exist...\\n\", parser_lib);"
          "\n         continue;"
          "\n       }"
          "\n	        dest = bm->lookup(it->first.c_str());"
          "\n         if (DEBUG) {\nprintf(\"Checking if %%s is false \\n\", it->first.c_str());\n}"
          "\n         if (!dest->IsActive()) {"
          "\n           continue;"
          "\n         }"
          "\n	        void* handle = dlopen(parser_lib, RTLD_NOW);"
          "\n	        char error[100];"
          "\n	        if (!handle) {"
          "\n	          sprintf(error, \"In addIOModule, %%s \\n\", dlerror());"
          "\n	          printf(\"Error while loading the module: %%s\\n\", error);"
          "\n	          continue;"
          "\n	        }"
          "\n	        putDataInBuffer = (void(*)(char*, ESL::buffer*, ESL::bufferMngr*)) dlsym("
          "\n	            handle, \"putDataInBuffer\");"
          "\n	        putDataInBuffer(const_cast<char*> (it->second.c_str()), dest, bm);"
          "\n	      }"
          "\n	      return 0; // Got data"
          "\n	    }"
          "\n"
          "\nint closeConnection() {"
          "\nif(fdesc >0)"
          "\nclose(fdesc);"
          "\nif(listensockfd >0)"
          "\nclose(listensockfd);"
          "\nfdesc = -1;"
          "\nlistensockfd = -1;"
          "\nreturn 0;"
          "\n}";
  char iomodFileText[MAX_STR_LEN];
  char putDataIntoBufText[MAX_STR_LEN];
  A_list fields;
  int dataoff = 0, keyoff = 0;
  int len, i, tuplesize[2] = { 0, 0 };
  char buf[MAX_STR_LEN];
  FILE* fdesc;
  putDataIntoBufText[0] = 0;

  Ty_ty ty = var->u.stream.ty;
  fields = ty->u.record;

  for (i = 0; i < fields->length; i++) {
    Ty_field f = (Ty_field) getNthElementList(fields, i);

    if (!((var->u.stream.tk == tk_external && strcmp(S_name(f->name), S_name(
        var->u.stream.timekey)) == 0) || strcmp(S_name(f->name), ITIME_COLUMN)
        == 0)) {
      switch (f->ty->kind) {
      case Ty_real:
        if (f->iskey) {
          tuplesize[1] += sizeof(double);
        }
        tuplesize[0] += sizeof(double);
        break;
      case Ty_timestamp:
        if (f->iskey) {
          tuplesize[1] += sizeof(struct timeval);
        }
        tuplesize[0] += sizeof(struct timeval);
        break;
      case Ty_int:
        if (f->iskey) {
          tuplesize[1] += sizeof(int);
        }
        tuplesize[0] += sizeof(int);
        break;
      case Ty_string:
        if (f->iskey) {
          tuplesize[1] += f->size;
        }
        tuplesize[0] += f->size;
        break;
      case Ty_ref:
        //      if (i!=fields->length-1) {
        if (f->iskey) {
          tuplesize[0] += sizeof(int);
        }
        tuplesize[0] += sizeof(int);
        //      }
        break;
      default:
        rc = ERR_NTSQL_INTERNAL;
        EM_error(0, rc, __LINE__, __FILE__, "transCSVIomod2C");
        goto exit;
      }
    }
  }

  sprintf(buf,
      "\nvoid putDataInBuffer(char* dataStr, buffer* buf, bufferMngr* bm) {"
        "\nchar loadkeybuf[%d], loaddatabuf[%d];"
        "\nDBT key, data;"
        "\nchar* tok;"
        "\nmemset(&key, 0, sizeof(key));"
        "\nmemset(&data, 0, sizeof(data));"
        "\nkey.data = loadkeybuf;"
        "\ndata.data = loaddatabuf;"
        "\nstruct timeval tv;"
        "\nstruct timezone tz;"
        "\ngettimeofday(&tv, &tz);"
        "\nchar ooName[256];"
        "\nbuffer* ooBuf;"
        "\nsprintf(ooName, \"%%s_outOfOrder\", buf->name);"
        "\nooBuf = bm->lookup(ooName);"
        "\ntok = strtok(dataStr, \",\\n\");"
        "\nfflush(stdout);", tuplesize[1] + 1, tuplesize[0] + 1);

  strcat(putDataIntoBufText, buf);

  sprintf(buf, "\n\nwhile(tok) {");
  strcat(putDataIntoBufText, buf);

  len = fields->length;
  for (i = 0; i < len; i++) {
    Ty_field f = (Ty_field) getNthElementList(fields, i);

    if (var->u.stream.tk == tk_external && strcmp(S_name(f->name), S_name(
        var->u.stream.timekey)) == 0) {
      sprintf(buf, "\nstruct timeval oos_%d = getTimeval(tok);"
        "\nif (&oos_%d == NULL) {"
        "\nprintf(\"data format error\\n\");"
        "\nreturn;"
        "\n}"
        "\nif(timeval_cmp(extTime, oos_%d) > 0) {"
        "\noutOfOrder = 1;"
        "\ntv.tv_sec = oos_%d.tv_sec;"
        "\ntv.tv_usec = oos_%d.tv_usec;"
        "\n}"
        "\nelse {"
        "\noutOfOrder = 0;"
        "\nextTime.tv_sec = oos_%d.tv_sec;"
        "\nextTime.tv_usec = oos_%d.tv_usec;"
        "\n}", i, i, i, i, i, i, i);
      strcat(putDataIntoBufText, buf);
    }

    if (strcmp(S_name(f->name), ITIME_COLUMN) == 0) {
      if (f->iskey) {
        sprintf(buf, "\nmemset((char*)key.data+%d, 0, %d);"
          "\nmemcpy((char*)key.data+%d, (char*)&tv, %d);", keyoff,
            sizeof(struct timeval), keyoff, sizeof(struct timeval));
        keyoff += sizeof(struct timeval);
        strcat(putDataIntoBufText, buf);
      }
      sprintf(buf, "\nmemset((char*)data.data+%d, 0, %d);"
        "\nmemcpy((char*)data.data+%d, (char*)&tv, %d);", dataoff,
          sizeof(struct timeval), dataoff, sizeof(struct timeval));
      dataoff += sizeof(struct timeval);

    } else {
      if (i != 0) {
        strcat(putDataIntoBufText, "\nif (!tok) {"
          "\nprintf(\"data format error\\n\");"
          "\nreturn;"
          "\n}");
      }
      switch (f->ty->kind) {
      case Ty_real:
        if (f->iskey) {
          sprintf(buf, "\n*(double*)((char*)key.data+%d) = atof(tok);", keyoff);
          keyoff += sizeof(double);
          strcat(putDataIntoBufText, buf);
        }
        sprintf(buf, "\n*(double*)((char*)data.data+%d) = atof(tok);", dataoff);
        dataoff += sizeof(double);
        break;
      case Ty_timestamp:
        if (f->iskey) {
          sprintf(buf, "\nmemset((char*)key.data+%d, 0, %d);"
            "\nstruct timeval tk_%d = getTimeval(tok);"
            "\nmemcpy((char*)key.data+%d, (char*)&tk_%d, %d);", keyoff,
              sizeof(struct timeval), i, keyoff, i, sizeof(struct timeval));
          keyoff += sizeof(struct timeval);
          strcat(putDataIntoBufText, buf);
        }
        sprintf(buf, "\nmemset((char*)data.data+%d, 0, %d);"
          "\nstruct timeval td_%d = getTimeval(tok);"
          "\nmemcpy((char*)data.data+%d, (char*)&td_%d, %d);", dataoff,
            sizeof(struct timeval), i, dataoff, i, sizeof(struct timeval));
        dataoff += sizeof(struct timeval);
        break;
      case Ty_ref:
      case Ty_int:
        if (f->iskey) {
          sprintf(buf, "\n*(int*)((char*)key.data+%d) = atoi(tok);", keyoff);
          keyoff += sizeof(int);
          strcat(putDataIntoBufText, buf);
        }
        sprintf(buf, "\n*(int*)((char*)data.data+%d) = atoi(tok);", dataoff);
        dataoff += sizeof(int);
        break;
      case Ty_string:
        if (f->iskey) {
          sprintf(
              buf,
              "\nmemset((char*)key.data+%d, 0, %d);"
                "\nmemcpy((char*)key.data+%d, tok, strlen(tok)<%d?strlen(tok):%d);",
              keyoff, f->size, keyoff, f->size, f->size);
          keyoff += f->size;
          strcat(putDataIntoBufText, buf);
        }
        sprintf(buf, "\nmemset((char*)data.data+%d, 0, %d);"
          "\nmemcpy((char*)data.data+%d, tok, strlen(tok)<%d?strlen(tok):%d);",
            dataoff, f->size, dataoff, f->size, f->size);//string length
        dataoff += f->size;
        break;
      default:
        rc = ERR_NTSQL_INTERNAL;
        EM_error(0, rc, __LINE__, __FILE__, "transLoad2C");
        goto exit;
      }
      strcat(putDataIntoBufText, buf);
      strcat(putDataIntoBufText, "\ntok = strtok(NULL, \",\\n\");\n");
    }

    if (i == len - 1) {
      if (var->u.stream.tk == tk_internal) {
        sprintf(buf, "\ndata.size = %d;"
          "\nkey.size = %d;"
          "\nbuf->put(&data, &tv, &key);", tuplesize[0], tuplesize[1]);
      } else if (var->u.stream.tk == tk_external) {
        sprintf(buf, "\ndata.size = %d;"
          "\nkey.size = %d;"
          "\nif(outOfOrder == 1) {"
          "\nif(!ooBuf) {"
          "\nprintf(\"out of order stream %%s not found.\\n\", ooName);"
          "\n}"
          "\nooBuf->put(&data, &tv, &key);"
          "\n}"
          "\nelse"
          "\nbuf->put(&data, &extTime, &key);", tuplesize[0], tuplesize[1]);
      } else {
        sprintf(buf, "\ndata.size = %d;"
          "\nkey.size = %d;"
          "\nstruct timeval tmpAtime;"
          "\ntmpAtime.tv_sec = 0;"
          "\ntmpAtime.tv_usec = 0;"
          "\nbuf->put(&data, &tmpAtime, &key);", tuplesize[0], tuplesize[1]);
      }

      dataoff = 0;
      keyoff = 0;
      strcat(putDataIntoBufText, buf);
    }
  }
  strcat(putDataIntoBufText, "\n} /* end while(!tok) */"
    "\n}");

  sprintf(fileName, "../exe/%s.cc", name);

  // Generate the stream parser dynamically for each user/stream combo.
  char putDataIntoBufFilename[80];
  sprintf(putDataIntoBufFilename, "../exe/%s.cc", S_name(var->u.stream.sname));
  FILE* putDataDsc;
  putDataDsc = fopen(putDataIntoBufFilename, "w");
  if (putDataDsc == NULL) {
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "transCSVIomod2C");
    goto exit;
  }
  char putDataIntoBufferHeader[100];
  sprintf(putDataIntoBufferHeader, "%s",
      "#include <buffer.h>\n#include <adllib.h>\nusing namespace ESL;\n\nextern \"C\" ");
  fprintf(putDataDsc, "%s%s\n", putDataIntoBufferHeader, putDataIntoBufText);
  fclose(putDataDsc);

  // TODO(nlaptev): Delete the lines below, when we had verified that we do not need it.
  sprintf(iomodFileText, iomodFileTemplate, var->u.stream.port);
  fdesc = fopen(fileName, "w");
  if (fdesc == NULL) {
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "transCSVIomod2C");
    goto exit;
  }
  fprintf(fdesc, "%s\n", iomodFileText);
  fclose(fdesc);

  exit: return rc;
}

err_t transStreamDec2C(S_table venv, char *name, char *buf, E_enventry var) {
  SMLog::SMLOG(10, "Entering transStreamDec2C name: %s", name);
  err_t rc = ERR_NONE;
  bufferMngr *bm = bufferMngr::getInstance();
  DBUG_ENTER("transStreamDec2C");
  char* sname = NULL;
  buffer *b = bm->lookup("_ioBuffer");

  if (!bm->lookup(name)) { // buffer not exists
    S_symbol source = NULL;

    //here check if inbuilt with port is used
    // if so create a .cc file based on var->ty
    // give cc file a name and send message to ioBuffer
    // to add the iomodule then connect

    b->put(50, name);

    if (var->u.stream.source != NULL) {
      sname = S_name(var->u.stream.source);
      sname = strlwr(sname);
    }

    if (sname != NULL && strstr(sname, "port") == sname) {
      char sourceName[80];
      char sourceSoFile[80];
      sprintf(sourceName, "%s", sname);
      sprintf(sourceSoFile, "%s.so", sourceName);

      var->u.stream.port = atoi(sname + 4);
      if (var->u.stream.port <= 1024 || var->u.stream.port > 65536) {
        rc = ERR_INVALID_PORT_NUM;
        EM_error(0, rc, __LINE__, __FILE__);
        return rc;
      }

      if ((rc = transCSVIomod2C(sourceName, var)) != ERR_NONE) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transStreamDec2C",
            "transCSVIomod2C");
        return ERR_HISTORY;
      }

      b->put(ADD_BUILTIN_IOMOD, sourceName);
      source = S_Symbol(sourceSoFile);
    } else if (sname != NULL && strstr(sname, "table") == sname) {
      char sourceName[80];
      char sourceSoFile[80];
      char* tableName;
      E_enventry te;
      sprintf(sourceName, "%s%s", name, sname);
      sprintf(sourceSoFile, "%s.so", sourceName);

      tableName = sname + 5;
      //printf("we are not here yet %s\n" tableName);fflush(stdout);
      te = (E_enventry) S_look(venv, S_Symbol(tableName));
      //at this point we should verify that the table exists, may be
      if (!te || te->kind != E_varEntry) {
        rc = ERR_UNDEFINED_VARIABLE;
        EM_error(0, rc, __LINE__, __FILE__, tableName);
        return rc;
      }

      //make sure that it is a disk table (this may be ok temporarily)
      if (te->u.var.scope != TAB_GLOBAL) {
        rc = ERR_INVALID_SCOPE;
        EM_error(0, rc, __LINE__, __FILE__, tableName, "memory");
        return rc;
      }

      //TODO make sure that the schemas match

      // printf("we got here %s\n", sourceName);fflush(stdout);
      //write up a module.
      if ((rc = transTabIomod2C(var, te, sourceName)) != ERR_NONE) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transStreamDec2C",
            "transTabIomod2C");
        return ERR_HISTORY;
      }

      b->put(ADD_BUILTIN_IOMOD, sourceName);
      source = S_Symbol(sourceSoFile);
    } else if (var->u.stream.source != NULL) {
      char sourceSoFile[80];

      sprintf(sourceSoFile, "%s.so", S_name(var->u.stream.source));
      source = S_Symbol(sourceSoFile);
    }

    S_symbol target = var->u.stream.target;
    //int shared = source != 0 || target != 0;
    int shared = 1;

    b->put(ADDED_STREAM_BUFFER, name);
    if (bm->create(name, shared ? SHARED : NOT_SHARED) != 0) {
      printf("Adding Stream buffer with name %s\n", name);
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transStreamDec2C",
          "bufferMngr::create()");
      return ERR_HISTORY;
    };
    if (source) {
      // notify I/O scheduler to connect I/O module to stream
      if (b) {
        b->put(CONNECT_IOMODULE, name, S_name(source));
      }
    } // end if source
  } // end if bm->lookup
  else {
    S_symbol source;
    if (sname != NULL && strstr(sname, "port") == sname) {
      char sourceSoFile[80];
      sprintf(sourceSoFile, "%s.so", name);
      source = S_Symbol(sourceSoFile);
    } else
      source = var->u.stream.source;
    if (source) {
      // notify I/O scheduler to connect I/O module to stream
      buffer *b = bm->lookup("_ioBuffer");
      if (b) {
        b->put(CONNECT_IOMODULE, name, S_name(source));
      }
    } // end if source
  }

  b->put(51, name);

  DBUG_RETURN(ERR_NONE);
  return ERR_NONE;
}

err_t transTabDec2C(char *name, char *buf, E_enventry var) {
  SMLog::SMLOG(10, "Entering transTabDec2C name: %s", name);
  char linebuf[MAX_STR_LEN];
  err_t rc = ERR_NONE;
  tabscope_t kind = var->u.var.scope;
  tabindex_t idx = (var->u.var.index == (A_index) 0) ? (tabindex_t) 1
      : var->u.var.index->kind;
  if (var->u.var.isBuffer) {
    sprintf(buf, "\nwindowBuf *%s;", name);
  } else if (kind == TAB_MEMORY)
    sprintf(buf, "\nIM_REL *%s;", name);
  else if (idx == INDEX_STREAM)
    sprintf(buf, "\nbuffer *%s;", name);
  else if (idx == INDEX_RTREE)
    sprintf(buf, "\nRTree *%s;", name);
  else {
    sprintf(buf, "\nDB *%s;", name);
    // declare btree comparison funtion

    int offset = 0;
    sprintf(linebuf, "\nint _%s_cmp(DB* dbp, const DBT *a, const DBT *b){"
      "\n\tint ai, bi, ri, rs;"
      "\n\tdouble ad, bd, rd;"
      "\n\tstruct timeval *at, *bt;");
    strcat(buf, linebuf);
    A_list fields = var->u.var.ty->u.record;
    for (int i = 0; i < fields->length; i++) {
      Ty_field f = (Ty_field) getNthElementList(fields, i);
      if (f->iskey) {
        switch (f->ty->kind) {
        case Ty_int:
          sprintf(linebuf, "\nmemcpy(&ai, (char*)a->data+%d, sizeof(int));"
            "\nmemcpy(&bi, (char*)b->data+%d, sizeof(int));"
            "\nri = ai - bi;"
            "\nif (ri !=0) return ri;", offset, offset);

          offset += sizeof(int);
          break;
        case Ty_real:
          sprintf(linebuf, "\nmemcpy(&ad, (char*)a->data+%d, sizeof(double));"
            "\nmemcpy(&bd, (char*)b->data+%d, sizeof(double));"
            "\nrd = ad - bd;"
            "\nif (fabs(rd) >=1e-5) return rd>0?1:-1;", offset, offset);

          offset += sizeof(double);
          break;
        case Ty_timestamp:
          sprintf(linebuf,
              "\nat=(struct timeval*)malloc(sizeof(struct timeval));"
                "\nmemcpy(at, (char*)a->data+%d, sizeof(struct timeval));"
                "\nbt=(struct timeval*)malloc(sizeof(struct timeval));"
                "\nmemcpy(bt, (char*)b->data+%d, sizeof(double));"
                "\nrd = timeval_subtract(*at, *bt);"
                "\nfree(at);"
                "\nfree(bt);"
                "\nif (fabs(rd) >=1e-5) return rd>0?1:-1;", offset, offset);

          offset += sizeof(double);
          break;
        case Ty_string:
          sprintf(linebuf,
              "\nreturn strncmp((char*)a->data+%d, (char*)b->data+%d, %d);",
              offset, offset, f->size);
          offset += f->size;
          break;
        default:
          EM_error(0, ERR_DATATYPE, __LINE__, __FILE__, "BTREE INDEX");
        } // end switch
        strcat(buf, linebuf);
      } // end if f->iskey
    } // end for
    sprintf(linebuf, "return 0;"
      "\n};");
    strcat(buf, linebuf);
  }

  SMLog::SMLOG(10, ">Exiting transTabDec2C");
  return rc;
}

err_t transTabInit2C(char *handlename, char *dbname, int haskey, char *buf,
    tabscope_t scope, tabindex_t idx, ty_t keyType, int inaggr, int isBuffer,
    S_symbol tname) {
  SMLog::SMLOG(10,
      "Entering transTabInit2C handlename: %s, DBname: %s, tname: %s",
      handlename, dbname, S_name(tname));
  err_t rc = ERR_NONE;
  char line[MAX_STR_LEN];
  char *dbtypes[] = { "", "DB_BTREE", "DB_HASH", "DB_RECNO", "DB_QUEUE",
      "DB_RTREE" };

  *buf = '\0';

  if (scope == TAB_MEMORY && !isBuffer) {
    if ((isAdHoc() || isESL() || isESLAggr()) && !inaggr) {
      sprintf(line, "\n%s = (IM_REL*)inMemTables->operator[](\"%s\");"
        "\nif(%s == NULL) {", handlename, S_name(tname), handlename);
      strcat(buf, line);
      if (isESL()) {
        sprintf(
            line,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: inMemTables lookup\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            line,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: inMemTables lookup\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(line, "\nadlabort(rc, \"inMemTables lookup\");"
          "\n}");
      }
      strcat(buf, line);
    } else {
      sprintf(line,
          "\nif ((rc = im_rel_create(&%s, NULL, IM_LINKEDLIST, %s)) != 0) {",
          handlename, (haskey) ? "IM_REL_INDEXED" : "0");
      strcat(buf, line);

      if (isESL()) {
        sprintf(
            line,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: im_rel_create()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            line,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: im_rel_create()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(line, "\nadlabort(rc, \"im_rel_create()\");"
          "\n}");
      }
      strcat(buf, line);

      sprintf(line, "\nif ((rc = %s->open(%s, %s, 0)) != 0) {", handlename,
          handlename, dbname);
      strcat(buf, line);

      if (isESL()) {
        sprintf(
            line,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: open()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            line,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: open()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(line, "\nadlabort(rc, \"open()\");"
          "\n}");
      }
      strcat(buf, line);
    }

  } else if (idx == INDEX_RTREE && !isBuffer) {

    char idxname[80];

    strcpy(idxname, os_filename(dbname));
    strcpy(idxname + strlen(idxname) - 1, ".idx\"");

    if (scope == TAB_LOCAL) {
      sprintf(line, "\n(void)unlink(%s);", os_filename(dbname));
      strcat(buf, line);
      sprintf(line, "\n(void)unlink(%s);", idxname);
      strcat(buf, line);
    }

    if (strstr(dbname, "_adl_db_")) { //new rtree
      sprintf(line, "\nif ((rc = rtree_create(&%s, %s, %s, NEW_RTREE)) !=0 &&"
        "\n(rc = %s->open(&%s, %s, %s, NEW_RTREE)) != 0) {", handlename,
          idxname, os_filename(dbname), handlename, handlename, idxname,
          os_filename(dbname));
      strcat(buf, line);

      if (isESL()) {
        sprintf(
            line,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: rtree_create()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            line,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: rtree_create()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(line, "\nadlabort(rc, \"rtree_create()\");"
          "\n}");
      }
      strcat(buf, line);

    } else { // existing rtree
      sprintf(line, "\nif ((rc = rtree_create(&%s, %s, %s, OLD_RTREE)) !=0 &&"
        "\n(rc = %s->open(&%s, %s, %s, OLD_RTREE)) != 0) {", handlename,
          idxname, os_filename(dbname), handlename, handlename, idxname,
          os_filename(dbname));
      strcat(buf, line);

      if (isESL()) {
        sprintf(
            line,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: rtree_create()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            line,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: rtree_create()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(line, "\nadlabort(rc, \"rtree_create()\");"
          "\n}");
      }
      strcat(buf, line);
    }

  } else if (!isBuffer) {

    if (scope == TAB_LOCAL) {
      sprintf(line, "\n(void)unlink(%s);", os_filename(dbname));
      strcat(buf, line);
    }

    sprintf(line, "\nif ((rc = db_create(&%s, NULL, 0)) != 0) {", handlename);
    strcat(buf, line);

    if (isESL()) {
      sprintf(
          line,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: db_create()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          line,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: db_create()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(line, "\nadlabort(rc, \"db_create()\");"
        "\n}");
    }
    strcat(buf, line);

    sprintf(line, "\nif ((rc = %s->set_pagesize(%s, 2048)) != 0) {",
        handlename, handlename);
    strcat(buf, line);

    if (isESL()) {
      sprintf(
          line,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: set_pagesize()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          line,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: set_pagesize()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(line, "\nadlabort(rc, \"set_pagesize()\");"
        "\n}");
    }
    strcat(buf, line);

    //      if (haskey == 0) {
    if (idx != INDEX_RECNO) {
      sprintf(line, "\nif ((rc = %s->set_flags(%s, DB_DUP)) != 0) {",
          handlename, handlename);
      strcat(buf, line);
      if (isESL()) {
        sprintf(
            line,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: set_flags()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            line,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: set_flags()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(line, "\nadlabort(rc, \"set_flags()\");"
          "\n}");
      }
      strcat(buf, line);
    }
    //
    if (idx == INDEX_BTREE && keyType != Ty_nil) {
      transCompFun(handlename, line);
      strcat(buf, line);
    }

    sprintf(line,
        "\nif ((rc = %s->open(%s, %s, NULL, %s, DB_CREATE, 0664)) != 0) {",
        handlename, handlename, os_filename(dbname), dbtypes[(int) idx]);
    strcat(buf, line);

    if (isESL()) {
      sprintf(
          line,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: open()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          line,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: open()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(line, "\nadlabort(rc, \"open()\");"
        "\n}");
    }
    strcat(buf, line);
  }

  exit: SMLog::SMLOG(10, ">Exiting transTabInit2C ");
  return rc;
}

err_t transTabInit2C_imdb(char *handlename, char *dbname, tabscope_t scope,
    int haskey, char *buf) {
  err_t rc = ERR_NONE;
  char line[MAX_STR_LEN];

  *buf = '\0';
  if (scope == TAB_MEMORY) {
    /* to be implemented */
  } else {
    if (scope == TAB_LOCAL) {
      sprintf(line, "\n(void)unlink(%s);", dbname);
      strcat(buf, line);
    }

    sprintf(line,
        "\nif ((rc = im_rel_create(&%s, NULL, IM_LINKEDLIST, %s)) != 0) {",
        handlename, (haskey) ? "IM_REL_INDEXED" : "0");
    strcat(buf, line);

    if (isESL()) {
      sprintf(
          line,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: im_rel_create()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          line,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: im_rel_create()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(line, "\nadlabort(rc, \"im_rel_create()\");"
        "\n}");
    }
    strcat(buf, line);

    if (haskey == 0) {
      /* NO set_flags fuction for IMDB */

      /*sprintf(line,
       "\nif ((rc = %s->set_flags(%s, DB_DUP)) != 0) {"
       "\nadlabort(rc, \"set_flags()\");"
       "\n}"
       , handlename, handlename);
       strcat(buf, line);*/
    }

    sprintf(line, "\nif ((rc = %s->open(%s, %s, 0)) != 0) {", handlename,
        handlename, dbname);
    strcat(buf, line);
    if (isESL()) {
      sprintf(
          line,
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: open()\");"
            "\nreturn s_failure;"
            "\n}", getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          line,
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: open()\");"
            "\nreturn;"
            "\n}", getUserName(), getAggrName());
    } else {
      sprintf(line, "\nadlabort(rc, \"open()\");"
        "\n}");
    }
    strcat(buf, line);
  }

  return rc;
}

/*
 transTabRemove2C() removes the contents of a table.

 DELETE * FROM tab;
 */
err_t transTabRemove2C(char *handlename, char *dbname, char *buf,
    tabscope_t scope, tabindex_t idx) {
  err_t rc = ERR_NONE;
  char line[MAX_STR_LEN];

  if (scope == TAB_MEMORY) {
    //sprintf(buf, "\nif ((rc = %s->close(%s, 0)) != 0) {"
    //    "\nadlabort(rc, \"IM_REL->remove()\");"
    //    "\n}",
    //    handlename, handlename);
    if (isESL()) {
      sprintf(
          buf,
          "\nif ((rc = %s->close(%s, 0)) != 0) {"
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->remove()\");"
            "\nreturn s_failure;"
            "\n}", handlename, handlename, getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          buf,
          "\nif ((rc = %s->close(%s, 0)) != 0) {"
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->remove()\");"
            "\nreturn;"
            "\n}", handlename, handlename, getUserName(), getAggrName());
    } else {
      sprintf(buf, "\nif ((rc = %s->close(%s, 0)) != 0) {"
        "\nadlabort(rc, \"IM_REL->remove()\");"
        "\n}", handlename, handlename);
    }
  } else if (idx == INDEX_RTREE) {
  } else {
    //sprintf(buf, "\nif ((rc = %s->remove(%s, %s, NULL, 0)) != 0) {"
    //    "\nadlabort(rc, \"DB->remove()\");"
    //    "\n}"
    //    "\n%s = NULL;",
    //	    handlename, handlename, handlename, dbname);
    if (isESL()) {
      sprintf(
          buf,
          "\nif ((rc = %s->remove(%s, %s, NULL, 0)) != 0) {"
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DB->remove()\");"
            "\nreturn s_failure;"
            "\n}"
            "\n%s = NULL;", handlename, handlename, handlename, getUserName(),
          getQueryName(), dbname);
    } else if (isESLAggr()) {
      sprintf(
          buf,
          "\nif ((rc = %s->remove(%s, %s, NULL, 0)) != 0) {"
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DB->remove()\");"
            "\nreturn;"
            "\n}"
            "\n%s = NULL;", handlename, handlename, handlename, getUserName(),
          getAggrName(), dbname);
    } else {
      sprintf(buf, "\nif ((rc = %s->remove(%s, %s, NULL, 0)) != 0) {"
        "\nadlabort(rc, \"DB->remove()\");"
        "\n}"
        "%s = NULL;", handlename, handlename, handlename, dbname);
    }
  }
  exit: return rc;
}
/*
 err_t transStreamClose2C(char *dbname,
 char *buf){
 DBUG_ENTER("transStreaClose2C");
 sprintf(buf, "\nbm->kill(\"%s\");"
 , dbname);
 DBUG_RETURN(ERR_NONE);
 }
 */
err_t transTabClose2C(char *handlename, char *dbname, char *buf,
    tabscope_t scope, tabindex_t idx, int term_cond) {
  err_t rc = ERR_NONE;
  char line[MAX_STR_LEN];
  char prefix[MAX_STR_LEN];
  prefix[0] = '\0';

  if (term_cond) {
    sprintf(prefix, "if(!not_delete) {");
  }

  if (scope == TAB_MEMORY) {
    //sprintf(buf, "\n%s"
    //    "\nif ((rc = %s->close(%s, 0)) != 0) {"
    //    "\nadlabort(rc, \"IM_REL->close()\");"
    //    "\n}",
    //    prefix, handlename, handlename);
    if (isESL()) {
      sprintf(
          buf,
          "\n%s"
            "\nif ((rc = %s->close(%s, 0)) != 0) {"
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->close()\");"
            "\nreturn s_failure;"
            "\n}", prefix, handlename, handlename, getUserName(),
          getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          buf,
          "\n%s"
            "\nif ((rc = %s->close(%s, 0)) != 0) {"
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->close()\");"
            "\nreturn;"
            "\n}", prefix, handlename, handlename, getUserName(), getAggrName());
    } else {
      sprintf(buf, "\n%s"
        "\nif ((rc = %s->close(%s, 0)) != 0) {"
        "\nadlabort(rc, \"IM_REL->close()\");"
        "\n}", prefix, handlename, handlename);
    }
  } else if (idx == INDEX_RTREE) {
    //sprintf(buf, "\n%s"
    //	    "\nif (%s && ((rc = %s->close(%s, 0)) != 0)) {"
    //    "\nadlabort(rc, \"DB->close()\");"
    //    "\n}"
    //    "\n%s = NULL;",
    //    prefix, handlename, handlename, handlename, handlename);
    if (isESL()) {
      sprintf(
          buf,
          "\n%s"
            "\nif (%s && ((rc = %s->close(%s, 0)) != 0)) {"
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DB->close()\");"
            "\nreturn s_failure;"
            "\n}"
            "\n%s = NULL;", prefix, handlename, handlename, handlename,
          getUserName(), getQueryName(), handlename);
    } else if (isESLAggr()) {
      sprintf(
          buf,
          "\n%s"
            "\nif (%s && ((rc = %s->close(%s, 0)) != 0)) {"
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DB->close()\");"
            "\nreturn;"
            "\n}"
            "\n%s = NULL;", prefix, handlename, handlename, handlename,
          getUserName(), getAggrName(), handlename);
    } else {
      sprintf(buf, "\n%s"
        "\nif (%s && ((rc = %s->close(%s, 0)) != 0)) {"
        "\nadlabort(rc, \"DB->close()\");"
        "\n}"
        "\n%s = NULL;", prefix, handlename, handlename, handlename, handlename);
    }

    if (scope == TAB_LOCAL) {
      char idxname[80];
      strcpy(idxname, os_filename(dbname));
      strcpy(idxname + strlen(idxname) - 1, ".idx\"");
      sprintf(line, "\n(void)unlink(%s);", os_filename(dbname));
      strcat(buf, line);
      sprintf(line, "\n(void)unlink(%s);", idxname);
      strcat(buf, line);
    }
  } else {
    //sprintf(buf, "\n%s"
    //    "\nif (%s && ((rc = %s->close(%s, 0)) != 0)) {"
    //    "\nadlabort(rc, \"DB->close()\");"
    //    "\n}"
    //    "\n%s = NULL;",
    //    prefix, handlename, handlename, handlename, handlename);
    if (isESL()) {
      sprintf(
          buf,
          "\n%s"
            "\nif (%s && ((rc = %s->close(%s, 0)) != 0)) {"
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: DB->close()\");"
            "\nreturn s_failure;"
            "\n}"
            "\n%s = NULL;", prefix, handlename, handlename, handlename,
          getUserName(), getQueryName(), handlename);
    } else if (isESLAggr()) {
      sprintf(
          buf,
          "\n%s"
            "\nif (%s && ((rc = %s->close(%s, 0)) != 0)) {"
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: DB->close()\");"
            "\nreturn;"
            "\n}"
            "\n%s = NULL;", prefix, handlename, handlename, handlename,
          getUserName(), getAggrName(), handlename);
    } else {
      sprintf(buf, "\n%s"
        "\nif (%s && ((rc = %s->close(%s, 0)) != 0)) {"
        "\nadlabort(rc, \"DB->close()\");"
        "\n}"
        "\n%s = NULL;", prefix, handlename, handlename, handlename, handlename);
    }

    if (scope == TAB_LOCAL) {
      sprintf(line, "\n(void)unlink(%s);", os_filename(dbname));
      strcat(buf, line);
    }
  }
  if (term_cond) {
    strcat(buf, "\n}");
  }
  exit: return rc;
}

/* given a (key,data) pair, decompose it into columns and assign the
 values to corresponding fields in a tuple */
err_t assignField2C(Ty_field f, char *stru, char *f_name, int *keyoff,
    int *dataoff, char *fbuf) {
  err_t rc = ERR_NONE;

  switch (f->ty->kind) {
  case Ty_ref:
    /* FALL THROUGH */
  case Ty_int:
    /*if (f->iskey) {
     sprintf(fbuf, "\nmemcpy(&(%s.%s), (char*)key.data+%d, sizeof(int));"
     "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(int));"
     "\n//printf(\"Retrieved %s.%s = %%d\\n\", %s.%s);"
     "\n//fflush(stdout);",
     stru, f_name, *keyoff,
     stru, f_name, *dataoff, stru, f_name, stru, f_name);
     *keyoff+=sizeof(int);
     *dataoff+=sizeof(int);
     } else { */
    sprintf(fbuf, "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(int));"
      "\n//printf(\"Retrieved %s.%s = %%d\\n\", %s.%s);"
      "\n//fflush(stdout);", stru, f_name, *dataoff, stru, f_name, stru, f_name);
    *dataoff += sizeof(int);
    //}

    break;
  case Ty_real:
    /*if (f->iskey) {
     sprintf(fbuf, "\nmemcpy(&(%s.%s), (char*)key.data+%d, sizeof(double));"
     "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(double));",
     stru, f_name, *keyoff,
     stru, f_name, *dataoff);
     *keyoff+=sizeof(double);
     *dataoff+=sizeof(double);
     }
     else { */
    sprintf(fbuf, "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(double));"
      "\n//printf(\"Retrieved %s.%s = %%f\\n\", %s.%s);"
      "\n//fflush(stdout);", stru, f_name, *dataoff, stru, f_name, stru, f_name);
    *dataoff += sizeof(double);
    //}
    break;
  case Ty_timestamp:
    /*if (f->iskey) {
     sprintf(fbuf, "\nmemcpy(&(%s.%s), (char*)key.data+%d, sizeof(struct timeval));"
     "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(struct timeval));",
     stru, f_name, *keyoff,
     stru, f_name, *dataoff);
     *keyoff+=sizeof(struct timeval);
     *dataoff+=sizeof(struct timeval);
     }   else */
    if (f->isTimekey) {
      sprintf(fbuf, "\nmemcpy(&(%s.%s), (char*)&atime, sizeof(atime));", stru,
          f_name);
    } else {
      sprintf(fbuf,
          "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(struct timeval));",
          stru, f_name, *dataoff);
      *dataoff += sizeof(struct timeval);
    }
    break;
  case Ty_iext:
    sprintf(
        fbuf,
        "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(struct iExt_));"
          "\n//printf(\"Retrieved  iExt %s.%s(%%d, %%d)\\n\", %s.%s.length, %s.%s.pt[1]);"
          "\n//fflush(stdout);", stru, f_name, *dataoff, stru, f_name, stru,
        f_name, stru, f_name);
    *dataoff += sizeof(struct iExt_);
    break;
  case Ty_rext:
    sprintf(
        fbuf,
        "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(struct rExt_));"
          "\n//printf(\"Retrieved  rExt %s.%s(%%d, %%d)\\n\", %s.%s.length, %s.%s.pt[1]);"
          "\n//fflush(stdout);", stru, f_name, *dataoff, stru, f_name, stru,
        f_name, stru, f_name);
    *dataoff += sizeof(struct rExt_);
    break;
  case Ty_cext:
    sprintf(
        fbuf,
        "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(struct cExt_));"
          "\n//printf(\"Retrieved  cExt %s.%s(%%d, %%d)\\n\", %s.%s.length, %s.%s.pt[1]);"
          "\n//fflush(stdout);", stru, f_name, *dataoff, stru, f_name, stru,
        f_name, stru, f_name);
    *dataoff += sizeof(struct cExt_);
    break;
  case Ty_text:
    sprintf(
        fbuf,
        "\nmemcpy(&(%s.%s), (char*)data.data+%d, sizeof(struct tExt_));"
          "\n//printf(\"Retrieved  tExt %s.%s(%%d, %%d)\\n\", %s.%s.length, %s.%s.pt[1]);"
          "\n//fflush(stdout);", stru, f_name, *dataoff, stru, f_name, stru,
        f_name, stru, f_name);
    *dataoff += sizeof(struct tExt_);
    break;
  case Ty_string:
    /*if (f->iskey) {
     sprintf(fbuf, "\nmemcpy(%s.%s, (char*)key.data+%d, %d);"
     "\n*(%s.%s+%d) = '\\0';"
     "\nmemcpy(%s.%s, (char*)data.data+%d, %d);"
     "\n*(%s.%s+%d) = '\\0';"
     , stru, f_name, *keyoff, f->size
     , stru, f_name, f->size
     , stru, f_name, *dataoff, f->size
     , stru, f_name, f->size);//string length
     *keyoff+=f->size;
     *dataoff+=f->size;
     }
     else { */
    sprintf(fbuf, "\nmemcpy(%s.%s, (char*)data.data+%d, %d);"
      "\n*(%s.%s+%d) = '\\0';", stru, f_name, *dataoff, f->size, stru, f_name,
        f->size);//string length
    *dataoff += f->size;
    //    }
    break;
  case Ty_record:
  default:
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "assignField2C");
    goto exit;
  }

  exit: return rc;
}

err_t assignKeyData2C(Ty_ty ty, char *stru, char *buf, tabindex_t idx) {
  SMLog::SMLOG(10, "In assignKeyData2C %s %s %s \n", ty->kind, stru, buf);
  err_t rc = ERR_NONE;
  A_list fields;
  //char fbuf[256];
  char fbuf[1024]; // sometimes there was a buffer overflow
  // so we had to extend this buffer.
  int dataoff = 0;
  int keyoff = 0;
  int nRecKeys;

  *buf = '\0';

  if (ty->kind != Ty_record) {
    rc = ERR_TUPLE_TYPE_REQUIRED;
    EM_error(0, rc, __LINE__, __FILE__, "assignKeyData2C");
    goto exit;
  }

  nRecKeys = 0;
  fields = ty->u.record;
  for (int i = 0; i < fields->length; i++) {
    Ty_field f = (Ty_field) getNthElementList(fields, i);
    /*if (idx == INDEX_RTREE && f->iskey) {
     sprintf(fbuf, "\n%s.%s = r_key.%s;", stru, S_name(f->name), recs[nRecKeys++]);
     }
     else {*/
    rc = assignField2C(f, stru, S_name(f->name), &keyoff, &dataoff, fbuf);
    //}
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "assignKeyData2C",
          "assignField2C");
      goto exit;
    }

    strcat(buf, fbuf);
  }

  exit: return rc;
}

err_t transLoad2C(S_table venv, char *target_handle, char *target_database,
    char *source_filename, E_enventry x, char *buf)
//		  Ty_ty ty, char *buf, tabscope_t kind, db_t dbtype)
{
  SMLog::SMLOG(10,
      "Entering transLoad2C: target_handle: %s, source_filename: %s",
      target_handle, source_filename);
  err_t rc = ERR_NONE;
  A_list fields;
  int dataoff = 0, keyoff = 0;
  int len, i, tuplesize[2] = { 0, 0 };
  char linebuf[MAX_STR_LEN];

  Ty_ty ty = x->u.var.ty;
  tabscope_t kind = x->u.var.scope;
  tabindex_t idx = (x->u.var.index == (A_index) 0) ? (tabindex_t) 1
      : x->u.var.index->kind;

  const char *flag = (idx == INDEX_BTREE) ? "0" : "DB_APPEND";

  fields = ty->u.record;

  for (i = 0; i < fields->length; i++) {
    Ty_field f = (Ty_field) getNthElementList(fields, i);
    switch (f->ty->kind) {
    case Ty_real:
      if (f->iskey)
        tuplesize[1] += sizeof(double);
      tuplesize[0] += sizeof(double);
      break;
    case Ty_timestamp:
      if (f->iskey)
        tuplesize[1] += sizeof(struct timeval);
      tuplesize[0] += sizeof(struct timeval);
      break;
    case Ty_int:
      if (f->iskey)
        tuplesize[1] += sizeof(int);
      tuplesize[0] += sizeof(int);
      break;
    case Ty_string:
      if (f->iskey)
        tuplesize[1] += f->size;
      tuplesize[0] += f->size;
      break;
    case Ty_ref:
      //      if (i!=fields->length-1) {
      if (f->iskey)
        tuplesize[1] += sizeof(int);
      tuplesize[0] += sizeof(int);
      //      }
      break;
    default:
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__, "transLoad2C");
      goto exit;
    }
  }

  //Hetal: key size when ext function is used
  if (x->u.var.index != (A_index) 0) {
    if (x->u.var.index->func != (S_symbol) 0 && x->u.var.index->len > 0) {
      tuplesize[1] = x->u.var.index->len;
    } else if (x->u.var.index->func != (S_symbol) 0) {
      //look up func and assign its size to be equal to the key size
      E_enventry f = (E_enventry) S_look(venv, x->u.var.index->func);
      if (!f || f->kind != E_extEntry) {
        rc = ERR_INVALID_INDEX_SPEC;
        EM_error(0, rc, __LINE__, __FILE__, "External function not found");
        goto exit;
      }
      switch (f->u.ext.result->kind) {
      case Ty_int:
        tuplesize[1] = sizeof(int);
        break;
      case Ty_real:
        tuplesize[1] = sizeof(double);
        break;
      case Ty_timestamp:
        tuplesize[1] = sizeof(timestamp);
        break;
      case Ty_iext:
        tuplesize[1] = sizeof(struct iExt_);
        break;
      case Ty_rext:
        tuplesize[1] = sizeof(struct rExt_);
        break;
      case Ty_cext:
        tuplesize[1] = sizeof(struct cExt_);
        break;
      case Ty_text:
        tuplesize[1] = sizeof(struct tExt_);
        break;
      case Ty_string:
        tuplesize[1] = f->u.ext.size;
        break;
      default:
        tuplesize[1] = sizeof(int); //should not happen
        break;
      }
    }
  }

  //TODO: Find out the reason for introducing variable ``bPoint''. 32 bit machine, 64 bit machine 32 bit code, 64 bit machine 64 bit code?
  sprintf(buf, "\n{\nFILE *_adl_load = fopen(\"%s\", \"rt\");"
    "\nchar _adl_load_buf[40960], *tok;"
    "\nchar loadkeybuf[%d], loaddatabuf[%d];"
    "\nint _adl_line_no=0;"
    //"\nchar bPoint =0;"
    "\nif (!_adl_load) {"
    "\nprintf(\"can not open file %s.\\n\");"
    "\nexit(1);"
    "\n}"
    "\nmemset(&key, 0, sizeof(key));"
    "\nmemset(&data, 0, sizeof(data));"
    "\nkey.data = loadkeybuf;"
    "\ndata.data = loaddatabuf;"
    "\nkey.size = %d;"
    "\ndata.size = %d;", os_filename(source_filename), tuplesize[1] + 1,
      tuplesize[0] + 1, os_filename(source_filename), tuplesize[1],
      tuplesize[0]);
  /*if (idx == INDEX_RTREE) {
    sprintf(linebuf, "\nif (key.size == 2 * sizeof(long)){"
      "\n\tkey.size *= 2;"
      "\n\tbPoint = 1;"
      "\n}");
    strcat(buf, linebuf);
  }*/
  sprintf(linebuf, "\nwhile (fgets(_adl_load_buf, 40959, _adl_load)) {");
  strcat(buf, linebuf);
  len = (kind == TAB_MEMORY) ? fields->length - 1 : fields->length;
  for (i = 0; i < len; i++) {
    Ty_field f = (Ty_field) getNthElementList(fields, i);

    switch (f->ty->kind) {
    case Ty_real:
      if (f->iskey && (x->u.var.index == (A_index) 0 || x->u.var.index->func
          == (S_symbol) 0)) {
        sprintf(linebuf, "\n*(double*)((char*)key.data+%d) = atof(tok);"
          "\n*(double*)((char*)data.data+%d) = atof(tok);", keyoff, dataoff);
        keyoff += sizeof(double);
        dataoff += sizeof(double);
      } else {
        sprintf(linebuf, "\n*(double*)((char*)data.data+%d) = atof(tok);",
            dataoff);
        dataoff += sizeof(double);
      }
      break;
    case Ty_timestamp:
      if (f->iskey && (x->u.var.index == (A_index) 0 || x->u.var.index->func
          == (S_symbol) 0)) {
        sprintf(linebuf, "\nmemset((char*)key.data+%d, 0, %d);"
          "\nmemcpy((char*)key.data+%d, &getTimeval(tok), %d);"
          "\nmemset((char*)data.data+%d, 0, %d);"
          "\nmemcpy((char*)data.data+%d, &getTimeval(tok), %d);", keyoff,
            sizeof(struct timeval), keyoff, sizeof(struct timeval),
            sizeof(struct timeval), dataoff, sizeof(struct timeval), dataoff,
            sizeof(struct timeval), sizeof(struct timeval));
        keyoff += sizeof(struct timeval);
        dataoff += sizeof(struct timeval);
      } else {
        sprintf(linebuf, "\nmemset((char*)data.data+%d, 0, %d);"
          "\nmemcpy((char*)data.data+%d, &getTimeval(tok), %d);", dataoff,
            sizeof(struct timeval), dataoff, sizeof(struct timeval),
            sizeof(struct timeval));
        dataoff += sizeof(struct timeval);
      }
      break;
    case Ty_ref:
    case Ty_int:
      if (f->iskey && (x->u.var.index == (A_index) 0 || x->u.var.index->func
          == (S_symbol) 0)) {
        sprintf(linebuf, "\n*(int*)((char*)key.data+%d) = atoi(tok);"
          "\n*(int*)((char*)data.data+%d) = atoi(tok);", keyoff, dataoff);
        keyoff += sizeof(int);
        dataoff += sizeof(int);
      } else {
        sprintf(linebuf, "\n*(int*)((char*)data.data+%d) = atoi(tok);", dataoff);
        dataoff += sizeof(int);
      }
      break;
    case Ty_string:
      if (f->iskey && (x->u.var.index == (A_index) 0 || x->u.var.index->func
          == (S_symbol) 0)) {
        sprintf(linebuf, "\nmemset((char*)key.data+%d, 0, %d);"
          "\nmemcpy((char*)key.data+%d, tok, strlen(tok)<%d?strlen(tok):%d);"
          "\nmemset((char*)data.data+%d, 0, %d);"
          "\nmemcpy((char*)data.data+%d, tok, strlen(tok)<%d?strlen(tok):%d);",
            keyoff, f->size, keyoff, f->size, f->size, dataoff, f->size,
            dataoff, f->size, f->size);
        keyoff += f->size;
        dataoff += f->size;
      } else {
        sprintf(linebuf, "\nmemset((char*)data.data+%d, 0, %d);"
          "\nmemcpy((char*)data.data+%d, tok, strlen(tok)<%d?strlen(tok):%d);",
            dataoff, f->size, dataoff, f->size, f->size);//string length
        dataoff += f->size;
      }
      break;
    default:
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__, "transLoad2C");
      goto exit;
    }

    if (i == 0) {
      strcat(buf, "\n_adl_line_no++;"
        "\ntok = csvtok(_adl_load_buf, \",\\n\");");
    } else {
      strcat(buf, "\ntok = csvtok(NULL, \",\\n\");");
    }
    strcat(buf, "\nif (!tok) {"
      "\nprintf(\"data format error at line %d\\n\", _adl_line_no);"
      "\ngoto exit;"
      "\n}");
    strcat(buf, linebuf);
  }

  if ((x->u.var.index != (A_index) 0) && (x->u.var.index->func != (S_symbol) 0)) {
    rc = assignFuncBtreeKey(venv, x, buf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transLoad2C",
          "assignFuncBtreeKey");
      goto exit;
    }
  }

  if (kind == TAB_MEMORY) {
    sprintf(linebuf, "\nif ((rc = %s->put(%s, &key, &data, DB_APPEND))!=0) {"
      "\nexit(rc);"
      "\n}"
      "\n} /* end of while */"
      "\nfclose(_adl_load);"
      "\n}", target_handle, target_handle, target_handle, target_handle);
  } else if (idx == INDEX_RTREE) {
    sprintf(linebuf, //"\nif (bPoint == 1){"
      //"\n\tmemcpy((char*)key.data+2*sizeof(long), key.data, 2*sizeof(long));"
      //"\n}"
      "\nif ((rc = %s->put(%s, &key, &data, 0))!=0) {"
      "\nprintf(\"RTREE Error! Error code = %d\", rc);"
      "\nexit(rc);"
      "\n}"
      "\n} /* end of while */"
      "\nfclose(_adl_load);"
      "\n}", target_handle, target_handle);

  } else {
    sprintf(linebuf, "\nif ((rc = %s->put(%s, NULL, &key, &data, %s))!=0) {"
      "\n%s->err(%s, rc, \"%s\");"
      "\nexit(rc);"
      "\n}"
      "\n} /* end of while */"
      "\nfclose(_adl_load);"
      "\n}"
      "\n%s->sync(%s, 0);", target_handle, target_handle, flag, target_handle,
        target_handle, target_database, target_handle, target_handle);
  }
  strcat(buf, linebuf);

  exit: return rc;
}

err_t transInsert2C(S_table venv, A_qun target, char *stru,
    char *target_handle, E_enventry x, // target env
    int mode, char *buf, const char* queryName, int in_aggr) {
  SMLog::SMLOG(10, "Entering transInsert2C queryName: %s, target_case: %s",
      queryName, target_handle);
  DBUG_ENTER("transInsert2C");
  err_t rc = ERR_NONE;
  A_list fields;
  int dataoff = 0;
  int keyoff = 0;
  char fbuf[MAX_STR_LEN];
  int nRecKeys = 0; /* number of keys for rtree, valid value is either
   2 (point) or 4 (rectangle)*/
  int isView = 0;
  char argbuf[1024];
  argbuf[0] = '\0';
  char temp[80];
  char fName[80];
  /* rectangle defined in rtree.h
   typedef struct rectangle
   {
   long x_ul,y_ul,x_lr,y_lr;
   } Rect;
   */

  Ty_ty ty;
  tabscope_t kind;
  tabindex_t idx;
  bool target_case = false;
  if (x->kind == E_streamEntry) {
    ty = x->u.stream.ty;
    idx = INDEX_STREAM;
  } else {
    ty = x->u.var.ty;
    kind = x->u.var.scope;
    idx = (x->u.var.index == (A_index) 0) ? (tabindex_t) 1
        : x->u.var.index->kind;
    isView = x->u.var.isBuffer;
  }

  if (isESL() && (x->kind == E_streamEntry) && x->u.stream.target != NULL) {
    target_case = true;
  }

  const char *flag = (idx == INDEX_BTREE) ? "0" : "DB_APPEND";

  if (ty->kind != Ty_record) {
    rc = ERR_TUPLE_TYPE_REQUIRED;
    EM_error(0, rc, __LINE__, __FILE__, "transInsert2C");
    goto exit;
  }

  *buf = '\0';
  if ((isESL() && in_aggr != 1 && mode == USE_STDOUT) || target_case) {
    char tempBuf[300];
    tempBuf[0] = '\0';

    if (isESL() && in_aggr != 1 && mode == USE_STDOUT) {
      sprintf(tempBuf, "\nchar formatField_%d[200];"
        "\nformatField_%d[0] = 0;"
        "\nchar formatTuple_%d[4096];"
        "\nformatTuple_%d[0] = 0;"
        "\nstrcat(formatTuple_%d, \"o||stdout_%s\");"
        "\nstrcat(formatTuple_%d, \"||\");", UID(ty), UID(ty), UID(ty),
          UID(ty), UID(ty), strdup(queryName), UID(ty));
    } else if (target_case) {
      sprintf(tempBuf, "\nchar formatField_%d[200];"
        "\nformatField_%d[0] = 0;"
        "\nchar formatTuple_%d[4096];"
        "\nformatTuple_%d[0] = 0;", UID(ty), UID(ty), UID(ty), UID(ty));
    }
    strcpy(buf, tempBuf);
  } else if (mode != USE_STDOUT) {
    strcpy(buf, "\nmemset(&key, 0, sizeof(key));"
      "\nmemset(&data, 0, sizeof(data));"
      "\ndata.data = datadata;"
      "\nkey.data = keydata;");
  }

  fields = ty->u.record;

  // copy or print each field
  for (int i = 0; i < fields->length; i++) {
    Ty_field f = (Ty_field) getNthElementList(fields, i);
    bool last = (i == fields->length - 1);

    switch (f->ty->kind) {
    case Ty_iext:
      if (mode == USE_STDOUT || target_case) {
        if (isESL() && in_aggr != 1) {
          sprintf(fbuf, "\nprintf(\"len->%%%dd \", %s.%s.length);"
            "\nformatField_%d[0] = 0;"
            "\nsprintf(formatField_%d, \"len->%%%dd%s\", %s.%s.length);"
            "\nstrcat(formatTuple_%d, formatField_%d);", 3, stru, S_name(
              f->name), UID(ty), UID(ty), 3, (target_case && !last) ? ","
              : (target_case ? "" : "$"), stru, S_name(f->name), UID(ty),
              UID(ty));
        } else {
          sprintf(fbuf, "\nprintf(\"len->%%%dd \", %s.%s.length);", 3, stru,
              S_name(f->name));
        }
      } else {
        sprintf(fbuf,
            "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(struct iExt_));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(struct iExt_);
      }
      break;
    case Ty_rext:
      if (mode == USE_STDOUT || target_case) {
        if (isESL() && in_aggr != 1) {
          sprintf(fbuf, "\nprintf(\"len->%%%dd \", %s.%s.length);"
            "\nformatField_%d[0] = 0;"
            "\nsprintf(formatField_%d, \"len->%%%dd%s\", %s.%s.length);"
            "\nstrcat(formatTuple_%d, formatField_%d);", 3, stru, S_name(
              f->name), UID(ty), UID(ty), 3, (target_case && !last) ? ","
              : (target_case ? "" : "$"), stru, S_name(f->name), UID(ty),
              UID(ty));
        } else {
          sprintf(fbuf, "\nprintf(\"len->%%%dd \", %s.%s.length);", 3, stru,
              S_name(f->name));
        }
      } else {
        sprintf(fbuf,
            "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(struct rExt_));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(struct rExt_);
      }
      break;
    case Ty_cext:
      if (mode == USE_STDOUT || target_case) {
        if (isESL() && in_aggr != 1) {
          sprintf(fbuf, "\nprintf(\"len->%%%dd \", %s.%s.length);"
            "\nformatField_%d[0] = 0;"
            "\nsprintf(formatField_%d, \"len->%%%dd%s\", %s.%s.length);"
            "\nstrcat(formatTuple_%d, formatField_%d);", 3, stru, S_name(
              f->name), UID(ty), UID(ty), 3, (target_case && !last) ? ","
              : (target_case ? "" : "$"), stru, S_name(f->name), UID(ty),
              UID(ty));
        } else {
          sprintf(fbuf, "\nprintf(\"len->%%%dd \", %s.%s.length);", 3, stru,
              S_name(f->name));
        }
      } else {
        sprintf(fbuf,
            "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(struct cExt_));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(struct cExt_);
      }
      break;
    case Ty_text:
      if (mode == USE_STDOUT || target_case) {
        if (isESL() && in_aggr != 1) {
          sprintf(fbuf, "\nprintf(\"len->%%%dd \", %s.%s.length);"
            "\nformatField_%d[0] = 0;"
            "\nsprintf(formatField_%d, \"len->%%%dd%s\", %s.%s.length);"
            "\nstrcat(formatTuple_%d, formatField_%d);", 3, stru, S_name(
              f->name), UID(ty), UID(ty), 3, (target_case && !last) ? ","
              : (target_case ? "" : "$"), stru, S_name(f->name), UID(ty),
              UID(ty));
        } else {
          sprintf(fbuf, "\nprintf(\"len->%%%dd \", %s.%s.length);", 3, stru,
              S_name(f->name));
        }
      } else {
        sprintf(fbuf,
            "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(struct tExt_));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(struct tExt_);
      }
      break;
    case Ty_nil:
      if (mode == USE_STDOUT) {
        if (isESL() && in_aggr != 1) {
          sprintf(fbuf, "\nprintf(\"NULL \");"
            "\nstrcat(formatTuple_%d, \"NULL\");", UID(ty));
        } else {
          sprintf(fbuf, "\nprintf(\"NULL \");");
        }
        break;
      }
      /* fall through */
    case Ty_ref:
    case Ty_int:
    case Ty_record:
      if (mode == USE_STDOUT || target_case) {
        if (isESL() && in_aggr != 1) {
          sprintf(fbuf, "\nprintf(\"%%%dd \", %s.%s);"
            "\nformatField_%d[0] = 0;"
            "\nsprintf(formatField_%d, \"%%%dd%s\", %s.%s);"
            "\nstrcat(formatTuple_%d, formatField_%d);", (f->size == 0) ? 10
              : f->size, stru, S_name(f->name), UID(ty), UID(ty),
              (f->size == 0) ? 10 : f->size, (target_case && !last) ? ","
                  : (target_case ? "" : "$"), stru, S_name(f->name), UID(ty),
              UID(ty));
        } else {
          sprintf(fbuf, "\nprintf(\"%%%dd \", %s.%s);", (f->size == 0) ? 10
              : f->size, stru, S_name(f->name));
        }
      } else if (f->iskey) {
        if (idx == INDEX_RTREE) {
          sprintf(fbuf, "\nr_key.%s =(long)%s.%s;", recs[nRecKeys++], stru,
              S_name(f->name));
          strcat(buf, fbuf);
        } else if (x->u.var.index == (A_index) 0 || x->u.var.index->func
            == (S_symbol) 0) {
          sprintf(fbuf, "\nmemcpy((char*)key.data+%d, &(%s.%s), sizeof(int));",
              keyoff, stru, S_name(f->name));
          keyoff += sizeof(int);
          strcat(buf, fbuf);
        } else if (x->u.var.index != (A_index) 0 && x->u.var.index->func
            != (S_symbol) 0) {
          sprintf(fbuf, "\nint field_%d = %s.%s;", i, stru, S_name(f->name));
          strcat(buf, fbuf);
          sprintf(temp, "field_%d", i);
          if (argbuf[0] != '\0') {
            strcat(argbuf, " ,");
          }
          strcat(argbuf, temp);
        }
        sprintf(fbuf, "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(int));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(int);
      } else {
        sprintf(fbuf, "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(int));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(int);
      }
      break;
    case Ty_real:
      if (mode == USE_STDOUT || target_case) {
        if (isESL() && in_aggr != 1) {
          sprintf(fbuf, "\nprintf(\"%%%df \", %s.%s);"
            "\nformatField_%d[0] = 0;"
            "\nsprintf(formatField_%d, \"%%%df%s\", %s.%s);"
            "\nstrcat(formatTuple_%d, formatField_%d);", (f->size == 0) ? 10
              : f->size, stru, S_name(f->name), UID(ty), UID(ty),
              (f->size == 0) ? 10 : f->size, (target_case && !last) ? ","
                  : (target_case ? "" : "$"), stru, S_name(f->name), UID(ty),
              UID(ty));
        } else {
          sprintf(fbuf, "\nprintf(\"%%%df \", %s.%s);", (f->size == 0) ? 10
              : f->size, stru, S_name(f->name));
        }
      } else if (f->iskey) {
        if (x->u.var.index == (A_index) 0 || x->u.var.index->func
            == (S_symbol) 0) {
          sprintf(fbuf,
              "\nmemcpy((char*)key.data+%d, &(%s.%s), sizeof(double));",
              keyoff, stru, S_name(f->name));
          keyoff += sizeof(double);
          strcat(buf, fbuf);
        } else if (x->u.var.index != (A_index) 0 && x->u.var.index->func
            != (S_symbol) 0) {
          sprintf(fbuf, "\ndouble field_%d = %s.%s;", i, stru, S_name(f->name));
          strcat(buf, fbuf);
          sprintf(temp, "field_%d", i);
          if (argbuf[0] != '\0') {
            strcat(argbuf, ", ");
          }
          strcat(argbuf, temp);
        }
        sprintf(fbuf,
            "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(double));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(double);
      } else {
        sprintf(fbuf,
            "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(double));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(double);
      }
      break;
    case Ty_timestamp:
      if (mode == USE_STDOUT || target_case) {
        if (isESL() && in_aggr != 1) {
          sprintf(fbuf, "\nchar ts_%d[40];"
            "\nstruct tm* temp_%d = localtime(&%s.%s.tv_sec);"
            "\nstrftime(ts_%d, 40, \"%s\", temp_%d);"
            "\nprintf(\"\\t%%s.%%d\\t \", ts_%d, %s.%s.tv_usec);"
            "\nformatField_%d[0] = 0;"
            "\nsprintf(formatField_%d, \"%%s.%%d%s\", ts_%d, %s.%s.tv_usec);"
            "\nstrcat(formatTuple_%d, formatField_%d);", i, i, stru, S_name(
              f->name), i, TIMESTAMP_FORMAT, i, i, stru, S_name(f->name),
              UID(ty), UID(ty), (target_case && !last) ? ","
                  : (target_case ? "" : "$"), i, stru, S_name(f->name),
              UID(ty), UID(ty)
          );
        } else {
          sprintf(fbuf, "\nchar ts_%d[40];"
            "\nstruct tm* temp_%d = localtime(&%s.%s.tv_sec);"
            "\nstrftime(ts_%d, 40, \"%s\", temp_%d);"
            "\nprintf(\"\\t%%s.%%d\\t \", ts_%d, %s.%s.tv_usec);", i, i, stru,
              S_name(f->name), i, TIMESTAMP_FORMAT, i, i, stru, S_name(f->name));
        }
      } else if (f->isTimekey) {
        sprintf(fbuf, "\nmemcpy((char*)&atime, &(%s.%s), sizeof(atime));",
            stru, S_name(f->name));

      } else if (f->iskey) {
        if (x->u.var.index == (A_index) 0 || x->u.var.index->func
            == (S_symbol) 0) {
          sprintf(
              fbuf,
              "\nmemcpy((char*)key.data+%d, &(%s.%s), sizeof(struct timeval));",
              keyoff, stru, S_name(f->name));
          keyoff += sizeof(struct timeval);
          strcat(buf, fbuf);
        } else if (x->u.var.index != (A_index) 0 && x->u.var.index->func
            != (S_symbol) 0) {
          sprintf(fbuf, "\ntimestamp field_%d;"
            "\nfield_%d.tv_sec = %s.%s.tv_sec;"
            "\nfield_%d.tv_usec = %s.%s.tv_usec;", i, i, stru, S_name(f->name),
              i, stru, S_name(f->name));
          strcat(buf, fbuf);
          sprintf(temp, "field_%d", i);
          if (argbuf[0] != '\0') {
            strcat(argbuf, " ,");
          }
          strcat(argbuf, temp);
        }
        sprintf(fbuf,
            "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(struct timeval));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(struct timeval);
      } else {
        sprintf(fbuf,
            "\nmemcpy((char*)data.data+%d, &(%s.%s), sizeof(struct timeval));",
            dataoff, stru, S_name(f->name));
        dataoff += sizeof(struct timeval);
      }
      break;
    case Ty_string:
      if (mode == USE_STDOUT || target_case) {
        if (isESL() && in_aggr != 1) {
          sprintf(fbuf, "\nprintf(\"\\t%%s\\t \", %s.%s);"
            "\nformatField_%d[0] = 0;"
            "\nsprintf(formatField_%d, \"%%s%s\", %s.%s);"
            "\nstrcat(formatTuple_%d, formatField_%d);", stru, S_name(f->name),
              UID(ty), UID(ty), (target_case && !last) ? ","
                  : (target_case ? "" : "$"), stru, S_name(f->name), UID(ty),
              UID(ty));
        } else {
          sprintf(fbuf, "\nprintf(\"\\t%%s\\t \", %s.%s);", stru, S_name(
              f->name));
        }
      } else if (f->iskey) {
        if (x->u.var.index == (A_index) 0 || x->u.var.index->func
            == (S_symbol) 0) {
          sprintf(fbuf, "\nmemset((char*)key.data+%d, 0, %d);"
            "\nmemcpy((char*)key.data+%d, %s.%s, %d);", keyoff, f->size,
              keyoff, stru, S_name(f->name),
              //S_name(f->name)
              f->size);//string length
          keyoff += f->size;
          strcat(buf, fbuf);
        } else if (x->u.var.index != (A_index) 0 && x->u.var.index->func
            != (S_symbol) 0) {
          sprintf(fbuf, "\nchar field_%d[%d];"
            "\nmemcpy(field_%d, %s.%s, %d);", i, f->size, i, stru, S_name(
              f->name), f->size);
          strcat(buf, fbuf);
          sprintf(temp, "field_%d", i);
          if (argbuf[0] != '\0') {
            strcat(argbuf, " ,");
          }
          strcat(argbuf, temp);
        }
        sprintf(fbuf, "\nmemcpy((char*)data.data+%d, %s.%s, %d);", dataoff,
            stru, S_name(f->name), f->size);//string length
        dataoff += f->size;
      } else {
        sprintf(fbuf, "\nmemcpy((char*)data.data+%d, %s.%s, %d);", dataoff,
            stru, S_name(f->name), f->size);//string length
        dataoff += f->size;
      }
      break;

    default:
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__, "transInsert2C");
      goto exit;
    }
    strcat(buf, fbuf);
  } //end for copy (or print) field
  if (nRecKeys == 2) {
    sprintf(fbuf, "\nr_key.x_lr = r_key.x_ul;\nr_key.y_lr = r_key.y_ul;");
    strcat(buf, fbuf);
  }

  if (x->kind == E_varEntry && x->u.var.index != (A_index) 0
      && x->u.var.index->func != (S_symbol) 0) {
    //can't find proper var, thus doing this in-place
    /*rc = assignFuncBtreeKey(venv, x, buf);
     if (rc) {
     EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "transInsert2C", "assignFuncBtreeKey");
     goto exit;
     }*/

    E_enventry f = (E_enventry) S_look(venv, x->u.var.index->func);
    char callbuf[1024];
    char retType[80];

    if (!f && !f->kind == E_extEntry) {
      rc = ERR_INVALID_INDEX_SPEC;
      EM_error(0, rc, __LINE__, __FILE__, "External func not found.");
      goto exit;
    }
    switch (f->u.ext.result->kind) {
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
    case Ty_iext:
      sprintf(retType, "struct iExt_");
      break;
    case Ty_rext:
      sprintf(retType, "struct rExt_");
      break;
    case Ty_cext:
      sprintf(retType, "struct cExt_");
      break;
    case Ty_text:
      sprintf(retType, "struct tExt_");
      break;
    default:
      rc = ERR_DATATYPE;
      EM_error(0, rc, __LINE__, __FILE__, "type not supported in UPDATE");
      goto exit;
    }
    sprintf(fbuf, "\nif(_adl_func_%d == NULL) { /*8*/"
      "\n_adl_func_%d = (%s(*)(%s))_adl_dlm(\"%s\", \"%s\");"
      "\n}", UID(f), UID(f), retType, argbuf, S_name(f->u.ext.externlib),
        S_name(f->u.ext.actual));
    strcat(buf, fbuf);

    if (isESL()) {
      sprintf(
          fbuf,
          "\nif(_adl_func_%d == NULL) {"
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"Error in query %s: external func not found\");"
            "\nreturn s_failure;"
            "\n}", UID(f), getUserName(), getQueryName());
    } else if (isESLAggr()) {
      sprintf(
          fbuf,
          "\nif(_adl_func_%d == NULL) {"
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"Error in Aggregate %s: external func not found\");"
            "\nreturn;"
            "\n}", UID(f), getUserName(), getAggrName());
    } else {
      sprintf(fbuf, "\nif(_adl_func_%d == NULL) {"
        "\nfprintf(stderr, \"ERR: external func not found\\n\");"
        "\nexit(1);"
        "\n}", UID(f));
    }
    strcat(buf, fbuf);

    sprintf(temp, "%d", x->u.var.index->len);
    switch (f->u.ext.result->kind) {
    case Ty_int:
      sprintf(callbuf, "\nint ret_%d = (int)((*_adl_func_%d)(%s));", UID(f),
          UID(f), argbuf);
      sprintf(retType, "sizeof(int)");
      sprintf(fName, "&ret_%d", UID(f));
      break;
    case Ty_real:
      sprintf(callbuf, "\ndouble ret_%d = (double)((*_adl_func_%d)(%s));",
          UID(f), UID(f), argbuf);
      sprintf(retType, "sizeof(double)");
      sprintf(fName, "&ret_%d", UID(f));
      break;
    case Ty_string:
      sprintf(callbuf, "\nchar* ret_%d = (char*)((*_adl_func_%d)(%s));",
          UID(f), UID(f), argbuf);
      sprintf(retType, "%d", x->u.ext.size);
      sprintf(fName, "ret_%d", UID(f));
      break;
    case Ty_timestamp:
      sprintf(callbuf,
          "\ntimestamp ret_%d = (timestamp)((*_adl_func_%d)(%s));", UID(f),
          UID(f), argbuf);
      sprintf(retType, "sizeof(timestamp)");
      sprintf(fName, "&ret_%d", UID(f));
      break;
    case Ty_iext:
      sprintf(callbuf,
          "\nstruct iExt_ ret_%d = (struct iExt_)((*_adl_func_%d)(%s));",
          UID(f), UID(f), argbuf);
      sprintf(retType, "sizeof(struct iExt_)");
      sprintf(fName, "&ret_%d", UID(f));
      break;
    case Ty_rext:
      sprintf(callbuf,
          "\nstruct rExt_ ret_%d = (struct rExt_)((*_adl_func_%d)(%s));",
          UID(f), UID(f), argbuf);
      sprintf(retType, "sizeof(struct rExt_)");
      sprintf(fName, "&ret_%d", UID(f));
      break;
    case Ty_cext:
      sprintf(callbuf,
          "\nstruct cExt_ ret_%d = (struct cExt_)((*_adl_func_%d)(%s));",
          UID(f), UID(f), argbuf);
      sprintf(retType, "sizeof(struct cExt_)");
      sprintf(fName, "&ret_%d", UID(f));
      break;
    case Ty_text:
      sprintf(callbuf,
          "\nstruct tExt_ ret_%d = (struct tExt_)((*_adl_func_%d)(%s));",
          UID(f), UID(f), argbuf);
      sprintf(retType, "sizeof(struct tExt_)");
      sprintf(fName, "&ret_%d", UID(f));
      break;
    default:
      rc = ERR_DATATYPE;
      EM_error(0, rc, __LINE__, __FILE__, "type not supported in UPDATE");
      goto exit;
    }
    sprintf(fbuf, "%s\nmemcpy((char*)key.data, %s, %s);", callbuf, fName,
        (x->u.var.index->len > 0) ? temp : retType);
    strcat(buf, fbuf);
  }

  if (x->kind == E_varEntry && x->u.var.index != (A_index) 0) {
    if (x->u.var.index->func != (S_symbol) 0 && x->u.var.index->len > 0) {
      keyoff = x->u.var.index->len;
    } else if (x->u.var.index->func != (S_symbol) 0) {
      //look up func and assign its size to be equal to the key size
      E_enventry f = (E_enventry) S_look(venv, x->u.var.index->func);
      if (!f || f->kind != E_extEntry) {
        rc = ERR_INVALID_INDEX_SPEC;
        EM_error(0, rc, __LINE__, __FILE__, "External function not found");
        goto exit;
      }
      switch (f->u.ext.result->kind) {
      case Ty_int:
        keyoff = sizeof(int);
        break;
      case Ty_real:
        keyoff = sizeof(double);
        break;
      case Ty_timestamp:
        keyoff = sizeof(timestamp);
        break;
      case Ty_iext:
        keyoff = sizeof(struct iExt_);
        break;
      case Ty_rext:
        keyoff = sizeof(struct rExt_);
        break;
      case Ty_cext:
        keyoff = sizeof(struct cExt_);
        break;
      case Ty_text:
        keyoff = sizeof(struct tExt_);
        break;
      case Ty_string:
        keyoff = f->u.ext.size;
        break;
      default:
        keyoff = sizeof(int); //should not happen
        break;
      }
    }
  }

  switch (mode) {
  case USE_STDOUT:
    if (isESL() && in_aggr != 1) {
      sprintf(fbuf, "\nprintf(\"\\n\");"
        "\nstrcat(formatTuple_%d, \"\\n\");"
        "\ncDBT cdbt(formatTuple_%d, 4096);"
        "\nbm->put(\"stdout_%s\", &cdbt);", UID(ty), UID(ty), queryName);
    } else {
      sprintf(fbuf, "\nprintf(\"\\n\");");
    }
    strcat(buf, fbuf);
    break;
  case USE_TEMP:
    sprintf(fbuf, "\ndata.size = %d;"
      "\nkey.size = %d;"
      "\ninsertTemp(%d, _rec_id, &key, &data);", dataoff, keyoff, UID(target));
    strcat(buf, fbuf);
    break;
  case USE_DIRECT:
    if (target_case) {
      sprintf(fbuf, "\nprintf(\"\\n\");"
        "\nstrcat(formatTuple_%d, \"\\n\");"
        "\ncDBT cdbt(formatTuple_%d, 4096);"
        "\nbm->put(\"%s\", &cdbt);", UID(ty), UID(ty), target_handle);
      strcat(buf, fbuf);
    } else if (idx == INDEX_STREAM) {
      //printf("but here stream, type %s, %d\n", S_name(x->u.stream.sname), x->u.stream.tk);
      if (x->u.stream.tk == tk_internal) { // internal timekey
        // set time
        sprintf(fbuf, "\ngettimeofday(&atime,NULL);");
        strcat(buf, fbuf);
      }
      //Don't understand why EM_error is being used below, instead of adlabort -- Hetal
      sprintf(fbuf, "\ndata.size = %d;"
        "\nkey.size = %d;"
        "\nbuffer *%s = bm->lookup(\"%s\");"
        "\nif (!%s){"
        "\nEM_error(0, (err_t)rc, __LINE__, __FILE__, \"Buffer %s not found\");"
        "\nrc = s_failure;"
        "\ngoto doClose;"
        "\n}"
        "\nelse if ((rc=%s->put(&data, &atime, &key)) != 0){"
        "\nEM_error(0, (err_t)rc, __LINE__, __FILE__, \"%s->put()\");"
        "\nrc = s_failure;"
        "\ngoto doClose;"
        "\n}", dataoff, keyoff, target_handle, target_handle, target_handle,
          target_handle, target_handle, target_handle);
      strcat(buf, fbuf);
    } else if (isView == 1) { // WindowBuf
      char updateString[4096];
      sprintf(updateString, "if(%s->getWinType() == _ADL_WIN_ROW) {"
        "\n%s->updateTupleID();"
        "\n}"
        "\nelse if(%s->getWinType() == _ADL_WIN_TIME) {"
        "\nif(atime.tv_sec == 0) {"
        "\nstruct timeval tv1;"
        "\ntv1 = __timeofday();"
        "\natime.tv_sec = tv1.tv_sec;"
        "\natime.tv_usec = tv1.tv_usec;"
        "\n}"
        "\n%s->updateTupleID(&atime);"
        "\n}", target_handle, target_handle, target_handle, target_handle);
      sprintf(fbuf, "\ndata.size = %d;"
        "\nkey.size = %d;"
        "\nif(%s == NULL) {"
        "\n%s = (windowBuf*)bm->lookup(\"%s\");"
        "\nif(%s == NULL) {", dataoff, keyoff, target_handle, target_handle,
          target_handle, target_handle);
      strcat(buf, fbuf);

      if (isESL()) {
        sprintf(
            fbuf,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: bm->lookup(), windowBuf not found\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            fbuf,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: bm->lookup(), windowBuf not found\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(fbuf, "\nadlabort(rc, \"bm->lookup(), windowBuf not found\");"
          "\n}");
      }
      strcat(buf, fbuf);

      sprintf(fbuf, "\n}"
        "\n%s"
        "\nwhile(%s->hasExpired()) %s->pop();"
        "\nif((rc = %s->put(&key, &data))!=0) {", updateString, target_handle,
          target_handle, target_handle);
      strcat(buf, fbuf);

      if (isESL()) {
        sprintf(
            fbuf,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: windowBuf->put()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            fbuf,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: windowBuf->put()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(fbuf, "\nadlabort(rc, \"windowBuf->put()\");"
          "\n}");
      }
      strcat(buf, fbuf);
    } else if (kind == TAB_WINDOW) {
      sprintf(fbuf, "\ndata.size = %d;"
        "\nkey.size = %d;"
        "\nif ((rc = status->win->put(&key, &data))!=0) {", dataoff, keyoff);
      strcat(buf, fbuf);

      if (isESL()) {
        sprintf(
            fbuf,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->put()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            fbuf,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->put()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(fbuf, "\nadlabort(rc, \"IM_REL->put()\");"
          "\n}");
      }
      strcat(buf, fbuf);
    } else if (kind == TAB_MEMORY) {
      sprintf(fbuf, "\ndata.size = %d;"
        "\nkey.size = %d;"
        "\nif ((rc = %s->put(%s, &key, &data, DB_APPEND))!=0) {", dataoff,
          keyoff, target_handle, target_handle);
      strcat(buf, fbuf);

      if (isESL()) {
        sprintf(
            fbuf,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->put()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            fbuf,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->put()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(fbuf, "\nadlabort(rc, \"IM_REL->put()\");"
          "\n}");
      }
      strcat(buf, fbuf);
    } else if (idx == INDEX_RTREE) { // R_TREE
      sprintf(fbuf, "\ndata.size = %d;"
        "\nkey.size = sizeof(Rect);"
        "\nkey.data = &r_key;"
        "\nif ((rc = %s->put(%s, &key, &data, 0))!=0) {", dataoff,
          target_handle, target_handle);
      strcat(buf, fbuf);

      if (isESL()) {
        sprintf(
            fbuf,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->put()\");"
              "\nreturn s_failure;"
              "\n}", getUserName(), getQueryName());
      } else if (isESLAggr()) {
        sprintf(
            fbuf,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->put()\");"
              "\nreturn;"
              "\n}", getUserName(), getAggrName());
      } else {
        sprintf(fbuf, "\nadlabort(rc, \"IM_REL->put()\");"
          "\n}");
      }
      strcat(buf, fbuf);
    } else { // B_TREE
      sprintf(fbuf, "\ndata.size = %d;"
        "\nkey.size = %d;"
        "\nif ((rc = %s->put(%s, NULL, &key, &data, %s))!=0) {", dataoff,
          keyoff, target_handle, target_handle, flag, target_handle,
          target_handle);
      strcat(buf, fbuf);

      if (isESL()) {
        sprintf(
            fbuf,
            "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->put()\");"
              "\nreturn s_failure;"
              "\n}"
              "\n%s->sync(%s, 0);", getUserName(), getQueryName(),
            target_handle, target_handle);
      } else if (isESLAggr()) {
        sprintf(
            fbuf,
            "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->put()\");"
              "\nreturn;"
              "\n}"
              "\n%s->sync(%s, 0);", getUserName(), getAggrName(),
            target_handle, target_handle);
      } else {
        sprintf(fbuf, "\nadlabort(rc, \"IM_REL->put()\");"
          "\n}"
          "\n%s->sync(%s, 0);", target_handle, target_handle);
      }
      strcat(buf, fbuf);
    }
    break;
  }
  exit: DBUG_RETURN(rc);
  return rc;
} // end transInsert2C
err_t declareQun2C(char *name, Ty_ty ty, char *buf, bool dcl_expire) {
  SMLog::SMLOG(10, "Entering declareQun2C name: %s", name);
  err_t rc = ERR_NONE;
  char tmp[MAX_STR_LEN];
  char expire[MAX_STR_LEN];
  char expire_block[MAX_STR_LEN];
  int i;

  expire_block[0] = 0;
  if (ty->kind != Ty_record) {
    rc = ERR_NTSQL_INTERNAL;
    EM_error(0, rc, __LINE__, __FILE__, "declareQun2C");
    goto exit;
  }

  sprintf(buf, "\nstruct {");
  for (i = 0; i < ty->u.record->length; i++) {
    Ty_field f = (Ty_field) getNthElementList(ty->u.record, i);
    switch (f->ty->kind) {
    case Ty_nil:
    case Ty_ref:
    case Ty_record:
      /* The values of ref types are memory pointers.
       Internally, we treat them as integers. */
    case Ty_int:
      sprintf(tmp, "\nint %s;", S_name(f->name));
      sprintf(expire, "\nint %s_expire;", S_name(f->name));
      break;
    case Ty_string:
      sprintf(tmp, "\nchar %s[%d];", S_name(f->name), f->size + 1);//string length
      sprintf(expire, "\nchar %s_expire[%d];", S_name(f->name), f->size + 1);//string length
      break;
    case Ty_real:
      sprintf(tmp, "\ndouble %s;", S_name(f->name));
      sprintf(expire, "\ndouble %s_expire;", S_name(f->name));
      break;
    case Ty_timestamp:
      sprintf(tmp, "\nstruct timeval %s;", S_name(f->name));
      sprintf(expire, "\nstruct timeval %s_expire;", S_name(f->name));
      break;
    case Ty_iext:
      sprintf(tmp, "\nstruct iExt_ %s;", S_name(f->name));
      sprintf(expire, "\nstruct iExt_ %s_expire;", S_name(f->name));
      break;
    case Ty_rext:
      sprintf(tmp, "\nstruct rExt_ %s;", S_name(f->name));
      sprintf(expire, "\nstruct rExt_ %s_expire;", S_name(f->name));
      break;
    case Ty_cext:
      sprintf(tmp, "\nstruct cExt_ %s;", S_name(f->name));
      sprintf(expire, "\nstruct cExt_ %s_expire;", S_name(f->name));
      break;
    case Ty_text:
      sprintf(tmp, "\nstruct tExt_ %s;", S_name(f->name));
      sprintf(expire, "\nstruct tExt_ %s_expire;", S_name(f->name));
      break;
    default:
      rc = ERR_NTSQL_INTERNAL;
      EM_error(0, rc, __LINE__, __FILE__, "field type");
      goto exit;
    }
    strcat(buf, tmp);
    strcat(expire_block, expire);
  }
  strcat(buf, expire_block);
  sprintf(tmp, "\nstruct timeval atime;"
    "\n} %s;", name);
  strcat(buf, tmp);

  exit: return rc;
}

/* Translate a BTREE comparison function into C code
 */
err_t transCompFun(char* handlename, // DB handle name
    char* buf) // output buffer
{

  SMLog::SMLOG(10, "Entering transCompFun handlename: %s", handlename);
  //sprintf(buf,
  //  "\nif ((rc=%s->set_bt_compare(%s, _%s_cmp)) != 0){"
  //  "\nadlabort(rc, \"BerkeleyDB->set_bt_compare()\");"
  //  "};",
  //  handlename, handlename, handlename);
  if (isESL()) {
    sprintf(
        buf,
        "\nif ((rc=%s->set_bt_compare(%s, _%s_cmp)) != 0) {"
          "\nadlabortESL(bm->lookup(\"%s_errors\"), rc, \"\\nError in query %s: IM_REL->put()\");"
          "\nreturn s_failure;"
          "\n}", handlename, handlename, handlename, getUserName(),
        getQueryName());
  } else if (isESLAggr()) {
    sprintf(
        buf,
        "\nif ((rc=%s->set_bt_compare(%s, _%s_cmp)) != 0) {"
          "\nadlabortESLAggr(bm->lookup(\"%s_errors\"), rc, \"\\nError in Aggregate %s: IM_REL->put()\");"
          "\nreturn;"
          "\n}", handlename, handlename, handlename, getUserName(),
        getAggrName());
  } else {
    sprintf(buf, "\nif ((rc=%s->set_bt_compare(%s, _%s_cmp)) != 0){"
      "\nadlabort(rc, \"IM_REL->put()\");"
      "\n}", handlename, handlename, handlename);
  }
  return ERR_NONE;
}

