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
IM_REL *t;
/**** Query Declarations ****/
int _adl_statement_2()
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
         if ((rc = t->put(t, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      }
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_10()
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
   } insert_3;
   IM_RELC *t_9;
   int first_entry_8 = 1;
   int first_entry_9 = 1;
   int index_7 = 0;
   int terminating_7=0;
   struct gb_status_7 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      winbuf *sum_0_win;
      int sum_0_last_out;
      bool sum_0_iterate;
      bool sum_0_init;
   };
   struct gb_status_7 *gbstatus_7 = (struct gb_status_7 *)0;
   
   int first_entry_7 = 1;
   int first_entry_5 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = t->cursor(t, &t_9, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0001_5_4;
      next_5:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0000_7_6;
      next_7:
      while (index_7>=0 && index_7 < 2) {
         switch(index_7) {
            case 0:
            {
               if (terminating_7 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int a;
                     int OID;
                     int a_expire;
                     int OID_expire;
                     struct timeval atime;
                  } t_9_8;
                  next_9:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = t_9->c_get(t_9, &key, &data, (first_entry_8)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_8 = 0;
                     memcpy(&(t_9_8.a), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved t_9_8.a = %d\n", t_9_8.a);
                     //fflush(stdout);
                     memcpy(&(t_9_8.OID), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved t_9_8.OID = %d\n", t_9_8.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_8 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0000_7_6.a_0 = t_9_8.a;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_7 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_7 = (struct gb_status_7 *)0;
                     rc = hash_get(7, _rec_id, gbkey, 4, (char**)&gbstatus_7);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_7 = (struct gb_status_7*)malloc(sizeof(*gbstatus_7));
                        gbstatus_7->sum_0_win = new winbuf(11, 4, 4, _ADL_WIN_ROW);
                        gbstatus_7->sum_0_last_out=0;
                        gbstatus_7->sum_0_iterate=false;
                        gbstatus_7->sum_0_init=true;
                        if((!(2 <= 1 || ((gbstatus_7->sum_0_win->getTupleID() == 0 && gbstatus_7->sum_0_last_out != 0) 
                        ||((((int)gbstatus_7->sum_0_win->getTupleID()) >= gbstatus_7->sum_0_last_out + 2)))))) {
                           slide_out = 0;
                        } else {
                           slide_out = 1;
                        }
                        gbstatus_7->_baggr_0_first_entry = 1;
                        gbstatus_7->sum_0_win->updateTupleID();
                        memcpy(_databuf+0, &(Q_0000_7_6.a_0), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 4;
                        gbstatus_7->sum_0_win->put(&windata);
                        gbstatus_7->_baggr_0_value =  Q_0000_7_6.a_0;
                        rc = hash_put(7, _rec_id, gbkey, 4, &gbstatus_7);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        gbstatus_7->_baggr_0_first_entry = 1;
                        gbstatus_7->sum_0_win->updateTupleID();
                        memcpy(_databuf+0, &(Q_0000_7_6.a_0), sizeof(int));
                        windata.data = _databuf;
                        windata.size = 4;
                        gbstatus_7->sum_0_win->put(&windata);
                        while (gbstatus_7->sum_0_win->hasExpired()) {
                           gbstatus_7->sum_0_win->getExpired(&windata);
                           memcpy(&(Q_0000_7_6.a_0_expire), (char*)windata.data+0, sizeof(int));
                           gbstatus_7->_baggr_0_value -=  Q_0000_7_6.a_0_expire;
                           gbstatus_7->sum_0_win->pop();
                        }
                        gbstatus_7->_baggr_0_value +=  Q_0000_7_6.a_0;
                        if((!(2 <= 1 || ((gbstatus_7->sum_0_win->getTupleID() == 0 && gbstatus_7->sum_0_last_out != 0) 
                        ||((((int)gbstatus_7->sum_0_win->getTupleID()) >= gbstatus_7->sum_0_last_out + 2)))))) {
                           slide_out = 0;
                           //printf("Here no output %d 2 %d\n", gbstatus_7->sum_0_last_out, gbstatus_7->sum_0_win->getTupleID());fflush(stdout);
                        } else {
                           slide_out = 1;
                           //printf("Here YES output %d 2 %d\n", gbstatus_7->sum_0_last_out, gbstatus_7->sum_0_win->getTupleID());fflush(stdout);
                           gbstatus_7->sum_0_last_out = gbstatus_7->sum_0_last_out + 2;
                           while(gbstatus_7->sum_0_last_out < (gbstatus_7->sum_0_win->getTupleID() - 2) && gbstatus_7->sum_0_win->getTupleID() > 0) {
                              if(2 == 1) {
                                 gbstatus_7->sum_0_last_out = gbstatus_7->sum_0_win->getTupleID();
                              }
                              else {
                                 gbstatus_7->sum_0_last_out = gbstatus_7->sum_0_last_out + 2;
                              }
                           }
                        }
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_7 = 1;
                  }
               }
               if (terminating_7 == 1) {
                  if (first_entry_7 == 1) {
                     rc = 0; /* fail on first entry, aggregate on empty set */
                  } else {
                     allkey = (char*)0;
                     rc = hash_get(7, _rec_id, allkey, 4, (char**)&gbstatus_7);
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
               if (gbstatus_7 == (struct gb_status_7 *)0) {
                  if (first_entry_7) {
                     rc = 0;
                     Q_0001_5_4.a_0 = 0;
                  }
               } else 
               if (gbstatus_7->_baggr_0_first_entry == 1) {
                  Q_0001_5_4.a_0 = gbstatus_7->_baggr_0_value;
                  gbstatus_7->_baggr_0_first_entry = 0;
                  rc = 0;
               } else {
                  gbstatus_7->_baggr_0_first_entry = 1;
               }
               first_entry_7 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_7++;
         }
         if (rc == DB_NOTFOUND) {
            index_7--;
            if (terminating_7 == 1 && index_7 == 0) {
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_7--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_7 = 0;
         first_entry_7 = 1;
         index_7 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(7, _rec_id, allkey, 4, (char**)&gbstatus_7);
            if (rc==0) {
               //printf("freeing 7\n");
               free(gbstatus_7);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(7, _rec_id);
      }
      if (rc==0) {
         insert_3.a_0 = Q_0001_5_4.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_3.a_0);
         printf("\n");
         /* INSERT ENDS */
      }
   } /* while (rc==0) */
   if (t_9 && (rc = t_9->c_close(t_9)) != 0) {
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
   _adl_statement_2();
   _adl_statement_10();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = t->close(t, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
