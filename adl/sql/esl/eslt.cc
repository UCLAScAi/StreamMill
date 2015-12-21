/* eslc.cc -- ESL compiler for debug purpose
 *
 * Web Information System Lab (http://wis.cs.ucla.edu)
 * Computer Science Dept.
 * University of California, Los Angeles
 * 2004/2
 */


#include <iostream>
#include <dlfcn.h>

#include <env.h>

#include <driver.h>
#include "querySchdl.h"
#include "buffer.h"
extern "C"{
#include <mm.h>
#include "dbug.h"
}
#include <getopt.h>
#include <compiler.h>
#include <vector>
#include <esl/ios/ios.h>
#include <semant.h>
#include <absyn.h>
#include <util.h>

using namespace ESL;
using namespace std;

extern int sqlInitScanner(char *buf);
extern int yyparse();

extern S_table E_base_venv;
extern S_table E_base_tenv;
extern A_exp abs_adl;
extern system_t *ntsys;

// ESLT Main program

bufferMngr *bm;

#ifndef DBUG_OFF
extern	int yydebug;
#endif
//  DBUG_EXECUTE("yydebug", yydebug=1;);
static void usage(char *program_name)
{
  fprintf(stderr, "Usage: %s userName streamDecFile tableDecFile aggrFile queriesFile\n", program_name);
}

int compile(char* wholeBuf, char* output_file) {
  int rc = 0;
  //cout<<"===================="<<queryName<<"===================="<<endl
  //   <<wholeBuf<<endl
  //    <<"===================="<<queryName<<"===================="<<endl;
  
  /* Lexical Analysis */
  sqlInitScanner(wholeBuf);
  
  /* Parsing */
  rc = yyparse();


  if (abs_adl != (A_exp)0 && rc == 0) {
    /* Semantic Analysis & Code Generation */
    rc = trans2C(E_base_venv, E_base_tenv, abs_adl, output_file);
  }
  else {
    rc = -1;
  }
  
  return rc;
}


