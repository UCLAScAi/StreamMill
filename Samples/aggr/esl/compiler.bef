#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#include <stdlib.h>


//#include <basic.h>

#include <adl_sys.h>
#include <symbol.h>
#include <absyn.h>
#include <semant.h>
#include <trans2C.h>
#include <env.h>
#include <err.h>
#include <stmt.h>


#include "querySchdl.h"
#include <stdlib.h>
#include <iostream>
#include <stmt.h>
#include <driver.h>
#include <compiler.h>
#include <ios/ios.h>

// use, if we want to debug intermediate compile messages
// using this shows the whole buffer to the user, which is not good
// that's why we don't use it for all cases.    1>>errorsFile 2>>errorsFile
static const char *compile_cmd={"./eslcc %s >& errorsFile1"}; 
extern system_t *ntsys;
extern int sqlInitScanner(char *buf);
extern int yyparse();

A_exp abs_adl = (A_exp)0;
compiler::compiler(){
  init();
};

int compiler::init(){
  dm = DrvMgr::getInstance();  
  bm = bufferMngr::getInstance();
};

compiler* compiler::_instance = 0;
compiler* compiler::getInstance(){
  if  (!_instance){
    _instance = (compiler*)malloc(sizeof(compiler));
    if (!_instance){
      perror("malloc compiler");
      return NULL;
    }
    else
      _instance->init();
  }
  return _instance;
}


void compiler::redirectStdout(char* fileName, char* mode, FILE* out) {
  tempStdout = freopen(fileName, mode, out);
}

void compiler::redirectStderr(char* fileName, char* mode, FILE* err) {
  tempStderr = freopen(fileName, mode, err);
}

FILE* compiler::getTempStdout() {
  return tempStdout;
}

FILE* compiler::getTempStderr() {
  return tempStderr;
}

void compiler::resetStdouterr() {
  tempStdout = NULL;
  tempStderr = NULL;
}



