#include "dbt.h"
#include <stdio.h>
#include "const.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
extern "C"{
#include <dbug.h>
}
//int dbt::getSize(){
//  return keysz + datasz + sizeof(*atime);
//};
dbt::dbt(int ksz,
	 int dsz
	 ){
  init(ksz, dsz);
};

int dbt::init(int ksz,
	  int dsz){
  keysz = ksz;
  datasz = dsz;  
}
dbt::~dbt(){

};
int dbt::copy(const DBT* d,
	      const timestamp *t, 
	      const DBT* k){
  keysz = k->size;
  memcpy(key, k->data, keysz);
  datasz = d->size;
  memcpy(data, d->data, datasz);
  memcpy(atime, t, TS_SZ);
}

int dbt::copy(const dbt* d){
  DBUG_ENTER("dbt::copy");
  keysz = d->keysz;
  datasz = d->datasz;  
  if (keysz>0 && d->key) {
    if (!key){
      EM_error(ERR_DBT_INCOMPATIBLE, __LINE__, __FILE__, "key");
    }
    memcpy(key, d->key, keysz);
  }
  if (datasz>0 && d->data) {
    if (!data){
      EM_error(ERR_DBT_INCOMPATIBLE, __LINE__, __FILE__, "data");
    }
    else
      memcpy(data, d->data, datasz);
  }
  if (d->atime){
    if (!atime) {
      EM_error(ERR_DBT_INCOMPATIBLE, __LINE__, __FILE__, "timestamp");
    }
    else
      memcpy(atime, d->atime, sizeof(*atime));
  }
  DBUG_RETURN(0);
}
// recover timestamp from data field
void dbt::recoverTS(){
  datasz -= TS_SZ;
  memcpy(atime, data + datasz, TS_SZ);
  data[datasz] = 0;
}  
void dbt::print() const{
  printf("(");
  if (datasz>0) {
    
    int i=-1;
    if(datasz > 4)
      memcpy(&i, data, sizeof(i));
    printf("%s (%d)", data, i);
    /*
    int i;
    memcpy(&i, data, sizeof(i));
    cout<<i;
    */
  }
  printf("(%d)", datasz);
  if (atime) 
    printf(",%d.%d",atime->tv_sec, atime->tv_usec);
  if (keysz>0) 
    printf(",%s(%d)", key, keysz);
  printf(")\n");
  
};

int dbt::get(DBT* d, timestamp *a,DBT* k){
  /*if (k){
    if (key)
      memcpy(k->data, key, keysz);
    k->size = keysz;    
  }*/
  if (d){
    if (data) memcpy(d->data, data, datasz);
    d->size = datasz;
  }
  if (atime) *a = *atime;
  return 0;
}
/*
 */

int sDBT::get(DBT* d, timestamp *a,DBT* k){
  /*if (k){
    if (key)
      memcpy(k->data, key, keysz);
    k->size = keysz;    
  }*/
  if (d){
    if (data) memcpy(d->data, data, datasz);
    d->size = datasz;
  }
  if (atime) *a = *atime;
  return 0;
}
MM* sDBT::dbt_mm=NULL;
MM* sDBT::createSM(){

  dbt_mm = mm_create(DBT_SM_SIZE, NULL);
  if (!dbt_mm){
    perror("Create shared memory for DBTs:");
    return NULL;
  }
  cout<<"DBT shared memory created!\n";
  return dbt_mm;    
};

void sDBT::destroySM(){
  mm_destroy(dbt_mm);
  cout<<"DBT shared memory destroyed!\n";
};
sDBT::sDBT(dbt*d){
  init(d->data, d->datasz,
       d->atime,NULL,
       d->key, d->keysz);
}
sDBT::sDBT() {
}
int sDBT::init(dbt*d){
  init(d->data, d->datasz,
       d->atime,NULL,
       d->key, d->keysz);

}
sDBT::sDBT(DBT* d, timestamp *a,DBT* k){
  init((const char*)d->data, d->size,
       a,NULL,
       (const char*)k->data, k->size);  
};
int sDBT::init(DBT* d, timestamp *a,DBT* k){
  init((const char*)d->data, d->size,
       a,NULL,
       (const char*)k->data, k->size);
};

