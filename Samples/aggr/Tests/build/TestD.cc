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
struct mytestd_status {
   IM_REL *prices;
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};

extern "C" void mytestd_init(struct mytestd_status *status, 
	double start_price, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL, char* data_all_row=NULL, char* data_schema=NULL);
extern "C" void mytestd_iterate(struct mytestd_status *status, 
	double start_price, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL, char* data_all_row=NULL, char* data_schema=NULL);
extern "C" void mytestd_terminate(struct mytestd_status *status, 
	double start_price, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL, char* data_all_row=NULL, char* data_schema=NULL);
extern "C" void mytestd_init(struct mytestd_status *status, double start_price,
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
      sprintf(_adl_dbname, "._%d_prices", status);
      if ((rc = im_rel_create(&status->prices, NULL, IM_LINKEDLIST, 0)) != 0) {
         adlabort(rc, "im_rel_create()");
      }
      if ((rc = status->prices->open(status->prices, _adl_dbname, 0)) != 0) {
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
      double field_0;
      double field_1;
      int field_2;
      double field_3;
      double field_4;
      double field_0_expire;
      double field_1_expire;
      int field_2_expire;
      double field_3_expire;
      double field_4_expire;
      struct timeval atime;
   } insert_0;
   char* insert_0_schema = 
   	"insert_0\n"
   	"field_0 Double\n"
   	"field_1 Double\n"
   	"field_2 INT\n"
   	"field_3 Double\n"
   	"field_4 Double\n";
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
         insert_0.field_4 = 0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_0.field_0), sizeof(double));
         memcpy((char*)data.data+8, &(insert_0.field_1), sizeof(double));
         memcpy((char*)data.data+16, &(insert_0.field_2), sizeof(int));
         memcpy((char*)data.data+20, &(insert_0.field_3), sizeof(double));
         memcpy((char*)data.data+28, &(insert_0.field_4), sizeof(double));
         data.size = 36;
         key.size = 0;
         if ((rc = status->prices->put(status->prices, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0)*/
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}

extern "C" void mytestd_iterate(struct mytestd_status *status, 
	double start_price, int _rec_id, bufferMngr* bm, 
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
   IM_RELC *prices_2;
   int first_entry_3 = 1;
   int first_entry_2 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY 3 */;
   if ((rc = status->prices->cursor(status->prices, &prices_2, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double xsqrd;
         double x;
         int count;
         double sum;
         double avg;
         int OID;
         double xsqrd_expire;
         double x_expire;
         int count_expire;
         double sum_expire;
         double avg_expire;
         int OID_expire;
         struct timeval atime;
      } prices_2_3;
      char* prices_2_3_schema = 
      	"prices_2_3\n"
      	"xsqrd Double\n"
      	"x Double\n"
      	"count INT\n"
      	"sum Double\n"
      	"avg Double\n"
      	"OID INT\n";
      next_2:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = prices_2->c_get(prices_2, &key, &data, (first_entry_3)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_3 = 0;
         memcpy(&(prices_2_3.xsqrd), (char*)data.data+0, sizeof(double));
         //printf("Retrieved prices_2_3.xsqrd = %f\n", prices_2_3.xsqrd);
         //fflush(stdout);
         memcpy(&(prices_2_3.x), (char*)data.data+8, sizeof(double));
         //printf("Retrieved prices_2_3.x = %f\n", prices_2_3.x);
         //fflush(stdout);
         memcpy(&(prices_2_3.count), (char*)data.data+16, sizeof(int));
         //printf("Retrieved prices_2_3.count = %d\n", prices_2_3.count);
         //fflush(stdout);
         memcpy(&(prices_2_3.sum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved prices_2_3.sum = %f\n", prices_2_3.sum);
         //fflush(stdout);
         memcpy(&(prices_2_3.avg), (char*)data.data+28, sizeof(double));
         //printf("Retrieved prices_2_3.avg = %f\n", prices_2_3.avg);
         //fflush(stdout);
         memcpy(&(prices_2_3.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_2_3.OID = %d\n", prices_2_3.OID);
         //fflush(stdout);
         memcpy(&(prices_2_3.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_2_3.OID = %d\n", prices_2_3.OID);
         //fflush(stdout);
         memcpy(&(prices_2_3.xsqrd), (char*)data.data+0, sizeof(double));
         //printf("Retrieved prices_2_3.xsqrd = %f\n", prices_2_3.xsqrd);
         //fflush(stdout);
         memcpy(&(prices_2_3.x), (char*)data.data+8, sizeof(double));
         //printf("Retrieved prices_2_3.x = %f\n", prices_2_3.x);
         //fflush(stdout);
         memcpy(&(prices_2_3.count), (char*)data.data+16, sizeof(int));
         //printf("Retrieved prices_2_3.count = %d\n", prices_2_3.count);
         //fflush(stdout);
         memcpy(&(prices_2_3.sum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved prices_2_3.sum = %f\n", prices_2_3.sum);
         //fflush(stdout);
         memcpy(&(prices_2_3.avg), (char*)data.data+28, sizeof(double));
         //printf("Retrieved prices_2_3.avg = %f\n", prices_2_3.avg);
         //fflush(stdout);
         memcpy(&(prices_2_3.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_2_3.OID = %d\n", prices_2_3.OID);
         //fflush(stdout);
         memcpy(&(prices_2_3.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_2_3.OID = %d\n", prices_2_3.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_3 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         /*UPDATE STARTS*/
         bool copied = false;
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         copied = true; // no subuery 
         *(int*)((char*)data.data+16) = ((prices_2_3.count) + 1);
         copied = true; // no subuery 
         *(double*)((char*)data.data+20) = ((prices_2_3.sum) + start_price);
         copied = true; // no subuery 
         *(double*)((char*)data.data+0) = ((prices_2_3.xsqrd) + ((start_price) * start_price));
         if (copied) {
            
            if ((rc = prices_2->c_put(prices_2, &key, &data, DB_CURRENT)) != 0) {
               adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
            }}
         copied = true;
         
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (prices_2 && (rc = prices_2->c_close(prices_2)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void mytestd_terminate(struct mytestd_status *status, 
	double start_price, int _rec_id, int not_delete, bufferMngr* bm, 
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
   IM_RELC *prices_4;
   int first_entry_5 = 1;
   int first_entry_4 = 1;
   struct {
      double field_0;
      double field_0_expire;
      struct timeval atime;
   } insert_6;
   char* insert_6_schema = 
   	"insert_6\n"
   	"field_0 Double\n";
   IM_RELC *prices_8;
   int first_entry_7 = 1;
   int first_entry_8 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY 3 */;
   if ((rc = status->prices->cursor(status->prices, &prices_4, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double xsqrd;
         double x;
         int count;
         double sum;
         double avg;
         int OID;
         double xsqrd_expire;
         double x_expire;
         int count_expire;
         double sum_expire;
         double avg_expire;
         int OID_expire;
         struct timeval atime;
      } prices_4_5;
      char* prices_4_5_schema = 
      	"prices_4_5\n"
      	"xsqrd Double\n"
      	"x Double\n"
      	"count INT\n"
      	"sum Double\n"
      	"avg Double\n"
      	"OID INT\n";
      next_4:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = prices_4->c_get(prices_4, &key, &data, (first_entry_5)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_5 = 0;
         memcpy(&(prices_4_5.xsqrd), (char*)data.data+0, sizeof(double));
         //printf("Retrieved prices_4_5.xsqrd = %f\n", prices_4_5.xsqrd);
         //fflush(stdout);
         memcpy(&(prices_4_5.x), (char*)data.data+8, sizeof(double));
         //printf("Retrieved prices_4_5.x = %f\n", prices_4_5.x);
         //fflush(stdout);
         memcpy(&(prices_4_5.count), (char*)data.data+16, sizeof(int));
         //printf("Retrieved prices_4_5.count = %d\n", prices_4_5.count);
         //fflush(stdout);
         memcpy(&(prices_4_5.sum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved prices_4_5.sum = %f\n", prices_4_5.sum);
         //fflush(stdout);
         memcpy(&(prices_4_5.avg), (char*)data.data+28, sizeof(double));
         //printf("Retrieved prices_4_5.avg = %f\n", prices_4_5.avg);
         //fflush(stdout);
         memcpy(&(prices_4_5.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_4_5.OID = %d\n", prices_4_5.OID);
         //fflush(stdout);
         memcpy(&(prices_4_5.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_4_5.OID = %d\n", prices_4_5.OID);
         //fflush(stdout);
         memcpy(&(prices_4_5.xsqrd), (char*)data.data+0, sizeof(double));
         //printf("Retrieved prices_4_5.xsqrd = %f\n", prices_4_5.xsqrd);
         //fflush(stdout);
         memcpy(&(prices_4_5.x), (char*)data.data+8, sizeof(double));
         //printf("Retrieved prices_4_5.x = %f\n", prices_4_5.x);
         //fflush(stdout);
         memcpy(&(prices_4_5.count), (char*)data.data+16, sizeof(int));
         //printf("Retrieved prices_4_5.count = %d\n", prices_4_5.count);
         //fflush(stdout);
         memcpy(&(prices_4_5.sum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved prices_4_5.sum = %f\n", prices_4_5.sum);
         //fflush(stdout);
         memcpy(&(prices_4_5.avg), (char*)data.data+28, sizeof(double));
         //printf("Retrieved prices_4_5.avg = %f\n", prices_4_5.avg);
         //fflush(stdout);
         memcpy(&(prices_4_5.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_4_5.OID = %d\n", prices_4_5.OID);
         //fflush(stdout);
         memcpy(&(prices_4_5.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_4_5.OID = %d\n", prices_4_5.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_5 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         /*UPDATE STARTS*/
         bool copied = false;
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         copied = true; // no subuery 
         *(double*)((char*)data.data+28) = ((prices_4_5.sum) / prices_4_5.count);
         copied = true; // no subuery 
         *(double*)((char*)data.data+8) = prices_4_5.sum;
         if (copied) {
            
            if ((rc = prices_4->c_put(prices_4, &key, &data, DB_CURRENT)) != 0) {
               adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
            }}
         copied = true;
         
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (prices_4 && (rc = prices_4->c_close(prices_4)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY 2 */;
   if ((rc = status->prices->cursor(status->prices, &prices_8, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) { 
      struct {
         double xsqrd;
         double x;
         int count;
         double sum;
         double avg;
         int OID;
         double xsqrd_expire;
         double x_expire;
         int count_expire;
         double sum_expire;
         double avg_expire;
         int OID_expire;
         struct timeval atime;
      } prices_8_7;
      char* prices_8_7_schema = 
      	"prices_8_7\n"
      	"xsqrd Double\n"
      	"x Double\n"
      	"count INT\n"
      	"sum Double\n"
      	"avg Double\n"
      	"OID INT\n";
      next_8:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = prices_8->c_get(prices_8, &key, &data, (first_entry_7)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_7 = 0;
         memcpy(&(prices_8_7.xsqrd), (char*)data.data+0, sizeof(double));
         //printf("Retrieved prices_8_7.xsqrd = %f\n", prices_8_7.xsqrd);
         //fflush(stdout);
         memcpy(&(prices_8_7.x), (char*)data.data+8, sizeof(double));
         //printf("Retrieved prices_8_7.x = %f\n", prices_8_7.x);
         //fflush(stdout);
         memcpy(&(prices_8_7.count), (char*)data.data+16, sizeof(int));
         //printf("Retrieved prices_8_7.count = %d\n", prices_8_7.count);
         //fflush(stdout);
         memcpy(&(prices_8_7.sum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved prices_8_7.sum = %f\n", prices_8_7.sum);
         //fflush(stdout);
         memcpy(&(prices_8_7.avg), (char*)data.data+28, sizeof(double));
         //printf("Retrieved prices_8_7.avg = %f\n", prices_8_7.avg);
         //fflush(stdout);
         memcpy(&(prices_8_7.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_8_7.OID = %d\n", prices_8_7.OID);
         //fflush(stdout);
         memcpy(&(prices_8_7.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_8_7.OID = %d\n", prices_8_7.OID);
         //fflush(stdout);
         memcpy(&(prices_8_7.xsqrd), (char*)data.data+0, sizeof(double));
         //printf("Retrieved prices_8_7.xsqrd = %f\n", prices_8_7.xsqrd);
         //fflush(stdout);
         memcpy(&(prices_8_7.x), (char*)data.data+8, sizeof(double));
         //printf("Retrieved prices_8_7.x = %f\n", prices_8_7.x);
         //fflush(stdout);
         memcpy(&(prices_8_7.count), (char*)data.data+16, sizeof(int));
         //printf("Retrieved prices_8_7.count = %d\n", prices_8_7.count);
         //fflush(stdout);
         memcpy(&(prices_8_7.sum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved prices_8_7.sum = %f\n", prices_8_7.sum);
         //fflush(stdout);
         memcpy(&(prices_8_7.avg), (char*)data.data+28, sizeof(double));
         //printf("Retrieved prices_8_7.avg = %f\n", prices_8_7.avg);
         //fflush(stdout);
         memcpy(&(prices_8_7.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_8_7.OID = %d\n", prices_8_7.OID);
         //fflush(stdout);
         memcpy(&(prices_8_7.OID), (char*)data.data+36, sizeof(int));
         //printf("Retrieved prices_8_7.OID = %d\n", prices_8_7.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_7 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_6.field_0 = sqrt((double)(((((((prices_8_7.xsqrd) - ((((((2) * prices_8_7.sum)) * prices_8_7.sum)) / prices_8_7.count))) + ((((prices_8_7.count) * prices_8_7.avg)) * prices_8_7.avg))) / prices_8_7.count)));
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_6.field_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_6.field_0), sizeof(double));
         data.size = 8;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0)*/
   if (prices_8 && (rc = prices_8->c_close(prices_8)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   sprintf(_adl_dbname, "._%d_prices", status);
   if(!not_delete) {
      if ((rc = status->prices->close(status->prices, 0)) != 0) {
         adlabort(rc, "IM_REL->close()");
      }
   }
   if(!not_delete) status->retc_first_entry=1;
}
void getResult(struct mytestd_status status, double *result) {
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
   struct mytestd_status mytestd_0;
   double resultAccumulator;
   int offset;
   char inputMemory[60000];
} AgrStorage;

void mytestd_mergeTables(AgrStorage* s1, AgrStorage* s2) {
   int localOffset = 0;
   while (localOffset  < s2->offset) {
      mytestd_iterate(&s1->mytestd_0, *(double*)&s2->inputMemory[localOffset], 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      localOffset += sizeof(double);
      
   }
}

void mytestd_reconstructS1(AgrStorage* s2, double start_price) { // ***NOTE: s2 is really s1. The reason for confusion is to save space in the compiler...
   mytestd_init(&s2->mytestd_0, start_price, 1, 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
   int localOffset = 0;
   while (localOffset  < s2->offset) {
      mytestd_iterate(&s2->mytestd_0, *(double*)&s2->inputMemory[localOffset], 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      localOffset += sizeof(double);
      
   }
}

extern "C" void mytestd(FNC_Phase phase, FNC_Context_t *fctx, double *start_price,double *result, int *i_start_price,int *i_result, char sqlstate[6], SQL_TEXT fncname[129], SQL_TEXT sfncname[129],SQL_TEXT errorMsg[257]);
extern "C" void mytestd(FNC_Phase phase, FNC_Context_t *fctx, double *start_price,double *result, int *i_start_price,int *i_result, char sqlstate[6], SQL_TEXT fncname[129], SQL_TEXT sfncname[129],SQL_TEXT errorMsg[257]){
   AgrStorage *s1 = (AgrStorage*)fctx->interim1;
   AgrStorage *s2 = (AgrStorage*)fctx->interim2;
   switch (phase) {
      	case AGR_INIT:
      		if ((s1 = (AgrStorage*)FNC_DefMem(sizeof(AgrStorage))) == NULL) {
         			strcpy(sqlstate, "U0001"); 
         			return; 
         		} 
      		
      		mytestd_init(&s1->mytestd_0, *start_price, 1, 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL); 
      		s1->resultAccumulator = 0;
      		s1->offset = 0;
       	case AGR_DETAIL:
      		mytestd_iterate(&s1->mytestd_0, *start_price, 1, NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      		memcpy(&s1->inputMemory[s1->offset],start_price,sizeof(double));
      		s1->offset += sizeof(double);
      		break;
      	case AGR_COMBINE:
      		mytestd_reconstructS1(s1, *start_price);
      		mytestd_mergeTables(s1,s2);
      		break;
      	case AGR_FINAL:
      		mytestd_terminate(&s1->mytestd_0, *start_price, 1,0 , NULL, inMemTables, NULL, 0, getModelId(), NULL, NULL);
      		getResult(s1->mytestd_0, result/*&s1->resultAccumulator*/);
      		//*result = s1->resultAccumulator;
      		s1->mytestd_0.ret->close(s1->mytestd_0.ret,0);
      		s1->mytestd_0.retc->c_close(s1->mytestd_0.retc);
      		delete(s1->mytestd_0.win);
      		free(&s1->mytestd_0);
      		free(s1);
      		break;
      	case AGR_NODATA:
      		break;
      	default:
      		strcpy(sqlstate, "U0005");
      	}
   return;
}
