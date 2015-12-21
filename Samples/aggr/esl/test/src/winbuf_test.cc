 // Test case for winbuf.cc -- aggregates over windows


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


static const char *default_dbug_option="d,commonBuf:t:i";
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <db.h>

#define DATABASE "access.db"

int main(){


  winbuf wb(1);
  char keydata[1024], datadata[1024];
  DBT cdbt;
  cdbt.data = datadata;
  memcpy(cdbt.data, "12", 3);
  cdbt.size = 3;
  DBT key;
  key.data = keydata;
  int id = 0;
  memcpy(key.data, &id, sizeof(id));
  unsigned int*i = (unsigned int*)(key.data);
  key.size = sizeof(id);
  wb.updateTupleID();
  wb.put(&cdbt);

  memcpy(cdbt.data, "34", 3);
  (*i)++;
  assert(*i ==1);  
  wb.updateTupleID();
  wb.put(&cdbt);

  DBT rk, rd;
  wb.getExpired(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "12") && *i ==1);
  wb.pop();
  /*wb.getHead(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "34") && *i ==2);
  */



  /*wb.getTail(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "34") && *i ==2);
  */

  // put ("56", 2);
  memcpy(cdbt.data, "56", 3);
  i = (unsigned int*)(key.data);
  (*i)++;
  wb.updateTupleID();
  wb.put(&cdbt);

  wb.getExpired(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "34") && *i ==2);
  wb.pop();
 /* wb.getHead(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "56") && *i ==3);
  */

  /*
  wb.getTail(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "56") && *i ==3);
  */


  winbuf tb(1, _ADL_WIN_TIME);
  timestamp ts;
  ts.tv_sec = 1;
  ts.tv_usec = 1000;
  memcpy(cdbt.data, "78", 3);
  cdbt.size = 3;
  tb.updateTupleID(&ts);
  tb.put(&cdbt);
  
  ts.tv_usec = 2000;
  memcpy(cdbt.data, "ab", 3);
  tb.updateTupleID(&ts);
  tb.put(&cdbt);
  
  tb.getExpired(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "78"));
  assert(*i ==0);

  tb.pop();
  /*tb.getHead(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "ab"));
  assert(*i ==1);
  */

  memcpy(cdbt.data, "cd", 3);
  ts.tv_usec = 2001;
  tb.updateTupleID(&ts);
  tb.put(&cdbt);

  //assert(!tb.hasExpired());
  /*tb.getHead(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "ab"));
  assert(*i ==1);
  */

  memcpy(cdbt.data, "ef", 3);
  ts.tv_sec = 2;
  ts.tv_usec = 2001;
  tb.updateTupleID(&ts);
  tb.put(&cdbt);

  tb.getExpired(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "ab"));
  assert(*i ==1);
  tb.pop();
  

  tb.getExpired(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "cd"));
  assert(*i ==1);

  /*tb.getHead(&rk, &rd);
  i = (unsigned int*)(rk.data);
  assert(!strcmp((char*)(rd.data), "ef"));
  assert(*i ==1001);
  */
  
  return 0;

};
