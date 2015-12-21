//#include "dbt.h"
#include <stdio.h>
#include "const.h"
#include "stdlib.h"
#include "string.h"  
#include "buffer.h"
#include "err.h"
#include <deque>
#include <iostream>
#include <db.h>
#include <adllib.h>

extern "C" {
#include <mm.h>
#include "dbug.h"
}
#include <ios/ios.h>
#include <string>
#include "windowBuf.h"
using namespace ESL;
using namespace std;

bool buffer::isTable(){
  return type == BUF_BTREE || type == BUF_RECNO;
}

int buffer::put(int code, const char *s1, const char *s2){
  int size = sizeof(int);
  if (s1) size+= strlen(s1)+1;
  if (s2) size+= strlen(s2)+1;
  cDBT cdbt(size);
  memcpy(cdbt.data, &code, sizeof(int));
  if (s1)
    strcpy((char*)cdbt.data+sizeof(int), s1);
  if (s2)
    strcpy(cdbt.data+sizeof(int)+strlen(s1)+1, s2);
  return put(&cdbt);
}

int buffer::get(int &code, char *s1, char *s2){
  cDBT cdbt(MAX_STR_LEN);
  int rc = get(&cdbt);
  if (rc == 0){
    memcpy(&code, cdbt.data, sizeof(int));
    if (s1 && cdbt.datasz>sizeof(int))
      strcpy(s1, cdbt.data+sizeof(int));
    if (s2 && cdbt.datasz > sizeof(int)+strlen(s1)+1){
      strcpy(s2, cdbt.data+sizeof(int)+strlen(s1)+1);
    }
  }
  return rc;
  
}

// add client to the monitor list
int buffer::addClient(GUIClient* c){
  clients.push_back(c);
};

// drop client from the monitor list
int buffer::dropClient(GUIClient *c){
  for (int i= 0; i <clients.size(); i++){
    if (strcmp(clients[i]->getClientIp(), c->getClientIp()) == 0){
      clients.erase(clients.begin()+i);
      return 0;
    }
  }
  return -1;
};

// get the monitor clients list
int buffer::getClients(vector<GUIClient*> &v){
  v = clients;
};

buffer::buffer(const char* name_in,
	       buf_t shared_in){
  strcpy(name, name_in);
  type = shared_in;
  mant = NULL;
  timestamp_type = TS_INTERNAL;
}

// get a tuple from the head of the buffer
/*
int buffer::get(DBT* data, timestamp *atime, DBT* key){
  int rc = get(data, atime, );
  return rc;
};
*/
/*
int buffer::put(DBT* data, timestamp *atime,DBT* key){
  DBUG_ENTER("buffer::put");
  cDBT *cdbt = new cDBT(data, atime, key);
  int rc = put(cdbt);
  delete cdbt;
  DBUG_RETURN(rc);
};
*/

buffer::~buffer(){
  if (mant != NULL) delete mant;
};

int buffer::setMant(const timestamp *mtime){
  if (mtime != NULL) {
    if (this->mant == NULL) this->mant = new struct timeval;
    memcpy(this->mant, mtime, TS_SZ);
  }
  
  return 0;
};

int buffer::getMant(timestamp *mtime){
  DBUG_ENTER("buffer::getMant");

  if (mtime != NULL && this->mant != NULL) {
    memcpy(mtime, this->mant, TS_SZ);
    return 0;
  } else if (this->mant == NULL) {
    return -1;    
  }
};


// put a tuple to the tail of the buffer
int commonBuf::put(dbt* d){
  DBUG_ENTER("commonBuf::put");
  cDBT *cdbt = new cDBT(d);
  b.push_back(cdbt);
  DBUG_RETURN(0);
  return 0;
};
// put a tuple (DBTs) to the tail of the buffer

int commonBuf::put(DBT *d, timestamp *atime, DBT *k){
  DBUG_ENTER("commonBuf::put(DBT*, timestamp*, DBT*)");
  cDBT *cdbt = new cDBT(d, atime, k);
  b.push_back(cdbt);
  DBUG_RETURN(0);
  return 0;
};



// get a tuple from the head of the buffer
int commonBuf::get(dbt* d){
  DBUG_ENTER("commonBuf::get");
  cDBT *cdbt = b.front();
  if (cdbt && !b.empty()){
    d->copy(cdbt);
    DBUG_RETURN(0);
  }
  else
    DBUG_RETURN(DB_NOTFOUND);
};

//get timestamp of the tuple on the head of the buffer
int commonBuf::get(timestamp *atime) {
  DBUG_ENTER("commonBuf::get(timestamp*)");
  
  DBT data;
  DBT key;

  char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  data.data = datadata;
  key.data = keydata;

  cDBT *cdbt = b.front();
  if (cdbt && !b.empty()){
    cdbt->get(&data, atime, &key);
    DBUG_RETURN(0);
  }
  else
    DBUG_RETURN(DB_NOTFOUND);
}


