#include <limits.h>
#include <windowBuf.h>
#include <stdio.h>
#include <db.h>
#include <error.h>
#include <adllib.h>
#include <iostream>
#include <adllib.h>
#include <SMLog.h>
extern "C"{
#include <dbug.h>
}



using namespace std;
windowBuf:: windowBuf(const char* name_in, unsigned int size_in,
		   _adl_win_type wtype_in):buffer(name_in, WINBUF){
  
	SMLog::SMLOG(10, "Entering windowBuf:: windowBuf");	
  strcpy(name, name_in);
  mode = win_memory;
  wtype = wtype_in;
  size = size_in;
  tuple_id = 0;
  first = true;
  n = 0;
  current.tv_sec = 0;
  current.tv_usec = 0;
  int rc;
  if ((rc = im_rel_create(&b, NULL, IM_LINKEDLIST, 0)) != 0) {
    adlerror(rc, "im_rel_create() in winbuf()");
  }
  if ((rc = b->open(b, "_adl_db_t", 0)) != 0) {
    adlerror(rc, "open() in winbuf()");
  }
  if ((rc = b->cursor(b, &temp, 0)) !=0){
    adlerror(rc, "IM_REL->cursor in winbuf()");
  }

};
windowBuf::~windowBuf(){
	SMLog::SMLOG(10, "Entering windowBuf::~windowBuf");	
  int rc;
  if ((rc = temp->c_close(temp)) !=0){
    adlerror(rc, "IM_RELC->c_close() in ~winbuf()");
  }
  if ((rc = b->close(b, 0)) != 0) {
    adlerror(rc, "IM_REL->close()");
  }
};

// put for tuple-based window
int windowBuf::put(DBT* data){
  //printf("In put1\n");
  DBT key;
  return put(&key, data);
}


