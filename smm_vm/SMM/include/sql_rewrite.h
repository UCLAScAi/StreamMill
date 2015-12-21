#ifndef _SQL_REWRITE_H
#define _SQL_REWRITE_H

#include <absyn.h>

err_t staticRewrite(S_table venv, S_table tenv, A_exp a, A_exp &result, vector<void*> aggregates);

err_t rewriteUpdate(S_table venv, S_table tenv, A_exp exp, vector<void*> aggregates);

err_t rewriteQuery(S_table venv, S_table tenv, A_exp a, A_exp &result, vector<void*> aggregates);

err_t rewriteExp(S_table venv, S_table tenv, A_sqlopr opr, A_exp exp, A_exp &nexp);
err_t constructSelOp(S_table venv, A_exp sel, A_exp &result);

#endif