sDBT::sDBT(const char *d,
	   int dsz,
	   struct timeval *a,
	   sDBT *n,
	   const char * k,
	   int ksz
	  )
   {
     init(d, dsz, a, n,k, ksz);
};
int sDBT::init( const char *d,
		int dsz,
		struct timeval *a,
		sDBT *n,
		const char *k,
		int ksz
		){
  dbt::init(ksz, dsz);
  if (keysz>0) {
    key=(char*)mm_malloc(dbt_mm, keysz);
    if (!key){
      perror("Malloc sDBT.key in shared memory");
      return errno;
    }
    if (k) memcpy(key, k, keysz);
  }
  else
    key = NULL;
  if (datasz>0){
    data=(char*)mm_malloc(dbt_mm, datasz);
    if (!data){
      perror("Malloc sDBT.data in shared memory");
      return errno;
    }
    if (d) memcpy(data, d, datasz);
  }
  else
    data = NULL;
  if (a){
    atime = (struct timeval*)mm_malloc(dbt_mm, sizeof(struct timeval));
    if (!atime){
      perror("Malloc sDBT.atime in shared memory");
      return errno;
    }
    else
      setTime(a);
  }
  else atime = NULL;
  next = n;
  return 0;
};

void sDBT::destroy(){
  //DBUG_ENTER("~sDBT:destroy");
  if (key) mm_free(dbt_mm, key);
  if (data) mm_free(dbt_mm, data);
  if (atime) mm_free(dbt_mm, atime);
  //DBUG_VOID_RETURN;

}

sDBT::~sDBT(){
  destroy();
};

cDBT::cDBT(const char *d,
	   int dsz,
	   struct timeval *a,const char * k,
	   int ksz){
  init(d, dsz, a, k, ksz);
}
//cDBT::cDBT(int dsz){
//  init(NULL, dsz);
//}

int cDBT::init(const char *d,
	       int dsz,
	       struct timeval *a,
	       const char * k,
	       int ksz
	       ){
  DBUG_ENTER("cDBT::init");
  dbt::init(ksz, dsz);
  if (keysz>0) {
    key=(char*)malloc(keysz);
    if (!key){
      perror("Malloc cDBT.key");
      return errno;
    }
    if (k) memcpy(key, k, keysz);
  }
  else
    key = NULL;
  if (datasz>0){
    data=(char*)malloc(datasz);
    if (!data){
      perror("Malloc cDBT.data");
      return errno;
    }
    if (d) memcpy(data, d, datasz);
  }
  else
    data = NULL;
  if (a){
    atime = (struct timeval*)malloc(sizeof(struct timeval));
    if (!atime){
      perror("Malloc cDBT.atime");
      return errno;
    }
    else
      setTime(a);
  }
  else
    atime =NULL;
  DBUG_RETURN(0);

};
int cDBT::init(dbt* d){
  init(d->data, d->datasz, d->atime,d->key, d->keysz);
}
cDBT::cDBT(dbt* d){
  DBUG_ENTER("cDBT::cDBT(dbt*)");
  init(d->data, d->datasz, d->atime,d->key, d->keysz);
  DBUG_VOID_RETURN;
};
int cDBT::init(DBT* d, timestamp *a, DBT* k){
  init((const char*)(d->data), d->size, a, (const char*)(k->data), k->size);
}
cDBT::cDBT( DBT* d, timestamp *a,DBT* k){
  init((const char*)(d->data), d->size, a, (const char*)(k->data), k->size);
};
cDBT::~cDBT(){
  //DBUG_ENTER("~cDBT");
  if (keysz>0) free(key);
  if (datasz>0) free(data);
  if (atime) free(atime);
  //DBUG_VOID_RETURN;
}
cDBT::cDBT(int dsz, timestamp *atime, int ksz){
  init(NULL, dsz, atime, NULL, ksz);
}
