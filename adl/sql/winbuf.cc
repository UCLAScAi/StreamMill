#include <limits.h>
#include <winbuf.h>
#include <stdio.h>
#include <db.h>
#include <error.h>
#include <adllib.h>
#include <iostream>
#include <adllib.h>
extern "C"{
#include <dbug.h>
}

using namespace std;
winbuf::    winbuf(unsigned int size_in,
		   int datasize_in,
		   int keysize_in,
		   _adl_win_type wtype_in){
  mode = win_memory;
  wtype = wtype_in;
  size = size_in;
  datasize = datasize_in;
  keysize = keysize_in;
  keybegin = datasize+sizeof(tuple_id);
  plidbegin = keybegin+keysize; 
  tuple_id = 0;
  first = true;
  n = 0;
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
winbuf::    ~winbuf(){
  int rc;
  if ((rc = temp->c_close(temp)) !=0){
    adlerror(rc, "IM_RELC->c_close() in ~winbuf()");
  }
  if ((rc = b->close(b, 0)) != 0) {
    adlerror(rc, "IM_REL->close()");
  }
};
/*
int winbuf::get(char** d){
  int rc=0;
  if (getExpired(d) !=0)
    rc = getHead(d);
  return rc;
};
*/
// put for tuple-based window
int winbuf::put(DBT* data){
  //printf("In put1\n");
  DBT key;
  //printf("In put2\n");
  char keydata[1];
  char datadata[datasize+sizeof(tuple_id)];
  //printf("In put3\n");
  keydata[0] = 0;
  key.data = keydata;
  key.size = 0;
  //key.size = sizeof(tuple_id);
  //memcpy(key.data, &tuple_id, sizeof(tuple_id));
  memcpy(datadata, data->data, datasize);
  memcpy(datadata+datasize, &tuple_id, sizeof(tuple_id));
  data->size = datasize + sizeof(tuple_id);
  //may be we should free data->data first, not sure since we don't do malloc for it
  data->data = datadata;
  //printf("Calling other put\n");
  return put(&key, data);
}

int winbuf::put(DBT* data, char* gbkey, int id){
  //printf("In put1\n");
  DBT key;
  //printf("In put2\n");
  char keydata[1];
  char datadata[datasize+sizeof(tuple_id)+keysize+sizeof(int)];
  //printf("In put3\n");
  keydata[0] = 0;
  key.data = keydata;
  key.size = 0;
  //key.size = sizeof(tuple_id);
  //memcpy(key.data, &tuple_id, sizeof(tuple_id));
  memcpy(datadata, data->data, datasize);
  memcpy(datadata+datasize, &tuple_id, sizeof(tuple_id));
  memcpy(datadata+datasize+sizeof(tuple_id), gbkey, keysize);
  memcpy(datadata+datasize+sizeof(tuple_id)+keysize, &id, sizeof(int));
  data->size = datasize + sizeof(tuple_id) + keysize + sizeof(int);
  //may be we should free data->data first, not sure since we don't do malloc for it
  data->data = datadata;
  //printf("Calling other put\n");
  return put(&key, data);
}

int winbuf::put(DBT* key, DBT* data){
  DBUG_ENTER("winbuf::put");
  int rc;
  
  // put the tuple
  //printf("buf put %d %d %d\n", key->size, data->size, b);
  if ((rc = b->put(b, key, data, DB_APPEND)) !=0){
    adlerror(rc, "IM_REL->put() in winbuf::put()");
  }
  //printf("after buf put %d\n", b);
  /*n++;
    
    DBT  k, d;
    if (n == 1){ // reset head pointer
      if ((rc = head->c_get(head, &k, &d,DB_FIRST)) !=0){
      adlerror(rc, "IM_REL->c_get() in winbuf::put() while reseting head pointer");
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

int winbuf::pop(){
  DBUG_ENTER("winbuf::pop()");
  DBT key, data;
  int rc = temp->c_get(temp, &key, &data, DB_FIRST);
  if (rc ==0) {
    rc = temp->c_del(temp, 0);
    n--;
  }
  else if (rc !=DB_NOTFOUND)
    adlerror(rc, "IMREL->c_get() in winbuf::pop()");
  DBUG_RETURN(rc);
  return rc;
};
int winbuf::empty(){
  DBT key, data;
  int rc = temp->c_get(temp, &key, &data, DB_FIRST);
  if(rc == DB_NOTFOUND) {
    return 1;
  }
  return 0;
};    
IM_REL* winbuf::get_im_rel(){
  return b;
};

// advance head curosr by one

bool winbuf::hasExpired(){
  DBT k, d;
  return (getExpired(&k,&d)==0);
}

int winbuf::getExpired(DBT *data){
  DBT key;
  return getExpired(&key, data);
}

// get the oldest expired tuple
int winbuf::getExpired(DBT* key, DBT* data){
  DBUG_ENTER("getExpired");
  int rc = 0;
  unsigned int t;
  unsigned int id = getTupleID();
  if (id < size) {
    if ((rc = temp->c_get(temp, key, data, DB_FIRST)) !=0){
      //printf("Here\n");
      if (rc!=DB_NOTFOUND) 
	adlerror(rc, "get Expired tuple in winbuf::getExpired()");
      goto exit;
    }
    memcpy(&t, ((char*)data->data)+datasize, sizeof(tuple_id));
    //memcpy(&t, key->data, key->size);
    //printf("data %s t %d UM %d, size %d, id %d\n", data->data, t, UINT_MAX, size, id);
    if(t<=UINT_MAX-size+id && t>id) {
      //printf("Data %s\n", data->data); 
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
	if (rc!=DB_NOTFOUND) 
	  adlerror(rc, "get Expired tuple in winbuf::getExpired()");
	goto exit;
      }
      memcpy(&t, ((char*)data->data)+datasize, sizeof(tuple_id));
      //memcpy(&t, key->data, key->size);
      if ((t<=id-size || t>id)) {
	//printf("Data %s\n", data->data);
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
/*int winbuf::getTail(DBT* key, DBT* data){
  DBUG_ENTER("getTail");
  int rc;
  if ((rc = temp->c_get(temp, key, data, DB_LAST)) !=0){
    if (rc!=DB_NOTFOUND) 
      adlerror(rc, "get last tuple in winbuf::getTail()");
    return rc;
  }
  else{
  }
  DBUG_RETURN(rc);
  return rc;
  }*/

int winbuf::getHeadInt(){
  DBT key, data;
  DBUG_ENTER("getTail");
  int rc;
  if ((rc = temp->c_get(temp, &key, &data, DB_FIRST)) !=0){
    if (rc!=DB_NOTFOUND) 
      adlerror(rc, "get last tuple in winbuf::getTail()");
    return rc;
  }
  else{
  }
  int ret=0;
  memcpy(&ret, (char*)(data.data), sizeof(int));
  
  DBUG_RETURN(ret);
  return ret;
}

void winbuf::deleteGreaterInt(int testVal){
  timestamp ts;
  DBT key, data;
  int rc;
  bool first =true;
  int i = 0;
  int val;
  while ((rc = temp->c_get(temp, &key, &data, first?DB_FIRST:DB_NEXT)) == 0){
    first = false;
    memcpy(&val, (char*)(data.data), sizeof(int));
    if(val > testVal) {
      rc = temp->c_del(temp, 0);
    }
  }
}

void winbuf::deleteLesserInt(int testVal){
  timestamp ts;
  DBT key, data;
  int rc;
  bool first =true;
  int i = 0;
  int val;
  while ((rc = temp->c_get(temp, &key, &data, first?DB_FIRST:DB_NEXT)) == 0){
    first = false;
    memcpy(&val, (char*)(data.data), sizeof(int));
    if(val < testVal) {
      rc = temp->c_del(temp, 0);
    }
  }
}

double winbuf::getHeadReal(){
  DBT key, data;
  DBUG_ENTER("getTail");
  int rc;
  if ((rc = temp->c_get(temp, &key, &data, DB_FIRST)) !=0){
    if (rc!=DB_NOTFOUND) 
      adlerror(rc, "get last tuple in winbuf::getTail()");
    return rc;
  }
  else{
  }
  double ret=0;
  memcpy(&ret, (char*)(data.data), sizeof(double));
  
  DBUG_RETURN(ret);
  return ret;
}

void winbuf::deleteGreaterReal(double testVal){
  timestamp ts;
  DBT key, data;
  int rc;
  bool first =true;
  int i = 0;
  double val;
  while ((rc = temp->c_get(temp, &key, &data, first?DB_FIRST:DB_NEXT)) == 0){
    first = false;
    memcpy(&val, (char*)(data.data), sizeof(double));
    if(val > testVal) {
      rc = temp->c_del(temp, 0);
    }
  }
}

void winbuf::deleteLesserReal(double testVal){
  timestamp ts;
  DBT key, data;
  int rc;
  bool first =true;
  int i = 0;
  double val;
  while ((rc = temp->c_get(temp, &key, &data, first?DB_FIRST:DB_NEXT)) == 0){
    first = false;
    memcpy(&val, (char*)(data.data), sizeof(double));
    if(val < testVal) {
      rc = temp->c_del(temp, 0);
    }
  }
}



int winbuf::printTuple(DBT* key, DBT* data){
  //unsigned int* i = (unsigned int*)(key->data);
  unsigned int* i = (unsigned int*)(((char*)data->data)+datasize);
  cout<<*i<<endl;
}
int winbuf::print(){
  timestamp ts;
  DBT key, data;
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
    adlerror(rc, "IM_RELC->c_get() in winbuf::print()");
  printf("tuples in window %d ------------\n", i);
  return rc;
}

void winbuf::resetTS(timestamp *ts){
  first_ts = *ts;
};

void winbuf::getTimestamp(timestamp *atime) {
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

unsigned int winbuf::getTupleID(){
  return tuple_id;
}

int winbuf::updateTupleID(timestamp *ts){
  if (ts){ // time-base window
    if(ts->tv_sec == 0) {
      gettimeofday(ts, NULL);
    }
    if (first){
      first = false;
      resetTS(ts);
    }
    tuple_id = (unsigned int)(timeval_subtract(*ts, first_ts) * 1000);
    /*printf("TupleId = %d; %d,%d; %d,%d; %f\n", tuple_id, 
       ts->tv_sec, ts->tv_usec, first_ts.tv_sec, first_ts.tv_usec,
       timeval_subtract(*ts, first_ts));
    */
  }
  else{ // count-based window
    //print();
    //fflush(stdout);
    tuple_id++;
  }

  // delete expired tuples  
  //while (hasExpired()){
  //pop();
  //}

}