// get a tuple from the head of the buffer to DBT
int commonBuf::get(DBT *data, timestamp *atime, DBT *key){
  DBUG_ENTER("commonBuf::get(DBT*, timestamp*, DBT*)");
  //printf("bfront\n");
  //fflush(stdout);
  cDBT *cdbt = b.front();
  //printf("bfront %d\n", cdbt);
  //fflush(stdout);
  if (cdbt && !b.empty()){
    cdbt->get(data, atime, key);
    //printf("returning\n");
    //fflush(stdout);
    DBUG_RETURN(0);
  }
  else {
    //printf("returning\n");
    //fflush(stdout);
    DBUG_RETURN(DB_NOTFOUND);
  }
};



// pop a tuple from the head of the buffer, and free the memory
int commonBuf::pop(){
  DBUG_ENTER("commonBuf::pop");
  cDBT* d=b.front();
  b.pop_front();
  delete d;
  DBUG_RETURN(0);
};


// if the buffer empty?
int commonBuf::empty(){
  return b.empty();
};

int commonBuf::bufSize(){
  return b.size();
};

int commonBuf::bufByteSize(){
  if (b.size() == 0) return 0;

  int dbtSize = b.front()->keysz+b.front()->datasz;
  return b.size()*dbtSize;
};

sharedBuf::sharedBuf(const char* name_in
		     ):buffer(name_in, SHARED){
  head = tail = NULL;
  n = 0;
};
/*

MM* sharedBuf::createSM(){

  //sDBT::dbt_mm = mm_create(DBT_SM_SIZE, NULL);
  //  if (!sDBT::dbt_mm){
  //perror("Create shared memory for DBTs:");
  //return NULL;
  //}
  cout<<"DBT shared memory created!\n";
  return sDBT::dbt_mm;    
};
void sharedBuf::destroySM(){
  //mm_destroy(sDBT::dbt_mm);
  cout<<"DBT shared memory destroyed!\n";
};
*/
sharedBuf::~sharedBuf(){
  
};

int sharedBuf::put(DBT* d, timestamp *atime, DBT *k){
  DBUG_ENTER("sharedBuf::put");
  sDBT *sdbt1 = (sDBT*)mm_malloc(sDBT::dbt_mm, sizeof(sDBT));
  if (!sdbt1){
    perror("Malloc shared memory in sharedBuf->put()");
    mm_unlock(sDBT::dbt_mm);
    return ERR_BUFFER;
  }
  sDBT* sdbt = new(sdbt1) sDBT(d, atime, k);

  //sdbt->init(d, atime, k);
  mm_lock(sDBT::dbt_mm, MM_LOCK_RW);
  if (n==0){
    head = tail = sdbt;
  }
  else{
    tail->next = sdbt;
    tail = tail->next;
  }
  n++;
  mm_unlock(sDBT::dbt_mm);
  DBUG_RETURN(0);
  return 0;
}

// put a tuple to the buffer
int sharedBuf::put(dbt* d){
  DBUG_ENTER("sharedBuf::put");
  sDBT *sdbt1 = (sDBT*)mm_malloc(sDBT::dbt_mm, sizeof(sDBT));
  if (!sdbt1){
    perror("Malloc shared memory in sharedBuf->put()");
    mm_unlock(sDBT::dbt_mm);
    return ERR_BUFFER;
  }
  sDBT* sdbt = new(sdbt1) sDBT(d);
  //sdbt->init(d);
  mm_lock(sDBT::dbt_mm, MM_LOCK_RW);
  if (n==0){
    head = tail = sdbt;
  }
  else{
    tail->next = sdbt;
    tail = tail->next;
  }
  n++;
  mm_unlock(sDBT::dbt_mm);
  DBUG_RETURN(0);
  return 0;
};

void sharedBuf::printAll(){
  sDBT *sdbt = head;
  while (sdbt){
    sdbt->print();
    cout<<"->";
    sdbt = sdbt->next;
  }
  cout<<endl;
}

// get a tuple from the buffer
int sharedBuf::get(dbt* d){
  DBUG_ENTER("sharedBuf::get");
  if (empty()){
    return DB_NOTFOUND;
  }
  d->copy(head);
  DBUG_RETURN(0);
  return 0;
};

//get timestamp of the tuple on the head of the buffer
int sharedBuf::get(timestamp *atime) {
  DBUG_ENTER("commonBuf::get(timestamp*)");
  
  DBT data;
  DBT key;

  char keydata[MAX_STR_LEN], datadata[MAX_STR_LEN];

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  data.data = datadata;
  key.data = keydata;

  if (empty())
    DBUG_RETURN(DB_NOTFOUND);

  head->get(&data, atime, &key);
  DBUG_RETURN(0);
  return 0;
}

