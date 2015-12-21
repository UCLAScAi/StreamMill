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
DB *swimparams;
int _swimparams_cmp(DB* dbp, const DBT *a, const DBT *b){
   	int ai, bi, ri, rs;
   	double ad, bd, rd;
   	struct timeval *at, *bt;return 0;
};
DB *tdata;
int _tdata_cmp(DB* dbp, const DBT *a, const DBT *b){
   	int ai, bi, ri, rs;
   	double ad, bd, rd;
   	struct timeval *at, *bt;return 0;
};
int (*_adl_func_0)(int, struct iExt_) = NULL;
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
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   
   if (swimparams && ((rc = swimparams->close(swimparams, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   swimparams = NULL;
   (void)unlink("paramTable.data");
   if ((rc = db_create(&swimparams, NULL, 0)) != 0) {
      adlabort(rc, "db_create()");
   }
   if ((rc = swimparams->set_pagesize(swimparams, 2048)) != 0) {
      adlabort(rc, "set_pagesize()");
   }
   if ((rc = swimparams->set_flags(swimparams, DB_DUP)) != 0) {
      adlabort(rc, "set_flags()");
   }
   if ((rc = swimparams->open(swimparams, "paramTable.data", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_5()
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
      struct timeval atime;
   } insert_3;
   int first_entry_4 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_4:
      rc = (first_entry_4)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_4=1;
      else {
         first_entry_4=0;
         memcpy(insert_3.field_0, "trans_avg_len", 14);
         insert_3.field_0[14]=0;
         memcpy(insert_3.field_1, "35", 3);
         insert_3.field_1[3]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_3.field_0, 100);
         memcpy((char*)data.data+100, insert_3.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         swimparams->sync(swimparams, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_8()
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
      struct timeval atime;
   } insert_6;
   int first_entry_7 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_7:
      rc = (first_entry_7)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_7=1;
      else {
         first_entry_7=0;
         memcpy(insert_6.field_0, "window_size", 12);
         insert_6.field_0[12]=0;
         memcpy(insert_6.field_1, "20000", 6);
         insert_6.field_1[6]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_6.field_0, 100);
         memcpy((char*)data.data+100, insert_6.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         swimparams->sync(swimparams, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_11()
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
      struct timeval atime;
   } insert_9;
   int first_entry_10 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_10:
      rc = (first_entry_10)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_10=1;
      else {
         first_entry_10=0;
         memcpy(insert_9.field_0, "slide_size", 11);
         insert_9.field_0[11]=0;
         memcpy(insert_9.field_1, "10000", 6);
         insert_9.field_1[6]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_9.field_0, 100);
         memcpy((char*)data.data+100, insert_9.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         swimparams->sync(swimparams, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
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
         memcpy(insert_12.field_0, "no_items", 9);
         insert_12.field_0[9]=0;
         memcpy(insert_12.field_1, "1000", 5);
         insert_12.field_1[5]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_12.field_0, 100);
         memcpy((char*)data.data+100, insert_12.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         swimparams->sync(swimparams, 0);
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
      char field_0[101];
      char field_1[101];
      char field_0_expire[101];
      char field_1_expire[101];
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
         memcpy(insert_15.field_0, "L_delay_max", 12);
         insert_15.field_0[12]=0;
         memcpy(insert_15.field_1, "1", 2);
         insert_15.field_1[2]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_15.field_0, 100);
         memcpy((char*)data.data+100, insert_15.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         swimparams->sync(swimparams, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
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
   } insert_18;
   int first_entry_19 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_19:
      rc = (first_entry_19)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_19=1;
      else {
         first_entry_19=0;
         memcpy(insert_18.field_0, "min_supp_10000", 15);
         insert_18.field_0[15]=0;
         memcpy(insert_18.field_1, "15", 3);
         insert_18.field_1[3]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_18.field_0, 100);
         memcpy((char*)data.data+100, insert_18.field_1, 100);
         data.size = 200;
         key.size = 0;
         if ((rc = swimparams->put(swimparams, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         swimparams->sync(swimparams, 0);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_22()
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
   
   if (tdata && ((rc = tdata->close(tdata, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   tdata = NULL;
   (void)unlink("_adl_db_tdata");
   (void)unlink("_adl_db_tdata");
   if ((rc = db_create(&tdata, NULL, 0)) != 0) {
      adlabort(rc, "db_create()");
   }
   if ((rc = tdata->set_pagesize(tdata, 2048)) != 0) {
      adlabort(rc, "set_pagesize()");
   }
   if ((rc = tdata->set_flags(tdata, DB_DUP)) != 0) {
      adlabort(rc, "set_flags()");
   }
   if ((rc = tdata->open(tdata, "_adl_db_tdata", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_23()
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
      FILE *_adl_load = fopen("/tmp/sigkdd1.stream", "rt");
      char _adl_load_buf[40960], *tok;
      char loadkeybuf[1], loaddatabuf[5];
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
      data.size = 4;
      while (fgets(_adl_load_buf, 40959, _adl_load)) {
         _adl_line_no++;
         tok = csvtok(_adl_load_buf, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(int*)((char*)data.data+0) = atoi(tok);
         if ((rc = tdata->put(tdata, NULL, &key, &data, 0))!=0) {
            tdata->err(tdata, rc, "tdata");
            exit(rc);
         }
      } /* end of while */
      fclose(_adl_load);
   }
   tdata->sync(tdata, 0);
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
      int field_0;
      int field_0_expire;
      struct timeval atime;
   } insert_24;
   DBC *tdata_26;
   int first_entry_25 = 1;
   int first_entry_27 = 1;
   int index_26 = 0;
   int first_entry_26 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = tdata->cursor(tdata, NULL, &tdata_26, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         int it;
         int it_expire;
         struct timeval atime;
      } tdata_26_25;
      struct {
         int tid;
         struct iExt_ val;
         int tid_expire;
         struct iExt_ val_expire;
         struct timeval atime;
      } t_26_27;
      next_26:
      while (index_26>=0 && index_26 < 2) { 
         switch(index_26) {
            case 0:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = tdata_26->c_get(tdata_26, &key, &data, (first_entry_25)? DB_FIRST:DB_NEXT);
               if (rc==0) {
                  first_entry_25 = 0;
                  memcpy(&(tdata_26_25.it), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved tdata_26_25.it = %d\n", tdata_26_25.it);
                  //fflush(stdout);
               } else if (rc == DB_NOTFOUND) {
                  first_entry_25 = 1;
               } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            }
            break;
            case 1:
            {
               rc = _buildIExt(first_entry_27, (void*)(&t_26_27), tdata_26_25.it);
               if (rc==0) {
                  first_entry_27 = 0;
               } else if (rc == DB_NOTFOUND) {
                  first_entry_27 = 1;
               } else {
                  adlabort(rc, "External Table Function");
               }
            }
            break;
         } /*switch */
         if (rc==0) {
            index_26++;
         } else if (rc==DB_NOTFOUND) {
            index_26--;
         }
      } /* while */
      if (rc!=0) {
         index_26++;    /* set index to the first subgoal */
      } else {
         index_26--;    /* set index to the last subgoal */
         insert_24.field_0 = (*_adl_func_0)(t_26_27.tid, t_26_27.val);
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (tdata_26 && (rc = tdata_26->c_close(tdata_26)) != 0) {
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
   if ((rc = db_create(&swimparams, NULL, 0)) != 0) {
      adlabort(rc, "db_create()");
   }
   if ((rc = swimparams->set_pagesize(swimparams, 2048)) != 0) {
      adlabort(rc, "set_pagesize()");
   }
   if ((rc = swimparams->set_flags(swimparams, DB_DUP)) != 0) {
      adlabort(rc, "set_flags()");
   }
   if ((rc=swimparams->set_bt_compare(swimparams, _swimparams_cmp)) != 0){
      adlabort(rc, "IM_REL->put()");
   }
   if ((rc = swimparams->open(swimparams, "paramTable.data", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      adlabort(rc, "open()");
   }
   (void)unlink("_adl_db_tdata");
   if ((rc = db_create(&tdata, NULL, 0)) != 0) {
      adlabort(rc, "db_create()");
   }
   if ((rc = tdata->set_pagesize(tdata, 2048)) != 0) {
      adlabort(rc, "set_pagesize()");
   }
   if ((rc = tdata->set_flags(tdata, DB_DUP)) != 0) {
      adlabort(rc, "set_flags()");
   }
   if ((rc=tdata->set_bt_compare(tdata, _tdata_cmp)) != 0){
      adlabort(rc, "IM_REL->put()");
   }
   if ((rc = tdata->open(tdata, "_adl_db_tdata", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_func_0 = (int(*)(int, struct iExt_))_adl_dlm("./funswim.so", "swimfis"); //h4
   _adl_statement_2();
   _adl_statement_5();
   _adl_statement_8();
   _adl_statement_11();
   _adl_statement_14();
   _adl_statement_17();
   _adl_statement_20();
   _adl_statement_22();
   _adl_statement_23();
   _adl_statement_28();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if (swimparams && ((rc = swimparams->close(swimparams, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   swimparams = NULL;
   
   if (tdata && ((rc = tdata->close(tdata, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   tdata = NULL;
   (void)unlink("_adl_db_tdata");
   return(rc);
};
