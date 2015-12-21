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
IM_REL *mytab1;
IM_REL *mytab2;
struct tmp_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void tmp_init(struct tmp_status *status, int a, char* tabName, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 	
hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, vector<A_timeexp>* plist=NULL, int endSlide=0);
extern "C" void tmp_init(struct tmp_status *status, int a, char* tabName, int _rec_id, int __is_init, bufferMngr* bm, 	
hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, vector<A_timeexp>* plist, int endSlide)
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
      int a;
      double b;
      char c[11];
      int a_expire;
      double b_expire;
      char c_expire[11];
      struct timeval atime;
   } insert_0;
   IM_RELC *t_3;
   int first_entry_4 = 1;
   int first_entry_2 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = ((IM_REL*)inMemTables->operator[](tabName))->cursor(((IM_REL*)inMemTables->operator[](tabName)), &t_3, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int a;
         double b;
         char c[11];
         int OID;
         int a_expire;
         double b_expire;
         char c_expire[11];
         int OID_expire;
         struct timeval atime;
      } t_2_1;
      next_2:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = t_3->c_get(t_3, &key, &data, (first_entry_4)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_4 = 0;
         memcpy(&(t_2_1.a), (char*)data.data+0, sizeof(int));
         //printf("Retrieved t_2_1.a = %d\n", t_2_1.a);
         //fflush(stdout);
         memcpy(&(t_2_1.b), (char*)data.data+4, sizeof(double));
         memcpy(t_2_1.c, (char*)data.data+12, 10);
         *(t_2_1.c+10) = '\0';
         memcpy(&(t_2_1.OID), (char*)data.data+22, sizeof(int));
         //printf("Retrieved t_2_1.OID = %d\n", t_2_1.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_4 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0 && slide_out == 1) {
         insert_0.a = t_2_1.a;
         insert_0.b = t_2_1.b;
         memcpy(insert_0.c, t_2_1.c, 10);
         insert_0.c[10]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_0.a);
         printf("%10f ", insert_0.b);
         printf("\t%s\t ", insert_0.c);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (t_3 && (rc = t_3->c_close(t_3)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
struct inWinType_tmp_window {
   int a;
   char* tabName;
};
struct tmp_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_tmp_window* getLastTuple_tmp_window(IM_REL* inwindow, inWinType_tmp_window* tuple, bufferMngr* bm) {
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
   memcpy((*tuple).tabName, (char*)data.data+4, 20);
   *((*tuple).tabName+20) = '\0';
   return tuple;
}
extern "C" void tmp_window_init(struct tmp_window_status *status, int a, char* tabName, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 	
hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, vector<A_timeexp>* plist=NULL, int endSlide=0);
extern "C" void tmp_window_init(struct tmp_window_status *status, int a, char* tabName, int _rec_id, int __is_init, bufferMngr* bm, 	
hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, vector<A_timeexp>* plist, int endSlide)
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
   struct inWinType_tmp_window tuple;
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
   } insert_5;
   IM_RELC *w_11;
   int first_entry_10 = 1;
   int first_entry_11 = 1;
   int index_9 = 0;
   int terminating_9=0;
   struct gb_status_9 {
      struct tmp_status *tmp_0;
   };
   struct gb_status_9 *gbstatus_9 = (struct gb_status_9 *)0;
   
   int first_entry_9 = 1;
   int first_entry_7 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_11, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0001_7_6;
      next_7:
      struct {
         int a_0;
         char a_1[21];
         int a_0_expire;
         char a_1_expire[21];
         struct timeval atime;
      } Q_0000_9_8;
      next_9:
      while (index_9>=0 && index_9 < 2) {
         switch(index_9) {
            case 0:
            {
               if (terminating_9 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int a;
                     char tabName[21];
                     int a_expire;
                     char tabName_expire[21];
                     struct timeval atime;
                  } w_11_10;
                  next_11:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_11->c_get(w_11, &key, &data, (first_entry_10)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_10 = 0;
                     memcpy(&(w_11_10.a), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_11_10.a = %d\n", w_11_10.a);
                     //fflush(stdout);
                     memcpy(w_11_10.tabName, (char*)data.data+4, 20);
                     *(w_11_10.tabName+20) = '\0';
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_10 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0 && slide_out == 1) {
                     Q_0000_9_8.a_0 = w_11_10.a;
                     memcpy(Q_0000_9_8.a_1, w_11_10.tabName, 20);
                     Q_0000_9_8.a_1[20]=0;
                  } /* if (rc == 0) */
                  if (rc==0 && slide_out == 1) {
                     first_entry_9 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_9 = (struct gb_status_9 *)0;
                     rc = hash_get(9, _rec_id, gbkey, 4, (char**)&gbstatus_9);
                     if (rc == DB_NOTFOUND) {
                        gbstatus_9 = (struct gb_status_9*)malloc(sizeof(*gbstatus_9));
                        gbstatus_9->tmp_0 = (struct tmp_status*)malloc(sizeof(struct tmp_status));
                        gbstatus_9->tmp_0->win = 0;
                        setModelId("");
                        tmp_init(gbstatus_9->tmp_0, Q_0000_9_8.a_0, Q_0000_9_8.a_1, _rec_id+1, 1, NULL, inMemTables);
                        rc = hash_put(9, _rec_id, gbkey, 4, &gbstatus_9);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        tmp_init(gbstatus_9->tmp_0, Q_0000_9_8.a_0, Q_0000_9_8.a_1, _rec_id+1, 0, NULL, inMemTables);
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_9 = 1;
                  }
               }
               if (terminating_9 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(9, _rec_id, allkey, 4, (char**)&gbstatus_9);
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
               rc = gbstatus_9->tmp_0->retc->c_get(gbstatus_9->tmp_0->retc, &key, &data, (gbstatus_9->tmp_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_9->tmp_0->retc_first_entry = 0;
                  memcpy(&(Q_0001_7_6.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0001_7_6.a_0 = %d\n", Q_0001_7_6.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_9->tmp_0->retc->c_del(gbstatus_9->tmp_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_9->tmp_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_9 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_9++;
         }
         if (rc == DB_NOTFOUND) {
            index_9--;
            if (terminating_9 == 1 && index_9 == 0) {
               if (gbstatus_9->tmp_0->retc && (rc = gbstatus_9->tmp_0->retc->c_close(gbstatus_9->tmp_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_9->tmp_0);
               
               if (gbstatus_9->tmp_0->ret && ((rc = gbstatus_9->tmp_0->ret->close(gbstatus_9->tmp_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_9->tmp_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_9--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_9 = 0;
         first_entry_9 = 1;
         index_9 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(9, _rec_id, allkey, 4, (char**)&gbstatus_9);
            if (rc==0) {
               free(gbstatus_9->tmp_0);
               //printf("freeing 9\n");
               free(gbstatus_9);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(9, _rec_id);
      }
      if (rc==0 && slide_out == 1) {
         insert_5.a_0 = Q_0001_7_6.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_5.a_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_5.a_0), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_11 && (rc = w_11->c_close(w_11)) != 0) {
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
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   struct {
      int field_0;
      double field_1;
      char field_2[11];
      int field_0_expire;
      double field_1_expire;
      char field_2_expire[11];
      struct timeval atime;
   } insert_12;
   int first_entry_13 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_13:
      rc = (first_entry_13)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_13=1;
      else {
         first_entry_13=0;
         insert_12.field_0 = 1;
         insert_12.field_1 = 2.200000;
         memcpy(insert_12.field_2, "abc", 4);
         insert_12.field_2[4]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_12.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_12.field_1), sizeof(double));
         memcpy((char*)data.data+12, insert_12.field_2, 10);
         data.size = 22;
         key.size = 0;
         if ((rc = mytab2->put(mytab2, &key, &data, DB_APPEND))!=0) {
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
int _adl_statement_17()
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
      int field_0;
      double field_1;
      char field_2[11];
      int field_0_expire;
      double field_1_expire;
      char field_2_expire[11];
      struct timeval atime;
   } insert_15;
   int first_entry_16 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_16:
      rc = (first_entry_16)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_16=1;
      else {
         first_entry_16=0;
         insert_15.field_0 = 2;
         insert_15.field_1 = 3.300000;
         memcpy(insert_15.field_2, "def", 4);
         insert_15.field_2[4]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_15.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_15.field_1), sizeof(double));
         memcpy((char*)data.data+12, insert_15.field_2, 10);
         data.size = 22;
         key.size = 0;
         if ((rc = mytab2->put(mytab2, &key, &data, DB_APPEND))!=0) {
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
      int a_0_expire;
      struct timeval atime;
   } insert_18;
   int first_entry_23 = 1;
   int index_22 = 0;
   int terminating_22=0;
   struct gb_status_22 {
      struct tmp_status *tmp_0;
   };
   struct gb_status_22 *gbstatus_22 = (struct gb_status_22 *)0;
   
   int first_entry_22 = 1;
   int first_entry_20 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0003_20_19;
      next_20:
      struct {
         int a_0;
         char a_1[8];
         int a_0_expire;
         char a_1_expire[8];
         struct timeval atime;
      } Q_0002_22_21;
      next_22:
      while (index_22>=0 && index_22 < 2) {
         switch(index_22) {
            case 0:
            {
               if (terminating_22 == 0) {
                  /* get source tuple from qun */
                  next_23:
                  rc = (first_entry_23)? 0:DB_NOTFOUND;
                  if (rc == DB_NOTFOUND) first_entry_23=1;
                  else {
                     first_entry_23=0;
                     Q_0002_22_21.a_0 = 1;
                     memcpy(Q_0002_22_21.a_1, "mytab2", 7);
                     Q_0002_22_21.a_1[7]=0;
                  } /* if (rc == 0) */
                  if (rc==0 && slide_out == 1) {
                     first_entry_22 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_22 = (struct gb_status_22 *)0;
                     rc = hash_get(22, _rec_id, gbkey, 4, (char**)&gbstatus_22);
                     if (rc == DB_NOTFOUND) {
                        gbstatus_22 = (struct gb_status_22*)malloc(sizeof(*gbstatus_22));
                        gbstatus_22->tmp_0 = (struct tmp_status*)malloc(sizeof(struct tmp_status));
                        gbstatus_22->tmp_0->win = 0;
                        setModelId("");
                        tmp_init(gbstatus_22->tmp_0, Q_0002_22_21.a_0, Q_0002_22_21.a_1, _rec_id+1, 1, NULL, inMemTables);
                        rc = hash_put(22, _rec_id, gbkey, 4, &gbstatus_22);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        tmp_init(gbstatus_22->tmp_0, Q_0002_22_21.a_0, Q_0002_22_21.a_1, _rec_id+1, 0, NULL, inMemTables);
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_22 = 1;
                  }
               }
               if (terminating_22 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(22, _rec_id, allkey, 4, (char**)&gbstatus_22);
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
               rc = gbstatus_22->tmp_0->retc->c_get(gbstatus_22->tmp_0->retc, &key, &data, (gbstatus_22->tmp_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_22->tmp_0->retc_first_entry = 0;
                  memcpy(&(Q_0003_20_19.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0003_20_19.a_0 = %d\n", Q_0003_20_19.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_22->tmp_0->retc->c_del(gbstatus_22->tmp_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_22->tmp_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_22 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_22++;
         }
         if (rc == DB_NOTFOUND) {
            index_22--;
            if (terminating_22 == 1 && index_22 == 0) {
               if (gbstatus_22->tmp_0->retc && (rc = gbstatus_22->tmp_0->retc->c_close(gbstatus_22->tmp_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_22->tmp_0);
               
               if (gbstatus_22->tmp_0->ret && ((rc = gbstatus_22->tmp_0->ret->close(gbstatus_22->tmp_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_22->tmp_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_22--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_22 = 0;
         first_entry_22 = 1;
         index_22 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(22, _rec_id, allkey, 4, (char**)&gbstatus_22);
            if (rc==0) {
               free(gbstatus_22->tmp_0);
               //printf("freeing 22\n");
               free(gbstatus_22);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(22, _rec_id);
      }
      if (rc==0 && slide_out == 1) {
         insert_18.a_0 = Q_0003_20_19.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_18.a_0);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
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
   if ((rc = im_rel_create(&mytab1, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = mytab1->open(mytab1, "_adl_db_mytab1", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("mytab1") == 0) {
      inMemTables->operator[](strdup("mytab1")) = mytab1;
   }
   if ((rc = im_rel_create(&mytab2, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = mytab2->open(mytab2, "_adl_db_mytab2", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("mytab2") == 0) {
      inMemTables->operator[](strdup("mytab2")) = mytab2;
   }
   _adl_statement_14();
   _adl_statement_17();
   _adl_statement_24();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = mytab1->close(mytab1, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = mytab2->close(mytab2, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
