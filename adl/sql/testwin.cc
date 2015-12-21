#include <math.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <db.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <rtree.h>
extern "C"{
   #include <im_db.h>
   #include "swimlib.h"
   #include "fptree.h"
   #include "memmgr.h"
   #include "timeval.h"
   #include "histmgr.h"
   #include <mcheck.h>
}
#include "adllib.h"
#include <ext/hash_map>
#include <winbuf.h>
#include <windowBuf.h>
using namespace ESL;

using namespace __gnu_cxx;

static hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables=new hash_map<const char*, void*, hash<const char*>, eqstrTab>;
extern int _ADL_NULL;
extern char* _ADL_NULL_STR;
#define MAX_STR_LEN 40960


int adldebug = 0;
int _adl_sqlcode, _adl_cursqlcode;
/**** Global Declarations ****/
struct mymin_status {
   DB *mSoFar;
   int _mSoFar_cmp(DB* dbp, const DBT *a, const DBT *b){
      	int ai, bi, ri, rs;
      	double ad, bd, rd;
      	struct timeval *at, *bt;return 0;
   };
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void mymin_init(struct mymin_status *status, 
	int i, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void mymin_iterate(struct mymin_status *status, 
	int i, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void mymin_terminate(struct mymin_status *status, 
	int i, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL);
extern "C" void mymin_init(struct mymin_status *status, int i,
	int _rec_id, int __is_init, bufferMngr* bm, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
	vector<A_timeexp>* plist, int endSlide, char* _modelId)
{
   int rc;
   int _adl_sqlcode, _adl_cursqlcode;
   DBT key, data, windata;
   Rect r_key;
   char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN], _databuf[MAX_STR_LEN];
   char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
   char _adl_dbname[80];
   IM_REL *window;
   int slide_out = 1;
   winbuf* rwinbuf = NULL;
   int rlast_out = 0;
   if (__is_init) {
      sprintf(_adl_dbname, "._%d_mSoFar", status);
      (void)unlink(_adl_dbname);
      if ((rc = db_create(&status->mSoFar, NULL, 0)) != 0) {
         adlabort(rc, "db_create()");
      }
      if ((rc = status->mSoFar->set_pagesize(status->mSoFar, 2048)) != 0) {
         adlabort(rc, "set_pagesize()");
      }
      if ((rc = status->mSoFar->set_flags(status->mSoFar, DB_DUP)) != 0) {
         adlabort(rc, "set_flags()");
      }
      if ((rc = status->mSoFar->open(status->mSoFar, _adl_dbname, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
         adlabort(rc, "open()");
      }
      sprintf(_adl_dbname, "._%d_ret", status);
      (void)unlink(_adl_dbname);
      if ((rc = im_rel_create(&status->ret, NULL, IM_LINKEDLIST, 0)) != 0) {
         adlabort(rc, "im_rel_create()");
      }
      if ((rc = status->ret->open(status->ret, _adl_dbname, 0)) != 0) {
         adlabort(rc, "open()");
      }
      if ((rc = status->ret->cursor(status->ret, &status->retc, 0)) != 0) {
         adlabort(rc, "IM_REL->cursor()");
      }
   } /* end of __is_init */
   struct {
      int i;
      int i_expire;
      struct timeval atime;
   } insert_0;
   int first_entry_1 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_1:
      rc = (first_entry_1)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_1=1;
      else {
         first_entry_1=0;
         insert_0.i = i;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_0.i), sizeof(int));
         data.size = 4;
         key.size = 0;
         if ((rc = status->mSoFar->put(status->mSoFar, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         status->mSoFar->sync(status->mSoFar, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void mymin_iterate(struct mymin_status *status, 
	int i, int _rec_id, bufferMngr* bm, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
	vector<A_timeexp>* plist, int endSlide, char* _modelId)
{
   int rc;
   int _adl_sqlcode, _adl_cursqlcode;
   DBT key, data, windata;
   Rect r_key;
   char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];
   char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
   char _adl_dbname[80];
   IM_REL *window;
   int slide_out = 1;
   winbuf* rwinbuf = NULL;
   int rlast_out = 0;
   DBC *mSoFar_2;
   int first_entry_3 = 1;
   int first_entry_2 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->mSoFar->cursor(status->mSoFar, NULL, &mSoFar_2, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int m;
         int m_expire;
         struct timeval atime;
      } mSoFar_2_3;
      next_2:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = mSoFar_2->c_get(mSoFar_2, &key, &data, (first_entry_3)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_3 = 0;
         memcpy(&(mSoFar_2_3.m), (char*)data.data+0, sizeof(int));
         //printf("Retrieved mSoFar_2_3.m = %d\n", mSoFar_2_3.m);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_3 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         tmpStore(&key, &data);
         rc = 0;          /* subquery could've overwritten rc */
         if (!((i < mSoFar_2_3.m))) {
            goto next_2;
         }
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         tmpRecover(&key, &data);
         *(int*)((char*)data.data+0) = i;
         if ((rc = mSoFar_2->c_put(mSoFar_2, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (mSoFar_2 && (rc = mSoFar_2->c_close(mSoFar_2)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void mymin_terminate(struct mymin_status *status, 
	int i, int _rec_id, int not_delete, bufferMngr* bm, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
	vector<A_timeexp>* plist, int endSlide, char* _modelId)
{
   int rc;
   int _adl_sqlcode, _adl_cursqlcode;
   DBT key, data, windata;
   Rect r_key;
   char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];
   char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
   char _adl_dbname[80];
   int slide_out = 1;
   winbuf* rwinbuf = NULL;
   int rlast_out = 0;
   struct {
      int m;
      int m_expire;
      struct timeval atime;
   } insert_4;
   DBC *mSoFar_6;
   int first_entry_5 = 1;
   int first_entry_6 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->mSoFar->cursor(status->mSoFar, NULL, &mSoFar_6, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int m;
         int m_expire;
         struct timeval atime;
      } mSoFar_6_5;
      next_6:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = mSoFar_6->c_get(mSoFar_6, &key, &data, (first_entry_5)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_5 = 0;
         memcpy(&(mSoFar_6_5.m), (char*)data.data+0, sizeof(int));
         //printf("Retrieved mSoFar_6_5.m = %d\n", mSoFar_6_5.m);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_5 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_4.m = mSoFar_6_5.m;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_4.m), sizeof(int));
         memcpy((char*)data.data+0, &(insert_4.m), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (mSoFar_6 && (rc = mSoFar_6->c_close(mSoFar_6)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   sprintf(_adl_dbname, "._%d_mSoFar", status);
   if(!not_delete) {
      if (status->mSoFar && ((rc = status->mSoFar->close(status->mSoFar, 0)) != 0)) {
         adlabort(rc, "DB->close()");
      }
      status->mSoFar = NULL;
      (void)unlink(_adl_dbname);
   }
   if(!not_delete) status->retc_first_entry=1;
}
struct inWinType_mymin_window {
   int i;
};
struct mymin_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_mymin_window* getLastTuple_mymin_window(IM_REL* inwindow, inWinType_mymin_window* tuple, bufferMngr* bm) {
   int rc;
   DBT key, data;
   char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN], _databuf[MAX_STR_LEN];
   IM_RELC* temp;
   
   memset(&key, 0, sizeof(key));
   memset(&data, 0, sizeof(data));
   data.data = datadata;
   key.data = keydata;
   if ((rc = inwindow->cursor(inwindow, &temp, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   rc = temp->c_get(temp, &key, &data, DB_FIRST);
   if (rc == DB_NOTFOUND) {
      adlabort(rc, "IMREL->c_get() in oldest()");
   }
   memcpy(&((*tuple).i), (char*)data.data+0, sizeof(int));
   //printf("Retrieved (*tuple).i = %d\n", (*tuple).i);
   //fflush(stdout);
   return tuple;
}
extern "C" void mymin_window_init(struct mymin_window_status *status, 
	int i, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void mymin_window_expire(struct mymin_window_status *status, 
	int i, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void mymin_window_init(struct mymin_window_status *status, int i, 
	int _rec_id, int __is_init, bufferMngr* bm, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
	vector<A_timeexp>* plist, int endSlide, char* _modelId)
{
   int rc;
   int _adl_sqlcode, _adl_cursqlcode;
   DBT key, data, windata;
   Rect r_key;
   char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN], _databuf[MAX_STR_LEN];
   char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
   char _adl_dbname[80];
   IM_REL *window;
   int slide_out = 1;
   winbuf* rwinbuf = NULL;
   int rlast_out = 0;
   if(status && status->win)
   window = status->win->get_im_rel();
   struct inWinType_mymin_window tuple;
   if (__is_init) {
      sprintf(_adl_dbname, "._%d_ret", status);
      (void)unlink(_adl_dbname);
      if ((rc = im_rel_create(&status->ret, NULL, IM_LINKEDLIST, 0)) != 0) {
         adlabort(rc, "im_rel_create()");
      }
      if ((rc = status->ret->open(status->ret, _adl_dbname, 0)) != 0) {
         adlabort(rc, "open()");
      }
      if ((rc = status->ret->cursor(status->ret, &status->retc, 0)) != 0) {
         adlabort(rc, "IM_REL->cursor()");
      }
   } /* end of __is_init */
   IM_RELC *inwindow_7;
   int first_entry_8 = 1;
   int first_entry_7 = 1;
   struct {
      int field_0;
      int field_0_expire;
      struct timeval atime;
   } insert_9;
   int first_entry_10 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &inwindow_7, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int i;
         int i_expire;
         struct timeval atime;
      } inwindow_7_8;
      next_7:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = inwindow_7->c_get(inwindow_7, &key, &data, (first_entry_8)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_8 = 0;
         memcpy(&(inwindow_7_8.i), (char*)data.data+0, sizeof(int));
         //printf("Retrieved inwindow_7_8.i = %d\n", inwindow_7_8.i);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_8 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         rc = 0;          /* subquery could've overwritten rc */
         if (!((inwindow_7_8.i > i))) {
            goto next_7;
         }
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = inwindow_7->c_get(inwindow_7, &key, &data, DB_CURRENT);
         if(rc == DB_NOTFOUND) adlabort(rc, "DBC->c_get()");
         struct iExt_ tmpIext_8;
         struct rExt_ tmpRext_8;
         struct cExt_ tmpCext_8;
         struct tExt_ tmpText_8;
         if(plist != NULL) {
            int tmpid = 0;
            memcpy(&tmpid, ((char*)data.data)+status->win->plidbegin, sizeof(int));
            (*plist)[tmpid]->deleted = true;
         }
         /*DELETE STARTS*/
         if ((rc = inwindow_7->c_del(inwindow_7, 0)) != 0) {
            adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
         }
         /*DELETE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (inwindow_7 && (rc = inwindow_7->c_close(inwindow_7)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_10:
      rc = (first_entry_10)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_10=1;
      else {
         first_entry_10=0;
         insert_9.field_0 = (getLastTuple_mymin_window(window, &tuple, bm)->i);
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_9.field_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_9.field_0), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void mymin_window_expire(struct mymin_window_status *status, 
	int i, int _rec_id, bufferMngr* bm, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
	vector<A_timeexp>* plist, int endSlide, char* _modelId)
{
   int rc;
   int _adl_sqlcode, _adl_cursqlcode;
   DBT key, data, windata;
   Rect r_key;
   char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];
   char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
   char _adl_dbname[80];
   IM_REL *window;
   int slide_out = 1;
   winbuf* rwinbuf = NULL;
   int rlast_out = 0;
   if(status && status->win)
   window = status->win->get_im_rel();
   struct inWinType_mymin_window tuple;
   status->retc_first_entry=1;
}
IM_REL *t;
/**** Query Declarations ****/
int _adl_statement_53()
{
   int rc = 0;
   DBT key, data, windata;
   struct timeval atime;
   Rect r_key;
   char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];
   char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
   int _rec_id=0; /* recursive id */
   char _adl_dbname[MAX_STR_LEN];
   int slide_out = 1;
   winbuf* rwinbuf = NULL;
   int rlast_out = 0;
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   struct {
      int field_0;
      int field_1;
      int field_0_expire;
      int field_1_expire;
      struct timeval atime;
   } insert_11;
   int index_12 = 0;
   int first_entry_14 = 1;
   int first_entry_16 = 1;
   int first_entry_18 = 1;
   int first_entry_20 = 1;
   int first_entry_22 = 1;
   int first_entry_24 = 1;
   int first_entry_26 = 1;
   int first_entry_28 = 1;
   int first_entry_30 = 1;
   int first_entry_32 = 1;
   int first_entry_34 = 1;
   int first_entry_36 = 1;
   int first_entry_38 = 1;
   int first_entry_40 = 1;
   int first_entry_42 = 1;
   int first_entry_44 = 1;
   int first_entry_46 = 1;
   int first_entry_48 = 1;
   int first_entry_50 = 1;
   int first_entry_52 = 1;
   int first_entry_12 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      if (first_entry_12==1) index_12 = 0;
      do {
         switch (index_12) {
            case 0:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_13;
               next_14:
               rc = (first_entry_14)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_14=1;
               else {
                  first_entry_14=0;
                  unionqun_13.field_0 = 0;
                  unionqun_13.field_1 = 1;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_13.field_0;
                  insert_11.field_1 = unionqun_13.field_1;
               }
            }
            break;
            case 1:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_15;
               next_16:
               rc = (first_entry_16)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_16=1;
               else {
                  first_entry_16=0;
                  unionqun_15.field_0 = 0;
                  unionqun_15.field_1 = 2;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_15.field_0;
                  insert_11.field_1 = unionqun_15.field_1;
               }
            }
            break;
            case 2:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_17;
               next_18:
               rc = (first_entry_18)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_18=1;
               else {
                  first_entry_18=0;
                  unionqun_17.field_0 = 0;
                  unionqun_17.field_1 = 3;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_17.field_0;
                  insert_11.field_1 = unionqun_17.field_1;
               }
            }
            break;
            case 3:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_19;
               next_20:
               rc = (first_entry_20)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_20=1;
               else {
                  first_entry_20=0;
                  unionqun_19.field_0 = 1;
                  unionqun_19.field_1 = 7;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_19.field_0;
                  insert_11.field_1 = unionqun_19.field_1;
               }
            }
            break;
            case 4:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_21;
               next_22:
               rc = (first_entry_22)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_22=1;
               else {
                  first_entry_22=0;
                  unionqun_21.field_0 = 1;
                  unionqun_21.field_1 = 6;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_21.field_0;
                  insert_11.field_1 = unionqun_21.field_1;
               }
            }
            break;
            case 5:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_23;
               next_24:
               rc = (first_entry_24)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_24=1;
               else {
                  first_entry_24=0;
                  unionqun_23.field_0 = 1;
                  unionqun_23.field_1 = 8;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_23.field_0;
                  insert_11.field_1 = unionqun_23.field_1;
               }
            }
            break;
            case 6:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_25;
               next_26:
               rc = (first_entry_26)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_26=1;
               else {
                  first_entry_26=0;
                  unionqun_25.field_0 = 1;
                  unionqun_25.field_1 = 9;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_25.field_0;
                  insert_11.field_1 = unionqun_25.field_1;
               }
            }
            break;
            case 7:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_27;
               next_28:
               rc = (first_entry_28)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_28=1;
               else {
                  first_entry_28=0;
                  unionqun_27.field_0 = 0;
                  unionqun_27.field_1 = 7;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_27.field_0;
                  insert_11.field_1 = unionqun_27.field_1;
               }
            }
            break;
            case 8:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_29;
               next_30:
               rc = (first_entry_30)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_30=1;
               else {
                  first_entry_30=0;
                  unionqun_29.field_0 = 0;
                  unionqun_29.field_1 = 8;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_29.field_0;
                  insert_11.field_1 = unionqun_29.field_1;
               }
            }
            break;
            case 9:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_31;
               next_32:
               rc = (first_entry_32)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_32=1;
               else {
                  first_entry_32=0;
                  unionqun_31.field_0 = 0;
                  unionqun_31.field_1 = 2;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_31.field_0;
                  insert_11.field_1 = unionqun_31.field_1;
               }
            }
            break;
            case 10:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_33;
               next_34:
               rc = (first_entry_34)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_34=1;
               else {
                  first_entry_34=0;
                  unionqun_33.field_0 = 0;
                  unionqun_33.field_1 = 4;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_33.field_0;
                  insert_11.field_1 = unionqun_33.field_1;
               }
            }
            break;
            case 11:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_35;
               next_36:
               rc = (first_entry_36)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_36=1;
               else {
                  first_entry_36=0;
                  unionqun_35.field_0 = 0;
                  unionqun_35.field_1 = 6;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_35.field_0;
                  insert_11.field_1 = unionqun_35.field_1;
               }
            }
            break;
            case 12:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_37;
               next_38:
               rc = (first_entry_38)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_38=1;
               else {
                  first_entry_38=0;
                  unionqun_37.field_0 = 0;
                  unionqun_37.field_1 = 7;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_37.field_0;
                  insert_11.field_1 = unionqun_37.field_1;
               }
            }
            break;
            case 13:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_39;
               next_40:
               rc = (first_entry_40)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_40=1;
               else {
                  first_entry_40=0;
                  unionqun_39.field_0 = 0;
                  unionqun_39.field_1 = 3;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_39.field_0;
                  insert_11.field_1 = unionqun_39.field_1;
               }
            }
            break;
            case 14:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_41;
               next_42:
               rc = (first_entry_42)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_42=1;
               else {
                  first_entry_42=0;
                  unionqun_41.field_0 = 0;
                  unionqun_41.field_1 = 8;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_41.field_0;
                  insert_11.field_1 = unionqun_41.field_1;
               }
            }
            break;
            case 15:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_43;
               next_44:
               rc = (first_entry_44)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_44=1;
               else {
                  first_entry_44=0;
                  unionqun_43.field_0 = 0;
                  unionqun_43.field_1 = 7;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_43.field_0;
                  insert_11.field_1 = unionqun_43.field_1;
               }
            }
            break;
            case 16:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_45;
               next_46:
               rc = (first_entry_46)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_46=1;
               else {
                  first_entry_46=0;
                  unionqun_45.field_0 = 0;
                  unionqun_45.field_1 = 4;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_45.field_0;
                  insert_11.field_1 = unionqun_45.field_1;
               }
            }
            break;
            case 17:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_47;
               next_48:
               rc = (first_entry_48)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_48=1;
               else {
                  first_entry_48=0;
                  unionqun_47.field_0 = 0;
                  unionqun_47.field_1 = 5;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_47.field_0;
                  insert_11.field_1 = unionqun_47.field_1;
               }
            }
            break;
            case 18:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_49;
               next_50:
               rc = (first_entry_50)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_50=1;
               else {
                  first_entry_50=0;
                  unionqun_49.field_0 = 0;
                  unionqun_49.field_1 = 6;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_49.field_0;
                  insert_11.field_1 = unionqun_49.field_1;
               }
            }
            break;
            case 19:
            {
               struct {
                  int field_0;
                  int field_1;
                  int field_0_expire;
                  int field_1_expire;
                  struct timeval atime;
               } unionqun_51;
               next_52:
               rc = (first_entry_52)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_52=1;
               else {
                  first_entry_52=0;
                  unionqun_51.field_0 = 0;
                  unionqun_51.field_1 = 8;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_11.field_0 = unionqun_51.field_0;
                  insert_11.field_1 = unionqun_51.field_1;
               }
            }
            break;
         }/* end of switch */
         if (rc == DB_NOTFOUND) index_12++;
      } while (rc == DB_NOTFOUND && index_12 < 20);
      next_12:
      if (rc == DB_NOTFOUND) {
         first_entry_12 = 1;
      }
      else {
         first_entry_12 = 0;
      }
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_11.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_11.field_1), sizeof(int));
         data.size = 8;
         key.size = 0;
         if ((rc = t->put(t, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_61()
{
   int rc = 0;
   DBT key, data, windata;
   struct timeval atime;
   Rect r_key;
   char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN],_databuf[MAX_STR_LEN];
   char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
   int _rec_id=0; /* recursive id */
   char _adl_dbname[MAX_STR_LEN];
   int slide_out = 1;
   winbuf* rwinbuf = NULL;
   int rlast_out = 0;
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   struct {
      int b;
      int b_expire;
      struct timeval atime;
   } insert_54;
   IM_RELC *t_60;
   int first_entry_59 = 1;
   int first_entry_60 = 1;
   int index_58 = 0;
   int terminating_58=0;
   struct gb_status_58 {
      struct mymin_window_status *mymin_window_0;
   };
   struct gb_status_58 *gbstatus_58 = (struct gb_status_58 *)0;
   
   int first_entry_58 = 1;
   int first_entry_56 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = t->cursor(t, &t_60, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0001_56_55;
      next_56:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0000_58_57;
      next_58:
      while (index_58>=0 && index_58 < 2) {
         switch(index_58) {
            case 0:
            {
               if (terminating_58 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int a;
                     int b;
                     int OID;
                     int a_expire;
                     int b_expire;
                     int OID_expire;
                     struct timeval atime;
                  } t_60_59;
                  next_60:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = t_60->c_get(t_60, &key, &data, (first_entry_59)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_59 = 0;
                     memcpy(&(t_60_59.a), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved t_60_59.a = %d\n", t_60_59.a);
                     //fflush(stdout);
                     memcpy(&(t_60_59.b), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved t_60_59.b = %d\n", t_60_59.b);
                     //fflush(stdout);
                     memcpy(&(t_60_59.OID), (char*)data.data+8, sizeof(int));
                     //printf("Retrieved t_60_59.OID = %d\n", t_60_59.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_59 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0000_58_57.a_0 = t_60_59.b;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_58 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_58 = (struct gb_status_58 *)0;
                     rc = hash_get(58, _rec_id, gbkey, 4, (char**)&gbstatus_58);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_58 = (struct gb_status_58*)malloc(sizeof(*gbstatus_58));
                        gbstatus_58->mymin_window_0 = (struct mymin_window_status*)malloc(sizeof(struct mymin_window_status));
                        gbstatus_58->mymin_window_0->win = 0;
                        gbstatus_58->mymin_window_0->win = new winbuf(4, 4, 4, _ADL_WIN_ROW);
                        gbstatus_58->mymin_window_0->last_out = 0;
                        gbstatus_58->mymin_window_0->iterate = false;
                        gbstatus_58->mymin_window_0->init = true;
                        setModelId("");
                        memcpy(_databuf+0, &(Q_0000_58_57.a_0), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 4;
                        gbstatus_58->mymin_window_0->win->updateTupleID();
                        gbstatus_58->mymin_window_0->win->put(&windata);
                        if((!(1 <= 1 || ((gbstatus_58->mymin_window_0->win->getTupleID() == 0 && gbstatus_58->mymin_window_0->last_out != 0) 
                        ||((((int)gbstatus_58->mymin_window_0->win->getTupleID()) >= gbstatus_58->mymin_window_0->last_out + 1)))))) {
                           slide_out = 0;
                           //printf("Here no output %d 1 %d\n", gbstatus_58->mymin_window_0->last_out, gbstatus_58->mymin_window_0->win->getTupleID());fflush(stdout);
                        } else {
                           slide_out = 1;
                           //printf("Here YES output %d 1 %d\n", gbstatus_58->mymin_window_0->last_out, gbstatus_58->mymin_window_0->win->getTupleID());fflush(stdout);
                           gbstatus_58->mymin_window_0->last_out = gbstatus_58->mymin_window_0->last_out + 1;
                           while(gbstatus_58->mymin_window_0->last_out < (gbstatus_58->mymin_window_0->win->getTupleID() - 1) && gbstatus_58->mymin_window_0->win->getTupleID() > 0) {
                              if(1 == 1) {
                                 gbstatus_58->mymin_window_0->last_out = gbstatus_58->mymin_window_0->win->getTupleID();
                              }
                              else {
                                 gbstatus_58->mymin_window_0->last_out = gbstatus_58->mymin_window_0->last_out + 1;
                              }
                           }
                        }
                        mymin_window_init(gbstatus_58->mymin_window_0, Q_0000_58_57.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(58, _rec_id, gbkey, 4, &gbstatus_58);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        memcpy(_databuf+0, &(Q_0000_58_57.a_0), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 4;
                        gbstatus_58->mymin_window_0->win->updateTupleID();
                        gbstatus_58->mymin_window_0->win->put(&windata);
                        if((!(1 <= 1 || ((gbstatus_58->mymin_window_0->win->getTupleID() == 0 && gbstatus_58->mymin_window_0->last_out != 0) 
                        ||((((int)gbstatus_58->mymin_window_0->win->getTupleID()) >= gbstatus_58->mymin_window_0->last_out + 1)))))) {
                           slide_out = 0;
                           //printf("Here no output %d 1 %d\n", gbstatus_58->mymin_window_0->last_out, gbstatus_58->mymin_window_0->win->getTupleID());fflush(stdout);
                        } else {
                           slide_out = 1;
                           //printf("Here YES output %d 1 %d\n", gbstatus_58->mymin_window_0->last_out, gbstatus_58->mymin_window_0->win->getTupleID());fflush(stdout);
                           gbstatus_58->mymin_window_0->last_out = gbstatus_58->mymin_window_0->last_out + 1;
                           while(gbstatus_58->mymin_window_0->last_out < (gbstatus_58->mymin_window_0->win->getTupleID() - 1) && gbstatus_58->mymin_window_0->win->getTupleID() > 0) {
                              if(1 == 1) {
                                 gbstatus_58->mymin_window_0->last_out = gbstatus_58->mymin_window_0->win->getTupleID();
                              }
                              else {
                                 gbstatus_58->mymin_window_0->last_out = gbstatus_58->mymin_window_0->last_out + 1;
                              }
                           }
                        }
                        while (gbstatus_58->mymin_window_0->win->hasExpired()){
                           gbstatus_58->mymin_window_0->win->getExpired(&windata);
                           memcpy(&(Q_0000_58_57.a_0_expire), (char*)windata.data+0, sizeof(int));
                           mymin_window_expire(gbstatus_58->mymin_window_0, Q_0000_58_57.a_0_expire, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                           gbstatus_58->mymin_window_0->win->pop();
                        }
                        mymin_window_init(gbstatus_58->mymin_window_0, Q_0000_58_57.a_0, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_58 = 1;
                  }
               }
               if (terminating_58 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(58, _rec_id, allkey, 4, (char**)&gbstatus_58);
                  if (rc==0) {
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_58->mymin_window_0->retc->c_get(gbstatus_58->mymin_window_0->retc, &key, &data, (gbstatus_58->mymin_window_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_58->mymin_window_0->retc_first_entry = 0;
                  memcpy(&(Q_0001_56_55.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0001_56_55.a_0 = %d\n", Q_0001_56_55.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_58->mymin_window_0->retc->c_del(gbstatus_58->mymin_window_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_58->mymin_window_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_58 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_58++;
         }
         if (rc == DB_NOTFOUND) {
            index_58--;
            if (terminating_58 == 1 && index_58 == 0) {
               if (gbstatus_58->mymin_window_0->retc && (rc = gbstatus_58->mymin_window_0->retc->c_close(gbstatus_58->mymin_window_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_58->mymin_window_0);
               
               if (gbstatus_58->mymin_window_0->ret && ((rc = gbstatus_58->mymin_window_0->ret->close(gbstatus_58->mymin_window_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_58->mymin_window_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_58--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_58 = 0;
         first_entry_58 = 1;
         index_58 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(58, _rec_id, allkey, 4, (char**)&gbstatus_58);
            if (rc==0) {
               if(gbstatus_58->mymin_window_0) {
                  if(gbstatus_58->mymin_window_0->win) {
                     delete(gbstatus_58->mymin_window_0->win);
                     gbstatus_58->mymin_window_0->win = 0;
                  }
                  free(gbstatus_58->mymin_window_0);
               }
               //printf("freeing 58\n");
               free(gbstatus_58);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(58, _rec_id);
      }
      if (rc==0) {
         insert_54.b = Q_0001_56_55.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_54.b);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (t_60 && (rc = t_60->c_close(t_60)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int main()
{
   int rc;
   tempdb_init();
   hashgb_init();
   _adl_dlm_init();
   // Initialization of Declarations
   if ((rc = im_rel_create(&t, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = t->open(t, "_adl_db_t", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("t") == 0) {
      inMemTables->operator[](strdup("t")) = t;
   }
   _adl_statement_53();
   _adl_statement_61();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = t->close(t, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
