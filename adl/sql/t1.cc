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
DB *t;
int _t_cmp(DB* dbp, const DBT *a, const DBT *b){
   	int ai, bi, ri, rs;
   	double ad, bd, rd;
   	struct timeval *at, *bt;return 0;
};
DB *tt;
int _tt_cmp(DB* dbp, const DBT *a, const DBT *b){
   	int ai, bi, ri, rs;
   	double ad, bd, rd;
   	struct timeval *at, *bt;return 0;
};
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
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   struct {
      int field_0;
      int field_1;
      int field_0_expire;
      int field_1_expire;
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
         insert_0.field_1 = 2;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_0.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_0.field_1), sizeof(int));
         data.size = 8;
         key.size = 0;
         if ((rc = t->put(t, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         t->sync(t, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_6()
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
      int a;
      int b;
      int a_expire;
      int b_expire;
      struct timeval atime;
   } insert_3;
   DBC *t_5;
   int first_entry_4 = 1;
   int first_entry_5 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = t->cursor(t, NULL, &t_5, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int a;
         int b;
         int a_expire;
         int b_expire;
         struct timeval atime;
      } t_5_4;
      next_5:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = t_5->c_get(t_5, &key, &data, (first_entry_4)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_4 = 0;
         memcpy(&(t_5_4.a), (char*)data.data+0, sizeof(int));
         //printf("Retrieved t_5_4.a = %d\n", t_5_4.a);
         //fflush(stdout);
         memcpy(&(t_5_4.b), (char*)data.data+4, sizeof(int));
         //printf("Retrieved t_5_4.b = %d\n", t_5_4.b);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_4 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0 && slide_out == 1) {
         insert_3.a = t_5_4.a;
         insert_3.b = t_5_4.b;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_3.a), sizeof(int));
         memcpy((char*)data.data+4, &(insert_3.b), sizeof(int));
         data.size = 8;
         key.size = 0;
         if ((rc = tt->put(tt, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         tt->sync(tt, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (t_5 && (rc = t_5->c_close(t_5)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
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
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   struct {
      int x;
      int y;
      int x_expire;
      int y_expire;
      struct timeval atime;
   } insert_7;
   DBC *tt_9;
   int first_entry_8 = 1;
   int first_entry_9 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = tt->cursor(tt, NULL, &tt_9, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int x;
         int y;
         int x_expire;
         int y_expire;
         struct timeval atime;
      } tt_9_8;
      next_9:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = tt_9->c_get(tt_9, &key, &data, (first_entry_8)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_8 = 0;
         memcpy(&(tt_9_8.x), (char*)data.data+0, sizeof(int));
         //printf("Retrieved tt_9_8.x = %d\n", tt_9_8.x);
         //fflush(stdout);
         memcpy(&(tt_9_8.y), (char*)data.data+4, sizeof(int));
         //printf("Retrieved tt_9_8.y = %d\n", tt_9_8.y);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_8 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0 && slide_out == 1) {
         insert_7.x = tt_9_8.x;
         insert_7.y = tt_9_8.y;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_7.x);
         printf("%10d ", insert_7.y);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (tt_9 && (rc = tt_9->c_close(tt_9)) != 0) {
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
   if ((rc = db_create(&t, NULL, 0)) != 0) {
      adlabort(rc, "db_create()");
   }
   if ((rc = t->set_pagesize(t, 2048)) != 0) {
      adlabort(rc, "set_pagesize()");
   }
   if ((rc = t->set_flags(t, DB_DUP)) != 0) {
      adlabort(rc, "set_flags()");
   }
   if ((rc=t->set_bt_compare(t, _t_cmp)) != 0){
      adlabort(rc, "IM_REL->put()");
   }
   if ((rc = t->open(t, "t", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      adlabort(rc, "open()");
   }
   if ((rc = db_create(&tt, NULL, 0)) != 0) {
      adlabort(rc, "db_create()");
   }
   if ((rc = tt->set_pagesize(tt, 2048)) != 0) {
      adlabort(rc, "set_pagesize()");
   }
   if ((rc = tt->set_flags(tt, DB_DUP)) != 0) {
      adlabort(rc, "set_flags()");
   }
   if ((rc=tt->set_bt_compare(tt, _tt_cmp)) != 0){
      adlabort(rc, "IM_REL->put()");
   }
   if ((rc = tt->open(tt, "tt", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_statement_2();
   _adl_statement_6();
   _adl_statement_10();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if (t && ((rc = t->close(t, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   t = NULL;
   
   if (tt && ((rc = tt->close(tt, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   tt = NULL;
   return(rc);
};
