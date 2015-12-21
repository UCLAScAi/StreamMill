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
   #include <mcheck.h>
}
#include "adllib.h"
#include <ext/hash_map>
#include <winbuf.h>
#include <windowBuf.h>
#define TBL_NOOPTIONS 0
#define TBL_LASTROW 1
#define TBL_NEWROW 2
#define TBL_NEWROWEOF 4
#define SQL_TEXT Latin_Text
#include "sqltypes_td.h"
using namespace ESL;

using namespace __gnu_cxx;

static hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables=new hash_map<const char*, void*, hash<const char*>, eqstrTab>;
extern int _ADL_NULL;
extern char* _ADL_NULL_STR;
#define MAX_STR_LEN 40960


int adldebug = 0;
int _adl_sqlcode, _adl_cursqlcode;
bool copied = true; // check if we need to update

/**** Global Declarations ****/
struct aggregatetesta_status {
   IM_REL *state;
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};

extern "C" void aggregatetesta_init(struct aggregatetesta_status *status, 
	int Next, double bin, double cin, int din, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL, char* data_all_row=NULL, char* data_schema=NULL);
extern "C" void aggregatetesta_iterate(struct aggregatetesta_status *status, 
	int Next, double bin, double cin, int din, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL, char* data_all_row=NULL, char* data_schema=NULL);
extern "C" void aggregatetesta_terminate(struct aggregatetesta_status *status, 
	int Next, double bin, double cin, int din, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL, char* data_all_row=NULL, char* data_schema=NULL);
