#include <winbuf.h>
#include <stdio.h>
#include <db.h>
#include <error.h>
#include <adllib.h>
#include <iostream>

extern "C"{
#include <dbug.h>
}


using namespace std;
winbuf::    winbuf(int size_in,
		   _adl_win_type wtype_in):buffer("winbuf", WINBUF){
  mode = win_memory;
  wtype = wtype_in;
  size = size_in;
  n = 0;
  int rc;
  if ((rc = im_rel_create(&b, NULL, IM_LINKEDLIST, 0)) != 0) {
    adlerror(rc, "im_rel_create() in winbuf()");
  }
  if ((rc = b->open(b, "_adl_db_t", 0)) != 0) {
    adlerror(rc, "open() in winbuf()");
  }
  if ((rc = b->cursor(b, &head, 0)) != 0) {
    adlerror(rc, "IM_REL->cursor() in winbuf()");
  }
  if ((rc = b->cursor(b, &temp, 0)) !=0){
    adlerror(rc, "IM_REL->cursor in winbuf()");
  }

};
winbuf::    ~winbuf(){
  int rc;
  if ((rc = head->c_close(head)) != 0) {
    adlerror(rc, "IM_RELC->c_close() in ~winbuf()");
  }
  if ((rc = temp->c_close(temp)) !=0){
    adlerror(rc, "IM_RELC->c_close() in ~winbuf()");
  }
  if ((rc = b->close(b, 0)) != 0) {
    adlerror(rc, "IM_REL->close()");
  }
};
int winbuf::get(dbt* d){
  int rc=0;
  if (getExpired(d) !=0)
    rc = getHead(d);
  return rc;
};
int winbuf::get(DBT*, timestamp*, DBT*){
};
int winbuf::put(dbt* d){
  DBUG_ENTER("winbuf::put");
  // delete expired tuples  
  timestamp ts;
  cDBT cdbt(MAX_STR_LEN, &ts, MAX_STR_LEN);
  while (getExpired(&cdbt) == 0){
    pop();
  }

  // put the tuple
  int rc;
  DBT key, data;
  key.data = d->key;
  key.size = d->keysz;
  // append timestamp to data structure, even if timestamp is NULL
  char datadata[d->datasz + TS_SZ+1];
  memcpy(datadata, d->data, d->datasz);
  memcpy(datadata + d->datasz, d->atime, TS_SZ);
  data.data = datadata;
  data.size = d->datasz+TS_SZ;
  if ((rc = b->put(b, &key, &data,DB_APPEND)) !=0){
    adlerror(rc, "IM_REL->put() in winbuf::put()");
  }
  n++;
  // move the head pointer
  cDBT t(MAX_STR_LEN, &ts, MAX_STR_LEN);
  cDBT h(MAX_STR_LEN, &ts, MAX_STR_LEN);    

  switch (wtype){
  case _ADL_WIN_ROW:
    rc = advanceHead();
    if (rc){
      adlerror(rc, "Can't advance head in winbuf::put()");
    };
    break;
  case _ADL_WIN_TIME:
    getTail(&t);
    do{
      getHead(&h);
    }while (t.atime->tv_sec - h.atime->tv_sec > size && advanceHead() == 0);
    break;
  default:
    EM_error(ERR_BUFFER, __LINE__, __FILE__, "Unknown window type");
  }
  DBUG_RETURN(rc);
};
int winbuf::put(DBT*, timestamp*, DBT*){
  EM_error(ERR_TO_BE_IMPLEMENTED, __LINE__, __FILE__, "winbuf::put(DBT*..)");
  return -1;
};
int winbuf::pop(){
  DBUG_ENTER("winbuf::pop()");
  DBT key, data;
  int rc = temp->c_get(temp, &key, &data, DB_FIRST);
  if (rc ==0) 
    rc = temp->c_del(temp, 0);
  else if (rc !=DB_NOTFOUND)
    adlerror(rc, "IMREL->c_get() in winbuf::pop()");
  if (rc == 0) 
    n--;
  else if (rc !=DB_NOTFOUND)
    adlerror(rc, "IMREL->c_del() in winbuf::pop()");  
  DBUG_RETURN(rc);
  return rc;
};
int winbuf::empty(){
  return n==0;

};    
IM_REL* winbuf::get_im_rel(){
  return b;
};
IM_RELC* winbuf::getWinCursor(){
  return head;
}
// advance head curosr by one
int winbuf::advanceHead(){
  DBUG_ENTER("winbuf::advanceHead");
  DBT key, data;
  int rc = head->c_get(head, &key, &data, DB_NEXT);
  if (rc){
    rc = head->c_get(head, &key, &data, DB_LAST);
  }
  DBUG_RETURN(rc);
  return rc;
};