// get a tuple from the buffer to DBT
int sharedBuf::get(DBT *d, timestamp *atime, DBT *k){
  DBUG_ENTER("sharedBuf::get(DBT*, timestamp*, DBT*)");
  if (empty()){
    return DB_NOTFOUND;
  }
  head->get(d, atime, k);
  DBUG_RETURN(0);
  return 0;
};

// pop a tuple from the buffer, and free the memory
int sharedBuf::pop(){
  DBUG_ENTER("sharedBuf->pop()");
  if (empty()){
    //EM_error(0, ERR_BUFFER, __LINE__, __FILE__, "Pop from empty buffer");
    DBUG_RETURN(ERR_BUFFER);

}  else{
  mm_lock(sDBT::dbt_mm, MM_LOCK_RW);
  sDBT* p=head;
  head = head->next;
  p->destroy();
  mm_free(sDBT::dbt_mm, p);
  n--;
  mm_unlock(sDBT::dbt_mm);
  DBUG_RETURN(0);
  }
};
  


int sharedBuf::empty(){
  return n==0||head==NULL;

}
int sharedBuf::full(){
  return 0;
};

int sharedBuf::bufSize(){
  return n;
}

int sharedBuf::bufByteSize(){
  if (n == 0) return 0;

  int dbtSize = head->keysz+head->datasz;
  return n*dbtSize;
};

bufferMngr* bufferMngr::_instance = 0;
bufferMngr* bufferMngr::getInstance(){
  if (!_instance){
    _instance= (bufferMngr*)mm_malloc(sDBT::dbt_mm, sizeof(bufferMngr));
    if (!_instance){
      cout<<mm_error()<<endl;
      return NULL;
    }
    else
      _instance->init();
  }
  return _instance;
  
}

int bufferMngr::destory(){
  if (_instance) {
    mm_free(sDBT::dbt_mm, _instance);
    _instance = NULL;
  }
}

int bufferMngr::init(){
  n = 0;
  new_arrival = 1; //default to true
  /*
  bStdout = (stdoutBuf*)mm_malloc(sDBT::dbt_mm, sizeof(stdoutBuf));
  if (!bStdout){
    perror("malloc shared memory for stdout buffer");
    return -1;
  }
  stdoutBuf temp("stdout");
  memcpy((void*)bStdout, &temp, sizeof(stdoutBuf));
  */
}

bufferMngr::bufferMngr(){
  init();
};

// create a buffer
int bufferMngr::create(const char *name,        // buffer name
		       buf_t shared,     // buffer in the shared memory?  yes for i/o buffers
		       unsigned int size_in,
		       _adl_win_type wtype_in,
		       const char* dbname)
{
  // check if the buf exists
  if (lookup(name)){
    return -1;
  }


  buffer *b;
  sharedBuf sb(name);

  switch (shared){
  case SHARED:
    b = (sharedBuf*)mm_malloc(sDBT::dbt_mm, sizeof(sharedBuf));
    if (!b){
      perror("malloc shared memory when creating buffer");
      return -1;
    }
    memcpy(b, (void*)&sb, sizeof(sharedBuf));
    break;
  case NOT_SHARED:
    b=new commonBuf(name);
    break;
  case BUF_BTREE:
  case BUF_RECNO:
    b = new table(name, shared, dbname);
    break;
  case WINBUF:
    b = new windowBuf(name, size_in, wtype_in);
    b->type = WINBUF;
    break;
  default:
    EM_error(ERR_BUFFER, __LINE__, __FILE__, "unknown buf type in bufferMngr::create()");
  }
  mm_lock(sDBT::dbt_mm, MM_LOCK_RW);
  dict[n].b = b;
  strcpy(dict[n].name, name);

  n++;
  qsort((void*)dict, n, sizeof(bufMap),
	bufMap::cmp);
  mm_unlock(sDBT::dbt_mm);

  return 0;
}

// delete a buffer
int bufferMngr::kill(const char* name){
  mm_lock(sDBT::dbt_mm, MM_LOCK_RW);
  int r = lookupBufMapIdx(name);
  if (r == -1) return -1;
  bufMap m = dict[r];
  buffer *b = m.b;
  switch (b->type){
  case SHARED:
    mm_free(sDBT::dbt_mm, b);
    break;
  case NOT_SHARED:
  case BUF_BTREE:
  case BUF_RECNO:
  case WINBUF:
    delete b;
    break;
  default:
    EM_error(ERR_BUFFER, __LINE__, __FILE__, "unknown buf type in bufferMngr::kill()");    
  }

  for (int i = r; i<n-1; i ++){
    dict[i] = dict[i+1];
  }

  n--;
  mm_unlock(sDBT::dbt_mm);
  return 0;
}
;

