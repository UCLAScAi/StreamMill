#ifndef __DBT_H__
#define __DBT_H__
#include <sys/time.h>
#include <db.h>
#include <types.h>
#include <stdio.h>
#include "const.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
extern "C"{
#include <mm.h>
}
#include <err.h>
#include <const.h>
using namespace std;

// DBT class:
// Do not add any member since we want to keep it minimal;
// Members are public for direct access to gain performance.
// We avoid virtual funtions in dbt otherwise "new" and "delete" won't work
class dbt{
 public:
  char *key;         // Key
  int keysz;         // key size
  char *data;        // Data
  int datasz;        // data size
  struct timeval *atime;         // Arrival time
  dbt(){};
  dbt(int ksz,
      int dsz);
  int init(int ksz, int dsz);
  ~dbt();
  virtual int get(DBT* d, 
	       timestamp *a=NULL,
	       DBT* k=NULL);

  void setTime(struct timeval *a){
    if (a && atime){ // has timestamp
      atime->tv_sec = a->tv_sec;
      atime->tv_usec = a->tv_usec;
    }
    else
      EM_error(ERR_TIMESTAMP, __LINE__, __FILE__, "timestamp field NULL in dbt::setTime");
  };

  void print() const;

  // recover timestamp from data field.  We have to do this since IM_REL doesn't have a timestamp field
  void recoverTS();

  int copy(const dbt *d);
  int copy(const DBT*, const timestamp*, const DBT*);
  // get the tuple actual size
  int getSize();
};

// DBT  in shared memory
class sDBT:public dbt{
 private:

 public:
  static MM* dbt_mm;        // shared memory pool for DBTs
  sDBT *next;

  sDBT();
  sDBT(dbt*);
  sDBT(DBT*, timestamp* atime=NULL, DBT* key=NULL);
  sDBT(const char *d,
       int dsz,
       struct timeval *a=NULL, 
       sDBT* n=NULL,
       const char * k=NULL,
       int ksz=0
       );

  // init() is used for malloc instead of "new" allocation
  int init(dbt*);
  int init(DBT*, timestamp* atime=NULL, 
	   DBT *key =NULL);
  int init(const char *d,
	   int dsz,
	   struct timeval *a=NULL, 
	   sDBT* n=NULL,
   	   const char * k=NULL,
	   int ksz=0);
  int get(DBT* d, 
	       timestamp *a=NULL,
	       DBT* k=NULL);
  ~sDBT();
  void destroy();
  static MM* createSM();
  static void destroySM();


};

// Common DBT: in heap
class cDBT:public dbt{
 public:
  cDBT(int dsz, timestamp *atime=NULL,int ksz=0); // allocate space only
  cDBT(const char * d=NULL,
       int dsz=MAX_STR_LEN,
       struct timeval *a=NULL,
       const char *k=NULL,
       int ksz=0);
  cDBT(dbt* d);
  cDBT(DBT* data, 
	     timestamp *a=NULL,
	     DBT* key=NULL);
  int init(const char * d,
       int dsz,
       struct timeval *a=NULL,
       const char *k=NULL,
       int ksz=0);
  int init(dbt* d);
  int init(DBT* data, 
	timestamp *a=NULL,
	DBT* key=NULL);
  ~cDBT();
};

#endif
