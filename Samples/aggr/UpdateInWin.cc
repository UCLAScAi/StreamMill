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
IM_REL *traffic;
struct inWinType_UpdateInWin_window {
   int ID;
   int Next;
};
struct UpdateInWin_window_status {
   DB *inwindow;
   int _inwindow_cmp(DB* dbp, const DBT *a, const DBT *b){
      	int ai, bi, ri, rs;
      	double ad, bd, rd;
      	struct timeval *at, *bt;return 0;
   };
   DB *state;
   int _state_cmp(DB* dbp, const DBT *a, const DBT *b){
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
inWinType_UpdateInWin_window* getLastTuple_UpdateInWin_window(IM_REL* inwindow, inWinType_UpdateInWin_window* tuple, bufferMngr* bm) {
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
   memcpy(&((*tuple).ID), (char*)data.data+0, sizeof(int));
   //printf("Retrieved (*tuple).ID = %d\n", (*tuple).ID);
   //fflush(stdout);
   memcpy(&((*tuple).Next), (char*)data.data+4, sizeof(int));
   //printf("Retrieved (*tuple).Next = %d\n", (*tuple).Next);
   //fflush(stdout);
   return tuple;
}
extern "C" void UpdateInWin_window_init(struct UpdateInWin_window_status *status, 
	int ID, int Next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void UpdateInWin_window_iterate(struct UpdateInWin_window_status *status, 
	int ID, int Next, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void UpdateInWin_window_terminate(struct UpdateInWin_window_status *status, 
	int ID, int Next, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL);
extern "C" void UpdateInWin_window_init(struct UpdateInWin_window_status *status, int ID, int Next, 
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
   struct inWinType_UpdateInWin_window tuple;
   if (__is_init) {
      sprintf(_adl_dbname, "._%d_inwindow", status);
      (void)unlink(_adl_dbname);
      if ((rc = db_create(&status->inwindow, NULL, 0)) != 0) {
         adlabort(rc, "db_create()");
      }
      if ((rc = status->inwindow->set_pagesize(status->inwindow, 2048)) != 0) {
         adlabort(rc, "set_pagesize()");
      }
      if ((rc = status->inwindow->set_flags(status->inwindow, DB_DUP)) != 0) {
         adlabort(rc, "set_flags()");
      }
      if ((rc = status->inwindow->open(status->inwindow, _adl_dbname, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
         adlabort(rc, "open()");
      }
      sprintf(_adl_dbname, "._%d_state", status);
      (void)unlink(_adl_dbname);
      if ((rc = db_create(&status->state, NULL, 0)) != 0) {
         adlabort(rc, "db_create()");
      }
      if ((rc = status->state->set_pagesize(status->state, 2048)) != 0) {
         adlabort(rc, "set_pagesize()");
      }
      if ((rc = status->state->set_flags(status->state, DB_DUP)) != 0) {
         adlabort(rc, "set_flags()");
      }
      if ((rc = status->state->open(status->state, _adl_dbname, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
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
      int field_0;
      int field_0_expire;
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
         insert_0.field_0 = 1;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_0.field_0), sizeof(int));
         data.size = 4;
         key.size = 0;
         if ((rc = status->state->put(status->state, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         status->state->sync(status->state, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void UpdateInWin_window_iterate(struct UpdateInWin_window_status *status, 
	int ID, int Next, int _rec_id, bufferMngr* bm, 
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
   struct inWinType_UpdateInWin_window tuple;
   DBC *state_2;
   int first_entry_3 = 1;
   int first_entry_2 = 1;
   IM_RELC *inwindow_4;
   int first_entry_5 = 1;
   int first_entry_6 = 1;
   DBC *state_6;
   int first_entry_8 = 1;
   int first_entry_4 = 1;
   IM_RELC *inwindow_9;
   int first_entry_10 = 1;
   int first_entry_11 = 1;
   DBC *state_11;
   int first_entry_13 = 1;
   int first_entry_9 = 1;
   struct {
      int field_0;
      int field_0_expire;
      struct timeval atime;
   } insert_14;
   int first_entry_15 = 1;
   struct {
      int max;
      int max_expire;
      struct timeval atime;
   } insert_16;
   DBC *state_18;
   int first_entry_17 = 1;
   int first_entry_18 = 1;
   struct {
      int a_0;
      int a_0_expire;
      struct timeval atime;
   } insert_19;
   IM_RELC *inwindow_25;
   int first_entry_24 = 1;
   int first_entry_25 = 1;
   int index_23 = 0;
   int terminating_23=0;
   struct gb_status_23 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int max_0_last_out;
      bool max_0_iterate;
      bool max_0_init;
   };
   struct gb_status_23 *gbstatus_23 = (struct gb_status_23 *)0;
   
   int first_entry_23 = 1;
   int first_entry_21 = 1;
   struct {
      int a_0;
      int a_0_expire;
      struct timeval atime;
   } insert_26;
   IM_RELC *inwindow_32;
   int first_entry_31 = 1;
   int first_entry_32 = 1;
   int index_30 = 0;
   int terminating_30=0;
   struct gb_status_30 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int max_0_last_out;
      bool max_0_iterate;
      bool max_0_init;
   };
   struct gb_status_30 *gbstatus_30 = (struct gb_status_30 *)0;
   
   int first_entry_30 = 1;
   int first_entry_28 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->state->cursor(status->state, NULL, &state_2, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int max;
         int max_expire;
         struct timeval atime;
      } state_2_3;
      next_2:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = state_2->c_get(state_2, &key, &data, (first_entry_3)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_3 = 0;
         memcpy(&(state_2_3.max), (char*)data.data+0, sizeof(int));
         //printf("Retrieved state_2_3.max = %d\n", state_2_3.max);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_3 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         *(int*)((char*)data.data+0) = ((state_2_3.max) + 1);
         if ((rc = state_2->c_put(state_2, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (state_2 && (rc = state_2->c_close(state_2)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &inwindow_4, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   if ((rc = status->state->cursor(status->state, NULL, &state_6, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int ID;
         int Next;
         int ID_expire;
         int Next_expire;
         struct timeval atime;
      } inwindow_4_5;
      next_4:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = inwindow_4->c_get(inwindow_4, &key, &data, (first_entry_5)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_5 = 0;
         memcpy(&(inwindow_4_5.ID), (char*)data.data+0, sizeof(int));
         //printf("Retrieved inwindow_4_5.ID = %d\n", inwindow_4_5.ID);
         //fflush(stdout);
         memcpy(&(inwindow_4_5.Next), (char*)data.data+4, sizeof(int));
         //printf("Retrieved inwindow_4_5.Next = %d\n", inwindow_4_5.Next);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_5 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         tmpStore(&key, &data);
         rc = 0;          /* subquery could've overwritten rc */
         if (!((ID == -1))) {
            goto next_4;
         }
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         tmpRecover(&key, &data);
          /* SUBQUERY IN UPDATE STARTS */
         kdPush(&key,&data);
         struct {
            int max;
            int max_expire;
            struct timeval atime;
         } embed_7;
         int embed_7_cnt = 0;
         struct {
            int max;
            int max_expire;
            struct timeval atime;
         } state_6_8;
         next_6:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_6->c_get(state_6, &key, &data, (first_entry_8)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_8 = 0;
            memcpy(&(state_6_8.max), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_6_8.max = %d\n", state_6_8.max);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_8 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            embed_7.max = state_6_8.max;
         } /* if (rc == 0) */
         kdPop(&key, &data);
         /* SUBQUERY IN UPDATE ENDS */
         *(int*)((char*)data.data+0) = embed_7.max;
         if ((rc = inwindow_4->c_put(inwindow_4, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (inwindow_4 && (rc = inwindow_4->c_close(inwindow_4)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (state_6 && (rc = state_6->c_close(state_6)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &inwindow_9, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   if ((rc = status->state->cursor(status->state, NULL, &state_11, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int ID;
         int Next;
         int ID_expire;
         int Next_expire;
         struct timeval atime;
      } inwindow_9_10;
      next_9:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = inwindow_9->c_get(inwindow_9, &key, &data, (first_entry_10)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_10 = 0;
         memcpy(&(inwindow_9_10.ID), (char*)data.data+0, sizeof(int));
         //printf("Retrieved inwindow_9_10.ID = %d\n", inwindow_9_10.ID);
         //fflush(stdout);
         memcpy(&(inwindow_9_10.Next), (char*)data.data+4, sizeof(int));
         //printf("Retrieved inwindow_9_10.Next = %d\n", inwindow_9_10.Next);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_10 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         tmpStore(&key, &data);
         rc = 0;          /* subquery could've overwritten rc */
         if (!((ID == -1))) {
            goto next_9;
         }
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         tmpRecover(&key, &data);
          /* SUBQUERY IN UPDATE STARTS */
         kdPush(&key,&data);
         struct {
            int max;
            int max_expire;
            struct timeval atime;
         } embed_12;
         int embed_12_cnt = 0;
         struct {
            int max;
            int max_expire;
            struct timeval atime;
         } state_11_13;
         next_11:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_11->c_get(state_11, &key, &data, (first_entry_13)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_13 = 0;
            memcpy(&(state_11_13.max), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_11_13.max = %d\n", state_11_13.max);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_13 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            embed_12.max = state_11_13.max;
         } /* if (rc == 0) */
         kdPop(&key, &data);
         /* SUBQUERY IN UPDATE ENDS */
         *(int*)((char*)data.data+4) = embed_12.max;
         if ((rc = inwindow_9->c_put(inwindow_9, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (inwindow_9 && (rc = inwindow_9->c_close(inwindow_9)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (state_11 && (rc = state_11->c_close(state_11)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_15:
      rc = (first_entry_15)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_15=1;
      else {
         first_entry_15=0;
         insert_14.field_0 = 11111;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_14.field_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_14.field_0), sizeof(int));
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
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->state->cursor(status->state, NULL, &state_18, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int max;
         int max_expire;
         struct timeval atime;
      } state_18_17;
      next_18:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = state_18->c_get(state_18, &key, &data, (first_entry_17)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_17 = 0;
         memcpy(&(state_18_17.max), (char*)data.data+0, sizeof(int));
         //printf("Retrieved state_18_17.max = %d\n", state_18_17.max);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_17 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_16.max = state_18_17.max;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_16.max), sizeof(int));
         memcpy((char*)data.data+0, &(insert_16.max), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (state_18 && (rc = state_18->c_close(state_18)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &inwindow_25, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0001_21_20;
      next_21:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0000_23_22;
      next_23:
      while (index_23>=0 && index_23 < 2) {
         switch(index_23) {
            case 0:
            {
               if (terminating_23 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int ID;
                     int Next;
                     int ID_expire;
                     int Next_expire;
                     struct timeval atime;
                  } inwindow_25_24;
                  next_25:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = inwindow_25->c_get(inwindow_25, &key, &data, (first_entry_24)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_24 = 0;
                     memcpy(&(inwindow_25_24.ID), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved inwindow_25_24.ID = %d\n", inwindow_25_24.ID);
                     //fflush(stdout);
                     memcpy(&(inwindow_25_24.Next), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved inwindow_25_24.Next = %d\n", inwindow_25_24.Next);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_24 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0000_23_22.a_0 = ID;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_23 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_23 = (struct gb_status_23 *)0;
                     rc = hash_get(23, _rec_id, gbkey, 4, (char**)&gbstatus_23);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_23 = (struct gb_status_23*)malloc(sizeof(*gbstatus_23));
                        gbstatus_23->_baggr_0_first_entry = 1;
                        gbstatus_23->_baggr_0_value =  Q_0000_23_22.a_0;
                        rc = hash_put(23, _rec_id, gbkey, 4, &gbstatus_23);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        gbstatus_23->_baggr_0_first_entry = 1;
                        if (gbstatus_23->_baggr_0_value <  Q_0000_23_22.a_0) {
                           gbstatus_23->_baggr_0_value =  Q_0000_23_22.a_0;
                        }
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_23 = 1;
                  }
               }
               if (terminating_23 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(23, _rec_id, allkey, 4, (char**)&gbstatus_23);
                  if (rc==0) {
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               rc = DB_NOTFOUND;
               if (terminating_23 == 1) {
                  if (gbstatus_23->_baggr_0_first_entry == 1) {
                     Q_0001_21_20.a_0 = gbstatus_23->_baggr_0_value;
                     gbstatus_23->_baggr_0_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_23->_baggr_0_first_entry = 1;
                  }
               }
               first_entry_23 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_23++;
         }
         if (rc == DB_NOTFOUND) {
            index_23--;
            if (terminating_23 == 1 && index_23 == 0) {
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_23--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_23 = 0;
         first_entry_23 = 1;
         index_23 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(23, _rec_id, allkey, 4, (char**)&gbstatus_23);
            if (rc==0) {
               //printf("freeing 23\n");
               free(gbstatus_23);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(23, _rec_id);
      }
      if (rc==0) {
         insert_19.a_0 = Q_0001_21_20.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_19.a_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_19.a_0), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (inwindow_25 && (rc = inwindow_25->c_close(inwindow_25)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &inwindow_32, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0003_28_27;
      next_28:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0002_30_29;
      next_30:
      while (index_30>=0 && index_30 < 2) {
         switch(index_30) {
            case 0:
            {
               if (terminating_30 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int ID;
                     int Next;
                     int ID_expire;
                     int Next_expire;
                     struct timeval atime;
                  } inwindow_32_31;
                  next_32:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = inwindow_32->c_get(inwindow_32, &key, &data, (first_entry_31)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_31 = 0;
                     memcpy(&(inwindow_32_31.ID), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved inwindow_32_31.ID = %d\n", inwindow_32_31.ID);
                     //fflush(stdout);
                     memcpy(&(inwindow_32_31.Next), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved inwindow_32_31.Next = %d\n", inwindow_32_31.Next);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_31 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0002_30_29.a_0 = Next;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_30 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_30 = (struct gb_status_30 *)0;
                     rc = hash_get(30, _rec_id, gbkey, 4, (char**)&gbstatus_30);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_30 = (struct gb_status_30*)malloc(sizeof(*gbstatus_30));
                        gbstatus_30->_baggr_0_first_entry = 1;
                        gbstatus_30->_baggr_0_value =  Q_0002_30_29.a_0;
                        rc = hash_put(30, _rec_id, gbkey, 4, &gbstatus_30);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        gbstatus_30->_baggr_0_first_entry = 1;
                        if (gbstatus_30->_baggr_0_value <  Q_0002_30_29.a_0) {
                           gbstatus_30->_baggr_0_value =  Q_0002_30_29.a_0;
                        }
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_30 = 1;
                  }
               }
               if (terminating_30 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(30, _rec_id, allkey, 4, (char**)&gbstatus_30);
                  if (rc==0) {
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               rc = DB_NOTFOUND;
               if (terminating_30 == 1) {
                  if (gbstatus_30->_baggr_0_first_entry == 1) {
                     Q_0003_28_27.a_0 = gbstatus_30->_baggr_0_value;
                     gbstatus_30->_baggr_0_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_30->_baggr_0_first_entry = 1;
                  }
               }
               first_entry_30 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_30++;
         }
         if (rc == DB_NOTFOUND) {
            index_30--;
            if (terminating_30 == 1 && index_30 == 0) {
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_30--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_30 = 0;
         first_entry_30 = 1;
         index_30 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(30, _rec_id, allkey, 4, (char**)&gbstatus_30);
            if (rc==0) {
               //printf("freeing 30\n");
               free(gbstatus_30);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(30, _rec_id);
      }
      if (rc==0) {
         insert_26.a_0 = Q_0003_28_27.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_26.a_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_26.a_0), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (inwindow_32 && (rc = inwindow_32->c_close(inwindow_32)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void UpdateInWin_window_terminate(struct UpdateInWin_window_status *status, 
	int ID, int Next, int _rec_id, int not_delete, bufferMngr* bm, 
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
   struct inWinType_UpdateInWin_window tuple;
   sprintf(_adl_dbname, "._%d_inwindow", status);
   if(!not_delete) {
      if (status->inwindow && ((rc = status->inwindow->close(status->inwindow, 0)) != 0)) {
         adlabort(rc, "DB->close()");
      }
      status->inwindow = NULL;
   }
   sprintf(_adl_dbname, "._%d_state", status);
   if(!not_delete) {
      if (status->state && ((rc = status->state->close(status->state, 0)) != 0)) {
         adlabort(rc, "DB->close()");
      }
      status->state = NULL;
      (void)unlink(_adl_dbname);
   }
   if(!not_delete) status->retc_first_entry=1;
}
/**** Query Declarations ****/
int _adl_statement_33()
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
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   {
      FILE *_adl_load = fopen("./test", "rt");
      char _adl_load_buf[40960], *tok;
      char loadkeybuf[1], loaddatabuf[17];
      int _adl_line_no=0;
      char bPoint =0;
      if (!_adl_load) {
         printf("can not open file ./test.\n");
         exit(1);
      }
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      key.data = loadkeybuf;
      data.data = loaddatabuf;
      key.size = 0;
      data.size = 16;
      while (fgets(_adl_load_buf, 40959, _adl_load)) {
         _adl_line_no++;
         tok = csvtok(_adl_load_buf, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(int*)((char*)data.data+0) = atoi(tok);
         tok = csvtok(NULL, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         memset((char*)data.data+4, 0, 8);
         memcpy((char*)data.data+4, &getTimeval(tok), 8);
         if ((rc = traffic->put(traffic, &key, &data, DB_APPEND))!=0) {
            exit(rc);
         }
      } /* end of while */
      fclose(_adl_load);
   }
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_41()
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
      int a_0;
      int a_0_expire;
      struct timeval atime;
   } insert_34;
   IM_RELC *traffic_40;
   int first_entry_39 = 1;
   int first_entry_40 = 1;
   int index_38 = 0;
   int terminating_38=0;
   struct gb_status_38 {
      struct UpdateInWin_window_status *UpdateInWin_window_0;
   };
   struct gb_status_38 *gbstatus_38 = (struct gb_status_38 *)0;
   
   int first_entry_38 = 1;
   int first_entry_36 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = traffic->cursor(traffic, &traffic_40, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0005_36_35;
      next_36:
      struct {
         int a_0;
         int a_1;
         int a_0_expire;
         int a_1_expire;
         struct timeval atime;
      } Q_0004_38_37;
      next_38:
      while (index_38>=0 && index_38 < 2) {
         switch(index_38) {
            case 0:
            {
               if (terminating_38 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int in1;
                     struct timeval time1;
                     int OID;
                     int in1_expire;
                     struct timeval time1_expire;
                     int OID_expire;
                     struct timeval atime;
                  } traffic_40_39;
                  next_40:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = traffic_40->c_get(traffic_40, &key, &data, (first_entry_39)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_39 = 0;
                     memcpy(&(traffic_40_39.in1), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved traffic_40_39.in1 = %d\n", traffic_40_39.in1);
                     //fflush(stdout);
                     memcpy(&(traffic_40_39.time1), (char*)data.data+4, sizeof(struct timeval));
                     memcpy(&(traffic_40_39.OID), (char*)data.data+12, sizeof(int));
                     //printf("Retrieved traffic_40_39.OID = %d\n", traffic_40_39.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_39 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0004_38_37.a_0 = -1;
                     Q_0004_38_37.a_1 = traffic_40_39.in1;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_38 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     memcpy(gbkey+0, &traffic_40_39.time1, 8);
                     gbstatus_38 = (struct gb_status_38 *)0;
                     rc = hash_get(38, _rec_id, gbkey, 8, (char**)&gbstatus_38);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_38 = (struct gb_status_38*)malloc(sizeof(*gbstatus_38));
                        gbstatus_38->UpdateInWin_window_0 = (struct UpdateInWin_window_status*)malloc(sizeof(struct UpdateInWin_window_status));
                        gbstatus_38->UpdateInWin_window_0->win = 0;
                        gbstatus_38->UpdateInWin_window_0->win = new winbuf(20, 8, 8, _ADL_WIN_ROW);
                        gbstatus_38->UpdateInWin_window_0->last_out = 0;
                        gbstatus_38->UpdateInWin_window_0->iterate = false;
                        gbstatus_38->UpdateInWin_window_0->init = true;
                        setModelId("");
                        memcpy(_databuf+0, &(Q_0004_38_37.a_0), sizeof(int));
                        memcpy(_databuf+4, &(Q_0004_38_37.a_1), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 8;
                        gbstatus_38->UpdateInWin_window_0->win->updateTupleID();
                        gbstatus_38->UpdateInWin_window_0->win->put(&windata);
                        if((!(10 <= 1 || ((gbstatus_38->UpdateInWin_window_0->win->getTupleID() == 0 && gbstatus_38->UpdateInWin_window_0->last_out != 0) 
                        ||((((int)gbstatus_38->UpdateInWin_window_0->win->getTupleID()) >= gbstatus_38->UpdateInWin_window_0->last_out + 10)))))) {
                           slide_out = 0;
                           //printf("Here no output %d 10 %d\n", gbstatus_38->UpdateInWin_window_0->last_out, gbstatus_38->UpdateInWin_window_0->win->getTupleID());fflush(stdout);
                        } else {
                           slide_out = 1;
                           //printf("Here YES output %d 10 %d\n", gbstatus_38->UpdateInWin_window_0->last_out, gbstatus_38->UpdateInWin_window_0->win->getTupleID());fflush(stdout);
                           gbstatus_38->UpdateInWin_window_0->last_out = gbstatus_38->UpdateInWin_window_0->last_out + 10;
                           while(gbstatus_38->UpdateInWin_window_0->last_out < (gbstatus_38->UpdateInWin_window_0->win->getTupleID() - 10) && gbstatus_38->UpdateInWin_window_0->win->getTupleID() > 0) {
                              if(10 == 1) {
                                 gbstatus_38->UpdateInWin_window_0->last_out = gbstatus_38->UpdateInWin_window_0->win->getTupleID();
                              }
                              else {
                                 gbstatus_38->UpdateInWin_window_0->last_out = gbstatus_38->UpdateInWin_window_0->last_out + 10;
                              }
                           }
                        }
                        UpdateInWin_window_init(gbstatus_38->UpdateInWin_window_0, Q_0004_38_37.a_0, Q_0004_38_37.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(38, _rec_id, gbkey, 8, &gbstatus_38);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        memcpy(_databuf+0, &(Q_0004_38_37.a_0), sizeof(int));
                        memcpy(_databuf+4, &(Q_0004_38_37.a_1), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 8;
                        gbstatus_38->UpdateInWin_window_0->win->updateTupleID();
                        gbstatus_38->UpdateInWin_window_0->win->put(&windata);
                        if((!(10 <= 1 || ((gbstatus_38->UpdateInWin_window_0->win->getTupleID() == 0 && gbstatus_38->UpdateInWin_window_0->last_out != 0) 
                        ||((((int)gbstatus_38->UpdateInWin_window_0->win->getTupleID()) >= gbstatus_38->UpdateInWin_window_0->last_out + 10)))))) {
                           slide_out = 0;
                           //printf("Here no output %d 10 %d\n", gbstatus_38->UpdateInWin_window_0->last_out, gbstatus_38->UpdateInWin_window_0->win->getTupleID());fflush(stdout);
                        } else {
                           slide_out = 1;
                           //printf("Here YES output %d 10 %d\n", gbstatus_38->UpdateInWin_window_0->last_out, gbstatus_38->UpdateInWin_window_0->win->getTupleID());fflush(stdout);
                           gbstatus_38->UpdateInWin_window_0->last_out = gbstatus_38->UpdateInWin_window_0->last_out + 10;
                           while(gbstatus_38->UpdateInWin_window_0->last_out < (gbstatus_38->UpdateInWin_window_0->win->getTupleID() - 10) && gbstatus_38->UpdateInWin_window_0->win->getTupleID() > 0) {
                              if(10 == 1) {
                                 gbstatus_38->UpdateInWin_window_0->last_out = gbstatus_38->UpdateInWin_window_0->win->getTupleID();
                              }
                              else {
                                 gbstatus_38->UpdateInWin_window_0->last_out = gbstatus_38->UpdateInWin_window_0->last_out + 10;
                              }
                           }
                        }
                        while (gbstatus_38->UpdateInWin_window_0->win->hasExpired()){
                           gbstatus_38->UpdateInWin_window_0->win->getExpired(&windata);
                           gbstatus_38->UpdateInWin_window_0->win->pop();
                        }
                        UpdateInWin_window_iterate(gbstatus_38->UpdateInWin_window_0, Q_0004_38_37.a_0, Q_0004_38_37.a_1, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_38 = 1;
                  }
               }
               if (terminating_38 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(38, _rec_id, allkey, 8, (char**)&gbstatus_38);
                  if (rc==0) {
                     setModelId("");
                     memcpy(_databuf+0, &(Q_0004_38_37.a_0), sizeof(int));
                     memcpy(_databuf+4, &(Q_0004_38_37.a_1), sizeof(int));
                     windata.data = _databuf;
                     windata.size = 8;
                     gbstatus_38->UpdateInWin_window_0->win->updateTupleID();
                     gbstatus_38->UpdateInWin_window_0->win->put(&windata);
                     if((!(10 <= 1 || ((gbstatus_38->UpdateInWin_window_0->win->getTupleID() == 0 && gbstatus_38->UpdateInWin_window_0->last_out != 0) 
                     ||((((int)gbstatus_38->UpdateInWin_window_0->win->getTupleID()) >= gbstatus_38->UpdateInWin_window_0->last_out + 10)))))) {
                        slide_out = 0;
                        //printf("Here no output %d 10 %d\n", gbstatus_38->UpdateInWin_window_0->last_out, gbstatus_38->UpdateInWin_window_0->win->getTupleID());fflush(stdout);
                     } else {
                        slide_out = 1;
                        //printf("Here YES output %d 10 %d\n", gbstatus_38->UpdateInWin_window_0->last_out, gbstatus_38->UpdateInWin_window_0->win->getTupleID());fflush(stdout);
                        gbstatus_38->UpdateInWin_window_0->last_out = gbstatus_38->UpdateInWin_window_0->last_out + 10;
                        while(gbstatus_38->UpdateInWin_window_0->last_out < (gbstatus_38->UpdateInWin_window_0->win->getTupleID() - 10) && gbstatus_38->UpdateInWin_window_0->win->getTupleID() > 0) {
                           if(10 == 1) {
                              gbstatus_38->UpdateInWin_window_0->last_out = gbstatus_38->UpdateInWin_window_0->win->getTupleID();
                           }
                           else {
                              gbstatus_38->UpdateInWin_window_0->last_out = gbstatus_38->UpdateInWin_window_0->last_out + 10;
                           }
                        }
                     }
                     UpdateInWin_window_terminate(gbstatus_38->UpdateInWin_window_0, Q_0004_38_37.a_0, Q_0004_38_37.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_38->UpdateInWin_window_0->retc->c_get(gbstatus_38->UpdateInWin_window_0->retc, &key, &data, (gbstatus_38->UpdateInWin_window_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_38->UpdateInWin_window_0->retc_first_entry = 0;
                  memcpy(&(Q_0005_36_35.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0005_36_35.a_0 = %d\n", Q_0005_36_35.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_38->UpdateInWin_window_0->retc->c_del(gbstatus_38->UpdateInWin_window_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_38->UpdateInWin_window_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_38 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_38++;
         }
         if (rc == DB_NOTFOUND) {
            index_38--;
            if (terminating_38 == 1 && index_38 == 0) {
               if (gbstatus_38->UpdateInWin_window_0->retc && (rc = gbstatus_38->UpdateInWin_window_0->retc->c_close(gbstatus_38->UpdateInWin_window_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_38->UpdateInWin_window_0);
               
               if (gbstatus_38->UpdateInWin_window_0->ret && ((rc = gbstatus_38->UpdateInWin_window_0->ret->close(gbstatus_38->UpdateInWin_window_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_38->UpdateInWin_window_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_38--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_38 = 0;
         first_entry_38 = 1;
         index_38 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(38, _rec_id, allkey, 8, (char**)&gbstatus_38);
            if (rc==0) {
               if(gbstatus_38->UpdateInWin_window_0) {
                  if(gbstatus_38->UpdateInWin_window_0->win) {
                     delete(gbstatus_38->UpdateInWin_window_0->win);
                     gbstatus_38->UpdateInWin_window_0->win = 0;
                  }
                  free(gbstatus_38->UpdateInWin_window_0);
               }
               //printf("freeing 38\n");
               free(gbstatus_38);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(38, _rec_id);
      }
      if (rc==0) {
         insert_34.a_0 = Q_0005_36_35.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_34.a_0);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (traffic_40 && (rc = traffic_40->c_close(traffic_40)) != 0) {
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
   if ((rc = im_rel_create(&traffic, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = traffic->open(traffic, "_adl_db_traffic", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("traffic") == 0) {
      inMemTables->operator[](strdup("traffic")) = traffic;
   }
   _adl_statement_33();
   _adl_statement_41();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = traffic->close(traffic, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
