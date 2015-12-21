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
IM_REL *traffic;
IM_REL *EH;
IM_REL *state;
struct merge_status {
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
void merge_udf(struct merge_status *status, int size, int k)
{
   int rc;
   int _adl_sqlcode, _adl_cursqlcode;
   char _gbkeybuf[MAX_STR_LEN], *allkey, *gbkey = _gbkeybuf;
   int _rec_id=0; /* recursive id */
   DBT key, data, windata;
   char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];
   char _adl_dbname[80];
   int slide_out = 1;
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
   struct {
      int TSFirst;
      int TSLast;
      int field_2;
      int TSFirst_expire;
      int TSLast_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_0;
   IM_RELC *A_2;
   int first_entry_1 = 1;
   IM_RELC *B_2;
   int first_entry_3 = 1;
   int index_2 = 0;
   int first_entry_4 = 1;
   IM_RELC *EH_10;
   int first_entry_9 = 1;
   int first_entry_10 = 1;
   int index_8 = 0;
   int terminating_8=0;
   struct gb_status_8 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int count_0_last_out;
      bool count_0_iterate;
      bool count_0_init;
   };
   struct gb_status_8 *gbstatus_8 = (struct gb_status_8 *)0;
   
   int first_entry_8 = 1;
   int first_entry_11 = 1;
   IM_RELC *C_11;
   int first_entry_13 = 1;
   int first_entry_2 = 1;
   IM_RELC *EH_15;
   int first_entry_16 = 1;
   int first_entry_17 = 1;
   IM_RELC *B_17;
   int first_entry_19 = 1;
   int first_entry_15 = 1;
   struct {
      int field_0;
      int field_0_expire;
      struct timeval atime;
   } insert_20;
   int first_entry_21 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = EH->cursor(EH, &A_2, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &B_2, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &EH_10, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &C_11, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int TSFirst;
         int TSLast;
         int OnesCount;
         int OID;
         int TSFirst_expire;
         int TSLast_expire;
         int OnesCount_expire;
         int OID_expire;
         struct timeval atime;
      } A_2_1;
      struct {
         int TSFirst;
         int TSLast;
         int OnesCount;
         int OID;
         int TSFirst_expire;
         int TSLast_expire;
         int OnesCount_expire;
         int OID_expire;
         struct timeval atime;
      } B_2_3;
      next_2:
      while (index_2>=0 && index_2 < 2) { 
         switch(index_2) {
            case 0:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = A_2->c_get(A_2, &key, &data, (first_entry_1)? DB_FIRST:DB_NEXT);
               if (rc==0) {
                  first_entry_1 = 0;
                  memcpy(&(A_2_1.TSFirst), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved A_2_1.TSFirst = %d\n", A_2_1.TSFirst);
                  //fflush(stdout);
                  memcpy(&(A_2_1.TSLast), (char*)data.data+4, sizeof(int));
                  //printf("Retrieved A_2_1.TSLast = %d\n", A_2_1.TSLast);
                  //fflush(stdout);
                  memcpy(&(A_2_1.OnesCount), (char*)data.data+8, sizeof(int));
                  //printf("Retrieved A_2_1.OnesCount = %d\n", A_2_1.OnesCount);
                  //fflush(stdout);
                  memcpy(&(A_2_1.OID), (char*)data.data+12, sizeof(int));
                  //printf("Retrieved A_2_1.OID = %d\n", A_2_1.OID);
                  //fflush(stdout);
               } else if (rc == DB_NOTFOUND) {
                  first_entry_1 = 1;
               } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = B_2->c_get(B_2, &key, &data, (first_entry_3)? DB_FIRST:DB_NEXT);
               if (rc==0) {
                  first_entry_3 = 0;
                  memcpy(&(B_2_3.TSFirst), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved B_2_3.TSFirst = %d\n", B_2_3.TSFirst);
                  //fflush(stdout);
                  memcpy(&(B_2_3.TSLast), (char*)data.data+4, sizeof(int));
                  //printf("Retrieved B_2_3.TSLast = %d\n", B_2_3.TSLast);
                  //fflush(stdout);
                  memcpy(&(B_2_3.OnesCount), (char*)data.data+8, sizeof(int));
                  //printf("Retrieved B_2_3.OnesCount = %d\n", B_2_3.OnesCount);
                  //fflush(stdout);
                  memcpy(&(B_2_3.OID), (char*)data.data+12, sizeof(int));
                  //printf("Retrieved B_2_3.OID = %d\n", B_2_3.OID);
                  //fflush(stdout);
               } else if (rc == DB_NOTFOUND) {
                  first_entry_3 = 1;
               } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            }
            break;
         } /*switch */
         if (rc==0) {
            index_2++;
         } else if (rc==DB_NOTFOUND) {
            index_2--;
         }
      } /* while */
      if (rc!=0) {
         index_2++;    /* set index to the first subgoal */
      } else {
         index_2--;    /* set index to the last subgoal */
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_5;
         int embed_5_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0001_4_6;
         next_4:
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0000_8_7;
         next_8:
         while (index_8>=0 && index_8 < 2) {
            switch(index_8) {
               case 0:
               {
                  if (terminating_8 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int TSFirst;
                        int TSLast;
                        int OnesCount;
                        int OID;
                        int TSFirst_expire;
                        int TSLast_expire;
                        int OnesCount_expire;
                        int OID_expire;
                        struct timeval atime;
                     } EH_10_9;
                     next_10:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = EH_10->c_get(EH_10, &key, &data, (first_entry_9)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_9 = 0;
                        memcpy(&(EH_10_9.TSFirst), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved EH_10_9.TSFirst = %d\n", EH_10_9.TSFirst);
                        //fflush(stdout);
                        memcpy(&(EH_10_9.TSLast), (char*)data.data+4, sizeof(int));
                        //printf("Retrieved EH_10_9.TSLast = %d\n", EH_10_9.TSLast);
                        //fflush(stdout);
                        memcpy(&(EH_10_9.OnesCount), (char*)data.data+8, sizeof(int));
                        //printf("Retrieved EH_10_9.OnesCount = %d\n", EH_10_9.OnesCount);
                        //fflush(stdout);
                        memcpy(&(EH_10_9.OID), (char*)data.data+12, sizeof(int));
                        //printf("Retrieved EH_10_9.OID = %d\n", EH_10_9.OID);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_9 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        rc = 0;          /* subquery could've overwritten rc */
                        if (!((EH_10_9.OnesCount == size))) {
                           goto next_10;
                        }
                        Q_0000_8_7.a_0 = EH_10_9.TSFirst;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_8 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_8 = (struct gb_status_8 *)0;
                        rc = hash_get(8, _rec_id, gbkey, 4, (char**)&gbstatus_8);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_8 = (struct gb_status_8*)malloc(sizeof(*gbstatus_8));
                           gbstatus_8->_baggr_0_first_entry = 1;
                           gbstatus_8->_baggr_0_value = 1;
                           rc = hash_put(8, _rec_id, gbkey, 4, &gbstatus_8);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_8->_baggr_0_first_entry = 1;
                           gbstatus_8->_baggr_0_value += 1;
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_8 = 1;
                     }
                  }
                  if (terminating_8 == 1) {
                     if (first_entry_8 == 1) {
                        rc = 0; /* fail on first entry, aggregate on empty set */
                     } else {
                        allkey = (char*)0;
                        rc = hash_get(8, _rec_id, allkey, 4, (char**)&gbstatus_8);
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
                  if (terminating_8 == 1) {
                     if (gbstatus_8 == (struct gb_status_8 *)0) {
                        if (first_entry_8) {
                           rc = 0;
                           Q_0001_4_6.a_0 = 0;
                        }
                     } else 
                     if (gbstatus_8->_baggr_0_first_entry == 1) {
                        Q_0001_4_6.a_0 = gbstatus_8->_baggr_0_value;
                        gbstatus_8->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_8->_baggr_0_first_entry = 1;
                     }
                  }
                  first_entry_8 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_8++;
            }
            if (rc == DB_NOTFOUND) {
               index_8--;
               if (terminating_8 == 1 && index_8 == 0) {
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_8--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_8 = 0;
            first_entry_8 = 1;
            index_8 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(8, _rec_id, allkey, 4, (char**)&gbstatus_8);
               if (rc==0) {
                  //printf("freeing 8\n");
                  free(gbstatus_8);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(8, _rec_id);
         }
         first_entry_4 = (rc)? 1:0;
         if (rc==0) {
            embed_5.a_0 = Q_0001_4_6.a_0;
            if (embed_5_cnt++ ==0) goto next_4; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_5_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 14.\n");
            exit(1);
         }
         else if (embed_5_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 14.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((embed_5.a_0 > ((((k) / 2)) + 1)))) {
            goto next_2;
         }
         rc = 0;          /* subquery could've overwritten rc */
         if (!((A_2_1.TSLast == B_2_3.TSFirst))) {
            goto next_2;
         }
         rc = 0;          /* subquery could've overwritten rc */
         if (!((A_2_1.OnesCount == B_2_3.OnesCount))) {
            goto next_2;
         }
         rc = 0;          /* subquery could've overwritten rc */
         if (!((A_2_1.OnesCount == size))) {
            goto next_2;
         }
         struct {
            int exists;
            int exists_expire;
            struct timeval atime;
         } embed_12;
         int embed_12_cnt = 0;
         struct {
            int TSFirst;
            int TSLast;
            int OnesCount;
            int OID;
            int TSFirst_expire;
            int TSLast_expire;
            int OnesCount_expire;
            int OID_expire;
            struct timeval atime;
         } C_11_13;
         next_11:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = C_11->c_get(C_11, &key, &data, (first_entry_13)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_13 = 0;
            memcpy(&(C_11_13.TSFirst), (char*)data.data+0, sizeof(int));
            //printf("Retrieved C_11_13.TSFirst = %d\n", C_11_13.TSFirst);
            //fflush(stdout);
            memcpy(&(C_11_13.TSLast), (char*)data.data+4, sizeof(int));
            //printf("Retrieved C_11_13.TSLast = %d\n", C_11_13.TSLast);
            //fflush(stdout);
            memcpy(&(C_11_13.OnesCount), (char*)data.data+8, sizeof(int));
            //printf("Retrieved C_11_13.OnesCount = %d\n", C_11_13.OnesCount);
            //fflush(stdout);
            memcpy(&(C_11_13.OID), (char*)data.data+12, sizeof(int));
            //printf("Retrieved C_11_13.OID = %d\n", C_11_13.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_13 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            rc = 0;          /* subquery could've overwritten rc */
            if (!((C_11_13.TSFirst <= B_2_3.TSLast))) {
               goto next_11;
            }
            rc = 0;          /* subquery could've overwritten rc */
            if (!((C_11_13.OnesCount == B_2_3.OnesCount))) {
               goto next_11;
            }
         } /* if (rc == 0) */
         embed_12.exists=(rc==DB_NOTFOUND);
         first_entry_13 = 1;
         rc = 0;          /* subquery could've overwritten rc */
         if (!(embed_12.exists)) {
            goto next_2;
         }
         insert_0.TSFirst = A_2_1.TSFirst;
         insert_0.TSLast = B_2_3.TSLast;
         insert_0.field_2 = ((A_2_1.OnesCount) + B_2_3.OnesCount);
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_0.TSFirst), sizeof(int));
         memcpy((char*)data.data+4, &(insert_0.TSLast), sizeof(int));
         memcpy((char*)data.data+8, &(insert_0.field_2), sizeof(int));
         data.size = 12;
         key.size = 0;
         insertTemp(14, _rec_id, &key, &data);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   mvTemp(14, _rec_id, EH);
   if (A_2 && (rc = A_2->c_close(A_2)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (B_2 && (rc = B_2->c_close(B_2)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (EH_10 && (rc = EH_10->c_close(EH_10)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (C_11 && (rc = C_11->c_close(C_11)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = EH->cursor(EH, &EH_15, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &B_17, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int TSFirst;
         int TSLast;
         int OnesCount;
         int OID;
         int TSFirst_expire;
         int TSLast_expire;
         int OnesCount_expire;
         int OID_expire;
         struct timeval atime;
      } EH_15_16;
      next_15:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = EH_15->c_get(EH_15, &key, &data, (first_entry_16)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_16 = 0;
         memcpy(&(EH_15_16.TSFirst), (char*)data.data+0, sizeof(int));
         //printf("Retrieved EH_15_16.TSFirst = %d\n", EH_15_16.TSFirst);
         //fflush(stdout);
         memcpy(&(EH_15_16.TSLast), (char*)data.data+4, sizeof(int));
         //printf("Retrieved EH_15_16.TSLast = %d\n", EH_15_16.TSLast);
         //fflush(stdout);
         memcpy(&(EH_15_16.OnesCount), (char*)data.data+8, sizeof(int));
         //printf("Retrieved EH_15_16.OnesCount = %d\n", EH_15_16.OnesCount);
         //fflush(stdout);
         memcpy(&(EH_15_16.OID), (char*)data.data+12, sizeof(int));
         //printf("Retrieved EH_15_16.OID = %d\n", EH_15_16.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_16 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         struct {
            int exists;
            int exists_expire;
            struct timeval atime;
         } embed_18;
         int embed_18_cnt = 0;
         struct {
            int TSFirst;
            int TSLast;
            int OnesCount;
            int OID;
            int TSFirst_expire;
            int TSLast_expire;
            int OnesCount_expire;
            int OID_expire;
            struct timeval atime;
         } B_17_19;
         next_17:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = B_17->c_get(B_17, &key, &data, (first_entry_19)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_19 = 0;
            memcpy(&(B_17_19.TSFirst), (char*)data.data+0, sizeof(int));
            //printf("Retrieved B_17_19.TSFirst = %d\n", B_17_19.TSFirst);
            //fflush(stdout);
            memcpy(&(B_17_19.TSLast), (char*)data.data+4, sizeof(int));
            //printf("Retrieved B_17_19.TSLast = %d\n", B_17_19.TSLast);
            //fflush(stdout);
            memcpy(&(B_17_19.OnesCount), (char*)data.data+8, sizeof(int));
            //printf("Retrieved B_17_19.OnesCount = %d\n", B_17_19.OnesCount);
            //fflush(stdout);
            memcpy(&(B_17_19.OID), (char*)data.data+12, sizeof(int));
            //printf("Retrieved B_17_19.OID = %d\n", B_17_19.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_19 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            rc = 0;          /* subquery could've overwritten rc */
            if (!((EH_15_16.OnesCount < B_17_19.OnesCount))) {
               goto next_17;
            }
            rc = 0;          /* subquery could've overwritten rc */
            if (!((((EH_15_16.TSFirst == B_17_19.TSFirst)) || ((EH_15_16.TSLast == B_17_19.TSLast))))) {
               goto next_17;
            }
         } /* if (rc == 0) */
         embed_18.exists=(rc==0);
         first_entry_19 = 1;
         if (rc == 0) {
            first_entry_19 = 1;
         }
         rc = 0;          /* subquery could've overwritten rc */
         if (!(embed_18.exists)) {
            goto next_15;
         }
         /*DELETE STARTS*/
         if ((rc = EH_15->c_del(EH_15, 0)) != 0) {
            adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
         }
         /*DELETE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (EH_15 && (rc = EH_15->c_close(EH_15)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (B_17 && (rc = B_17->c_close(B_17)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_21:
      rc = (first_entry_21)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_21=1;
      else {
         first_entry_21=0;
         insert_20.field_0 = 222222;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_20.field_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_20.field_0), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
   status->retc_first_entry=1;
}
struct mergeAggr_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void mergeAggr_init(struct mergeAggr_status *status, 
	int size, int k, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void mergeAggr_init(struct mergeAggr_status *status, int size, int k,
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
      int TSFirst;
      int TSLast;
      int field_2;
      int TSFirst_expire;
      int TSLast_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_22;
   IM_RELC *A_24;
   int first_entry_23 = 1;
   IM_RELC *B_24;
   int first_entry_25 = 1;
   int index_24 = 0;
   int first_entry_26 = 1;
   IM_RELC *EH_32;
   int first_entry_31 = 1;
   int first_entry_32 = 1;
   int index_30 = 0;
   int terminating_30=0;
   struct gb_status_30 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int count_0_last_out;
      bool count_0_iterate;
      bool count_0_init;
   };
   struct gb_status_30 *gbstatus_30 = (struct gb_status_30 *)0;
   
   int first_entry_30 = 1;
   int first_entry_33 = 1;
   IM_RELC *C_33;
   int first_entry_35 = 1;
   int first_entry_24 = 1;
   IM_RELC *EH_37;
   int first_entry_38 = 1;
   int first_entry_39 = 1;
   IM_RELC *B_39;
   int first_entry_41 = 1;
   int first_entry_37 = 1;
   struct {
      int a_0;
      int a_0_expire;
      struct timeval atime;
   } insert_42;
   IM_RELC *A_48;
   int first_entry_47 = 1;
   IM_RELC *B_48;
   int first_entry_49 = 1;
   int index_48 = 0;
   int first_entry_50 = 1;
   IM_RELC *EH_56;
   int first_entry_55 = 1;
   int first_entry_56 = 1;
   int index_54 = 0;
   int terminating_54=0;
   struct gb_status_54 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int count_0_last_out;
      bool count_0_iterate;
      bool count_0_init;
   };
   struct gb_status_54 *gbstatus_54 = (struct gb_status_54 *)0;
   
   int first_entry_54 = 1;
   int first_entry_57 = 1;
   IM_RELC *C_57;
   int first_entry_59 = 1;
   int first_entry_48 = 1;
   int index_46 = 0;
   int terminating_46=0;
   struct gb_status_46 {
      struct mergeAggr_status *mergeAggr_0;
   };
   struct gb_status_46 *gbstatus_46 = (struct gb_status_46 *)0;
   
   int first_entry_46 = 1;
   int first_entry_44 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = EH->cursor(EH, &A_24, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &B_24, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &EH_32, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &C_33, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int TSFirst;
         int TSLast;
         int OnesCount;
         int OID;
         int TSFirst_expire;
         int TSLast_expire;
         int OnesCount_expire;
         int OID_expire;
         struct timeval atime;
      } A_24_23;
      struct {
         int TSFirst;
         int TSLast;
         int OnesCount;
         int OID;
         int TSFirst_expire;
         int TSLast_expire;
         int OnesCount_expire;
         int OID_expire;
         struct timeval atime;
      } B_24_25;
      next_24:
      while (index_24>=0 && index_24 < 2) { 
         switch(index_24) {
            case 0:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = A_24->c_get(A_24, &key, &data, (first_entry_23)? DB_FIRST:DB_NEXT);
               if (rc==0) {
                  first_entry_23 = 0;
                  memcpy(&(A_24_23.TSFirst), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved A_24_23.TSFirst = %d\n", A_24_23.TSFirst);
                  //fflush(stdout);
                  memcpy(&(A_24_23.TSLast), (char*)data.data+4, sizeof(int));
                  //printf("Retrieved A_24_23.TSLast = %d\n", A_24_23.TSLast);
                  //fflush(stdout);
                  memcpy(&(A_24_23.OnesCount), (char*)data.data+8, sizeof(int));
                  //printf("Retrieved A_24_23.OnesCount = %d\n", A_24_23.OnesCount);
                  //fflush(stdout);
                  memcpy(&(A_24_23.OID), (char*)data.data+12, sizeof(int));
                  //printf("Retrieved A_24_23.OID = %d\n", A_24_23.OID);
                  //fflush(stdout);
               } else if (rc == DB_NOTFOUND) {
                  first_entry_23 = 1;
               } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = B_24->c_get(B_24, &key, &data, (first_entry_25)? DB_FIRST:DB_NEXT);
               if (rc==0) {
                  first_entry_25 = 0;
                  memcpy(&(B_24_25.TSFirst), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved B_24_25.TSFirst = %d\n", B_24_25.TSFirst);
                  //fflush(stdout);
                  memcpy(&(B_24_25.TSLast), (char*)data.data+4, sizeof(int));
                  //printf("Retrieved B_24_25.TSLast = %d\n", B_24_25.TSLast);
                  //fflush(stdout);
                  memcpy(&(B_24_25.OnesCount), (char*)data.data+8, sizeof(int));
                  //printf("Retrieved B_24_25.OnesCount = %d\n", B_24_25.OnesCount);
                  //fflush(stdout);
                  memcpy(&(B_24_25.OID), (char*)data.data+12, sizeof(int));
                  //printf("Retrieved B_24_25.OID = %d\n", B_24_25.OID);
                  //fflush(stdout);
               } else if (rc == DB_NOTFOUND) {
                  first_entry_25 = 1;
               } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            }
            break;
         } /*switch */
         if (rc==0) {
            index_24++;
         } else if (rc==DB_NOTFOUND) {
            index_24--;
         }
      } /* while */
      if (rc!=0) {
         index_24++;    /* set index to the first subgoal */
      } else {
         index_24--;    /* set index to the last subgoal */
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_27;
         int embed_27_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0003_26_28;
         next_26:
         struct {
            int a_0;
            int a_0_expire;
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
                        int TSFirst;
                        int TSLast;
                        int OnesCount;
                        int OID;
                        int TSFirst_expire;
                        int TSLast_expire;
                        int OnesCount_expire;
                        int OID_expire;
                        struct timeval atime;
                     } EH_32_31;
                     next_32:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = EH_32->c_get(EH_32, &key, &data, (first_entry_31)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_31 = 0;
                        memcpy(&(EH_32_31.TSFirst), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved EH_32_31.TSFirst = %d\n", EH_32_31.TSFirst);
                        //fflush(stdout);
                        memcpy(&(EH_32_31.TSLast), (char*)data.data+4, sizeof(int));
                        //printf("Retrieved EH_32_31.TSLast = %d\n", EH_32_31.TSLast);
                        //fflush(stdout);
                        memcpy(&(EH_32_31.OnesCount), (char*)data.data+8, sizeof(int));
                        //printf("Retrieved EH_32_31.OnesCount = %d\n", EH_32_31.OnesCount);
                        //fflush(stdout);
                        memcpy(&(EH_32_31.OID), (char*)data.data+12, sizeof(int));
                        //printf("Retrieved EH_32_31.OID = %d\n", EH_32_31.OID);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_31 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        rc = 0;          /* subquery could've overwritten rc */
                        if (!((EH_32_31.OnesCount == size))) {
                           goto next_32;
                        }
                        Q_0002_30_29.a_0 = EH_32_31.TSFirst;
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
                           gbstatus_30->_baggr_0_first_entry = 1;
                           gbstatus_30->_baggr_0_value = 1;
                           rc = hash_put(30, _rec_id, gbkey, 4, &gbstatus_30);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_30->_baggr_0_first_entry = 1;
                           gbstatus_30->_baggr_0_value += 1;
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_30 = 1;
                     }
                  }
                  if (terminating_30 == 1) {
                     if (first_entry_30 == 1) {
                        rc = 0; /* fail on first entry, aggregate on empty set */
                     } else {
                        allkey = (char*)0;
                        rc = hash_get(30, _rec_id, allkey, 4, (char**)&gbstatus_30);
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
                  if (terminating_30 == 1) {
                     if (gbstatus_30 == (struct gb_status_30 *)0) {
                        if (first_entry_30) {
                           rc = 0;
                           Q_0003_26_28.a_0 = 0;
                        }
                     } else 
                     if (gbstatus_30->_baggr_0_first_entry == 1) {
                        Q_0003_26_28.a_0 = gbstatus_30->_baggr_0_value;
                        gbstatus_30->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_30->_baggr_0_first_entry = 1;
                     }
                  }
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
         first_entry_26 = (rc)? 1:0;
         if (rc==0) {
            embed_27.a_0 = Q_0003_26_28.a_0;
            if (embed_27_cnt++ ==0) goto next_26; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_27_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 40.\n");
            exit(1);
         }
         else if (embed_27_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 40.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((embed_27.a_0 > ((((k) / 2)) + 1)))) {
            goto next_24;
         }
         rc = 0;          /* subquery could've overwritten rc */
         if (!((A_24_23.TSLast == B_24_25.TSFirst))) {
            goto next_24;
         }
         rc = 0;          /* subquery could've overwritten rc */
         if (!((A_24_23.OnesCount == B_24_25.OnesCount))) {
            goto next_24;
         }
         rc = 0;          /* subquery could've overwritten rc */
         if (!((A_24_23.OnesCount == size))) {
            goto next_24;
         }
         struct {
            int exists;
            int exists_expire;
            struct timeval atime;
         } embed_34;
         int embed_34_cnt = 0;
         struct {
            int TSFirst;
            int TSLast;
            int OnesCount;
            int OID;
            int TSFirst_expire;
            int TSLast_expire;
            int OnesCount_expire;
            int OID_expire;
            struct timeval atime;
         } C_33_35;
         next_33:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = C_33->c_get(C_33, &key, &data, (first_entry_35)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_35 = 0;
            memcpy(&(C_33_35.TSFirst), (char*)data.data+0, sizeof(int));
            //printf("Retrieved C_33_35.TSFirst = %d\n", C_33_35.TSFirst);
            //fflush(stdout);
            memcpy(&(C_33_35.TSLast), (char*)data.data+4, sizeof(int));
            //printf("Retrieved C_33_35.TSLast = %d\n", C_33_35.TSLast);
            //fflush(stdout);
            memcpy(&(C_33_35.OnesCount), (char*)data.data+8, sizeof(int));
            //printf("Retrieved C_33_35.OnesCount = %d\n", C_33_35.OnesCount);
            //fflush(stdout);
            memcpy(&(C_33_35.OID), (char*)data.data+12, sizeof(int));
            //printf("Retrieved C_33_35.OID = %d\n", C_33_35.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_35 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            rc = 0;          /* subquery could've overwritten rc */
            if (!((C_33_35.TSFirst <= B_24_25.TSLast))) {
               goto next_33;
            }
            rc = 0;          /* subquery could've overwritten rc */
            if (!((C_33_35.OnesCount == B_24_25.OnesCount))) {
               goto next_33;
            }
         } /* if (rc == 0) */
         embed_34.exists=(rc==DB_NOTFOUND);
         first_entry_35 = 1;
         rc = 0;          /* subquery could've overwritten rc */
         if (!(embed_34.exists)) {
            goto next_24;
         }
         insert_22.TSFirst = A_24_23.TSFirst;
         insert_22.TSLast = B_24_25.TSLast;
         insert_22.field_2 = ((A_24_23.OnesCount) + B_24_25.OnesCount);
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_22.TSFirst), sizeof(int));
         memcpy((char*)data.data+4, &(insert_22.TSLast), sizeof(int));
         memcpy((char*)data.data+8, &(insert_22.field_2), sizeof(int));
         data.size = 12;
         key.size = 0;
         insertTemp(36, _rec_id, &key, &data);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   mvTemp(36, _rec_id, EH);
   if (A_24 && (rc = A_24->c_close(A_24)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (B_24 && (rc = B_24->c_close(B_24)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (EH_32 && (rc = EH_32->c_close(EH_32)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (C_33 && (rc = C_33->c_close(C_33)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = EH->cursor(EH, &EH_37, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &B_39, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int TSFirst;
         int TSLast;
         int OnesCount;
         int OID;
         int TSFirst_expire;
         int TSLast_expire;
         int OnesCount_expire;
         int OID_expire;
         struct timeval atime;
      } EH_37_38;
      next_37:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = EH_37->c_get(EH_37, &key, &data, (first_entry_38)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_38 = 0;
         memcpy(&(EH_37_38.TSFirst), (char*)data.data+0, sizeof(int));
         //printf("Retrieved EH_37_38.TSFirst = %d\n", EH_37_38.TSFirst);
         //fflush(stdout);
         memcpy(&(EH_37_38.TSLast), (char*)data.data+4, sizeof(int));
         //printf("Retrieved EH_37_38.TSLast = %d\n", EH_37_38.TSLast);
         //fflush(stdout);
         memcpy(&(EH_37_38.OnesCount), (char*)data.data+8, sizeof(int));
         //printf("Retrieved EH_37_38.OnesCount = %d\n", EH_37_38.OnesCount);
         //fflush(stdout);
         memcpy(&(EH_37_38.OID), (char*)data.data+12, sizeof(int));
         //printf("Retrieved EH_37_38.OID = %d\n", EH_37_38.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_38 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         struct {
            int exists;
            int exists_expire;
            struct timeval atime;
         } embed_40;
         int embed_40_cnt = 0;
         struct {
            int TSFirst;
            int TSLast;
            int OnesCount;
            int OID;
            int TSFirst_expire;
            int TSLast_expire;
            int OnesCount_expire;
            int OID_expire;
            struct timeval atime;
         } B_39_41;
         next_39:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = B_39->c_get(B_39, &key, &data, (first_entry_41)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_41 = 0;
            memcpy(&(B_39_41.TSFirst), (char*)data.data+0, sizeof(int));
            //printf("Retrieved B_39_41.TSFirst = %d\n", B_39_41.TSFirst);
            //fflush(stdout);
            memcpy(&(B_39_41.TSLast), (char*)data.data+4, sizeof(int));
            //printf("Retrieved B_39_41.TSLast = %d\n", B_39_41.TSLast);
            //fflush(stdout);
            memcpy(&(B_39_41.OnesCount), (char*)data.data+8, sizeof(int));
            //printf("Retrieved B_39_41.OnesCount = %d\n", B_39_41.OnesCount);
            //fflush(stdout);
            memcpy(&(B_39_41.OID), (char*)data.data+12, sizeof(int));
            //printf("Retrieved B_39_41.OID = %d\n", B_39_41.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_41 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            rc = 0;          /* subquery could've overwritten rc */
            if (!((EH_37_38.OnesCount < B_39_41.OnesCount))) {
               goto next_39;
            }
            rc = 0;          /* subquery could've overwritten rc */
            if (!((((EH_37_38.TSFirst == B_39_41.TSFirst)) || ((EH_37_38.TSLast == B_39_41.TSLast))))) {
               goto next_39;
            }
         } /* if (rc == 0) */
         embed_40.exists=(rc==0);
         first_entry_41 = 1;
         if (rc == 0) {
            first_entry_41 = 1;
         }
         rc = 0;          /* subquery could've overwritten rc */
         if (!(embed_40.exists)) {
            goto next_37;
         }
         /*DELETE STARTS*/
         if ((rc = EH_37->c_del(EH_37, 0)) != 0) {
            adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
         }
         /*DELETE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (EH_37 && (rc = EH_37->c_close(EH_37)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (B_39 && (rc = B_39->c_close(B_39)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = EH->cursor(EH, &A_48, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &B_48, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &EH_56, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &C_57, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0005_44_43;
      next_44:
      struct {
         int a_0;
         int a_1;
         int a_0_expire;
         int a_1_expire;
         struct timeval atime;
      } Q_0004_46_45;
      next_46:
      while (index_46>=0 && index_46 < 2) {
         switch(index_46) {
            case 0:
            {
               if (terminating_46 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int TSFirst;
                     int TSLast;
                     int OnesCount;
                     int OID;
                     int TSFirst_expire;
                     int TSLast_expire;
                     int OnesCount_expire;
                     int OID_expire;
                     struct timeval atime;
                  } A_48_47;
                  struct {
                     int TSFirst;
                     int TSLast;
                     int OnesCount;
                     int OID;
                     int TSFirst_expire;
                     int TSLast_expire;
                     int OnesCount_expire;
                     int OID_expire;
                     struct timeval atime;
                  } B_48_49;
                  next_48:
                  while (index_48>=0 && index_48 < 2) { 
                     switch(index_48) {
                        case 0:
                        {
                           memset(&key, 0, sizeof(key));
                           memset(&data, 0, sizeof(data));
                           rc = A_48->c_get(A_48, &key, &data, (first_entry_47)? DB_FIRST:DB_NEXT);
                           if (rc==0) {
                              first_entry_47 = 0;
                              memcpy(&(A_48_47.TSFirst), (char*)data.data+0, sizeof(int));
                              //printf("Retrieved A_48_47.TSFirst = %d\n", A_48_47.TSFirst);
                              //fflush(stdout);
                              memcpy(&(A_48_47.TSLast), (char*)data.data+4, sizeof(int));
                              //printf("Retrieved A_48_47.TSLast = %d\n", A_48_47.TSLast);
                              //fflush(stdout);
                              memcpy(&(A_48_47.OnesCount), (char*)data.data+8, sizeof(int));
                              //printf("Retrieved A_48_47.OnesCount = %d\n", A_48_47.OnesCount);
                              //fflush(stdout);
                              memcpy(&(A_48_47.OID), (char*)data.data+12, sizeof(int));
                              //printf("Retrieved A_48_47.OID = %d\n", A_48_47.OID);
                              //fflush(stdout);
                           } else if (rc == DB_NOTFOUND) {
                              first_entry_47 = 1;
                           } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                        }
                        break;
                        case 1:
                        {
                           memset(&key, 0, sizeof(key));
                           memset(&data, 0, sizeof(data));
                           rc = B_48->c_get(B_48, &key, &data, (first_entry_49)? DB_FIRST:DB_NEXT);
                           if (rc==0) {
                              first_entry_49 = 0;
                              memcpy(&(B_48_49.TSFirst), (char*)data.data+0, sizeof(int));
                              //printf("Retrieved B_48_49.TSFirst = %d\n", B_48_49.TSFirst);
                              //fflush(stdout);
                              memcpy(&(B_48_49.TSLast), (char*)data.data+4, sizeof(int));
                              //printf("Retrieved B_48_49.TSLast = %d\n", B_48_49.TSLast);
                              //fflush(stdout);
                              memcpy(&(B_48_49.OnesCount), (char*)data.data+8, sizeof(int));
                              //printf("Retrieved B_48_49.OnesCount = %d\n", B_48_49.OnesCount);
                              //fflush(stdout);
                              memcpy(&(B_48_49.OID), (char*)data.data+12, sizeof(int));
                              //printf("Retrieved B_48_49.OID = %d\n", B_48_49.OID);
                              //fflush(stdout);
                           } else if (rc == DB_NOTFOUND) {
                              first_entry_49 = 1;
                           } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                        }
                        break;
                     } /*switch */
                     if (rc==0) {
                        index_48++;
                     } else if (rc==DB_NOTFOUND) {
                        index_48--;
                     }
                  } /* while */
                  if (rc!=0) {
                     index_48++;    /* set index to the first subgoal */
                  } else {
                     index_48--;    /* set index to the last subgoal */
                     struct {
                        int a_0;
                        int a_0_expire;
                        struct timeval atime;
                     } embed_51;
                     int embed_51_cnt = 0;
                     struct {
                        int a_0;
                        int a_0_expire;
                        struct timeval atime;
                     } Q_0007_50_52;
                     next_50:
                     struct {
                        int a_0;
                        int a_0_expire;
                        struct timeval atime;
                     } Q_0006_54_53;
                     next_54:
                     while (index_54>=0 && index_54 < 2) {
                        switch(index_54) {
                           case 0:
                           {
                              if (terminating_54 == 0) {
                                 /* get source tuple from qun */
                                 struct {
                                    int TSFirst;
                                    int TSLast;
                                    int OnesCount;
                                    int OID;
                                    int TSFirst_expire;
                                    int TSLast_expire;
                                    int OnesCount_expire;
                                    int OID_expire;
                                    struct timeval atime;
                                 } EH_56_55;
                                 next_56:
                                 memset(&key, 0, sizeof(key));
                                 memset(&data, 0, sizeof(data));
                                 rc = EH_56->c_get(EH_56, &key, &data, (first_entry_55)? DB_FIRST:DB_NEXT);
                                 if (rc==0) {
                                    first_entry_55 = 0;
                                    memcpy(&(EH_56_55.TSFirst), (char*)data.data+0, sizeof(int));
                                    //printf("Retrieved EH_56_55.TSFirst = %d\n", EH_56_55.TSFirst);
                                    //fflush(stdout);
                                    memcpy(&(EH_56_55.TSLast), (char*)data.data+4, sizeof(int));
                                    //printf("Retrieved EH_56_55.TSLast = %d\n", EH_56_55.TSLast);
                                    //fflush(stdout);
                                    memcpy(&(EH_56_55.OnesCount), (char*)data.data+8, sizeof(int));
                                    //printf("Retrieved EH_56_55.OnesCount = %d\n", EH_56_55.OnesCount);
                                    //fflush(stdout);
                                    memcpy(&(EH_56_55.OID), (char*)data.data+12, sizeof(int));
                                    //printf("Retrieved EH_56_55.OID = %d\n", EH_56_55.OID);
                                    //fflush(stdout);
                                 } else if (rc == DB_NOTFOUND) {
                                    first_entry_55 = 1;
                                 } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                                 if (rc==0) {
                                    rc = 0;          /* subquery could've overwritten rc */
                                    if (!((EH_56_55.OnesCount == ((size) * 2)))) {
                                       goto next_56;
                                    }
                                    Q_0006_54_53.a_0 = EH_56_55.TSFirst;
                                 } /* if (rc == 0) */
                                 if (rc==0) {
                                    first_entry_54 = 0;
                                    /* make assignments of non-aggr head expr */
                                    /* merge group-by columns into a key */
                                    strcpy(gbkey, "____");
                                    gbstatus_54 = (struct gb_status_54 *)0;
                                    rc = hash_get(54, _rec_id, gbkey, 4, (char**)&gbstatus_54);
                                    if (rc == DB_NOTFOUND) {//blah
                                       gbstatus_54 = (struct gb_status_54*)malloc(sizeof(*gbstatus_54));
                                       gbstatus_54->_baggr_0_first_entry = 1;
                                       gbstatus_54->_baggr_0_value = 1;
                                       rc = hash_put(54, _rec_id, gbkey, 4, &gbstatus_54);
                                    } else if (rc == 0) {
                                       /* PHASE iterate */
                                       gbstatus_54->_baggr_0_first_entry = 1;
                                       gbstatus_54->_baggr_0_value += 1;
                                    } else adlabort(rc, "hash->get()");
                                 } else if (rc == DB_NOTFOUND) {
                                    terminating_54 = 1;
                                 }
                              }
                              if (terminating_54 == 1) {
                                 if (first_entry_54 == 1) {
                                    rc = 0; /* fail on first entry, aggregate on empty set */
                                 } else {
                                    allkey = (char*)0;
                                    rc = hash_get(54, _rec_id, allkey, 4, (char**)&gbstatus_54);
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
                              if (terminating_54 == 1) {
                                 if (gbstatus_54 == (struct gb_status_54 *)0) {
                                    if (first_entry_54) {
                                       rc = 0;
                                       Q_0007_50_52.a_0 = 0;
                                    }
                                 } else 
                                 if (gbstatus_54->_baggr_0_first_entry == 1) {
                                    Q_0007_50_52.a_0 = gbstatus_54->_baggr_0_value;
                                    gbstatus_54->_baggr_0_first_entry = 0;
                                    rc = 0;
                                 } else {
                                    gbstatus_54->_baggr_0_first_entry = 1;
                                 }
                              }
                              first_entry_54 = 0;
                           }
                           break;
                        } /*end of switch*/
                        if (rc == 0) {
                           index_54++;
                        }
                        if (rc == DB_NOTFOUND) {
                           index_54--;
                           if (terminating_54 == 1 && index_54 == 0) {
                              rc = DB_NOTFOUND;
                           }
                        }
                     }/*end of while */
                     if (rc == 0) index_54--;
                     else 
                     {
                        int rc;		/* local rc */ 
                        terminating_54 = 0;
                        first_entry_54 = 1;
                        index_54 = 0;
                        /* free gbstatus */
                        do {
                           allkey = (char*)0;
                           rc = hash_get(54, _rec_id, allkey, 4, (char**)&gbstatus_54);
                           if (rc==0) {
                              //printf("freeing 54\n");
                              free(gbstatus_54);
                           }
                        } while (rc==0);
                        if (rc != DB_NOTFOUND) {
                           adlabort(rc, "hash->get()");
                        }
                        /* release hash entry */
                        hashgb_delete(54, _rec_id);
                     }
                     first_entry_50 = (rc)? 1:0;
                     if (rc==0) {
                        embed_51.a_0 = Q_0007_50_52.a_0;
                        if (embed_51_cnt++ ==0) goto next_50; /* scalar opr */
                     } /* if (rc == 0) */
                     if (embed_51_cnt == 0) {
                        fprintf(stderr, "ERR: scalar subquery returns no tuple at line 60.\n");
                        exit(1);
                     }
                     else if (embed_51_cnt >  1) {
                        fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 60.\n");
                        exit(1);
                     }
                     rc = 0;
                     rc = 0;          /* subquery could've overwritten rc */
                     if (!((embed_51.a_0 > ((((k) / 2)) + 1)))) {
                        goto next_48;
                     }
                     rc = 0;          /* subquery could've overwritten rc */
                     if (!((A_48_47.TSLast == B_48_49.TSFirst))) {
                        goto next_48;
                     }
                     rc = 0;          /* subquery could've overwritten rc */
                     if (!((A_48_47.OnesCount == B_48_49.OnesCount))) {
                        goto next_48;
                     }
                     rc = 0;          /* subquery could've overwritten rc */
                     if (!((A_48_47.OnesCount == ((size) * 2)))) {
                        goto next_48;
                     }
                     struct {
                        int exists;
                        int exists_expire;
                        struct timeval atime;
                     } embed_58;
                     int embed_58_cnt = 0;
                     struct {
                        int TSFirst;
                        int TSLast;
                        int OnesCount;
                        int OID;
                        int TSFirst_expire;
                        int TSLast_expire;
                        int OnesCount_expire;
                        int OID_expire;
                        struct timeval atime;
                     } C_57_59;
                     next_57:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = C_57->c_get(C_57, &key, &data, (first_entry_59)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_59 = 0;
                        memcpy(&(C_57_59.TSFirst), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved C_57_59.TSFirst = %d\n", C_57_59.TSFirst);
                        //fflush(stdout);
                        memcpy(&(C_57_59.TSLast), (char*)data.data+4, sizeof(int));
                        //printf("Retrieved C_57_59.TSLast = %d\n", C_57_59.TSLast);
                        //fflush(stdout);
                        memcpy(&(C_57_59.OnesCount), (char*)data.data+8, sizeof(int));
                        //printf("Retrieved C_57_59.OnesCount = %d\n", C_57_59.OnesCount);
                        //fflush(stdout);
                        memcpy(&(C_57_59.OID), (char*)data.data+12, sizeof(int));
                        //printf("Retrieved C_57_59.OID = %d\n", C_57_59.OID);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_59 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        rc = 0;          /* subquery could've overwritten rc */
                        if (!((C_57_59.TSFirst <= B_48_49.TSLast))) {
                           goto next_57;
                        }
                        rc = 0;          /* subquery could've overwritten rc */
                        if (!((C_57_59.OnesCount == B_48_49.OnesCount))) {
                           goto next_57;
                        }
                     } /* if (rc == 0) */
                     embed_58.exists=(rc==DB_NOTFOUND);
                     first_entry_59 = 1;
                     rc = 0;          /* subquery could've overwritten rc */
                     if (!(embed_58.exists)) {
                        goto next_48;
                     }
                     Q_0004_46_45.a_0 = ((size) * 2);
                     Q_0004_46_45.a_1 = k;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_46 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_46 = (struct gb_status_46 *)0;
                     rc = hash_get(46, _rec_id, gbkey, 4, (char**)&gbstatus_46);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_46 = (struct gb_status_46*)malloc(sizeof(*gbstatus_46));
                        gbstatus_46->mergeAggr_0 = (struct mergeAggr_status*)malloc(sizeof(struct mergeAggr_status));
                        gbstatus_46->mergeAggr_0->win = 0;
                        setModelId("");
                        mergeAggr_init(gbstatus_46->mergeAggr_0, Q_0004_46_45.a_0, Q_0004_46_45.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(46, _rec_id, gbkey, 4, &gbstatus_46);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_46 = 1;
                  }
               }
               if (terminating_46 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(46, _rec_id, allkey, 4, (char**)&gbstatus_46);
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
               rc = gbstatus_46->mergeAggr_0->retc->c_get(gbstatus_46->mergeAggr_0->retc, &key, &data, (gbstatus_46->mergeAggr_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_46->mergeAggr_0->retc_first_entry = 0;
                  memcpy(&(Q_0005_44_43.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0005_44_43.a_0 = %d\n", Q_0005_44_43.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_46->mergeAggr_0->retc->c_del(gbstatus_46->mergeAggr_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_46->mergeAggr_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_46 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_46++;
         }
         if (rc == DB_NOTFOUND) {
            index_46--;
            if (terminating_46 == 1 && index_46 == 0) {
               if (gbstatus_46->mergeAggr_0->retc && (rc = gbstatus_46->mergeAggr_0->retc->c_close(gbstatus_46->mergeAggr_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_46->mergeAggr_0);
               
               if (gbstatus_46->mergeAggr_0->ret && ((rc = gbstatus_46->mergeAggr_0->ret->close(gbstatus_46->mergeAggr_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_46->mergeAggr_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_46--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_46 = 0;
         first_entry_46 = 1;
         index_46 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(46, _rec_id, allkey, 4, (char**)&gbstatus_46);
            if (rc==0) {
               free(gbstatus_46->mergeAggr_0);
               //printf("freeing 46\n");
               free(gbstatus_46);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(46, _rec_id);
      }
      if (rc==0) {
         insert_42.a_0 = Q_0005_44_43.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_42.a_0);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (A_48 && (rc = A_48->c_close(A_48)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (B_48 && (rc = B_48->c_close(B_48)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (EH_56 && (rc = EH_56->c_close(EH_56)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (C_57 && (rc = C_57->c_close(C_57)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
struct inWinType_mergeAggr_window {
   int size;
   int k;
};
struct mergeAggr_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_mergeAggr_window* getLastTuple_mergeAggr_window(IM_REL* inwindow, inWinType_mergeAggr_window* tuple, bufferMngr* bm) {
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
   memcpy(&((*tuple).size), (char*)data.data+0, sizeof(int));
   //printf("Retrieved (*tuple).size = %d\n", (*tuple).size);
   //fflush(stdout);
   memcpy(&((*tuple).k), (char*)data.data+4, sizeof(int));
   //printf("Retrieved (*tuple).k = %d\n", (*tuple).k);
   //fflush(stdout);
   return tuple;
}
extern "C" void mergeAggr_window_init(struct mergeAggr_window_status *status, 
	int size, int k, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void mergeAggr_window_init(struct mergeAggr_window_status *status, int size, int k, 
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
   struct inWinType_mergeAggr_window tuple;
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
   } insert_60;
   IM_RELC *w_66;
   int first_entry_65 = 1;
   int first_entry_66 = 1;
   int index_64 = 0;
   int terminating_64=0;
   struct gb_status_64 {
      struct mergeAggr_status *mergeAggr_0;
   };
   struct gb_status_64 *gbstatus_64 = (struct gb_status_64 *)0;
   
   int first_entry_64 = 1;
   int first_entry_62 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_66, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_0_expire;
         struct timeval atime;
      } Q_0009_62_61;
      next_62:
      struct {
         int a_0;
         int a_1;
         int a_0_expire;
         int a_1_expire;
         struct timeval atime;
      } Q_0008_64_63;
      next_64:
      while (index_64>=0 && index_64 < 2) {
         switch(index_64) {
            case 0:
            {
               if (terminating_64 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int size;
                     int k;
                     int size_expire;
                     int k_expire;
                     struct timeval atime;
                  } w_66_65;
                  next_66:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_66->c_get(w_66, &key, &data, (first_entry_65)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_65 = 0;
                     memcpy(&(w_66_65.size), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_66_65.size = %d\n", w_66_65.size);
                     //fflush(stdout);
                     memcpy(&(w_66_65.k), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved w_66_65.k = %d\n", w_66_65.k);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_65 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0008_64_63.a_0 = w_66_65.size;
                     Q_0008_64_63.a_1 = w_66_65.k;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_64 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_64 = (struct gb_status_64 *)0;
                     rc = hash_get(64, _rec_id, gbkey, 4, (char**)&gbstatus_64);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_64 = (struct gb_status_64*)malloc(sizeof(*gbstatus_64));
                        gbstatus_64->mergeAggr_0 = (struct mergeAggr_status*)malloc(sizeof(struct mergeAggr_status));
                        gbstatus_64->mergeAggr_0->win = 0;
                        setModelId("");
                        mergeAggr_init(gbstatus_64->mergeAggr_0, Q_0008_64_63.a_0, Q_0008_64_63.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(64, _rec_id, gbkey, 4, &gbstatus_64);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_64 = 1;
                  }
               }
               if (terminating_64 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(64, _rec_id, allkey, 4, (char**)&gbstatus_64);
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
               rc = gbstatus_64->mergeAggr_0->retc->c_get(gbstatus_64->mergeAggr_0->retc, &key, &data, (gbstatus_64->mergeAggr_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_64->mergeAggr_0->retc_first_entry = 0;
                  memcpy(&(Q_0009_62_61.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0009_62_61.a_0 = %d\n", Q_0009_62_61.a_0);
                  //fflush(stdout);
                  if ((rc = gbstatus_64->mergeAggr_0->retc->c_del(gbstatus_64->mergeAggr_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_64->mergeAggr_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_64 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_64++;
         }
         if (rc == DB_NOTFOUND) {
            index_64--;
            if (terminating_64 == 1 && index_64 == 0) {
               if (gbstatus_64->mergeAggr_0->retc && (rc = gbstatus_64->mergeAggr_0->retc->c_close(gbstatus_64->mergeAggr_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_64->mergeAggr_0);
               
               if (gbstatus_64->mergeAggr_0->ret && ((rc = gbstatus_64->mergeAggr_0->ret->close(gbstatus_64->mergeAggr_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_64->mergeAggr_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_64--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_64 = 0;
         first_entry_64 = 1;
         index_64 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(64, _rec_id, allkey, 4, (char**)&gbstatus_64);
            if (rc==0) {
               free(gbstatus_64->mergeAggr_0);
               //printf("freeing 64\n");
               free(gbstatus_64);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(64, _rec_id);
      }
      if (rc==0) {
         insert_60.a_0 = Q_0009_62_61.a_0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_60.a_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_60.a_0), sizeof(int));
         data.size = 4;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_66 && (rc = w_66->c_close(w_66)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
struct EHSimpleCount_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
extern "C" void EHSimpleCount_init(struct EHSimpleCount_status *status, 
	int k, int W, int Next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void EHSimpleCount_iterate(struct EHSimpleCount_status *status, 
	int k, int W, int Next, int _rec_id, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void EHSimpleCount_terminate(struct EHSimpleCount_status *status, 
	int k, int W, int Next, int _rec_id, int not_delete = 0, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0,
	char* _modelId=NULL);
extern "C" void EHSimpleCount_init(struct EHSimpleCount_status *status, int k, int W, int Next,
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
      int field_0;
      int field_0_expire;
      struct timeval atime;
   } insert_67;
   int first_entry_68 = 1;
   struct {
      int field_0;
      int field_1;
      int field_2;
      int field_0_expire;
      int field_1_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_69;
   int first_entry_71 = 1;
   IM_RELC *state_71;
   int first_entry_73 = 1;
   int first_entry_74 = 1;
   IM_RELC *state_74;
   int first_entry_76 = 1;
   int first_entry_70 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   while (rc==0) {
      next_68:
      rc = (first_entry_68)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_68=1;
      else {
         first_entry_68=0;
         insert_67.field_0 = 0;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_67.field_0), sizeof(int));
         data.size = 4;
         key.size = 0;
         if ((rc = state->put(state, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = state->cursor(state, &state_71, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = state->cursor(state, &state_74, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      /* precomputed predicates */
      if (! ((Next == 1)) ) {
         rc = DB_NOTFOUND;
      } else {
         next_70:
         rc = (first_entry_70)? 0:DB_NOTFOUND;
         if (rc == DB_NOTFOUND) first_entry_70=1;
         else {
            first_entry_70=0;
            struct {
               int maxID;
               int maxID_expire;
               struct timeval atime;
            } embed_72;
            int embed_72_cnt = 0;
            struct {
               int maxID;
               int OID;
               int maxID_expire;
               int OID_expire;
               struct timeval atime;
            } state_71_73;
            next_71:
            memset(&key, 0, sizeof(key));
            memset(&data, 0, sizeof(data));
            rc = state_71->c_get(state_71, &key, &data, (first_entry_73)? DB_FIRST:DB_NEXT);
            if (rc==0) {
               first_entry_73 = 0;
               memcpy(&(state_71_73.maxID), (char*)data.data+0, sizeof(int));
               //printf("Retrieved state_71_73.maxID = %d\n", state_71_73.maxID);
               //fflush(stdout);
               memcpy(&(state_71_73.OID), (char*)data.data+4, sizeof(int));
               //printf("Retrieved state_71_73.OID = %d\n", state_71_73.OID);
               //fflush(stdout);
            } else if (rc == DB_NOTFOUND) {
               first_entry_73 = 1;
            } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            first_entry_71 = (rc)? 1:0;
            if (rc==0) {
               embed_72.maxID = state_71_73.maxID;
               if (embed_72_cnt++ ==0) goto next_71; /* scalar opr */
            } /* if (rc == 0) */
            if (embed_72_cnt == 0) {
               fprintf(stderr, "ERR: scalar subquery returns no tuple at line 84.\n");
               exit(1);
            }
            else if (embed_72_cnt >  1) {
               fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 84.\n");
               exit(1);
            }
            rc = 0;
            insert_69.field_0 = ((embed_72.maxID) + 1);
            struct {
               int maxID;
               int maxID_expire;
               struct timeval atime;
            } embed_75;
            int embed_75_cnt = 0;
            struct {
               int maxID;
               int OID;
               int maxID_expire;
               int OID_expire;
               struct timeval atime;
            } state_74_76;
            next_74:
            memset(&key, 0, sizeof(key));
            memset(&data, 0, sizeof(data));
            rc = state_74->c_get(state_74, &key, &data, (first_entry_76)? DB_FIRST:DB_NEXT);
            if (rc==0) {
               first_entry_76 = 0;
               memcpy(&(state_74_76.maxID), (char*)data.data+0, sizeof(int));
               //printf("Retrieved state_74_76.maxID = %d\n", state_74_76.maxID);
               //fflush(stdout);
               memcpy(&(state_74_76.OID), (char*)data.data+4, sizeof(int));
               //printf("Retrieved state_74_76.OID = %d\n", state_74_76.OID);
               //fflush(stdout);
            } else if (rc == DB_NOTFOUND) {
               first_entry_76 = 1;
            } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            if (rc==0) {
               embed_75.maxID = state_74_76.maxID;
            } /* if (rc == 0) */
            insert_69.field_1 = embed_75.maxID;
            insert_69.field_2 = 1;
         } /* if (rc == 0) */
      } /* end of precomputed predicates */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_69.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_69.field_1), sizeof(int));
         memcpy((char*)data.data+8, &(insert_69.field_2), sizeof(int));
         data.size = 12;
         key.size = 0;
         if ((rc = EH->put(EH, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (state_71 && (rc = state_71->c_close(state_71)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (state_74 && (rc = state_74->c_close(state_74)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void EHSimpleCount_iterate(struct EHSimpleCount_status *status, 
	int k, int W, int Next, int _rec_id, bufferMngr* bm, 
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
   IM_RELC *state_77;
   int first_entry_78 = 1;
   int first_entry_77 = 1;
   struct {
      int field_0;
      int field_1;
      int field_2;
      int field_0_expire;
      int field_1_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_79;
   int first_entry_81 = 1;
   IM_RELC *state_81;
   int first_entry_83 = 1;
   int first_entry_84 = 1;
   IM_RELC *state_84;
   int first_entry_86 = 1;
   int first_entry_80 = 1;
   struct {
      int TSFirst;
      int TSLast;
      int field_2;
      int TSFirst_expire;
      int TSLast_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_87;
   IM_RELC *A_89;
   int first_entry_88 = 1;
   IM_RELC *B_89;
   int first_entry_90 = 1;
   int index_89 = 0;
   int first_entry_91 = 1;
   IM_RELC *EH_97;
   int first_entry_96 = 1;
   int first_entry_97 = 1;
   int index_95 = 0;
   int terminating_95=0;
   struct gb_status_95 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int count_0_last_out;
      bool count_0_iterate;
      bool count_0_init;
   };
   struct gb_status_95 *gbstatus_95 = (struct gb_status_95 *)0;
   
   int first_entry_95 = 1;
   int first_entry_98 = 1;
   IM_RELC *C_98;
   int first_entry_100 = 1;
   int first_entry_89 = 1;
   IM_RELC *EH_102;
   int first_entry_103 = 1;
   int first_entry_104 = 1;
   IM_RELC *B_104;
   int first_entry_106 = 1;
   int first_entry_102 = 1;
   struct {
      int a_0;
      int a_0_expire;
      struct timeval atime;
   } insert_107;
   IM_RELC *state_113;
   int first_entry_112 = 1;
   int first_entry_113 = 1;
   int index_111 = 0;
   int terminating_111=0;
   struct gb_status_111 {
      struct mergeAggr_status *mergeAggr_0;
   };
   struct gb_status_111 *gbstatus_111 = (struct gb_status_111 *)0;
   
   int first_entry_111 = 1;
   int first_entry_109 = 1;
   IM_RELC *EH_114;
   int first_entry_115 = 1;
   int first_entry_116 = 1;
   IM_RELC *state_116;
   int first_entry_118 = 1;
   int first_entry_114 = 1;
   struct {
      int k;
      int W;
      int field_2;
      int k_expire;
      int W_expire;
      int field_2_expire;
      struct timeval atime;
   } insert_119;
   int first_entry_121 = 1;
   IM_RELC *EH_127;
   int first_entry_126 = 1;
   int first_entry_127 = 1;
   int index_125 = 0;
   int terminating_125=0;
   struct gb_status_125 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int sum_0_last_out;
      bool sum_0_iterate;
      bool sum_0_init;
   };
   struct gb_status_125 *gbstatus_125 = (struct gb_status_125 *)0;
   
   int first_entry_125 = 1;
   int first_entry_128 = 1;
   IM_RELC *EH_128;
   int first_entry_130 = 1;
   int first_entry_131 = 1;
   IM_RELC *EH_137;
   int first_entry_136 = 1;
   int first_entry_137 = 1;
   int index_135 = 0;
   int terminating_135=0;
   struct gb_status_135 {
      int _baggr_0_value;
      int _baggr_0_first_entry;
      int min_0_last_out;
      bool min_0_iterate;
      bool min_0_init;
   };
   struct gb_status_135 *gbstatus_135 = (struct gb_status_135 *)0;
   
   int first_entry_135 = 1;
   int first_entry_120 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = state->cursor(state, &state_77, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int maxID;
         int OID;
         int maxID_expire;
         int OID_expire;
         struct timeval atime;
      } state_77_78;
      next_77:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = state_77->c_get(state_77, &key, &data, (first_entry_78)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_78 = 0;
         memcpy(&(state_77_78.maxID), (char*)data.data+0, sizeof(int));
         //printf("Retrieved state_77_78.maxID = %d\n", state_77_78.maxID);
         //fflush(stdout);
         memcpy(&(state_77_78.OID), (char*)data.data+4, sizeof(int));
         //printf("Retrieved state_77_78.OID = %d\n", state_77_78.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_78 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         /*UPDATE STARTS*/
         if (key.data == (char*)0) {
            /* key may not be initialized if r_key is in use */
            key.data = keydata;
         }
         *(int*)((char*)data.data+0) = ((state_77_78.maxID) + 1);
         if ((rc = state_77->c_put(state_77, &key, &data, DB_CURRENT)) != 0) {
            adlabort(rc, "IM_RELC->c_put() or DBC->c_put()");
         }
         /*UPDATE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (state_77 && (rc = state_77->c_close(state_77)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = state->cursor(state, &state_81, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = state->cursor(state, &state_84, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      /* precomputed predicates */
      if (! ((Next == 1)) ) {
         rc = DB_NOTFOUND;
      } else {
         next_80:
         rc = (first_entry_80)? 0:DB_NOTFOUND;
         if (rc == DB_NOTFOUND) first_entry_80=1;
         else {
            first_entry_80=0;
            struct {
               int maxID;
               int maxID_expire;
               struct timeval atime;
            } embed_82;
            int embed_82_cnt = 0;
            struct {
               int maxID;
               int OID;
               int maxID_expire;
               int OID_expire;
               struct timeval atime;
            } state_81_83;
            next_81:
            memset(&key, 0, sizeof(key));
            memset(&data, 0, sizeof(data));
            rc = state_81->c_get(state_81, &key, &data, (first_entry_83)? DB_FIRST:DB_NEXT);
            if (rc==0) {
               first_entry_83 = 0;
               memcpy(&(state_81_83.maxID), (char*)data.data+0, sizeof(int));
               //printf("Retrieved state_81_83.maxID = %d\n", state_81_83.maxID);
               //fflush(stdout);
               memcpy(&(state_81_83.OID), (char*)data.data+4, sizeof(int));
               //printf("Retrieved state_81_83.OID = %d\n", state_81_83.OID);
               //fflush(stdout);
            } else if (rc == DB_NOTFOUND) {
               first_entry_83 = 1;
            } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            first_entry_81 = (rc)? 1:0;
            if (rc==0) {
               embed_82.maxID = state_81_83.maxID;
               if (embed_82_cnt++ ==0) goto next_81; /* scalar opr */
            } /* if (rc == 0) */
            if (embed_82_cnt == 0) {
               fprintf(stderr, "ERR: scalar subquery returns no tuple at line 90.\n");
               exit(1);
            }
            else if (embed_82_cnt >  1) {
               fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 90.\n");
               exit(1);
            }
            rc = 0;
            insert_79.field_0 = ((embed_82.maxID) + 1);
            struct {
               int maxID;
               int maxID_expire;
               struct timeval atime;
            } embed_85;
            int embed_85_cnt = 0;
            struct {
               int maxID;
               int OID;
               int maxID_expire;
               int OID_expire;
               struct timeval atime;
            } state_84_86;
            next_84:
            memset(&key, 0, sizeof(key));
            memset(&data, 0, sizeof(data));
            rc = state_84->c_get(state_84, &key, &data, (first_entry_86)? DB_FIRST:DB_NEXT);
            if (rc==0) {
               first_entry_86 = 0;
               memcpy(&(state_84_86.maxID), (char*)data.data+0, sizeof(int));
               //printf("Retrieved state_84_86.maxID = %d\n", state_84_86.maxID);
               //fflush(stdout);
               memcpy(&(state_84_86.OID), (char*)data.data+4, sizeof(int));
               //printf("Retrieved state_84_86.OID = %d\n", state_84_86.OID);
               //fflush(stdout);
            } else if (rc == DB_NOTFOUND) {
               first_entry_86 = 1;
            } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            if (rc==0) {
               embed_85.maxID = state_84_86.maxID;
            } /* if (rc == 0) */
            insert_79.field_1 = embed_85.maxID;
            insert_79.field_2 = 1;
         } /* if (rc == 0) */
      } /* end of precomputed predicates */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_79.field_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_79.field_1), sizeof(int));
         memcpy((char*)data.data+8, &(insert_79.field_2), sizeof(int));
         data.size = 12;
         key.size = 0;
         if ((rc = EH->put(EH, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (state_81 && (rc = state_81->c_close(state_81)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (state_84 && (rc = state_84->c_close(state_84)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = EH->cursor(EH, &A_89, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &B_89, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &EH_97, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &C_98, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      /* precomputed predicates */
      if (! ((Next == 1)) ) {
         rc = DB_NOTFOUND;
      } else {
         struct {
            int TSFirst;
            int TSLast;
            int OnesCount;
            int OID;
            int TSFirst_expire;
            int TSLast_expire;
            int OnesCount_expire;
            int OID_expire;
            struct timeval atime;
         } A_89_88;
         struct {
            int TSFirst;
            int TSLast;
            int OnesCount;
            int OID;
            int TSFirst_expire;
            int TSLast_expire;
            int OnesCount_expire;
            int OID_expire;
            struct timeval atime;
         } B_89_90;
         next_89:
         while (index_89>=0 && index_89 < 2) { 
            switch(index_89) {
               case 0:
               {
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = A_89->c_get(A_89, &key, &data, (first_entry_88)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_88 = 0;
                     memcpy(&(A_89_88.TSFirst), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved A_89_88.TSFirst = %d\n", A_89_88.TSFirst);
                     //fflush(stdout);
                     memcpy(&(A_89_88.TSLast), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved A_89_88.TSLast = %d\n", A_89_88.TSLast);
                     //fflush(stdout);
                     memcpy(&(A_89_88.OnesCount), (char*)data.data+8, sizeof(int));
                     //printf("Retrieved A_89_88.OnesCount = %d\n", A_89_88.OnesCount);
                     //fflush(stdout);
                     memcpy(&(A_89_88.OID), (char*)data.data+12, sizeof(int));
                     //printf("Retrieved A_89_88.OID = %d\n", A_89_88.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_88 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
               }
               break;
               case 1:
               {
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = B_89->c_get(B_89, &key, &data, (first_entry_90)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_90 = 0;
                     memcpy(&(B_89_90.TSFirst), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved B_89_90.TSFirst = %d\n", B_89_90.TSFirst);
                     //fflush(stdout);
                     memcpy(&(B_89_90.TSLast), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved B_89_90.TSLast = %d\n", B_89_90.TSLast);
                     //fflush(stdout);
                     memcpy(&(B_89_90.OnesCount), (char*)data.data+8, sizeof(int));
                     //printf("Retrieved B_89_90.OnesCount = %d\n", B_89_90.OnesCount);
                     //fflush(stdout);
                     memcpy(&(B_89_90.OID), (char*)data.data+12, sizeof(int));
                     //printf("Retrieved B_89_90.OID = %d\n", B_89_90.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_90 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
               }
               break;
            } /*switch */
            if (rc==0) {
               index_89++;
            } else if (rc==DB_NOTFOUND) {
               index_89--;
            }
         } /* while */
         if (rc!=0) {
            index_89++;    /* set index to the first subgoal */
         } else {
            index_89--;    /* set index to the last subgoal */
            struct {
               int a_0;
               int a_0_expire;
               struct timeval atime;
            } embed_92;
            int embed_92_cnt = 0;
            struct {
               int a_0;
               int a_0_expire;
               struct timeval atime;
            } Q_0011_91_93;
            next_91:
            struct {
               int a_0;
               int a_0_expire;
               struct timeval atime;
            } Q_0010_95_94;
            next_95:
            while (index_95>=0 && index_95 < 2) {
               switch(index_95) {
                  case 0:
                  {
                     if (terminating_95 == 0) {
                        /* get source tuple from qun */
                        struct {
                           int TSFirst;
                           int TSLast;
                           int OnesCount;
                           int OID;
                           int TSFirst_expire;
                           int TSLast_expire;
                           int OnesCount_expire;
                           int OID_expire;
                           struct timeval atime;
                        } EH_97_96;
                        next_97:
                        memset(&key, 0, sizeof(key));
                        memset(&data, 0, sizeof(data));
                        rc = EH_97->c_get(EH_97, &key, &data, (first_entry_96)? DB_FIRST:DB_NEXT);
                        if (rc==0) {
                           first_entry_96 = 0;
                           memcpy(&(EH_97_96.TSFirst), (char*)data.data+0, sizeof(int));
                           //printf("Retrieved EH_97_96.TSFirst = %d\n", EH_97_96.TSFirst);
                           //fflush(stdout);
                           memcpy(&(EH_97_96.TSLast), (char*)data.data+4, sizeof(int));
                           //printf("Retrieved EH_97_96.TSLast = %d\n", EH_97_96.TSLast);
                           //fflush(stdout);
                           memcpy(&(EH_97_96.OnesCount), (char*)data.data+8, sizeof(int));
                           //printf("Retrieved EH_97_96.OnesCount = %d\n", EH_97_96.OnesCount);
                           //fflush(stdout);
                           memcpy(&(EH_97_96.OID), (char*)data.data+12, sizeof(int));
                           //printf("Retrieved EH_97_96.OID = %d\n", EH_97_96.OID);
                           //fflush(stdout);
                        } else if (rc == DB_NOTFOUND) {
                           first_entry_96 = 1;
                        } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                        if (rc==0) {
                           rc = 0;          /* subquery could've overwritten rc */
                           if (!((EH_97_96.OnesCount == 1))) {
                              goto next_97;
                           }
                           Q_0010_95_94.a_0 = EH_97_96.TSFirst;
                        } /* if (rc == 0) */
                        if (rc==0) {
                           first_entry_95 = 0;
                           /* make assignments of non-aggr head expr */
                           /* merge group-by columns into a key */
                           strcpy(gbkey, "____");
                           gbstatus_95 = (struct gb_status_95 *)0;
                           rc = hash_get(95, _rec_id, gbkey, 4, (char**)&gbstatus_95);
                           if (rc == DB_NOTFOUND) {//blah
                              gbstatus_95 = (struct gb_status_95*)malloc(sizeof(*gbstatus_95));
                              gbstatus_95->_baggr_0_first_entry = 1;
                              gbstatus_95->_baggr_0_value = 1;
                              rc = hash_put(95, _rec_id, gbkey, 4, &gbstatus_95);
                           } else if (rc == 0) {
                              /* PHASE iterate */
                              gbstatus_95->_baggr_0_first_entry = 1;
                              gbstatus_95->_baggr_0_value += 1;
                           } else adlabort(rc, "hash->get()");
                        } else if (rc == DB_NOTFOUND) {
                           terminating_95 = 1;
                        }
                     }
                     if (terminating_95 == 1) {
                        if (first_entry_95 == 1) {
                           rc = 0; /* fail on first entry, aggregate on empty set */
                        } else {
                           allkey = (char*)0;
                           rc = hash_get(95, _rec_id, allkey, 4, (char**)&gbstatus_95);
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
                     if (terminating_95 == 1) {
                        if (gbstatus_95 == (struct gb_status_95 *)0) {
                           if (first_entry_95) {
                              rc = 0;
                              Q_0011_91_93.a_0 = 0;
                           }
                        } else 
                        if (gbstatus_95->_baggr_0_first_entry == 1) {
                           Q_0011_91_93.a_0 = gbstatus_95->_baggr_0_value;
                           gbstatus_95->_baggr_0_first_entry = 0;
                           rc = 0;
                        } else {
                           gbstatus_95->_baggr_0_first_entry = 1;
                        }
                     }
                     first_entry_95 = 0;
                  }
                  break;
               } /*end of switch*/
               if (rc == 0) {
                  index_95++;
               }
               if (rc == DB_NOTFOUND) {
                  index_95--;
                  if (terminating_95 == 1 && index_95 == 0) {
                     rc = DB_NOTFOUND;
                  }
               }
            }/*end of while */
            if (rc == 0) index_95--;
            else 
            {
               int rc;		/* local rc */ 
               terminating_95 = 0;
               first_entry_95 = 1;
               index_95 = 0;
               /* free gbstatus */
               do {
                  allkey = (char*)0;
                  rc = hash_get(95, _rec_id, allkey, 4, (char**)&gbstatus_95);
                  if (rc==0) {
                     //printf("freeing 95\n");
                     free(gbstatus_95);
                  }
               } while (rc==0);
               if (rc != DB_NOTFOUND) {
                  adlabort(rc, "hash->get()");
               }
               /* release hash entry */
               hashgb_delete(95, _rec_id);
            }
            first_entry_91 = (rc)? 1:0;
            if (rc==0) {
               embed_92.a_0 = Q_0011_91_93.a_0;
               if (embed_92_cnt++ ==0) goto next_91; /* scalar opr */
            } /* if (rc == 0) */
            if (embed_92_cnt == 0) {
               fprintf(stderr, "ERR: scalar subquery returns no tuple at line 115.\n");
               exit(1);
            }
            else if (embed_92_cnt >  1) {
               fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 115.\n");
               exit(1);
            }
            rc = 0;
            rc = 0;          /* subquery could've overwritten rc */
            if (!((embed_92.a_0 > ((k) + 1)))) {
               goto next_89;
            }
            rc = 0;          /* subquery could've overwritten rc */
            if (!((A_89_88.TSLast == B_89_90.TSFirst))) {
               goto next_89;
            }
            rc = 0;          /* subquery could've overwritten rc */
            if (!((A_89_88.OnesCount == B_89_90.OnesCount))) {
               goto next_89;
            }
            rc = 0;          /* subquery could've overwritten rc */
            if (!((A_89_88.OnesCount == 1))) {
               goto next_89;
            }
            struct {
               int exists;
               int exists_expire;
               struct timeval atime;
            } embed_99;
            int embed_99_cnt = 0;
            struct {
               int TSFirst;
               int TSLast;
               int OnesCount;
               int OID;
               int TSFirst_expire;
               int TSLast_expire;
               int OnesCount_expire;
               int OID_expire;
               struct timeval atime;
            } C_98_100;
            next_98:
            memset(&key, 0, sizeof(key));
            memset(&data, 0, sizeof(data));
            rc = C_98->c_get(C_98, &key, &data, (first_entry_100)? DB_FIRST:DB_NEXT);
            if (rc==0) {
               first_entry_100 = 0;
               memcpy(&(C_98_100.TSFirst), (char*)data.data+0, sizeof(int));
               //printf("Retrieved C_98_100.TSFirst = %d\n", C_98_100.TSFirst);
               //fflush(stdout);
               memcpy(&(C_98_100.TSLast), (char*)data.data+4, sizeof(int));
               //printf("Retrieved C_98_100.TSLast = %d\n", C_98_100.TSLast);
               //fflush(stdout);
               memcpy(&(C_98_100.OnesCount), (char*)data.data+8, sizeof(int));
               //printf("Retrieved C_98_100.OnesCount = %d\n", C_98_100.OnesCount);
               //fflush(stdout);
               memcpy(&(C_98_100.OID), (char*)data.data+12, sizeof(int));
               //printf("Retrieved C_98_100.OID = %d\n", C_98_100.OID);
               //fflush(stdout);
            } else if (rc == DB_NOTFOUND) {
               first_entry_100 = 1;
            } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            if (rc==0) {
               rc = 0;          /* subquery could've overwritten rc */
               if (!((C_98_100.TSFirst <= B_89_90.TSLast))) {
                  goto next_98;
               }
               rc = 0;          /* subquery could've overwritten rc */
               if (!((C_98_100.OnesCount == B_89_90.OnesCount))) {
                  goto next_98;
               }
            } /* if (rc == 0) */
            embed_99.exists=(rc==DB_NOTFOUND);
            first_entry_100 = 1;
            rc = 0;          /* subquery could've overwritten rc */
            if (!(embed_99.exists)) {
               goto next_89;
            }
            insert_87.TSFirst = A_89_88.TSFirst;
            insert_87.TSLast = B_89_90.TSLast;
            insert_87.field_2 = ((A_89_88.OnesCount) + B_89_90.OnesCount);
         } /* if (rc == 0) */
      } /* end of precomputed predicates */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)data.data+0, &(insert_87.TSFirst), sizeof(int));
         memcpy((char*)data.data+4, &(insert_87.TSLast), sizeof(int));
         memcpy((char*)data.data+8, &(insert_87.field_2), sizeof(int));
         data.size = 12;
         key.size = 0;
         insertTemp(101, _rec_id, &key, &data);
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   mvTemp(101, _rec_id, EH);
   if (A_89 && (rc = A_89->c_close(A_89)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (B_89 && (rc = B_89->c_close(B_89)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (EH_97 && (rc = EH_97->c_close(EH_97)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (C_98 && (rc = C_98->c_close(C_98)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = EH->cursor(EH, &EH_102, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &B_104, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      /* precomputed predicates */
      if (! ((Next == 1)) ) {
         rc = DB_NOTFOUND;
      } else {
         struct {
            int TSFirst;
            int TSLast;
            int OnesCount;
            int OID;
            int TSFirst_expire;
            int TSLast_expire;
            int OnesCount_expire;
            int OID_expire;
            struct timeval atime;
         } EH_102_103;
         next_102:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = EH_102->c_get(EH_102, &key, &data, (first_entry_103)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_103 = 0;
            memcpy(&(EH_102_103.TSFirst), (char*)data.data+0, sizeof(int));
            //printf("Retrieved EH_102_103.TSFirst = %d\n", EH_102_103.TSFirst);
            //fflush(stdout);
            memcpy(&(EH_102_103.TSLast), (char*)data.data+4, sizeof(int));
            //printf("Retrieved EH_102_103.TSLast = %d\n", EH_102_103.TSLast);
            //fflush(stdout);
            memcpy(&(EH_102_103.OnesCount), (char*)data.data+8, sizeof(int));
            //printf("Retrieved EH_102_103.OnesCount = %d\n", EH_102_103.OnesCount);
            //fflush(stdout);
            memcpy(&(EH_102_103.OID), (char*)data.data+12, sizeof(int));
            //printf("Retrieved EH_102_103.OID = %d\n", EH_102_103.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_103 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         if (rc==0) {
            struct {
               int exists;
               int exists_expire;
               struct timeval atime;
            } embed_105;
            int embed_105_cnt = 0;
            struct {
               int TSFirst;
               int TSLast;
               int OnesCount;
               int OID;
               int TSFirst_expire;
               int TSLast_expire;
               int OnesCount_expire;
               int OID_expire;
               struct timeval atime;
            } B_104_106;
            next_104:
            memset(&key, 0, sizeof(key));
            memset(&data, 0, sizeof(data));
            rc = B_104->c_get(B_104, &key, &data, (first_entry_106)? DB_FIRST:DB_NEXT);
            if (rc==0) {
               first_entry_106 = 0;
               memcpy(&(B_104_106.TSFirst), (char*)data.data+0, sizeof(int));
               //printf("Retrieved B_104_106.TSFirst = %d\n", B_104_106.TSFirst);
               //fflush(stdout);
               memcpy(&(B_104_106.TSLast), (char*)data.data+4, sizeof(int));
               //printf("Retrieved B_104_106.TSLast = %d\n", B_104_106.TSLast);
               //fflush(stdout);
               memcpy(&(B_104_106.OnesCount), (char*)data.data+8, sizeof(int));
               //printf("Retrieved B_104_106.OnesCount = %d\n", B_104_106.OnesCount);
               //fflush(stdout);
               memcpy(&(B_104_106.OID), (char*)data.data+12, sizeof(int));
               //printf("Retrieved B_104_106.OID = %d\n", B_104_106.OID);
               //fflush(stdout);
            } else if (rc == DB_NOTFOUND) {
               first_entry_106 = 1;
            } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
            if (rc==0) {
               rc = 0;          /* subquery could've overwritten rc */
               if (!((EH_102_103.OnesCount < B_104_106.OnesCount))) {
                  goto next_104;
               }
               rc = 0;          /* subquery could've overwritten rc */
               if (!((((EH_102_103.TSFirst == B_104_106.TSFirst)) || ((EH_102_103.TSLast == B_104_106.TSLast))))) {
                  goto next_104;
               }
            } /* if (rc == 0) */
            embed_105.exists=(rc==0);
            first_entry_106 = 1;
            if (rc == 0) {
               first_entry_106 = 1;
            }
            rc = 0;          /* subquery could've overwritten rc */
            if (!(embed_105.exists)) {
               goto next_102;
            }
            /*DELETE STARTS*/
            if ((rc = EH_102->c_del(EH_102, 0)) != 0) {
               adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
            }
            /*DELETE ENDS*/
         } /* if (rc == 0) */
      } /* end of precomputed predicates */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (EH_102 && (rc = EH_102->c_close(EH_102)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (B_104 && (rc = B_104->c_close(B_104)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = state->cursor(state, &state_113, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      /* precomputed predicates */
      if (! ((Next == 1)) ) {
         rc = DB_NOTFOUND;
      } else {
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0013_109_108;
         next_109:
         struct {
            int a_0;
            int a_1;
            int a_0_expire;
            int a_1_expire;
            struct timeval atime;
         } Q_0012_111_110;
         next_111:
         while (index_111>=0 && index_111 < 2) {
            switch(index_111) {
               case 0:
               {
                  if (terminating_111 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int maxID;
                        int OID;
                        int maxID_expire;
                        int OID_expire;
                        struct timeval atime;
                     } state_113_112;
                     next_113:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = state_113->c_get(state_113, &key, &data, (first_entry_112)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_112 = 0;
                        memcpy(&(state_113_112.maxID), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved state_113_112.maxID = %d\n", state_113_112.maxID);
                        //fflush(stdout);
                        memcpy(&(state_113_112.OID), (char*)data.data+4, sizeof(int));
                        //printf("Retrieved state_113_112.OID = %d\n", state_113_112.OID);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_112 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        Q_0012_111_110.a_0 = 2;
                        Q_0012_111_110.a_1 = k;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_111 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_111 = (struct gb_status_111 *)0;
                        rc = hash_get(111, _rec_id, gbkey, 4, (char**)&gbstatus_111);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_111 = (struct gb_status_111*)malloc(sizeof(*gbstatus_111));
                           gbstatus_111->mergeAggr_0 = (struct mergeAggr_status*)malloc(sizeof(struct mergeAggr_status));
                           gbstatus_111->mergeAggr_0->win = 0;
                           setModelId("");
                           mergeAggr_init(gbstatus_111->mergeAggr_0, Q_0012_111_110.a_0, Q_0012_111_110.a_1, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                           rc = hash_put(111, _rec_id, gbkey, 4, &gbstatus_111);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_111 = 1;
                     }
                  }
                  if (terminating_111 == 1) {
                     allkey = (char*)0;
                     rc = hash_get(111, _rec_id, allkey, 4, (char**)&gbstatus_111);
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
                  rc = gbstatus_111->mergeAggr_0->retc->c_get(gbstatus_111->mergeAggr_0->retc, &key, &data, (gbstatus_111->mergeAggr_0->retc_first_entry)? DB_FIRST:DB_NEXT);
                  if (rc == 0) {
                     gbstatus_111->mergeAggr_0->retc_first_entry = 0;
                     memcpy(&(Q_0013_109_108.a_0), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved Q_0013_109_108.a_0 = %d\n", Q_0013_109_108.a_0);
                     //fflush(stdout);
                     if ((rc = gbstatus_111->mergeAggr_0->retc->c_del(gbstatus_111->mergeAggr_0->retc, 0)) != 0) {
                        adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                     }
                  } else if (rc == DB_NOTFOUND) {
                     gbstatus_111->mergeAggr_0->retc_first_entry = 1;
                  } else adlabort(rc, "DBC->c_get()");
                  first_entry_111 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_111++;
            }
            if (rc == DB_NOTFOUND) {
               index_111--;
               if (terminating_111 == 1 && index_111 == 0) {
                  if (gbstatus_111->mergeAggr_0->retc && (rc = gbstatus_111->mergeAggr_0->retc->c_close(gbstatus_111->mergeAggr_0->retc)) != 0) {
                     adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
                  }
                  sprintf(_adl_dbname, "._%d_ret", gbstatus_111->mergeAggr_0);
                  
                  if (gbstatus_111->mergeAggr_0->ret && ((rc = gbstatus_111->mergeAggr_0->ret->close(gbstatus_111->mergeAggr_0->ret, 0)) != 0)) {
                     adlabort(rc, "DB->close()");
                  }
                  gbstatus_111->mergeAggr_0->ret = NULL;
                  (void)unlink(_adl_dbname);
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_111--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_111 = 0;
            first_entry_111 = 1;
            index_111 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(111, _rec_id, allkey, 4, (char**)&gbstatus_111);
               if (rc==0) {
                  free(gbstatus_111->mergeAggr_0);
                  //printf("freeing 111\n");
                  free(gbstatus_111);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(111, _rec_id);
         }
         if (rc==0) {
            insert_107.a_0 = Q_0013_109_108.a_0;
         } /* if (rc == 0) */
      } /* end of precomputed predicates */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_107.a_0);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (state_113 && (rc = state_113->c_close(state_113)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = EH->cursor(EH, &EH_114, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = state->cursor(state, &state_116, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int TSFirst;
         int TSLast;
         int OnesCount;
         int OID;
         int TSFirst_expire;
         int TSLast_expire;
         int OnesCount_expire;
         int OID_expire;
         struct timeval atime;
      } EH_114_115;
      next_114:
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      rc = EH_114->c_get(EH_114, &key, &data, (first_entry_115)? DB_FIRST:DB_NEXT);
      if (rc==0) {
         first_entry_115 = 0;
         memcpy(&(EH_114_115.TSFirst), (char*)data.data+0, sizeof(int));
         //printf("Retrieved EH_114_115.TSFirst = %d\n", EH_114_115.TSFirst);
         //fflush(stdout);
         memcpy(&(EH_114_115.TSLast), (char*)data.data+4, sizeof(int));
         //printf("Retrieved EH_114_115.TSLast = %d\n", EH_114_115.TSLast);
         //fflush(stdout);
         memcpy(&(EH_114_115.OnesCount), (char*)data.data+8, sizeof(int));
         //printf("Retrieved EH_114_115.OnesCount = %d\n", EH_114_115.OnesCount);
         //fflush(stdout);
         memcpy(&(EH_114_115.OID), (char*)data.data+12, sizeof(int));
         //printf("Retrieved EH_114_115.OID = %d\n", EH_114_115.OID);
         //fflush(stdout);
      } else if (rc == DB_NOTFOUND) {
         first_entry_115 = 1;
      } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
      if (rc==0) {
         struct {
            int maxID;
            int maxID_expire;
            struct timeval atime;
         } embed_117;
         int embed_117_cnt = 0;
         struct {
            int maxID;
            int OID;
            int maxID_expire;
            int OID_expire;
            struct timeval atime;
         } state_116_118;
         next_116:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = state_116->c_get(state_116, &key, &data, (first_entry_118)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_118 = 0;
            memcpy(&(state_116_118.maxID), (char*)data.data+0, sizeof(int));
            //printf("Retrieved state_116_118.maxID = %d\n", state_116_118.maxID);
            //fflush(stdout);
            memcpy(&(state_116_118.OID), (char*)data.data+4, sizeof(int));
            //printf("Retrieved state_116_118.OID = %d\n", state_116_118.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_118 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         first_entry_116 = (rc)? 1:0;
         if (rc==0) {
            embed_117.maxID = state_116_118.maxID;
            if (embed_117_cnt++ ==0) goto next_116; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_117_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 176.\n");
            exit(1);
         }
         else if (embed_117_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 176.\n");
            exit(1);
         }
         rc = 0;
         rc = 0;          /* subquery could've overwritten rc */
         if (!((((EH_114_115.TSFirst) + W) < embed_117.maxID))) {
            goto next_114;
         }
         /*DELETE STARTS*/
         if ((rc = EH_114->c_del(EH_114, 0)) != 0) {
            adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
         }
         /*DELETE ENDS*/
      } /* if (rc == 0) */
      if (rc ==0) {
         _adl_cursqlcode = 0; /* SUCCESS */
      }
   } /* while (rc==0) */
   if (EH_114 && (rc = EH_114->c_close(EH_114)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (state_116 && (rc = state_116->c_close(state_116)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = EH->cursor(EH, &EH_127, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &EH_128, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   if ((rc = EH->cursor(EH, &EH_137, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      next_120:
      rc = (first_entry_120)? 0:DB_NOTFOUND;
      if (rc == DB_NOTFOUND) first_entry_120=1;
      else {
         first_entry_120=0;
         insert_119.k = k;
         insert_119.W = W;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } embed_122;
         int embed_122_cnt = 0;
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0015_121_123;
         next_121:
         struct {
            int a_0;
            int a_0_expire;
            struct timeval atime;
         } Q_0014_125_124;
         next_125:
         while (index_125>=0 && index_125 < 2) {
            switch(index_125) {
               case 0:
               {
                  if (terminating_125 == 0) {
                     /* get source tuple from qun */
                     struct {
                        int TSFirst;
                        int TSLast;
                        int OnesCount;
                        int OID;
                        int TSFirst_expire;
                        int TSLast_expire;
                        int OnesCount_expire;
                        int OID_expire;
                        struct timeval atime;
                     } EH_127_126;
                     next_127:
                     memset(&key, 0, sizeof(key));
                     memset(&data, 0, sizeof(data));
                     rc = EH_127->c_get(EH_127, &key, &data, (first_entry_126)? DB_FIRST:DB_NEXT);
                     if (rc==0) {
                        first_entry_126 = 0;
                        memcpy(&(EH_127_126.TSFirst), (char*)data.data+0, sizeof(int));
                        //printf("Retrieved EH_127_126.TSFirst = %d\n", EH_127_126.TSFirst);
                        //fflush(stdout);
                        memcpy(&(EH_127_126.TSLast), (char*)data.data+4, sizeof(int));
                        //printf("Retrieved EH_127_126.TSLast = %d\n", EH_127_126.TSLast);
                        //fflush(stdout);
                        memcpy(&(EH_127_126.OnesCount), (char*)data.data+8, sizeof(int));
                        //printf("Retrieved EH_127_126.OnesCount = %d\n", EH_127_126.OnesCount);
                        //fflush(stdout);
                        memcpy(&(EH_127_126.OID), (char*)data.data+12, sizeof(int));
                        //printf("Retrieved EH_127_126.OID = %d\n", EH_127_126.OID);
                        //fflush(stdout);
                     } else if (rc == DB_NOTFOUND) {
                        first_entry_126 = 1;
                     } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                     if (rc==0) {
                        Q_0014_125_124.a_0 = EH_127_126.OnesCount;
                     } /* if (rc == 0) */
                     if (rc==0) {
                        first_entry_125 = 0;
                        /* make assignments of non-aggr head expr */
                        /* merge group-by columns into a key */
                        strcpy(gbkey, "____");
                        gbstatus_125 = (struct gb_status_125 *)0;
                        rc = hash_get(125, _rec_id, gbkey, 4, (char**)&gbstatus_125);
                        if (rc == DB_NOTFOUND) {//blah
                           gbstatus_125 = (struct gb_status_125*)malloc(sizeof(*gbstatus_125));
                           gbstatus_125->_baggr_0_first_entry = 1;
                           gbstatus_125->_baggr_0_value =  Q_0014_125_124.a_0;
                           rc = hash_put(125, _rec_id, gbkey, 4, &gbstatus_125);
                        } else if (rc == 0) {
                           /* PHASE iterate */
                           gbstatus_125->_baggr_0_first_entry = 1;
                           gbstatus_125->_baggr_0_value +=  Q_0014_125_124.a_0;
                        } else adlabort(rc, "hash->get()");
                     } else if (rc == DB_NOTFOUND) {
                        terminating_125 = 1;
                     }
                  }
                  if (terminating_125 == 1) {
                     if (first_entry_125 == 1) {
                        rc = 0; /* fail on first entry, aggregate on empty set */
                     } else {
                        allkey = (char*)0;
                        rc = hash_get(125, _rec_id, allkey, 4, (char**)&gbstatus_125);
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
                  if (terminating_125 == 1) {
                     if (gbstatus_125 == (struct gb_status_125 *)0) {
                        if (first_entry_125) {
                           rc = 0;
                           Q_0015_121_123.a_0 = 0;
                        }
                     } else 
                     if (gbstatus_125->_baggr_0_first_entry == 1) {
                        Q_0015_121_123.a_0 = gbstatus_125->_baggr_0_value;
                        gbstatus_125->_baggr_0_first_entry = 0;
                        rc = 0;
                     } else {
                        gbstatus_125->_baggr_0_first_entry = 1;
                     }
                  }
                  first_entry_125 = 0;
               }
               break;
            } /*end of switch*/
            if (rc == 0) {
               index_125++;
            }
            if (rc == DB_NOTFOUND) {
               index_125--;
               if (terminating_125 == 1 && index_125 == 0) {
                  rc = DB_NOTFOUND;
               }
            }
         }/*end of while */
         if (rc == 0) index_125--;
         else 
         {
            int rc;		/* local rc */ 
            terminating_125 = 0;
            first_entry_125 = 1;
            index_125 = 0;
            /* free gbstatus */
            do {
               allkey = (char*)0;
               rc = hash_get(125, _rec_id, allkey, 4, (char**)&gbstatus_125);
               if (rc==0) {
                  //printf("freeing 125\n");
                  free(gbstatus_125);
               }
            } while (rc==0);
            if (rc != DB_NOTFOUND) {
               adlabort(rc, "hash->get()");
            }
            /* release hash entry */
            hashgb_delete(125, _rec_id);
         }
         first_entry_121 = (rc)? 1:0;
         if (rc==0) {
            embed_122.a_0 = Q_0015_121_123.a_0;
            if (embed_122_cnt++ ==0) goto next_121; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_122_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 186.\n");
            exit(1);
         }
         else if (embed_122_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 186.\n");
            exit(1);
         }
         rc = 0;
         struct {
            int field_0;
            int field_0_expire;
            struct timeval atime;
         } embed_129;
         int embed_129_cnt = 0;
         struct {
            int TSFirst;
            int TSLast;
            int OnesCount;
            int OID;
            int TSFirst_expire;
            int TSLast_expire;
            int OnesCount_expire;
            int OID_expire;
            struct timeval atime;
         } EH_128_130;
         next_128:
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         rc = EH_128->c_get(EH_128, &key, &data, (first_entry_130)? DB_FIRST:DB_NEXT);
         if (rc==0) {
            first_entry_130 = 0;
            memcpy(&(EH_128_130.TSFirst), (char*)data.data+0, sizeof(int));
            //printf("Retrieved EH_128_130.TSFirst = %d\n", EH_128_130.TSFirst);
            //fflush(stdout);
            memcpy(&(EH_128_130.TSLast), (char*)data.data+4, sizeof(int));
            //printf("Retrieved EH_128_130.TSLast = %d\n", EH_128_130.TSLast);
            //fflush(stdout);
            memcpy(&(EH_128_130.OnesCount), (char*)data.data+8, sizeof(int));
            //printf("Retrieved EH_128_130.OnesCount = %d\n", EH_128_130.OnesCount);
            //fflush(stdout);
            memcpy(&(EH_128_130.OID), (char*)data.data+12, sizeof(int));
            //printf("Retrieved EH_128_130.OID = %d\n", EH_128_130.OID);
            //fflush(stdout);
         } else if (rc == DB_NOTFOUND) {
            first_entry_130 = 1;
         } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
         first_entry_128 = (rc)? 1:0;
         if (rc==0) {
            struct {
               int a_0;
               int a_0_expire;
               struct timeval atime;
            } embed_132;
            int embed_132_cnt = 0;
            struct {
               int a_0;
               int a_0_expire;
               struct timeval atime;
            } Q_0017_131_133;
            next_131:
            struct {
               int a_0;
               int a_0_expire;
               struct timeval atime;
            } Q_0016_135_134;
            next_135:
            while (index_135>=0 && index_135 < 2) {
               switch(index_135) {
                  case 0:
                  {
                     if (terminating_135 == 0) {
                        /* get source tuple from qun */
                        struct {
                           int TSFirst;
                           int TSLast;
                           int OnesCount;
                           int OID;
                           int TSFirst_expire;
                           int TSLast_expire;
                           int OnesCount_expire;
                           int OID_expire;
                           struct timeval atime;
                        } EH_137_136;
                        next_137:
                        memset(&key, 0, sizeof(key));
                        memset(&data, 0, sizeof(data));
                        rc = EH_137->c_get(EH_137, &key, &data, (first_entry_136)? DB_FIRST:DB_NEXT);
                        if (rc==0) {
                           first_entry_136 = 0;
                           memcpy(&(EH_137_136.TSFirst), (char*)data.data+0, sizeof(int));
                           //printf("Retrieved EH_137_136.TSFirst = %d\n", EH_137_136.TSFirst);
                           //fflush(stdout);
                           memcpy(&(EH_137_136.TSLast), (char*)data.data+4, sizeof(int));
                           //printf("Retrieved EH_137_136.TSLast = %d\n", EH_137_136.TSLast);
                           //fflush(stdout);
                           memcpy(&(EH_137_136.OnesCount), (char*)data.data+8, sizeof(int));
                           //printf("Retrieved EH_137_136.OnesCount = %d\n", EH_137_136.OnesCount);
                           //fflush(stdout);
                           memcpy(&(EH_137_136.OID), (char*)data.data+12, sizeof(int));
                           //printf("Retrieved EH_137_136.OID = %d\n", EH_137_136.OID);
                           //fflush(stdout);
                        } else if (rc == DB_NOTFOUND) {
                           first_entry_136 = 1;
                        } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                        if (rc==0) {
                           Q_0016_135_134.a_0 = EH_137_136.TSFirst;
                        } /* if (rc == 0) */
                        if (rc==0) {
                           first_entry_135 = 0;
                           /* make assignments of non-aggr head expr */
                           /* merge group-by columns into a key */
                           strcpy(gbkey, "____");
                           gbstatus_135 = (struct gb_status_135 *)0;
                           rc = hash_get(135, _rec_id, gbkey, 4, (char**)&gbstatus_135);
                           if (rc == DB_NOTFOUND) {//blah
                              gbstatus_135 = (struct gb_status_135*)malloc(sizeof(*gbstatus_135));
                              gbstatus_135->_baggr_0_first_entry = 1;
                              gbstatus_135->_baggr_0_value =  Q_0016_135_134.a_0;
                              rc = hash_put(135, _rec_id, gbkey, 4, &gbstatus_135);
                           } else if (rc == 0) {
                              /* PHASE iterate */
                              gbstatus_135->_baggr_0_first_entry = 1;
                              if (gbstatus_135->_baggr_0_value >  Q_0016_135_134.a_0) {
                                 gbstatus_135->_baggr_0_value =  Q_0016_135_134.a_0;
                              }
                           } else adlabort(rc, "hash->get()");
                        } else if (rc == DB_NOTFOUND) {
                           terminating_135 = 1;
                        }
                     }
                     if (terminating_135 == 1) {
                        allkey = (char*)0;
                        rc = hash_get(135, _rec_id, allkey, 4, (char**)&gbstatus_135);
                        if (rc==0) {
                        } else if(rc == DB_NOTFOUND) {
                        } else adlabort(rc, "hash->get()");
                     }
                  }
                  break;
                  case 1:
                  {
                     rc = DB_NOTFOUND;
                     if (terminating_135 == 1) {
                        if (gbstatus_135->_baggr_0_first_entry == 1) {
                           Q_0017_131_133.a_0 = gbstatus_135->_baggr_0_value;
                           gbstatus_135->_baggr_0_first_entry = 0;
                           rc = 0;
                        } else {
                           gbstatus_135->_baggr_0_first_entry = 1;
                        }
                     }
                     first_entry_135 = 0;
                  }
                  break;
               } /*end of switch*/
               if (rc == 0) {
                  index_135++;
               }
               if (rc == DB_NOTFOUND) {
                  index_135--;
                  if (terminating_135 == 1 && index_135 == 0) {
                     rc = DB_NOTFOUND;
                  }
               }
            }/*end of while */
            if (rc == 0) index_135--;
            else 
            {
               int rc;		/* local rc */ 
               terminating_135 = 0;
               first_entry_135 = 1;
               index_135 = 0;
               /* free gbstatus */
               do {
                  allkey = (char*)0;
                  rc = hash_get(135, _rec_id, allkey, 4, (char**)&gbstatus_135);
                  if (rc==0) {
                     //printf("freeing 135\n");
                     free(gbstatus_135);
                  }
               } while (rc==0);
               if (rc != DB_NOTFOUND) {
                  adlabort(rc, "hash->get()");
               }
               /* release hash entry */
               hashgb_delete(135, _rec_id);
            }
            first_entry_131 = (rc)? 1:0;
            if (rc==0) {
               embed_132.a_0 = Q_0017_131_133.a_0;
               if (embed_132_cnt++ ==0) goto next_131; /* scalar opr */
            } /* if (rc == 0) */
            if (embed_132_cnt == 0) {
               fprintf(stderr, "ERR: scalar subquery returns no tuple at line 188.\n");
               exit(1);
            }
            else if (embed_132_cnt >  1) {
               fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 188.\n");
               exit(1);
            }
            rc = 0;
            rc = 0;          /* subquery could've overwritten rc */
            if (!((EH_128_130.TSFirst == embed_132.a_0))) {
               goto next_128;
            }
            embed_129.field_0 = ((EH_128_130.OnesCount) / 2);
            if (embed_129_cnt++ ==0) goto next_128; /* scalar opr */
         } /* if (rc == 0) */
         if (embed_129_cnt == 0) {
            fprintf(stderr, "ERR: scalar subquery returns no tuple at line 187.\n");
            exit(1);
         }
         else if (embed_129_cnt >  1) {
            fprintf(stderr, "ERR: scalar subquery returns more than one tuple at line 187.\n");
            exit(1);
         }
         rc = 0;
         insert_119.field_2 = ((embed_122.a_0) - embed_129.field_0);
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_119.k), sizeof(int));
         memcpy((char*)data.data+0, &(insert_119.k), sizeof(int));
         memcpy((char*)data.data+4, &(insert_119.W), sizeof(int));
         memcpy((char*)data.data+8, &(insert_119.field_2), sizeof(int));
         data.size = 12;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (EH_127 && (rc = EH_127->c_close(EH_127)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (EH_128 && (rc = EH_128->c_close(EH_128)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   if (EH_137 && (rc = EH_137->c_close(EH_137)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
extern "C" void EHSimpleCount_terminate(struct EHSimpleCount_status *status, 
	int k, int W, int Next, int _rec_id, int not_delete, bufferMngr* bm, 
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
   if(!not_delete) status->retc_first_entry=1;
}
struct inWinType_EHSimpleCount_window {
   int k;
   int W;
   int Next;
};
struct EHSimpleCount_window_status {
   winbuf *win;
   int last_out;
   bool iterate;
   bool init;
   IM_REL *ret;
   IM_RELC *retc;
   int retc_first_entry;
};
inWinType_EHSimpleCount_window* getLastTuple_EHSimpleCount_window(IM_REL* inwindow, inWinType_EHSimpleCount_window* tuple, bufferMngr* bm) {
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
   memcpy(&((*tuple).W), (char*)data.data+4, sizeof(int));
   //printf("Retrieved (*tuple).W = %d\n", (*tuple).W);
   //fflush(stdout);
   memcpy(&((*tuple).Next), (char*)data.data+8, sizeof(int));
   //printf("Retrieved (*tuple).Next = %d\n", (*tuple).Next);
   //fflush(stdout);
   return tuple;
}
extern "C" void EHSimpleCount_window_init(struct EHSimpleCount_window_status *status, 
	int k, int W, int Next, int _rec_id, int __is_init=1, bufferMngr* bm=NULL, 
	hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables = NULL, 
	vector<A_timeexp>* plist=NULL, int endSlide=0, 
	char* _modelId=NULL);
extern "C" void EHSimpleCount_window_init(struct EHSimpleCount_window_status *status, int k, int W, int Next, 
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
   struct inWinType_EHSimpleCount_window tuple;
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
      int a_1;
      int a_2;
      int a_0_expire;
      int a_1_expire;
      int a_2_expire;
      struct timeval atime;
   } insert_138;
   IM_RELC *w_144;
   int first_entry_143 = 1;
   int first_entry_144 = 1;
   int index_142 = 0;
   int terminating_142=0;
   struct gb_status_142 {
      struct EHSimpleCount_status *EHSimpleCount_0;
   };
   struct gb_status_142 *gbstatus_142 = (struct gb_status_142 *)0;
   
   int first_entry_142 = 1;
   int first_entry_140 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = window->cursor(window, &w_144, 0)) != 0) {
      adlabort(rc, "WINDOW->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_1;
         int a_2;
         int a_0_expire;
         int a_1_expire;
         int a_2_expire;
         struct timeval atime;
      } Q_0019_140_139;
      next_140:
      struct {
         int a_0;
         int a_1;
         int a_2;
         int a_0_expire;
         int a_1_expire;
         int a_2_expire;
         struct timeval atime;
      } Q_0018_142_141;
      next_142:
      while (index_142>=0 && index_142 < 2) {
         switch(index_142) {
            case 0:
            {
               if (terminating_142 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int k;
                     int W;
                     int Next;
                     int k_expire;
                     int W_expire;
                     int Next_expire;
                     struct timeval atime;
                  } w_144_143;
                  next_144:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = w_144->c_get(w_144, &key, &data, (first_entry_143)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_143 = 0;
                     memcpy(&(w_144_143.k), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved w_144_143.k = %d\n", w_144_143.k);
                     //fflush(stdout);
                     memcpy(&(w_144_143.W), (char*)data.data+4, sizeof(int));
                     //printf("Retrieved w_144_143.W = %d\n", w_144_143.W);
                     //fflush(stdout);
                     memcpy(&(w_144_143.Next), (char*)data.data+8, sizeof(int));
                     //printf("Retrieved w_144_143.Next = %d\n", w_144_143.Next);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_143 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0018_142_141.a_0 = w_144_143.k;
                     Q_0018_142_141.a_1 = w_144_143.W;
                     Q_0018_142_141.a_2 = w_144_143.Next;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_142 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_142 = (struct gb_status_142 *)0;
                     rc = hash_get(142, _rec_id, gbkey, 4, (char**)&gbstatus_142);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_142 = (struct gb_status_142*)malloc(sizeof(*gbstatus_142));
                        gbstatus_142->EHSimpleCount_0 = (struct EHSimpleCount_status*)malloc(sizeof(struct EHSimpleCount_status));
                        gbstatus_142->EHSimpleCount_0->win = 0;
                        setModelId("");
                        EHSimpleCount_init(gbstatus_142->EHSimpleCount_0, Q_0018_142_141.a_0, Q_0018_142_141.a_1, Q_0018_142_141.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(142, _rec_id, gbkey, 4, &gbstatus_142);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        EHSimpleCount_iterate(gbstatus_142->EHSimpleCount_0, Q_0018_142_141.a_0, Q_0018_142_141.a_1, Q_0018_142_141.a_2, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_142 = 1;
                  }
               }
               if (terminating_142 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(142, _rec_id, allkey, 4, (char**)&gbstatus_142);
                  if (rc==0) {
                     setModelId("");
                     EHSimpleCount_terminate(gbstatus_142->EHSimpleCount_0, Q_0018_142_141.a_0, Q_0018_142_141.a_1, Q_0018_142_141.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_142->EHSimpleCount_0->retc->c_get(gbstatus_142->EHSimpleCount_0->retc, &key, &data, (gbstatus_142->EHSimpleCount_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_142->EHSimpleCount_0->retc_first_entry = 0;
                  memcpy(&(Q_0019_140_139.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0019_140_139.a_0 = %d\n", Q_0019_140_139.a_0);
                  //fflush(stdout);
                  memcpy(&(Q_0019_140_139.a_1), (char*)data.data+4, sizeof(int));
                  //printf("Retrieved Q_0019_140_139.a_1 = %d\n", Q_0019_140_139.a_1);
                  //fflush(stdout);
                  memcpy(&(Q_0019_140_139.a_2), (char*)data.data+8, sizeof(int));
                  //printf("Retrieved Q_0019_140_139.a_2 = %d\n", Q_0019_140_139.a_2);
                  //fflush(stdout);
                  if ((rc = gbstatus_142->EHSimpleCount_0->retc->c_del(gbstatus_142->EHSimpleCount_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_142->EHSimpleCount_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_142 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_142++;
         }
         if (rc == DB_NOTFOUND) {
            index_142--;
            if (terminating_142 == 1 && index_142 == 0) {
               if (gbstatus_142->EHSimpleCount_0->retc && (rc = gbstatus_142->EHSimpleCount_0->retc->c_close(gbstatus_142->EHSimpleCount_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_142->EHSimpleCount_0);
               
               if (gbstatus_142->EHSimpleCount_0->ret && ((rc = gbstatus_142->EHSimpleCount_0->ret->close(gbstatus_142->EHSimpleCount_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_142->EHSimpleCount_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_142--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_142 = 0;
         first_entry_142 = 1;
         index_142 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(142, _rec_id, allkey, 4, (char**)&gbstatus_142);
            if (rc==0) {
               free(gbstatus_142->EHSimpleCount_0);
               //printf("freeing 142\n");
               free(gbstatus_142);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(142, _rec_id);
      }
      if (rc==0) {
         insert_138.a_0 = Q_0019_140_139.a_0;
         insert_138.a_1 = Q_0019_140_139.a_1;
         insert_138.a_2 = Q_0019_140_139.a_2;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         memset(&key, 0, sizeof(key));
         memset(&data, 0, sizeof(data));
         data.data = datadata;
         key.data = keydata;
         memcpy((char*)key.data+0, &(insert_138.a_0), sizeof(int));
         memcpy((char*)data.data+0, &(insert_138.a_0), sizeof(int));
         memcpy((char*)data.data+4, &(insert_138.a_1), sizeof(int));
         memcpy((char*)data.data+8, &(insert_138.a_2), sizeof(int));
         data.size = 12;
         key.size = 4;
         if ((rc = status->ret->put(status->ret, &key, &data, DB_APPEND))!=0) {
            adlabort(rc, "IM_REL->put()");
         }
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (w_144 && (rc = w_144->c_close(w_144)) != 0) {
      adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
   }
   _adl_sqlcode = _adl_cursqlcode;
   status->retc_first_entry=1;
}
/**** Query Declarations ****/
int _adl_statement_145()
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
      FILE *_adl_load = fopen("./test", "rt");
      char _adl_load_buf[40960], *tok;
      char loadkeybuf[1], loaddatabuf[17];
      int _adl_line_no=0;
      char bPoint =0;
      if (!_adl_load) {
         printf("can not open file ./test.\n");
         exit(1);
      }
      memset(&key, 0, sizeof(key));
      memset(&data, 0, sizeof(data));
      key.data = loadkeybuf;
      data.data = loaddatabuf;
      key.size = 0;
      data.size = 16;
      while (fgets(_adl_load_buf, 40959, _adl_load)) {
         _adl_line_no++;
         tok = csvtok(_adl_load_buf, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         *(int*)((char*)data.data+0) = atoi(tok);
         tok = csvtok(NULL, ",\n");
         if (!tok) {
            printf("data format error at line %d\n", _adl_line_no);
            goto exit;
         }
         memset((char*)data.data+4, 0, 8);
         memcpy((char*)data.data+4, &getTimeval(tok), 8);
         if ((rc = traffic->put(traffic, &key, &data, DB_APPEND))!=0) {
            exit(rc);
         }
      } /* end of while */
      fclose(_adl_load);
   }
   _adl_sqlcode = _adl_cursqlcode;
   exit:
   return rc;
};
int _adl_statement_153()
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
      int a_1;
      int a_2;
      int a_0_expire;
      int a_1_expire;
      int a_2_expire;
      struct timeval atime;
   } insert_146;
   IM_RELC *traffic_152;
   int first_entry_151 = 1;
   int first_entry_152 = 1;
   int index_150 = 0;
   int terminating_150=0;
   struct gb_status_150 {
      struct EHSimpleCount_status *EHSimpleCount_0;
   };
   struct gb_status_150 *gbstatus_150 = (struct gb_status_150 *)0;
   
   int first_entry_150 = 1;
   int first_entry_148 = 1;
   rc = 0;
   _adl_cursqlcode = 1 /* ASSUME GUITY */;
   if ((rc = traffic->cursor(traffic, &traffic_152, 0)) != 0) {
      adlabort(rc, "IM_REL->cursor()");
   }
   while (rc==0) {
      struct {
         int a_0;
         int a_1;
         int a_2;
         int a_0_expire;
         int a_1_expire;
         int a_2_expire;
         struct timeval atime;
      } Q_0021_148_147;
      next_148:
      struct {
         int a_0;
         int a_1;
         int a_2;
         int a_0_expire;
         int a_1_expire;
         int a_2_expire;
         struct timeval atime;
      } Q_0020_150_149;
      next_150:
      while (index_150>=0 && index_150 < 2) {
         switch(index_150) {
            case 0:
            {
               if (terminating_150 == 0) {
                  /* get source tuple from qun */
                  struct {
                     int in1;
                     struct timeval time1;
                     int OID;
                     int in1_expire;
                     struct timeval time1_expire;
                     int OID_expire;
                     struct timeval atime;
                  } traffic_152_151;
                  next_152:
                  memset(&key, 0, sizeof(key));
                  memset(&data, 0, sizeof(data));
                  rc = traffic_152->c_get(traffic_152, &key, &data, (first_entry_151)? DB_FIRST:DB_NEXT);
                  if (rc==0) {
                     first_entry_151 = 0;
                     memcpy(&(traffic_152_151.in1), (char*)data.data+0, sizeof(int));
                     //printf("Retrieved traffic_152_151.in1 = %d\n", traffic_152_151.in1);
                     //fflush(stdout);
                     memcpy(&(traffic_152_151.time1), (char*)data.data+4, sizeof(struct timeval));
                     memcpy(&(traffic_152_151.OID), (char*)data.data+12, sizeof(int));
                     //printf("Retrieved traffic_152_151.OID = %d\n", traffic_152_151.OID);
                     //fflush(stdout);
                  } else if (rc == DB_NOTFOUND) {
                     first_entry_151 = 1;
                  } else adlabort(rc, "DBC->c_get() or IM_RELC->c_get()");
                  if (rc==0) {
                     Q_0020_150_149.a_0 = 2;
                     Q_0020_150_149.a_1 = 100;
                     Q_0020_150_149.a_2 = traffic_152_151.in1;
                  } /* if (rc == 0) */
                  if (rc==0) {
                     first_entry_150 = 0;
                     /* make assignments of non-aggr head expr */
                     /* merge group-by columns into a key */
                     strcpy(gbkey, "____");
                     gbstatus_150 = (struct gb_status_150 *)0;
                     rc = hash_get(150, _rec_id, gbkey, 4, (char**)&gbstatus_150);
                     if (rc == DB_NOTFOUND) {//blah
                        gbstatus_150 = (struct gb_status_150*)malloc(sizeof(*gbstatus_150));
                        gbstatus_150->EHSimpleCount_0 = (struct EHSimpleCount_status*)malloc(sizeof(struct EHSimpleCount_status));
                        gbstatus_150->EHSimpleCount_0->win = 0;
                        setModelId("");
                        EHSimpleCount_init(gbstatus_150->EHSimpleCount_0, Q_0020_150_149.a_0, Q_0020_150_149.a_1, Q_0020_150_149.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                        rc = hash_put(150, _rec_id, gbkey, 4, &gbstatus_150);
                     } else if (rc == 0) {
                        /* PHASE iterate */
                        setModelId("");
                        EHSimpleCount_iterate(gbstatus_150->EHSimpleCount_0, Q_0020_150_149.a_0, Q_0020_150_149.a_1, Q_0020_150_149.a_2, _rec_id+1, NULL, inMemTables, NULL, 0, getModelId());
                     } else adlabort(rc, "hash->get()");
                  } else if (rc == DB_NOTFOUND) {
                     terminating_150 = 1;
                  }
               }
               if (terminating_150 == 1) {
                  allkey = (char*)0;
                  rc = hash_get(150, _rec_id, allkey, 4, (char**)&gbstatus_150);
                  if (rc==0) {
                     setModelId("");
                     EHSimpleCount_terminate(gbstatus_150->EHSimpleCount_0, Q_0020_150_149.a_0, Q_0020_150_149.a_1, Q_0020_150_149.a_2, _rec_id+1, 1, NULL, inMemTables, NULL, 0, getModelId());
                  } else if(rc == DB_NOTFOUND) {
                  } else adlabort(rc, "hash->get()");
               }
            }
            break;
            case 1:
            {
               memset(&key, 0, sizeof(key));
               memset(&data, 0, sizeof(data));
               rc = gbstatus_150->EHSimpleCount_0->retc->c_get(gbstatus_150->EHSimpleCount_0->retc, &key, &data, (gbstatus_150->EHSimpleCount_0->retc_first_entry)? DB_FIRST:DB_NEXT);
               if (rc == 0) {
                  gbstatus_150->EHSimpleCount_0->retc_first_entry = 0;
                  memcpy(&(Q_0021_148_147.a_0), (char*)data.data+0, sizeof(int));
                  //printf("Retrieved Q_0021_148_147.a_0 = %d\n", Q_0021_148_147.a_0);
                  //fflush(stdout);
                  memcpy(&(Q_0021_148_147.a_1), (char*)data.data+4, sizeof(int));
                  //printf("Retrieved Q_0021_148_147.a_1 = %d\n", Q_0021_148_147.a_1);
                  //fflush(stdout);
                  memcpy(&(Q_0021_148_147.a_2), (char*)data.data+8, sizeof(int));
                  //printf("Retrieved Q_0021_148_147.a_2 = %d\n", Q_0021_148_147.a_2);
                  //fflush(stdout);
                  if ((rc = gbstatus_150->EHSimpleCount_0->retc->c_del(gbstatus_150->EHSimpleCount_0->retc, 0)) != 0) {
                     adlabort(rc, "IM_RELC->c_del() or DBC->c_del()");
                  }
               } else if (rc == DB_NOTFOUND) {
                  gbstatus_150->EHSimpleCount_0->retc_first_entry = 1;
               } else adlabort(rc, "DBC->c_get()");
               first_entry_150 = 0;
            }
            break;
         } /*end of switch*/
         if (rc == 0) {
            index_150++;
         }
         if (rc == DB_NOTFOUND) {
            index_150--;
            if (terminating_150 == 1 && index_150 == 0) {
               if (gbstatus_150->EHSimpleCount_0->retc && (rc = gbstatus_150->EHSimpleCount_0->retc->c_close(gbstatus_150->EHSimpleCount_0->retc)) != 0) {
                  adlabort(rc, "IM_RELC->c_close() or DBC->c_close()");
               }
               sprintf(_adl_dbname, "._%d_ret", gbstatus_150->EHSimpleCount_0);
               
               if (gbstatus_150->EHSimpleCount_0->ret && ((rc = gbstatus_150->EHSimpleCount_0->ret->close(gbstatus_150->EHSimpleCount_0->ret, 0)) != 0)) {
                  adlabort(rc, "DB->close()");
               }
               gbstatus_150->EHSimpleCount_0->ret = NULL;
               (void)unlink(_adl_dbname);
               rc = DB_NOTFOUND;
            }
         }
      }/*end of while */
      if (rc == 0) index_150--;
      else 
      {
         int rc;		/* local rc */ 
         terminating_150 = 0;
         first_entry_150 = 1;
         index_150 = 0;
         /* free gbstatus */
         do {
            allkey = (char*)0;
            rc = hash_get(150, _rec_id, allkey, 4, (char**)&gbstatus_150);
            if (rc==0) {
               free(gbstatus_150->EHSimpleCount_0);
               //printf("freeing 150\n");
               free(gbstatus_150);
            }
         } while (rc==0);
         if (rc != DB_NOTFOUND) {
            adlabort(rc, "hash->get()");
         }
         /* release hash entry */
         hashgb_delete(150, _rec_id);
      }
      if (rc==0) {
         insert_146.a_0 = Q_0021_148_147.a_0;
         insert_146.a_1 = Q_0021_148_147.a_1;
         insert_146.a_2 = Q_0021_148_147.a_2;
      } /* if (rc == 0) */
      if (rc ==0 && slide_out == 1) {
         _adl_cursqlcode = 0; /* SUCCESS */ 
         /* INSERT STARTS */
         printf("%10d ", insert_146.a_0);
         printf("%10d ", insert_146.a_1);
         printf("%10d ", insert_146.a_2);
         printf("\n");
         /* INSERT ENDS */
      } else 
      slide_out = 1;
   } /* while (rc==0) */
   if (traffic_152 && (rc = traffic_152->c_close(traffic_152)) != 0) {
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
   if ((rc = im_rel_create(&traffic, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = traffic->open(traffic, "_adl_db_traffic", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("traffic") == 0) {
      inMemTables->operator[](strdup("traffic")) = traffic;
   }
   if ((rc = im_rel_create(&EH, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = EH->open(EH, "_adl_db_EH", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("EH") == 0) {
      inMemTables->operator[](strdup("EH")) = EH;
   }
   if ((rc = im_rel_create(&state, NULL, IM_LINKEDLIST, 0)) != 0) {
      adlabort(rc, "im_rel_create()");
   }
   if ((rc = state->open(state, "_adl_db_state", 0)) != 0) {
      adlabort(rc, "open()");
   }
   if (inMemTables->count("state") == 0) {
      inMemTables->operator[](strdup("state")) = state;
   }
   _adl_statement_145();
   _adl_statement_153();
   exit:
   tempdb_delete();
   _adl_dlm_delete();
   
   if ((rc = traffic->close(traffic, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = EH->close(EH, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   
   if ((rc = state->close(state, 0)) != 0) {
      adlabort(rc, "IM_REL->close()");
   }
   return(rc);
};