int compiler::compile(const char* queryName, compile_opt opt){
  char qName[1024];  // e.g. stream.cq
  char wholeBuf[40960];
  char *buf;
  char *dclBuf;
  char output_file[120];
  char qNameWithDir[256];
  char exe_file[120];
  // init compiler
  int verbose = 0;
  int db2 = 0;
  int rc;
  char program_dir[120]={".\\"};
  char *program_name={"esl"};
  ntSysInit(program_dir, program_name);
  ntsys->verbose = verbose;
  ntsys->db2 = db2;
  wholeBuf[0]=0;

  if(opt == cmp_snapshot) {
    setAdHoc(1);
    setAdHocNum(0);
  }
  else
    setAdHoc(0);
  if (opt == cmp_aggr) {
    setIsESLAggr(1);
    setAggrName(queryName);
  }
  else {
    setAggrName("");
    setIsESLAggr(0);
  }

  rc = readFileIntoBuffer("../aggr/ext.dcl", dclBuf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
    //      continue;
  }
  if (dclBuf){
    strcpy(wholeBuf, dclBuf);
    free(dclBuf);
  }
  
  if (opt == cmp_aggr || opt == cmp_snapshot) { // aggr
    strcpy(qName, queryName);
    
    if(opt == cmp_snapshot) {
      strcat(qName, ".cq");    
      strcpy(qNameWithDir, "../cq/");
    }
    else if(opt == cmp_aggr) {
      strcat(qName, ".aggr");
      strcpy(qNameWithDir, "../aggr/");
    }

    strcpy(output_file, "../exe/");
    strcat(output_file, queryName);
    strcat(output_file, ".cc");

    strcpy(exe_file, "../exe/");
    strcat(exe_file, queryName);
    strcat(exe_file, ".so");
    setQueryName("");

    // read table declarations
    rc = readFileIntoBuffer("../dcl/tables.dcl", dclBuf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
      //      continue;
    }
    if(dclBuf)
    {
      strcat(wholeBuf, dclBuf);
      int length = strlen(dclBuf);
      memset(dclBuf, '\0', length);
      free(dclBuf);
      dclBuf = 0;
    }

    if(opt==cmp_snapshot||opt==cmp_aggr) {
      // read aggregates
      rc = readFileIntoBuffer("../aggr/aggr.dcl", dclBuf);
      if (rc) {
        EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
        //      continue;
      }
      if (dclBuf){
        strcat(wholeBuf, dclBuf);
        free(dclBuf);
      }
    }

    strcat(qNameWithDir, qName);
    /*rc = readFileIntoBuffer(qNameWithDir, buf);
      if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
      //      continue;
      }*/
    
  }
  else{ // CQ

    // prepare filenames
    strcpy(qName, queryName);
    strcat(qName, ".cq");
    /*
    // remove the suffix
    for (int i = strlen(queryName)-1; i>0; i--){
      if (queryName[i] == '.'){
	cq = (queryName[i+1]=='c'); // suffix .cq
	queryName[i] = 0;
      }
    }
    */
    strcpy(output_file, "../exe/");
    strcat(output_file, queryName);
    strcat(output_file, ".cc");

    strcpy(exe_file, "../exe/");
    strcat(exe_file, queryName);
    strcat(exe_file, ".so");
    setQueryName(queryName);
    
    // read table declarations
    rc = readFileIntoBuffer("../dcl/tables.dcl", dclBuf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
      //      continue;
    }
    if(dclBuf)
    {
      strcat(wholeBuf, dclBuf);
      int length = strlen(dclBuf);
      memset(dclBuf, '\0', length);
      free(dclBuf);
      dclBuf = 0;
    }

    // read stream declarations
    rc = readFileIntoBuffer("../dcl/system.dcl", dclBuf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
      //      continue;
    }
    if(dclBuf)
    {
      strcat(wholeBuf, dclBuf);
      int length = strlen(dclBuf);
      memset(dclBuf, '\0', length);
      free(dclBuf);
      dclBuf = 0;
    }

    // read aggregates
    rc = readFileIntoBuffer("../aggr/aggr.dcl", dclBuf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
      //      continue;
    }
    if (dclBuf){
      strcat(wholeBuf, dclBuf);
      free(dclBuf);
    }

    strcpy(qNameWithDir, "../cq/");
    strcat(qNameWithDir, qName);
  }
  rc = readFileIntoBuffer("../model/model.dcl", dclBuf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
    //      continue;
  }
  if (dclBuf){
    strcat(wholeBuf, dclBuf);
    free(dclBuf);
  }
  rc = readFileIntoBuffer(qNameWithDir, buf);
  if (rc) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
  }
  
  strcat(wholeBuf, buf);
  cout <<"===================="<<queryName<<"===================="<<endl
         <<wholeBuf<<endl
      <<"===================="<<queryName<<"===================="<<endl;

  char* errors = (char*) malloc(200);
  errors[0] = 0; 

  /* Lexical Analysis */
  sqlInitScanner(wholeBuf);

  /* Parsing */
  rc = yyparse();

  buffer *b = bm->lookup("_ioBuffer");
  
  if(rc != 0 || abs_adl == (A_exp)0) {
    sprintf(errors, "Error occurred while parsing the file.\n");
    rc = 1;
    goto exit;
  }
  
  if (abs_adl != (A_exp)0 && rc == 0) {
    /* Semantic Analysis & Code Generation */
    rc = trans2C(E_base_venv, E_base_tenv, abs_adl, output_file);
  }

  if(rc != 0) {
    sprintf(errors, "A semantic error occurred while translating the file.\n");
    rc = 1;
    goto exit;
  }

 exit:
  ntSysQuit();
  free(buf);
  if (rc==0){  // compilation succeeds
    char cmd[256];
    char id[10];
    sprintf(cmd, compile_cmd,
	    queryName);
    
    cout<<cmd<<endl;
    rc = system(cmd);
    if (rc ==0){
      if (b){
	//sprintf(id, "%d", getAdHocNum());
	//b->put(ADHOC_QUERIES, id);
	b->put(COMPILE_SUCCESS, queryName);
      } // end if not aggr
    }
    else if(rc != 0) {
      sprintf(errors, "Error occurred while compiling translation, but unable to read errors file.\n");
      //message will be put in the below if
    }
  }

  if (rc !=0){
    // compilation fails
    if (b) {
      b->put(COMPILE_FAILURE, errors);
    }
  }

  free(errors);
  return rc;
}