extern "C" void aggregatetesta_init(struct aggregatetesta_status *status, int Next, double bin, double cin, int din,
	int _rec_id, int __is_init, bufferMngr* bm, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
	vector<A_timeexp>* plist, int endSlide, char* _modelId, char* data_all_row, char* data_schema)
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
   if (__is_init) {//???
      sprintf(_adl_dbname, "._%d_state", status);
      if ((rc = im_rel_create(&status->state, NULL, IM_LINKEDLIST, 0)) != 0) {
         adlabort(rc, "im_rel_create()");
      }
      if ((rc = status->state->open(status->state, _adl_dbname, 0)) != 0) {
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
      double field_2;
      int field_3;
      int field_0_expire;
      double field_1_expire;
      double field_2_expire;
      int field_3_expire;
      struct timeval atime;
   } insert_0;
   char* insert_0_schema = 
   	"insert_0\n"
   	"field_0 INT\n"
   	"field_1 Double\n"
   	"field_2 Double\n"
   	"field_3 INT\n";
   int first_entry_1 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY 2 */;
   while (rc==0) { 
      next_1:
      rc = (first_entry_1)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_1=1;
      else {
         first_entry_1=0;
         insert_0.field_0 = 0;
         insert_0.field_1 = 0;
         insert_0.field_2 = 0;
         insert_0.field_3 = 0;
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
         memcpy((char*)data.data+12, &(insert_0.field_2), sizeof(double));
         memcpy((char*)data.data+20, &(insert_0.field_3), sizeof(int));
         data.size = 24;
         key.size = 0;
         if ((rc = status->state->put(status->state, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0)*/
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}

extern "C" void aggregatetesta_iterate(struct aggregatetesta_status *status, 
	int Next, double bin, double cin, int din, int _rec_id, bufferMngr* bm, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
	vector<A_timeexp>* plist, int endSlide, char* _modelId, char* data_all_row, char* data_schema)
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
   struct {
      int a_0;
      double a_1;
      double a_2;
      int a_3;
      int a_0_expire;
      double a_1_expire;
      double a_2_expire;
      int a_3_expire;
      struct timeval atime;
   } insert_2;
   char* insert_2_schema = 
   	"insert_2\n"
   	"a_0 INT\n"
   	"a_1 Double\n"
   	"a_2 Double\n"
   	"a_3 INT\n";
   IM_RELC *state_8;
   int first_entry_7 = 1;
   int first_entry_8 = 1;
   int index_6 = 0;
   int terminating_6=0;
   struct gb_status_6 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int max_0_last_out;
      bool max_0_iterate;
      bool max_0_init;
      int _baggr_1_value;
      int _baggr_1_first_entry;
      int max_1_last_out;
      bool max_1_iterate;
      bool max_1_init;
      int _baggr_2_value;
      int _baggr_2_first_entry;
      int max_2_last_out;
      bool max_2_iterate;
      bool max_2_init;
      int _baggr_3_value;
      int _baggr_3_first_entry;
      int max_3_last_out;
      bool max_3_iterate;
      bool max_3_init;
   };
   struct gb_status_6 *gbstatus_6 = (struct gb_status_6 *)0;
   
   int first_entry_6 = 1;
   int first_entry_4 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY 2 */;
   if ((rc = status->state->cursor(status->state, &state_8, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) { 
      struct {
         int a_0;
         int a_1;
         int a_2;
         int a_3;
         int a_0_expire;
         int a_1_expire;
         int a_2_expire;
         int a_3_expire;
         struct timeval atime;
      } Q_0001_4_3;
      char* Q_0001_4_3_schema = 
      	"Q_0001_4_3\n"
      	"a_0 INT\n"
      	"a_1 INT\n"
      	"a_2 INT\n"
      	"a_3 INT\n";
      next_4:
      struct {
         int a_0;
         double a_1;
         double a_2;
         int a_3;
         int a_0_expire;
         double a_1_expire;
         double a_2_expire;
         int a_3_expire;
         struct timeval atime;
      } Q_0000_6_5;
      char* Q_0000_6_5_schema = 
      	"Q_0000_6_5\n"
      	"a_0 INT\n"
      	"a_1 Double\n"
      	"a_2 Double\n"
      	"a_3 INT\n";
      /* The inner while */
      next_6:
      while (index_6>=0 && index_6 < 5) {
         /* get source tuple from qun */
         struct {
            int sum;
            double b;
            double c;
            int d;
            int OID;
            int sum_expire;
            double b_expire;
            double c_expire;
            int d_expire;
            int OID_expire;
            struct timeval atime;
         } state_8_7;
         char* state_8_7_schema = 
         	"state_8_7\n"
         	"sum INT\n"
         	"b Double\n"
         	"c Double\n"
         	"d INT\n"
         	"OID INT\n";
         next_8:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_8->c_get(state_8, &key, &data, (first_entry_7)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_7 = 0;
            memcpy(&(state_8_7.sum), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_8_7.sum = %d\n", state_8_7.sum);
            //fflush(stdout);
            memcpy(&(state_8_7.b), (char*)data.data+4, sizeof(double));
            //printf("Retrieved state_8_7.b = %f\n", state_8_7.b);
            //fflush(stdout);
            memcpy(&(state_8_7.c), (char*)data.data+12, sizeof(double));
            //printf("Retrieved state_8_7.c = %f\n", state_8_7.c);
            //fflush(stdout);
            memcpy(&(state_8_7.d), (char*)data.data+20, sizeof(int));
            //printf("Retrieved state_8_7.d = %d\n", state_8_7.d);
            //fflush(stdout);
            memcpy(&(state_8_7.OID), (char*)data.data+24, sizeof(int));
            //printf("Retrieved state_8_7.OID = %d\n", state_8_7.OID);
            //fflush(stdout);
            memcpy(&(state_8_7.OID), (char*)data.data+24, sizeof(int));
            //printf("Retrieved state_8_7.OID = %d\n", state_8_7.OID);
            //fflush(stdout);
            memcpy(&(state_8_7.sum), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_8_7.sum = %d\n", state_8_7.sum);
            //fflush(stdout);
            memcpy(&(state_8_7.b), (char*)data.data+4, sizeof(double));
            //printf("Retrieved state_8_7.b = %f\n", state_8_7.b);
            //fflush(stdout);
            memcpy(&(state_8_7.c), (char*)data.data+12, sizeof(double));
            //printf("Retrieved state_8_7.c = %f\n", state_8_7.c);
            //fflush(stdout);
            memcpy(&(state_8_7.d), (char*)data.data+20, sizeof(int));
            //printf("Retrieved state_8_7.d = %d\n", state_8_7.d);
            //fflush(stdout);
            memcpy(&(state_8_7.OID), (char*)data.data+24, sizeof(int));
            //printf("Retrieved state_8_7.OID = %d\n", state_8_7.OID);
            //fflush(stdout);
            memcpy(&(state_8_7.OID), (char*)data.data+24, sizeof(int));
            //printf("Retrieved state_8_7.OID = %d\n", state_8_7.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_7 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            Q_0000_6_5.a_0 = ((state_8_7.sum) + Next);
            Q_0000_6_5.a_1 = ((state_8_7.b) + bin);
            Q_0000_6_5.a_2 = ((state_8_7.c) + cin);
            Q_0000_6_5.a_3 = ((state_8_7.d) + din);
         } /* if (rc == 0) */
         switch(index_6) {
            case 0:
            {
               if (terminating_6 == 0) {
                  if (rc==0) {
                     first_entry_6 = 0;
                     /* make assignments of non-aggr head expr */
                     /* Creating select Items */
                     /* Done with creating select Items */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_6 = (struct gb_status_6 *)0;
                     rc = hash_get(6, _rec_id, gbkey, 4, (char**)&gbstatus_6);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_6 = (struct gb_status_6*)malloc(sizeof(*gbstatus_6));
                        /* initializing UDA states */
                        /* end of initializing UDA states */
                        /* calling the aggregate */
                        /* PHASE init */
                        gbstatus_6->_baggr_0_first_entry = 1;
                        gbstatus_6->_baggr_0_value =  Q_0000_6_5.a_0;
                        gbstatus_6->_baggr_1_first_entry = 1;
                        gbstatus_6->_baggr_1_value =  Q_0000_6_5.a_1;
                        gbstatus_6->_baggr_2_first_entry = 1;
                        gbstatus_6->_baggr_2_value =  Q_0000_6_5.a_2;
                        gbstatus_6->_baggr_3_first_entry = 1;
                        gbstatus_6->_baggr_3_value =  Q_0000_6_5.a_3;
                        rc = hash_put(6, _rec_id, gbkey, 4, &gbstatus_6);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        gbstatus_6->_baggr_0_first_entry = 1;
                        if (gbstatus_6->_baggr_0_value <  Q_0000_6_5.a_0) {
                           gbstatus_6->_baggr_0_value =  Q_0000_6_5.a_0;
                        }
                        gbstatus_6->_baggr_1_first_entry = 1;
                        if (gbstatus_6->_baggr_1_value <  Q_0000_6_5.a_1) {
                           gbstatus_6->_baggr_1_value =  Q_0000_6_5.a_1;
                        }
                        gbstatus_6->_baggr_2_first_entry = 1;
                        if (gbstatus_6->_baggr_2_value <  Q_0000_6_5.a_2) {
                           gbstatus_6->_baggr_2_value =  Q_0000_6_5.a_2;
                        }
                        gbstatus_6->_baggr_3_first_entry = 1;
                        if (gbstatus_6->_baggr_3_value <  Q_0000_6_5.a_3) {
                           gbstatus_6->_baggr_3_value =  Q_0000_6_5.a_3;
                        }
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
               rc = DB_NOTFOUND;
               if (terminating_6 == 1) {
                  if (gbstatus_6->_baggr_0_first_entry == 1) {
                     Q_0001_4_3.a_0 = gbstatus_6->_baggr_0_value;
                     gbstatus_6->_baggr_0_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_6->_baggr_0_first_entry = 1;
                  }
               }
            }
            break;
            case 2:
            {
               rc = DB_NOTFOUND;
               if (terminating_6 == 1) {
                  if (gbstatus_6->_baggr_1_first_entry == 1) {
                     Q_0001_4_3.a_1 = gbstatus_6->_baggr_1_value;
                     gbstatus_6->_baggr_1_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_6->_baggr_1_first_entry = 1;
                  }
               }
            }
            break;
            case 3:
            {
               rc = DB_NOTFOUND;
               if (terminating_6 == 1) {
                  if (gbstatus_6->_baggr_2_first_entry == 1) {
                     Q_0001_4_3.a_2 = gbstatus_6->_baggr_2_value;
                     gbstatus_6->_baggr_2_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_6->_baggr_2_first_entry = 1;
                  }
               }
            }
            break;
            case 4:
            {
               rc = DB_NOTFOUND;
               if (terminating_6 == 1) {
                  if (gbstatus_6->_baggr_3_first_entry == 1) {
                     Q_0001_4_3.a_3 = gbstatus_6->_baggr_3_value;
                     gbstatus_6->_baggr_3_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_6->_baggr_3_first_entry = 1;
                  }
               }
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
               rc = DB_NOTFOUND;
            }
         }
         // start -- this was added to make nested aggregates not be stuck in an infinite loop
         if (terminating_6 == 0) {
            index_6 = 0;
         }
         else if (terminating_6 == 1) {
            if (index_6 == 5 && rc == 0) {
               index_6 = -1;
               rc = DB_NOTFOUND;
               free(gbstatus_6);
            }
         }
         // end --this was added to make nested aggregates not be stuck in an infinite loop
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
      // start -- this is a hack to make nested aggregates work 
       rc=0;
       // end -- this is a hack...
      
      if (rc==0) {
         insert_2.a_0 = Q_0001_4_3.a_0;
         insert_2.a_1 = Q_0001_4_3.a_1;
         insert_2.a_2 = Q_0001_4_3.a_2;
         insert_2.a_3 = Q_0001_4_3.a_3;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_2.a_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_2.a_1), sizeof(double));
         memcpy((char*)data.data+12, &(insert_2.a_2), sizeof(double));
         memcpy((char*)data.data+20, &(insert_2.a_3), sizeof(int));
         data.size = 24;
         key.size = 0;
         insertTempInMem(9, _rec_id, &key, &data);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
       // start -- this was added to make nested aggregates work
      rc = 1;
       // end -- this was added to make nested aggregates work
   } /* while (rc==0)*/
   mvTempInMem(9, _rec_id, status->state);
   if (state_8 && (rc = state_8->c_close(state_8)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void aggregatetesta_terminate(struct aggregatetesta_status *status, 
	int Next, double bin, double cin, int din, int _rec_id, int not_delete, bufferMngr* bm, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables, 
	vector<A_timeexp>* plist, int endSlide, char* _modelId, char* data_all_row, char* data_schema)
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
   int rlast_out = 0;//
   struct {
      double a_0;
      double a_0_expire;
      struct timeval atime;
   } insert_10;
   char* insert_10_schema = 
   	"insert_10\n"
   	"a_0 Double\n";
   IM_RELC *state_16;
   int first_entry_15 = 1;
   int first_entry_16 = 1;
   int index_14 = 0;
   int terminating_14=0;
   struct gb_status_14 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int max_0_last_out;
      bool max_0_iterate;
      bool max_0_init;
   };
   struct gb_status_14 *gbstatus_14 = (struct gb_status_14 *)0;
   
   int first_entry_14 = 1;
   int first_entry_12 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY 2 */;
   if ((rc = status->state->cursor(status->state, &state_16, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) { 
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0003_12_11;
      char* Q_0003_12_11_schema = 
      	"Q_0003_12_11\n"
      	"a_0 INT\n";
      next_12:
      struct {
         double a_0;
         double a_0_expire;
         struct timeval atime;
      } Q_0002_14_13;
      char* Q_0002_14_13_schema = 
      	"Q_0002_14_13\n"
      	"a_0 Double\n";
      /* The inner while */
      next_14:
      while (index_14>=0 && index_14 < 2) {
         /* get source tuple from qun */
         struct {
            int sum;
            double b;
            double c;
            int d;
            int OID;
            int sum_expire;
            double b_expire;
            double c_expire;
            int d_expire;
            int OID_expire;
            struct timeval atime;
         } state_16_15;
         char* state_16_15_schema = 
         	"state_16_15\n"
         	"sum INT\n"
         	"b Double\n"
         	"c Double\n"
         	"d INT\n"
         	"OID INT\n";
         next_16:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_16->c_get(state_16, &key, &data, (first_entry_15)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_15 = 0;
            memcpy(&(state_16_15.sum), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_16_15.sum = %d\n", state_16_15.sum);
            //fflush(stdout);
            memcpy(&(state_16_15.b), (char*)data.data+4, sizeof(double));
            //printf("Retrieved state_16_15.b = %f\n", state_16_15.b);
            //fflush(stdout);
            memcpy(&(state_16_15.c), (char*)data.data+12, sizeof(double));
            //printf("Retrieved state_16_15.c = %f\n", state_16_15.c);
            //fflush(stdout);
            memcpy(&(state_16_15.d), (char*)data.data+20, sizeof(int));
            //printf("Retrieved state_16_15.d = %d\n", state_16_15.d);
            //fflush(stdout);
            memcpy(&(state_16_15.OID), (char*)data.data+24, sizeof(int));
            //printf("Retrieved state_16_15.OID = %d\n", state_16_15.OID);
            //fflush(stdout);
            memcpy(&(state_16_15.OID), (char*)data.data+24, sizeof(int));
            //printf("Retrieved state_16_15.OID = %d\n", state_16_15.OID);
            //fflush(stdout);
            memcpy(&(state_16_15.sum), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_16_15.sum = %d\n", state_16_15.sum);
            //fflush(stdout);
            memcpy(&(state_16_15.b), (char*)data.data+4, sizeof(double));
            //printf("Retrieved state_16_15.b = %f\n", state_16_15.b);
            //fflush(stdout);
            memcpy(&(state_16_15.c), (char*)data.data+12, sizeof(double));
            //printf("Retrieved state_16_15.c = %f\n", state_16_15.c);
            //fflush(stdout);
            memcpy(&(state_16_15.d), (char*)data.data+20, sizeof(int));
            //printf("Retrieved state_16_15.d = %d\n", state_16_15.d);
            //fflush(stdout);
            memcpy(&(state_16_15.OID), (char*)data.data+24, sizeof(int));
            //printf("Retrieved state_16_15.OID = %d\n", state_16_15.OID);
            //fflush(stdout);
            memcpy(&(state_16_15.OID), (char*)data.data+24, sizeof(int));
            //printf("Retrieved state_16_15.OID = %d\n", state_16_15.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_15 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            Q_0002_14_13.a_0 = state_16_15.b;
         } /* if (rc == 0) */
         switch(index_14) {
            case 0:
            {
               if (terminating_14 == 0) {
                  if (rc==0) {
                     first_entry_14 = 0;
                     /* make assignments of non-aggr head expr */
                     /* Creating select Items */
                     /* Done with creating select Items */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_14 = (struct gb_status_14 *)0;
                     rc = hash_get(14, _rec_id, gbkey, 4, (char**)&gbstatus_14);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_14 = (struct gb_status_14*)malloc(sizeof(*gbstatus_14));
                        /* initializing UDA states */
                        /* end of initializing UDA states */
                        /* calling the aggregate */
                        /* PHASE init */
                        gbstatus_14->_baggr_0_first_entry = 1;
                        gbstatus_14->_baggr_0_value =  Q_0002_14_13.a_0;
                        rc = hash_put(14, _rec_id, gbkey, 4, &gbstatus_14);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        gbstatus_14->_baggr_0_first_entry = 1;
                        if (gbstatus_14->_baggr_0_value <  Q_0002_14_13.a_0) {
                           gbstatus_14->_baggr_0_value =  Q_0002_14_13.a_0;
                        }
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_14 = 1;
                  }
               }
               if (terminating_14 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(14, _rec_id, allkey, 4, (char**)&gbstatus_14);
                  if (rc==0) {
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               rc = DB_NOTFOUND;
               if (terminating_14 == 1) {
                  if (gbstatus_14->_baggr_0_first_entry == 1) {
                     Q_0003_12_11.a_0 = gbstatus_14->_baggr_0_value;
                     gbstatus_14->_baggr_0_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_14->_baggr_0_first_entry = 1;
                  }
               }
               first_entry_14 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_14++;
         }
         if (rc == DB_NOTFOUND) {
            index_14--;
            if (terminating_14 == 1 && index_14 == 0) {
               rc = DB_NOTFOUND;
            }
         }
         // start -- this was added to make nested aggregates not be stuck in an infinite loop
         if (terminating_14 == 0) {
            index_14 = 0;
         }
         else if (terminating_14 == 1) {
            if (index_14 == 2 && rc == 0) {
               index_14 = -1;
               rc = DB_NOTFOUND;
               free(gbstatus_14);
            }
         }
         // end --this was added to make nested aggregates not be stuck in an infinite loop
      }/*end of while */
      if (rc == 0) index_14--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_14 = 0;
         first_entry_14 = 1;
         index_14 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(14, _rec_id, allkey, 4, (char**)&gbstatus_14);
            if (rc==0) {
               //printf("freeing 14\n");
               free(gbstatus_14);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(14, _rec_id);
      }
      // start -- this is a hack to make nested aggregates work 
       rc=0;
       // end -- this is a hack...
      
      if (rc==0) {
         insert_10.a_0 = Q_0003_12_11.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_10.a_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_10.a_0), sizeof(double));
         data.size = 8;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
       // start -- this was added to make nested aggregates work
      rc = 1;
       // end -- this was added to make nested aggregates work
   } /* while (rc==0)*/
   if (state_16 && (rc = state_16->c_close(state_16)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   sprintf(_adl_dbname, "._%d_state", status);
   if(!not_delete) {
      if ((rc = status->state->close(status->state, 0)) != 0) {
         adlabort(rc, "IM_REL->close()");
      }
   }
   if(!not_delete) status->retc_first_entry=1;
}
void getResult(struct aggregatetesta_status status, double *result) {
   DBT key, data;
   int rc = 0;
   //double returnVal = 0;
   memset(&key, 0, sizeof(key));
   memset(&data, 0, sizeof(data));
   rc = status.retc->c_get(status.retc, &key, &data, DB_LAST); // we get DB_LAST because in window mode, we only want to get the latest result
   
   if (rc ==0) {
      memcpy((double*)result, (char*)data.data+0, data.size);
      //*result = returnVal;
   }
   else
   *result = NULL;
}

typedef struct {
   struct aggregatetesta_status aggregatetesta_0;
   double resultAccumulator;
   int offset;
   char inputMemory[60000];
} AgrStorage;

void aggregatetesta_mergeTables(AgrStorage* s1, AgrStorage* s2) {
   int localOffset = 0;
   while (localOffset  < s2->offset) {
      aggregatetesta_iterate(&s1->aggregatetesta_0, *(int*)&s2->inputMemory[localOffset],*(double*)&s2->inputMemory[localOffset+sizeof(int)],*(double*)&s2->inputMemory[localOffset+sizeof(double)],*(int*)&s2->inputMemory[localOffset+sizeof(double)], 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      localOffset += sizeof(int)+sizeof(double)+sizeof(double)+sizeof(int);
      
   }
}

void aggregatetesta_reconstructS1(AgrStorage* s2, int Next, double bin, double cin, int din) { // ***NOTE: s2 is really s1. The reason for confusion is to save space in the compiler...
   aggregatetesta_init(&s2->aggregatetesta_0, Next,bin,cin,din, 1, 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
   int localOffset = 0;
   while (localOffset  < s2->offset) {
      aggregatetesta_iterate(&s2->aggregatetesta_0, *(int*)&s2->inputMemory[localOffset],*(double*)&s2->inputMemory[localOffset+sizeof(int)],*(double*)&s2->inputMemory[localOffset+sizeof(double)],*(int*)&s2->inputMemory[localOffset+sizeof(double)], 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      localOffset += sizeof(int)+sizeof(double)+sizeof(double)+sizeof(int);
      
   }
}

extern "C" void aggregatetesta(FNC_Phase phase, FNC_Context_t *fctx, int *Next, double *bin, double *cin, int *din,double *result, int *i_Next,int *i_bin,int *i_cin,int *i_din,int *i_result, char sqlstate[6], SQL_TEXT fncname[129], SQL_TEXT sfncname[129],SQL_TEXT errorMsg[257]);
extern "C" void aggregatetesta(FNC_Phase phase, FNC_Context_t *fctx, int *Next, double *bin, double *cin, int *din,double *result, int *i_Next,int *i_bin,int *i_cin,int *i_din,int *i_result, char sqlstate[6], SQL_TEXT fncname[129], SQL_TEXT sfncname[129],SQL_TEXT errorMsg[257]){
   AgrStorage *s1 = (AgrStorage*)fctx->interim1;
   AgrStorage *s2 = (AgrStorage*)fctx->interim2;
   switch (phase) {
      	case AGR_INIT:
      		if ((s1 = (AgrStorage*)FNC_DefMem(sizeof(AgrStorage))) == NULL) {
         			strcpy(sqlstate, "U0001"); 
         			return; 
         		} 
      		
      		aggregatetesta_init(&s1->aggregatetesta_0, *Next,*bin,*cin,*din, 1, 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL); 
      		s1->resultAccumulator = 0;
      		s1->offset = 0;
       	case AGR_DETAIL:
      		aggregatetesta_iterate(&s1->aggregatetesta_0, *Next,*bin,*cin,*din, 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      		memcpy(&s1->inputMemory[s1->offset],Next,sizeof(int));
      		s1->offset += sizeof(int);
      		memcpy(&s1->inputMemory[s1->offset],bin,sizeof(double));
      		s1->offset += sizeof(double);
      		memcpy(&s1->inputMemory[s1->offset],cin,sizeof(double));
      		s1->offset += sizeof(double);
      		memcpy(&s1->inputMemory[s1->offset],din,sizeof(int));
      		s1->offset += sizeof(int);
      		break;
      	case AGR_COMBINE:
      		aggregatetesta_reconstructS1(s1, *Next,*bin,*cin,*din);
      		aggregatetesta_mergeTables(s1,s2);
      		break;
      	case AGR_FINAL:
      		aggregatetesta_terminate(&s1->aggregatetesta_0, *Next,*bin,*cin,*din, 1,0 , NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      		getResult(s1->aggregatetesta_0, result/*&s1->resultAccumulator*/);
      		//*result = s1->resultAccumulator;
      		s1->aggregatetesta_0.ret->close(s1->aggregatetesta_0.ret,0);
      		s1->aggregatetesta_0.retc->c_close(s1->aggregatetesta_0.retc);
      		delete(s1->aggregatetesta_0.win);
      		free(&s1->aggregatetesta_0);
      		free(s1);
      		break;
      	case AGR_NODATA:
      		break;
      	default:
      		strcpy(sqlstate, "U0005");
      	}
   return;
}
