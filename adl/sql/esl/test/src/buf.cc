#include <cassert>
#include <iostream>
//#include "querySchdl.h"
//#include "ios.h"
#include <buffer.h>
extern "C"{
#include <mm.h>
#include <dbug.h>
#include <sql/const.h>
}
using namespace ESL;
using namespace std;

// ESL Main program

bufferMngr *bm;
static const char *default_dbug_option="d,commonBuf:t:i";
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <db.h>

#define DATABASE "access.db"

int _temp_cmp(DB* dbp, const DBT *a, const DBT *b){
        int ai, bi, ri, rs;
        double ad, bd, rd;
        struct timeval *at, *bt;return 0;
};

int testBDB(DB* dbp) {
  DBT key, data; 
  int ret, t_ret;

  char datadata[60];
  char keydata[60];
  int field_1;
  int field_2;
  // sprintf(fruit, "%s", "fruit");
  // sprintf(apple, "%s", "apple");
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  data.data = datadata;
  key.data = keydata;
  memcpy((char*)data.data+0, &(field_1), sizeof(int));
  // memcpy((char*)key.data+0, &(field_2), sizeof(int));
  data.size = 4;
  key.size = 0;

  if ((ret = db_create(&dbp, NULL, 0)) != 0) {
    dbp->err(dbp, ret, "db_create()"); goto err;
  }
  if ((ret = dbp->set_pagesize(dbp, 2048)) != 0) {
    dbp->err(dbp, ret, "dbp->set_pagesize()"); goto err;
  }
  if ((ret = dbp->set_flags(dbp, DB_DUP)) != 0) {
    dbp->err(dbp, ret, "dbp->set_flags()"); goto err;
  }
  if ((ret = dbp->set_bt_compare(dbp, _temp_cmp)) != 0){
    dbp->err(dbp, ret, "dbp->set_bt_compare()"); goto err;
  }
  if ((ret = dbp->open(dbp, "_adl_db_temp", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
    dbp->err(dbp, ret, "dbp->open()"); goto err;
  }

  if ((ret = dbp->put(dbp, NULL, &key, &data, 0))!=0) {
    dbp->err(dbp, ret, "dbp->put()"); goto err;
  }

  if ((ret = dbp->put(dbp, NULL, &key, &data, DB_CURRENT)) == 0)
    printf("db: %s: key stored.\n", (char *)key.data); else { printf("Put error %d \n", ret); dbp->err(dbp, ret, "DB->put"); goto err; }

  if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) == 0) 
    printf("db: %s: key retrieved: data was %s.\n", (char *)key.data, (char *)data.data); else { dbp->err(dbp, ret, "DB->get"); goto err; }

  if ((ret = dbp->del(dbp, NULL, &key, 0)) == 0) 
    printf("db: %s: key was deleted.\n", (char *)key.data); else { dbp->err(dbp, ret, "DB->del"); goto err; }

 err: 
  if ((t_ret = dbp->close(dbp, 0)) != 0 && ret == 0) ret = t_ret;

  return ret; 
}

int main(){
  //cout<<"ESL started!\n";
  //DBUG_PUSH(default_dbug_option);
  sDBT::createSM();
  //  sharedBuf::createSM();
  struct timeval tv;
  tv.tv_sec = 100;
  tv.tv_usec = 200;
  /*
    bm= (bufferMngr*)mm_malloc(sDBT::dbt_mm, sizeof(bufferMngr));
    if (!bm){
    perror("Malloc buffer manager in shared memory");
    return 1;
    }
    bufferMngr bmTemp;
    memcpy(bm, (void*)&bmTemp, sizeof(bufferMngr));
  */
  bm=bufferMngr::getInstance();
  bm->create("1", SHARED);
  bm->create("_ioBuffer", SHARED);
  bm->create("_queryBuffer", SHARED);
  bm->create("stdout", SHARED);

  bm->create("table", BUF_BTREE);
  DB *db;
  bm->get("table", db);
  // assert(testBDB(db)==0);
  bm->create("buf0",
	     SHARED);
  bm->create("buf1",
	     SHARED);
  bm->create("abc");
  bm->kill("abc");
  //  Ios ios();
  //drvMngr dm();
  cDBT r(MAX_STR_LEN, &tv);

  if (fork()){ // Query Schduler
    bm->create("abc");
    bm->kill("abc");
    bm->create("abc");
    bm->kill("abc");

    //qs.run();
    //cout<<"Query Scheduler"<<endl;
    buffer *b = bm->lookup("buf1");
    cDBT *d= new cDBT("12",3,&tv);
    tv.tv_sec = 100;
    tv.tv_usec = 200;
    b->put(d);

    tv.tv_sec = 300;
    tv.tv_usec = 400;
    cDBT *d2 = new cDBT("34", 3, &tv);
    b->put(d2);


    b->get(&r);
    assert(!strcmp(r.data, "12") && r.atime->tv_sec==100 && r.atime->tv_usec==200);
    b->pop();

    strcpy(d2->data, "56");
    b->put(d2);

    b->get(&r);
    assert(!strcmp(r.data, "34") && r.atime->tv_sec==300 && r.atime->tv_usec==400);
    delete d;
    delete d2;

    bm->create("buf2");
    cDBT *cdbt = new cDBT("cDBT",5);
    b = bm->lookup("buf2");
    b->put(cdbt);

    strcpy(cdbt->data, "7890");
    b->put(cdbt);

    strcpy(cdbt->data, "3456");    
    b->get(&r);

    assert(!strcmp(r.data, "cDBT"));
    b->pop();

    b->get(&r);
    assert(!strcmp(r.data, "7890"));

    delete cdbt;
    bm->kill("2");

    b = bm->lookup("buf0");
    b->put(1, "filename");
    b->put(2, "queryName", "xyz");
    sleep(3);
  }
  else { // I/O Scheduler
    sleep(2);
    bm=bufferMngr::getInstance();
    //cout<<"I/O Scheduler"<<endl;
    buffer *b = bm->lookup("buf1");
    cDBT *d= new cDBT("ABC",MAX_STR_LEN, &tv);
    b->get(d);
    assert(!strcmp(d->data, "34") && d->atime->tv_sec==300 && d->atime->tv_usec==400);    
    
    b->pop();
    b->get(d);

    assert(!strcmp(d->data, "56") && d->atime->tv_sec==300 && d->atime->tv_usec==400);    

    //    ios.run();
    delete d;
    b = bm->lookup("buf0");
    int i;
    char s[1024];
    char s2[1024];
    b->get(i, s);
    assert(i==1 && !strcmp(s, "filename"));
    b->pop();
    b->get(i, s, s2);
    assert(i==2 && !strcmp(s, "queryName") && !strcmp(s2, "xyz"));

    b->get(i, s);
    assert(i==2 && !strcmp(s, "queryName"));

    bm->kill("buf1");

    bm->create("ios");
    cDBT *cdbt = new cDBT("ios1",5);
    b = bm->lookup("ios");
    b->put(cdbt);

    strcpy(cdbt->data, "ios2");
    b->put(cdbt);

    strcpy(cdbt->data, "ios3");    
    b->get(&r);

    assert(!strcmp(r.data, "ios1"));
    b->pop();

    b->get(&r);
    assert(!strcmp(r.data, "ios2"));

    delete cdbt;
    bm->kill("ios");

    mm_free(sDBT::dbt_mm, bm);
    sDBT::destroySM();
    //    sharedBuf::destroySM();
    
  }
  return 0;

};
