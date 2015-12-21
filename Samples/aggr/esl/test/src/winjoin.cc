#include <iostream>
//#include "querySchdl.h"
//#include "ios.h"
#include <buffer.h>
#include <winbuf.h>
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

int main(){

  struct timeval tv;
  tv.tv_sec = 100;
  tv.tv_usec = 200;
  bm=bufferMngr::getInstance();
  winbuf wb("row", 1);
  cDBT cdbt("12", 3, &tv);
  wb.put(&cdbt);
  tv.tv_sec = 300;
  tv.tv_usec = 400;
  cDBT cdbt2("34", 3, &tv);
  wb.put(&cdbt2);
  cDBT r(MAX_STR_LEN, &tv);
  wb.get(&r);
  assert(!strcmp(r.data, "12") && r.atime->tv_sec==100 && r.atime->tv_usec==200);
  wb.getHead(&r);
  assert(!strcmp(r.data, "34") && r.atime->tv_sec==300 && r.atime->tv_usec==400);
  wb.getTail(&r);
  assert(!strcmp(r.data, "34") && r.atime->tv_sec==300 && r.atime->tv_usec==400);
  wb.getExpired(&r);
  assert(!strcmp(r.data, "12") && r.atime->tv_sec==100 && r.atime->tv_usec==200);
  tv.tv_sec = 500;
  tv.tv_usec = 600;
  cDBT cdbt3("56", 3, &tv);
  wb.put(&cdbt3);
  wb.get(&r);
  assert(!strcmp(r.data, "34") && r.atime->tv_sec==300 && r.atime->tv_usec==400);
  wb.getHead(&r);
  assert(!strcmp(r.data, "56") && r.atime->tv_sec==500 && r.atime->tv_usec==600);
  wb.getTail(&r);
  assert(!strcmp(r.data, "56") && r.atime->tv_sec==500 && r.atime->tv_usec==600);
  wb.getExpired(&r);
  assert(!strcmp(r.data, "34") && r.atime->tv_sec==300 && r.atime->tv_usec==400);

  winbuf tb("time", 1, _ADL_WIN_TIME);
  
  return 0;

};
