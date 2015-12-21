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
DB *sales;
int _sales_cmp(DB* dbp, const DBT *a, const DBT *b){
   	int ai, bi, ri, rs;
   	double ad, bd, rd;
   	struct timeval *at, *bt;return 0;
};
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
      char field_0[3];
      char field_1[3];
      int field_2;
      char field_0_expire[3];
      char field_1_expire[3];
      int field_2_expire;
      struct timeval atime;
   } insert_0;
   int index_1 = 0;
   int first_entry_3 = 1;
   int first_entry_5 = 1;
   int first_entry_7 = 1;
   int first_entry_9 = 1;
   int first_entry_11 = 1;
   int first_entry_13 = 1;
   int first_entry_1 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      if (first_entry_1==1) index_1 = 0;
      do {
         switch (index_1) {
            case 0:
            {
               struct {
                  char field_0[3];
                  char field_1[3];
                  int field_2;
                  char field_0_expire[3];
                  char field_1_expire[3];
                  int field_2_expire;
                  struct timeval atime;
               } unionqun_2;
               next_3:
               rc = (first_entry_3)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_3=1;
               else {
                  first_entry_3=0;
                  memcpy(unionqun_2.field_0, "E", 2);
                  unionqun_2.field_0[2]=0;
                  memcpy(unionqun_2.field_1, "b", 2);
                  unionqun_2.field_1[2]=0;
                  unionqun_2.field_2 = 1;
               } /* if (rc == 0) */
               if (rc==0) {
                  memcpy(insert_0.field_0, unionqun_2.field_0, 2);
                  insert_0.field_0[2] = 0;
                  memcpy(insert_0.field_1, unionqun_2.field_1, 2);
                  insert_0.field_1[2] = 0;
                  insert_0.field_2 = unionqun_2.field_2;
               }
            }
            break;
            case 1:
            {
               struct {
                  char field_0[3];
                  char field_1[3];
                  int field_2;
                  char field_0_expire[3];
                  char field_1_expire[3];
                  int field_2_expire;
                  struct timeval atime;
               } unionqun_4;
               next_5:
               rc = (first_entry_5)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_5=1;
               else {
                  first_entry_5=0;
                  memcpy(unionqun_4.field_0, "E", 2);
                  unionqun_4.field_0[2]=0;
                  memcpy(unionqun_4.field_1, "g", 2);
                  unionqun_4.field_1[2]=0;
                  unionqun_4.field_2 = 2;
               } /* if (rc == 0) */
               if (rc==0) {
                  memcpy(insert_0.field_0, unionqun_4.field_0, 2);
                  insert_0.field_0[2] = 0;
                  memcpy(insert_0.field_1, unionqun_4.field_1, 2);
                  insert_0.field_1[2] = 0;
                  insert_0.field_2 = unionqun_4.field_2;
               }
            }
            break;
            case 2:
            {
               struct {
                  char field_0[3];
                  char field_1[3];
                  int field_2;
                  char field_0_expire[3];
                  char field_1_expire[3];
                  int field_2_expire;
                  struct timeval atime;
               } unionqun_6;
               next_7:
               rc = (first_entry_7)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_7=1;
               else {
                  first_entry_7=0;
                  memcpy(unionqun_6.field_0, "E", 2);
                  unionqun_6.field_0[2]=0;
                  memcpy(unionqun_6.field_1, "b", 2);
                  unionqun_6.field_1[2]=0;
                  unionqun_6.field_2 = 3;
               } /* if (rc == 0) */
               if (rc==0) {
                  memcpy(insert_0.field_0, unionqun_6.field_0, 2);
                  insert_0.field_0[2] = 0;
                  memcpy(insert_0.field_1, unionqun_6.field_1, 2);
                  insert_0.field_1[2] = 0;
                  insert_0.field_2 = unionqun_6.field_2;
               }
            }
            break;
            case 3:
            {
               struct {
                  char field_0[3];
                  char field_1[3];
                  int field_2;
                  char field_0_expire[3];
                  char field_1_expire[3];
                  int field_2_expire;
                  struct timeval atime;
               } unionqun_8;
               next_9:
               rc = (first_entry_9)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_9=1;
               else {
                  first_entry_9=0;
                  memcpy(unionqun_8.field_0, "E", 2);
                  unionqun_8.field_0[2]=0;
                  memcpy(unionqun_8.field_1, "g", 2);
                  unionqun_8.field_1[2]=0;
                  unionqun_8.field_2 = 10;
               } /* if (rc == 0) */
               if (rc==0) {
                  memcpy(insert_0.field_0, unionqun_8.field_0, 2);
                  insert_0.field_0[2] = 0;
                  memcpy(insert_0.field_1, unionqun_8.field_1, 2);
                  insert_0.field_1[2] = 0;
                  insert_0.field_2 = unionqun_8.field_2;
               }
            }
            break;
            case 4:
            {
               struct {
                  char field_0[3];
                  char field_1[3];
                  int field_2;
                  char field_0_expire[3];
                  char field_1_expire[3];
                  int field_2_expire;
                  struct timeval atime;
               } unionqun_10;
               next_11:
               rc = (first_entry_11)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_11=1;
               else {
                  first_entry_11=0;
                  memcpy(unionqun_10.field_0, "W", 2);
                  unionqun_10.field_0[2]=0;
                  memcpy(unionqun_10.field_1, "g", 2);
                  unionqun_10.field_1[2]=0;
                  unionqun_10.field_2 = 2;
               } /* if (rc == 0) */
               if (rc==0) {
                  memcpy(insert_0.field_0, unionqun_10.field_0, 2);
                  insert_0.field_0[2] = 0;
                  memcpy(insert_0.field_1, unionqun_10.field_1, 2);
                  insert_0.field_1[2] = 0;
                  insert_0.field_2 = unionqun_10.field_2;
               }
            }
            break;
            case 5:
            {
               struct {
                  char field_0[3];
                  char field_1[3];
                  int field_2;
                  char field_0_expire[3];
                  char field_1_expire[3];
                  int field_2_expire;
                  struct timeval atime;
               } unionqun_12;
               next_13:
               rc = (first_entry_13)? 0:DB_NOTFOUND;
               if (rc == DB_NOTFOUND) first_entry_13=1;
               else {
                  first_entry_13=0;
                  memcpy(unionqun_12.field_0, "N", 2);
                  unionqun_12.field_0[2]=0;
                  memcpy(unionqun_12.field_1, "b", 2);
                  unionqun_12.field_1[2]=0;
                  unionqun_12.field_2 = 1;
               } /* if (rc == 0) */
               if (rc==0) {
                  memcpy(insert_0.field_0, unionqun_12.field_0, 2);
                  insert_0.field_0[2] = 0;
                  memcpy(insert_0.field_1, unionqun_12.field_1, 2);
                  insert_0.field_1[2] = 0;
                  insert_0.field_2 = unionqun_12.field_2;
               }
            }
            break;
         }/* end of switch */
         if (rc == DB_NOTFOUND) index_1++;
      } while (rc == DB_NOTFOUND && index_1 < 6);
      next_1:
      if (rc == DB_NOTFOUND) {
         first_entry_1 = 1;
      }
      else {
         first_entry_1 = 0;
      }
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, insert_0.field_0, 2);
         memcpy((char*)data.data+2, insert_0.field_1, 2);
         memcpy((char*)data.data+4, &(insert_0.field_2), sizeof(int));
         data.size = 8;
         key.size = 0;
         if ((rc = sales->put(sales, NULL, &key, &data, 0))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         sales->sync(sales, 0);
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
   static int last_out = 0;
   static bool iterate = false;
   static bool init = true;
   char _timeexpkey[MAX_STR_LEN];
   char *timeexpkey=_timeexpkey;
   struct {
      char r[3];
      char g[3];
      char r_expire[3];
      char g_expire[3];
      struct timeval atime;
   } insert_15;
   DBC *sales_17;
   int first_entry_16 = 1;
   int first_entry_17 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = sales->cursor(sales, NULL, &sales_17, 0)) != 0) {
      adlabort(rc, "DB->cursor()");
   }
   while (rc==0) {
      struct {
         char r[3];
         char g[3];
         int u;
         char r_expire[3];
         char g_expire[3];
         int u_expire;
         struct timeval atime;
      } sales_17_16;
      next_17:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = sales_17->c_get(sales_17, &key, &data, (first_entry_16)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_16 = 0;
         memcpy(sales_17_16.r, (char*)data.data+0, 2);
         *(sales_17_16.r+2) = '\0';
         memcpy(sales_17_16.g, (char*)data.data+2, 2);
         *(sales_17_16.g+2) = '\0';
         memcpy(&(sales_17_16.u), (char*)data.data+4, sizeof(int));
         //printf("Retrieved sales_17_16.u = %d\n", sales_17_16.u);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_16 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0 && slide_out == 1) {
         memcpy(insert_15.r, sales_17_16.r, 2);
         insert_15.r[2]=0;
         memcpy(insert_15.g, sales_17_16.g, 2);
         insert_15.g[2]=0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("\t%s\t ", insert_15.r);
         printf("\t%s\t ", insert_15.g);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (sales_17 && (rc = sales_17->c_close(sales_17)) != 0) {
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
   (void)unlink("_adl_db_sales");
   if ((rc = db_create(&sales, NULL, 0)) != 0) {
      adlabort(rc, "db_create()");
   }
   if ((rc = sales->set_pagesize(sales, 2048)) != 0) {
      adlabort(rc, "set_pagesize()");
   }
   if ((rc = sales->set_flags(sales, DB_DUP)) != 0) {
      adlabort(rc, "set_flags()");
   }
   if ((rc=sales->set_bt_compare(sales, _sales_cmp)) != 0){
      adlabort(rc, "IM_REL->put()");
   }
   if ((rc = sales->open(sales, "_adl_db_sales", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_statement_14();
   _adl_statement_18();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if (sales && ((rc = sales->close(sales, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   sales = NULL;
   (void)unlink("_adl_db_sales");
   return(rc);
};
