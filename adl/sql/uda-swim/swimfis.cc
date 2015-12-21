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
		const int MAX_FREQ_ITEM_LEN = 100; // should change the singature too.
	/* global c stuff here */
	
struct swimfis_status {
     	/* things to put in the status structure here */
   		// config variables
   		SwimFisStatus sfs;		
   	
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void swimfis_init(struct swimfis_status *status, 
	int tid, struct iExt_ cur_trans, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void swimfis_iterate(struct swimfis_status *status, 
	int tid, struct iExt_ cur_trans, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void swimfis_terminate(struct swimfis_status *status, 
	int tid, struct iExt_ cur_trans, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL);
extern "C" void swimfis_init(struct swimfis_status *status, int tid, struct iExt_ cur_trans,
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
   		char tabName[20];
   		IM_RELC * paramTable;
   		char param_name[MAX_FREQ_ITEM_LEN+1];
   		char param_value[MAX_FREQ_ITEM_LEN+1];
   		int argv[6] = {0,0,0,0,0,0};
   		int first_entry = 1;
   
   		status->sfs.trans_avg_len = 0;
   		status->sfs.no_items = 0;
   		status->sfs.global_min_supp = 0;
   		status->sfs.slide_min_supp = 0;
   		status->sfs.window_size = 0; 
   		status->sfs.slide_size = 0;
   		status->sfs.L_delay_max = 0;
   		status->sfs.no_slides = 0;
   		status->sfs.sum_all_pat_lens = 0;
   		status->sfs.max_needed_mem = 0;
   		
   		status->sfs.StreamMillBuffer = (int *)malloc(MAX_FREQ_ITEM_LEN*sizeof(int)*status->sfs.slide_size);
   		status->sfs.StreamMillPtr = 0;
   
   //create table swimparams(param_name char(100), param_value char(100)) memory;
   		sprintf(tabName, "swimparams"); 
   		if ((rc = ((IM_REL*)inMemTables->operator[](tabName))->cursor(((IM_REL*)inMemTables->operator[](tabName)), &paramTable, 0)) != 0) {
      			adlabortESLAggr(bm->lookup("hetal_errors"), rc, "\nError in Aggregate hetal__computeavg: IM_REL->cursor()");
      			return;
      		}
   		
   		while (rc==0) {
      			memset(&key, 0, sizeof(key));
      			memset(&data, 0, sizeof(data));
      			rc = paramTable->c_get(paramTable, &key, &data, (first_entry)? DB_FIRST:DB_NEXT);
      			if (rc==0) {
         				first_entry = 0;
         				memcpy(param_name, (char*)data.data, 100);
         				param_name[100] = '\0';
         				memcpy(param_value, (char*)data.data+100, 100);
         				param_value[100] = '\0';
         				//TODO: Now use these pairs to fill up the configuration parameters.
         				if (!strcmp(param_name, "trans_avg_len")) {
            					status->sfs.trans_avg_len = atoi(param_value);
            					argv[0] = 1;
            				}	else if (!strcmp(param_name, "window_size")) {
            					status->sfs.window_size = atoi(param_value);
            					argv[1] = 1;
            				}	else if (!strcmp(param_name, "slide_size")) {
            					status->sfs.slide_size = atoi(param_value);
            					argv[2] = 1;	
            				}	else if (!strcmp(param_name, "L_delay_max")) {
            					status->sfs.L_delay_max = atoi(param_value);
            					argv[3] = 1;
            				}	else if (!strcmp(param_name, "no_items")) {
            					status->sfs.no_items = atoi(param_value);
            					argv[4] = 1;
            				}	else if (!strcmp(param_name, "min_supp_10000")) {
            					status->sfs.global_min_supp = atoi(param_value);
            					status->sfs.slide_min_supp = atoi(param_value);
            					argv[5] = 1;
            				}
         			} 
      
      		} /* while (rc==0) */
   //Initialization starts
   	for (int i=0; i<6 ; ++i)
   		if (!argv[i]){
      			fprintf(stderr,"Too few parameters. Agrv[%d] missing. ",i);
      			return; //return -1000;// should be probably replaced with a fail() function.
      		}
   				
   	status->sfs.global_min_supp = ceil(status->sfs.global_min_supp * status->sfs.window_size / 10000.0);
   	status->sfs.slide_min_supp = ceil(status->sfs.slide_min_supp * status->sfs.slide_size / 10000.0);
   
   	fprintf(stderr,"# status->sfs.global_min_supp=%d,status->sfs.slide_min_supp=%d,status->sfs.window_size=%d,status->sfs.slide_size=%d,status->sfs.L_delay_max=%d\n",status->sfs.global_min_supp,status->sfs.slide_min_supp,status->sfs.window_size,status->sfs.slide_size,status->sfs.L_delay_max);
   
   	status->sfs.no_slides = status->sfs.window_size / status->sfs.slide_size;
   	if (status->sfs.window_size != (status->sfs.no_slides*status->sfs.slide_size) || status->sfs.L_delay_max < 0 || status->sfs.L_delay_max >= status->sfs.no_slides){
      		fprintf(stderr,"Error:status->sfs.slide_size must be a divisor of status->sfs.window_size.\n");
      		return; //return -1000;	
      	}
   	
   //Allocating memories and initialization	
   	status->sfs.trans.items = (int*)malloc(sizeof(int)*status->sfs.no_items);
   	status->sfs.archive_fpos = (long*)malloc(sizeof(long)*(status->sfs.no_slides+1));
   	status->sfs.localFPs = (Tree **)malloc((status->sfs.no_slides+1)*sizeof(Tree *));
   	for (int i=0; i<=status->sfs.no_slides; i++)
   		status->sfs.localFPs[i] = new_tree(status->sfs.no_items);
   	status->sfs.local_all_trans_len = (int *)malloc((status->sfs.no_slides+1)*sizeof(int));
   //itemName is what we read/write from/in the input/output
   //But itemNo is our ordering over single items
   // In SWIM implementations we assumet the single item numbers exatly like the output of IBM 
   // generator:	
   //Basically, status->sfs.order[itemName] = itemNo	
   //For the first scan, itemNo = status->sfs.order[itemName] = itemName
   //Also recall that the status->sfs.order's status->sfs.index is always between 0 and status->sfs.no_items-1
   //And that's why we do not say malloc(sizeof(int)*(status->sfs.no_items+1)
   	status->sfs.order = (int*)malloc(sizeof(int)*(status->sfs.no_items));
   	status->sfs.rev_order = (int*)malloc(sizeof(int)*(status->sfs.no_items));
   	for (int i=0; i<status->sfs.no_items; i++){
      		status->sfs.order[i] = i;
      		status->sfs.rev_order[i] = i;
      	}
   
   //Notice that 0<= itemNo <= status->sfs.no_items-1
   	status->sfs.patternTree = new_tree(status->sfs.no_items);
   	status->sfs.temp_patternTree = new_tree(status->sfs.no_items);
   
   //Initialization end
   		int windowid;
       printf("Tran %d: ", cur_trans.length);
   		for (int i=0; i<cur_trans.length; ++i)
   			printf("%d ",cur_trans.pt[i]);
   		printf("\n");
   		status->sfs.StreamMillPtr += addTuple(cur_trans.length, cur_trans.pt, status->sfs.StreamMillBuffer, status->sfs.StreamMillPtr);
   		if ( ((tid+1)%status->sfs.slide_size)==0 ){
      			windowid = tid/status->sfs.slide_size;
      			ProcessMain(windowid, &(status->sfs));
      			status->sfs.StreamMillPtr = 0;
      		}
   	
   	
   status->retc_first_entry=1;
}
extern "C" void swimfis_iterate(struct swimfis_status *status, 
	int tid, struct iExt_ cur_trans, int _rec_id, bufferMngr* bm, 
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
   		int windowid;
   
       printf("Tran %d: ", cur_trans.length);
   		for (int i=0; i<cur_trans.length; ++i)
   			printf("%d ",cur_trans.pt[i]);
   		printf("\n");
   
   		status->sfs.StreamMillPtr += addTuple(cur_trans.length, cur_trans.pt, status->sfs.StreamMillBuffer, status->sfs.StreamMillPtr);
   		
   		if ( ((tid+1)%status->sfs.slide_size)==0 ){
      			windowid = tid/status->sfs.slide_size;
      			ProcessMain(windowid, &(status->sfs));
      			status->sfs.StreamMillPtr = 0;
      		}
   	
   			
   	
   status->retc_first_entry=1;
}
extern "C" void swimfis_terminate(struct swimfis_status *status, 
	int tid, struct iExt_ cur_trans, int _rec_id, int not_delete, bufferMngr* bm, 
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
   		/*
   		cleanUp(&(status->sfs));
   		*/
   	
   if(!not_delete) status->retc_first_entry=1;
}
struct inWinType_swimfis_window {
   int tid;
   struct iExt_ cur_trans;
};
struct swimfis_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_swimfis_window* getLastTuple_swimfis_window(IM_REL* inwindow, inWinType_swimfis_window* tuple, bufferMngr* bm) {
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
   memcpy(&((*tuple).tid), (char*)data.data+0, sizeof(int));
   //printf("Retrieved (*tuple).tid = %d\n", (*tuple).tid);
   //fflush(stdout);
   memcpy(&((*tuple).cur_trans), (char*)data.data+4, sizeof(struct iExt_));
   //printf("Retrieved  iExt (*tuple).cur_trans(%d, %d)\n", (*tuple).cur_trans.length, (*tuple).cur_trans.pt[1]);
   //fflush(stdout);
   return tuple;
}
extern "C" void swimfis_window_init(struct swimfis_window_status *status, 
	int tid, struct iExt_ cur_trans, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void swimfis_window_init(struct swimfis_window_status *status, int tid, struct iExt_ cur_trans, 
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
   struct inWinType_swimfis_window tuple;
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
      char a_0[101];
      int a_1;
      char a_0_expire[101];
      int a_1_expire;
      struct timeval atime;
   } insert_0;
   IM_RELC *w_6;
   int first_entry_5 = 1;
   int first_entry_6 = 1;
   int index_4 = 0;
   int terminating_4=0;
   struct gb_status_4 {
      struct swimfis_status *swimfis_0;
   };
   struct gb_status_4 *gbstatus_4 = (struct gb_status_4 *)0;
   
   int first_entry_4 = 1;
   int first_entry_2 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_6, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         char a_0[101];
         int a_1;
         char a_0_expire[101];
         int a_1_expire;
         struct timeval atime;
      } Q_0001_2_1;
      next_2:
      struct {
         int a_0;
         struct iExt_ a_1;
         int a_0_expire;
         struct iExt_ a_1_expire;
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
                     int tid;
                     struct iExt_ cur_trans;
                     int tid_expire;
                     struct iExt_ cur_trans_expire;
                     struct timeval atime;
                  } w_6_5;
                  next_6:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_6->c_get(w_6, &key, &data, (first_entry_5)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_5 = 0;
                     memcpy(&(w_6_5.tid), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_6_5.tid = %d\n", w_6_5.tid);
                     //fflush(stdout);
                     memcpy(&(w_6_5.cur_trans), (char*)data.data+4, sizeof(struct iExt_));
                     //printf("Retrieved  iExt w_6_5.cur_trans(%d, %d)\n", w_6_5.cur_trans.length, w_6_5.cur_trans.pt[1]);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_5 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0000_4_3.a_0 = w_6_5.tid;
                     memcpy(&(Q_0000_4_3.a_1), &w_6_5.cur_trans, sizeof(struct iExt_));
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
                        gbstatus_4->swimfis_0 = (struct swimfis_status*)malloc(sizeof(struct swimfis_status));
                        gbstatus_4->swimfis_0->win = 0;
                        setModelId("");
                        swimfis_init(gbstatus_4->swimfis_0, Q_0000_4_3.a_0, Q_0000_4_3.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0000_4_3.a_1);
                        rc = hash_put(4, _rec_id, gbkey, 4, &gbstatus_4);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        swimfis_iterate(gbstatus_4->swimfis_0, Q_0000_4_3.a_0, Q_0000_4_3.a_1, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0000_4_3.a_1);
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_4 = 1;
                  }
               }
               if (terminating_4 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(4, _rec_id, allkey, 4, (char**)&gbstatus_4);
                  if (rc==0) {
                     setModelId("");
                     swimfis_terminate(gbstatus_4->swimfis_0, Q_0000_4_3.a_0, Q_0000_4_3.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_4->swimfis_0->retc->c_get(gbstatus_4->swimfis_0->retc, &key, &data, (gbstatus_4->swimfis_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_4->swimfis_0->retc_first_entry = 0;
                  memcpy(Q_0001_2_1.a_0, (char*)data.data+0, 100);
                  *(Q_0001_2_1.a_0+100) = '\0';
                  memcpy(&(Q_0001_2_1.a_1), (char*)data.data+100, sizeof(int));
                  //printf("Retrieved Q_0001_2_1.a_1 = %d\n", Q_0001_2_1.a_1);
                  //fflush(stdout);
                  if ((rc = gbstatus_4->swimfis_0->retc->c_del(gbstatus_4->swimfis_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_4->swimfis_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
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
               if (gbstatus_4->swimfis_0->retc && (rc = gbstatus_4->swimfis_0->retc->c_close(gbstatus_4->swimfis_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_4->swimfis_0);
               
               if (gbstatus_4->swimfis_0->ret && ((rc = gbstatus_4->swimfis_0->ret->close(gbstatus_4->swimfis_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_4->swimfis_0->ret = NULL;
               (void)unlink(_adl_dbname);
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
               free(gbstatus_4->swimfis_0);
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
         memcpy(insert_0.a_0, Q_0001_2_1.a_0, 100);
         insert_0.a_0[100]=0;
         insert_0.a_1 = Q_0001_2_1.a_1;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memset((char*)key.data+0, 0, 100);
         memcpy((char*)key.data+0, insert_0.a_0, 100);
         memcpy((char*)data.data+0, insert_0.a_0, 100);
         memcpy((char*)data.data+100, &(insert_0.a_1), sizeof(int));
         data.size = 104;
         key.size = 100;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_6 && (rc = w_6->c_close(w_6)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
IM_REL *swimparams;
IM_REL *tdata;
/**** Query Declarations ****/
int _adl_statement_9()
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
      struct timeval atime;
   } insert_7;
   int first_entry_8 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_8:
      rc = (first_entry_8)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_8=1;
      else {
         first_entry_8=0;
         memcpy(insert_7.field_0, "trans_avg_len", 14);
         insert_7.field_0[14]=0;
         memcpy(insert_7.field_1, "20", 3);
         insert_7.field_1[3]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_7.field_0, 100);
         memcpy((char*)data.data+100, insert_7.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, &key, &data, DB_APPEND))!=0) {
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
int _adl_statement_12()
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
      struct timeval atime;
   } insert_10;
   int first_entry_11 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_11:
      rc = (first_entry_11)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_11=1;
      else {
         first_entry_11=0;
         memcpy(insert_10.field_0, "window_size", 12);
         insert_10.field_0[12]=0;
         memcpy(insert_10.field_1, "20000", 6);
         insert_10.field_1[6]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_10.field_0, 100);
         memcpy((char*)data.data+100, insert_10.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, &key, &data, DB_APPEND))!=0) {
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
int _adl_statement_15()
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
      struct timeval atime;
   } insert_13;
   int first_entry_14 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_14:
      rc = (first_entry_14)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_14=1;
      else {
         first_entry_14=0;
         memcpy(insert_13.field_0, "slide_size", 11);
         insert_13.field_0[11]=0;
         memcpy(insert_13.field_1, "10000", 6);
         insert_13.field_1[6]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_13.field_0, 100);
         memcpy((char*)data.data+100, insert_13.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, &key, &data, DB_APPEND))!=0) {
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
int _adl_statement_18()
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
      struct timeval atime;
   } insert_16;
   int first_entry_17 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_17:
      rc = (first_entry_17)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_17=1;
      else {
         first_entry_17=0;
         memcpy(insert_16.field_0, "no_items", 9);
         insert_16.field_0[9]=0;
         memcpy(insert_16.field_1, "1000", 5);
         insert_16.field_1[5]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_16.field_0, 100);
         memcpy((char*)data.data+100, insert_16.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, &key, &data, DB_APPEND))!=0) {
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
int _adl_statement_21()
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
      struct timeval atime;
   } insert_19;
   int first_entry_20 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_20:
      rc = (first_entry_20)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_20=1;
      else {
         first_entry_20=0;
         memcpy(insert_19.field_0, "L_delay_max", 12);
         insert_19.field_0[12]=0;
         memcpy(insert_19.field_1, "1", 2);
         insert_19.field_1[2]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_19.field_0, 100);
         memcpy((char*)data.data+100, insert_19.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, &key, &data, DB_APPEND))!=0) {
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
   int rlast_out = 0;
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   struct {
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
      struct timeval atime;
   } insert_22;
   int first_entry_23 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_23:
      rc = (first_entry_23)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_23=1;
      else {
         first_entry_23=0;
         memcpy(insert_22.field_0, "min_supp_10000", 15);
         insert_22.field_0[15]=0;
         memcpy(insert_22.field_1, "100", 4);
         insert_22.field_1[4]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_22.field_0, 100);
         memcpy((char*)data.data+100, insert_22.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, &key, &data, DB_APPEND))!=0) {
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
int _adl_statement_25()
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
      FILE *_adl_load = fopen("/tmp/sigkdd1.stream", "rt");
      char _adl_load_buf[40960], *tok;
      char loadkeybuf[1], loaddatabuf[9];
      int _adl_line_no=0;
      char bPoint =0;
      if (!_adl_load) {
         printf("can not open file /tmp/sigkdd1.stream.\n");
         exit(1);
      }
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      key.data = loadkeybuf;
      data.data = loaddatabuf;
      key.size = 0;
      data.size = 8;
      while (fgets(_adl_load_buf, 40959, _adl_load)) {
         _adl_line_no++;
         tok = csvtok(_adl_load_buf, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(int*)((char*)data.data+0) = atoi(tok);
         if ((rc = tdata->put(tdata, &key, &data, DB_APPEND))!=0) {
            exit(rc);
         }
      } /* end of while */
      fclose(_adl_load);
   }
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_34()
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
      char a_0[101];
      int a_1;
      char a_0_expire[101];
      int a_1_expire;
      struct timeval atime;
   } insert_26;
   IM_RELC *tdata_32;
   int first_entry_31 = 1;
   int first_entry_33 = 1;
   int index_32 = 0;
   int first_entry_32 = 1;
   int index_30 = 0;
   int terminating_30=0;
   struct gb_status_30 {
      struct swimfis_status *swimfis_0;
   };
   struct gb_status_30 *gbstatus_30 = (struct gb_status_30 *)0;
   
   int first_entry_30 = 1;
   int first_entry_28 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = tdata->cursor(tdata, &tdata_32, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         char a_0[101];
         int a_1;
         char a_0_expire[101];
         int a_1_expire;
         struct timeval atime;
      } Q_0003_28_27;
      next_28:
      struct {
         int a_0;
         struct iExt_ a_1;
         int a_0_expire;
         struct iExt_ a_1_expire;
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
                     int it;
                     int OID;
                     int it_expire;
                     int OID_expire;
                     struct timeval atime;
                  } tdata_32_31;
                  struct {
                     int tid;
                     struct iExt_ val;
                     int tid_expire;
                     struct iExt_ val_expire;
                     struct timeval atime;
                  } t_32_33;
                  next_32:
                  while (index_32>=0 && index_32 < 2) { 
                     switch(index_32) {
                        case 0:
                        {
                           memset(&key, 0, sizeof(key));
                           memset(&data, 0, sizeof(data));
                           rc = tdata_32->c_get(tdata_32, &key, &data, (first_entry_31)? DB_FIRST:DB_NEXT);
                           if (rc==0) {
                              first_entry_31 = 0;
                              memcpy(&(tdata_32_31.it), (char*)data.data+0, sizeof(int));
                              //printf("Retrieved tdata_32_31.it = %d\n", tdata_32_31.it);
                              //fflush(stdout);
                              memcpy(&(tdata_32_31.OID), (char*)data.data+4, sizeof(int));
                              //printf("Retrieved tdata_32_31.OID = %d\n", tdata_32_31.OID);
                              //fflush(stdout);
                           } else if (rc == DB_NOTFOUND) {
                              first_entry_31 = 1;
                           } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                        }
                        break;
                        case 1:
                        {
                           rc = _buildIExt(first_entry_33, (void*)(&t_32_33), tdata_32_31.it);
                           if (rc==0) {
                              first_entry_33 = 0;
                           } else if (rc == DB_NOTFOUND) {
                              first_entry_33 = 1;
                           } else {
                              adlabort(rc, "External Table Function");
                           }
                        }
                        break;
                     } /*switch */
                     if (rc==0) {
                        index_32++;
                     } else if (rc==DB_NOTFOUND) {
                        index_32--;
                     }
                  } /* while */
                  if (rc!=0) {
                     index_32++;    /* set index to the first subgoal */
                  } else {
                     index_32--;    /* set index to the last subgoal */
                     Q_0002_30_29.a_0 = t_32_33.tid;
                     memcpy(&(Q_0002_30_29.a_1), &t_32_33.val, sizeof(struct iExt_));
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
                        gbstatus_30->swimfis_0 = (struct swimfis_status*)malloc(sizeof(struct swimfis_status));
                        gbstatus_30->swimfis_0->win = 0;
                        setModelId("");
                        swimfis_init(gbstatus_30->swimfis_0, Q_0002_30_29.a_0, Q_0002_30_29.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0002_30_29.a_1);
                        rc = hash_put(30, _rec_id, gbkey, 4, &gbstatus_30);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        swimfis_iterate(gbstatus_30->swimfis_0, Q_0002_30_29.a_0, Q_0002_30_29.a_1, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0002_30_29.a_1);
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_30 = 1;
                  }
               }
               if (terminating_30 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(30, _rec_id, allkey, 4, (char**)&gbstatus_30);
                  if (rc==0) {
                     setModelId("");
                     swimfis_terminate(gbstatus_30->swimfis_0, Q_0002_30_29.a_0, Q_0002_30_29.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_30->swimfis_0->retc->c_get(gbstatus_30->swimfis_0->retc, &key, &data, (gbstatus_30->swimfis_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_30->swimfis_0->retc_first_entry = 0;
                  memcpy(Q_0003_28_27.a_0, (char*)data.data+0, 100);
                  *(Q_0003_28_27.a_0+100) = '\0';
                  memcpy(&(Q_0003_28_27.a_1), (char*)data.data+100, sizeof(int));
                  //printf("Retrieved Q_0003_28_27.a_1 = %d\n", Q_0003_28_27.a_1);
                  //fflush(stdout);
                  if ((rc = gbstatus_30->swimfis_0->retc->c_del(gbstatus_30->swimfis_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_30->swimfis_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
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
               if (gbstatus_30->swimfis_0->retc && (rc = gbstatus_30->swimfis_0->retc->c_close(gbstatus_30->swimfis_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_30->swimfis_0);
               
               if (gbstatus_30->swimfis_0->ret && ((rc = gbstatus_30->swimfis_0->ret->close(gbstatus_30->swimfis_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_30->swimfis_0->ret = NULL;
               (void)unlink(_adl_dbname);
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
               free(gbstatus_30->swimfis_0);
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
         memcpy(insert_26.a_0, Q_0003_28_27.a_0, 100);
         insert_26.a_0[100]=0;
         insert_26.a_1 = Q_0003_28_27.a_1;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("\t%s\t ", insert_26.a_0);
         printf("%10d ", insert_26.a_1);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (tdata_32 && (rc = tdata_32->c_close(tdata_32)) != 0) {
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
   if ((rc = im_rel_create(&swimparams, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = swimparams->open(swimparams, "_adl_db_swimparams", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("swimparams") == 0) {
      inMemTables->operator[](strdup("swimparams")) = swimparams;
   }
   if ((rc = im_rel_create(&tdata, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = tdata->open(tdata, "_adl_db_tdata", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("tdata") == 0) {
      inMemTables->operator[](strdup("tdata")) = tdata;
   }
   _adl_statement_9();
   _adl_statement_12();
   _adl_statement_15();
   _adl_statement_18();
   _adl_statement_21();
   _adl_statement_24();
   _adl_statement_25();
   _adl_statement_34();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = swimparams->close(swimparams, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = tdata->close(tdata, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