// put a tuple to the tail of the specified buffer
int bufferMngr::put(const char* name,
		    dbt *d){
  buffer *b = lookup(name);
  return b? b->put(d): -1;
}
;


// get a tuple from the head of the specified buffer
int bufferMngr::get(const char* name,
		    dbt *d){
  buffer *b = lookup(name);
  return b? b->get(d): -1;
}
;

// get a timestamp of the tuple on the head of the specified buffer
int bufferMngr::get(const char* name,
		    timestamp *atime){
  buffer *b = lookup(name);
  return b? b->get(atime): -1;
}
;

// get the DB pointer when the buffer is a btree
int bufferMngr::get(const char* name,
	DB *&db){
  buffer *b = lookup(name);  
  if (!b || !(b->isTable())) {
    EM_error(ERR_BUFFER, __LINE__, __FILE__, name, " is not a table");
    return -1;
  }
  table *t = (table*)(b);
  db = t->db;
  return 0;
}


// pop a tuple from the head of the buffer
int bufferMngr::pop(const char *name){
  buffer *b = lookup(name);
  return b? b->pop() : -1;
}
;


// if the buffer empty?
int bufferMngr::empty(const char* name){
  buffer *b = lookup(name);
  return b? b->empty():1;
}
;

// given the buffer name, return the actual buffer address pointer
// or NULL if failed;
buffer* bufferMngr::lookup(const char* name){
  bufMap *bmap = lookupBufMap(name);
  return bmap? bmap->b : NULL;
};


void bufferMngr::displayBufNames(){
  cout<<n<<" buffers in buffer manager:"<<endl;
  for (int i = 0; i < n; i++){
    cout<<"("<<dict[i].name<<endl;
  }
}

// Display all buffers

void bufferMngr::displayBufs(){
  cout<<n<<" buffers in buffer manager:"<<endl;
  for (int i = 0; i < n; i++){
    cout<<"("<<dict[i].name<<","<<dict[i].b->name<<","<<dict[i].b->type<<","<<dict[i].b<<")"<<endl;
  }
}
// given the name, return the bufMap pointer, used in kill()
bufMap* bufferMngr::lookupBufMap(const char* name){
  bufMap b;
  strcpy(b.name, name);
  return ((bufMap*)bsearch(&b, dict, n,
			   sizeof(bufMap), 
			   bufMap::cmp));
};
// given the name, return the index in dict, used in kill()
int bufferMngr::lookupBufMapIdx(const char* name){
  bufMap b;
  strcpy(b.name, name);
  bufMap *m= ((bufMap*)bsearch(&b, dict, n,
			       sizeof(bufMap), 
			       bufMap::cmp));
  return m? (m-dict):-1;
};

// get the stdout buffer
/*
stdoutBuf *bufferMngr::getStdout(){
  return bStdout;
};
*/
bufferMngr::~bufferMngr(){
  //mm_free(sDBT::dbt_mm, bStdout);
}
/*
stdoutBuf::stdoutBuf(const char *name):buffer(name, BUF_STDOUT){
  
}
stdoutBuf::~stdoutBuf(){};
int stdoutBuf::get(dbt* d){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int stdoutBuf::get(DBT* d, timestamp *atime, DBT *k){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int stdoutBuf::put(dbt* d){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int stdoutBuf::put(DBT* d, timestamp *atime, DBT *k){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int stdoutBuf::pop(){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int stdoutBuf::empty(){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
*/
table::table(const char *name, buf_t type, const char*dbname):buffer(name, type){
  int rc;
  char dbFile[256];
  if(dbname == NULL) {
    sprintf(dbFile, "__%s", name);
  }
  else {
    sprintf(dbFile, "%s", dbname);
  }
   if ((rc = db_create(&db, NULL, 0)) != 0) {
      adlerror(rc, "db_create()");
   }
   if ((rc = db->set_pagesize(db, 2048)) != 0) {
      adlerror(rc, "set_pagesize()");
   }
   if ((rc = db->open(db, dbFile, NULL, type == BUF_BTREE? DB_BTREE:DB_RECNO, DB_CREATE, 0664)) != 0) {
      adlerror(rc, "open()");
   }
  
}

table::~table(){
  int rc;
   if ((rc = db->close(db, 0)) != 0) {
      adlerror(rc, "DB->close()");
   }

};
int table::get(dbt* d){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int table::get(timestamp* d){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int table::get(DBT* d, timestamp *atime, DBT *k){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int table::put(dbt* d){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int table::put(DBT* d, timestamp *atime, DBT *k){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int table::pop(){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int table::empty(){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int table::bufSize(){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
int table::bufByteSize(){
  EM_error(0, ERR_STREAM, __LINE__, __FILE__, "function not implemented");
  return -1;
}
