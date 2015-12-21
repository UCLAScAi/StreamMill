#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <absyn.h>
#include <symbol.h>
#include <adl_obj.h>
#include <err.h>
#include <types.h>
#include <list.h>
#include <env.h>
#include <time.h>
typedef char* Tr_exp;
/*  typedef struct Tr_exp_ *Tr_exp; */


void Tr_Delete(Tr_exp ele);
Tr_exp Tr_String(char *s);
Tr_exp Tr_QuoteString(char *s);
Tr_exp Tr_Int(int n);
Tr_exp Tr_Timestamp(struct timeval* tv);
Tr_exp Tr_Real(double r);
Tr_exp Tr_Seq(Tr_exp list, Tr_exp ele);
err_t trans2C(S_table venv, S_table tenv, A_exp e, char *filename);

err_t transTabRemove2C(char *table, char *dbname, char *buf, 
		       tabscope_t scope, tabindex_t idx = INDEX_BTREE);
err_t transCursorDec2C(char *name, char *buf, tabscope_t kind, tabindex_t idx=INDEX_BTREE);
err_t transCursorInit2C(char *dbname, 
			char *cursorname, 
			char *buf,
			tabscope_t kind,
			tabindex_t idx=INDEX_BTREE,
			int isView=0
			);
err_t transCursorGet2C(S_table venv,
		       char *first_entry_name, 
		       char *cursorname, 
		       E_enventry x,
		       //Ty_ty ty,		/* tuple type*/
		       Tr_exp *bndp,		/* bindings equlity of the keys*/
		       Tr_exp *bndpUpper,	/* bindings upper bound of the keys*/
		       Tr_exp *bndpLower,	/* bindings lower bound of the keys*/
		       int flag,               /* binding flag */
		       char *buf);
err_t transStreamGet2C(E_enventry x,
		       char *buf);
err_t transCursorClose2C(char *cursorname, char *buf);
err_t transCursorDelete2C(char *cursorname, char *buf);
err_t transCursorPut2C(char *cursorname, char *buf);
err_t transCursorUpdate2C(char *name, char *buf);

err_t transTabDec2C(char *name, char *buf, E_enventry var);
err_t transStreamDec2C(S_table venv, char *name, char *buf, E_enventry var);

err_t transTabInit2C(char *handlename, 
		     char *dbname, 
		     int haskey,
		     char *buf,
		     tabscope_t scope, 
		     tabindex_t idx=INDEX_BTREE,
		     ty_t keyType=Ty_int,
                     int in_aggr = 0,
                     int isView = 0,
                     S_symbol tname = NULL
		     );
err_t transTabInit2C_imdb(char *handlename, 
		     	  char *dbname, 
			  tabscope_t scope, 
		     	  int haskey,
		     	  char *buf
			  );
err_t transTabClose2C(char *handlename, 
		      char *dbname, 
		      char *buf,
		      tabscope_t scope,
		      tabindex_t idx=INDEX_BTREE,
		      int term_cond = 0
		      );
err_t transStreamClose2C(char *dbname, 
			 char *buf
			 );

err_t assignKeyData2C(Ty_ty ty, char *name, char *buf, tabindex_t idx=INDEX_BTREE);
err_t assignStreamKeyData2C(Ty_ty ty, char *name, char *buf);
err_t assignField2C(Ty_field f, 
		    char *s_name, char *f_name, 
		    int *keyoff, int *dataoff, char *fbuf);

err_t transInsert2C(S_table venv,
		    A_qun target, char *stru, char *target_handle, 
		    E_enventry x, int mode, char *buf, const char* queryName,
                    int in_aggr);
//		    Ty_ty ty, char *buf, int mode, tabscope_t kind, db_t dbtype);
err_t transLoad2C(S_table venv,
		  char * target_handle,
		  char * target_database,
		  char *filename, 
		  E_enventry x, char *buf);
//		  Ty_ty ty, char *buf, tabscope_t kind, db_t dbtype);

err_t declareQun2C(char *name, Ty_ty ty, char *buf,
		   bool dcl_expire = false);

err_t Ty2C(Ty_ty f, char *cdef);
err_t Dereference2C(Ty_ty f, char *refbuf);
err_t TyName2C(Ty_field f, char *cdef);
err_t TyList2C(A_list tylist, char* cdef);



/* Translate a BTREE comparison function into C code 
 */
err_t transCompFun(char* handlename,   // DB handle name
		   char* buf);        // output buffer


#endif
