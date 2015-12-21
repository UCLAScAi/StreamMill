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
IM_REL *Points;
struct kmean_status {
   IM_REL *Means;
   IM_REL *Distance;
   IM_REL *Result;
   IM_REL *MinDist;
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};

struct MyMin_status {
   IM_REL *Temp;
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void MyMin_init(struct kmean_status *kmean_Inst, struct MyMin_status *status, 
	double MyDist, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void MyMin_iterate(struct kmean_status *kmean_Inst, struct MyMin_status *status, 
	double MyDist, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void MyMin_terminate(struct kmean_status *kmean_Inst, struct MyMin_status *status, 
	double MyDist, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL);
extern "C" void MyMin_init(struct kmean_status *kmean_Inst, struct MyMin_status *status, double MyDist,
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
      sprintf(_adl_dbname, "._%d_Temp", status);
      if ((rc = im_rel_create(&status->Temp, NULL, IM_LINKEDLIST, 0)) != 0) {
         adlabort(rc, "im_rel_create()");
      }
      if ((rc = status->Temp->open(status->Temp, _adl_dbname, 0)) != 0) {
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
      double MyDist;
      double MyDist_expire;
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
         insert_0.MyDist = MyDist;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_0.MyDist), sizeof(double));
         data.size = 8;
         key.size = 0;
         if ((rc = status->Temp->put(status->Temp, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void MyMin_iterate(struct kmean_status *kmean_Inst, struct MyMin_status *status, 
	double MyDist, int _rec_id, bufferMngr* bm, 
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
   IM_RELC *Temp_2;
   int first_entry_3 = 1;
   int first_entry_2 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Temp->cursor(status->Temp, &Temp_2, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double B;
         int OID;
         double B_expire;
         int OID_expire;
         struct timeval atime;
      } Temp_2_3;
      next_2:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = Temp_2->c_get(Temp_2, &key, &data, (first_entry_3)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_3 = 0;
         memcpy(&(Temp_2_3.B), (char*)data.data+0, sizeof(double));
         //printf("Retrieved Temp_2_3.B = %f\n", Temp_2_3.B);
         //fflush(stdout);
         memcpy(&(Temp_2_3.OID), (char*)data.data+8, sizeof(int));
         //printf("Retrieved Temp_2_3.OID = %d\n", Temp_2_3.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_3 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         tmpStore(&key, &data);
         rc = 0;          /* subquery could've overwritten rc */
         if (!((MyDist < Temp_2_3.B))) {
            goto next_2;
         }
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         tmpRecover(&key, &data);
         *(double*)((char*)data.data+0) = MyDist;
         if ((rc = Temp_2->c_put(Temp_2, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (Temp_2 && (rc = Temp_2->c_close(Temp_2)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void MyMin_terminate(struct kmean_status *kmean_Inst, struct MyMin_status *status, 
	double MyDist, int _rec_id, int not_delete, bufferMngr* bm, 
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
   struct {
      double B;
      double B_expire;
      struct timeval atime;
   } insert_4;
   IM_RELC *Temp_6;
   int first_entry_5 = 1;
   int first_entry_6 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Temp->cursor(status->Temp, &Temp_6, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double B;
         int OID;
         double B_expire;
         int OID_expire;
         struct timeval atime;
      } Temp_6_5;
      next_6:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = Temp_6->c_get(Temp_6, &key, &data, (first_entry_5)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_5 = 0;
         memcpy(&(Temp_6_5.B), (char*)data.data+0, sizeof(double));
         //printf("Retrieved Temp_6_5.B = %f\n", Temp_6_5.B);
         //fflush(stdout);
         memcpy(&(Temp_6_5.OID), (char*)data.data+8, sizeof(int));
         //printf("Retrieved Temp_6_5.OID = %d\n", Temp_6_5.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_5 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_4.B = Temp_6_5.B;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_4.B), sizeof(double));
         memcpy((char*)data.data+0, &(insert_4.B), sizeof(double));
         data.size = 8;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (Temp_6 && (rc = Temp_6->c_close(Temp_6)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   sprintf(_adl_dbname, "._%d_Temp", status);
   if(!not_delete) {
      if ((rc = status->Temp->close(status->Temp, 0)) != 0) {
         adlabort(rc, "IM_REL->close()");
      }
   }
   if(!not_delete) status->retc_first_entry=1;
}

extern "C" void kmean_init(struct kmean_status *status, 
	int k, double X, double Y, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void kmean_terminate(struct kmean_status *status, 
	int k, double X, double Y, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL);
extern "C" void kmean_init(struct kmean_status *status, int k, double X, double Y,
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
      sprintf(_adl_dbname, "._%d_Means", status);
      if ((rc = im_rel_create(&status->Means, NULL, IM_LINKEDLIST, 0)) != 0) {
         adlabort(rc, "im_rel_create()");
      }
      if ((rc = status->Means->open(status->Means, _adl_dbname, 0)) != 0) {
         adlabort(rc, "open()");
      }
      sprintf(_adl_dbname, "._%d_Distance", status);
      if ((rc = im_rel_create(&status->Distance, NULL, IM_LINKEDLIST, 0)) != 0) {
         adlabort(rc, "im_rel_create()");
      }
      if ((rc = status->Distance->open(status->Distance, _adl_dbname, 0)) != 0) {
         adlabort(rc, "open()");
      }
      sprintf(_adl_dbname, "._%d_Result", status);
      if ((rc = im_rel_create(&status->Result, NULL, IM_LINKEDLIST, 0)) != 0) {
         adlabort(rc, "im_rel_create()");
      }
      if ((rc = status->Result->open(status->Result, _adl_dbname, 0)) != 0) {
         adlabort(rc, "open()");
      }
      sprintf(_adl_dbname, "._%d_MinDist", status);
      if ((rc = im_rel_create(&status->MinDist, NULL, IM_LINKEDLIST, 0)) != 0) {
         adlabort(rc, "im_rel_create()");
      }
      if ((rc = status->MinDist->open(status->MinDist, _adl_dbname, 0)) != 0) {
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
      double X;
      double Y;
      double field_3;
      double field_4;
      int field_5;
      int field_0_expire;
      double X_expire;
      double Y_expire;
      double field_3_expire;
      double field_4_expire;
      int field_5_expire;
      struct timeval atime;
   } insert_7;
   int first_entry_9 = 1;
   IM_RELC *M_15;
   int first_entry_14 = 1;
   int first_entry_15 = 1;
   int index_13 = 0;
   int terminating_13=0;
   struct gb_status_13 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int count_0_last_out;
      bool count_0_iterate;
      bool count_0_init;
   };
   struct gb_status_13 *gbstatus_13 = (struct gb_status_13 *)0;
   
   int first_entry_13 = 1;
   int first_entry_16 = 1;
   IM_RELC *M_22;
   int first_entry_21 = 1;
   int first_entry_22 = 1;
   int index_20 = 0;
   int terminating_20=0;
   struct gb_status_20 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int count_0_last_out;
      bool count_0_iterate;
      bool count_0_init;
   };
   struct gb_status_20 *gbstatus_20 = (struct gb_status_20 *)0;
   
   int first_entry_20 = 1;
   int first_entry_8 = 1;
   struct {
      int Pno;
      double field_1;
      int Pno_expire;
      double field_1_expire;
      struct timeval atime;
   } insert_24;
   IM_RELC *M_26;
   int first_entry_25 = 1;
   int first_entry_26 = 1;
   struct {
      double X;
      double Y;
      int field_2;
      double X_expire;
      double Y_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_28;
   int first_entry_29 = 1;
   struct {
      double a_0;
      double a_0_expire;
      struct timeval atime;
   } insert_31;
   IM_RELC *Distance_37;
   int first_entry_36 = 1;
   int first_entry_37 = 1;
   int index_35 = 0;
   int terminating_35=0;
   struct gb_status_35 {
      struct MyMin_status *MyMin_0;
   };
   struct gb_status_35 *gbstatus_35 = (struct gb_status_35 *)0;
   
   int first_entry_35 = 1;
   int first_entry_33 = 1;
   IM_RELC *Result_38;
   int first_entry_39 = 1;
   int first_entry_40 = 1;
   IM_RELC *D_40;
   int first_entry_42 = 1;
   int first_entry_43 = 1;
   IM_RELC *MinDist_43;
   int first_entry_45 = 1;
   int first_entry_38 = 1;
   IM_RELC *Means_46;
   int first_entry_47 = 1;
   int first_entry_48 = 1;
   IM_RELC *Result_48;
   int first_entry_50 = 1;
   int first_entry_46 = 1;
   IM_RELC *Means_51;
   int first_entry_52 = 1;
   int first_entry_53 = 1;
   IM_RELC *Result_53;
   int first_entry_55 = 1;
   int first_entry_51 = 1;
   struct {
      double X;
      double Y;
      int C;
      double X_expire;
      double Y_expire;
      int C_expire;
      struct timeval atime;
   } insert_56;
   IM_RELC *R_58;
   int first_entry_57 = 1;
   int first_entry_58 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Means->cursor(status->Means, &M_15, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = status->Means->cursor(status->Means, &M_22, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      next_8:
      rc = (first_entry_8)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_8=1;
      else {
         first_entry_8=0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_10;
         int embed_10_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0001_9_11;
         next_9:
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0000_13_12;
         next_13:
         while (index_13>=0 && index_13 < 2) {
            switch(index_13) {
               case 0:
               {
                  if (terminating_13 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int Pno;
                        double X;
                        double Y;
                        double XSum;
                        double YSum;
                        int Total;
                        int OID;
                        int Pno_expire;
                        double X_expire;
                        double Y_expire;
                        double XSum_expire;
                        double YSum_expire;
                        int Total_expire;
                        int OID_expire;
                        struct timeval atime;
                     } M_15_14;
                     next_15:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = M_15->c_get(M_15, &key, &data, (first_entry_14)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_14 = 0;
                        memcpy(&(M_15_14.Pno), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved M_15_14.Pno = %d\n", M_15_14.Pno);
                        //fflush(stdout);
                        memcpy(&(M_15_14.X), (char*)data.data+4, sizeof(double));
                        //printf("Retrieved M_15_14.X = %f\n", M_15_14.X);
                        //fflush(stdout);
                        memcpy(&(M_15_14.Y), (char*)data.data+12, sizeof(double));
                        //printf("Retrieved M_15_14.Y = %f\n", M_15_14.Y);
                        //fflush(stdout);
                        memcpy(&(M_15_14.XSum), (char*)data.data+20, sizeof(double));
                        //printf("Retrieved M_15_14.XSum = %f\n", M_15_14.XSum);
                        //fflush(stdout);
                        memcpy(&(M_15_14.YSum), (char*)data.data+28, sizeof(double));
                        //printf("Retrieved M_15_14.YSum = %f\n", M_15_14.YSum);
                        //fflush(stdout);
                        memcpy(&(M_15_14.Total), (char*)data.data+36, sizeof(int));
                        //printf("Retrieved M_15_14.Total = %d\n", M_15_14.Total);
                        //fflush(stdout);
                        memcpy(&(M_15_14.OID), (char*)data.data+40, sizeof(int));
                        //printf("Retrieved M_15_14.OID = %d\n", M_15_14.OID);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_14 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        Q_0000_13_12.a_0 = M_15_14.Pno;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_13 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_13 = (struct gb_status_13 *)0;
                        rc = hash_get(13, _rec_id, gbkey, 4, (char**)&gbstatus_13);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_13 = (struct gb_status_13*)malloc(sizeof(*gbstatus_13));
                           gbstatus_13->_baggr_0_first_entry = 1;
                           gbstatus_13->_baggr_0_value = 1;
                           rc = hash_put(13, _rec_id, gbkey, 4, &gbstatus_13);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_13->_baggr_0_first_entry = 1;
                           gbstatus_13->_baggr_0_value += 1;
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_13 = 1;
                     }
                  }
                  if (terminating_13 == 1) {
                     if (first_entry_13 == 1) {
                        rc = 0; /* fail on first entry, aggregate on empty set */
                     } else {
                        allkey = (char*)0;
                        rc = hash_get(13, _rec_id, allkey, 4, (char**)&gbstatus_13);
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
                  if (terminating_13 == 1) {
                     if (gbstatus_13 == (struct gb_status_13 *)0) {
                        if (first_entry_13) {
                           rc = 0;
                           Q_0001_9_11.a_0 = 0;
                        }
                     } else 
                     if (gbstatus_13->_baggr_0_first_entry == 1) {
                        Q_0001_9_11.a_0 = gbstatus_13->_baggr_0_value;
                        gbstatus_13->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_13->_baggr_0_first_entry = 1;
                     }
                  }
                  first_entry_13 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_13++;
            }
            if (rc == DB_NOTFOUND) {
               index_13--;
               if (terminating_13 == 1 && index_13 == 0) {
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_13--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_13 = 0;
            first_entry_13 = 1;
            index_13 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(13, _rec_id, allkey, 4, (char**)&gbstatus_13);
               if (rc==0) {
                  //printf("freeing 13\n");
                  free(gbstatus_13);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(13, _rec_id);
         }
         first_entry_9 = (rc)? 1:0;
         if (rc==0) {
            embed_10.a_0 = Q_0001_9_11.a_0;
            if (embed_10_cnt++ ==0) goto next_9; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_10_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 46.\n");
            exit(1);
         }
         else if (embed_10_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 46.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((embed_10.a_0 < k))) {
            goto next_8;
         }
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_17;
         int embed_17_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0003_16_18;
         next_16:
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0002_20_19;
         next_20:
         while (index_20>=0 && index_20 < 2) {
            switch(index_20) {
               case 0:
               {
                  if (terminating_20 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int Pno;
                        double X;
                        double Y;
                        double XSum;
                        double YSum;
                        int Total;
                        int OID;
                        int Pno_expire;
                        double X_expire;
                        double Y_expire;
                        double XSum_expire;
                        double YSum_expire;
                        int Total_expire;
                        int OID_expire;
                        struct timeval atime;
                     } M_22_21;
                     next_22:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = M_22->c_get(M_22, &key, &data, (first_entry_21)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_21 = 0;
                        memcpy(&(M_22_21.Pno), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved M_22_21.Pno = %d\n", M_22_21.Pno);
                        //fflush(stdout);
                        memcpy(&(M_22_21.X), (char*)data.data+4, sizeof(double));
                        //printf("Retrieved M_22_21.X = %f\n", M_22_21.X);
                        //fflush(stdout);
                        memcpy(&(M_22_21.Y), (char*)data.data+12, sizeof(double));
                        //printf("Retrieved M_22_21.Y = %f\n", M_22_21.Y);
                        //fflush(stdout);
                        memcpy(&(M_22_21.XSum), (char*)data.data+20, sizeof(double));
                        //printf("Retrieved M_22_21.XSum = %f\n", M_22_21.XSum);
                        //fflush(stdout);
                        memcpy(&(M_22_21.YSum), (char*)data.data+28, sizeof(double));
                        //printf("Retrieved M_22_21.YSum = %f\n", M_22_21.YSum);
                        //fflush(stdout);
                        memcpy(&(M_22_21.Total), (char*)data.data+36, sizeof(int));
                        //printf("Retrieved M_22_21.Total = %d\n", M_22_21.Total);
                        //fflush(stdout);
                        memcpy(&(M_22_21.OID), (char*)data.data+40, sizeof(int));
                        //printf("Retrieved M_22_21.OID = %d\n", M_22_21.OID);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_21 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        Q_0002_20_19.a_0 = M_22_21.Pno;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_20 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_20 = (struct gb_status_20 *)0;
                        rc = hash_get(20, _rec_id, gbkey, 4, (char**)&gbstatus_20);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_20 = (struct gb_status_20*)malloc(sizeof(*gbstatus_20));
                           gbstatus_20->_baggr_0_first_entry = 1;
                           gbstatus_20->_baggr_0_value = 1;
                           rc = hash_put(20, _rec_id, gbkey, 4, &gbstatus_20);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_20->_baggr_0_first_entry = 1;
                           gbstatus_20->_baggr_0_value += 1;
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_20 = 1;
                     }
                  }
                  if (terminating_20 == 1) {
                     if (first_entry_20 == 1) {
                        rc = 0; /* fail on first entry, aggregate on empty set */
                     } else {
                        allkey = (char*)0;
                        rc = hash_get(20, _rec_id, allkey, 4, (char**)&gbstatus_20);
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
                  if (terminating_20 == 1) {
                     if (gbstatus_20 == (struct gb_status_20 *)0) {
                        if (first_entry_20) {
                           rc = 0;
                           Q_0003_16_18.a_0 = 0;
                        }
                     } else 
                     if (gbstatus_20->_baggr_0_first_entry == 1) {
                        Q_0003_16_18.a_0 = gbstatus_20->_baggr_0_value;
                        gbstatus_20->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_20->_baggr_0_first_entry = 1;
                     }
                  }
                  first_entry_20 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_20++;
            }
            if (rc == DB_NOTFOUND) {
               index_20--;
               if (terminating_20 == 1 && index_20 == 0) {
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_20--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_20 = 0;
            first_entry_20 = 1;
            index_20 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(20, _rec_id, allkey, 4, (char**)&gbstatus_20);
               if (rc==0) {
                  //printf("freeing 20\n");
                  free(gbstatus_20);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(20, _rec_id);
         }
         first_entry_16 = (rc)? 1:0;
         if (rc==0) {
            embed_17.a_0 = Q_0003_16_18.a_0;
            if (embed_17_cnt++ ==0) goto next_16; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_17_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 45.\n");
            exit(1);
         }
         else if (embed_17_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 45.\n");
            exit(1);
         }
         rc = 0;
         insert_7.field_0 = ((embed_17.a_0) + 1);
         insert_7.X = X;
         insert_7.Y = Y;
         insert_7.field_3 = 0;
         insert_7.field_4 = 0;
         insert_7.field_5 = 0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_7.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_7.X), sizeof(double));
         memcpy((char*)data.data+12, &(insert_7.Y), sizeof(double));
         memcpy((char*)data.data+20, &(insert_7.field_3), sizeof(double));
         memcpy((char*)data.data+28, &(insert_7.field_4), sizeof(double));
         memcpy((char*)data.data+36, &(insert_7.field_5), sizeof(int));
         data.size = 40;
         key.size = 0;
         if ((rc = status->Means->put(status->Means, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (M_15 && (rc = M_15->c_close(M_15)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (M_22 && (rc = M_22->c_close(M_22)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   sprintf(_adl_dbname, "._%d_status->Distance", status);
   
   if (status->Distance && ((rc = status->Distance->close(status->Distance, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   status->Distance = NULL;
   (void)unlink(_adl_dbname);
   if ((rc = im_rel_create(&status->Distance, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = status->Distance->open(status->Distance, _adl_dbname, 0)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Means->cursor(status->Means, &M_26, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int Pno;
         double X;
         double Y;
         double XSum;
         double YSum;
         int Total;
         int OID;
         int Pno_expire;
         double X_expire;
         double Y_expire;
         double XSum_expire;
         double YSum_expire;
         int Total_expire;
         int OID_expire;
         struct timeval atime;
      } M_26_25;
      next_26:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = M_26->c_get(M_26, &key, &data, (first_entry_25)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_25 = 0;
         memcpy(&(M_26_25.Pno), (char*)data.data+0, sizeof(int));
         //printf("Retrieved M_26_25.Pno = %d\n", M_26_25.Pno);
         //fflush(stdout);
         memcpy(&(M_26_25.X), (char*)data.data+4, sizeof(double));
         //printf("Retrieved M_26_25.X = %f\n", M_26_25.X);
         //fflush(stdout);
         memcpy(&(M_26_25.Y), (char*)data.data+12, sizeof(double));
         //printf("Retrieved M_26_25.Y = %f\n", M_26_25.Y);
         //fflush(stdout);
         memcpy(&(M_26_25.XSum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved M_26_25.XSum = %f\n", M_26_25.XSum);
         //fflush(stdout);
         memcpy(&(M_26_25.YSum), (char*)data.data+28, sizeof(double));
         //printf("Retrieved M_26_25.YSum = %f\n", M_26_25.YSum);
         //fflush(stdout);
         memcpy(&(M_26_25.Total), (char*)data.data+36, sizeof(int));
         //printf("Retrieved M_26_25.Total = %d\n", M_26_25.Total);
         //fflush(stdout);
         memcpy(&(M_26_25.OID), (char*)data.data+40, sizeof(int));
         //printf("Retrieved M_26_25.OID = %d\n", M_26_25.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_25 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_24.Pno = M_26_25.Pno;
         insert_24.field_1 = sqrt((double)(((((((M_26_25.X) - X)) * ((M_26_25.X) - X))) + ((((M_26_25.Y) - Y)) * ((M_26_25.Y) - Y)))));
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_24.Pno), sizeof(int));
         memcpy((char*)data.data+4, &(insert_24.field_1), sizeof(double));
         data.size = 12;
         key.size = 0;
         if ((rc = status->Distance->put(status->Distance, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (M_26 && (rc = M_26->c_close(M_26)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   sprintf(_adl_dbname, "._%d_status->Result", status);
   
   if (status->Result && ((rc = status->Result->close(status->Result, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   status->Result = NULL;
   (void)unlink(_adl_dbname);
   if ((rc = im_rel_create(&status->Result, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = status->Result->open(status->Result, _adl_dbname, 0)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_29:
      rc = (first_entry_29)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_29=1;
      else {
         first_entry_29=0;
         insert_28.X = X;
         insert_28.Y = Y;
         insert_28.field_2 = -1;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_28.X), sizeof(double));
         memcpy((char*)data.data+8, &(insert_28.Y), sizeof(double));
         memcpy((char*)data.data+16, &(insert_28.field_2), sizeof(int));
         data.size = 20;
         key.size = 0;
         if ((rc = status->Result->put(status->Result, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   sprintf(_adl_dbname, "._%d_status->MinDist", status);
   
   if (status->MinDist && ((rc = status->MinDist->close(status->MinDist, 0)) != 0)) {
      adlabort(rc, "DB->close()");
   }
   status->MinDist = NULL;
   (void)unlink(_adl_dbname);
   if ((rc = im_rel_create(&status->MinDist, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = status->MinDist->open(status->MinDist, _adl_dbname, 0)) != 0) {
      adlabort(rc, "open()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Distance->cursor(status->Distance, &Distance_37, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double a_0;
         double a_0_expire;
         struct timeval atime;
      } Q_0005_33_32;
      next_33:
      struct {
         double a_0;
         double a_0_expire;
         struct timeval atime;
      } Q_0004_35_34;
      next_35:
      while (index_35>=0 && index_35 < 2) {
         switch(index_35) {
            case 0:
            {
               if (terminating_35 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int Pno;
                     double Dist;
                     int OID;
                     int Pno_expire;
                     double Dist_expire;
                     int OID_expire;
                     struct timeval atime;
                  } Distance_37_36;
                  next_37:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = Distance_37->c_get(Distance_37, &key, &data, (first_entry_36)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_36 = 0;
                     memcpy(&(Distance_37_36.Pno), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved Distance_37_36.Pno = %d\n", Distance_37_36.Pno);
                     //fflush(stdout);
                     memcpy(&(Distance_37_36.Dist), (char*)data.data+4, sizeof(double));
                     //printf("Retrieved Distance_37_36.Dist = %f\n", Distance_37_36.Dist);
                     //fflush(stdout);
                     memcpy(&(Distance_37_36.OID), (char*)data.data+12, sizeof(int));
                     //printf("Retrieved Distance_37_36.OID = %d\n", Distance_37_36.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_36 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0004_35_34.a_0 = Distance_37_36.Dist;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_35 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_35 = (struct gb_status_35 *)0;
                     rc = hash_get(35, _rec_id, gbkey, 4, (char**)&gbstatus_35);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_35 = (struct gb_status_35*)malloc(sizeof(*gbstatus_35));
                        gbstatus_35->MyMin_0 = (struct MyMin_status*)malloc(sizeof(struct MyMin_status));
                        gbstatus_35->MyMin_0->win = 0;
                        MyMin_init(status, gbstatus_35->MyMin_0, Q_0004_35_34.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, _modelId);
                        rc = hash_put(35, _rec_id, gbkey, 4, &gbstatus_35);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        MyMin_iterate(status, gbstatus_35->MyMin_0, Q_0004_35_34.a_0, _rec_id+1, NULL, inMemTables, NULL, 0, _modelId);
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_35 = 1;
                  }
               }
               if (terminating_35 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(35, _rec_id, allkey, 4, (char**)&gbstatus_35);
                  if (rc==0) {
                     MyMin_terminate(status, gbstatus_35->MyMin_0, Q_0004_35_34.a_0, _rec_id+1, 1, NULL, inMemTables, NULL, 0, _modelId);
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_35->MyMin_0->retc->c_get(gbstatus_35->MyMin_0->retc, &key, &data, (gbstatus_35->MyMin_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_35->MyMin_0->retc_first_entry = 0;
                  memcpy(&(Q_0005_33_32.a_0), (char*)data.data+0, sizeof(double));
                  //printf("Retrieved Q_0005_33_32.a_0 = %f\n", Q_0005_33_32.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_35->MyMin_0->retc->c_del(gbstatus_35->MyMin_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_35->MyMin_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_35 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_35++;
         }
         if (rc == DB_NOTFOUND) {
            index_35--;
            if (terminating_35 == 1 && index_35 == 0) {
               if (gbstatus_35->MyMin_0->retc && (rc = gbstatus_35->MyMin_0->retc->c_close(gbstatus_35->MyMin_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_35->MyMin_0);
               
               if (gbstatus_35->MyMin_0->ret && ((rc = gbstatus_35->MyMin_0->ret->close(gbstatus_35->MyMin_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_35->MyMin_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_35--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_35 = 0;
         first_entry_35 = 1;
         index_35 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(35, _rec_id, allkey, 4, (char**)&gbstatus_35);
            if (rc==0) {
               free(gbstatus_35->MyMin_0);
               //printf("freeing 35\n");
               free(gbstatus_35);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(35, _rec_id);
      }
      if (rc==0) {
         insert_31.a_0 = Q_0005_33_32.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_31.a_0), sizeof(double));
         data.size = 8;
         key.size = 0;
         if ((rc = status->MinDist->put(status->MinDist, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (Distance_37 && (rc = Distance_37->c_close(Distance_37)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Result->cursor(status->Result, &Result_38, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = status->Distance->cursor(status->Distance, &D_40, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = status->MinDist->cursor(status->MinDist, &MinDist_43, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double X;
         double Y;
         int C;
         int OID;
         double X_expire;
         double Y_expire;
         int C_expire;
         int OID_expire;
         struct timeval atime;
      } Result_38_39;
      next_38:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = Result_38->c_get(Result_38, &key, &data, (first_entry_39)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_39 = 0;
         memcpy(&(Result_38_39.X), (char*)data.data+0, sizeof(double));
         //printf("Retrieved Result_38_39.X = %f\n", Result_38_39.X);
         //fflush(stdout);
         memcpy(&(Result_38_39.Y), (char*)data.data+8, sizeof(double));
         //printf("Retrieved Result_38_39.Y = %f\n", Result_38_39.Y);
         //fflush(stdout);
         memcpy(&(Result_38_39.C), (char*)data.data+16, sizeof(int));
         //printf("Retrieved Result_38_39.C = %d\n", Result_38_39.C);
         //fflush(stdout);
         memcpy(&(Result_38_39.OID), (char*)data.data+20, sizeof(int));
         //printf("Retrieved Result_38_39.OID = %d\n", Result_38_39.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_39 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
          /* SUBQUERY IN UPDATE STARTS */
         kdPush(&key,&data);
         struct {
            int Pno;
            int Pno_expire;
            struct timeval atime;
         } embed_41;
         int embed_41_cnt = 0;
         struct {
            int Pno;
            double Dist;
            int OID;
            int Pno_expire;
            double Dist_expire;
            int OID_expire;
            struct timeval atime;
         } D_40_42;
         next_40:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = D_40->c_get(D_40, &key, &data, (first_entry_42)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_42 = 0;
            memcpy(&(D_40_42.Pno), (char*)data.data+0, sizeof(int));
            //printf("Retrieved D_40_42.Pno = %d\n", D_40_42.Pno);
            //fflush(stdout);
            memcpy(&(D_40_42.Dist), (char*)data.data+4, sizeof(double));
            //printf("Retrieved D_40_42.Dist = %f\n", D_40_42.Dist);
            //fflush(stdout);
            memcpy(&(D_40_42.OID), (char*)data.data+12, sizeof(int));
            //printf("Retrieved D_40_42.OID = %d\n", D_40_42.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_42 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            struct {
               double Val;
               double Val_expire;
               struct timeval atime;
            } embed_44;
            int embed_44_cnt = 0;
            struct {
               double Val;
               int OID;
               double Val_expire;
               int OID_expire;
               struct timeval atime;
            } MinDist_43_45;
            next_43:
            memset(&key, 0, sizeof(key));
            memset(&data, 0, sizeof(data));
            rc = MinDist_43->c_get(MinDist_43, &key, &data, (first_entry_45)? DB_FIRST:DB_NEXT);
            if (rc==0) {
               first_entry_45 = 0;
               memcpy(&(MinDist_43_45.Val), (char*)data.data+0, sizeof(double));
               //printf("Retrieved MinDist_43_45.Val = %f\n", MinDist_43_45.Val);
               //fflush(stdout);
               memcpy(&(MinDist_43_45.OID), (char*)data.data+8, sizeof(int));
               //printf("Retrieved MinDist_43_45.OID = %d\n", MinDist_43_45.OID);
               //fflush(stdout);
            } else if (rc == DB_NOTFOUND) {
               first_entry_45 = 1;
            } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            first_entry_43 = (rc)? 1:0;
            if (rc==0) {
               embed_44.Val = MinDist_43_45.Val;
               if (embed_44_cnt++ ==0) goto next_43; /* scalar opr */
            } /* if (rc == 0) */
            if (embed_44_cnt == 0) {
               fprintf(stderr, "ERR: scalar subquery returns no tuple at line 64.\n");
               exit(1);
            }
            else if (embed_44_cnt >  1) {
               fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 64.\n");
               exit(1);
            }
            rc = 0;
            rc = 0;          /* subquery could've overwritten rc */
            if (!((D_40_42.Dist == embed_44.Val))) {
               goto next_40;
            }
            embed_41.Pno = D_40_42.Pno;
         } /* if (rc == 0) */
         kdPop(&key, &data);
         /* SUBQUERY IN UPDATE ENDS */
         *(int*)((char*)data.data+16) = embed_41.Pno;
         if ((rc = Result_38->c_put(Result_38, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (Result_38 && (rc = Result_38->c_close(Result_38)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (D_40 && (rc = D_40->c_close(D_40)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (MinDist_43 && (rc = MinDist_43->c_close(MinDist_43)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Means->cursor(status->Means, &Means_46, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = status->Result->cursor(status->Result, &Result_48, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int Pno;
         double X;
         double Y;
         double XSum;
         double YSum;
         int Total;
         int OID;
         int Pno_expire;
         double X_expire;
         double Y_expire;
         double XSum_expire;
         double YSum_expire;
         int Total_expire;
         int OID_expire;
         struct timeval atime;
      } Means_46_47;
      next_46:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = Means_46->c_get(Means_46, &key, &data, (first_entry_47)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_47 = 0;
         memcpy(&(Means_46_47.Pno), (char*)data.data+0, sizeof(int));
         //printf("Retrieved Means_46_47.Pno = %d\n", Means_46_47.Pno);
         //fflush(stdout);
         memcpy(&(Means_46_47.X), (char*)data.data+4, sizeof(double));
         //printf("Retrieved Means_46_47.X = %f\n", Means_46_47.X);
         //fflush(stdout);
         memcpy(&(Means_46_47.Y), (char*)data.data+12, sizeof(double));
         //printf("Retrieved Means_46_47.Y = %f\n", Means_46_47.Y);
         //fflush(stdout);
         memcpy(&(Means_46_47.XSum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved Means_46_47.XSum = %f\n", Means_46_47.XSum);
         //fflush(stdout);
         memcpy(&(Means_46_47.YSum), (char*)data.data+28, sizeof(double));
         //printf("Retrieved Means_46_47.YSum = %f\n", Means_46_47.YSum);
         //fflush(stdout);
         memcpy(&(Means_46_47.Total), (char*)data.data+36, sizeof(int));
         //printf("Retrieved Means_46_47.Total = %d\n", Means_46_47.Total);
         //fflush(stdout);
         memcpy(&(Means_46_47.OID), (char*)data.data+40, sizeof(int));
         //printf("Retrieved Means_46_47.OID = %d\n", Means_46_47.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_47 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         tmpStore(&key, &data);
         struct {
            int C;
            int C_expire;
            struct timeval atime;
         } embed_49;
         int embed_49_cnt = 0;
         struct {
            double X;
            double Y;
            int C;
            int OID;
            double X_expire;
            double Y_expire;
            int C_expire;
            int OID_expire;
            struct timeval atime;
         } Result_48_50;
         next_48:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = Result_48->c_get(Result_48, &key, &data, (first_entry_50)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_50 = 0;
            memcpy(&(Result_48_50.X), (char*)data.data+0, sizeof(double));
            //printf("Retrieved Result_48_50.X = %f\n", Result_48_50.X);
            //fflush(stdout);
            memcpy(&(Result_48_50.Y), (char*)data.data+8, sizeof(double));
            //printf("Retrieved Result_48_50.Y = %f\n", Result_48_50.Y);
            //fflush(stdout);
            memcpy(&(Result_48_50.C), (char*)data.data+16, sizeof(int));
            //printf("Retrieved Result_48_50.C = %d\n", Result_48_50.C);
            //fflush(stdout);
            memcpy(&(Result_48_50.OID), (char*)data.data+20, sizeof(int));
            //printf("Retrieved Result_48_50.OID = %d\n", Result_48_50.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_50 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         first_entry_48 = (rc)? 1:0;
         if (rc==0) {
            embed_49.C = Result_48_50.C;
            if (embed_49_cnt++ ==0) goto next_48; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_49_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 71.\n");
            exit(1);
         }
         else if (embed_49_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 71.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((Means_46_47.Pno == embed_49.C))) {
            goto next_46;
         }
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         tmpRecover(&key, &data);
         *(int*)((char*)data.data+36) = ((Means_46_47.Total) + 1);
         *(double*)((char*)data.data+20) = ((Means_46_47.XSum) + X);
         *(double*)((char*)data.data+28) = ((Means_46_47.YSum) + Y);
         if ((rc = Means_46->c_put(Means_46, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (Means_46 && (rc = Means_46->c_close(Means_46)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (Result_48 && (rc = Result_48->c_close(Result_48)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Means->cursor(status->Means, &Means_51, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = status->Result->cursor(status->Result, &Result_53, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int Pno;
         double X;
         double Y;
         double XSum;
         double YSum;
         int Total;
         int OID;
         int Pno_expire;
         double X_expire;
         double Y_expire;
         double XSum_expire;
         double YSum_expire;
         int Total_expire;
         int OID_expire;
         struct timeval atime;
      } Means_51_52;
      next_51:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = Means_51->c_get(Means_51, &key, &data, (first_entry_52)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_52 = 0;
         memcpy(&(Means_51_52.Pno), (char*)data.data+0, sizeof(int));
         //printf("Retrieved Means_51_52.Pno = %d\n", Means_51_52.Pno);
         //fflush(stdout);
         memcpy(&(Means_51_52.X), (char*)data.data+4, sizeof(double));
         //printf("Retrieved Means_51_52.X = %f\n", Means_51_52.X);
         //fflush(stdout);
         memcpy(&(Means_51_52.Y), (char*)data.data+12, sizeof(double));
         //printf("Retrieved Means_51_52.Y = %f\n", Means_51_52.Y);
         //fflush(stdout);
         memcpy(&(Means_51_52.XSum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved Means_51_52.XSum = %f\n", Means_51_52.XSum);
         //fflush(stdout);
         memcpy(&(Means_51_52.YSum), (char*)data.data+28, sizeof(double));
         //printf("Retrieved Means_51_52.YSum = %f\n", Means_51_52.YSum);
         //fflush(stdout);
         memcpy(&(Means_51_52.Total), (char*)data.data+36, sizeof(int));
         //printf("Retrieved Means_51_52.Total = %d\n", Means_51_52.Total);
         //fflush(stdout);
         memcpy(&(Means_51_52.OID), (char*)data.data+40, sizeof(int));
         //printf("Retrieved Means_51_52.OID = %d\n", Means_51_52.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_52 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         tmpStore(&key, &data);
         struct {
            int C;
            int C_expire;
            struct timeval atime;
         } embed_54;
         int embed_54_cnt = 0;
         struct {
            double X;
            double Y;
            int C;
            int OID;
            double X_expire;
            double Y_expire;
            int C_expire;
            int OID_expire;
            struct timeval atime;
         } Result_53_55;
         next_53:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = Result_53->c_get(Result_53, &key, &data, (first_entry_55)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_55 = 0;
            memcpy(&(Result_53_55.X), (char*)data.data+0, sizeof(double));
            //printf("Retrieved Result_53_55.X = %f\n", Result_53_55.X);
            //fflush(stdout);
            memcpy(&(Result_53_55.Y), (char*)data.data+8, sizeof(double));
            //printf("Retrieved Result_53_55.Y = %f\n", Result_53_55.Y);
            //fflush(stdout);
            memcpy(&(Result_53_55.C), (char*)data.data+16, sizeof(int));
            //printf("Retrieved Result_53_55.C = %d\n", Result_53_55.C);
            //fflush(stdout);
            memcpy(&(Result_53_55.OID), (char*)data.data+20, sizeof(int));
            //printf("Retrieved Result_53_55.OID = %d\n", Result_53_55.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_55 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         first_entry_53 = (rc)? 1:0;
         if (rc==0) {
            embed_54.C = Result_53_55.C;
            if (embed_54_cnt++ ==0) goto next_53; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_54_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 77.\n");
            exit(1);
         }
         else if (embed_54_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 77.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((Means_51_52.Pno == embed_54.C))) {
            goto next_51;
         }
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         tmpRecover(&key, &data);
         *(double*)((char*)data.data+4) = ((Means_51_52.XSum) / Means_51_52.Total);
         *(double*)((char*)data.data+12) = ((Means_51_52.YSum) / Means_51_52.Total);
         if ((rc = Means_51->c_put(Means_51, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (Means_51 && (rc = Means_51->c_close(Means_51)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (Result_53 && (rc = Result_53->c_close(Result_53)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Result->cursor(status->Result, &R_58, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double X;
         double Y;
         int C;
         int OID;
         double X_expire;
         double Y_expire;
         int C_expire;
         int OID_expire;
         struct timeval atime;
      } R_58_57;
      next_58:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = R_58->c_get(R_58, &key, &data, (first_entry_57)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_57 = 0;
         memcpy(&(R_58_57.X), (char*)data.data+0, sizeof(double));
         //printf("Retrieved R_58_57.X = %f\n", R_58_57.X);
         //fflush(stdout);
         memcpy(&(R_58_57.Y), (char*)data.data+8, sizeof(double));
         //printf("Retrieved R_58_57.Y = %f\n", R_58_57.Y);
         //fflush(stdout);
         memcpy(&(R_58_57.C), (char*)data.data+16, sizeof(int));
         //printf("Retrieved R_58_57.C = %d\n", R_58_57.C);
         //fflush(stdout);
         memcpy(&(R_58_57.OID), (char*)data.data+20, sizeof(int));
         //printf("Retrieved R_58_57.OID = %d\n", R_58_57.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_57 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_56.X = R_58_57.X;
         insert_56.Y = R_58_57.Y;
         insert_56.C = R_58_57.C;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_56.X), sizeof(double));
         memcpy((char*)data.data+0, &(insert_56.X), sizeof(double));
         memcpy((char*)data.data+8, &(insert_56.Y), sizeof(double));
         memcpy((char*)data.data+16, &(insert_56.C), sizeof(int));
         data.size = 20;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (R_58 && (rc = R_58->c_close(R_58)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void kmean_terminate(struct kmean_status *status, 
	int k, double X, double Y, int _rec_id, int not_delete, bufferMngr* bm, 
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
   struct {
      int Pno;
      double X;
      double Y;
      double XSum;
      double YSum;
      int Total;
      int Pno_expire;
      double X_expire;
      double Y_expire;
      double XSum_expire;
      double YSum_expire;
      int Total_expire;
      struct timeval atime;
   } insert_59;
   IM_RELC *M_61;
   int first_entry_60 = 1;
   int first_entry_61 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = status->Means->cursor(status->Means, &M_61, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int Pno;
         double X;
         double Y;
         double XSum;
         double YSum;
         int Total;
         int OID;
         int Pno_expire;
         double X_expire;
         double Y_expire;
         double XSum_expire;
         double YSum_expire;
         int Total_expire;
         int OID_expire;
         struct timeval atime;
      } M_61_60;
      next_61:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = M_61->c_get(M_61, &key, &data, (first_entry_60)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_60 = 0;
         memcpy(&(M_61_60.Pno), (char*)data.data+0, sizeof(int));
         //printf("Retrieved M_61_60.Pno = %d\n", M_61_60.Pno);
         //fflush(stdout);
         memcpy(&(M_61_60.X), (char*)data.data+4, sizeof(double));
         //printf("Retrieved M_61_60.X = %f\n", M_61_60.X);
         //fflush(stdout);
         memcpy(&(M_61_60.Y), (char*)data.data+12, sizeof(double));
         //printf("Retrieved M_61_60.Y = %f\n", M_61_60.Y);
         //fflush(stdout);
         memcpy(&(M_61_60.XSum), (char*)data.data+20, sizeof(double));
         //printf("Retrieved M_61_60.XSum = %f\n", M_61_60.XSum);
         //fflush(stdout);
         memcpy(&(M_61_60.YSum), (char*)data.data+28, sizeof(double));
         //printf("Retrieved M_61_60.YSum = %f\n", M_61_60.YSum);
         //fflush(stdout);
         memcpy(&(M_61_60.Total), (char*)data.data+36, sizeof(int));
         //printf("Retrieved M_61_60.Total = %d\n", M_61_60.Total);
         //fflush(stdout);
         memcpy(&(M_61_60.OID), (char*)data.data+40, sizeof(int));
         //printf("Retrieved M_61_60.OID = %d\n", M_61_60.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_60 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         insert_59.Pno = M_61_60.Pno;
         insert_59.X = M_61_60.X;
         insert_59.Y = M_61_60.Y;
         insert_59.XSum = M_61_60.XSum;
         insert_59.YSum = M_61_60.YSum;
         insert_59.Total = M_61_60.Total;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_59.Pno);
         printf("%10f ", insert_59.X);
         printf("%10f ", insert_59.Y);
         printf("%10f ", insert_59.XSum);
         printf("%10f ", insert_59.YSum);
         printf("%10d ", insert_59.Total);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (M_61 && (rc = M_61->c_close(M_61)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   sprintf(_adl_dbname, "._%d_Means", status);
   if(!not_delete) {
      if ((rc = status->Means->close(status->Means, 0)) != 0) {
         adlabort(rc, "IM_REL->close()");
      }
   }
   sprintf(_adl_dbname, "._%d_Distance", status);
   if(!not_delete) {
      if ((rc = status->Distance->close(status->Distance, 0)) != 0) {
         adlabort(rc, "IM_REL->close()");
      }
   }
   sprintf(_adl_dbname, "._%d_Result", status);
   if(!not_delete) {
      if ((rc = status->Result->close(status->Result, 0)) != 0) {
         adlabort(rc, "IM_REL->close()");
      }
   }
   sprintf(_adl_dbname, "._%d_MinDist", status);
   if(!not_delete) {
      if ((rc = status->MinDist->close(status->MinDist, 0)) != 0) {
         adlabort(rc, "IM_REL->close()");
      }
   }
   if(!not_delete) status->retc_first_entry=1;
}
struct inWinType_kmean_window {
   int k;
   double X;
   double Y;
};
struct kmean_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_kmean_window* getLastTuple_kmean_window(IM_REL* inwindow, inWinType_kmean_window* tuple, bufferMngr* bm) {
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
   memcpy(&((*tuple).k), (char*)data.data+0, sizeof(int));
   //printf("Retrieved (*tuple).k = %d\n", (*tuple).k);
   //fflush(stdout);
   memcpy(&((*tuple).X), (char*)data.data+4, sizeof(double));
   //printf("Retrieved (*tuple).X = %f\n", (*tuple).X);
   //fflush(stdout);
   memcpy(&((*tuple).Y), (char*)data.data+12, sizeof(double));
   //printf("Retrieved (*tuple).Y = %f\n", (*tuple).Y);
   //fflush(stdout);
   return tuple;
}
extern "C" void kmean_window_init(struct kmean_window_status *status, 
	int k, double X, double Y, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void kmean_window_init(struct kmean_window_status *status, int k, double X, double Y, 
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
   struct inWinType_kmean_window tuple;
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
      double a_0;
      double a_1;
      int a_2;
      double a_0_expire;
      double a_1_expire;
      int a_2_expire;
      struct timeval atime;
   } insert_62;
   IM_RELC *w_68;
   int first_entry_67 = 1;
   int first_entry_68 = 1;
   int index_66 = 0;
   int terminating_66=0;
   struct gb_status_66 {
      struct kmean_status *kmean_0;
   };
   struct gb_status_66 *gbstatus_66 = (struct gb_status_66 *)0;
   
   int first_entry_66 = 1;
   int first_entry_64 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_68, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         double a_0;
         double a_1;
         int a_2;
         double a_0_expire;
         double a_1_expire;
         int a_2_expire;
         struct timeval atime;
      } Q_0007_64_63;
      next_64:
      struct {
         int a_0;
         double a_1;
         double a_2;
         int a_0_expire;
         double a_1_expire;
         double a_2_expire;
         struct timeval atime;
      } Q_0006_66_65;
      next_66:
      while (index_66>=0 && index_66 < 2) {
         switch(index_66) {
            case 0:
            {
               if (terminating_66 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int k;
                     double X;
                     double Y;
                     int k_expire;
                     double X_expire;
                     double Y_expire;
                     struct timeval atime;
                  } w_68_67;
                  next_68:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_68->c_get(w_68, &key, &data, (first_entry_67)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_67 = 0;
                     memcpy(&(w_68_67.k), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_68_67.k = %d\n", w_68_67.k);
                     //fflush(stdout);
                     memcpy(&(w_68_67.X), (char*)data.data+4, sizeof(double));
                     //printf("Retrieved w_68_67.X = %f\n", w_68_67.X);
                     //fflush(stdout);
                     memcpy(&(w_68_67.Y), (char*)data.data+12, sizeof(double));
                     //printf("Retrieved w_68_67.Y = %f\n", w_68_67.Y);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_67 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0006_66_65.a_0 = w_68_67.k;
                     Q_0006_66_65.a_1 = w_68_67.X;
                     Q_0006_66_65.a_2 = w_68_67.Y;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_66 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_66 = (struct gb_status_66 *)0;
                     rc = hash_get(66, _rec_id, gbkey, 4, (char**)&gbstatus_66);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_66 = (struct gb_status_66*)malloc(sizeof(*gbstatus_66));
                        gbstatus_66->kmean_0 = (struct kmean_status*)malloc(sizeof(struct kmean_status));
                        gbstatus_66->kmean_0->win = 0;
                        setModelId("");
                        kmean_init(gbstatus_66->kmean_0, Q_0006_66_65.a_0, Q_0006_66_65.a_1, Q_0006_66_65.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(66, _rec_id, gbkey, 4, &gbstatus_66);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        kmean_init(gbstatus_66->kmean_0, Q_0006_66_65.a_0, Q_0006_66_65.a_1, Q_0006_66_65.a_2, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_66 = 1;
                  }
               }
               if (terminating_66 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(66, _rec_id, allkey, 4, (char**)&gbstatus_66);
                  if (rc==0) {
                     setModelId("");
                     kmean_terminate(gbstatus_66->kmean_0, Q_0006_66_65.a_0, Q_0006_66_65.a_1, Q_0006_66_65.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_66->kmean_0->retc->c_get(gbstatus_66->kmean_0->retc, &key, &data, (gbstatus_66->kmean_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_66->kmean_0->retc_first_entry = 0;
                  memcpy(&(Q_0007_64_63.a_0), (char*)data.data+0, sizeof(double));
                  //printf("Retrieved Q_0007_64_63.a_0 = %f\n", Q_0007_64_63.a_0);
                  //fflush(stdout);
                  memcpy(&(Q_0007_64_63.a_1), (char*)data.data+8, sizeof(double));
                  //printf("Retrieved Q_0007_64_63.a_1 = %f\n", Q_0007_64_63.a_1);
                  //fflush(stdout);
                  memcpy(&(Q_0007_64_63.a_2), (char*)data.data+16, sizeof(int));
                  //printf("Retrieved Q_0007_64_63.a_2 = %d\n", Q_0007_64_63.a_2);
                  //fflush(stdout);
                  if ((rc = gbstatus_66->kmean_0->retc->c_del(gbstatus_66->kmean_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_66->kmean_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_66 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_66++;
         }
         if (rc == DB_NOTFOUND) {
            index_66--;
            if (terminating_66 == 1 && index_66 == 0) {
               if (gbstatus_66->kmean_0->retc && (rc = gbstatus_66->kmean_0->retc->c_close(gbstatus_66->kmean_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_66->kmean_0);
               
               if (gbstatus_66->kmean_0->ret && ((rc = gbstatus_66->kmean_0->ret->close(gbstatus_66->kmean_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_66->kmean_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_66--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_66 = 0;
         first_entry_66 = 1;
         index_66 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(66, _rec_id, allkey, 4, (char**)&gbstatus_66);
            if (rc==0) {
               free(gbstatus_66->kmean_0);
               //printf("freeing 66\n");
               free(gbstatus_66);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(66, _rec_id);
      }
      if (rc==0) {
         insert_62.a_0 = Q_0007_64_63.a_0;
         insert_62.a_1 = Q_0007_64_63.a_1;
         insert_62.a_2 = Q_0007_64_63.a_2;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_62.a_0), sizeof(double));
         memcpy((char*)data.data+0, &(insert_62.a_0), sizeof(double));
         memcpy((char*)data.data+8, &(insert_62.a_1), sizeof(double));
         memcpy((char*)data.data+16, &(insert_62.a_2), sizeof(int));
         data.size = 20;
         key.size = 8;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_68 && (rc = w_68->c_close(w_68)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
/**** Query Declarations ****/
int _adl_statement_69()
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
      FILE *_adl_load = fopen("./test1", "rt");
      char _adl_load_buf[40960], *tok;
      char loadkeybuf[1], loaddatabuf[21];
      int _adl_line_no=0;
      char bPoint =0;
      if (!_adl_load) {
         printf("can not open file ./test1.\n");
         exit(1);
      }
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      key.data = loadkeybuf;
      data.data = loaddatabuf;
      key.size = 0;
      data.size = 20;
      while (fgets(_adl_load_buf, 40959, _adl_load)) {
         _adl_line_no++;
         tok = csvtok(_adl_load_buf, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(double*)((char*)data.data+0) = atof(tok);
         tok = csvtok(NULL, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(double*)((char*)data.data+8) = atof(tok);
         if ((rc = Points->put(Points, &key, &data, DB_APPEND))!=0) {
            exit(rc);
         }
      } /* end of while */
      fclose(_adl_load);
   }
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_77()
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
      double a_0;
      double a_1;
      int a_2;
      double a_0_expire;
      double a_1_expire;
      int a_2_expire;
      struct timeval atime;
   } insert_70;
   IM_RELC *Points_76;
   int first_entry_75 = 1;
   int first_entry_76 = 1;
   int index_74 = 0;
   int terminating_74=0;
   struct gb_status_74 {
      struct kmean_status *kmean_0;
   };
   struct gb_status_74 *gbstatus_74 = (struct gb_status_74 *)0;
   
   int first_entry_74 = 1;
   int first_entry_72 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = Points->cursor(Points, &Points_76, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double a_0;
         double a_1;
         int a_2;
         double a_0_expire;
         double a_1_expire;
         int a_2_expire;
         struct timeval atime;
      } Q_0009_72_71;
      next_72:
      struct {
         int a_0;
         double a_1;
         double a_2;
         int a_0_expire;
         double a_1_expire;
         double a_2_expire;
         struct timeval atime;
      } Q_0008_74_73;
      next_74:
      while (index_74>=0 && index_74 < 2) {
         switch(index_74) {
            case 0:
            {
               if (terminating_74 == 0) {
                  /* get source tuple from qun */
                  struct {
                     double X;
                     double Y;
                     int OID;
                     double X_expire;
                     double Y_expire;
                     int OID_expire;
                     struct timeval atime;
                  } Points_76_75;
                  next_76:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = Points_76->c_get(Points_76, &key, &data, (first_entry_75)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_75 = 0;
                     memcpy(&(Points_76_75.X), (char*)data.data+0, sizeof(double));
                     //printf("Retrieved Points_76_75.X = %f\n", Points_76_75.X);
                     //fflush(stdout);
                     memcpy(&(Points_76_75.Y), (char*)data.data+8, sizeof(double));
                     //printf("Retrieved Points_76_75.Y = %f\n", Points_76_75.Y);
                     //fflush(stdout);
                     memcpy(&(Points_76_75.OID), (char*)data.data+16, sizeof(int));
                     //printf("Retrieved Points_76_75.OID = %d\n", Points_76_75.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_75 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0008_74_73.a_0 = 2;
                     Q_0008_74_73.a_1 = Points_76_75.X;
                     Q_0008_74_73.a_2 = Points_76_75.Y;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_74 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_74 = (struct gb_status_74 *)0;
                     rc = hash_get(74, _rec_id, gbkey, 4, (char**)&gbstatus_74);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_74 = (struct gb_status_74*)malloc(sizeof(*gbstatus_74));
                        gbstatus_74->kmean_0 = (struct kmean_status*)malloc(sizeof(struct kmean_status));
                        gbstatus_74->kmean_0->win = 0;
                        setModelId("");
                        kmean_init(gbstatus_74->kmean_0, Q_0008_74_73.a_0, Q_0008_74_73.a_1, Q_0008_74_73.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(74, _rec_id, gbkey, 4, &gbstatus_74);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        kmean_init(gbstatus_74->kmean_0, Q_0008_74_73.a_0, Q_0008_74_73.a_1, Q_0008_74_73.a_2, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_74 = 1;
                  }
               }
               if (terminating_74 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(74, _rec_id, allkey, 4, (char**)&gbstatus_74);
                  if (rc==0) {
                     setModelId("");
                     kmean_terminate(gbstatus_74->kmean_0, Q_0008_74_73.a_0, Q_0008_74_73.a_1, Q_0008_74_73.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_74->kmean_0->retc->c_get(gbstatus_74->kmean_0->retc, &key, &data, (gbstatus_74->kmean_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_74->kmean_0->retc_first_entry = 0;
                  memcpy(&(Q_0009_72_71.a_0), (char*)data.data+0, sizeof(double));
                  //printf("Retrieved Q_0009_72_71.a_0 = %f\n", Q_0009_72_71.a_0);
                  //fflush(stdout);
                  memcpy(&(Q_0009_72_71.a_1), (char*)data.data+8, sizeof(double));
                  //printf("Retrieved Q_0009_72_71.a_1 = %f\n", Q_0009_72_71.a_1);
                  //fflush(stdout);
                  memcpy(&(Q_0009_72_71.a_2), (char*)data.data+16, sizeof(int));
                  //printf("Retrieved Q_0009_72_71.a_2 = %d\n", Q_0009_72_71.a_2);
                  //fflush(stdout);
                  if ((rc = gbstatus_74->kmean_0->retc->c_del(gbstatus_74->kmean_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_74->kmean_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_74 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_74++;
         }
         if (rc == DB_NOTFOUND) {
            index_74--;
            if (terminating_74 == 1 && index_74 == 0) {
               if (gbstatus_74->kmean_0->retc && (rc = gbstatus_74->kmean_0->retc->c_close(gbstatus_74->kmean_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_74->kmean_0);
               
               if (gbstatus_74->kmean_0->ret && ((rc = gbstatus_74->kmean_0->ret->close(gbstatus_74->kmean_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_74->kmean_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_74--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_74 = 0;
         first_entry_74 = 1;
         index_74 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(74, _rec_id, allkey, 4, (char**)&gbstatus_74);
            if (rc==0) {
               free(gbstatus_74->kmean_0);
               //printf("freeing 74\n");
               free(gbstatus_74);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(74, _rec_id);
      }
      if (rc==0) {
         insert_70.a_0 = Q_0009_72_71.a_0;
         insert_70.a_1 = Q_0009_72_71.a_1;
         insert_70.a_2 = Q_0009_72_71.a_2;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10f ", insert_70.a_0);
         printf("%10f ", insert_70.a_1);
         printf("%10d ", insert_70.a_2);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (Points_76 && (rc = Points_76->c_close(Points_76)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_85()
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
      double a_0;
      double a_1;
      int a_2;
      double a_0_expire;
      double a_1_expire;
      int a_2_expire;
      struct timeval atime;
   } insert_78;
   IM_RELC *Points_84;
   int first_entry_83 = 1;
   int first_entry_84 = 1;
   int index_82 = 0;
   int terminating_82=0;
   struct gb_status_82 {
      struct kmean_status *kmean_0;
   };
   struct gb_status_82 *gbstatus_82 = (struct gb_status_82 *)0;
   
   int first_entry_82 = 1;
   int first_entry_80 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = Points->cursor(Points, &Points_84, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         double a_0;
         double a_1;
         int a_2;
         double a_0_expire;
         double a_1_expire;
         int a_2_expire;
         struct timeval atime;
      } Q_0011_80_79;
      next_80:
      struct {
         int a_0;
         double a_1;
         double a_2;
         int a_0_expire;
         double a_1_expire;
         double a_2_expire;
         struct timeval atime;
      } Q_0010_82_81;
      next_82:
      while (index_82>=0 && index_82 < 2) {
         switch(index_82) {
            case 0:
            {
               if (terminating_82 == 0) {
                  /* get source tuple from qun */
                  struct {
                     double X;
                     double Y;
                     int OID;
                     double X_expire;
                     double Y_expire;
                     int OID_expire;
                     struct timeval atime;
                  } Points_84_83;
                  next_84:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = Points_84->c_get(Points_84, &key, &data, (first_entry_83)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_83 = 0;
                     memcpy(&(Points_84_83.X), (char*)data.data+0, sizeof(double));
                     //printf("Retrieved Points_84_83.X = %f\n", Points_84_83.X);
                     //fflush(stdout);
                     memcpy(&(Points_84_83.Y), (char*)data.data+8, sizeof(double));
                     //printf("Retrieved Points_84_83.Y = %f\n", Points_84_83.Y);
                     //fflush(stdout);
                     memcpy(&(Points_84_83.OID), (char*)data.data+16, sizeof(int));
                     //printf("Retrieved Points_84_83.OID = %d\n", Points_84_83.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_83 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0010_82_81.a_0 = 4;
                     Q_0010_82_81.a_1 = Points_84_83.X;
                     Q_0010_82_81.a_2 = Points_84_83.Y;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_82 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_82 = (struct gb_status_82 *)0;
                     rc = hash_get(82, _rec_id, gbkey, 4, (char**)&gbstatus_82);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_82 = (struct gb_status_82*)malloc(sizeof(*gbstatus_82));
                        gbstatus_82->kmean_0 = (struct kmean_status*)malloc(sizeof(struct kmean_status));
                        gbstatus_82->kmean_0->win = 0;
                        setModelId("");
                        kmean_init(gbstatus_82->kmean_0, Q_0010_82_81.a_0, Q_0010_82_81.a_1, Q_0010_82_81.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(82, _rec_id, gbkey, 4, &gbstatus_82);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        kmean_init(gbstatus_82->kmean_0, Q_0010_82_81.a_0, Q_0010_82_81.a_1, Q_0010_82_81.a_2, _rec_id+1, 0, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_82 = 1;
                  }
               }
               if (terminating_82 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(82, _rec_id, allkey, 4, (char**)&gbstatus_82);
                  if (rc==0) {
                     setModelId("");
                     kmean_terminate(gbstatus_82->kmean_0, Q_0010_82_81.a_0, Q_0010_82_81.a_1, Q_0010_82_81.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_82->kmean_0->retc->c_get(gbstatus_82->kmean_0->retc, &key, &data, (gbstatus_82->kmean_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_82->kmean_0->retc_first_entry = 0;
                  memcpy(&(Q_0011_80_79.a_0), (char*)data.data+0, sizeof(double));
                  //printf("Retrieved Q_0011_80_79.a_0 = %f\n", Q_0011_80_79.a_0);
                  //fflush(stdout);
                  memcpy(&(Q_0011_80_79.a_1), (char*)data.data+8, sizeof(double));
                  //printf("Retrieved Q_0011_80_79.a_1 = %f\n", Q_0011_80_79.a_1);
                  //fflush(stdout);
                  memcpy(&(Q_0011_80_79.a_2), (char*)data.data+16, sizeof(int));
                  //printf("Retrieved Q_0011_80_79.a_2 = %d\n", Q_0011_80_79.a_2);
                  //fflush(stdout);
                  if ((rc = gbstatus_82->kmean_0->retc->c_del(gbstatus_82->kmean_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_82->kmean_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_82 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_82++;
         }
         if (rc == DB_NOTFOUND) {
            index_82--;
            if (terminating_82 == 1 && index_82 == 0) {
               if (gbstatus_82->kmean_0->retc && (rc = gbstatus_82->kmean_0->retc->c_close(gbstatus_82->kmean_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_82->kmean_0);
               
               if (gbstatus_82->kmean_0->ret && ((rc = gbstatus_82->kmean_0->ret->close(gbstatus_82->kmean_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_82->kmean_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_82--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_82 = 0;
         first_entry_82 = 1;
         index_82 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(82, _rec_id, allkey, 4, (char**)&gbstatus_82);
            if (rc==0) {
               free(gbstatus_82->kmean_0);
               //printf("freeing 82\n");
               free(gbstatus_82);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(82, _rec_id);
      }
      if (rc==0) {
         insert_78.a_0 = Q_0011_80_79.a_0;
         insert_78.a_1 = Q_0011_80_79.a_1;
         insert_78.a_2 = Q_0011_80_79.a_2;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10f ", insert_78.a_0);
         printf("%10f ", insert_78.a_1);
         printf("%10d ", insert_78.a_2);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (Points_84 && (rc = Points_84->c_close(Points_84)) != 0) {
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
   if ((rc = im_rel_create(&Points, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = Points->open(Points, "_adl_db_Points", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("Points") == 0) {
      inMemTables->operator[](strdup("Points")) = Points;
   }
   _adl_statement_69();
   _adl_statement_77();
   _adl_statement_85();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = Points->close(Points, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
