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
struct inWinType_TopkNoOrder_window {
   int Next;
};
struct TopkNoOrder_window_status {
   DB *state;
   int _state_cmp(DB* dbp, const DBT *a, const DBT *b){
      	int ai, bi, ri, rs;
      	double ad, bd, rd;
      	struct timeval *at, *bt;return 0;
   };
   DB *myWin;
   int _myWin_cmp(DB* dbp, const DBT *a, const DBT *b){
      	int ai, bi, ri, rs;
      	double ad, bd, rd;
      	struct timeval *at, *bt;return 0;
   };
   DB *resu;
   int _resu_cmp(DB* dbp, const DBT *a, const DBT *b){
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
inWinType_TopkNoOrder_window* getLastTuple_TopkNoOrder_window(IM_REL* inwindow, inWinType_TopkNoOrder_window* tuple, bufferMngr* bm) {
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
extern "C" void TopkNoOrder_window_init(struct TopkNoOrder_window_status *status, 
	int Next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void TopkNoOrder_window_iterate(struct TopkNoOrder_window_status *status, 
	int Next, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void TopkNoOrder_window_expire(struct TopkNoOrder_window_status *status, 
	int Next, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void TopkNoOrder_window_init(struct TopkNoOrder_window_status *status, int Next, 
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
   struct inWinType_TopkNoOrder_window tuple;
   if (__is_init) {
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
      sprintf(_adl_dbname, "._%d_myWin", status);
      (void)unlink(_adl_dbname);
      if ((rc = db_create(&status->myWin, NULL, 0)) != 0) {
         adlabort(rc, "db_create()");
      }
      if ((rc = status->myWin->set_pagesize(status->myWin, 2048)) != 0) {
         adlabort(rc, "set_pagesize()");
      }
      if ((rc = status->myWin->set_flags(status->myWin, DB_DUP)) != 0) {
         adlabort(rc, "set_flags()");
      }
      if ((rc = status->myWin->open(status->myWin, _adl_dbname, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
         adlabort(rc, "open()");
      }
      sprintf(_adl_dbname, "._%d_resu", status);
      (void)unlink(_adl_dbname);
      if ((rc = db_create(&status->resu, NULL, 0)) != 0) {
         adlabort(rc, "db_create()");
      }
      if ((rc = status->resu->set_pagesize(status->resu, 2048)) != 0) {
         adlabort(rc, "set_pagesize()");
      }
      if ((rc = status->resu->set_flags(status->resu, DB_DUP)) != 0) {
         adlabort(rc, "set_flags()");
      }
      if ((rc = status->resu->open(status->resu, _adl_dbname, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
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
      double field_1;
      int field_0_expire;
      double field_1_expire;
      struct timeval atime;
   } insert_0;
   int first_entry_1 = 1;
   struct {
      int field_0;
      double Next;
      int field_2;
      int field_0_expire;
      double Next_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_2;
   int first_entry_3 = 1;
   struct {
      int field_0;
      double Next;
      int field_0_expire;
      double Next_expire;
      struct timeval atime;
   } insert_4;
   int first_entry_5 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_1:
      rc = (first_entry_1)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_1=1;
      else {
         first_entry_1=0;
         insert_0.field_0 = 1;
         insert_0.field_1 = 1;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_0.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_0.field_1), sizeof(double));
         data.size = 12;
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
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_3:
      rc = (first_entry_3)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_3=1;
      else {
         first_entry_3=0;
         insert_2.field_0 = 1;
         insert_2.Next = Next;
         insert_2.field_2 = 0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_2.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_2.Next), sizeof(double));
         memcpy((char*)data.data+12, &(insert_2.field_2), sizeof(int));
         data.size = 16;
         key.size = 0;
         if ((rc = status->myWin->put(status->myWin, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         status->myWin->sync(status->myWin, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_5:
      rc = (first_entry_5)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_5=1;
      else {
         first_entry_5=0;
         insert_4.field_0 = 1;
         insert_4.Next = Next;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_4.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_4.Next), sizeof(double));
         data.size = 12;
         key.size = 0;
         if ((rc = status->resu->put(status->resu, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         status->resu->sync(status->resu, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void TopkNoOrder_window_iterate(struct TopkNoOrder_window_status *status, 
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
   if(status && status->win)
   window = status->win->get_im_rel();
   struct inWinType_TopkNoOrder_window tuple;
   DBC *state_6;
   int first_entry_7 = 1;
   int first_entry_8 = 1;
   DBC *myWin_14;
   int first_entry_13 = 1;
   int first_entry_14 = 1;
   int index_12 = 0;
   int terminating_12=0;
   struct gb_status_12 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int count_0_last_out;
      bool count_0_iterate;
      bool count_0_init;
   };
   struct gb_status_12 *gbstatus_12 = (struct gb_status_12 *)0;
   
   int first_entry_12 = 1;
   int first_entry_6 = 1;
   DBC *myWin_15;
   int first_entry_16 = 1;
   int first_entry_17 = 1;
   DBC *state_17;
   int first_entry_19 = 1;
   int first_entry_15 = 1;
   DBC *resu_20;
   int first_entry_21 = 1;
   int first_entry_22 = 1;
   DBC *state_22;
   int first_entry_24 = 1;
   int first_entry_20 = 1;
   struct {
      int field_0;
      double Next;
      int field_2;
      int field_0_expire;
      double Next_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_25;
   int first_entry_27 = 1;
   DBC *state_27;
   int first_entry_29 = 1;
   int first_entry_26 = 1;
   DBC *resu_30;
   int first_entry_31 = 1;
   int first_entry_32 = 1;
   DBC *resu_38;
   int first_entry_37 = 1;
   int first_entry_38 = 1;
   int index_36 = 0;
   int terminating_36=0;
   struct gb_status_36 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int min_0_last_out;
      bool min_0_iterate;
      bool min_0_init;
   };
   struct gb_status_36 *gbstatus_36 = (struct gb_status_36 *)0;
   
   int first_entry_36 = 1;
   int first_entry_39 = 1;
   DBC *resu_45;
   int first_entry_44 = 1;
   int first_entry_45 = 1;
   int index_43 = 0;
   int terminating_43=0;
   struct gb_status_43 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int min_0_last_out;
      bool min_0_iterate;
      bool min_0_init;
   };
   struct gb_status_43 *gbstatus_43 = (struct gb_status_43 *)0;
   
   int first_entry_43 = 1;
   int first_entry_46 = 1;
   DBC *state_46;
   int first_entry_48 = 1;
   int first_entry_30 = 1;
   struct {
      int id;
      double val;
      int id_expire;
      double val_expire;
      struct timeval atime;
   } insert_49;
   DBC *myWin_51;
   int first_entry_50 = 1;
   int first_entry_52 = 1;
   DBC *resu_58;
   int first_entry_57 = 1;
   int first_entry_58 = 1;
   int index_56 = 0;
   int terminating_56=0;
   struct gb_status_56 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int count_0_last_out;
      bool count_0_iterate;
      bool count_0_init;
   };
   struct gb_status_56 *gbstatus_56 = (struct gb_status_56 *)0;
   
   int first_entry_56 = 1;
   int first_entry_59 = 1;
   DBC *myWin_65;
   int first_entry_64 = 1;
   int first_entry_66 = 1;
   DBC *resu_66;
   int first_entry_68 = 1;
   int first_entry_65 = 1;
   int index_63 = 0;
   int terminating_63=0;
   struct gb_status_63 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int max_0_last_out;
      bool max_0_iterate;
      bool max_0_init;
   };
   struct gb_status_63 *gbstatus_63 = (struct gb_status_63 *)0;
   
   int first_entry_63 = 1;
   int first_entry_51 = 1;
   DBC *myWin_69;
   int first_entry_70 = 1;
   int first_entry_71 = 1;
   DBC *state_71;
   int first_entry_73 = 1;
   int first_entry_69 = 1;
   DBC *myWin_74;
   int first_entry_75 = 1;
   int first_entry_74 = 1;
   struct {
      double field_0;
      double field_0_expire;
      struct timeval atime;
   } insert_76;
   DBC *state_78;
   int first_entry_77 = 1;
   int first_entry_78 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->state->cursor(status->state, NULL, &state_6, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->myWin->cursor(status->myWin, NULL, &myWin_14, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int max;
         double sum;
         int max_expire;
         double sum_expire;
         struct timeval atime;
      } state_6_7;
      next_6:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = state_6->c_get(state_6, &key, &data, (first_entry_7)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_7 = 0;
         memcpy(&(state_6_7.max), (char*)data.data+0, sizeof(int));
         //printf("Retrieved state_6_7.max = %d\n", state_6_7.max);
         //fflush(stdout);
         memcpy(&(state_6_7.sum), (char*)data.data+4, sizeof(double));
         //printf("Retrieved state_6_7.sum = %f\n", state_6_7.sum);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_7 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         *(int*)((char*)data.data+0) = ((state_6_7.max) + 1);
          /* SUBQUERY IN UPDATE STARTS */
         kdPush(&key,&data);
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_9;
         int embed_9_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0001_8_10;
         next_8:
         struct {
            double a_0;
            double a_0_expire;
            struct timeval atime;
         } Q_0000_12_11;
         next_12:
         while (index_12>=0 && index_12 < 2) {
            switch(index_12) {
               case 0:
               {
                  if (terminating_12 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int id;
                        double val;
                        int cnt;
                        int id_expire;
                        double val_expire;
                        int cnt_expire;
                        struct timeval atime;
                     } myWin_14_13;
                     next_14:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = myWin_14->c_get(myWin_14, &key, &data, (first_entry_13)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_13 = 0;
                        memcpy(&(myWin_14_13.id), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved myWin_14_13.id = %d\n", myWin_14_13.id);
                        //fflush(stdout);
                        memcpy(&(myWin_14_13.val), (char*)data.data+4, sizeof(double));
                        //printf("Retrieved myWin_14_13.val = %f\n", myWin_14_13.val);
                        //fflush(stdout);
                        memcpy(&(myWin_14_13.cnt), (char*)data.data+12, sizeof(int));
                        //printf("Retrieved myWin_14_13.cnt = %d\n", myWin_14_13.cnt);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_13 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        Q_0000_12_11.a_0 = myWin_14_13.val;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_12 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_12 = (struct gb_status_12 *)0;
                        rc = hash_get(12, _rec_id, gbkey, 4, (char**)&gbstatus_12);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_12 = (struct gb_status_12*)malloc(sizeof(*gbstatus_12));
                           gbstatus_12->_baggr_0_first_entry = 1;
                           gbstatus_12->_baggr_0_value = 1;
                           rc = hash_put(12, _rec_id, gbkey, 4, &gbstatus_12);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_12->_baggr_0_first_entry = 1;
                           gbstatus_12->_baggr_0_value += 1;
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_12 = 1;
                     }
                  }
                  if (terminating_12 == 1) {
                     if (first_entry_12 == 1) {
                        rc = 0; /* fail on first entry, aggregate on empty set */
                     } else {
                        allkey = (char*)0;
                        rc = hash_get(12, _rec_id, allkey, 4, (char**)&gbstatus_12);
                        if (rc==0) {
                        } else if(rc == DB_NOTFOUND) {
                        } else adlabort(rc, "hash->get()");
                     }
                  }
               }
               break;
               case 1:
               {
                  rc = DB_NOTFOUND;
                  if (terminating_12 == 1) {
                     if (gbstatus_12 == (struct gb_status_12 *)0) {
                        if (first_entry_12) {
                           rc = 0;
                           Q_0001_8_10.a_0 = 0;
                        }
                     } else 
                     if (gbstatus_12->_baggr_0_first_entry == 1) {
                        Q_0001_8_10.a_0 = gbstatus_12->_baggr_0_value;
                        gbstatus_12->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_12->_baggr_0_first_entry = 1;
                     }
                  }
                  first_entry_12 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_12++;
            }
            if (rc == DB_NOTFOUND) {
               index_12--;
               if (terminating_12 == 1 && index_12 == 0) {
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_12--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_12 = 0;
            first_entry_12 = 1;
            index_12 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(12, _rec_id, allkey, 4, (char**)&gbstatus_12);
               if (rc==0) {
                  //printf("freeing 12\n");
                  free(gbstatus_12);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(12, _rec_id);
         }
         first_entry_8 = (rc)? 1:0;
         if (rc==0) {
            embed_9.a_0 = Q_0001_8_10.a_0;
            if (embed_9_cnt++ ==0) goto next_8; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_9_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 21.\n");
            exit(1);
         }
         else if (embed_9_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 21.\n");
            exit(1);
         }
         rc = 0;
         kdPop(&key, &data);
         /* SUBQUERY IN UPDATE ENDS */
         *(double*)((char*)data.data+4) = ((state_6_7.sum) + embed_9.a_0);
         if ((rc = state_6->c_put(state_6, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (state_6 && (rc = state_6->c_close(state_6)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (myWin_14 && (rc = myWin_14->c_close(myWin_14)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->myWin->cursor(status->myWin, NULL, &myWin_15, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->state->cursor(status->state, NULL, &state_17, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int id;
         double val;
         int cnt;
         int id_expire;
         double val_expire;
         int cnt_expire;
         struct timeval atime;
      } myWin_15_16;
      next_15:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = myWin_15->c_get(myWin_15, &key, &data, (first_entry_16)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_16 = 0;
         memcpy(&(myWin_15_16.id), (char*)data.data+0, sizeof(int));
         //printf("Retrieved myWin_15_16.id = %d\n", myWin_15_16.id);
         //fflush(stdout);
         memcpy(&(myWin_15_16.val), (char*)data.data+4, sizeof(double));
         //printf("Retrieved myWin_15_16.val = %f\n", myWin_15_16.val);
         //fflush(stdout);
         memcpy(&(myWin_15_16.cnt), (char*)data.data+12, sizeof(int));
         //printf("Retrieved myWin_15_16.cnt = %d\n", myWin_15_16.cnt);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_16 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         struct {
            int max;
            int max_expire;
            struct timeval atime;
         } embed_18;
         int embed_18_cnt = 0;
         struct {
            int max;
            double sum;
            int max_expire;
            double sum_expire;
            struct timeval atime;
         } state_17_19;
         next_17:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_17->c_get(state_17, &key, &data, (first_entry_19)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_19 = 0;
            memcpy(&(state_17_19.max), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_17_19.max = %d\n", state_17_19.max);
            //fflush(stdout);
            memcpy(&(state_17_19.sum), (char*)data.data+4, sizeof(double));
            //printf("Retrieved state_17_19.sum = %f\n", state_17_19.sum);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_19 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         first_entry_17 = (rc)? 1:0;
         if (rc==0) {
            embed_18.max = state_17_19.max;
            if (embed_18_cnt++ ==0) goto next_17; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_18_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 24.\n");
            exit(1);
         }
         else if (embed_18_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 24.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((((myWin_15_16.id) + 10000) == embed_18.max))) {
            goto next_15;
         }
         /*DELETE STARTS*/
         if ((rc = myWin_15->c_del(myWin_15, 0)) != 0) {
            adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
         }
         /*DELETE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (myWin_15 && (rc = myWin_15->c_close(myWin_15)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (state_17 && (rc = state_17->c_close(state_17)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->resu->cursor(status->resu, NULL, &resu_20, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->state->cursor(status->state, NULL, &state_22, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int id;
         double val;
         int id_expire;
         double val_expire;
         struct timeval atime;
      } resu_20_21;
      next_20:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = resu_20->c_get(resu_20, &key, &data, (first_entry_21)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_21 = 0;
         memcpy(&(resu_20_21.id), (char*)data.data+0, sizeof(int));
         //printf("Retrieved resu_20_21.id = %d\n", resu_20_21.id);
         //fflush(stdout);
         memcpy(&(resu_20_21.val), (char*)data.data+4, sizeof(double));
         //printf("Retrieved resu_20_21.val = %f\n", resu_20_21.val);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_21 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         struct {
            int max;
            int max_expire;
            struct timeval atime;
         } embed_23;
         int embed_23_cnt = 0;
         struct {
            int max;
            double sum;
            int max_expire;
            double sum_expire;
            struct timeval atime;
         } state_22_24;
         next_22:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_22->c_get(state_22, &key, &data, (first_entry_24)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_24 = 0;
            memcpy(&(state_22_24.max), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_22_24.max = %d\n", state_22_24.max);
            //fflush(stdout);
            memcpy(&(state_22_24.sum), (char*)data.data+4, sizeof(double));
            //printf("Retrieved state_22_24.sum = %f\n", state_22_24.sum);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_24 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         first_entry_22 = (rc)? 1:0;
         if (rc==0) {
            embed_23.max = state_22_24.max;
            if (embed_23_cnt++ ==0) goto next_22; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_23_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 25.\n");
            exit(1);
         }
         else if (embed_23_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 25.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((((resu_20_21.id) + 10000) == embed_23.max))) {
            goto next_20;
         }
         /*DELETE STARTS*/
         if ((rc = resu_20->c_del(resu_20, 0)) != 0) {
            adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
         }
         /*DELETE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (resu_20 && (rc = resu_20->c_close(resu_20)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (state_22 && (rc = state_22->c_close(state_22)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->state->cursor(status->state, NULL, &state_27, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      next_26:
      rc = (first_entry_26)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_26=1;
      else {
         first_entry_26=0;
         struct {
            int max;
            int max_expire;
            struct timeval atime;
         } embed_28;
         int embed_28_cnt = 0;
         struct {
            int max;
            double sum;
            int max_expire;
            double sum_expire;
            struct timeval atime;
         } state_27_29;
         next_27:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_27->c_get(state_27, &key, &data, (first_entry_29)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_29 = 0;
            memcpy(&(state_27_29.max), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_27_29.max = %d\n", state_27_29.max);
            //fflush(stdout);
            memcpy(&(state_27_29.sum), (char*)data.data+4, sizeof(double));
            //printf("Retrieved state_27_29.sum = %f\n", state_27_29.sum);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_29 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            embed_28.max = state_27_29.max;
         } /* if (rc == 0) */
         insert_25.field_0 = embed_28.max;
         insert_25.Next = Next;
         insert_25.field_2 = 0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_25.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_25.Next), sizeof(double));
         memcpy((char*)data.data+12, &(insert_25.field_2), sizeof(int));
         data.size = 16;
         key.size = 0;
         if ((rc = status->myWin->put(status->myWin, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         status->myWin->sync(status->myWin, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (state_27 && (rc = state_27->c_close(state_27)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->resu->cursor(status->resu, NULL, &resu_30, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->resu->cursor(status->resu, NULL, &resu_38, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->resu->cursor(status->resu, NULL, &resu_45, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->state->cursor(status->state, NULL, &state_46, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int id;
         double val;
         int id_expire;
         double val_expire;
         struct timeval atime;
      } resu_30_31;
      next_30:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = resu_30->c_get(resu_30, &key, &data, (first_entry_31)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_31 = 0;
         memcpy(&(resu_30_31.id), (char*)data.data+0, sizeof(int));
         //printf("Retrieved resu_30_31.id = %d\n", resu_30_31.id);
         //fflush(stdout);
         memcpy(&(resu_30_31.val), (char*)data.data+4, sizeof(double));
         //printf("Retrieved resu_30_31.val = %f\n", resu_30_31.val);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_31 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         tmpStore(&key, &data);
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_33;
         int embed_33_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0003_32_34;
         next_32:
         struct {
            double a_0;
            double a_0_expire;
            struct timeval atime;
         } Q_0002_36_35;
         next_36:
         while (index_36>=0 && index_36 < 2) {
            switch(index_36) {
               case 0:
               {
                  if (terminating_36 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int id;
                        double val;
                        int id_expire;
                        double val_expire;
                        struct timeval atime;
                     } resu_38_37;
                     next_38:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = resu_38->c_get(resu_38, &key, &data, (first_entry_37)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_37 = 0;
                        memcpy(&(resu_38_37.id), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved resu_38_37.id = %d\n", resu_38_37.id);
                        //fflush(stdout);
                        memcpy(&(resu_38_37.val), (char*)data.data+4, sizeof(double));
                        //printf("Retrieved resu_38_37.val = %f\n", resu_38_37.val);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_37 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        Q_0002_36_35.a_0 = resu_38_37.val;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_36 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_36 = (struct gb_status_36 *)0;
                        rc = hash_get(36, _rec_id, gbkey, 4, (char**)&gbstatus_36);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_36 = (struct gb_status_36*)malloc(sizeof(*gbstatus_36));
                           gbstatus_36->_baggr_0_first_entry = 1;
                           gbstatus_36->_baggr_0_value =  Q_0002_36_35.a_0;
                           rc = hash_put(36, _rec_id, gbkey, 4, &gbstatus_36);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_36->_baggr_0_first_entry = 1;
                           if (gbstatus_36->_baggr_0_value >  Q_0002_36_35.a_0) {
                              gbstatus_36->_baggr_0_value =  Q_0002_36_35.a_0;
                           }
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_36 = 1;
                     }
                  }
                  if (terminating_36 == 1) {
                     allkey = (char*)0;
                     rc = hash_get(36, _rec_id, allkey, 4, (char**)&gbstatus_36);
                     if (rc==0) {
                     } else if(rc == DB_NOTFOUND) {
                     } else adlabort(rc, "hash->get()");
                  }
               }
               break;
               case 1:
               {
                  rc = DB_NOTFOUND;
                  if (terminating_36 == 1) {
                     if (gbstatus_36->_baggr_0_first_entry == 1) {
                        Q_0003_32_34.a_0 = gbstatus_36->_baggr_0_value;
                        gbstatus_36->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_36->_baggr_0_first_entry = 1;
                     }
                  }
                  first_entry_36 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_36++;
            }
            if (rc == DB_NOTFOUND) {
               index_36--;
               if (terminating_36 == 1 && index_36 == 0) {
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_36--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_36 = 0;
            first_entry_36 = 1;
            index_36 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(36, _rec_id, allkey, 4, (char**)&gbstatus_36);
               if (rc==0) {
                  //printf("freeing 36\n");
                  free(gbstatus_36);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(36, _rec_id);
         }
         first_entry_32 = (rc)? 1:0;
         if (rc==0) {
            embed_33.a_0 = Q_0003_32_34.a_0;
            if (embed_33_cnt++ ==0) goto next_32; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_33_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 33.\n");
            exit(1);
         }
         else if (embed_33_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 33.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((Next > embed_33.a_0))) {
            goto next_30;
         }
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_40;
         int embed_40_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0005_39_41;
         next_39:
         struct {
            double a_0;
            double a_0_expire;
            struct timeval atime;
         } Q_0004_43_42;
         next_43:
         while (index_43>=0 && index_43 < 2) {
            switch(index_43) {
               case 0:
               {
                  if (terminating_43 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int id;
                        double val;
                        int id_expire;
                        double val_expire;
                        struct timeval atime;
                     } resu_45_44;
                     next_45:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = resu_45->c_get(resu_45, &key, &data, (first_entry_44)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_44 = 0;
                        memcpy(&(resu_45_44.id), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved resu_45_44.id = %d\n", resu_45_44.id);
                        //fflush(stdout);
                        memcpy(&(resu_45_44.val), (char*)data.data+4, sizeof(double));
                        //printf("Retrieved resu_45_44.val = %f\n", resu_45_44.val);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_44 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        Q_0004_43_42.a_0 = resu_45_44.val;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_43 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_43 = (struct gb_status_43 *)0;
                        rc = hash_get(43, _rec_id, gbkey, 4, (char**)&gbstatus_43);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_43 = (struct gb_status_43*)malloc(sizeof(*gbstatus_43));
                           gbstatus_43->_baggr_0_first_entry = 1;
                           gbstatus_43->_baggr_0_value =  Q_0004_43_42.a_0;
                           rc = hash_put(43, _rec_id, gbkey, 4, &gbstatus_43);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_43->_baggr_0_first_entry = 1;
                           if (gbstatus_43->_baggr_0_value >  Q_0004_43_42.a_0) {
                              gbstatus_43->_baggr_0_value =  Q_0004_43_42.a_0;
                           }
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_43 = 1;
                     }
                  }
                  if (terminating_43 == 1) {
                     allkey = (char*)0;
                     rc = hash_get(43, _rec_id, allkey, 4, (char**)&gbstatus_43);
                     if (rc==0) {
                     } else if(rc == DB_NOTFOUND) {
                     } else adlabort(rc, "hash->get()");
                  }
               }
               break;
               case 1:
               {
                  rc = DB_NOTFOUND;
                  if (terminating_43 == 1) {
                     if (gbstatus_43->_baggr_0_first_entry == 1) {
                        Q_0005_39_41.a_0 = gbstatus_43->_baggr_0_value;
                        gbstatus_43->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_43->_baggr_0_first_entry = 1;
                     }
                  }
                  first_entry_43 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_43++;
            }
            if (rc == DB_NOTFOUND) {
               index_43--;
               if (terminating_43 == 1 && index_43 == 0) {
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_43--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_43 = 0;
            first_entry_43 = 1;
            index_43 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(43, _rec_id, allkey, 4, (char**)&gbstatus_43);
               if (rc==0) {
                  //printf("freeing 43\n");
                  free(gbstatus_43);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(43, _rec_id);
         }
         first_entry_39 = (rc)? 1:0;
         if (rc==0) {
            embed_40.a_0 = Q_0005_39_41.a_0;
            if (embed_40_cnt++ ==0) goto next_39; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_40_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 34.\n");
            exit(1);
         }
         else if (embed_40_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 34.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((resu_30_31.val == embed_40.a_0))) {
            goto next_30;
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
         } embed_47;
         int embed_47_cnt = 0;
         struct {
            int max;
            double sum;
            int max_expire;
            double sum_expire;
            struct timeval atime;
         } state_46_48;
         next_46:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_46->c_get(state_46, &key, &data, (first_entry_48)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_48 = 0;
            memcpy(&(state_46_48.max), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_46_48.max = %d\n", state_46_48.max);
            //fflush(stdout);
            memcpy(&(state_46_48.sum), (char*)data.data+4, sizeof(double));
            //printf("Retrieved state_46_48.sum = %f\n", state_46_48.sum);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_48 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            embed_47.max = state_46_48.max;
         } /* if (rc == 0) */
         kdPop(&key, &data);
         /* SUBQUERY IN UPDATE ENDS */
         *(int*)((char*)data.data+0) = embed_47.max;
         *(double*)((char*)data.data+4) = Next;
         if ((rc = resu_30->c_put(resu_30, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (resu_30 && (rc = resu_30->c_close(resu_30)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (resu_38 && (rc = resu_38->c_close(resu_38)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (resu_45 && (rc = resu_45->c_close(resu_45)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (state_46 && (rc = state_46->c_close(state_46)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->myWin->cursor(status->myWin, NULL, &myWin_51, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->resu->cursor(status->resu, NULL, &resu_58, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->myWin->cursor(status->myWin, NULL, &myWin_65, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->resu->cursor(status->resu, NULL, &resu_66, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int id;
         double val;
         int cnt;
         int id_expire;
         double val_expire;
         int cnt_expire;
         struct timeval atime;
      } myWin_51_50;
      next_51:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = myWin_51->c_get(myWin_51, &key, &data, (first_entry_50)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_50 = 0;
         memcpy(&(myWin_51_50.id), (char*)data.data+0, sizeof(int));
         //printf("Retrieved myWin_51_50.id = %d\n", myWin_51_50.id);
         //fflush(stdout);
         memcpy(&(myWin_51_50.val), (char*)data.data+4, sizeof(double));
         //printf("Retrieved myWin_51_50.val = %f\n", myWin_51_50.val);
         //fflush(stdout);
         memcpy(&(myWin_51_50.cnt), (char*)data.data+12, sizeof(int));
         //printf("Retrieved myWin_51_50.cnt = %d\n", myWin_51_50.cnt);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_50 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_53;
         int embed_53_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0007_52_54;
         next_52:
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0006_56_55;
         next_56:
         while (index_56>=0 && index_56 < 2) {
            switch(index_56) {
               case 0:
               {
                  if (terminating_56 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int id;
                        double val;
                        int id_expire;
                        double val_expire;
                        struct timeval atime;
                     } resu_58_57;
                     next_58:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = resu_58->c_get(resu_58, &key, &data, (first_entry_57)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_57 = 0;
                        memcpy(&(resu_58_57.id), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved resu_58_57.id = %d\n", resu_58_57.id);
                        //fflush(stdout);
                        memcpy(&(resu_58_57.val), (char*)data.data+4, sizeof(double));
                        //printf("Retrieved resu_58_57.val = %f\n", resu_58_57.val);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_57 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        Q_0006_56_55.a_0 = resu_58_57.id;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_56 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_56 = (struct gb_status_56 *)0;
                        rc = hash_get(56, _rec_id, gbkey, 4, (char**)&gbstatus_56);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_56 = (struct gb_status_56*)malloc(sizeof(*gbstatus_56));
                           gbstatus_56->_baggr_0_first_entry = 1;
                           gbstatus_56->_baggr_0_value = 1;
                           rc = hash_put(56, _rec_id, gbkey, 4, &gbstatus_56);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_56->_baggr_0_first_entry = 1;
                           gbstatus_56->_baggr_0_value += 1;
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_56 = 1;
                     }
                  }
                  if (terminating_56 == 1) {
                     if (first_entry_56 == 1) {
                        rc = 0; /* fail on first entry, aggregate on empty set */
                     } else {
                        allkey = (char*)0;
                        rc = hash_get(56, _rec_id, allkey, 4, (char**)&gbstatus_56);
                        if (rc==0) {
                        } else if(rc == DB_NOTFOUND) {
                        } else adlabort(rc, "hash->get()");
                     }
                  }
               }
               break;
               case 1:
               {
                  rc = DB_NOTFOUND;
                  if (terminating_56 == 1) {
                     if (gbstatus_56 == (struct gb_status_56 *)0) {
                        if (first_entry_56) {
                           rc = 0;
                           Q_0007_52_54.a_0 = 0;
                        }
                     } else 
                     if (gbstatus_56->_baggr_0_first_entry == 1) {
                        Q_0007_52_54.a_0 = gbstatus_56->_baggr_0_value;
                        gbstatus_56->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_56->_baggr_0_first_entry = 1;
                     }
                  }
                  first_entry_56 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_56++;
            }
            if (rc == DB_NOTFOUND) {
               index_56--;
               if (terminating_56 == 1 && index_56 == 0) {
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_56--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_56 = 0;
            first_entry_56 = 1;
            index_56 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(56, _rec_id, allkey, 4, (char**)&gbstatus_56);
               if (rc==0) {
                  //printf("freeing 56\n");
                  free(gbstatus_56);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(56, _rec_id);
         }
         first_entry_52 = (rc)? 1:0;
         if (rc==0) {
            embed_53.a_0 = Q_0007_52_54.a_0;
            if (embed_53_cnt++ ==0) goto next_52; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_53_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 40.\n");
            exit(1);
         }
         else if (embed_53_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 40.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((embed_53.a_0 < 30))) {
            goto next_51;
         }
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_60;
         int embed_60_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0009_59_61;
         next_59:
         struct {
            double a_0;
            double a_0_expire;
            struct timeval atime;
         } Q_0008_63_62;
         next_63:
         while (index_63>=0 && index_63 < 2) {
            switch(index_63) {
               case 0:
               {
                  if (terminating_63 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int id;
                        double val;
                        int cnt;
                        int id_expire;
                        double val_expire;
                        int cnt_expire;
                        struct timeval atime;
                     } myWin_65_64;
                     next_65:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = myWin_65->c_get(myWin_65, &key, &data, (first_entry_64)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_64 = 0;
                        memcpy(&(myWin_65_64.id), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved myWin_65_64.id = %d\n", myWin_65_64.id);
                        //fflush(stdout);
                        memcpy(&(myWin_65_64.val), (char*)data.data+4, sizeof(double));
                        //printf("Retrieved myWin_65_64.val = %f\n", myWin_65_64.val);
                        //fflush(stdout);
                        memcpy(&(myWin_65_64.cnt), (char*)data.data+12, sizeof(int));
                        //printf("Retrieved myWin_65_64.cnt = %d\n", myWin_65_64.cnt);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_64 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        struct {
                           int exists;
                           int exists_expire;
                           struct timeval atime;
                        } embed_67;
                        int embed_67_cnt = 0;
                        struct {
                           int id;
                           double val;
                           int id_expire;
                           double val_expire;
                           struct timeval atime;
                        } resu_66_68;
                        next_66:
                        memset(&key, 0, sizeof(key));
                        memset(&data, 0, sizeof(data));
                        rc = resu_66->c_get(resu_66, &key, &data, (first_entry_68)? DB_FIRST:DB_NEXT);
                        if (rc==0) {
                           first_entry_68 = 0;
                           memcpy(&(resu_66_68.id), (char*)data.data+0, sizeof(int));
                           //printf("Retrieved resu_66_68.id = %d\n", resu_66_68.id);
                           //fflush(stdout);
                           memcpy(&(resu_66_68.val), (char*)data.data+4, sizeof(double));
                           //printf("Retrieved resu_66_68.val = %f\n", resu_66_68.val);
                           //fflush(stdout);
                        } else if (rc == DB_NOTFOUND) {
                           first_entry_68 = 1;
                        } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                        if (rc==0) {
                           rc = 0;          /* subquery could've overwritten rc */
                           if (!((resu_66_68.id == myWin_65_64.id))) {
                              goto next_66;
                           }
                        } /* if (rc == 0) */
                        embed_67.exists=(rc==DB_NOTFOUND);
                        first_entry_68 = 1;
                        rc = 0;          /* subquery could've overwritten rc */
                        if (!(embed_67.exists)) {
                           goto next_65;
                        }
                        Q_0008_63_62.a_0 = myWin_65_64.val;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_63 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_63 = (struct gb_status_63 *)0;
                        rc = hash_get(63, _rec_id, gbkey, 4, (char**)&gbstatus_63);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_63 = (struct gb_status_63*)malloc(sizeof(*gbstatus_63));
                           gbstatus_63->_baggr_0_first_entry = 1;
                           gbstatus_63->_baggr_0_value =  Q_0008_63_62.a_0;
                           rc = hash_put(63, _rec_id, gbkey, 4, &gbstatus_63);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_63->_baggr_0_first_entry = 1;
                           if (gbstatus_63->_baggr_0_value <  Q_0008_63_62.a_0) {
                              gbstatus_63->_baggr_0_value =  Q_0008_63_62.a_0;
                           }
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_63 = 1;
                     }
                  }
                  if (terminating_63 == 1) {
                     allkey = (char*)0;
                     rc = hash_get(63, _rec_id, allkey, 4, (char**)&gbstatus_63);
                     if (rc==0) {
                     } else if(rc == DB_NOTFOUND) {
                     } else adlabort(rc, "hash->get()");
                  }
               }
               break;
               case 1:
               {
                  rc = DB_NOTFOUND;
                  if (terminating_63 == 1) {
                     if (gbstatus_63->_baggr_0_first_entry == 1) {
                        Q_0009_59_61.a_0 = gbstatus_63->_baggr_0_value;
                        gbstatus_63->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_63->_baggr_0_first_entry = 1;
                     }
                  }
                  first_entry_63 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_63++;
            }
            if (rc == DB_NOTFOUND) {
               index_63--;
               if (terminating_63 == 1 && index_63 == 0) {
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_63--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_63 = 0;
            first_entry_63 = 1;
            index_63 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(63, _rec_id, allkey, 4, (char**)&gbstatus_63);
               if (rc==0) {
                  //printf("freeing 63\n");
                  free(gbstatus_63);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(63, _rec_id);
         }
         first_entry_59 = (rc)? 1:0;
         if (rc==0) {
            embed_60.a_0 = Q_0009_59_61.a_0;
            if (embed_60_cnt++ ==0) goto next_59; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_60_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 41.\n");
            exit(1);
         }
         else if (embed_60_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 41.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((myWin_51_50.val == embed_60.a_0))) {
            goto next_51;
         }
         insert_49.id = myWin_51_50.id;
         insert_49.val = myWin_51_50.val;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_49.id), sizeof(int));
         memcpy((char*)data.data+4, &(insert_49.val), sizeof(double));
         data.size = 12;
         key.size = 0;
         if ((rc = status->resu->put(status->resu, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         status->resu->sync(status->resu, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (myWin_51 && (rc = myWin_51->c_close(myWin_51)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (resu_58 && (rc = resu_58->c_close(resu_58)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (myWin_65 && (rc = myWin_65->c_close(myWin_65)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (resu_66 && (rc = resu_66->c_close(resu_66)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->myWin->cursor(status->myWin, NULL, &myWin_69, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   if ((rc = status->state->cursor(status->state, NULL, &state_71, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int id;
         double val;
         int cnt;
         int id_expire;
         double val_expire;
         int cnt_expire;
         struct timeval atime;
      } myWin_69_70;
      next_69:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = myWin_69->c_get(myWin_69, &key, &data, (first_entry_70)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_70 = 0;
         memcpy(&(myWin_69_70.id), (char*)data.data+0, sizeof(int));
         //printf("Retrieved myWin_69_70.id = %d\n", myWin_69_70.id);
         //fflush(stdout);
         memcpy(&(myWin_69_70.val), (char*)data.data+4, sizeof(double));
         //printf("Retrieved myWin_69_70.val = %f\n", myWin_69_70.val);
         //fflush(stdout);
         memcpy(&(myWin_69_70.cnt), (char*)data.data+12, sizeof(int));
         //printf("Retrieved myWin_69_70.cnt = %d\n", myWin_69_70.cnt);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_70 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         tmpStore(&key, &data);
         rc = 0;          /* subquery could've overwritten rc */
         if (!((myWin_69_70.val <= Next))) {
            goto next_69;
         }
         struct {
            int max;
            int max_expire;
            struct timeval atime;
         } embed_72;
         int embed_72_cnt = 0;
         struct {
            int max;
            double sum;
            int max_expire;
            double sum_expire;
            struct timeval atime;
         } state_71_73;
         next_71:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_71->c_get(state_71, &key, &data, (first_entry_73)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_73 = 0;
            memcpy(&(state_71_73.max), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_71_73.max = %d\n", state_71_73.max);
            //fflush(stdout);
            memcpy(&(state_71_73.sum), (char*)data.data+4, sizeof(double));
            //printf("Retrieved state_71_73.sum = %f\n", state_71_73.sum);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_73 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         first_entry_71 = (rc)? 1:0;
         if (rc==0) {
            embed_72.max = state_71_73.max;
            if (embed_72_cnt++ ==0) goto next_71; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_72_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 46.\n");
            exit(1);
         }
         else if (embed_72_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 46.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((myWin_69_70.id != embed_72.max))) {
            goto next_69;
         }
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         tmpRecover(&key, &data);
         *(int*)((char*)data.data+12) = ((myWin_69_70.cnt) + 1);
         if ((rc = myWin_69->c_put(myWin_69, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (myWin_69 && (rc = myWin_69->c_close(myWin_69)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (state_71 && (rc = state_71->c_close(state_71)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->myWin->cursor(status->myWin, NULL, &myWin_74, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int id;
         double val;
         int cnt;
         int id_expire;
         double val_expire;
         int cnt_expire;
         struct timeval atime;
      } myWin_74_75;
      next_74:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = myWin_74->c_get(myWin_74, &key, &data, (first_entry_75)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_75 = 0;
         memcpy(&(myWin_74_75.id), (char*)data.data+0, sizeof(int));
         //printf("Retrieved myWin_74_75.id = %d\n", myWin_74_75.id);
         //fflush(stdout);
         memcpy(&(myWin_74_75.val), (char*)data.data+4, sizeof(double));
         //printf("Retrieved myWin_74_75.val = %f\n", myWin_74_75.val);
         //fflush(stdout);
         memcpy(&(myWin_74_75.cnt), (char*)data.data+12, sizeof(int));
         //printf("Retrieved myWin_74_75.cnt = %d\n", myWin_74_75.cnt);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_75 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         rc = 0;          /* subquery could've overwritten rc */
         if (!((myWin_74_75.cnt >= 30))) {
            goto next_74;
         }
         /*DELETE STARTS*/
         if ((rc = myWin_74->c_del(myWin_74, 0)) != 0) {
            adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
         }
         /*DELETE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (myWin_74 && (rc = myWin_74->c_close(myWin_74)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->state->cursor(status->state, NULL, &state_78, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int max;
         double sum;
         int max_expire;
         double sum_expire;
         struct timeval atime;
      } state_78_77;
      next_78:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = state_78->c_get(state_78, &key, &data, (first_entry_77)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_77 = 0;
         memcpy(&(state_78_77.max), (char*)data.data+0, sizeof(int));
         //printf("Retrieved state_78_77.max = %d\n", state_78_77.max);
         //fflush(stdout);
         memcpy(&(state_78_77.sum), (char*)data.data+4, sizeof(double));
         //printf("Retrieved state_78_77.sum = %f\n", state_78_77.sum);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_77 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_76.field_0 = ((state_78_77.sum) / state_78_77.max);
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_76.field_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_76.field_0), sizeof(double));
         data.size = 8;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (state_78 && (rc = state_78->c_close(state_78)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void TopkNoOrder_window_expire(struct TopkNoOrder_window_status *status, 
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
   if(status && status->win)
   window = status->win->get_im_rel();
   struct inWinType_TopkNoOrder_window tuple;
   status->retc_first_entry=1;
}
/**** Query Declarations ****/
int _adl_statement_79()
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
int _adl_statement_87()
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
      double a_0;
      double a_0_expire;
      struct timeval atime;
   } insert_80;
   IM_RELC *traffic_86;
   int first_entry_85 = 1;
   int first_entry_86 = 1;
   int index_84 = 0;
   int terminating_84=0;
   struct gb_status_84 {
      struct TopkNoOrder_window_status *TopkNoOrder_window_0;
   };
   struct gb_status_84 *gbstatus_84 = (struct gb_status_84 *)0;
   
   int first_entry_84 = 1;
   int first_entry_82 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = traffic->cursor(traffic, &traffic_86, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double a_0;
         double a_0_expire;
         struct timeval atime;
      } Q_0011_82_81;
      next_82:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0010_84_83;
      next_84:
      while (index_84>=0 && index_84 < 2) {
         switch(index_84) {
            case 0:
            {
               if (terminating_84 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int in1;
                     struct timeval time1;
                     int OID;
                     int in1_expire;
                     struct timeval time1_expire;
                     int OID_expire;
                     struct timeval atime;
                  } traffic_86_85;
                  next_86:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = traffic_86->c_get(traffic_86, &key, &data, (first_entry_85)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_85 = 0;
                     memcpy(&(traffic_86_85.in1), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved traffic_86_85.in1 = %d\n", traffic_86_85.in1);
                     //fflush(stdout);
                     memcpy(&(traffic_86_85.time1), (char*)data.data+4, sizeof(struct timeval));
                     memcpy(&(traffic_86_85.OID), (char*)data.data+12, sizeof(int));
                     //printf("Retrieved traffic_86_85.OID = %d\n", traffic_86_85.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_85 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0010_84_83.a_0 = traffic_86_85.in1;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_84 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_84 = (struct gb_status_84 *)0;
                     rc = hash_get(84, _rec_id, gbkey, 4, (char**)&gbstatus_84);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_84 = (struct gb_status_84*)malloc(sizeof(*gbstatus_84));
                        gbstatus_84->TopkNoOrder_window_0 = (struct TopkNoOrder_window_status*)malloc(sizeof(struct TopkNoOrder_window_status));
                        gbstatus_84->TopkNoOrder_window_0->win = 0;
                        gbstatus_84->TopkNoOrder_window_0->win = new winbuf(17, 4, 4, _ADL_WIN_ROW);
                        gbstatus_84->TopkNoOrder_window_0->last_out = 0;
                        gbstatus_84->TopkNoOrder_window_0->iterate = false;
                        gbstatus_84->TopkNoOrder_window_0->init = true;
                        setModelId("");
                        memcpy(_databuf+0, &(Q_0010_84_83.a_0), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 4;
                        gbstatus_84->TopkNoOrder_window_0->win->updateTupleID();
                        gbstatus_84->TopkNoOrder_window_0->win->put(&windata);
                        if((!(1 <= 1 || ((gbstatus_84->TopkNoOrder_window_0->win->getTupleID() == 0 && gbstatus_84->TopkNoOrder_window_0->last_out != 0) 
                        ||((((int)gbstatus_84->TopkNoOrder_window_0->win->getTupleID()) >= gbstatus_84->TopkNoOrder_window_0->last_out + 1)))))) {
                           slide_out = 0;
                           //printf("Here no output %d 1 %d\n", gbstatus_84->TopkNoOrder_window_0->last_out, gbstatus_84->TopkNoOrder_window_0->win->getTupleID());fflush(stdout);
                        } else {
                           slide_out = 1;
                           //printf("Here YES output %d 1 %d\n", gbstatus_84->TopkNoOrder_window_0->last_out, gbstatus_84->TopkNoOrder_window_0->win->getTupleID());fflush(stdout);
                           gbstatus_84->TopkNoOrder_window_0->last_out = gbstatus_84->TopkNoOrder_window_0->last_out + 1;
                           while(gbstatus_84->TopkNoOrder_window_0->last_out < (gbstatus_84->TopkNoOrder_window_0->win->getTupleID() - 1) && gbstatus_84->TopkNoOrder_window_0->win->getTupleID() > 0) {
                              if(1 == 1) {
                                 gbstatus_84->TopkNoOrder_window_0->last_out = gbstatus_84->TopkNoOrder_window_0->win->getTupleID();
                              }
                              else {
                                 gbstatus_84->TopkNoOrder_window_0->last_out = gbstatus_84->TopkNoOrder_window_0->last_out + 1;
                              }
                           }
                        }
                        TopkNoOrder_window_init(gbstatus_84->TopkNoOrder_window_0, Q_0010_84_83.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(84, _rec_id, gbkey, 4, &gbstatus_84);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        memcpy(_databuf+0, &(Q_0010_84_83.a_0), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 4;
                        gbstatus_84->TopkNoOrder_window_0->win->updateTupleID();
                        gbstatus_84->TopkNoOrder_window_0->win->put(&windata);
                        if((!(1 <= 1 || ((gbstatus_84->TopkNoOrder_window_0->win->getTupleID() == 0 && gbstatus_84->TopkNoOrder_window_0->last_out != 0) 
                        ||((((int)gbstatus_84->TopkNoOrder_window_0->win->getTupleID()) >= gbstatus_84->TopkNoOrder_window_0->last_out + 1)))))) {
                           slide_out = 0;
                           //printf("Here no output %d 1 %d\n", gbstatus_84->TopkNoOrder_window_0->last_out, gbstatus_84->TopkNoOrder_window_0->win->getTupleID());fflush(stdout);
                        } else {
                           slide_out = 1;
                           //printf("Here YES output %d 1 %d\n", gbstatus_84->TopkNoOrder_window_0->last_out, gbstatus_84->TopkNoOrder_window_0->win->getTupleID());fflush(stdout);
                           gbstatus_84->TopkNoOrder_window_0->last_out = gbstatus_84->TopkNoOrder_window_0->last_out + 1;
                           while(gbstatus_84->TopkNoOrder_window_0->last_out < (gbstatus_84->TopkNoOrder_window_0->win->getTupleID() - 1) && gbstatus_84->TopkNoOrder_window_0->win->getTupleID() > 0) {
                              if(1 == 1) {
                                 gbstatus_84->TopkNoOrder_window_0->last_out = gbstatus_84->TopkNoOrder_window_0->win->getTupleID();
                              }
                              else {
                                 gbstatus_84->TopkNoOrder_window_0->last_out = gbstatus_84->TopkNoOrder_window_0->last_out + 1;
                              }
                           }
                        }
                        while (gbstatus_84->TopkNoOrder_window_0->win->hasExpired()){
                           gbstatus_84->TopkNoOrder_window_0->win->getExpired(&windata);
                           memcpy(&(Q_0010_84_83.a_0_expire), (char*)windata.data+0, sizeof(int));
                           TopkNoOrder_window_expire(gbstatus_84->TopkNoOrder_window_0, Q_0010_84_83.a_0_expire, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                           gbstatus_84->TopkNoOrder_window_0->win->pop();
                        }
                        TopkNoOrder_window_iterate(gbstatus_84->TopkNoOrder_window_0, Q_0010_84_83.a_0, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_84 = 1;
                  }
               }
               if (terminating_84 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(84, _rec_id, allkey, 4, (char**)&gbstatus_84);
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
               rc = gbstatus_84->TopkNoOrder_window_0->retc->c_get(gbstatus_84->TopkNoOrder_window_0->retc, &key, &data, (gbstatus_84->TopkNoOrder_window_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_84->TopkNoOrder_window_0->retc_first_entry = 0;
                  memcpy(&(Q_0011_82_81.a_0), (char*)data.data+0, sizeof(double));
                  //printf("Retrieved Q_0011_82_81.a_0 = %f\n", Q_0011_82_81.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_84->TopkNoOrder_window_0->retc->c_del(gbstatus_84->TopkNoOrder_window_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_84->TopkNoOrder_window_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_84 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_84++;
         }
         if (rc == DB_NOTFOUND) {
            index_84--;
            if (terminating_84 == 1 && index_84 == 0) {
               if (gbstatus_84->TopkNoOrder_window_0->retc && (rc = gbstatus_84->TopkNoOrder_window_0->retc->c_close(gbstatus_84->TopkNoOrder_window_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_84->TopkNoOrder_window_0);
               
               if (gbstatus_84->TopkNoOrder_window_0->ret && ((rc = gbstatus_84->TopkNoOrder_window_0->ret->close(gbstatus_84->TopkNoOrder_window_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_84->TopkNoOrder_window_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_84--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_84 = 0;
         first_entry_84 = 1;
         index_84 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(84, _rec_id, allkey, 4, (char**)&gbstatus_84);
            if (rc==0) {
               if(gbstatus_84->TopkNoOrder_window_0) {
                  if(gbstatus_84->TopkNoOrder_window_0->win) {
                     delete(gbstatus_84->TopkNoOrder_window_0->win);
                     gbstatus_84->TopkNoOrder_window_0->win = 0;
                  }
                  free(gbstatus_84->TopkNoOrder_window_0);
               }
               //printf("freeing 84\n");
               free(gbstatus_84);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(84, _rec_id);
      }
      if (rc==0) {
         insert_80.a_0 = Q_0011_82_81.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10f ", insert_80.a_0);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (traffic_86 && (rc = traffic_86->c_close(traffic_86)) != 0) {
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
   _adl_statement_79();
   _adl_statement_87();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = traffic->close(traffic, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
