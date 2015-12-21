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
IM_REL *traffic_2;
IM_REL *traffic_3;
struct myavg_online_status {
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
extern "C" void myavg_online_init(struct myavg_online_status *status, 
	int Next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void myavg_online_iterate(struct myavg_online_status *status, 
	int Next, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void myavg_online_terminate(struct myavg_online_status *status, 
	int Next, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL);
extern "C" void myavg_online_init(struct myavg_online_status *status, int Next,
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
      int Next;
      int field_1;
      int Next_expire;
      int field_1_expire;
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
         insert_0.Next = Next;
         insert_0.field_1 = 1;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_0.Next), sizeof(int));
         memcpy((char*)data.data+4, &(insert_0.field_1), sizeof(int));
         data.size = 8;
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
extern "C" void myavg_online_iterate(struct myavg_online_status *status, 
	int Next, int _rec_id, bufferMngr* bm, 
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
   DBC *state_2;
   int first_entry_3 = 1;
   int first_entry_2 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->state->cursor(status->state, NULL, &state_2, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int sum;
         int cnt;
         int sum_expire;
         int cnt_expire;
         struct timeval atime;
      } state_2_3;
      next_2:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = state_2->c_get(state_2, &key, &data, (first_entry_3)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_3 = 0;
         memcpy(&(state_2_3.sum), (char*)data.data+0, sizeof(int));
         //printf("Retrieved state_2_3.sum = %d\n", state_2_3.sum);
         //fflush(stdout);
         memcpy(&(state_2_3.cnt), (char*)data.data+4, sizeof(int));
         //printf("Retrieved state_2_3.cnt = %d\n", state_2_3.cnt);
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
         *(int*)((char*)data.data+0) = ((state_2_3.sum) + Next);
         *(int*)((char*)data.data+4) = ((state_2_3.cnt) + 1);
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
   status->retc_first_entry=1;
}
extern "C" void myavg_online_terminate(struct myavg_online_status *status, 
	int Next, int _rec_id, int not_delete, bufferMngr* bm, 
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
      double field_0;
      double field_0_expire;
      struct timeval atime;
   } insert_4;
   DBC *state_6;
   int first_entry_5 = 1;
   int first_entry_6 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->state->cursor(status->state, NULL, &state_6, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int sum;
         int cnt;
         int sum_expire;
         int cnt_expire;
         struct timeval atime;
      } state_6_5;
      next_6:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = state_6->c_get(state_6, &key, &data, (first_entry_5)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_5 = 0;
         memcpy(&(state_6_5.sum), (char*)data.data+0, sizeof(int));
         //printf("Retrieved state_6_5.sum = %d\n", state_6_5.sum);
         //fflush(stdout);
         memcpy(&(state_6_5.cnt), (char*)data.data+4, sizeof(int));
         //printf("Retrieved state_6_5.cnt = %d\n", state_6_5.cnt);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_5 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_4.field_0 = ((state_6_5.sum) / state_6_5.cnt);
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_4.field_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_4.field_0), sizeof(double));
         data.size = 8;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (state_6 && (rc = state_6->c_close(state_6)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   sprintf(_adl_dbname, "._%d_inwindow", status);
   if(!not_delete) {
      if (status->inwindow && ((rc = status->inwindow->close(status->inwindow, 0)) != 0)) {
         adlabort(rc, "DB->close()");
      }
      status->inwindow = NULL;
      (void)unlink(_adl_dbname);
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
struct inWinType_myavg_online_window {
   int Next;
};
struct myavg_online_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_myavg_online_window* getLastTuple_myavg_online_window(IM_REL* inwindow, inWinType_myavg_online_window* tuple, bufferMngr* bm) {
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
   memcpy(&((*tuple).Next), (char*)data.data+0, sizeof(int));
   //printf("Retrieved (*tuple).Next = %d\n", (*tuple).Next);
   //fflush(stdout);
   return tuple;
}
extern "C" void myavg_online_window_init(struct myavg_online_window_status *status, 
	int Next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void myavg_online_window_init(struct myavg_online_window_status *status, int Next, 
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
   struct inWinType_myavg_online_window tuple;
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
   struct {
      double a_0;
      double a_0_expire;
      struct timeval atime;
   } insert_7;
   IM_RELC *w_13;
   int first_entry_12 = 1;
   int first_entry_13 = 1;
   int index_11 = 0;
   int terminating_11=0;
   struct gb_status_11 {
      struct myavg_online_status *myavg_online_0;
   };
   struct gb_status_11 *gbstatus_11 = (struct gb_status_11 *)0;
   
   int first_entry_11 = 1;
   int first_entry_9 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_13, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         double a_0;
         double a_0_expire;
         struct timeval atime;
      } Q_0001_9_8;
      next_9:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0000_11_10;
      next_11:
      while (index_11>=0 && index_11 < 2) {
         switch(index_11) {
            case 0:
            {
               if (terminating_11 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int Next;
                     int Next_expire;
                     struct timeval atime;
                  } w_13_12;
                  next_13:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_13->c_get(w_13, &key, &data, (first_entry_12)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_12 = 0;
                     memcpy(&(w_13_12.Next), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_13_12.Next = %d\n", w_13_12.Next);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_12 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0000_11_10.a_0 = w_13_12.Next;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_11 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_11 = (struct gb_status_11 *)0;
                     rc = hash_get(11, _rec_id, gbkey, 4, (char**)&gbstatus_11);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_11 = (struct gb_status_11*)malloc(sizeof(*gbstatus_11));
                        gbstatus_11->myavg_online_0 = (struct myavg_online_status*)malloc(sizeof(struct myavg_online_status));
                        gbstatus_11->myavg_online_0->win = 0;
                        setModelId("");
                        myavg_online_init(gbstatus_11->myavg_online_0, Q_0000_11_10.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(11, _rec_id, gbkey, 4, &gbstatus_11);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        myavg_online_iterate(gbstatus_11->myavg_online_0, Q_0000_11_10.a_0, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_11 = 1;
                  }
               }
               if (terminating_11 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(11, _rec_id, allkey, 4, (char**)&gbstatus_11);
                  if (rc==0) {
                     setModelId("");
                     myavg_online_terminate(gbstatus_11->myavg_online_0, Q_0000_11_10.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_11->myavg_online_0->retc->c_get(gbstatus_11->myavg_online_0->retc, &key, &data, (gbstatus_11->myavg_online_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_11->myavg_online_0->retc_first_entry = 0;
                  memcpy(&(Q_0001_9_8.a_0), (char*)data.data+0, sizeof(double));
                  //printf("Retrieved Q_0001_9_8.a_0 = %f\n", Q_0001_9_8.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_11->myavg_online_0->retc->c_del(gbstatus_11->myavg_online_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_11->myavg_online_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_11 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_11++;
         }
         if (rc == DB_NOTFOUND) {
            index_11--;
            if (terminating_11 == 1 && index_11 == 0) {
               if (gbstatus_11->myavg_online_0->retc && (rc = gbstatus_11->myavg_online_0->retc->c_close(gbstatus_11->myavg_online_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_11->myavg_online_0);
               
               if (gbstatus_11->myavg_online_0->ret && ((rc = gbstatus_11->myavg_online_0->ret->close(gbstatus_11->myavg_online_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_11->myavg_online_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_11--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_11 = 0;
         first_entry_11 = 1;
         index_11 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(11, _rec_id, allkey, 4, (char**)&gbstatus_11);
            if (rc==0) {
               free(gbstatus_11->myavg_online_0);
               //printf("freeing 11\n");
               free(gbstatus_11);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(11, _rec_id);
      }
      if (rc==0) {
         insert_7.a_0 = Q_0001_9_8.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_7.a_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_7.a_0), sizeof(double));
         data.size = 8;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_13 && (rc = w_13->c_close(w_13)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
/**** Query Declarations ****/
int _adl_statement_14()
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
      FILE *_adl_load = fopen("./test1", "rt");
      char _adl_load_buf[40960], *tok;
      char loadkeybuf[1], loaddatabuf[17];
      int _adl_line_no=0;
      char bPoint =0;
      if (!_adl_load) {
         printf("can not open file ./test1.\n");
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
int _adl_statement_20()
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
      int in1;
      int field_1;
      int field_2;
      int in1_expire;
      int field_1_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_15;
   IM_RELC *a_17;
   int first_entry_16 = 1;
   IM_RELC *b_17;
   int first_entry_18 = 1;
   IM_RELC *c_17;
   int first_entry_19 = 1;
   int index_17 = 0;
   int first_entry_17 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = traffic->cursor(traffic, &a_17, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = traffic_2->cursor(traffic_2, &b_17, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = traffic_3->cursor(traffic_3, &c_17, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int in1;
         struct timeval time1;
         int OID;
         int in1_expire;
         struct timeval time1_expire;
         int OID_expire;
         struct timeval atime;
      } a_17_16;
      struct {
         int in1;
         struct timeval time1;
         int OID;
         int in1_expire;
         struct timeval time1_expire;
         int OID_expire;
         struct timeval atime;
      } b_17_18;
      struct {
         int in1;
         struct timeval time1;
         int OID;
         int in1_expire;
         struct timeval time1_expire;
         int OID_expire;
         struct timeval atime;
      } c_17_19;
      next_17:
      while (index_17>=0 && index_17 < 3) { 
         switch(index_17) {
            case 0:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = a_17->c_get(a_17, &key, &data, (first_entry_16)? DB_FIRST:DB_NEXT);
               if (rc==0) {
                  first_entry_16 = 0;
                  memcpy(&(a_17_16.in1), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved a_17_16.in1 = %d\n", a_17_16.in1);
                  //fflush(stdout);
                  memcpy(&(a_17_16.time1), (char*)data.data+4, sizeof(struct timeval));
                  memcpy(&(a_17_16.OID), (char*)data.data+12, sizeof(int));
                  //printf("Retrieved a_17_16.OID = %d\n", a_17_16.OID);
                  //fflush(stdout);
               } else if (rc == DB_NOTFOUND) {
                  first_entry_16 = 1;
               } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = b_17->c_get(b_17, &key, &data, (first_entry_18)? DB_FIRST:DB_NEXT);
               if (rc==0) {
                  first_entry_18 = 0;
                  memcpy(&(b_17_18.in1), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved b_17_18.in1 = %d\n", b_17_18.in1);
                  //fflush(stdout);
                  memcpy(&(b_17_18.time1), (char*)data.data+4, sizeof(struct timeval));
                  memcpy(&(b_17_18.OID), (char*)data.data+12, sizeof(int));
                  //printf("Retrieved b_17_18.OID = %d\n", b_17_18.OID);
                  //fflush(stdout);
               } else if (rc == DB_NOTFOUND) {
                  first_entry_18 = 1;
               } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            }
            break;
            case 2:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = c_17->c_get(c_17, &key, &data, (first_entry_19)? DB_FIRST:DB_NEXT);
               if (rc==0) {
                  first_entry_19 = 0;
                  memcpy(&(c_17_19.in1), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved c_17_19.in1 = %d\n", c_17_19.in1);
                  //fflush(stdout);
                  memcpy(&(c_17_19.time1), (char*)data.data+4, sizeof(struct timeval));
                  memcpy(&(c_17_19.OID), (char*)data.data+12, sizeof(int));
                  //printf("Retrieved c_17_19.OID = %d\n", c_17_19.OID);
                  //fflush(stdout);
               } else if (rc == DB_NOTFOUND) {
                  first_entry_19 = 1;
               } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            }
            break;
         } /*switch */
         if (rc==0) {
            index_17++;
         } else if (rc==DB_NOTFOUND) {
            index_17--;
         }
      } /* while */
      if (rc!=0) {
         index_17++;    /* set index to the first subgoal */
      } else {
         index_17--;    /* set index to the last subgoal */
         insert_15.in1 = a_17_16.in1;
         insert_15.field_1 = b_17_18.in1;
         insert_15.field_2 = c_17_19.in1;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_15.in1);
         printf("%10d ", insert_15.field_1);
         printf("%10d ", insert_15.field_2);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (a_17 && (rc = a_17->c_close(a_17)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (b_17 && (rc = b_17->c_close(b_17)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (c_17 && (rc = c_17->c_close(c_17)) != 0) {
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
   if ((rc = im_rel_create(&traffic_2, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = traffic_2->open(traffic_2, "_adl_db_traffic_2", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("traffic_2") == 0) {
      inMemTables->operator[](strdup("traffic_2")) = traffic_2;
   }
   if ((rc = im_rel_create(&traffic_3, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = traffic_3->open(traffic_3, "_adl_db_traffic_3", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("traffic_3") == 0) {
      inMemTables->operator[](strdup("traffic_3")) = traffic_3;
   }
   _adl_statement_14();
   _adl_statement_20();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = traffic->close(traffic, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = traffic_2->close(traffic_2, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = traffic_3->close(traffic_3, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