int main(int argc, char**argv){
  err_t ERR_NONE;
  char program_dir[120];
  char *program_name, *p;
  char *userName;
  char str_file[120];
  char tab_file[120];
  char aggr_file[120];
  char que_file[120];
  char* str_buf;
  char* tab_buf;
  char* aggr_buf;
  char* que_buf;
  str_file[0] = 0;
  tab_file[0] = 0;
  aggr_file[0] = 0;
  que_file[0] = 0;


  /* get option */
  strcpy(program_dir, argv[0]); 
  if ((p = strrchr(program_dir, '/'))) {
    program_name = p+1;
    *p='\0';
  } else {
    program_name = argv[0];
    strcpy(program_dir,".");
  }

  if(argc <= 5) {
    usage(program_name);
    return -1;
  }

  ntSysInit(program_dir, program_name);
  ntsys->verbose = 0;
  ntsys->db2 = 0;


  userName = strdup(argv[1]);
  setUserName(userName);
  sprintf(str_file, "%s.dcl", argv[2]);
  sprintf(tab_file, "%s.dcl", argv[3]);
  sprintf(aggr_file, "%s.dcl", argv[4]);
  sprintf(que_file, "%s.tq", argv[5]);


  //DBUG_PUSH(default_dbug_option);
  sDBT::createSM();
  bm = bufferMngr::getInstance();

  bm->create("stdout", SHARED);
  bm->create("_ioBuffer", SHARED);
  bm->create("_queryBuffer", SHARED);
  //TAKE OUT  bm->create("hetal__abc", SHARED);
  bm->create("hetal__cvstream", SHARED);
  bm->create("hetal__highspeeds", WINBUF, 4,_ADL_WIN_ROW);
  
  int rc = readFileIntoBuffer(str_file, str_buf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "readFileIntoBuffer");
    return -1;
  }

  rc = readFileIntoBuffer(tab_file, tab_buf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "readFileIntoBuffer");
    return -1;
  }

  rc = readFileIntoBuffer(aggr_file, aggr_buf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "readFileIntoBuffer");
    return -1;
  }

  rc = readFileIntoBuffer(que_file, que_buf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "readFileIntoBuffer");
    return -1;
  }
  
  vector<char*> stmts = parseStmts(que_buf);
  int size = stmts.size();

  for(int i = 0; i < size; i++) {
    char fileId[20];
    char fileName[40];
    char* sql_buf;
    FILE* fdesc;
    fileId[0] = '\0';
    fileName[0] = '\0';

      buffer* buf = bm->lookup("hetal__cvstream");
      if(buf) {
      cDBT cdbt(2*sizeof(int)+sizeof(struct timeval));
      int x = 4;
      memset(cdbt.data, 0, 2*sizeof(int));
      memcpy(cdbt.data, &x, sizeof(int));
      memcpy(cdbt.data+sizeof(int), &x, sizeof(int));

      struct timeval tv;
      struct timezone tz;
      gettimeofday(&tv, &tz);
      memcpy(cdbt.data+2*sizeof(int), (char*)&tv, sizeof(struct timeval));

      buf->put(&cdbt);
      
      buffer* wbuf = bm->lookup("hetal__cvstream");
      if(wbuf) {
	//wbuf->updateTupleID();
	wbuf->put(&cdbt);
      }
      }
  
  
    /*TAKE OUT
      buffer* buf = bm->lookup("hetal__abc");
      if(buf) {
      cDBT cdbt(sizeof(int));
      int x = 4;
      memset(cdbt.data, 0, 10);
      memcpy(cdbt.data, &x, sizeof(int));
      buf->put(&cdbt);
      }*/

    sprintf(fileId, "tmpFile_%d", i);
    setQueryName(fileId);
    setESLTestFlag();
    printf("%d\n", isESLTest());
    sprintf(fileName, "../cq/%s.cq", fileId);
    fdesc = fopen(fileName, "w");
    if(fdesc == NULL) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "fopen");
      return -1;
    }
    if(str_buf)
      fprintf(fdesc, "%s", str_buf);
    if(tab_buf)
      fprintf(fdesc, "\n%s", tab_buf);
    if(aggr_buf)
      fprintf(fdesc, "\n%s", aggr_buf);
    fprintf(fdesc, "\n%s", stmts[i]);

    fclose(fdesc);

    char command[1024];
    command[0] = '\0';

    rc = readFileIntoBuffer(fileName, sql_buf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "readFileIntoBuffer");
      return -1;
    }

    char ccFileName[40];
    ccFileName[0] = '\0';

    sprintf(ccFileName, "../exe/%s.cc", fileId);

    int ret;
    /*ret = compile(sql_buf, ccFileName);
    if(ret < 0) {
      remove(fileName);
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "eslt::compile");
      return -1;
      }*/

    /*sprintf(command, "./eslcc %s", fileId);
      ret = system(command);
      printf("here ret %d\n", ret);
      if(ret < 0)
      {
      remove(ccFileName);
      remove(fileName);
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "eslcc");
      return -1;
      }*/
    
    char soFileName[40];
    soFileName[0] = 0;
    sprintf(soFileName, "../exe/%s.so", fileId);

    void* handle = dlopen(soFileName, RTLD_NOW);
    char* error;

    if(!handle) {
      remove(fileName);
      //remove(ccFileName);
      //remove(soFileName);
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "dlopen");
      return -1;
    }

    int (*func)(bufferMngr*, int, buffer*);
    func = (int(*)(bufferMngr*, int, buffer*))dlsym(handle, fileId);
    if (dlerror() != NULL)  {
      remove(fileName);
      //remove(ccFileName);
      //remove(soFileName);
      dlclose(handle);
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "eslt::main", "dlsym");
      return -1;
    }

    printf("Executing Once Query: %s\n", stmts[i]);
    //Call the func
    (*func)(bm, 0, NULL);

    remove(fileName);
    //remove(ccFileName);
    dlclose(handle);
    //remove(soFileName);
    cout<<"Done!\n";
  }

  free(str_buf);
  free(tab_buf);
  free(aggr_buf);
  free(que_buf);
  
  ntSysQuit();
  sDBT::destroySM();
};
