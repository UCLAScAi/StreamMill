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
DB *temp;
int _temp_cmp(DB* dbp, const DBT *a, const DBT *b){
   	int ai, bi, ri, rs;
   	double ad, bd, rd;
   	struct timeval *at, *bt;return 0;
};
struct mcount_status {
   DB *tmp;
   int _tmp_cmp(DB* dbp, const DBT *a, const DBT *b){
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
extern "C" void mcount_init(struct mcount_status *status, 
	 int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void mcount_iterate(struct mcount_status *status, 
	 int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void mcount_init(struct mcount_status *status, 
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
      sprintf(_adl_dbname, "._%d_tmp", status);
      (void)unlink(_adl_dbname);
      if ((rc = db_create(&status->tmp, NULL, 0)) != 0) {
         adlabort(rc, "db_create()");
      }
      if ((rc = status->tmp->set_pagesize(status->tmp, 2048)) != 0) {
         adlabort(rc, "set_pagesize()");
      }
      if ((rc = status->tmp->set_flags(status->tmp, DB_DUP)) != 0) {
         adlabort(rc, "set_flags()");
      }
      if ((rc = status->tmp->open(status->tmp, _adl_dbname, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
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
   struct {
      int field_0;
      int field_0_expire;
      struct timeval atime;
   } insert_2;
   int first_entry_3 = 1;
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
         if ((rc = status->tmp->put(status->tmp, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         status->tmp->sync(status->tmp, 0);
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
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_2.field_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_2.field_0), sizeof(int));
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
extern "C" void mcount_iterate(struct mcount_status *status, 
	 int _rec_id, bufferMngr* bm, 
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
   DBC *tmp_4;
   int first_entry_5 = 1;
   int first_entry_4 = 1;
   struct {
      int i;
      int i_expire;
      struct timeval atime;
   } insert_6;
   DBC *tmp_8;
   int first_entry_7 = 1;
   int first_entry_8 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->tmp->cursor(status->tmp, NULL, &tmp_4, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int i;
         int i_expire;
         struct timeval atime;
      } tmp_4_5;
      next_4:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = tmp_4->c_get(tmp_4, &key, &data, (first_entry_5)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_5 = 0;
         memcpy(&(tmp_4_5.i), (char*)data.data+0, sizeof(int));
         //printf("Retrieved tmp_4_5.i = %d\n", tmp_4_5.i);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_5 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         *(int*)((char*)data.data+0) = ((tmp_4_5.i) + 1);
         if ((rc = tmp_4->c_put(tmp_4, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (tmp_4 && (rc = tmp_4->c_close(tmp_4)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->tmp->cursor(status->tmp, NULL, &tmp_8, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int i;
         int i_expire;
         struct timeval atime;
      } tmp_8_7;
      next_8:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = tmp_8->c_get(tmp_8, &key, &data, (first_entry_7)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_7 = 0;
         memcpy(&(tmp_8_7.i), (char*)data.data+0, sizeof(int));
         //printf("Retrieved tmp_8_7.i = %d\n", tmp_8_7.i);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_7 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_6.i = tmp_8_7.i;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_6.i), sizeof(int));
         memcpy((char*)data.data+0, &(insert_6.i), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (tmp_8 && (rc = tmp_8->c_close(tmp_8)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
struct inWinType_mcount_window {
};
struct mcount_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_mcount_window* getLastTuple_mcount_window(IM_REL* inwindow, inWinType_mcount_window* tuple, bufferMngr* bm) {
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
   return tuple;
}
extern "C" void mcount_window_init(struct mcount_window_status *status, 
	 int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void mcount_window_init(struct mcount_window_status *status,  
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
   struct inWinType_mcount_window tuple;
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
      int a_0;
      int a_0_expire;
      struct timeval atime;
   } insert_9;
   IM_RELC *w_15;
   int first_entry_14 = 1;
   int first_entry_15 = 1;
   int index_13 = 0;
   int terminating_13=0;
   struct gb_status_13 {
      struct mcount_status *mcount_0;
   };
   struct gb_status_13 *gbstatus_13 = (struct gb_status_13 *)0;
   
   int first_entry_13 = 1;
   int first_entry_11 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_15, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0001_11_10;
      next_11:
      struct {
         int field_0;
         int field_0_expire;
         struct timeval atime;
      } Q_0000_13_12;
      next_13:
      while (index_13>=0 && index_13 < 2) {
         switch(index_13) {
            case 0:
            {
               if (terminating_13 == 0) {
                  /* get source tuple from qun */
                  struct {
                     struct timeval atime;
                  } w_15_14;
                  next_15:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_15->c_get(w_15, &key, &data, (first_entry_14)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_14 = 0;
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_14 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0000_13_12.field_0 = 1;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_13 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_13 = (struct gb_status_13 *)0;
                     rc = hash_get(13, _rec_id, gbkey, 4, (char**)&gbstatus_13);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_13 = (struct gb_status_13*)malloc(sizeof(*gbstatus_13));
                        gbstatus_13->mcount_0 = (struct mcount_status*)malloc(sizeof(struct mcount_status));
                        gbstatus_13->mcount_0->win = 0;
                        setModelId("");
                        mcount_init(gbstatus_13->mcount_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(13, _rec_id, gbkey, 4, &gbstatus_13);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        mcount_iterate(gbstatus_13->mcount_0, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_13 = 1;
                  }
               }
               if (terminating_13 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(13, _rec_id, allkey, 4, (char**)&gbstatus_13);
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
               rc = gbstatus_13->mcount_0->retc->c_get(gbstatus_13->mcount_0->retc, &key, &data, (gbstatus_13->mcount_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_13->mcount_0->retc_first_entry = 0;
                  memcpy(&(Q_0001_11_10.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0001_11_10.a_0 = %d\n", Q_0001_11_10.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_13->mcount_0->retc->c_del(gbstatus_13->mcount_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_13->mcount_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_13 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_13++;
         }
         if (rc == DB_NOTFOUND) {
            index_13--;
            if (terminating_13 == 1 && index_13 == 0) {
               if (gbstatus_13->mcount_0->retc && (rc = gbstatus_13->mcount_0->retc->c_close(gbstatus_13->mcount_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_13->mcount_0);
               
               if (gbstatus_13->mcount_0->ret && ((rc = gbstatus_13->mcount_0->ret->close(gbstatus_13->mcount_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_13->mcount_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_13--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_13 = 0;
         first_entry_13 = 1;
         index_13 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(13, _rec_id, allkey, 4, (char**)&gbstatus_13);
            if (rc==0) {
               free(gbstatus_13->mcount_0);
               //printf("freeing 13\n");
               free(gbstatus_13);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(13, _rec_id);
      }
      if (rc==0) {
         insert_9.a_0 = Q_0001_11_10.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_9.a_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_9.a_0), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_15 && (rc = w_15->c_close(w_15)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
/**** Query Declarations ****/
int _adl_statement_28()
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
      int field_0_expire;
      struct timeval atime;
   } insert_16;
   int index_17 = 0;
   int first_entry_19 = 1;
   int first_entry_21 = 1;
   int first_entry_23 = 1;
   int first_entry_25 = 1;
   int first_entry_27 = 1;
   int first_entry_17 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      if (first_entry_17==1) index_17 = 0;
      do {
         switch (index_17) {
            case 0:
            {
               struct {
                  int field_0;
                  int field_0_expire;
                  struct timeval atime;
               } unionqun_18;
               next_19:
               rc = (first_entry_19)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_19=1;
               else {
                  first_entry_19=0;
                  unionqun_18.field_0 = 5;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_16.field_0 = unionqun_18.field_0;
               }
            }
            break;
            case 1:
            {
               struct {
                  int field_0;
                  int field_0_expire;
                  struct timeval atime;
               } unionqun_20;
               next_21:
               rc = (first_entry_21)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_21=1;
               else {
                  first_entry_21=0;
                  unionqun_20.field_0 = 4;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_16.field_0 = unionqun_20.field_0;
               }
            }
            break;
            case 2:
            {
               struct {
                  int field_0;
                  int field_0_expire;
                  struct timeval atime;
               } unionqun_22;
               next_23:
               rc = (first_entry_23)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_23=1;
               else {
                  first_entry_23=0;
                  unionqun_22.field_0 = 3;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_16.field_0 = unionqun_22.field_0;
               }
            }
            break;
            case 3:
            {
               struct {
                  int field_0;
                  int field_0_expire;
                  struct timeval atime;
               } unionqun_24;
               next_25:
               rc = (first_entry_25)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_25=1;
               else {
                  first_entry_25=0;
                  unionqun_24.field_0 = 2;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_16.field_0 = unionqun_24.field_0;
               }
            }
            break;
            case 4:
            {
               struct {
                  int field_0;
                  int field_0_expire;
                  struct timeval atime;
               } unionqun_26;
               next_27:
               rc = (first_entry_27)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_27=1;
               else {
                  first_entry_27=0;
                  unionqun_26.field_0 = 1;
               } /* if (rc == 0) */
               if (rc==0) {
                  insert_16.field_0 = unionqun_26.field_0;
               }
            }
            break;
         }/* end of switch */
         if (rc == DB_NOTFOUND) index_17++;
      } while (rc == DB_NOTFOUND && index_17 < 5);
      next_17:
      if (rc == DB_NOTFOUND) {
         first_entry_17 = 1;
      }
      else {
         first_entry_17 = 0;
      }
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_16.field_0), sizeof(int));
         data.size = 4;
         key.size = 0;
         if ((rc = temp->put(temp, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         temp->sync(temp, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_36()
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
   } insert_29;
   DBC *temp_35;
   int first_entry_34 = 1;
   int first_entry_35 = 1;
   int index_33 = 0;
   int terminating_33=0;
   struct gb_status_33 {
      struct mcount_status *mcount_0;
   };
   struct gb_status_33 *gbstatus_33 = (struct gb_status_33 *)0;
   
   int first_entry_33 = 1;
   int first_entry_31 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = temp->cursor(temp, NULL, &temp_35, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0003_31_30;
      next_31:
      struct {
         int field_0;
         int field_0_expire;
         struct timeval atime;
      } Q_0002_33_32;
      next_33:
      while (index_33>=0 && index_33 < 2) {
         switch(index_33) {
            case 0:
            {
               if (terminating_33 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int a;
                     int a_expire;
                     struct timeval atime;
                  } temp_35_34;
                  next_35:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = temp_35->c_get(temp_35, &key, &data, (first_entry_34)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_34 = 0;
                     memcpy(&(temp_35_34.a), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved temp_35_34.a = %d\n", temp_35_34.a);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_34 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0002_33_32.field_0 = 1;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_33 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_33 = (struct gb_status_33 *)0;
                     rc = hash_get(33, _rec_id, gbkey, 4, (char**)&gbstatus_33);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_33 = (struct gb_status_33*)malloc(sizeof(*gbstatus_33));
                        gbstatus_33->mcount_0 = (struct mcount_status*)malloc(sizeof(struct mcount_status));
                        gbstatus_33->mcount_0->win = 0;
                        setModelId("");
                        mcount_init(gbstatus_33->mcount_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(33, _rec_id, gbkey, 4, &gbstatus_33);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        mcount_iterate(gbstatus_33->mcount_0, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_33 = 1;
                  }
               }
               if (terminating_33 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(33, _rec_id, allkey, 4, (char**)&gbstatus_33);
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
               rc = gbstatus_33->mcount_0->retc->c_get(gbstatus_33->mcount_0->retc, &key, &data, (gbstatus_33->mcount_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_33->mcount_0->retc_first_entry = 0;
                  memcpy(&(Q_0003_31_30.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0003_31_30.a_0 = %d\n", Q_0003_31_30.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_33->mcount_0->retc->c_del(gbstatus_33->mcount_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_33->mcount_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_33 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_33++;
         }
         if (rc == DB_NOTFOUND) {
            index_33--;
            if (terminating_33 == 1 && index_33 == 0) {
               if (gbstatus_33->mcount_0->retc && (rc = gbstatus_33->mcount_0->retc->c_close(gbstatus_33->mcount_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_33->mcount_0);
               
               if (gbstatus_33->mcount_0->ret && ((rc = gbstatus_33->mcount_0->ret->close(gbstatus_33->mcount_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_33->mcount_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_33--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_33 = 0;
         first_entry_33 = 1;
         index_33 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(33, _rec_id, allkey, 4, (char**)&gbstatus_33);
            if (rc==0) {
               free(gbstatus_33->mcount_0);
               //printf("freeing 33\n");
               free(gbstatus_33);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(33, _rec_id);
      }
      if (rc==0) {
         insert_29.a_0 = Q_0003_31_30.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_29.a_0);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (temp_35 && (rc = temp_35->c_close(temp_35)) != 0) {
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
   (void)unlink("_adl_db_temp");
   if ((rc = db_create(&temp, NULL, 0)) != 0) {
      adlabort(rc, "db_create()");
   }
   if ((rc = temp->set_pagesize(temp, 2048)) != 0) {
      adlabort(rc, "set_pagesize()");
   }
   if ((rc = temp->set_flags(temp, DB_DUP)) != 0) {
      adlabort(rc, "set_flags()");
   }
   if ((rc=temp->set_bt_compare(temp, _temp_cmp)) != 0){
      adlabort(rc, "IM_REL->put()");
   }
   if ((rc = temp->open(temp, "_adl_db_temp", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_statement_28();
   _adl_statement_36();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if (temp && ((rc = temp->close(temp, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   temp = NULL;
   (void)unlink("_adl_db_temp");
   return(rc);
};
