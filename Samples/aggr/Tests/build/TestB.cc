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
struct aggregatetestb_status {
   IM_REL *state;
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};

extern "C" void aggregatetestb_init(struct aggregatetestb_status *status, 
	int Next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL, char* data_all_row=NULL, char* data_schema=NULL);
extern "C" void aggregatetestb_iterate(struct aggregatetestb_status *status, 
	int Next, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL, char* data_all_row=NULL, char* data_schema=NULL);
extern "C" void aggregatetestb_terminate(struct aggregatetestb_status *status, 
	int Next, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL, char* data_all_row=NULL, char* data_schema=NULL);
extern "C" void aggregatetestb_init(struct aggregatetestb_status *status, int Next,
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
      int field_0_expire;
      struct timeval atime;
   } insert_0;
   char* insert_0_schema = 
   	"insert_0\n"
   	"field_0 INT\n";
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

extern "C" void aggregatetestb_iterate(struct aggregatetestb_status *status, 
	int Next, int _rec_id, bufferMngr* bm, 
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
      int field_0;
      int field_0_expire;
      struct timeval atime;
   } insert_2;
   char* insert_2_schema = 
   	"insert_2\n"
   	"field_0 INT\n";
   IM_RELC *state_4;
   int first_entry_3 = 1;
   int first_entry_4 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY 2 */;
   if ((rc = status->state->cursor(status->state, &state_4, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) { 
      struct {
         int sum;
         int OID;
         int sum_expire;
         int OID_expire;
         struct timeval atime;
      } state_4_3;
      char* state_4_3_schema = 
      	"state_4_3\n"
      	"sum INT\n"
      	"OID INT\n";
      next_4:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = state_4->c_get(state_4, &key, &data, (first_entry_3)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_3 = 0;
         memcpy(&(state_4_3.sum), (char*)data.data+0, sizeof(int));
         //printf("Retrieved state_4_3.sum = %d\n", state_4_3.sum);
         //fflush(stdout);
         memcpy(&(state_4_3.OID), (char*)data.data+4, sizeof(int));
         //printf("Retrieved state_4_3.OID = %d\n", state_4_3.OID);
         //fflush(stdout);
         memcpy(&(state_4_3.OID), (char*)data.data+4, sizeof(int));
         //printf("Retrieved state_4_3.OID = %d\n", state_4_3.OID);
         //fflush(stdout);
         memcpy(&(state_4_3.sum), (char*)data.data+0, sizeof(int));
         //printf("Retrieved state_4_3.sum = %d\n", state_4_3.sum);
         //fflush(stdout);
         memcpy(&(state_4_3.OID), (char*)data.data+4, sizeof(int));
         //printf("Retrieved state_4_3.OID = %d\n", state_4_3.OID);
         //fflush(stdout);
         memcpy(&(state_4_3.OID), (char*)data.data+4, sizeof(int));
         //printf("Retrieved state_4_3.OID = %d\n", state_4_3.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_3 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_2.field_0 = ((state_4_3.sum) + Next);
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_2.field_0), sizeof(int));
         data.size = 4;
         key.size = 0;
         insertTempInMem(5, _rec_id, &key, &data);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0)*/
   mvTempInMem(5, _rec_id, status->state);
   if (state_4 && (rc = state_4->c_close(state_4)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void aggregatetestb_terminate(struct aggregatetestb_status *status, 
	int Next, int _rec_id, int not_delete, bufferMngr* bm, 
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
   } insert_6;
   char* insert_6_schema = 
   	"insert_6\n"
   	"a_0 Double\n";
   IM_RELC *state_12;
   int first_entry_11 = 1;
   int first_entry_12 = 1;
   int index_10 = 0;
   int terminating_10=0;
   struct gb_status_10 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int max_0_last_out;
      bool max_0_iterate;
      bool max_0_init;
   };
   struct gb_status_10 *gbstatus_10 = (struct gb_status_10 *)0;
   
   int first_entry_10 = 1;
   int first_entry_8 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY 2 */;
   if ((rc = status->state->cursor(status->state, &state_12, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) { 
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0001_8_7;
      char* Q_0001_8_7_schema = 
      	"Q_0001_8_7\n"
      	"a_0 INT\n";
      next_8:
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0000_10_9;
      char* Q_0000_10_9_schema = 
      	"Q_0000_10_9\n"
      	"a_0 INT\n";
      /* The inner while */
      next_10:
      while (index_10>=0 && index_10 < 2) {
         /* get source tuple from qun */
         struct {
            int sum;
            int OID;
            int sum_expire;
            int OID_expire;
            struct timeval atime;
         } state_12_11;
         char* state_12_11_schema = 
         	"state_12_11\n"
         	"sum INT\n"
         	"OID INT\n";
         next_12:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_12->c_get(state_12, &key, &data, (first_entry_11)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_11 = 0;
            memcpy(&(state_12_11.sum), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_12_11.sum = %d\n", state_12_11.sum);
            //fflush(stdout);
            memcpy(&(state_12_11.OID), (char*)data.data+4, sizeof(int));
            //printf("Retrieved state_12_11.OID = %d\n", state_12_11.OID);
            //fflush(stdout);
            memcpy(&(state_12_11.OID), (char*)data.data+4, sizeof(int));
            //printf("Retrieved state_12_11.OID = %d\n", state_12_11.OID);
            //fflush(stdout);
            memcpy(&(state_12_11.sum), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_12_11.sum = %d\n", state_12_11.sum);
            //fflush(stdout);
            memcpy(&(state_12_11.OID), (char*)data.data+4, sizeof(int));
            //printf("Retrieved state_12_11.OID = %d\n", state_12_11.OID);
            //fflush(stdout);
            memcpy(&(state_12_11.OID), (char*)data.data+4, sizeof(int));
            //printf("Retrieved state_12_11.OID = %d\n", state_12_11.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_11 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            Q_0000_10_9.a_0 = state_12_11.sum;
         } /* if (rc == 0) */
         switch(index_10) {
            case 0:
            {
               if (terminating_10 == 0) {
                  if (rc==0) {
                     first_entry_10 = 0;
                     /* make assignments of non-aggr head expr */
                     /* Creating select Items */
                     /* Done with creating select Items */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_10 = (struct gb_status_10 *)0;
                     rc = hash_get(10, _rec_id, gbkey, 4, (char**)&gbstatus_10);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_10 = (struct gb_status_10*)malloc(sizeof(*gbstatus_10));
                        /* initializing UDA states */
                        /* end of initializing UDA states */
                        /* calling the aggregate */
                        /* PHASE init */
                        gbstatus_10->_baggr_0_first_entry = 1;
                        gbstatus_10->_baggr_0_value =  Q_0000_10_9.a_0;
                        rc = hash_put(10, _rec_id, gbkey, 4, &gbstatus_10);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        gbstatus_10->_baggr_0_first_entry = 1;
                        if (gbstatus_10->_baggr_0_value <  Q_0000_10_9.a_0) {
                           gbstatus_10->_baggr_0_value =  Q_0000_10_9.a_0;
                        }
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_10 = 1;
                  }
               }
               if (terminating_10 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(10, _rec_id, allkey, 4, (char**)&gbstatus_10);
                  if (rc==0) {
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               rc = DB_NOTFOUND;
               if (terminating_10 == 1) {
                  if (gbstatus_10->_baggr_0_first_entry == 1) {
                     Q_0001_8_7.a_0 = gbstatus_10->_baggr_0_value;
                     gbstatus_10->_baggr_0_first_entry = 0;
                     rc = 0;
                  } else {
                     gbstatus_10->_baggr_0_first_entry = 1;
                  }
               }
               first_entry_10 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_10++;
         }
         if (rc == DB_NOTFOUND) {
            index_10--;
            if (terminating_10 == 1 && index_10 == 0) {
               rc = DB_NOTFOUND;
            }
         }
         // start -- this was added to make nested aggregates not be stuck in an infinite loop
         if (terminating_10 == 0) {
            index_10 = 0;
         }
         else if (terminating_10 == 1) {
            if (index_10 == 2 && rc == 0) {
               index_10 = -1;
               rc = DB_NOTFOUND;
               free(gbstatus_10);
            }
         }
         // end --this was added to make nested aggregates not be stuck in an infinite loop
      }/*end of while */
      if (rc == 0) index_10--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_10 = 0;
         first_entry_10 = 1;
         index_10 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(10, _rec_id, allkey, 4, (char**)&gbstatus_10);
            if (rc==0) {
               //printf("freeing 10\n");
               free(gbstatus_10);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(10, _rec_id);
      }
      // start -- this is a hack to make nested aggregates work 
       rc=0;
       // end -- this is a hack...
      
      if (rc==0) {
         insert_6.a_0 = Q_0001_8_7.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_6.a_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_6.a_0), sizeof(double));
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
   if (state_12 && (rc = state_12->c_close(state_12)) != 0) {
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
void getResult(struct aggregatetestb_status status, double *result) {
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
   struct aggregatetestb_status aggregatetestb_0;
   double resultAccumulator;
   int offset;
   char inputMemory[60000];
} AgrStorage;

void aggregatetestb_mergeTables(AgrStorage* s1, AgrStorage* s2) {
   int localOffset = 0;
   while (localOffset  < s2->offset) {
      aggregatetestb_iterate(&s1->aggregatetestb_0, *(int*)&s2->inputMemory[localOffset], 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      localOffset += sizeof(int);
      
   }
}

void aggregatetestb_reconstructS1(AgrStorage* s2, int Next) { // ***NOTE: s2 is really s1. The reason for confusion is to save space in the compiler...
   aggregatetestb_init(&s2->aggregatetestb_0, Next, 1, 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
   int localOffset = 0;
   while (localOffset  < s2->offset) {
      aggregatetestb_iterate(&s2->aggregatetestb_0, *(int*)&s2->inputMemory[localOffset], 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      localOffset += sizeof(int);
      
   }
}

extern "C" void aggregatetestb(FNC_Phase phase, FNC_Context_t *fctx, int *Next,double *result, int *i_Next,int *i_result, char sqlstate[6], SQL_TEXT fncname[129], SQL_TEXT sfncname[129],SQL_TEXT errorMsg[257]);
extern "C" void aggregatetestb(FNC_Phase phase, FNC_Context_t *fctx, int *Next,double *result, int *i_Next,int *i_result, char sqlstate[6], SQL_TEXT fncname[129], SQL_TEXT sfncname[129],SQL_TEXT errorMsg[257]){
   AgrStorage *s1 = (AgrStorage*)fctx->interim1;
   AgrStorage *s2 = (AgrStorage*)fctx->interim2;
   switch (phase) {
      	case AGR_INIT:
      		if ((s1 = (AgrStorage*)FNC_DefMem(sizeof(AgrStorage))) == NULL) {
         			strcpy(sqlstate, "U0001"); 
         			return; 
         		} 
      		
      		aggregatetestb_init(&s1->aggregatetestb_0, *Next, 1, 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL); 
      		s1->resultAccumulator = 0;
      		s1->offset = 0;
       	case AGR_DETAIL:
      		aggregatetestb_iterate(&s1->aggregatetestb_0, *Next, 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      		memcpy(&s1->inputMemory[s1->offset],Next,sizeof(int));
      		s1->offset += sizeof(int);
      		break;
      	case AGR_COMBINE:
      		aggregatetestb_reconstructS1(s1, *Next);
      		aggregatetestb_mergeTables(s1,s2);
      		break;
      	case AGR_FINAL:
      		aggregatetestb_terminate(&s1->aggregatetestb_0, *Next, 1,0 , NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      		getResult(s1->aggregatetestb_0, result/*&s1->resultAccumulator*/);
      		//*result = s1->resultAccumulator;
      		s1->aggregatetestb_0.ret->close(s1->aggregatetestb_0.ret,0);
      		s1->aggregatetestb_0.retc->c_close(s1->aggregatetestb_0.retc);
      		delete(s1->aggregatetestb_0.win);
      		free(&s1->aggregatetestb_0);
      		free(s1);
      		break;
      	case AGR_NODATA:
      		break;
      	default:
      		strcpy(sqlstate, "U0005");
      	}
   return;
}