bool winbuf::hasExpired(){
  timestamp ts;
  cDBT d(MAX_STR_LEN, &ts, MAX_STR_LEN);
  return (getExpired(&d)==0);
}

// get the oldest expired tuple
int winbuf::getExpired(dbt* d){
  DBUG_ENTER("getExpired");
  DBT key, data;
  timestamp ts;
  int rc = 0;
  if ((rc = temp->c_get(temp, &key, &data, DB_FIRST)) !=0){
    if (rc!=DB_NOTFOUND) 
      adlerror(rc, "get Expired tuple in winbuf::getExpired()");
    goto exit;
  }else{
    if (temp->sameAs(temp, head)){
      rc = DB_NOTFOUND;
      goto exit;
    }
    else{
      d->copy(&data, &ts, &key);
      d->recoverTS();
    }
  }
 exit:
  DBUG_RETURN(rc);
  return rc;
}

// get the youngest tuple
int winbuf::getTail(dbt* d){
  DBUG_ENTER("getTail");
  DBT key, data;
  timestamp ts;
  int rc = 0;
  if ((rc = temp->c_get(temp, &key, &data, DB_LAST)) !=0){
    if (rc!=DB_NOTFOUND) 
      adlerror(rc, "get last tuple in winbuf::getTail()");
    return rc;
  }
  else{
    d->copy(&data, &ts, &key);
    d->recoverTS();
  }
  DBUG_RETURN(rc);
  return rc;
}
int winbuf::getHead(dbt* d){
  DBUG_ENTER("getHead");
  DBT key, data;
  timestamp ts;
  int rc = 0;
  if ((rc=head->c_get(head, &key, &data, DB_CURRENT)) ==0){
    d->copy(&data, &ts, &key);
    d->recoverTS();
  }
  else if (rc!=DB_NOTFOUND) 
    adlerror(rc, "get the oldest tuple in winbuf::getHead()");      
  DBUG_RETURN(rc);
  return rc;
}

int winbuf::print(){
  timestamp ts;
  DBT key, data;
  cDBT cdbt(MAX_STR_LEN, &ts, MAX_STR_LEN);
  cDBT t(MAX_STR_LEN, &ts, MAX_STR_LEN);
  printf("------ %s window %s size=%d #=%d ------\n", wtype==_ADL_WIN_ROW? "Physical" :"Logical", name, size, n);
  printf("Expired:\n");
  int rc = 0;
  if (hasExpired())
  while (rc ==0){
    if ((rc = temp->c_get(temp, &key, &data, DB_FIRST)) ==0){
      cdbt.copy(&data, &ts, &key);
      cdbt.recoverTS();
    }
    if (rc && rc !=DB_NOTFOUND){
      adlerror(rc, "IM_REL->c_get() in winbuf::print()");
      break;
    }
    if (wtype == _ADL_WIN_ROW){
      if (!rc) cdbt.print();
      break;
    }
    // logical window
    getTail(&t);
    if (t.atime->tv_sec - cdbt.atime->tv_sec <= size)
      break;
    if (!rc) 
      cdbt.print();
  }
  printf("Window:\n");
  memcpy(temp,head, sizeof(*head));
  bool first =true;
  while ((rc = temp->c_get(temp, &key, &data, first?DB_CURRENT:DB_NEXT)) == 0){
    first = false;
    cdbt.copy(&data, &ts, &key);
    cdbt.recoverTS();
    cdbt.print();
  }
  if (rc && rc != DB_NOTFOUND)
    adlerror(rc, "IM_RELC->c_get() in winbuf::print()");
  printf("------------\n");
  return rc;
}
