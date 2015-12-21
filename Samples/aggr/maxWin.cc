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
struct inWinType_myMax_online_window {
   int Next;
};
struct myMax_online_window_status {
   DB *inwindow;
   int _inwindow_cmp(DB* dbp, const DBT *a, const DBT *b){
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
inWinType_myMax_online_window* getLastTuple_myMax_online_window(IM_REL* inwindow, inWinType_myMax_online_window* tuple, bufferMngr* bm) {
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
extern "C" void myMax_online_window_init(struct myMax_online_window_status *status, 
	int Next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void myMax_online_window_iterate(struct myMax_online_window_status *status, 
	int Next, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void myMax_online_window_terminate(struct myMax_online_window_status *status, 
	int Next, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL);
extern "C" void myMax_online_window_init(struct myMax_online_window_status *status, int Next, 
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
   struct inWinType_myMax_online_window tuple;
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
   status->retc_first_entry=1;
}
extern "C" void myMax_online_window_iterate(struct myMax_online_window_status *status, 
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
   struct inWinType_myMax_online_window tuple;
   struct {
      double a_0;
      double a_0_expire;
      struct timeval atime;
   } insert_0;
   IM_RELC *inwindow_6;
   int first_entry_5 = 1;
   int first_entry_6 = 1;
   int index_4 = 0;
   int terminating_4=0;
   struct gb_status_4 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int max_0_last_out;
      bool max_0_iterate;
      bool max_0_init;
   };
   struct gb_status_4 *gbstatus_4 = (struct gb_status_4 *)0;
   
   int first_entry_4 = 1;
   int first_entry_2 = 1;
   struct {
      double field_0;
      double field_0_expire;
      struct timeval atime;
   } insert_7;
   int first_entry_8 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &inwindow_6, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0001_2_1;
      next_2:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0000_4_3;
      next_4:
      while (index_4>=0 && index_4 < 2) {
         switch(index_4) {
            case 0:
            {
               if (terminating_4 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int Next;
                     int Next_expire;
                     struct timeval atime;
                  } inwindow_6_5;
                  next_6:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = inwindow_6->c_get(inwindow_6, &key, &data, (first_entry_5)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_5 = 0;
                     memcpy(&(inwindow_6_5.Next), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved inwindow_6_5.Next = %d\n", inwindow_6_5.Next);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_5 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0000_4_3.a_0 = Next;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_4 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_4 = (struct gb_status_4 *)0;
                     rc = hash_get(4, _rec_id, gbkey, 4, (char**)&gbstatus_4);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_4 = (struct gb_status_4*)malloc(sizeof(*gbstatus_4));
                        gbstatus_4->_baggr_0_first_entry = 1;
                        gbstatus_4->_baggr_0_value =  Q_0000_4_3.a_0;
                        rc = hash_put(4, _rec_id, gbkey, 4, &gbstatus_4);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        gbstatus_4->_baggr_0_first_entry = 1;
                        if (gbstatus_4->_baggr_0_value <  Q_0000_4_3.a_0) {
                           gbstatus_4->_baggr_0_value =  Q_0000_4_3.a_0;
                        }
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_4 = 1;
                  }
               }
               if (terminating_4 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(4, _rec_id, allkey, 4, (char**)&gbstatus_4);
                  if (rc==0) {
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               rc = DB_NOTFOUND;
               if (terminating_4 == 1) {
                  if (gbstatus_4->_baggr_0_first_entry == 1) {
                     Q_0001_2_1.a_0 = gbstatus_4->_baggr_0_value;
                     gbstatus_4->_baggr_0_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_4->_baggr_0_first_entry = 1;
                  }
               }
               first_entry_4 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_4++;
         }
         if (rc == DB_NOTFOUND) {
            index_4--;
            if (terminating_4 == 1 && index_4 == 0) {
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_4--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_4 = 0;
         first_entry_4 = 1;
         index_4 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(4, _rec_id, allkey, 4, (char**)&gbstatus_4);
            if (rc==0) {
               //printf("freeing 4\n");
               free(gbstatus_4);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(4, _rec_id);
      }
      if (rc==0) {
         insert_0.a_0 = Q_0001_2_1.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_0.a_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_0.a_0), sizeof(double));
         data.size = 8;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (inwindow_6 && (rc = inwindow_6->c_close(inwindow_6)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_8:
      rc = (first_entry_8)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_8=1;
      else {
         first_entry_8=0;
         insert_7.field_0 = 4444444;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_7.field_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_7.field_0), sizeof(double));
         data.size = 8;
         key.size = 8;
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
extern "C" void myMax_online_window_terminate(struct myMax_online_window_status *status, 
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
   IM_REL *window;
   int slide_out = 1;
   winbuf* rwinbuf = NULL;
   int rlast_out = 0;
   if(status && status->win)
   window = status->win->get_im_rel();
   struct inWinType_myMax_online_window tuple;
   struct {
      double field_0;
      double field_0_expire;
      struct timeval atime;
   } insert_9;
   int first_entry_10 = 1;
   struct {
      double a_0;
      double a_0_expire;
      struct timeval atime;
   } insert_11;
   IM_RELC *inwindow_17;
   int first_entry_16 = 1;
   int first_entry_17 = 1;
   int index_15 = 0;
   int terminating_15=0;
   struct gb_status_15 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int max_0_last_out;
      bool max_0_iterate;
      bool max_0_init;
   };
   struct gb_status_15 *gbstatus_15 = (struct gb_status_15 *)0;
   
   int first_entry_15 = 1;
   int first_entry_13 = 1;
   struct {
      double field_0;
      double field_0_expire;
      struct timeval atime;
   } insert_18;
   int first_entry_19 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_10:
      rc = (first_entry_10)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_10=1;
      else {
         first_entry_10=0;
         insert_9.field_0 = 5555555;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_9.field_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_9.field_0), sizeof(double));
         data.size = 8;
         key.size = 8;
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
   if ((rc = window->cursor(window, &inwindow_17, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0003_13_12;
      next_13:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0002_15_14;
      next_15:
      while (index_15>=0 && index_15 < 2) {
         switch(index_15) {
            case 0:
            {
               if (terminating_15 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int Next;
                     int Next_expire;
                     struct timeval atime;
                  } inwindow_17_16;
                  next_17:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = inwindow_17->c_get(inwindow_17, &key, &data, (first_entry_16)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_16 = 0;
                     memcpy(&(inwindow_17_16.Next), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved inwindow_17_16.Next = %d\n", inwindow_17_16.Next);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_16 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0002_15_14.a_0 = Next;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_15 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_15 = (struct gb_status_15 *)0;
                     rc = hash_get(15, _rec_id, gbkey, 4, (char**)&gbstatus_15);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_15 = (struct gb_status_15*)malloc(sizeof(*gbstatus_15));
                        gbstatus_15->_baggr_0_first_entry = 1;
                        gbstatus_15->_baggr_0_value =  Q_0002_15_14.a_0;
                        rc = hash_put(15, _rec_id, gbkey, 4, &gbstatus_15);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        gbstatus_15->_baggr_0_first_entry = 1;
                        if (gbstatus_15->_baggr_0_value <  Q_0002_15_14.a_0) {
                           gbstatus_15->_baggr_0_value =  Q_0002_15_14.a_0;
                        }
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_15 = 1;
                  }
               }
               if (terminating_15 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(15, _rec_id, allkey, 4, (char**)&gbstatus_15);
                  if (rc==0) {
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               rc = DB_NOTFOUND;
               if (terminating_15 == 1) {
                  if (gbstatus_15->_baggr_0_first_entry == 1) {
                     Q_0003_13_12.a_0 = gbstatus_15->_baggr_0_value;
                     gbstatus_15->_baggr_0_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_15->_baggr_0_first_entry = 1;
                  }
               }
               first_entry_15 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_15++;
         }
         if (rc == DB_NOTFOUND) {
            index_15--;
            if (terminating_15 == 1 && index_15 == 0) {
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_15--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_15 = 0;
         first_entry_15 = 1;
         index_15 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(15, _rec_id, allkey, 4, (char**)&gbstatus_15);
            if (rc==0) {
               //printf("freeing 15\n");
               free(gbstatus_15);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(15, _rec_id);
      }
      if (rc==0) {
         insert_11.a_0 = Q_0003_13_12.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_11.a_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_11.a_0), sizeof(double));
         data.size = 8;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (inwindow_17 && (rc = inwindow_17->c_close(inwindow_17)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_19:
      rc = (first_entry_19)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_19=1;
      else {
         first_entry_19=0;
         insert_18.field_0 = 6666666;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_18.field_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_18.field_0), sizeof(double));
         data.size = 8;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   sprintf(_adl_dbname, "._%d_inwindow", status);
   if(!not_delete) {
      if (status->inwindow && ((rc = status->inwindow->close(status->inwindow, 0)) != 0)) {
         adlabort(rc, "DB->close()");
      }
      status->inwindow = NULL;
   }
   if(!not_delete) status->retc_first_entry=1;
}
/**** Query Declarations ****/
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
      double a_0;
      double a_0_expire;
      struct timeval atime;
   } insert_21;
   IM_RELC *traffic_27;
   int first_entry_26 = 1;
   int first_entry_27 = 1;
   int index_25 = 0;
   int terminating_25=0;
   struct gb_status_25 {
      struct myMax_online_window_status *myMax_online_window_0;
   };
   struct gb_status_25 *gbstatus_25 = (struct gb_status_25 *)0;
   
   int first_entry_25 = 1;
   int first_entry_23 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = traffic->cursor(traffic, &traffic_27, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double a_0;
         double a_0_expire;
         struct timeval atime;
      } Q_0005_23_22;
      next_23:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0004_25_24;
      next_25:
      while (index_25>=0 && index_25 < 2) {
         switch(index_25) {
            case 0:
            {
               if (terminating_25 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int in1;
                     struct timeval time1;
                     int OID;
                     int in1_expire;
                     struct timeval time1_expire;
                     int OID_expire;
                     struct timeval atime;
                  } traffic_27_26;
                  next_27:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = traffic_27->c_get(traffic_27, &key, &data, (first_entry_26)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_26 = 0;
                     memcpy(&(traffic_27_26.in1), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved traffic_27_26.in1 = %d\n", traffic_27_26.in1);
                     //fflush(stdout);
                     memcpy(&(traffic_27_26.time1), (char*)data.data+4, sizeof(struct timeval));
                     memcpy(&(traffic_27_26.OID), (char*)data.data+12, sizeof(int));
                     //printf("Retrieved traffic_27_26.OID = %d\n", traffic_27_26.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_26 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0004_25_24.a_0 = traffic_27_26.in1;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_25 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     memcpy(gbkey+0, &traffic_27_26.time1, 8);
                     gbstatus_25 = (struct gb_status_25 *)0;
                     rc = hash_get(25, _rec_id, gbkey, 8, (char**)&gbstatus_25);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_25 = (struct gb_status_25*)malloc(sizeof(*gbstatus_25));
                        gbstatus_25->myMax_online_window_0 = (struct myMax_online_window_status*)malloc(sizeof(struct myMax_online_window_status));
                        gbstatus_25->myMax_online_window_0->win = 0;
                        gbstatus_25->myMax_online_window_0->win = new winbuf(20, 4, 8, _ADL_WIN_ROW);
                        gbstatus_25->myMax_online_window_0->last_out = 0;
                        gbstatus_25->myMax_online_window_0->iterate = false;
                        gbstatus_25->myMax_online_window_0->init = true;
                        setModelId("");
                        memcpy(_databuf+0, &(Q_0004_25_24.a_0), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 4;
                        gbstatus_25->myMax_online_window_0->win->updateTupleID();
                        gbstatus_25->myMax_online_window_0->win->put(&windata);
                        if((!(10 <= 1 || ((gbstatus_25->myMax_online_window_0->win->getTupleID() == 0 && gbstatus_25->myMax_online_window_0->last_out != 0) 
                        ||((((int)gbstatus_25->myMax_online_window_0->win->getTupleID()) >= gbstatus_25->myMax_online_window_0->last_out + 10)))))) {
                           slide_out = 0;
                           //printf("Here no output %d 10 %d\n", gbstatus_25->myMax_online_window_0->last_out, gbstatus_25->myMax_online_window_0->win->getTupleID());fflush(stdout);
                        } else {
                           slide_out = 1;
                           //printf("Here YES output %d 10 %d\n", gbstatus_25->myMax_online_window_0->last_out, gbstatus_25->myMax_online_window_0->win->getTupleID());fflush(stdout);
                           gbstatus_25->myMax_online_window_0->last_out = gbstatus_25->myMax_online_window_0->last_out + 10;
                           while(gbstatus_25->myMax_online_window_0->last_out < (gbstatus_25->myMax_online_window_0->win->getTupleID() - 10) && gbstatus_25->myMax_online_window_0->win->getTupleID() > 0) {
                              if(10 == 1) {
                                 gbstatus_25->myMax_online_window_0->last_out = gbstatus_25->myMax_online_window_0->win->getTupleID();
                              }
                              else {
                                 gbstatus_25->myMax_online_window_0->last_out = gbstatus_25->myMax_online_window_0->last_out + 10;
                              }
                           }
                        }
                        myMax_online_window_init(gbstatus_25->myMax_online_window_0, Q_0004_25_24.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(25, _rec_id, gbkey, 8, &gbstatus_25);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        memcpy(_databuf+0, &(Q_0004_25_24.a_0), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 4;
                        gbstatus_25->myMax_online_window_0->win->updateTupleID();
                        gbstatus_25->myMax_online_window_0->win->put(&windata);
                        if((!(10 <= 1 || ((gbstatus_25->myMax_online_window_0->win->getTupleID() == 0 && gbstatus_25->myMax_online_window_0->last_out != 0) 
                        ||((((int)gbstatus_25->myMax_online_window_0->win->getTupleID()) >= gbstatus_25->myMax_online_window_0->last_out + 10)))))) {
                           slide_out = 0;
                           //printf("Here no output %d 10 %d\n", gbstatus_25->myMax_online_window_0->last_out, gbstatus_25->myMax_online_window_0->win->getTupleID());fflush(stdout);
                        } else {
                           slide_out = 1;
                           //printf("Here YES output %d 10 %d\n", gbstatus_25->myMax_online_window_0->last_out, gbstatus_25->myMax_online_window_0->win->getTupleID());fflush(stdout);
                           gbstatus_25->myMax_online_window_0->last_out = gbstatus_25->myMax_online_window_0->last_out + 10;
                           while(gbstatus_25->myMax_online_window_0->last_out < (gbstatus_25->myMax_online_window_0->win->getTupleID() - 10) && gbstatus_25->myMax_online_window_0->win->getTupleID() > 0) {
                              if(10 == 1) {
                                 gbstatus_25->myMax_online_window_0->last_out = gbstatus_25->myMax_online_window_0->win->getTupleID();
                              }
                              else {
                                 gbstatus_25->myMax_online_window_0->last_out = gbstatus_25->myMax_online_window_0->last_out + 10;
                              }
                           }
                        }
                        while (gbstatus_25->myMax_online_window_0->win->hasExpired()){
                           gbstatus_25->myMax_online_window_0->win->getExpired(&windata);
                           gbstatus_25->myMax_online_window_0->win->pop();
                        }
                        myMax_online_window_iterate(gbstatus_25->myMax_online_window_0, Q_0004_25_24.a_0, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_25 = 1;
                  }
               }
               if (terminating_25 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(25, _rec_id, allkey, 8, (char**)&gbstatus_25);
                  if (rc==0) {
                     setModelId("");
                     memcpy(_databuf+0, &(Q_0004_25_24.a_0), sizeof(int));
                     windata.data = _databuf;
                     windata.size = 4;
                     gbstatus_25->myMax_online_window_0->win->updateTupleID();
                     gbstatus_25->myMax_online_window_0->win->put(&windata);
                     if((!(10 <= 1 || ((gbstatus_25->myMax_online_window_0->win->getTupleID() == 0 && gbstatus_25->myMax_online_window_0->last_out != 0) 
                     ||((((int)gbstatus_25->myMax_online_window_0->win->getTupleID()) >= gbstatus_25->myMax_online_window_0->last_out + 10)))))) {
                        slide_out = 0;
                        //printf("Here no output %d 10 %d\n", gbstatus_25->myMax_online_window_0->last_out, gbstatus_25->myMax_online_window_0->win->getTupleID());fflush(stdout);
                     } else {
                        slide_out = 1;
                        //printf("Here YES output %d 10 %d\n", gbstatus_25->myMax_online_window_0->last_out, gbstatus_25->myMax_online_window_0->win->getTupleID());fflush(stdout);
                        gbstatus_25->myMax_online_window_0->last_out = gbstatus_25->myMax_online_window_0->last_out + 10;
                        while(gbstatus_25->myMax_online_window_0->last_out < (gbstatus_25->myMax_online_window_0->win->getTupleID() - 10) && gbstatus_25->myMax_online_window_0->win->getTupleID() > 0) {
                           if(10 == 1) {
                              gbstatus_25->myMax_online_window_0->last_out = gbstatus_25->myMax_online_window_0->win->getTupleID();
                           }
                           else {
                              gbstatus_25->myMax_online_window_0->last_out = gbstatus_25->myMax_online_window_0->last_out + 10;
                           }
                        }
                     }
                     myMax_online_window_terminate(gbstatus_25->myMax_online_window_0, Q_0004_25_24.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_25->myMax_online_window_0->retc->c_get(gbstatus_25->myMax_online_window_0->retc, &key, &data, (gbstatus_25->myMax_online_window_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_25->myMax_online_window_0->retc_first_entry = 0;
                  memcpy(&(Q_0005_23_22.a_0), (char*)data.data+0, sizeof(double));
                  //printf("Retrieved Q_0005_23_22.a_0 = %f\n", Q_0005_23_22.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_25->myMax_online_window_0->retc->c_del(gbstatus_25->myMax_online_window_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_25->myMax_online_window_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_25 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_25++;
         }
         if (rc == DB_NOTFOUND) {
            index_25--;
            if (terminating_25 == 1 && index_25 == 0) {
               if (gbstatus_25->myMax_online_window_0->retc && (rc = gbstatus_25->myMax_online_window_0->retc->c_close(gbstatus_25->myMax_online_window_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_25->myMax_online_window_0);
               
               if (gbstatus_25->myMax_online_window_0->ret && ((rc = gbstatus_25->myMax_online_window_0->ret->close(gbstatus_25->myMax_online_window_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_25->myMax_online_window_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_25--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_25 = 0;
         first_entry_25 = 1;
         index_25 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(25, _rec_id, allkey, 8, (char**)&gbstatus_25);
            if (rc==0) {
               if(gbstatus_25->myMax_online_window_0) {
                  if(gbstatus_25->myMax_online_window_0->win) {
                     delete(gbstatus_25->myMax_online_window_0->win);
                     gbstatus_25->myMax_online_window_0->win = 0;
                  }
                  free(gbstatus_25->myMax_online_window_0);
               }
               //printf("freeing 25\n");
               free(gbstatus_25);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(25, _rec_id);
      }
      if (rc==0) {
         insert_21.a_0 = Q_0005_23_22.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10f ", insert_21.a_0);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (traffic_27 && (rc = traffic_27->c_close(traffic_27)) != 0) {
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
   _adl_statement_20();
   _adl_statement_28();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = traffic->close(traffic, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
