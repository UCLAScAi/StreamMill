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
DB *A;
int _A_cmp(DB* dbp, const DBT *a, const DBT *b){
   	int ai, bi, ri, rs;
   	double ad, bd, rd;
   	struct timeval *at, *bt;return 0;
};
struct test_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void test_init(struct test_status *status, 
	int a, char* b, struct timeval c, double d, struct iExt_ ae, struct cExt_ be, struct tExt_ ce, struct rExt_ de, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void test_init(struct test_status *status, int a, char* b, struct timeval c, double d, struct iExt_ ae, struct cExt_ be, struct tExt_ ce, struct rExt_ de,
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
   struct {
      char field_0[13];
      int a;
      char b[11];
      struct timeval c;
      double d;
      int col;
      int val;
      char field_0_expire[13];
      int a_expire;
      char b_expire[11];
      struct timeval c_expire;
      double d_expire;
      int col_expire;
      int val_expire;
      struct timeval atime;
   } insert_0;
   int first_entry_1 = 1;
   int first_entry_2 = 1;
   struct {
      char field_0[14];
      int a;
      char b[11];
      struct timeval c;
      double d;
      int col;
      double val;
      char field_0_expire[14];
      int a_expire;
      char b_expire[11];
      struct timeval c_expire;
      double d_expire;
      int col_expire;
      double val_expire;
      struct timeval atime;
   } insert_3;
   int first_entry_4 = 1;
   int first_entry_5 = 1;
   struct {
      char field_0[14];
      int a;
      char b[11];
      struct timeval c;
      double d;
      int col;
      char val[101];
      char field_0_expire[14];
      int a_expire;
      char b_expire[11];
      struct timeval c_expire;
      double d_expire;
      int col_expire;
      char val_expire[101];
      struct timeval atime;
   } insert_6;
   int first_entry_7 = 1;
   int first_entry_8 = 1;
   struct {
      char field_0[13];
      int a;
      char b[11];
      struct timeval c;
      double d;
      int col;
      struct timeval val;
      char field_0_expire[13];
      int a_expire;
      char b_expire[11];
      struct timeval c_expire;
      double d_expire;
      int col_expire;
      struct timeval val_expire;
      struct timeval atime;
   } insert_9;
   int first_entry_10 = 1;
   int first_entry_11 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      struct {
         int col;
         int val;
         int col_expire;
         int val_expire;
         struct timeval atime;
      } X_2_1;
      next_2:
      rc = _iExtVert(first_entry_1, (void*)(&X_2_1), ae);
      if (rc==0) {
         first_entry_1 = 0;
      } else if (rc == DB_NOTFOUND) {
         first_entry_1 = 1;
      } else {
         adlabort(rc, "External Table Function");
      }
      if (rc==0) {
         memcpy(insert_0.field_0, "we got ints", 12);
         insert_0.field_0[12]=0;
         insert_0.a = a;
         memcpy(insert_0.b, b, 10);
         insert_0.b[10]=0;
         memcpy(&(insert_0.c), &c, sizeof(timestamp));
         insert_0.d = d;
         insert_0.col = X_2_1.col;
         insert_0.val = X_2_1.val;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("\t%s\t ", insert_0.field_0);
         printf("%10d ", insert_0.a);
         printf("\t%s\t ", insert_0.b);
         char ts_3[40];
         struct tm* temp_3 = localtime(&insert_0.c.tv_sec);
         strftime(ts_3, 40, "%I:%M:%S %p %D", temp_3);
         printf("\t%s.%d\t ", ts_3, insert_0.c.tv_usec);
         printf("%10f ", insert_0.d);
         printf("%4d ", insert_0.col);
         printf("%4d ", insert_0.val);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      struct {
         int col;
         double val;
         int col_expire;
         double val_expire;
         struct timeval atime;
      } X_5_4;
      next_5:
      rc = _rExtVert(first_entry_4, (void*)(&X_5_4), de);
      if (rc==0) {
         first_entry_4 = 0;
      } else if (rc == DB_NOTFOUND) {
         first_entry_4 = 1;
      } else {
         adlabort(rc, "External Table Function");
      }
      if (rc==0) {
         memcpy(insert_3.field_0, "we got reals", 13);
         insert_3.field_0[13]=0;
         insert_3.a = a;
         memcpy(insert_3.b, b, 10);
         insert_3.b[10]=0;
         memcpy(&(insert_3.c), &c, sizeof(timestamp));
         insert_3.d = d;
         insert_3.col = X_5_4.col;
         insert_3.val = X_5_4.val;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("\t%s\t ", insert_3.field_0);
         printf("%10d ", insert_3.a);
         printf("\t%s\t ", insert_3.b);
         char ts_3[40];
         struct tm* temp_3 = localtime(&insert_3.c.tv_sec);
         strftime(ts_3, 40, "%I:%M:%S %p %D", temp_3);
         printf("\t%s.%d\t ", ts_3, insert_3.c.tv_usec);
         printf("%10f ", insert_3.d);
         printf("%4d ", insert_3.col);
         printf("%4f ", insert_3.val);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      struct {
         int col;
         char val[101];
         int col_expire;
         char val_expire[101];
         struct timeval atime;
      } X_8_7;
      next_8:
      rc = _cExtVert(first_entry_7, (void*)(&X_8_7), be);
      if (rc==0) {
         first_entry_7 = 0;
      } else if (rc == DB_NOTFOUND) {
         first_entry_7 = 1;
      } else {
         adlabort(rc, "External Table Function");
      }
      if (rc==0) {
         memcpy(insert_6.field_0, "we got chars", 13);
         insert_6.field_0[13]=0;
         insert_6.a = a;
         memcpy(insert_6.b, b, 10);
         insert_6.b[10]=0;
         memcpy(&(insert_6.c), &c, sizeof(timestamp));
         insert_6.d = d;
         insert_6.col = X_8_7.col;
         memcpy(insert_6.val, X_8_7.val, 100);
         insert_6.val[100]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("\t%s\t ", insert_6.field_0);
         printf("%10d ", insert_6.a);
         printf("\t%s\t ", insert_6.b);
         char ts_3[40];
         struct tm* temp_3 = localtime(&insert_6.c.tv_sec);
         strftime(ts_3, 40, "%I:%M:%S %p %D", temp_3);
         printf("\t%s.%d\t ", ts_3, insert_6.c.tv_usec);
         printf("%10f ", insert_6.d);
         printf("%4d ", insert_6.col);
         printf("\t%s\t ", insert_6.val);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      struct {
         int col;
         struct timeval val;
         int col_expire;
         struct timeval val_expire;
         struct timeval atime;
      } X_11_10;
      next_11:
      rc = _tExtVert(first_entry_10, (void*)(&X_11_10), ce);
      if (rc==0) {
         first_entry_10 = 0;
      } else if (rc == DB_NOTFOUND) {
         first_entry_10 = 1;
      } else {
         adlabort(rc, "External Table Function");
      }
      if (rc==0) {
         memcpy(insert_9.field_0, "we got tsps", 12);
         insert_9.field_0[12]=0;
         insert_9.a = a;
         memcpy(insert_9.b, b, 10);
         insert_9.b[10]=0;
         memcpy(&(insert_9.c), &c, sizeof(timestamp));
         insert_9.d = d;
         insert_9.col = X_11_10.col;
         memcpy(&(insert_9.val), &X_11_10.val, sizeof(timestamp));
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("\t%s\t ", insert_9.field_0);
         printf("%10d ", insert_9.a);
         printf("\t%s\t ", insert_9.b);
         char ts_3[40];
         struct tm* temp_3 = localtime(&insert_9.c.tv_sec);
         strftime(ts_3, 40, "%I:%M:%S %p %D", temp_3);
         printf("\t%s.%d\t ", ts_3, insert_9.c.tv_usec);
         printf("%10f ", insert_9.d);
         printf("%4d ", insert_9.col);
         char ts_6[40];
         struct tm* temp_6 = localtime(&insert_9.val.tv_sec);
         strftime(ts_6, 40, "%I:%M:%S %p %D", temp_6);
         printf("\t%s.%d\t ", ts_6, insert_9.val.tv_usec);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
struct inWinType_test_window {
   int a;
   char* b;
   struct timeval c;
   double d;
   struct iExt_ ae;
   struct cExt_ be;
   struct tExt_ ce;
   struct rExt_ de;
};
struct test_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_test_window* getLastTuple_test_window(IM_REL* inwindow, inWinType_test_window* tuple, bufferMngr* bm) {
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
   memcpy((*tuple).b, (char*)data.data+4, 10);
   *((*tuple).b+10) = '\0';
   memcpy(&((*tuple).c), (char*)data.data+14, sizeof(struct timeval));
   memcpy(&((*tuple).d), (char*)data.data+22, sizeof(double));
   //printf("Retrieved (*tuple).d = %f\n", (*tuple).d);
   //fflush(stdout);
   memcpy(&((*tuple).ae), (char*)data.data+30, sizeof(struct iExt_));
   //printf("Retrieved  iExt (*tuple).ae(%d, %d)\n", (*tuple).ae.length, (*tuple).ae.pt[1]);
   //fflush(stdout);
   memcpy(&((*tuple).be), (char*)data.data+38, sizeof(struct cExt_));
   //printf("Retrieved  cExt (*tuple).be(%d, %d)\n", (*tuple).be.length, (*tuple).be.pt[1]);
   //fflush(stdout);
   memcpy(&((*tuple).ce), (char*)data.data+46, sizeof(struct tExt_));
   //printf("Retrieved  tExt (*tuple).ce(%d, %d)\n", (*tuple).ce.length, (*tuple).ce.pt[1]);
   //fflush(stdout);
   memcpy(&((*tuple).de), (char*)data.data+54, sizeof(struct rExt_));
   //printf("Retrieved  rExt (*tuple).de(%d, %d)\n", (*tuple).de.length, (*tuple).de.pt[1]);
   //fflush(stdout);
   return tuple;
}
extern "C" void test_window_init(struct test_window_status *status, 
	int a, char* b, struct timeval c, double d, struct iExt_ ae, struct cExt_ be, struct tExt_ ce, struct rExt_ de, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void test_window_init(struct test_window_status *status, int a, char* b, struct timeval c, double d, struct iExt_ ae, struct cExt_ be, struct tExt_ ce, struct rExt_ de, 
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
   struct inWinType_test_window tuple;
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
   } insert_12;
   IM_RELC *w_18;
   int first_entry_17 = 1;
   int first_entry_18 = 1;
   int index_16 = 0;
   int terminating_16=0;
   struct gb_status_16 {
      struct test_status *test_0;
   };
   struct gb_status_16 *gbstatus_16 = (struct gb_status_16 *)0;
   
   int first_entry_16 = 1;
   int first_entry_14 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_18, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0001_14_13;
      next_14:
      struct {
         int a_0;
         char a_1[11];
         struct timeval a_2;
         double a_3;
         struct iExt_ a_4;
         struct cExt_ a_5;
         struct tExt_ a_6;
         struct rExt_ a_7;
         int a_0_expire;
         char a_1_expire[11];
         struct timeval a_2_expire;
         double a_3_expire;
         struct iExt_ a_4_expire;
         struct cExt_ a_5_expire;
         struct tExt_ a_6_expire;
         struct rExt_ a_7_expire;
         struct timeval atime;
      } Q_0000_16_15;
      next_16:
      while (index_16>=0 && index_16 < 2) {
         switch(index_16) {
            case 0:
            {
               if (terminating_16 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int a;
                     char b[11];
                     struct timeval c;
                     double d;
                     struct iExt_ ae;
                     struct cExt_ be;
                     struct tExt_ ce;
                     struct rExt_ de;
                     int a_expire;
                     char b_expire[11];
                     struct timeval c_expire;
                     double d_expire;
                     struct iExt_ ae_expire;
                     struct cExt_ be_expire;
                     struct tExt_ ce_expire;
                     struct rExt_ de_expire;
                     struct timeval atime;
                  } w_18_17;
                  next_18:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_18->c_get(w_18, &key, &data, (first_entry_17)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_17 = 0;
                     memcpy(&(w_18_17.a), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_18_17.a = %d\n", w_18_17.a);
                     //fflush(stdout);
                     memcpy(w_18_17.b, (char*)data.data+4, 10);
                     *(w_18_17.b+10) = '\0';
                     memcpy(&(w_18_17.c), (char*)data.data+14, sizeof(struct timeval));
                     memcpy(&(w_18_17.d), (char*)data.data+22, sizeof(double));
                     //printf("Retrieved w_18_17.d = %f\n", w_18_17.d);
                     //fflush(stdout);
                     memcpy(&(w_18_17.ae), (char*)data.data+30, sizeof(struct iExt_));
                     //printf("Retrieved  iExt w_18_17.ae(%d, %d)\n", w_18_17.ae.length, w_18_17.ae.pt[1]);
                     //fflush(stdout);
                     memcpy(&(w_18_17.be), (char*)data.data+38, sizeof(struct cExt_));
                     //printf("Retrieved  cExt w_18_17.be(%d, %d)\n", w_18_17.be.length, w_18_17.be.pt[1]);
                     //fflush(stdout);
                     memcpy(&(w_18_17.ce), (char*)data.data+46, sizeof(struct tExt_));
                     //printf("Retrieved  tExt w_18_17.ce(%d, %d)\n", w_18_17.ce.length, w_18_17.ce.pt[1]);
                     //fflush(stdout);
                     memcpy(&(w_18_17.de), (char*)data.data+54, sizeof(struct rExt_));
                     //printf("Retrieved  rExt w_18_17.de(%d, %d)\n", w_18_17.de.length, w_18_17.de.pt[1]);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_17 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0000_16_15.a_0 = w_18_17.a;
                     memcpy(Q_0000_16_15.a_1, w_18_17.b, 10);
                     Q_0000_16_15.a_1[10]=0;
                     memcpy(&(Q_0000_16_15.a_2), &w_18_17.c, sizeof(timestamp));
                     Q_0000_16_15.a_3 = w_18_17.d;
                     memcpy(&(Q_0000_16_15.a_4), &w_18_17.ae, sizeof(struct iExt_));
                     memcpy(&(Q_0000_16_15.a_5), &w_18_17.be, sizeof(struct cExt_));
                     memcpy(&(Q_0000_16_15.a_6), &w_18_17.ce, sizeof(struct tExt_));
                     memcpy(&(Q_0000_16_15.a_7), &w_18_17.de, sizeof(struct rExt_));
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_16 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_16 = (struct gb_status_16 *)0;
                     rc = hash_get(16, _rec_id, gbkey, 4, (char**)&gbstatus_16);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_16 = (struct gb_status_16*)malloc(sizeof(*gbstatus_16));
                        gbstatus_16->test_0 = (struct test_status*)malloc(sizeof(struct test_status));
                        gbstatus_16->test_0->win = 0;
                        setModelId("");
                        test_init(gbstatus_16->test_0, Q_0000_16_15.a_0, Q_0000_16_15.a_1, Q_0000_16_15.a_2, Q_0000_16_15.a_3, Q_0000_16_15.a_4, Q_0000_16_15.a_5, Q_0000_16_15.a_6, Q_0000_16_15.a_7, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0000_16_15.a_4);
                        rc = hash_put(16, _rec_id, gbkey, 4, &gbstatus_16);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        test_init(gbstatus_16->test_0, Q_0000_16_15.a_0, Q_0000_16_15.a_1, Q_0000_16_15.a_2, Q_0000_16_15.a_3, Q_0000_16_15.a_4, Q_0000_16_15.a_5, Q_0000_16_15.a_6, Q_0000_16_15.a_7, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0000_16_15.a_4);
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_16 = 1;
                  }
               }
               if (terminating_16 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(16, _rec_id, allkey, 4, (char**)&gbstatus_16);
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
               rc = gbstatus_16->test_0->retc->c_get(gbstatus_16->test_0->retc, &key, &data, (gbstatus_16->test_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_16->test_0->retc_first_entry = 0;
                  memcpy(&(Q_0001_14_13.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0001_14_13.a_0 = %d\n", Q_0001_14_13.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_16->test_0->retc->c_del(gbstatus_16->test_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_16->test_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_16 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_16++;
         }
         if (rc == DB_NOTFOUND) {
            index_16--;
            if (terminating_16 == 1 && index_16 == 0) {
               if (gbstatus_16->test_0->retc && (rc = gbstatus_16->test_0->retc->c_close(gbstatus_16->test_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_16->test_0);
               
               if (gbstatus_16->test_0->ret && ((rc = gbstatus_16->test_0->ret->close(gbstatus_16->test_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_16->test_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_16--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_16 = 0;
         first_entry_16 = 1;
         index_16 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(16, _rec_id, allkey, 4, (char**)&gbstatus_16);
            if (rc==0) {
               free(gbstatus_16->test_0);
               //printf("freeing 16\n");
               free(gbstatus_16);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(16, _rec_id);
      }
      if (rc==0) {
         insert_12.a_0 = Q_0001_14_13.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_12.a_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_12.a_0), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_18 && (rc = w_18->c_close(w_18)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
struct test2_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void test2_init(struct test2_status *status, 
	int id, struct iExt_ items, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void test2_init(struct test2_status *status, int id, struct iExt_ items,
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
   struct {
      int id;
      char field_1[15];
      int col;
      int val;
      int id_expire;
      char field_1_expire[15];
      int col_expire;
      int val_expire;
      struct timeval atime;
   } insert_19;
   int first_entry_20 = 1;
   int first_entry_21 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      struct {
         int col;
         int val;
         int col_expire;
         int val_expire;
         struct timeval atime;
      } X_21_20;
      next_21:
      rc = _iExtVert(first_entry_20, (void*)(&X_21_20), items);
      if (rc==0) {
         first_entry_20 = 0;
      } else if (rc == DB_NOTFOUND) {
         first_entry_20 = 1;
      } else {
         adlabort(rc, "External Table Function");
      }
      if (rc==0) {
         insert_19.id = id;
         memcpy(insert_19.field_1, " we got items", 14);
         insert_19.field_1[14]=0;
         insert_19.col = X_21_20.col;
         insert_19.val = X_21_20.val;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_19.id);
         printf("\t%s\t ", insert_19.field_1);
         printf("%4d ", insert_19.col);
         printf("%4d ", insert_19.val);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
struct inWinType_test2_window {
   int id;
   struct iExt_ items;
};
struct test2_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_test2_window* getLastTuple_test2_window(IM_REL* inwindow, inWinType_test2_window* tuple, bufferMngr* bm) {
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
   memcpy(&((*tuple).items), (char*)data.data+4, sizeof(struct iExt_));
   //printf("Retrieved  iExt (*tuple).items(%d, %d)\n", (*tuple).items.length, (*tuple).items.pt[1]);
   //fflush(stdout);
   return tuple;
}
extern "C" void test2_window_init(struct test2_window_status *status, 
	int id, struct iExt_ items, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void test2_window_init(struct test2_window_status *status, int id, struct iExt_ items, 
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
   struct inWinType_test2_window tuple;
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
   } insert_22;
   IM_RELC *w_28;
   int first_entry_27 = 1;
   int first_entry_28 = 1;
   int index_26 = 0;
   int terminating_26=0;
   struct gb_status_26 {
      struct test2_status *test2_0;
   };
   struct gb_status_26 *gbstatus_26 = (struct gb_status_26 *)0;
   
   int first_entry_26 = 1;
   int first_entry_24 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_28, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0003_24_23;
      next_24:
      struct {
         int a_0;
         struct iExt_ a_1;
         int a_0_expire;
         struct iExt_ a_1_expire;
         struct timeval atime;
      } Q_0002_26_25;
      next_26:
      while (index_26>=0 && index_26 < 2) {
         switch(index_26) {
            case 0:
            {
               if (terminating_26 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int id;
                     struct iExt_ items;
                     int id_expire;
                     struct iExt_ items_expire;
                     struct timeval atime;
                  } w_28_27;
                  next_28:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_28->c_get(w_28, &key, &data, (first_entry_27)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_27 = 0;
                     memcpy(&(w_28_27.id), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_28_27.id = %d\n", w_28_27.id);
                     //fflush(stdout);
                     memcpy(&(w_28_27.items), (char*)data.data+4, sizeof(struct iExt_));
                     //printf("Retrieved  iExt w_28_27.items(%d, %d)\n", w_28_27.items.length, w_28_27.items.pt[1]);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_27 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0002_26_25.a_0 = w_28_27.id;
                     memcpy(&(Q_0002_26_25.a_1), &w_28_27.items, sizeof(struct iExt_));
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_26 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_26 = (struct gb_status_26 *)0;
                     rc = hash_get(26, _rec_id, gbkey, 4, (char**)&gbstatus_26);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_26 = (struct gb_status_26*)malloc(sizeof(*gbstatus_26));
                        gbstatus_26->test2_0 = (struct test2_status*)malloc(sizeof(struct test2_status));
                        gbstatus_26->test2_0->win = 0;
                        setModelId("");
                        test2_init(gbstatus_26->test2_0, Q_0002_26_25.a_0, Q_0002_26_25.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0002_26_25.a_1);
                        rc = hash_put(26, _rec_id, gbkey, 4, &gbstatus_26);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        test2_init(gbstatus_26->test2_0, Q_0002_26_25.a_0, Q_0002_26_25.a_1, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0002_26_25.a_1);
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_26 = 1;
                  }
               }
               if (terminating_26 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(26, _rec_id, allkey, 4, (char**)&gbstatus_26);
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
               rc = gbstatus_26->test2_0->retc->c_get(gbstatus_26->test2_0->retc, &key, &data, (gbstatus_26->test2_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_26->test2_0->retc_first_entry = 0;
                  memcpy(&(Q_0003_24_23.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0003_24_23.a_0 = %d\n", Q_0003_24_23.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_26->test2_0->retc->c_del(gbstatus_26->test2_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_26->test2_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_26 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_26++;
         }
         if (rc == DB_NOTFOUND) {
            index_26--;
            if (terminating_26 == 1 && index_26 == 0) {
               if (gbstatus_26->test2_0->retc && (rc = gbstatus_26->test2_0->retc->c_close(gbstatus_26->test2_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_26->test2_0);
               
               if (gbstatus_26->test2_0->ret && ((rc = gbstatus_26->test2_0->ret->close(gbstatus_26->test2_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_26->test2_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_26--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_26 = 0;
         first_entry_26 = 1;
         index_26 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(26, _rec_id, allkey, 4, (char**)&gbstatus_26);
            if (rc==0) {
               free(gbstatus_26->test2_0);
               //printf("freeing 26\n");
               free(gbstatus_26);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(26, _rec_id);
      }
      if (rc==0) {
         insert_22.a_0 = Q_0003_24_23.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_22.a_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_22.a_0), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_28 && (rc = w_28->c_close(w_28)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
/**** Query Declarations ****/
int _adl_statement_31()
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
      char field_1[11];
      struct timeval field_2;
      double field_3;
      int field_0_expire;
      char field_1_expire[11];
      struct timeval field_2_expire;
      double field_3_expire;
      struct timeval atime;
   } insert_29;
   int first_entry_30 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_30:
      rc = (first_entry_30)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_30=1;
      else {
         first_entry_30=0;
         insert_29.field_0 = 1;
         memcpy(insert_29.field_1, "ahshs", 6);
         insert_29.field_1[6]=0;
         insert_29.field_2.tv_sec =  907794523;
         insert_29.field_2.tv_usec = 233200;
         insert_29.field_3 = 4.650000;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_29.field_0), sizeof(int));
         memcpy((char*)data.data+4, insert_29.field_1, 10);
         memcpy((char*)data.data+14, &(insert_29.field_2), sizeof(struct timeval));
         memcpy((char*)data.data+22, &(insert_29.field_3), sizeof(double));
         data.size = 30;
         key.size = 0;
         if ((rc = A->put(A, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         A->sync(A, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
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
      int field_0;
      char field_1[11];
      struct timeval field_2;
      double field_3;
      int field_0_expire;
      char field_1_expire[11];
      struct timeval field_2_expire;
      double field_3_expire;
      struct timeval atime;
   } insert_32;
   int first_entry_33 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_33:
      rc = (first_entry_33)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_33=1;
      else {
         first_entry_33=0;
         insert_32.field_0 = 2;
         memcpy(insert_32.field_1, "akadfd", 7);
         insert_32.field_1[7]=0;
         insert_32.field_2.tv_sec =  718402113;
         insert_32.field_2.tv_usec = 233200;
         insert_32.field_3 = 3.450000;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_32.field_0), sizeof(int));
         memcpy((char*)data.data+4, insert_32.field_1, 10);
         memcpy((char*)data.data+14, &(insert_32.field_2), sizeof(struct timeval));
         memcpy((char*)data.data+22, &(insert_32.field_3), sizeof(double));
         data.size = 30;
         key.size = 0;
         if ((rc = A->put(A, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         A->sync(A, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_42()
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
   } insert_35;
   DBC *A_41;
   int first_entry_40 = 1;
   int first_entry_41 = 1;
   int index_39 = 0;
   int terminating_39=0;
   struct gb_status_39 {
      struct test_status *test_0;
   };
   struct gb_status_39 *gbstatus_39 = (struct gb_status_39 *)0;
   
   int first_entry_39 = 1;
   int first_entry_37 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = A->cursor(A, NULL, &A_41, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0005_37_36;
      next_37:
      struct {
         int a_0;
         char a_1[11];
         struct timeval a_2;
         double a_3;
         struct iExt_ a_4;
         struct cExt_ a_5;
         struct tExt_ a_6;
         struct rExt_ a_7;
         int a_0_expire;
         char a_1_expire[11];
         struct timeval a_2_expire;
         double a_3_expire;
         struct iExt_ a_4_expire;
         struct cExt_ a_5_expire;
         struct tExt_ a_6_expire;
         struct rExt_ a_7_expire;
         struct timeval atime;
      } Q_0004_39_38;
      next_39:
      while (index_39>=0 && index_39 < 2) {
         switch(index_39) {
            case 0:
            {
               if (terminating_39 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int a;
                     char b[11];
                     struct timeval c;
                     double d;
                     int a_expire;
                     char b_expire[11];
                     struct timeval c_expire;
                     double d_expire;
                     struct timeval atime;
                  } A_41_40;
                  next_41:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = A_41->c_get(A_41, &key, &data, (first_entry_40)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_40 = 0;
                     memcpy(&(A_41_40.a), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved A_41_40.a = %d\n", A_41_40.a);
                     //fflush(stdout);
                     memcpy(A_41_40.b, (char*)data.data+4, 10);
                     *(A_41_40.b+10) = '\0';
                     memcpy(&(A_41_40.c), (char*)data.data+14, sizeof(struct timeval));
                     memcpy(&(A_41_40.d), (char*)data.data+22, sizeof(double));
                     //printf("Retrieved A_41_40.d = %f\n", A_41_40.d);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_40 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0004_39_38.a_0 = A_41_40.a;
                     memcpy(Q_0004_39_38.a_1, A_41_40.b, 10);
                     Q_0004_39_38.a_1[10]=0;
                     memcpy(&(Q_0004_39_38.a_2), &A_41_40.c, sizeof(timestamp));
                     Q_0004_39_38.a_3 = A_41_40.d;
                     memcpy(&(Q_0004_39_38.a_4), &_newiext(2, A_41_40.a,((A_41_40.a) * A_41_40.a)), sizeof(struct iExt_));
                     memcpy(&(Q_0004_39_38.a_5), &_newcext(2, A_41_40.b,A_41_40.b), sizeof(struct cExt_));
                     memcpy(&(Q_0004_39_38.a_6), &_newtext(2, A_41_40.c,timeval_add(A_41_40.c, 300)), sizeof(struct tExt_));
                     memcpy(&(Q_0004_39_38.a_7), &_newrext(2, A_41_40.d,((A_41_40.d) + A_41_40.d)), sizeof(struct rExt_));
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_39 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_39 = (struct gb_status_39 *)0;
                     rc = hash_get(39, _rec_id, gbkey, 4, (char**)&gbstatus_39);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_39 = (struct gb_status_39*)malloc(sizeof(*gbstatus_39));
                        gbstatus_39->test_0 = (struct test_status*)malloc(sizeof(struct test_status));
                        gbstatus_39->test_0->win = 0;
                        setModelId("");
                        test_init(gbstatus_39->test_0, Q_0004_39_38.a_0, Q_0004_39_38.a_1, Q_0004_39_38.a_2, Q_0004_39_38.a_3, Q_0004_39_38.a_4, Q_0004_39_38.a_5, Q_0004_39_38.a_6, Q_0004_39_38.a_7, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0004_39_38.a_4);
                        rc = hash_put(39, _rec_id, gbkey, 4, &gbstatus_39);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        test_init(gbstatus_39->test_0, Q_0004_39_38.a_0, Q_0004_39_38.a_1, Q_0004_39_38.a_2, Q_0004_39_38.a_3, Q_0004_39_38.a_4, Q_0004_39_38.a_5, Q_0004_39_38.a_6, Q_0004_39_38.a_7, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
                        _deleteiext(Q_0004_39_38.a_4);
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_39 = 1;
                  }
               }
               if (terminating_39 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(39, _rec_id, allkey, 4, (char**)&gbstatus_39);
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
               rc = gbstatus_39->test_0->retc->c_get(gbstatus_39->test_0->retc, &key, &data, (gbstatus_39->test_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_39->test_0->retc_first_entry = 0;
                  memcpy(&(Q_0005_37_36.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0005_37_36.a_0 = %d\n", Q_0005_37_36.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_39->test_0->retc->c_del(gbstatus_39->test_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_39->test_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_39 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_39++;
         }
         if (rc == DB_NOTFOUND) {
            index_39--;
            if (terminating_39 == 1 && index_39 == 0) {
               if (gbstatus_39->test_0->retc && (rc = gbstatus_39->test_0->retc->c_close(gbstatus_39->test_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_39->test_0);
               
               if (gbstatus_39->test_0->ret && ((rc = gbstatus_39->test_0->ret->close(gbstatus_39->test_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_39->test_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_39--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_39 = 0;
         first_entry_39 = 1;
         index_39 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(39, _rec_id, allkey, 4, (char**)&gbstatus_39);
            if (rc==0) {
               free(gbstatus_39->test_0);
               //printf("freeing 39\n");
               free(gbstatus_39);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(39, _rec_id);
      }
      if (rc==0) {
         insert_35.a_0 = Q_0005_37_36.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_35.a_0);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (A_41 && (rc = A_41->c_close(A_41)) != 0) {
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
   (void)unlink("_adl_db_A");
   if ((rc = db_create(&A, NULL, 0)) != 0) {
      adlabort(rc, "db_create()");
   }
   if ((rc = A->set_pagesize(A, 2048)) != 0) {
      adlabort(rc, "set_pagesize()");
   }
   if ((rc = A->set_flags(A, DB_DUP)) != 0) {
      adlabort(rc, "set_flags()");
   }
   if ((rc=A->set_bt_compare(A, _A_cmp)) != 0){
      adlabort(rc, "IM_REL->put()");
   }
   if ((rc = A->open(A, "_adl_db_A", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_statement_31();
   _adl_statement_34();
   _adl_statement_42();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if (A && ((rc = A->close(A, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   A = NULL;
   (void)unlink("_adl_db_A");
   return(rc);
};
