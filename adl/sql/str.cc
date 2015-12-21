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
IM_REL *trainFl;
IM_REL *sampleTrainFl;
IM_REL *NBModel;
IM_REL *params;
struct sample_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void sample_init(struct sample_status *status, 
	int id, int SL, int SW, int PL, int PW, int isSetosa, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void sample_init(struct sample_status *status, int id, int SL, int SW, int PL, int PW, int isSetosa,
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
      int id;
      int SL;
      int SW;
      int PL;
      int PW;
      int isSetosa;
      int id_expire;
      int SL_expire;
      int SW_expire;
      int PL_expire;
      int PW_expire;
      int isSetosa_expire;
      struct timeval atime;
   } insert_0;
   int first_entry_1 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      /* precomputed predicates */
      if (! (((rand()/(RAND_MAX+1.0)) < 0.900000)) ) {
         rc = DB_NOTFOUND;
      } else {
         next_1:
         rc = (first_entry_1)? 0:DB_NOTFOUND;
         if (rc == DB_NOTFOUND) first_entry_1=1;
         else {
            first_entry_1=0;
            insert_0.id = id;
            insert_0.SL = SL;
            insert_0.SW = SW;
            insert_0.PL = PL;
            insert_0.PW = PW;
            insert_0.isSetosa = isSetosa;
         } /* if (rc == 0) */
      } /* end of precomputed predicates */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_0.id), sizeof(int));
         memcpy((char*)data.data+0, &(insert_0.id), sizeof(int));
         memcpy((char*)data.data+4, &(insert_0.SL), sizeof(int));
         memcpy((char*)data.data+8, &(insert_0.SW), sizeof(int));
         memcpy((char*)data.data+12, &(insert_0.PL), sizeof(int));
         memcpy((char*)data.data+16, &(insert_0.PW), sizeof(int));
         memcpy((char*)data.data+20, &(insert_0.isSetosa), sizeof(int));
         data.size = 24;
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
struct inWinType_sample_window {
   int id;
   int SL;
   int SW;
   int PL;
   int PW;
   int isSetosa;
};
struct sample_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_sample_window* getLastTuple_sample_window(IM_REL* inwindow, inWinType_sample_window* tuple, bufferMngr* bm) {
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
   memcpy(&((*tuple).id), (char*)data.data+0, sizeof(int));
   //printf("Retrieved (*tuple).id = %d\n", (*tuple).id);
   //fflush(stdout);
   memcpy(&((*tuple).SL), (char*)data.data+4, sizeof(int));
   //printf("Retrieved (*tuple).SL = %d\n", (*tuple).SL);
   //fflush(stdout);
   memcpy(&((*tuple).SW), (char*)data.data+8, sizeof(int));
   //printf("Retrieved (*tuple).SW = %d\n", (*tuple).SW);
   //fflush(stdout);
   memcpy(&((*tuple).PL), (char*)data.data+12, sizeof(int));
   //printf("Retrieved (*tuple).PL = %d\n", (*tuple).PL);
   //fflush(stdout);
   memcpy(&((*tuple).PW), (char*)data.data+16, sizeof(int));
   //printf("Retrieved (*tuple).PW = %d\n", (*tuple).PW);
   //fflush(stdout);
   memcpy(&((*tuple).isSetosa), (char*)data.data+20, sizeof(int));
   //printf("Retrieved (*tuple).isSetosa = %d\n", (*tuple).isSetosa);
   //fflush(stdout);
   return tuple;
}
extern "C" void sample_window_init(struct sample_window_status *status, 
	int id, int SL, int SW, int PL, int PW, int isSetosa, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void sample_window_init(struct sample_window_status *status, int id, int SL, int SW, int PL, int PW, int isSetosa, 
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
   if(status && status->win)
   window = status->win->get_im_rel();
   struct inWinType_sample_window tuple;
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
      int a_1;
      int a_2;
      int a_3;
      int a_4;
      int a_5;
      int a_0_expire;
      int a_1_expire;
      int a_2_expire;
      int a_3_expire;
      int a_4_expire;
      int a_5_expire;
      struct timeval atime;
   } insert_2;
   IM_RELC *w_8;
   int first_entry_7 = 1;
   int first_entry_8 = 1;
   int index_6 = 0;
   int terminating_6=0;
   struct gb_status_6 {
      struct sample_status *sample_0;
   };
   struct gb_status_6 *gbstatus_6 = (struct gb_status_6 *)0;
   
   int first_entry_6 = 1;
   int first_entry_4 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_8, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_1;
         int a_2;
         int a_3;
         int a_4;
         int a_5;
         int a_0_expire;
         int a_1_expire;
         int a_2_expire;
         int a_3_expire;
         int a_4_expire;
         int a_5_expire;
         struct timeval atime;
      } Q_0001_4_3;
      next_4:
      struct {
         int a_0;
         int a_1;
         int a_2;
         int a_3;
         int a_4;
         int a_5;
         int a_0_expire;
         int a_1_expire;
         int a_2_expire;
         int a_3_expire;
         int a_4_expire;
         int a_5_expire;
         struct timeval atime;
      } Q_0000_6_5;
      next_6:
      while (index_6>=0 && index_6 < 2) {
         switch(index_6) {
            case 0:
            {
               if (terminating_6 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int id;
                     int SL;
                     int SW;
                     int PL;
                     int PW;
                     int isSetosa;
                     int id_expire;
                     int SL_expire;
                     int SW_expire;
                     int PL_expire;
                     int PW_expire;
                     int isSetosa_expire;
                     struct timeval atime;
                  } w_8_7;
                  next_8:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_8->c_get(w_8, &key, &data, (first_entry_7)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_7 = 0;
                     memcpy(&(w_8_7.id), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_8_7.id = %d\n", w_8_7.id);
                     //fflush(stdout);
                     memcpy(&(w_8_7.SL), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved w_8_7.SL = %d\n", w_8_7.SL);
                     //fflush(stdout);
                     memcpy(&(w_8_7.SW), (char*)data.data+8, sizeof(int));
                     //printf("Retrieved w_8_7.SW = %d\n", w_8_7.SW);
                     //fflush(stdout);
                     memcpy(&(w_8_7.PL), (char*)data.data+12, sizeof(int));
                     //printf("Retrieved w_8_7.PL = %d\n", w_8_7.PL);
                     //fflush(stdout);
                     memcpy(&(w_8_7.PW), (char*)data.data+16, sizeof(int));
                     //printf("Retrieved w_8_7.PW = %d\n", w_8_7.PW);
                     //fflush(stdout);
                     memcpy(&(w_8_7.isSetosa), (char*)data.data+20, sizeof(int));
                     //printf("Retrieved w_8_7.isSetosa = %d\n", w_8_7.isSetosa);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_7 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0 && slide_out == 1) {
                     Q_0000_6_5.a_0 = w_8_7.id;
                     Q_0000_6_5.a_1 = w_8_7.SL;
                     Q_0000_6_5.a_2 = w_8_7.SW;
                     Q_0000_6_5.a_3 = w_8_7.PL;
                     Q_0000_6_5.a_4 = w_8_7.PW;
                     Q_0000_6_5.a_5 = w_8_7.isSetosa;
                  } /* if (rc == 0) */
                  if (rc==0 && slide_out == 1) {
                     first_entry_6 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_6 = (struct gb_status_6 *)0;
                     rc = hash_get(6, _rec_id, gbkey, 4, (char**)&gbstatus_6);
                     if (rc == DB_NOTFOUND) {
                        gbstatus_6 = (struct gb_status_6*)malloc(sizeof(*gbstatus_6));
                        gbstatus_6->sample_0 = (struct sample_status*)malloc(sizeof(struct sample_status));
                        gbstatus_6->sample_0->win = 0;
                        setModelId("");
                        sample_init(gbstatus_6->sample_0, Q_0000_6_5.a_0, Q_0000_6_5.a_1, Q_0000_6_5.a_2, Q_0000_6_5.a_3, Q_0000_6_5.a_4, Q_0000_6_5.a_5, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(6, _rec_id, gbkey, 4, &gbstatus_6);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        sample_init(gbstatus_6->sample_0, Q_0000_6_5.a_0, Q_0000_6_5.a_1, Q_0000_6_5.a_2, Q_0000_6_5.a_3, Q_0000_6_5.a_4, Q_0000_6_5.a_5, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_6 = 1;
                  }
               }
               if (terminating_6 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(6, _rec_id, allkey, 4, (char**)&gbstatus_6);
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
               rc = gbstatus_6->sample_0->retc->c_get(gbstatus_6->sample_0->retc, &key, &data, (gbstatus_6->sample_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_6->sample_0->retc_first_entry = 0;
                  memcpy(&(Q_0001_4_3.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0001_4_3.a_0 = %d\n", Q_0001_4_3.a_0);
                  //fflush(stdout);
                  memcpy(&(Q_0001_4_3.a_1), (char*)data.data+4, sizeof(int));
                  //printf("Retrieved Q_0001_4_3.a_1 = %d\n", Q_0001_4_3.a_1);
                  //fflush(stdout);
                  memcpy(&(Q_0001_4_3.a_2), (char*)data.data+8, sizeof(int));
                  //printf("Retrieved Q_0001_4_3.a_2 = %d\n", Q_0001_4_3.a_2);
                  //fflush(stdout);
                  memcpy(&(Q_0001_4_3.a_3), (char*)data.data+12, sizeof(int));
                  //printf("Retrieved Q_0001_4_3.a_3 = %d\n", Q_0001_4_3.a_3);
                  //fflush(stdout);
                  memcpy(&(Q_0001_4_3.a_4), (char*)data.data+16, sizeof(int));
                  //printf("Retrieved Q_0001_4_3.a_4 = %d\n", Q_0001_4_3.a_4);
                  //fflush(stdout);
                  memcpy(&(Q_0001_4_3.a_5), (char*)data.data+20, sizeof(int));
                  //printf("Retrieved Q_0001_4_3.a_5 = %d\n", Q_0001_4_3.a_5);
                  //fflush(stdout);
                  if ((rc = gbstatus_6->sample_0->retc->c_del(gbstatus_6->sample_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_6->sample_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_6 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_6++;
         }
         if (rc == DB_NOTFOUND) {
            index_6--;
            if (terminating_6 == 1 && index_6 == 0) {
               if (gbstatus_6->sample_0->retc && (rc = gbstatus_6->sample_0->retc->c_close(gbstatus_6->sample_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_6->sample_0);
               
               if (gbstatus_6->sample_0->ret && ((rc = gbstatus_6->sample_0->ret->close(gbstatus_6->sample_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_6->sample_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_6--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_6 = 0;
         first_entry_6 = 1;
         index_6 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(6, _rec_id, allkey, 4, (char**)&gbstatus_6);
            if (rc==0) {
               free(gbstatus_6->sample_0);
               //printf("freeing 6\n");
               free(gbstatus_6);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(6, _rec_id);
      }
      if (rc==0 && slide_out == 1) {
         insert_2.a_0 = Q_0001_4_3.a_0;
         insert_2.a_1 = Q_0001_4_3.a_1;
         insert_2.a_2 = Q_0001_4_3.a_2;
         insert_2.a_3 = Q_0001_4_3.a_3;
         insert_2.a_4 = Q_0001_4_3.a_4;
         insert_2.a_5 = Q_0001_4_3.a_5;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_2.a_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_2.a_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_2.a_1), sizeof(int));
         memcpy((char*)data.data+8, &(insert_2.a_2), sizeof(int));
         memcpy((char*)data.data+12, &(insert_2.a_3), sizeof(int));
         memcpy((char*)data.data+16, &(insert_2.a_4), sizeof(int));
         memcpy((char*)data.data+20, &(insert_2.a_5), sizeof(int));
         data.size = 24;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_8 && (rc = w_8->c_close(w_8)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
struct blah_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void blah_init(struct blah_status *status, 
	int a, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void blah_init(struct blah_status *status, int a,
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
   status->retc_first_entry=1;
}
struct inWinType_blah_window {
   int a;
};
struct blah_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_blah_window* getLastTuple_blah_window(IM_REL* inwindow, inWinType_blah_window* tuple, bufferMngr* bm) {
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
   memcpy(&((*tuple).a), (char*)data.data+0, sizeof(int));
   //printf("Retrieved (*tuple).a = %d\n", (*tuple).a);
   //fflush(stdout);
   return tuple;
}
extern "C" void blah_window_init(struct blah_window_status *status, 
	int a, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void blah_window_init(struct blah_window_status *status, int a, 
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
   if(status && status->win)
   window = status->win->get_im_rel();
   struct inWinType_blah_window tuple;
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
      struct blah_status *blah_0;
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
      } Q_0003_11_10;
      next_11:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0002_13_12;
      next_13:
      while (index_13>=0 && index_13 < 2) {
         switch(index_13) {
            case 0:
            {
               if (terminating_13 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int a;
                     int a_expire;
                     struct timeval atime;
                  } w_15_14;
                  next_15:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_15->c_get(w_15, &key, &data, (first_entry_14)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_14 = 0;
                     memcpy(&(w_15_14.a), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_15_14.a = %d\n", w_15_14.a);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_14 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0 && slide_out == 1) {
                     Q_0002_13_12.a_0 = w_15_14.a;
                  } /* if (rc == 0) */
                  if (rc==0 && slide_out == 1) {
                     first_entry_13 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_13 = (struct gb_status_13 *)0;
                     rc = hash_get(13, _rec_id, gbkey, 4, (char**)&gbstatus_13);
                     if (rc == DB_NOTFOUND) {
                        gbstatus_13 = (struct gb_status_13*)malloc(sizeof(*gbstatus_13));
                        gbstatus_13->blah_0 = (struct blah_status*)malloc(sizeof(struct blah_status));
                        gbstatus_13->blah_0->win = 0;
                        setModelId("");
                        blah_init(gbstatus_13->blah_0, Q_0002_13_12.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(13, _rec_id, gbkey, 4, &gbstatus_13);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        blah_init(gbstatus_13->blah_0, Q_0002_13_12.a_0, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
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
               rc = gbstatus_13->blah_0->retc->c_get(gbstatus_13->blah_0->retc, &key, &data, (gbstatus_13->blah_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_13->blah_0->retc_first_entry = 0;
                  memcpy(&(Q_0003_11_10.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0003_11_10.a_0 = %d\n", Q_0003_11_10.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_13->blah_0->retc->c_del(gbstatus_13->blah_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_13->blah_0->retc_first_entry = 1;
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
               if (gbstatus_13->blah_0->retc && (rc = gbstatus_13->blah_0->retc->c_close(gbstatus_13->blah_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_13->blah_0);
               
               if (gbstatus_13->blah_0->ret && ((rc = gbstatus_13->blah_0->ret->close(gbstatus_13->blah_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_13->blah_0->ret = NULL;
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
               free(gbstatus_13->blah_0);
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
      if (rc==0 && slide_out == 1) {
         insert_9.a_0 = Q_0003_11_10.a_0;
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
IM_REL *naivebayesian_NBModel;
IM_REL *naivebayesian_params;
/**** Query Declarations ****/
int _adl_statement_16()
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
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   {
      FILE *_adl_load = fopen("/home/hthakkar/adl/sql/clsf/iris2.data", "rt");
      char _adl_load_buf[40960], *tok;
      char loadkeybuf[1], loaddatabuf[29];
      int _adl_line_no=0;
      char bPoint =0;
      if (!_adl_load) {
         printf("can not open file /home/hthakkar/adl/sql/clsf/iris2.data.\n");
         exit(1);
      }
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      key.data = loadkeybuf;
      data.data = loaddatabuf;
      key.size = 0;
      data.size = 28;
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
         *(int*)((char*)data.data+4) = atoi(tok);
         tok = csvtok(NULL, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(int*)((char*)data.data+8) = atoi(tok);
         tok = csvtok(NULL, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(int*)((char*)data.data+12) = atoi(tok);
         tok = csvtok(NULL, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(int*)((char*)data.data+16) = atoi(tok);
         tok = csvtok(NULL, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(int*)((char*)data.data+20) = atoi(tok);
         if ((rc = trainFl->put(trainFl, &key, &data, DB_APPEND))!=0) {
            exit(rc);
         }
      } /* end of while */
      fclose(_adl_load);
   }
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_24()
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
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   struct {
      int a_0;
      int a_1;
      int a_2;
      int a_3;
      int a_4;
      int a_5;
      int a_0_expire;
      int a_1_expire;
      int a_2_expire;
      int a_3_expire;
      int a_4_expire;
      int a_5_expire;
      struct timeval atime;
   } insert_17;
   IM_RELC *trainFl_23;
   int first_entry_22 = 1;
   int first_entry_23 = 1;
   int index_21 = 0;
   int terminating_21=0;
   struct gb_status_21 {
      struct sample_status *sample_0;
   };
   struct gb_status_21 *gbstatus_21 = (struct gb_status_21 *)0;
   
   int first_entry_21 = 1;
   int first_entry_19 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = trainFl->cursor(trainFl, &trainFl_23, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_1;
         int a_2;
         int a_3;
         int a_4;
         int a_5;
         int a_0_expire;
         int a_1_expire;
         int a_2_expire;
         int a_3_expire;
         int a_4_expire;
         int a_5_expire;
         struct timeval atime;
      } Q_0005_19_18;
      next_19:
      struct {
         int a_0;
         int a_1;
         int a_2;
         int a_3;
         int a_4;
         int a_5;
         int a_0_expire;
         int a_1_expire;
         int a_2_expire;
         int a_3_expire;
         int a_4_expire;
         int a_5_expire;
         struct timeval atime;
      } Q_0004_21_20;
      next_21:
      while (index_21>=0 && index_21 < 2) {
         switch(index_21) {
            case 0:
            {
               if (terminating_21 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int id;
                     int SL;
                     int SW;
                     int PL;
                     int PW;
                     int isSetosa;
                     int OID;
                     int id_expire;
                     int SL_expire;
                     int SW_expire;
                     int PL_expire;
                     int PW_expire;
                     int isSetosa_expire;
                     int OID_expire;
                     struct timeval atime;
                  } trainFl_23_22;
                  next_23:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = trainFl_23->c_get(trainFl_23, &key, &data, (first_entry_22)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_22 = 0;
                     memcpy(&(trainFl_23_22.id), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved trainFl_23_22.id = %d\n", trainFl_23_22.id);
                     //fflush(stdout);
                     memcpy(&(trainFl_23_22.SL), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved trainFl_23_22.SL = %d\n", trainFl_23_22.SL);
                     //fflush(stdout);
                     memcpy(&(trainFl_23_22.SW), (char*)data.data+8, sizeof(int));
                     //printf("Retrieved trainFl_23_22.SW = %d\n", trainFl_23_22.SW);
                     //fflush(stdout);
                     memcpy(&(trainFl_23_22.PL), (char*)data.data+12, sizeof(int));
                     //printf("Retrieved trainFl_23_22.PL = %d\n", trainFl_23_22.PL);
                     //fflush(stdout);
                     memcpy(&(trainFl_23_22.PW), (char*)data.data+16, sizeof(int));
                     //printf("Retrieved trainFl_23_22.PW = %d\n", trainFl_23_22.PW);
                     //fflush(stdout);
                     memcpy(&(trainFl_23_22.isSetosa), (char*)data.data+20, sizeof(int));
                     //printf("Retrieved trainFl_23_22.isSetosa = %d\n", trainFl_23_22.isSetosa);
                     //fflush(stdout);
                     memcpy(&(trainFl_23_22.OID), (char*)data.data+24, sizeof(int));
                     //printf("Retrieved trainFl_23_22.OID = %d\n", trainFl_23_22.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_22 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0 && slide_out == 1) {
                     Q_0004_21_20.a_0 = trainFl_23_22.id;
                     Q_0004_21_20.a_1 = trainFl_23_22.SL;
                     Q_0004_21_20.a_2 = trainFl_23_22.SW;
                     Q_0004_21_20.a_3 = trainFl_23_22.PL;
                     Q_0004_21_20.a_4 = trainFl_23_22.PW;
                     Q_0004_21_20.a_5 = trainFl_23_22.isSetosa;
                  } /* if (rc == 0) */
                  if (rc==0 && slide_out == 1) {
                     first_entry_21 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_21 = (struct gb_status_21 *)0;
                     rc = hash_get(21, _rec_id, gbkey, 4, (char**)&gbstatus_21);
                     if (rc == DB_NOTFOUND) {
                        gbstatus_21 = (struct gb_status_21*)malloc(sizeof(*gbstatus_21));
                        gbstatus_21->sample_0 = (struct sample_status*)malloc(sizeof(struct sample_status));
                        gbstatus_21->sample_0->win = 0;
                        setModelId("naivebayesian_");
                        sample_init(gbstatus_21->sample_0, Q_0004_21_20.a_0, Q_0004_21_20.a_1, Q_0004_21_20.a_2, Q_0004_21_20.a_3, Q_0004_21_20.a_4, Q_0004_21_20.a_5, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(21, _rec_id, gbkey, 4, &gbstatus_21);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("naivebayesian_");
                        sample_init(gbstatus_21->sample_0, Q_0004_21_20.a_0, Q_0004_21_20.a_1, Q_0004_21_20.a_2, Q_0004_21_20.a_3, Q_0004_21_20.a_4, Q_0004_21_20.a_5, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_21 = 1;
                  }
               }
               if (terminating_21 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(21, _rec_id, allkey, 4, (char**)&gbstatus_21);
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
               rc = gbstatus_21->sample_0->retc->c_get(gbstatus_21->sample_0->retc, &key, &data, (gbstatus_21->sample_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_21->sample_0->retc_first_entry = 0;
                  memcpy(&(Q_0005_19_18.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0005_19_18.a_0 = %d\n", Q_0005_19_18.a_0);
                  //fflush(stdout);
                  memcpy(&(Q_0005_19_18.a_1), (char*)data.data+4, sizeof(int));
                  //printf("Retrieved Q_0005_19_18.a_1 = %d\n", Q_0005_19_18.a_1);
                  //fflush(stdout);
                  memcpy(&(Q_0005_19_18.a_2), (char*)data.data+8, sizeof(int));
                  //printf("Retrieved Q_0005_19_18.a_2 = %d\n", Q_0005_19_18.a_2);
                  //fflush(stdout);
                  memcpy(&(Q_0005_19_18.a_3), (char*)data.data+12, sizeof(int));
                  //printf("Retrieved Q_0005_19_18.a_3 = %d\n", Q_0005_19_18.a_3);
                  //fflush(stdout);
                  memcpy(&(Q_0005_19_18.a_4), (char*)data.data+16, sizeof(int));
                  //printf("Retrieved Q_0005_19_18.a_4 = %d\n", Q_0005_19_18.a_4);
                  //fflush(stdout);
                  memcpy(&(Q_0005_19_18.a_5), (char*)data.data+20, sizeof(int));
                  //printf("Retrieved Q_0005_19_18.a_5 = %d\n", Q_0005_19_18.a_5);
                  //fflush(stdout);
                  if ((rc = gbstatus_21->sample_0->retc->c_del(gbstatus_21->sample_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_21->sample_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_21 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_21++;
         }
         if (rc == DB_NOTFOUND) {
            index_21--;
            if (terminating_21 == 1 && index_21 == 0) {
               if (gbstatus_21->sample_0->retc && (rc = gbstatus_21->sample_0->retc->c_close(gbstatus_21->sample_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_21->sample_0);
               
               if (gbstatus_21->sample_0->ret && ((rc = gbstatus_21->sample_0->ret->close(gbstatus_21->sample_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_21->sample_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_21--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_21 = 0;
         first_entry_21 = 1;
         index_21 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(21, _rec_id, allkey, 4, (char**)&gbstatus_21);
            if (rc==0) {
               free(gbstatus_21->sample_0);
               //printf("freeing 21\n");
               free(gbstatus_21);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(21, _rec_id);
      }
      if (rc==0 && slide_out == 1) {
         insert_17.a_0 = Q_0005_19_18.a_0;
         insert_17.a_1 = Q_0005_19_18.a_1;
         insert_17.a_2 = Q_0005_19_18.a_2;
         insert_17.a_3 = Q_0005_19_18.a_3;
         insert_17.a_4 = Q_0005_19_18.a_4;
         insert_17.a_5 = Q_0005_19_18.a_5;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_17.a_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_17.a_1), sizeof(int));
         memcpy((char*)data.data+8, &(insert_17.a_2), sizeof(int));
         memcpy((char*)data.data+12, &(insert_17.a_3), sizeof(int));
         memcpy((char*)data.data+16, &(insert_17.a_4), sizeof(int));
         memcpy((char*)data.data+20, &(insert_17.a_5), sizeof(int));
         data.size = 24;
         key.size = 0;
         if ((rc = sampleTrainFl->put(sampleTrainFl, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (trainFl_23 && (rc = trainFl_23->c_close(trainFl_23)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
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
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   struct {
      int id;
      int SL;
      int SW;
      int PL;
      int PW;
      int isSetosa;
      int id_expire;
      int SL_expire;
      int SW_expire;
      int PL_expire;
      int PW_expire;
      int isSetosa_expire;
      struct timeval atime;
   } insert_25;
   IM_RELC *sampleTrainFl_27;
   int first_entry_26 = 1;
   int first_entry_27 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = sampleTrainFl->cursor(sampleTrainFl, &sampleTrainFl_27, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int id;
         int SL;
         int SW;
         int PL;
         int PW;
         int isSetosa;
         int OID;
         int id_expire;
         int SL_expire;
         int SW_expire;
         int PL_expire;
         int PW_expire;
         int isSetosa_expire;
         int OID_expire;
         struct timeval atime;
      } sampleTrainFl_27_26;
      next_27:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = sampleTrainFl_27->c_get(sampleTrainFl_27, &key, &data, (first_entry_26)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_26 = 0;
         memcpy(&(sampleTrainFl_27_26.id), (char*)data.data+0, sizeof(int));
         //printf("Retrieved sampleTrainFl_27_26.id = %d\n", sampleTrainFl_27_26.id);
         //fflush(stdout);
         memcpy(&(sampleTrainFl_27_26.SL), (char*)data.data+4, sizeof(int));
         //printf("Retrieved sampleTrainFl_27_26.SL = %d\n", sampleTrainFl_27_26.SL);
         //fflush(stdout);
         memcpy(&(sampleTrainFl_27_26.SW), (char*)data.data+8, sizeof(int));
         //printf("Retrieved sampleTrainFl_27_26.SW = %d\n", sampleTrainFl_27_26.SW);
         //fflush(stdout);
         memcpy(&(sampleTrainFl_27_26.PL), (char*)data.data+12, sizeof(int));
         //printf("Retrieved sampleTrainFl_27_26.PL = %d\n", sampleTrainFl_27_26.PL);
         //fflush(stdout);
         memcpy(&(sampleTrainFl_27_26.PW), (char*)data.data+16, sizeof(int));
         //printf("Retrieved sampleTrainFl_27_26.PW = %d\n", sampleTrainFl_27_26.PW);
         //fflush(stdout);
         memcpy(&(sampleTrainFl_27_26.isSetosa), (char*)data.data+20, sizeof(int));
         //printf("Retrieved sampleTrainFl_27_26.isSetosa = %d\n", sampleTrainFl_27_26.isSetosa);
         //fflush(stdout);
         memcpy(&(sampleTrainFl_27_26.OID), (char*)data.data+24, sizeof(int));
         //printf("Retrieved sampleTrainFl_27_26.OID = %d\n", sampleTrainFl_27_26.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_26 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0 && slide_out == 1) {
         insert_25.id = sampleTrainFl_27_26.id;
         insert_25.SL = sampleTrainFl_27_26.SL;
         insert_25.SW = sampleTrainFl_27_26.SW;
         insert_25.PL = sampleTrainFl_27_26.PL;
         insert_25.PW = sampleTrainFl_27_26.PW;
         insert_25.isSetosa = sampleTrainFl_27_26.isSetosa;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_25.id);
         printf("%10d ", insert_25.SL);
         printf("%10d ", insert_25.SW);
         printf("%10d ", insert_25.PL);
         printf("%10d ", insert_25.PW);
         printf("%10d ", insert_25.isSetosa);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (sampleTrainFl_27 && (rc = sampleTrainFl_27->c_close(sampleTrainFl_27)) != 0) {
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
   if ((rc = im_rel_create(&trainFl, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = trainFl->open(trainFl, "_adl_db_trainFl", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("trainFl") == 0) {
      inMemTables->operator[](strdup("trainFl")) = trainFl;
   }
   if ((rc = im_rel_create(&sampleTrainFl, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = sampleTrainFl->open(sampleTrainFl, "_adl_db_sampleTrainFl", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("sampleTrainFl") == 0) {
      inMemTables->operator[](strdup("sampleTrainFl")) = sampleTrainFl;
   }
   if ((rc = im_rel_create(&NBModel, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = NBModel->open(NBModel, "_adl_db_NBModel", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("NBModel") == 0) {
      inMemTables->operator[](strdup("NBModel")) = NBModel;
   }
   if ((rc = im_rel_create(&params, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = params->open(params, "_adl_db_params", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("params") == 0) {
      inMemTables->operator[](strdup("params")) = params;
   }
   if ((rc = im_rel_create(&naivebayesian_NBModel, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = naivebayesian_NBModel->open(naivebayesian_NBModel, "_adl_db_naivebayesian_NBModel", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("naivebayesian_NBModel") == 0) {
      inMemTables->operator[](strdup("naivebayesian_NBModel")) = naivebayesian_NBModel;
   }
   if ((rc = im_rel_create(&naivebayesian_params, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = naivebayesian_params->open(naivebayesian_params, "_adl_db_naivebayesian_params", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("naivebayesian_params") == 0) {
      inMemTables->operator[](strdup("naivebayesian_params")) = naivebayesian_params;
   }
   _adl_statement_16();
   _adl_statement_24();
   _adl_statement_28();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = trainFl->close(trainFl, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = sampleTrainFl->close(sampleTrainFl, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = NBModel->close(NBModel, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = params->close(params, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = naivebayesian_NBModel->close(naivebayesian_NBModel, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = naivebayesian_params->close(naivebayesian_params, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