/* buffer interface */
int windowBuf::get(dbt* d){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int windowBuf::get(timestamp* d){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int windowBuf::get(DBT* d, timestamp *atime, DBT *k){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int windowBuf::put(dbt* d){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int windowBuf::put(DBT* d, timestamp *atime, DBT *k){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
/* already implemented 
   int windowBuf::pop(){
   EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
   return -1;
   }
   int windowBuf::empty(){
   //may be overwrite it
   EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
   return -1;
   }
*/



int windowBuf::put(DBT* key, DBT* data){
	SMLog::SMLOG(10, "Entering windowBuf::put");	
  DBUG_ENTER("windowBuf::put");
  int rc;

  char keydata[sizeof(tuple_id)];
  key->data = keydata;
  key->size = sizeof(tuple_id);

  //always use internal timestamp as the key, which is tuple_id
  memcpy(key->data, &tuple_id, sizeof(tuple_id));
  
  //printf("Calling other put w/ t_id %u, key.size %d\n", tuple_id, key->size);

  
  //printf("here\n");
  // put the tuple
  //printf("buf put %d %d %d\n", key->size, data->size, b);
  if ((rc = b->put(b, key, data, DB_APPEND)) !=0){
    adlerror(rc, "IM_REL->put() in windowBuf::put()");
  }


  //printf("after buf put %d\n", b);
  /*n++;
    
    DBT  k, d;
    if (n == 1){ // reset head pointer
      if ((rc = head->c_get(head, &k, &d,DB_FIRST)) !=0){
      adlerror(rc, "IM_REL->c_get() in windowBuf::put() while reseting head pointer");
      }
      goto exit;
      }
      // n>1, move the head pointer
      unsigned int id;
      memcpy(&id, key->data, key->size);
      unsigned int t;
      if (id < size) {
      do{
        getHead(&k, &d);
	memcpy(&t, k.data, k.size);
	}while (t<=UINT_MAX-size+id && t>id  && advanceHead() == 0);
	}
	else // id >= size
	{
	do{
	  getHead(&k, &d);
	  memcpy(&t, k.data, k.size);
	}while ((t<=id-size || t>id)  && advanceHead() == 0);
    }
  */
 exit:
  DBUG_RETURN(rc);
};

int windowBuf::pop(){
	SMLog::SMLOG(10, "Entering windowBuf::pop");	
  DBUG_ENTER("windowBuf::pop()");
  DBT key, data;
  char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  data.data = datadata;
  key.data = keydata;

  int rc = temp->c_get(temp, &key, &data, DB_FIRST);
  if (rc ==0) {
    rc = temp->c_del(temp, 0);
    n--;
  }
  else if (rc !=DB_NOTFOUND)
    adlerror(rc, "IMREL->c_get() in windowBuf::pop()");
  DBUG_RETURN(rc);
  return rc;
};
int windowBuf::empty(){
  if(current.tv_sec == 0 && current.tv_usec == 0) 
    return 0;
  
  timestamp t;
  getTimestamp(&t);
  if(timeval_cmp(t, current) < 0) 
    return 1;
  return 0;
};

int windowBuf::bufSize(){
  return n;
};

int windowBuf::bufByteSize(){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
};

IM_REL* windowBuf::get_im_rel(){
  return b;
};

// advance head curosr by one

bool windowBuf::hasExpired(){
  DBT k, d;

  char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];

  memset(&k, 0, sizeof(k));
  memset(&d, 0, sizeof(d));
  d.data = datadata;
  k.data = keydata;

  int rc = (getExpired(&k,&d)==0);

  //printf("has Expired returning %d\n", rc);
  //fflush(stdout);

  return rc;
}

int windowBuf::getExpired(DBT *data){
  DBT key;

  char keydata[MAX_STR_LEN];

  memset(&key, 0, sizeof(key));
  key.data = keydata;
  return getExpired(&key, data);
}

// get the oldest expired tuple
int windowBuf::getExpired(DBT* key, DBT* data){
	SMLog::SMLOG(10, "Entering windowBuf::getExpired");	
  DBUG_ENTER("getExpired");
  int rc = 0;
  unsigned int t;
  unsigned int id = getTupleID();
  //printf("id %d, size %d\n", id, size);
  if (id < size) {
    if ((rc = temp->c_get(temp, key, data, DB_FIRST)) !=0){
      //printf("Here\n");
      if (rc!=DB_NOTFOUND) 
	adlerror(rc, "get Expired tuple in windowBuf::getExpired()");
      goto exit;
    }
    memcpy(&t, key->data, key->size);
    //printf("data %s t %u UM %u, size %u, id %u\n", data->data, t, UINT_MAX, size, id);
    if(t<=UINT_MAX-size+id && t>id) {
      //printf("rc = 0 - good \n"); 
      rc = 0;
    }
    else {
      //printf("DBNOTFO1\n");
      rc = DB_NOTFOUND;
    }
  }
  else // id >= size
    {
      if ((rc = temp->c_get(temp, key, data, DB_FIRST)) !=0){
        //printf("Here2\n");
	if (rc!=DB_NOTFOUND) 
	  adlerror(rc, "get Expired tuple in windowBuf::getExpired()");
	goto exit;
      }
      memcpy(&t, key->data, key->size);

      //printf("id >= size case, data %s t %u, size %u, id %u\n", data->data, t, size, id);
      if ((t<=id-size || t>id)) {
	//printf("rc = 0 -- good\n");
        rc = 0;
      }
      else {
	//printf("DBNOTFO\n");
     	rc = DB_NOTFOUND;
      }
    }

exit:
  DBUG_RETURN(rc);
  return rc;
}

// get the youngest tuple
/*int windowBuf::getTail(DBT* key, DBT* data){
  DBUG_ENTER("getTail");
  int rc;
  if ((rc = temp->c_get(temp, key, data, DB_LAST)) !=0){
    if (rc!=DB_NOTFOUND) 
      adlerror(rc, "get last tuple in windowBuf::getTail()");
    return rc;
  }
  else{
  }
  DBUG_RETURN(rc);
  return rc;
  }*/


int windowBuf::printTuple(DBT* key, DBT* data){
  unsigned int* i = (unsigned int*)(key->data);
  cout<<*i<<endl;
}
int windowBuf::print(){
	SMLog::SMLOG(10, "Entering windowBuf::print");	
  timestamp ts;
  DBT key, data;
  char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  data.data = datadata;
  key.data = keydata;

  int rc;
  printf("------ %s window size=%d------\n", wtype==_ADL_WIN_ROW? "Physical" :"Logical", size);
  bool first =true;
  int i = 0;
  while ((rc = temp->c_get(temp, &key, &data, first?DB_FIRST:DB_NEXT)) == 0){
    first = false;
    i++;
    printTuple(&key, &data);
  }
  if (rc && rc != DB_NOTFOUND)
    adlerror(rc, "IM_RELC->c_get() in windowBuf::print()");
  printf("tuples in window %d ------------\n", i);
  return rc;
}

void windowBuf::resetTS(timestamp *ts){
  first_ts.tv_sec = (*ts).tv_sec;
  first_ts.tv_usec = (*ts).tv_usec;
};

void windowBuf::getTimestamp(timestamp *atime) {
	SMLog::SMLOG(10, "Entering windowBuf::getTimestamp");	
  if(wtype == _ADL_WIN_TIME) {
    long microsecsInTho = (tuple_id % 1000);
    long microsecs = first_ts.tv_usec + 1000 * microsecsInTho;
    
    if(microsecs >= 1000000) {
      atime->tv_sec = first_ts.tv_sec + ((tuple_id - microsecsInTho) / 1000)+1;
      atime->tv_usec = microsecs - 1000000;
    }
    else {
      atime->tv_sec = first_ts.tv_sec + ((tuple_id - microsecsInTho) / 1000);
      atime->tv_usec = microsecs;
    }
  }
  else {
    atime->tv_sec = current.tv_sec;
    atime->tv_usec = current.tv_usec;
  }
}

void windowBuf::setCurrentTimestamp(timestamp *ts) {
  current.tv_sec = ts->tv_sec;
  current.tv_usec = ts->tv_usec;
}

unsigned int windowBuf::getTupleID(){
  return tuple_id;
}

int windowBuf::updateTupleID(timestamp *ts){
	SMLog::SMLOG(10, "Entering windowBuf::updateTupleID");	
  if (ts){ // time-base window
    //printf("in update timestamp %d %d", ts->tv_sec, ts->tv_usec);
    if (first){
      first = false;
      resetTS(ts);
    }
    tuple_id = (unsigned int)(timeval_subtract(*ts, first_ts) * 1000);
    //current.tv_sec = ts->tv_sec;
    //current.tv_usec = ts->tv_usec;
    //printf("TupleId = %d; %d,%d; %d,%d; %f\n", tuple_id, 
    //   ts->tv_sec, ts->tv_usec, first_ts.tv_sec, first_ts.tv_usec,
    //   timeval_subtract(*ts, first_ts));

  }
  else{ // count-based window
    tuple_id++;
  }

  // delete expired tuples  
  //while (hasExpired()){
  //pop();
  //}

}




















