#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <dlfcn.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <absyn.h>
#include "../dbt.h"
#include "compiler.h"
#include "ios.h"
#include "GUIClient.h"
#include "io.h"
#include "esl/querySchdl.h"

#include "basic.h"

#include "esl/querySchdl.h"

#include <vector>
using namespace ESL;
using namespace std;

#define DELAY_USECS 100000

#define OUT_PORT 5431

#define DECLARE_TABLE_STMT 0
#define QUERY_STMT 1
#define AGGR_STMT 2
#define EXTERN_STMT 3
#define DECLARE_STREAM_STMT 4
#define MODEL_STMT 5

#define MAX_BUFFER 819200
#define MAX_STMT_LEN 204800
#define MAX_STMT_PATH 165

#define GET_TUPLE_ERROR -1
#define GET_TUPLE_GOT_DATA 0
#define GET_TUPLE_CLOSE_CONNECTION 1
#define GET_TUPLE_NO_DATA 2

#define ADD_QUERIES_COMMAND "AddQueries"
#define VIEWALL_QUERIES_COMMAND "ViewAllQueries"
#define VIEW_QUERY_MODULE_COMMAND "ViewQueryModule"
#define VIEW_QUERY_COMMAND "ViewQuery"
#define ACTIVATE_QUERY_COMMAND "ActivateQuery"
#define DEACTIVATE_QUERY_COMMAND "DeactivateQuery"
#define ACTIVATE_QUERY_MODULE_BY_NAME_COMMAND "ActivateQueryModuleByName"
#define ACTIVATE_QUERY_MODULE_COMMAND "ActivateQueryModule"
#define DEACTIVATE_QUERY_MODULE_COMMAND "DeactivateQueryModule"
#define DELETE_QUERY_MODULE_COMMAND "DeleteQueryModule"
#define DELETE_QUERY_COMMAND "DeleteQuery"

#define ADD_TABLES_COMMAND "AddTables"
#define ADD_STREAMS_COMMAND "AddStreams"
#define VIEWALL_STREAMS_COMMAND "ViewAllStreams"
#define VIEWALL_TABLES_COMMAND "ViewAllTables"
#define VIEW_DECLARE_MODULE_COMMAND "ViewDeclareModule"
#define VIEW_DECLARE_COMMAND "ViewDeclare"
#define DELETE_DECLARE_MODULE_COMMAND "DeleteDeclareModule"
#define DELETE_DECLARE_COMMAND "DeleteDeclare"

#define ACTIVATE_STREAM_COMMAND "ActivateStream"
#define DEACTIVATE_STREAM_COMMAND "DeactivateStream"
#define ACTIVATE_STREAM_MODULE_BY_NAME_COMMAND "ActivateStreamModuleByName"
#define ACTIVATE_STREAM_MODULE_COMMAND "ActivateStreamModule"
#define DEACTIVATE_STREAM_MODULE_COMMAND "DeactivateStreamModule"

#define ADD_AGGREGATE_COMMAND "AddAggregate"
#define ADD_EXT_AGGREGATE_COMMAND "AddExtAggregate"
#define VIEWALL_AGGREGATES_COMMAND "ViewAllAggregates"
#define VIEW_AGGREGATE_COMMAND "ViewAggregate"
#define DELETE_AGGREGATE_COMMAND "DeleteAggregate"

#define ADD_MODEL_COMMAND "AddModel"
#define VIEWALL_MODELS_COMMAND "ViewAllModels"
#define VIEW_MODEL_COMMAND "ViewModel"
#define DELETE_MODEL_COMMAND "DeleteModel"

#define ADD_EXTERN_COMMAND "AddExtern"
#define VIEWALL_EXTERNS_COMMAND "ViewAllExterns"
#define VIEW_EXTERN_COMMAND "ViewExtern"
#define DELETE_EXTERN_COMMAND "DeleteExtern"

#define ADD_TS_QUERY_COMMAND "AddTSQuery"
#define VIEWALL_TS_QUERIES_COMMAND "ViewAllTSQueries"
#define VIEW_TS_QUERY_COMMAND "ViewTSQuery"
#define DELETE_TS_QUERY_COMMAND "DeleteTSQuery"
#define ACTIVATE_TS_QUERY_COMMAND "ActivateTSQuery"
#define DEACTIVATE_TS_QUERY_COMMAND "DeactivateTSQuery"

#define VIEW_BUFFERS_COMMAND "ViewBuffers"
#define MONITOR_BUFFER_COMMAND "MonitorBuffer"
#define UNMONITOR_BUFFER_COMMAND "UnMonitorBuffer"
#define UNMONITOR_ALL_OF_IP_COMMAND "UnMonitorAllOfIP"

#define VIEW_PERFORMANCES_COMMAND "ViewPerformances"
#define MONITOR_PERFORMANCES_COMMAND "MonitorPerformances"
#define UNMONITOR_PERFORMANCES_COMMAND "UnMonitorPerformances"

#define ADD_IOMODULE_COMMAND "AddIOModule"
#define VIEW_IOMODULES_COMMAND "ViewIOModules"
#define VIEW_IOMODULE_COMMAND "ViewIOModule"
#define DROP_IOMODULE_COMMAND "DropIOModule"
//#define ACTIVATE_IOMODULE_COMMAND "ActivateIOModule"
//#define DEACTIVATE_IOMODULE_COMMAND "DeactivateIOModule"

#define VIEW_COMPONENTS_COMMAND "ViewComponents"
#define VIEW_COMPONENT_DETAILS_COMMAND "ViewComponentDetails"
#define MOVE_STATEMENT_COMMAND "MoveStatement"
#define BREAK_COMPONENT_COMMAND "BreakComponent"
#define MERGE_COMPONENTS_COMMAND "MergeComponents"
#define SET_COMPONENT_PRIORITY_COMMAND "SetComponentPriority"

#define DOES_USERNAME_EXIST_COMMAND "DoesUserExist"
#define ADD_NEW_USER_COMMAND "AddNewUser"
#define AUTHENTICATE_USER_COMMAND "AuthenticateUser"
#define MAKE_USER_PUBLIC_COMMAND "MakeUserPublic"
#define MAKE_USER_PRIVATE_COMMAND "MakeUserPrivate"

#define VIEW_LIB_COMMAND "ViewLib"

#define VIEW_ALL_COMMAND "ViewAll"

#define ONE_TUPLE_TEST_COMMAND "OneTupleTest"
#define SNAPSHOT_QUERY_COMMAND "SnapshotQuery"


#define QUERY_FILE_PATH "./../cq/"
#define QUERY_FILE_EXT ".cq"

#define EXE_FILE_PATH "./../exe/"

#define DECLARE_FILE_PATH "./../dcl/"
#define DECLARE_FILE_EXT ".dcl"

#define AGGREGATE_FILE_PATH "./../aggr/"
#define AGGREGATE_FILE_EXT ".aggr"

#define MODEL_FILE_PATH "./../model/"
#define MODEL_FILE_EXT ".model"

#define USERS_TABLENAME "__users__"
#define IOMODULES_TABLENAME "__iomodules__"
#define AGGREGATES_TABLENAME "__aggregates__"
#define EXTERNS_TABLENAME "__externs__"
#define DECLARES_TABLENAME "__declares__"
#define QUERIES_TABLENAME "__queries__"
#define TSQUERIES_TABLENAME "__tsqueries__"
#define MODELS_TABLENAME "__models__"

#define MODULE_FLAG 0
#define STMT_FLAG 1


using namespace std;

A_iomodule A_IOmodule(buffer* buf, char*  name, char* fileName, 
		      int isActive, char* userName)
{
  A_iomodule iomodule = (A_iomodule)malloc(sizeof(*iomodule));
  iomodule->buf = buf;
  iomodule->name = strdup(name);
  iomodule->isActive = isActive;
  iomodule->userName = strdup(userName);
  iomodule->fileName = strdup(fileName);
  iomodule->handle = NULL;
  return iomodule;
}

A_user A_User(char* userName, char* email, char* password, int isPublic)
{
  A_user user = (A_user)malloc(sizeof(*user));

  user->userName = strdup(userName);
  user->email = strdup(email);
  user->password = strdup(password);
  user->isPublic = isPublic;

  return user;
}


A_stdoutBuf A_StdoutBuf(char* userName, char* bufName, sb_t type, 
			char* machine, int portNum)
{
  A_stdoutBuf stdoutBuf = (A_stdoutBuf)malloc(sizeof(*stdoutBuf));

  stdoutBuf->userName = strdup(userName);
  stdoutBuf->bufName = strdup(bufName);
  stdoutBuf->type = type;
  if(machine!= NULL)
    stdoutBuf->machine = strdup(machine);
  else
    stdoutBuf->machine = NULL;
  stdoutBuf->portNum = portNum;
  return stdoutBuf;
}


A_stmt A_Stmt(int isActive, char* id, char* userName, char* bufName, 
	      int isDirectStream, char* displayText)
{
  A_stmt stmt = (A_stmt)malloc(sizeof(*stmt));
  stmt->isActive = isActive;
  stmt->isDirectStream = isDirectStream;
  stmt->id = strdup(id);
  if(displayText)
    stmt->displayText = strdup(displayText);
  else
    stmt->displayText = NULL;
  stmt->userName = strdup(userName);
  if(bufName)
    stmt->bufName = strdup(bufName);
  else
    stmt->bufName = NULL;
  return stmt;
}

StmtModule::StmtModule() {
}

StmtModule::StmtModule(int isActive, char* moduleName, char* moduleId, 
                       vector<A_stmt> stmtIds, char* userName,
                       int stmtType, int hidden)
{
  this->isActive = isActive;
  this->moduleName = strdup(moduleName);
  this->moduleId = strdup(moduleId);
  this->stmtIds = stmtIds;
  this->userName = strdup(userName);
  this->stmtType = stmtType;
  this->hidden = hidden;
}

int StmtModule::getIsActive() {
  return isActive;
}

void StmtModule::setIsActive(int isActive) {
  this->isActive = isActive;
}

char* StmtModule::getModuleName()
{
  return moduleName;
}

char* StmtModule::getModuleId()
{
  return moduleId;
}

char* StmtModule::getUserName()
{
  return userName;
}

int StmtModule::getStmtType()
{
  return stmtType;
}

int StmtModule::isHidden() {
  return hidden;
}
 
vector<A_stmt> StmtModule::getStmtIdsVector()
{
  return stmtIds;
}

void StmtModule::addStmt(A_stmt stmt) {
  stmtIds.push_back(stmt);
}

buffer* Ios::stdoutMessageBuffer = 0;
buffer* Ios::qBuf = 0;

Ios* Ios::_instance = 0;

Ios* Ios::getInstance(){
  if (_instance == (Ios*)NULL){
    _instance= new Ios();
    if (_instance == (Ios*)NULL){
      perror("malloc IOS");
      return NULL;
    }
    //else
      //_instance->init();
  }
  return _instance;
  
}

void createDummyCQFile()
{
  char* fileName = "../cq/__dummy__.cq";
  FILE* fdesc;
  
  fdesc = fopen(fileName, "w");

  fwrite("  ", 2, 1, fdesc);
  fclose(fdesc);
}


A_stmt findStmtById(vector<StmtModule> stmtModules, char* stmtId) {
  int size = stmtModules.size();

  for(int i = 0; i < size; i++) {
    StmtModule sm = stmtModules[i];
    vector<A_stmt> stmts = sm.getStmtIdsVector();
    int stmtsSize = stmts.size();
    for(int j = 0; j < stmtsSize; j++) {
      if(strcmp(stmtId, stmts[j]->id) == 0)
	return stmts[j];
    }
  }
  return NULL;
}


void Ios::initializeSystemTables()
{
  FILE* sysFdesc;
  char sysFileName[MAX_STMT_PATH];
  //char* buf = (char*)malloc(10*MAX_STMT_LEN);
  //char* temp = (char*)malloc(10*MAX_STMT_LEN);

  sprintf(sysFileName, "%stables%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);

  sysFdesc = fopen(sysFileName, "w");
  remove(sysFileName);
  fwrite(" ", strlen(" "), 1, sysFdesc);
  fclose(sysFdesc);
  /*
  memset(temp, '\0', 10*MAX_STMT_LEN);
  if(sysFdesc)
  {
    fread(temp, MAX_STMT_LEN, 1, sysFdesc);
    fclose(sysFdesc);
  }

  if(temp != NULL && strlen(temp) != 0)
  {
    rewriteSystemDeclares(DECLARE_TABLE_STMT);

    sprintf(buf, "TablesAtSystemStart\n%s", temp);

    processAddDeclaresCommand("system", 0, buf, "1_1_1_0", DECLARE_TABLE_STMT);
  }

  free(buf);
  free(temp);
  */
}

void Ios::initializeSystemStreams()
{
  FILE* sysFdesc;
  char sysFileName[MAX_STMT_PATH];
  //char* buf = (char*)malloc(10*MAX_STMT_LEN);
  //char* temp = (char*)malloc(10*MAX_STMT_LEN);

  sprintf(sysFileName, "%ssystem%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);
  remove(sysFileName);

  sysFdesc = fopen(sysFileName, "w");
  fwrite(" ", strlen(" "), 1, sysFdesc);

  fclose(sysFdesc);
  /*
  memset(temp, '\0', 10*MAX_STMT_LEN);
  if(sysFdesc)
  {
    fread(temp, MAX_STMT_LEN, 1, sysFdesc);
    fclose(sysFdesc);
  }

  if(temp != NULL && strlen(temp) != 0)
  {
    rewriteSystemDeclares(DECLARE_STREAM_STMT);
    sprintf(buf, "StreamsAtSystemStart\n%s", temp);
    processAddDeclaresCommand("system", 0, buf, "1_1_1_1", 
			      DECLARE_STREAM_STMT);
  }
  
  free(buf);
  free(temp);*/
}

void Ios::initializeAggregateDeclares()
{
  FILE* aggregateFdesc;
  FILE* aggrFdesc;
  char aggrFileName[MAX_STMT_PATH];
  char aggregateFileName[MAX_STMT_PATH];
  //char* buf = (char*)malloc(10*MAX_STMT_LEN);

  sprintf(aggrFileName, "%saggr%s", AGGREGATE_FILE_PATH, ".dcl");
  sprintf(aggregateFileName, "%slastTimeAggrs%s", AGGREGATE_FILE_PATH, ".aggr");

  aggrFdesc = fopen(aggrFileName, "w");
  remove(aggrFileName);
  fwrite(" ", strlen(" "), 1, aggrFdesc);
  fclose(aggrFdesc);
  /*
  if(aggrFdesc)
  {
    memset(buf, '\0', 10*MAX_STMT_LEN);
    fread(buf, 10*MAX_STMT_LEN, 1, aggrFdesc);
    fclose(aggrFdesc);
  }
  if(buf && strlen(buf) > 0)
  {
    char* moduleName = "lastTimeAggrs";
    vector<A_stmt> declareIds;

    aggregateFdesc = fopen(aggregateFileName, "w");
    fwrite(buf, strlen(buf), 1, aggregateFdesc);
    fclose(aggregateFdesc);
    activeAggregateModules.push_back(StmtModule(0, moduleName, moduleName, declareIds, "system"));

  }
  free(buf);*/
}

int Ios::init(){
  bufferMngr* bm = bufferMngr::getInstance();
  mkdir(QUERY_FILE_PATH, 0755);
  mkdir(EXE_FILE_PATH, 0755);
  mkdir(DECLARE_FILE_PATH, 0755);
  mkdir(AGGREGATE_FILE_PATH, 0755);
  mkdir(MODEL_FILE_PATH, 0755);


  /* not completely sure if all three of these are required */
  /*tempdb_init();
  hashgb_init();
  _adl_dlm_init(); */
   

  createDummyCQFile();

  /* Setting varaibles for query module */
  qReadSockfd = -1;
  qListenSockfd = -1;

  stdoutBuffers = new vector<A_stdoutBuf>;

  /* Comment out for testing with main.cc */
  qBuf = bm->lookup("_queryBuffer");
  stdoutMessageBuffer = bm->lookup("_ioBuffer");

  rewriteAggregateDeclares();
  rewriteModelDeclares();

  rewriteSystemDeclares(DECLARE_TABLE_STMT);
  rewriteSystemDeclares(DECLARE_STREAM_STMT);
}

Ios::Ios()
{
  init();
}

void Ios::printErrorMessage(char* message, int rc) {
  fprintf(stderr, "ERROR:\n");
  fprintf(stderr, message, rc);
}


void appendDeclaresToSystemFile(vector<A_stmt> declareIds, int stmtType)
{
  FILE* sysFdesc;
  char sysFileName[MAX_STMT_PATH];
  char* buf = (char*)malloc(MAX_STMT_LEN);

  if(stmtType == DECLARE_STREAM_STMT)
  {
    sprintf(sysFileName, "%ssystem%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);
  }
  else if(stmtType == DECLARE_TABLE_STMT)
  {
    sprintf(sysFileName, "%stables%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);
  }

  sysFdesc = fopen(sysFileName, "a");

  int size = declareIds.size();
  for(int i=0; i < size; i++)
  {
    char* decId = declareIds.operator[](i)->id;
    char decFilePath[MAX_STMT_PATH];
    FILE* decFile;
    buf[0] = '\0';

    sprintf(decFilePath, "%s%s%s", DECLARE_FILE_PATH, decId, DECLARE_FILE_EXT);
    decFile = fopen(decFilePath, "r");
    
    memset(buf, '\0', MAX_STMT_LEN);
    fread(buf, MAX_STMT_LEN, 1, decFile);
    fwrite(buf, strlen(buf), 1, sysFdesc);
    fclose(decFile);
  }
  free(buf);

  fclose(sysFdesc);
}

void appendAggregatesToSystemFile(int stmtType, int aType)
{
  FILE* sysFdesc;
  char sysFileName[MAX_STMT_PATH];
  char* buf = (char*)malloc(10*MAX_STMT_LEN);

  if(stmtType == DECLARE_STREAM_STMT)
    sprintf(sysFileName, "%ssystem%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);
  else if(stmtType == DECLARE_TABLE_STMT)
    sprintf(sysFileName, "%stables%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);

  sysFdesc = fopen(sysFileName, "a");

  FILE* aggrFile;
  buf[0] = '\0';

  if(aType == MODEL_STMT) {
    aggrFile = fopen("../model/model.dcl", "r");
  }
  else if(aType == AGGR_STMT) {
    aggrFile = fopen("../aggr/aggr.dcl", "r");
  }
  else if(aType == EXTERN_STMT) {
    aggrFile = fopen("../aggr/ext.dcl", "r");
  }
    
  memset(buf, '\0', 10*MAX_STMT_LEN);
  fread(buf, MAX_STMT_LEN, 1, aggrFile);
  fwrite(buf, strlen(buf), 1, sysFdesc);
  fclose(aggrFile);
  free(buf);

  fclose(sysFdesc);
}

void appendDeclareToSystemFile(char* fileName, int stmtType)
{
  FILE* sysFdesc;
  char sysFileName[MAX_STMT_PATH];
  char* buf = (char*)malloc(MAX_STMT_LEN);

  if(stmtType == DECLARE_STREAM_STMT)
    sprintf(sysFileName, "%ssystem%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);
  else if(stmtType == DECLARE_TABLE_STMT)
    sprintf(sysFileName, "%stables%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);

  sysFdesc = fopen(sysFileName, "a");

  FILE* decFile;
  buf[0] = '\0';

  decFile = fopen(fileName, "r");
    
  memset(buf, '\0', MAX_STMT_LEN);
  fread(buf, MAX_STMT_LEN, 1, decFile);
  fwrite(buf, strlen(buf), 1, sysFdesc);
  fclose(decFile);
  free(buf);

  fclose(sysFdesc);
}


int findModuleById(char* userName, vector<StmtModule> asms, char* id)
{
  int size = asms.size();

  for(int i = 0; i< size; i++)
  {
    StmtModule sm = asms.operator[](i);
    if(strcmp(id, sm.getModuleId()) == 0 && 
       strcmp(userName, sm.getUserName()) == 0)
    {
      return i;
    }
  }
  return -1;
}

int findModuleByName(char* userName, vector<StmtModule> asms, char* name)
{
  int size = asms.size();

  for(int i = 0; i< size; i++)
  {
    StmtModule sm = asms.operator[](i);
    printf("comparing %s %s\n", name, sm.getModuleName());fflush(stdout);
    if(strcmp(name, sm.getModuleName()) == 0 && 
       strcmp(userName, sm.getUserName()) == 0)
    {
      return i;
    }
  }
  return -1;
}



/* Persistent */
void Ios::addUser(A_user user) {
  DB* usersTable;
  int rc;

  users.push_back(user);

  if ((rc = db_create(&usersTable, NULL, 0)) != 0) {
    printErrorMessage("addUser, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = usersTable->set_pagesize(usersTable, 2048)) != 0) {
    printErrorMessage("addUser, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = usersTable->open(usersTable, USERS_TABLENAME, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("addUser, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }

  DBT key, data;
  char datadata[128+128+4];

  
  memset(&data, 0, sizeof(data));
  memset(&key, 0, sizeof(key));
  data.data = datadata;
  strcpy((char*)data.data, user->userName);
  strcpy((char*)data.data + strlen(user->userName)+1, user->email);
  strcpy((char*)data.data + strlen(user->userName)+1 + strlen(user->email)+1,
	 user->password);
  memcpy((char*)data.data + strlen(user->userName)+1 + strlen(user->email)+1 
	 + strlen(user->password)+1, &(user->isPublic), sizeof(int));

  data.size = strlen(user->userName)+1 + strlen(user->email)+1 +
    strlen(user->password)+1 +sizeof(int);
  key.size = 0;

  if((rc = usersTable->put(usersTable, NULL, &key, &data, DB_APPEND)) != 0) {
    printErrorMessage("addUser, put returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }

  if((rc = usersTable->close(usersTable, 0)) != 0) {
    printErrorMessage("addUser, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}

void Ios::readPersistentUsers() {
  DB* usersTable;
  DBC* usersTableC;
  int rc;
  DBT key, data;
  char userName[128];
  char email[256];
  char password[128];
  int isPublic;
  int i = 1;
  bufferMngr* bm = bufferMngr::getInstance();

  if ((rc = db_create(&usersTable, NULL, 0)) != 0) {
    printErrorMessage("readPersistentUsers, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = usersTable->set_pagesize(usersTable, 2048)) != 0) {
    printErrorMessage("readPersistentUsers, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = usersTable->open(usersTable, USERS_TABLENAME, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("readPersistentUsers, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = usersTable->cursor(usersTable, NULL, &usersTableC, 0)) != 0) {
    printErrorMessage("readPersistentUsers, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = usersTableC->c_get(usersTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;
  
  strcpy(userName, (char*)data.data);
  strcpy(email, (char*)data.data+ strlen(userName)+1);
  strcpy(password, (char*)data.data+ strlen(userName)+1 + strlen(email)+1);
  memcpy(&isPublic, (char*)data.data + strlen(userName)+1 + strlen(email)+1 
	    + strlen(password)+1, sizeof(int));
  //printf("user %s email %s password %s isPublic %d\n", 
//	 userName, email, password, isPublic);
  users.push_back(A_User(userName, email, password, isPublic));
  char errorBuf[1024];
  sprintf(errorBuf, "%s_errors", userName);
  bm->create(errorBuf, SHARED);
  stdoutBuffers->push_back(A_StdoutBuf(strdup(userName), 
				       strdup(errorBuf)));

  while(rc == 0) {
    rc = usersTableC->c_get(usersTableC, &key, &data, DB_NEXT);
    if(rc == 0) {
      i++;

      strcpy(userName, (char*)data.data);
      strcpy(email, (char*)data.data+ strlen(userName)+1);
      strcpy(password, (char*)data.data+ strlen(userName)+1 + strlen(email)+1);
      memcpy(&isPublic, (char*)data.data + strlen(userName)+1 + 
	     strlen(email)+1 + strlen(password)+1, sizeof(int));
      printf("user %s email %s isPublic %d\n", 
	     userName, email, isPublic);
      users.push_back(A_User(userName, email, password, isPublic));
      char errorBuf[1024];
      sprintf(errorBuf, "%s_errors", userName);
      bm->create(errorBuf, SHARED);
      stdoutBuffers->push_back(A_StdoutBuf(strdup(userName), 
					   strdup(errorBuf)));

    }
  }
  printf("Retreived %d users from persistent system table.\n", i);
 exit:
  if(usersTableC != NULL) {
    if((rc = usersTableC->c_close(usersTableC)) != 0) {
      printErrorMessage("readPersistentUsers, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(usersTable != NULL && (rc = usersTable->close(usersTable, 0)) != 0) {
    printErrorMessage("readPersistentUsers, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}

void Ios::changeUserTypeInPersistentTable(char* userName1, int isPublic1) {
  DB* usersTable;
  DBC* usersTableC;
  int rc;
  DBT key, data;
  char userName[128];
  char email[256];
  char password[128];
  int isPublic;

  if ((rc = db_create(&usersTable, NULL, 0)) != 0) {
    printErrorMessage("changeUserTypeInPersi, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = usersTable->set_pagesize(usersTable, 2048)) != 0) {
    printErrorMessage("changeUserTypeInPersi, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = usersTable->open(usersTable, USERS_TABLENAME, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("changeUserTypeInPersi, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = usersTable->cursor(usersTable, NULL, &usersTableC, 0)) != 0) {
    printErrorMessage("changeUserTypeInPersi, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = usersTableC->c_get(usersTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;
  
  strcpy(userName, (char*)data.data);
  strcpy(email, (char*)data.data+ strlen(userName)+1);
  strcpy(password, (char*)data.data+ strlen(userName)+1 + strlen(email)+1);
  memcpy(&isPublic, (char*)data.data + strlen(userName)+1 + strlen(email)+1 
	    + strlen(password)+1, sizeof(int));
  if(strcmp(userName, userName1) == 0) {
    memcpy((char*)data.data+ strlen(userName)+1 + strlen(email)+1
	   + strlen(password)+1, &isPublic1, sizeof(int));
    if((rc = usersTableC->c_put(usersTableC, &key, &data, DB_CURRENT)) != 0) {
      printErrorMessage("changeUserTypeInPersi, cursor put returned %d. "
			"Persistence may not work correctly\n", rc);
      goto exit;
    }
  }

  while(rc == 0) {
    rc = usersTableC->c_get(usersTableC, &key, &data, DB_NEXT);
    if(rc == 0) {
      strcpy(userName, (char*)data.data);
      strcpy(email, (char*)data.data+ strlen(userName)+1);
      strcpy(password, (char*)data.data+ strlen(userName)+1 + strlen(email)+1);
     
      if(strcmp(userName, userName1) == 0) {
	memcpy((char*)data.data+ strlen(userName)+1 + strlen(email)+1
	       + strlen(password)+1, &isPublic1, sizeof(int));
	if((rc = usersTableC->c_put(usersTableC, &key, &data, DB_CURRENT)) != 0) {
	  printErrorMessage("changeUserTypeInPersi, cursor put returned %d. "
			    "Persistence may not work correctly\n", rc);
	  goto exit;
	}
      }
    }
  }
 exit:
  if(usersTableC != NULL) {
    if((rc = usersTableC->c_close(usersTableC)) != 0) {
      printErrorMessage("changeUserTypeInPersi, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(usersTable != NULL && (rc = usersTable->close(usersTable, 0)) != 0) {
    printErrorMessage("changeUserTypeInPersi, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}


int Ios::addIOModule(A_iomodule m, char* filename, char*& errors)
{
  if(doesIOModuleExist(m->name) == false) {  
    m->handle = dlopen(filename, RTLD_NOW);
    char* error;
    int rc;
    
    if(!m->handle) {
      sprintf(errors, "In addIOModule, %s - file %s\n", dlerror(), filename);
      return 1;
    }
    
    m->getTuple = (int(*)(buffer*, bufferMngr*))dlsym(m->handle, "getTuple");
    if ((error = dlerror()) != NULL)  {
      sprintf (errors, "In addIOModule, %s\n", error);
      dlclose(m->handle);
      return 1;
    }

    m->closeConnection = (int(*)())dlsym(m->handle, "closeConnection");
    if ((error = dlerror()) != NULL)  {
      sprintf (errors, "In addIOModule, %s\n", error);
      dlclose(m->handle);
      return 1;
    }

    activeIOmodules.push_back(m);
    
    DB* iomodulesTable;
    if ((rc = db_create(&iomodulesTable, NULL, 0)) != 0) {
      sprintf(errors, "addIOModule, db_create returned %d. "
	      "Persistence may not work correctly\n", rc);
      return 0;
    }
    if ((rc = iomodulesTable->set_pagesize(iomodulesTable, 2048)) != 0) {
      sprintf(errors, "addIOModule, set_pagesize returned %d. "
	      "Persistence may not work correctly\n", rc);
      return 0;
    }
    if ((rc = iomodulesTable->open(iomodulesTable, IOMODULES_TABLENAME, 
				   NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
      sprintf(errors, "addIOModule, open returned %d. "
	      "Persistence may not work correctly\n", rc);
      return 0;
    }
    
    DBT key, data;
    char datadata[128+4+128+128];
    
    memset(&data, 0, sizeof(data));
    memset(&key, 0, sizeof(key));
    data.data = datadata;
    strcpy((char*)data.data, m->name);
    memcpy((char*)data.data + strlen(m->name)+1, &(m->isActive) , sizeof(int));
    strcpy((char*)data.data + strlen(m->name)+1 + sizeof(int), m->userName);
    strcpy((char*)data.data + strlen(m->name)+1 + sizeof(int) + 
	   strlen(m->userName)+1, m->fileName);
    
    data.size = strlen(m->name)+1 + sizeof(int) + strlen(m->userName)+1
    + strlen(m->fileName)+1;
    key.size = 0;
    
    if((rc = iomodulesTable->put(iomodulesTable, NULL, &key, &data, DB_APPEND)) != 0) {
      sprintf(errors, "addIOModule, put returned %d. "
	      "Persistence may not work correctly\n", rc);
      return 0;
    }
    
    if((rc = iomodulesTable->close(iomodulesTable, 0)) != 0) {
      sprintf(errors, "addIOModule, close returned %d. "
	      "Persistence may not work correctly\n", rc);
      return 0;
    }
    
    return 0;
  }
  return 0;
}
  
int Ios::prepareIOModule(A_iomodule m, char* filename) {
  m->handle = dlopen(filename, RTLD_NOW);
  char* error;
  int rc;

  if(!m->handle)
  {
    printf("In prepareIOModule, %s\n", dlerror());
    return 1;
  }

  m->getTuple = (int(*)(buffer*, bufferMngr*))dlsym(m->handle, "getTuple");
  if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "In prepareIOModule, %s\n", error);
    return 1;
  }

  m->closeConnection = (int(*)())dlsym(m->handle, "closeConnection");
  if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "In prepareIOModule, %s\n", error);
    return 1;
  }
  return 0;
}

void Ios::readPersistentIOModules() {
  DB* iomodulesTable;
  DBC* iomodulesTableC;
  int rc;
  DBT key, data;
  char name[128];
  int isActive;
  char userName[128];
  char fileName[128];
  int i = 1;
  A_iomodule m = NULL;

  if ((rc = db_create(&iomodulesTable, NULL, 0)) != 0) {
    printErrorMessage("readPersistentIOModules, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = iomodulesTable->set_pagesize(iomodulesTable, 2048)) != 0) {
    printErrorMessage("readPersistentIOModules, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = iomodulesTable->open(iomodulesTable, IOMODULES_TABLENAME, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("readPersistentIOModules, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = iomodulesTable->cursor(iomodulesTable, NULL, &iomodulesTableC, 0)) != 0) {
    printErrorMessage("readPersistentIOModules, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = iomodulesTableC->c_get(iomodulesTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;
  
  strcpy(name, (char*)data.data);
  memcpy(&isActive, (char*)data.data + strlen(name)+1, sizeof(int));
  strcpy(userName, (char*)data.data + strlen(name)+1 + sizeof(int));
  strcpy(fileName, (char*)data.data + strlen(name)+1 + sizeof(int) + strlen(userName)+1);

  m = A_IOmodule(NULL, name, fileName, isActive, userName);
  if(prepareIOModule(m, fileName) != 0) {
    printf("Unable to retrieve IOModule %s for user %s\n", name, userName);
  }
  else {
    activeIOmodules.push_back(m);
  }

  while(rc == 0) {
    rc = iomodulesTableC->c_get(iomodulesTableC, &key, &data, DB_NEXT);
    if(rc == 0) {
      strcpy(name, (char*)data.data);
      memcpy(&isActive, (char*)data.data + strlen(name)+1, sizeof(int));
      strcpy(userName, (char*)data.data + strlen(name)+1 + sizeof(int));
      strcpy(fileName, (char*)data.data + strlen(name)+1 + sizeof(int) + strlen(userName)+1);

      m = A_IOmodule(NULL, name, fileName, isActive, userName);
      if(prepareIOModule(m, fileName) != 0) {
	printf("Unable to retrieve IOModule %s for user %s\n", 
	       name, userName);
      }
      else {
	i++;
	activeIOmodules.push_back(m);
      }
    }
  }
  printf("Retreived %d iomodules from persistent system table.\n", i);
 exit:
  if(iomodulesTableC != NULL) {
    if((rc = iomodulesTableC->c_close(iomodulesTableC)) != 0) {
      printErrorMessage("readPersistentIOModules, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(iomodulesTable != NULL && 
     (rc = iomodulesTable->close(iomodulesTable, 0)) != 0) {
    printErrorMessage("readPersistentIOModules, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}

void Ios::removeIOModuleFromPersistentTable(char* userName1, char* name1) {
  DB* iomodulesTable;
  DBC* iomodulesTableC;
  int rc;
  DBT key, data;
  char name[128];
  int isActive;
  char userName[128];
  char fileName[128];

  if ((rc = db_create(&iomodulesTable, NULL, 0)) != 0) {
    printErrorMessage("removeIOModuleFromPersi, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = iomodulesTable->set_pagesize(iomodulesTable, 2048)) != 0) {
    printErrorMessage("removeIOModuleFromPersi, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = iomodulesTable->open(iomodulesTable, IOMODULES_TABLENAME, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("removeIOModuleFromPersi, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = iomodulesTable->cursor(iomodulesTable, NULL, &iomodulesTableC, 0)) != 0) {
    printErrorMessage("removeIOModuleFromPersi, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = iomodulesTableC->c_get(iomodulesTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;
  
  strcpy(name, (char*)data.data);
  memcpy(&isActive, (char*)data.data + strlen(name)+1, sizeof(int));
  strcpy(userName, (char*)data.data + strlen(name)+1 + sizeof(int));
  strcpy(fileName, (char*)data.data + strlen(name)+1 + sizeof(int) + strlen(userName)+1);

  if(strcmp(userName, userName1) == 0 && strcmp(name, name1) ==0) {
    if((rc = iomodulesTableC->c_del(iomodulesTableC, 0)) != 0) {
      printErrorMessage("removeIOModuleFromPersi, delete cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
    goto exit;
  }
  while(rc == 0) {
    rc = iomodulesTableC->c_get(iomodulesTableC, &key, &data, DB_NEXT);
    if(rc == 0) {
      strcpy(name, (char*)data.data);
      memcpy(&isActive, (char*)data.data + strlen(name)+1, sizeof(int));
      strcpy(userName, (char*)data.data + strlen(name)+1 + sizeof(int));
      strcpy(fileName, (char*)data.data + strlen(name)+1 + sizeof(int) + strlen(userName)+1);
      
      if(strcmp(userName, userName1) == 0 && strcmp(name, name1) ==0) {
	if((rc = iomodulesTableC->c_del(iomodulesTableC, 0)) != 0) {
	  printErrorMessage("removeIOModuleFromPersi, delete cursor returned %d. "
			    "Persistence may not work correctly\n", rc);
	}
	goto exit;
      }
    }
  }
 exit:
  if(iomodulesTableC != NULL) {
    if((rc = iomodulesTableC->c_close(iomodulesTableC)) != 0) {
      printErrorMessage("removeIOModuleFromPersi, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(iomodulesTable != NULL && 
     (rc = iomodulesTable->close(iomodulesTable, 0)) != 0) {
    printErrorMessage("removeIOModuleFromPersi, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}

void Ios::removeIOModule(char* userName, char* name)
{
  int size = activeIOmodules.size();

  for(int i = 0; i < size; i++)
  {
    A_iomodule m = activeIOmodules.operator[](i);
    if(strcmp(m->name, name) == 0 && strcmp(userName, m->userName) == 0)
    {
      char* error;
      activeIOmodules.erase(activeIOmodules.begin() + i);
      (*(m->closeConnection))();
      if(m->handle != NULL) {
	dlclose(m->handle);
      }
      if ((error = dlerror()) != NULL)  {
	fprintf (stderr, "In removeIOModule, %s\n", error);
	return;
      }

      // TODO remove the assoc files
      removeIOModuleFromPersistentTable(userName, name);
      free(m);
      break;
    }
  }
}

void Ios::addModule(StmtModule sm, char* tableName, vector<StmtModule>& sms) {
  sms.push_back(sm);
  
  DB* modulesTable;
  int rc;  
  /*int moduleFlag;
    int isActive;
    char moduleName[128];
    char moduleId[128];
    char userName[128];
    int stmtType;*/
  
  if ((rc = db_create(&modulesTable, NULL, 0)) != 0) {
    printErrorMessage("addModule, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->set_pagesize(modulesTable, 2048)) != 0) {
    printErrorMessage("addModule, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->open(modulesTable, tableName, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("addModule, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }

  DBT key, data;
  char datadata[4+4+128+128+124+4+4+128+128+128];
  int moduleLength = 0;  
  vector<A_stmt> stmtIds = sm.getStmtIdsVector();
  int stmtIdsSize = stmtIds.size();

  memset(&data, 0, sizeof(data));
  memset(&key, 0, sizeof(key));
  data.data = datadata;
  int flag = MODULE_FLAG;
  int isActive = sm.getIsActive();
  int stmtType = sm.getStmtType();
  memcpy((char*)data.data, &flag, sizeof(int));
  memcpy((char*)data.data + sizeof(int), &isActive, sizeof(int));
  strcpy((char*)data.data + sizeof(int) + sizeof(int), sm.getModuleName());
  strcpy((char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(sm.getModuleName())+1, sm.getModuleId());
  strcpy((char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(sm.getModuleName())+1 + strlen(sm.getModuleId())+1, 
	 sm.getUserName());
  memcpy((char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(sm.getModuleName())+1 + strlen(sm.getModuleId())+1 + 
	 strlen(sm.getUserName())+1, &stmtType, sizeof(int));

  moduleLength = sizeof(int) + sizeof(int) + 
    strlen(sm.getModuleName())+1 + strlen(sm.getModuleId())+1 + 
    strlen(sm.getUserName())+1 + sizeof(int);
  data.size = moduleLength;
  key.size = 0;

  if((rc = modulesTable->put(modulesTable, NULL, &key, &data, DB_APPEND)) 
     != 0) {
    printErrorMessage("addModule, put returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  flag = STMT_FLAG;
  memcpy((char*)data.data, &flag, sizeof(int));
  for(int i = 0; i < stmtIdsSize; i++) {
    /*int stmtIsActive;
      char stmtId[128];
      char stmtUserName[128];
      char stmtBufName[128];*/
    int datasize = 0;
    int directLength = 4;
    A_stmt stmt = stmtIds[i];
    memcpy((char*)data.data + moduleLength, &(stmt->isActive), sizeof(int));
    strcpy((char*)data.data + moduleLength + sizeof(int), stmt->id);
    strcpy((char*)data.data + moduleLength + sizeof(int) + strlen(stmt->id)+1,
	   stmt->userName);
    if(stmtType == DECLARE_STREAM_STMT) {
      strcpy((char*)data.data + moduleLength + sizeof(int) + 
	     strlen(stmt->id)+1 + strlen(stmt->userName)+1, stmt->bufName);
      memcpy((char*)data.data +  moduleLength + sizeof(int) + 
	     strlen(stmt->id)+1 + strlen(stmt->userName)+1 + strlen(stmt->bufName)+1,
	     &(stmt->isDirectStream), sizeof(int));
      if(stmt->isDirectStream == 1) {
	strcpy((char*)data.data +  moduleLength + sizeof(int) + 
	       strlen(stmt->id)+1 + strlen(stmt->userName)+1 + 
	       strlen(stmt->bufName)+1 + sizeof(int),
	       stmt->displayText);
	directLength = directLength + strlen(stmt->displayText)+1;
      }

      datasize = moduleLength + sizeof(int) + strlen(stmt->id)+1 + 
	strlen(stmt->userName)+1 + strlen(stmt->bufName)+1 + directLength;
    }
    else if(stmtType == DECLARE_TABLE_STMT) {
      memcpy((char*)data.data +  moduleLength + sizeof(int) + 
	     strlen(stmt->id)+1 + strlen(stmt->userName)+1,
	     &(stmt->isDirectStream), sizeof(int));
      if(stmt->isDirectStream == 1) {
	strcpy((char*)data.data +  moduleLength + sizeof(int) + 
	       strlen(stmt->id)+1 + strlen(stmt->userName)+1 + 
	       + sizeof(int),
	       stmt->displayText);
	directLength = directLength + strlen(stmt->displayText)+1;
      }

      datasize = moduleLength + sizeof(int) + strlen(stmt->id)+1 + 
	strlen(stmt->userName)+1 + directLength;
    }
    else {
      datasize = moduleLength + sizeof(int) + 
	strlen(stmt->id)+1 + strlen(stmt->userName)+1;
    }
    data.size = datasize;

    if((rc = modulesTable->put(modulesTable, NULL, &key, &data, DB_APPEND)) 
       != 0) {
      printErrorMessage("addModule, put returned %d. "
			"Persistence may not work correctly\n", rc);
      goto exit;
    }
  }

 exit:
  if((rc = modulesTable->close(modulesTable, 0)) != 0) {
    printErrorMessage("addModule, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}

void Ios::readPersistentDecModules(char* tableName, vector<StmtModule>& sms,
                                int decType) {
  DB* modulesTable;
  DBC* modulesTableC;
  int rc;
  DBT key, data;
  int moduleFlag;
  int isActive;
  char moduleName[128];
  char moduleId[128];
  char userName[128];
  int stmtType;
  int stmtIsActive;
  char stmtId[128];
  char stmtUserName[128];
  char stmtBufName[128];
  int isDirectStream;
  int modules = 0;
  int stmts = 0;
  int index = 0;
  StmtModule sm;
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buff = bm->lookup("_ioBuffer");

  if ((rc = db_create(&modulesTable, NULL, 0)) != 0) {
    printErrorMessage("readPersistentModulesDec, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->set_pagesize(modulesTable, 2048)) != 0) {
    printErrorMessage("readPersistentModulesDec, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }

  if ((rc = modulesTable->open(modulesTable, tableName, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("readPersistentModulesDec, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = modulesTable->cursor(modulesTable, NULL, &modulesTableC, 0)) != 0) {
    printErrorMessage("readPersistentModulesDec, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;

  memcpy(&moduleFlag, (char*)data.data, sizeof(int));
  memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
  strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
  strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(moduleName)+1);
  strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(moduleName)+1 + strlen(moduleId)+1);
  memcpy(&stmtType, (char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(moduleName)+1 + strlen(moduleId)+1 + strlen(userName)+1, 
	 sizeof(int));
	  
    index = findModuleById(userName, sms, moduleId);
    if(index < 0) {
      vector<A_stmt> v;
      modules++;
      if(stmtType != QUERY_STMT)
        sm = StmtModule(isActive, moduleName, moduleId, v, userName, stmtType);
      else if(stmtType == decType) 
        sm = StmtModule(0, moduleName, moduleId, v, userName, stmtType);
      sms.push_back(sm);
      index = sms.size() - 1;
    }
    else {
      sm = sms[index];
    }

    if(moduleFlag == STMT_FLAG) {
      int moduleLength = sizeof(int) + sizeof(int) + strlen(moduleName)+1 + 
        strlen(moduleId)+1 + strlen(userName)+1 + sizeof(int);
      memcpy(&stmtIsActive, (char*)data.data + moduleLength, sizeof(int));
      strcpy(stmtId, (char*)data.data + moduleLength + sizeof(int));
      strcpy(stmtUserName, (char*)data.data + moduleLength + sizeof(int) + 
  	     strlen(stmtId)+1);

      if(stmtType == DECLARE_STREAM_STMT && stmtType == decType) {
        int messageCt = 0;
        int code;
        int accessingPrivateUser = 0;
        char qName[4096];
        int gotMessage = 0;
        char fName[MAX_STMT_PATH];
        int gotAddedBuffer = 0;
        fName[0] = '\0';
      
        strcpy(stmtBufName, (char*)data.data + moduleLength + sizeof(int) + 
	       strlen(stmtId)+1 + strlen(stmtUserName)+1);
        memcpy(&isDirectStream, (char*)data.data + moduleLength + sizeof(int) + 
	       strlen(stmtId)+1 + strlen(stmtUserName)+1 + 
               strlen(stmtBufName)+1, sizeof(int));
	  
        rc = 0;
        sprintf(fName, "%s%s%s", DECLARE_FILE_PATH, stmtId, DECLARE_FILE_EXT);
      
        Ios::sendMessageToQueryBuf(SET_USER_NAME, stmtUserName);
      
        if(isDirectStream == 0) {
	  appendDeclareToSystemFile(fName, stmtType);
	  Ios::sendMessageToQueryBuf(ADD_DECLARE_CMD_CODE, stmtId);      
	  while(messageCt <= 1) {
	    while(buff->empty())
	      usleep(100000);
	    buff->get(code, (char*)qName);
	    if(code == COMPILE_FAILURE) {
	      rc = 1;
	      messageCt = 2;
	      buff->pop();
	    }
	    else if(code == COMPILE_SUCCESS) {
	      messageCt = 2;
	      buff->pop();
	    }
            else if(code == ADDED_STREAM_BUFFER) {
              strcpy(stmtBufName, qName);
              Ios::sendMessageToQueryBuf(ADDED_BUFFER, stmtBufName);
              gotAddedBuffer = 1;
              messageCt=1;
              buff->pop(); 
            }
	    else {
	      pollStdoutMessageBuffer();
	    }
	  }
	  if(gotAddedBuffer == 0) {
	    Ios::sendMessageToQueryBuf((rc==0)?COMPILE_SUCCESS:COMPILE_FAILURE, "");
	  }
        }
        else {
	  Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, stmtId);
	  while(!gotMessage) {
	    while(buff->empty())
	      usleep(100000);
	    buff->get(code, (char*)qName);
	  
	    if(code == COMPILE_FAILURE) {
	      rc = 1;
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == ADD_STREAM_DEC) {
	      buff->pop();
	    }	      
	    else if(code == COMPILE_SUCCESS) {
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == USES_LIBRARY) {
	      buff->pop();
	      if(!isUserPublic(qName)) {
	        accessingPrivateUser = 1;
	      }
	    }
	    else {
	      pollStdoutMessageBuffer();
	    }
	  }
	  if(rc != 1 && accessingPrivateUser == 1) {
	    Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmtId);
	    rc = 1;
	  }
        }
        rewriteSystemDeclares(DECLARE_STREAM_STMT);
      }
      else if(stmtType == DECLARE_TABLE_STMT) {
        int messageCt = 0;
        int code;
        int accessingPrivateUser = 0;
        char qName[4096];
        int gotMessage = 0;
        char fName[MAX_STMT_PATH];
        int gotAddedBuffer = 0;
        fName[0] = '\0';
      
        memcpy(&isDirectStream, (char*)data.data + moduleLength + sizeof(int) + 
	       strlen(stmtId)+1 + strlen(stmtUserName)+1, 
	       sizeof(int));
	  
        rc = 0;
        sprintf(fName, "%s%s%s", DECLARE_FILE_PATH, stmtId, DECLARE_FILE_EXT);
      
        Ios::sendMessageToQueryBuf(SET_USER_NAME, stmtUserName);
      
        if(isDirectStream == 0 && decType == stmtType) {
  	  appendDeclareToSystemFile(fName, stmtType);
	  Ios::sendMessageToQueryBuf(ADD_DECLARE_CMD_CODE, stmtId);      
	  while(messageCt <= 1) {
	    while(buff->empty())
	      usleep(100000);
	    buff->get(code, (char*)qName);
	    if(code == COMPILE_FAILURE) {
	      rc = 1;
	      messageCt = 2;
	      buff->pop();
	    }
	    else if(code == COMPILE_SUCCESS) {
	      messageCt = 2;
	      buff->pop();
	    }
            else if(code == ADDED_BUFFER) {
              strcpy(stmtBufName, qName);
              Ios::sendMessageToQueryBuf(ADDED_BUFFER, stmtBufName);
              gotAddedBuffer = 1;
              messageCt=1;
              buff->pop();
            }
	    else {
	      pollStdoutMessageBuffer();
	    }
	  }
	  if(gotAddedBuffer == 0) {
	    Ios::sendMessageToQueryBuf((rc==0)?COMPILE_SUCCESS:COMPILE_FAILURE, "");
	  }
        }
        else  if(isDirectStream == 1) {
	  int gotAddTableDec = 0;
	  Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, stmtId);
	  while(!gotMessage) {
	    while(buff->empty())
	      usleep(100000);
	    buff->get(code, (char*)qName);
	  
	    if(code == COMPILE_FAILURE) {
	      rc = 1;
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == ADD_TABLE_DEC) {
	      gotAddTableDec = 1;
	      buff->pop();
	    }	      
	    else if(code == COMPILE_SUCCESS) {
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == USES_LIBRARY) {
	      buff->pop();
	      if(!isUserPublic(qName)) {
	        accessingPrivateUser = 1;
	      }
	    }
	    else {
	      pollStdoutMessageBuffer();
	    }
	  }
	  if(gotAddTableDec == 1) {
	    int code1;
	    char qName1[4096];
	    int gotMessage1 = 0;
	  
	    sendMessageToQueryBuf(ACTIVATE_QUERY_CMD_CODE, stmtId);
	    while(!gotMessage1) {
	      while(buff->empty()) {
	        usleep(100000);
	      }
	      buff->get(code1, (char*)qName1);
	      if(code1 == COMMAND_FAILED) {
	        rc = 1;
		buff->pop();
		gotMessage1 = 1;
	      }
	      else if(code1 == COMMAND_SUCCESSFUL) {
	        buff->pop();
	        //updateStmtIsActive(QUERIES_TABLENAME, userName, queryId, 1);
	        gotMessage1 = 1;
	      }
	      else if(code1 == ACTIVATED_QUERY) {
	        buff->pop();
	        A_stmt stmt = findStmtById(activeQueryModules, qName1);
	        if(stmt != NULL && stmt->isActive != 1) {
		  stmt->isActive = 1;
		  //updateStmtIsActive(QUERIES_TABLENAME, userName, qName1, 1);
	        }
	        else if(stmt == NULL) {
		  stmt = findStmtById(activeDeclareModules, qName1);
		  if(stmt != NULL && stmt->isDirectStream == 1 && stmt->isActive != 1) {
		    stmt->isActive = 1;
		    //updateStmtIsActive(QUERIES_TABLENAME, userName, qName1, 1);
	          }
	        }
	      }
	      else {
	        pollStdoutMessageBuffer();
	      }
	    }
	  }
	  else {
	    rc = 1;
	  }
	  if(rc != 1 && accessingPrivateUser == 1) {
	    Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmtId);
	    rc = 1;
	  }
        }
        rewriteSystemDeclares(DECLARE_TABLE_STMT);
      }
      if(rc == 0 && stmtType == DECLARE_STREAM_STMT && decType == stmtType) {
        char displayText[4096];
        displayText[0] = 0;
        if(isDirectStream == 1) {
	  strcpy(displayText, (char*)data.data + moduleLength + sizeof(int) + 
	         strlen(stmtId)+1 + strlen(stmtUserName)+1 + 
                 strlen(stmtBufName)+1 + sizeof(int));
        }

        stmts++;
        sm.addStmt(A_Stmt(stmtIsActive, stmtId, stmtUserName, stmtBufName, 
	  		  isDirectStream, displayText));
        sms.erase(sms.begin()+index);
        sms.insert(sms.begin()+index, sm);
        rewriteSystemDeclares(DECLARE_STREAM_STMT);
      }
      else if(rc == 0 && stmtType == DECLARE_TABLE_STMT &&
              ((decType == stmtType) || (decType == DECLARE_STREAM_STMT &&
                                         isDirectStream))) {
        char displayText[4096];
        displayText[0] = 0;
        if(isDirectStream == 1) {
	  strcpy(displayText, (char*)data.data + moduleLength + sizeof(int) + 
	         strlen(stmtId)+1 + strlen(stmtUserName)+1 + sizeof(int));
        }

        stmts++;
        sm.addStmt(A_Stmt(stmtIsActive, stmtId, stmtUserName, stmtBufName, 
			isDirectStream, displayText));
        sms.erase(sms.begin()+index);
        sms.insert(sms.begin()+index, sm);
        rewriteSystemDeclares(DECLARE_TABLE_STMT);
      }
      else if(rc == 0) {
        stmts++;
        if(stmtType != QUERY_STMT)
	  sm.addStmt(A_Stmt(stmtIsActive, stmtId, stmtUserName));
        else
	  sm.addStmt(A_Stmt(0, stmtId, stmtUserName));
        sms.erase(sms.begin()+index);
        sms.insert(sms.begin()+index, sm);
      }
      else if(rc != 0) {
        printf("Error compiling retrieved query %s, continuing\n", stmtId);
        rc = 0;
      }
    }
  
  while(rc == 0) {
    rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_NEXT);
    if(rc == 0) {

      memcpy(&moduleFlag, (char*)data.data, sizeof(int));
      memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
      strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
      strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1);
      strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1 + strlen(moduleId)+1);
      memcpy(&stmtType, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1 + strlen(moduleId)+1 + strlen(userName)+1, 
	     sizeof(int));
        index = findModuleById(userName, sms, moduleId);
        if(index < 0) {
          vector<A_stmt> v;
          modules++;
          if(stmtType != QUERY_STMT)
            sm = StmtModule(isActive, moduleName, moduleId, v, userName, stmtType);
          else if(stmtType == decType)
            sm = StmtModule(0, moduleName, moduleId, v, userName, stmtType);
          sms.push_back(sm);
          index = sms.size() - 1;
        }
        else {
          sm = sms[index];
        }
 
        if(moduleFlag == STMT_FLAG) {
          int moduleLength = sizeof(int) + sizeof(int) + strlen(moduleName)+1 + 
            strlen(moduleId)+1 + strlen(userName)+1 + sizeof(int);
          memcpy(&stmtIsActive, (char*)data.data + moduleLength, sizeof(int));
          strcpy(stmtId, (char*)data.data + moduleLength + sizeof(int));
          strcpy(stmtUserName, (char*)data.data + moduleLength + sizeof(int) + 
        	     strlen(stmtId)+1);
          if(stmtType == DECLARE_STREAM_STMT && stmtType == decType) {
            int messageCt = 0;
            int code;
            int accessingPrivateUser = 0;
            char qName[4096];
            int gotMessage = 0;
            char fName[MAX_STMT_PATH];
            int gotAddedBuffer = 0;
	    fName[0] = '\0';
	    
	    strcpy(stmtBufName, (char*)data.data + moduleLength + sizeof(int) + 
		   strlen(stmtId)+1 + strlen(stmtUserName)+1);
	    memcpy(&isDirectStream, (char*)data.data + moduleLength + sizeof(int) + 
		   strlen(stmtId)+1 + strlen(stmtUserName)+1 + 
		   strlen(stmtBufName)+1, sizeof(int));
	    
	    rc = 0;
	    sprintf(fName, "%s%s%s", DECLARE_FILE_PATH, stmtId, DECLARE_FILE_EXT);
      
	    Ios::sendMessageToQueryBuf(SET_USER_NAME, stmtUserName);
      
	    if(isDirectStream == 0) {
	      appendDeclareToSystemFile(fName, stmtType);
	      Ios::sendMessageToQueryBuf(ADD_DECLARE_CMD_CODE, stmtId);      
	      while(messageCt <= 1) {
		while(buff->empty())
		  usleep(100000);
		buff->get(code, (char*)qName);
		if(code == COMPILE_FAILURE) {
		  rc = 1;
		  messageCt = 2;
		  buff->pop();
		}
		else if(code == COMPILE_SUCCESS) {
		  messageCt = 2;
		  buff->pop();
		}
                else if(code == ADDED_STREAM_BUFFER) {
                  strcpy(stmtBufName, qName);
                  Ios::sendMessageToQueryBuf(ADDED_BUFFER, stmtBufName);
                  gotAddedBuffer = 1;
                  messageCt=1;
                  buff->pop();
                }
		else {
		  pollStdoutMessageBuffer();
		}
	      }
	      if(gotAddedBuffer == 0) {
		Ios::sendMessageToQueryBuf((rc==0)?COMPILE_SUCCESS:COMPILE_FAILURE, "");
	      }
	    }
	    else {
	      Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, stmtId);
	      while(!gotMessage) {
		while(buff->empty())
		  usleep(100000);
		buff->get(code, (char*)qName);
	  
		if(code == COMPILE_FAILURE) {
		  rc = 1;
		  buff->pop();
		  gotMessage = 1;
		}
		else if(code == ADD_STREAM_DEC) {
		  buff->pop();
		}	      
		else if(code == COMPILE_SUCCESS) {
		  buff->pop();
		  gotMessage = 1;
		}
		else if(code == USES_LIBRARY) {
		  buff->pop();
		  if(!isUserPublic(qName)) {
		    accessingPrivateUser = 1;
		  }
		}
		else {
		  pollStdoutMessageBuffer();
		}
	      }
	      if(rc != 1 && accessingPrivateUser == 1) {
		Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmtId);
		rc = 1;
	      }
	    }
	    rewriteSystemDeclares(DECLARE_STREAM_STMT);
	  }
	  else if(stmtType == DECLARE_TABLE_STMT) {
	    int messageCt = 0;
	    int code;
	    int accessingPrivateUser = 0;
	    char qName[4096];
	    int gotMessage = 0;
	    char fName[MAX_STMT_PATH];
	    int gotAddedBuffer = 0;
	    fName[0] = '\0';
      
	    memcpy(&isDirectStream, (char*)data.data + moduleLength + sizeof(int) + 
		   strlen(stmtId)+1 + strlen(stmtUserName)+1, 
		   sizeof(int));
	  
	    rc = 0;
	    sprintf(fName, "%s%s%s", DECLARE_FILE_PATH, stmtId, DECLARE_FILE_EXT);
      
	    Ios::sendMessageToQueryBuf(SET_USER_NAME, stmtUserName);
      
	    if(isDirectStream == 0 && stmtType == decType) {

	      appendDeclareToSystemFile(fName, stmtType);
	      Ios::sendMessageToQueryBuf(ADD_DECLARE_CMD_CODE, stmtId);      
	      while(messageCt <= 1) {
		while(buff->empty())
		  usleep(100000);
		buff->get(code, (char*)qName);

		if(code == COMPILE_FAILURE) {
		  rc = 1;
		  messageCt = 2;
		  buff->pop();
		}
		else if(code == COMPILE_SUCCESS) {
		  messageCt = 2;
		  buff->pop();
		}
                else if(code == ADDED_BUFFER) {
                  strcpy(stmtBufName, qName);
                  Ios::sendMessageToQueryBuf(ADDED_BUFFER, stmtBufName);
                  gotAddedBuffer = 1;
                  messageCt=1;
                  buff->pop();
                }
		else {
		  pollStdoutMessageBuffer();
		}
	      }
	      if(gotAddedBuffer == 0) {
		Ios::sendMessageToQueryBuf((rc==0)?COMPILE_SUCCESS:COMPILE_FAILURE, "");
	      }
	    }
	    else  if(isDirectStream == 1) {
	      int gotAddTableDec = 0;
	      Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, stmtId);
	      while(!gotMessage) {
		while(buff->empty())
		  usleep(100000);
		buff->get(code, (char*)qName);
	  
		if(code == COMPILE_FAILURE) {
		  rc = 1;
		  buff->pop();
		  gotMessage = 1;
		}
		else if(code == ADD_TABLE_DEC) {
		  gotAddTableDec = 1;
		  buff->pop();
		}	      
		else if(code == COMPILE_SUCCESS) {
		  buff->pop();
		  gotMessage = 1;
		}
		else if(code == USES_LIBRARY) {
		  buff->pop();
		  if(!isUserPublic(qName)) {
		    accessingPrivateUser = 1;
		  }
		}
		else {
		  pollStdoutMessageBuffer();
		}
	      }
	      if(gotAddTableDec == 1) {
		int code1;
		char qName1[4096];
		int gotMessage1 = 0;
	  
		sendMessageToQueryBuf(ACTIVATE_QUERY_CMD_CODE, stmtId);
		while(!gotMessage1) {
		  while(buff->empty()) {
		    usleep(100000);
		  }
		  buff->get(code1, (char*)qName1);
		  if(code1 == COMMAND_FAILED) {
		    rc = 1;
		    buff->pop();
		    gotMessage1 = 1;
		  }
		  else if(code1 == COMMAND_SUCCESSFUL) {
		    buff->pop();
		    //updateStmtIsActive(QUERIES_TABLENAME, userName, queryId, 1);
		    gotMessage1 = 1;
		  }
		  else if(code1 == ACTIVATED_QUERY) {
		    buff->pop();
		    A_stmt stmt = findStmtById(activeQueryModules, qName1);
		    if(stmt != NULL && stmt->isActive != 1) {
		      stmt->isActive = 1;
		      //updateStmtIsActive(QUERIES_TABLENAME, userName, qName1, 1);
		    }
		    else if(stmt == NULL) {
		      stmt = findStmtById(activeDeclareModules, qName1);
		      if(stmt != NULL && stmt->isDirectStream == 1 && stmt->isActive != 1) {
			stmt->isActive = 1;
			//updateStmtIsActive(QUERIES_TABLENAME, userName, qName1, 1);
		      }
		    }
		  }
		  else {
		    pollStdoutMessageBuffer();
		  }
		}
	      }
	      else {
		rc = 1;
	      }
	      if(rc != 1 && accessingPrivateUser == 1) {
		Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmtId);
		rc = 1;
	      }
	    }
	    rewriteSystemDeclares(DECLARE_TABLE_STMT);
	  }
	  if(rc == 0 && stmtType == DECLARE_STREAM_STMT 
             && stmtType == decType) {
	    char displayText[4096];
	    displayText[0] = 0;
	    if(isDirectStream == 1) {
	      strcpy(displayText, (char*)data.data + moduleLength + sizeof(int) + 
		     strlen(stmtId)+1 + strlen(stmtUserName)+1 + 
		     strlen(stmtBufName)+1 + sizeof(int));
	    }

	    stmts++;
	    sm.addStmt(A_Stmt(stmtIsActive, stmtId, stmtUserName, stmtBufName, 
			      isDirectStream, displayText));
	    sms.erase(sms.begin()+index);
	    sms.insert(sms.begin()+index, sm);
	    rewriteSystemDeclares(DECLARE_STREAM_STMT);
	  }
	  else if(rc == 0 && stmtType == DECLARE_TABLE_STMT
                  && ((decType == stmtType)||
                      (decType == DECLARE_STREAM_STMT && isDirectStream))) {
	    char displayText[4096];
	    displayText[0] = 0;
	    if(isDirectStream == 1) {
	      strcpy(displayText, (char*)data.data + moduleLength + sizeof(int) + 
		     strlen(stmtId)+1 + strlen(stmtUserName)+1 + sizeof(int));
	    }

	    stmts++;
	    sm.addStmt(A_Stmt(stmtIsActive, stmtId, stmtUserName, stmtBufName, 
			      isDirectStream, displayText));
	    sms.erase(sms.begin()+index);
	    sms.insert(sms.begin()+index, sm);
	    rewriteSystemDeclares(DECLARE_TABLE_STMT);
	  }
	  /*else if(rc == 0) {
	    stmts++;
	    if(stmtType != QUERY_STMT)
	      sm.addStmt(A_Stmt(stmtIsActive, stmtId, stmtUserName));
	    else
	      sm.addStmt(A_Stmt(0, stmtId, stmtUserName));
	    sms.erase(sms.begin()+index);
	    sms.insert(sms.begin()+index, sm);
	  }*/
	  else if(rc != 0) {
	    printf("Error compiling retrieved query %s, continuing, if tables are defined using create table then this is ok, because they will be recompiled.\n", stmtId);
	    rc = 0;
	  }
	}
    }
  }
  printf("Retreived %d modules and %d stmts from persistent system table %s.\n"
	 , modules, stmts, tableName);
 exit:
  if(modulesTableC != NULL) {
    if((rc = modulesTableC->c_close(modulesTableC)) != 0) {
      printErrorMessage("readPersistentModulesDec, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(modulesTable != NULL && 
     (rc = modulesTable->close(modulesTable, 0)) != 0) {
    printErrorMessage("readPersistentModulesDec, close returned %d. "
		      "Persistence may not work correctly\n", rc);
  }

  return;
}
void Ios::readPersistentModules(char* tableName, vector<StmtModule>& sms) {
  DB* modulesTable;
  DBC* modulesTableC;
  int rc;
  DBT key, data;
  int moduleFlag;
  int isActive;
  char moduleName[128];
  char moduleId[128];
  char userName[128];
  int stmtType;
  int stmtIsActive;
  char stmtId[128];
  char stmtUserName[128];
  char stmtBufName[128];
  int isDirectStream;
  int modules = 0;
  int stmts = 0;
  int index = 0;
  StmtModule sm;
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buff = bm->lookup("_ioBuffer");

  if ((rc = db_create(&modulesTable, NULL, 0)) != 0) {
    printErrorMessage("readPersistentModules, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->set_pagesize(modulesTable, 2048)) != 0) {
    printErrorMessage("readPersistentModules, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }

  if ((rc = modulesTable->open(modulesTable, tableName, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("readPersistentModules, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = modulesTable->cursor(modulesTable, NULL, &modulesTableC, 0)) != 0) {
    printErrorMessage("readPersistentModules, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;

  memcpy(&moduleFlag, (char*)data.data, sizeof(int));
  memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
  strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
  strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(moduleName)+1);
  strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(moduleName)+1 + strlen(moduleId)+1);
  memcpy(&stmtType, (char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(moduleName)+1 + strlen(moduleId)+1 + strlen(userName)+1, 
	 sizeof(int));
  index = findModuleById(userName, sms, moduleId);
  if(index < 0) {
    vector<A_stmt> v;
    modules++;
    if(stmtType != QUERY_STMT)
      sm = StmtModule(isActive, moduleName, moduleId, v, userName, stmtType);
    else
      sm = StmtModule(0, moduleName, moduleId, v, userName, stmtType);
    sms.push_back(sm);
    index = sms.size() - 1;
  }
  else {
    sm = sms[index];
  }

  if(moduleFlag == STMT_FLAG) {
    int moduleLength = sizeof(int) + sizeof(int) + strlen(moduleName)+1 + 
      strlen(moduleId)+1 + strlen(userName)+1 + sizeof(int);
    memcpy(&stmtIsActive, (char*)data.data + moduleLength, sizeof(int));
    strcpy(stmtId, (char*)data.data + moduleLength + sizeof(int));
    strcpy(stmtUserName, (char*)data.data + moduleLength + sizeof(int) + 
	   strlen(stmtId)+1);
    if(stmtType == QUERY_STMT) {
      int code;
      int accessingPrivateUser = 0;
      char qName[4096];
      int gotMessage = 0;
      rc = 0;

      Ios::sendMessageToQueryBuf(SET_USER_NAME, stmtUserName);
      Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, stmtId);
      while(!gotMessage) {
	while(buff->empty())
	  usleep(100000);
	buff->get(code, (char*)qName);
	
	if(code == COMPILE_FAILURE) {
	  rc = 1;
	  buff->pop();
	  gotMessage = 1;
	}
	else if(code == COMPILE_SUCCESS) {
	  buff->pop();
	  gotMessage = 1;
	}
	else if(code == USES_LIBRARY) {
	  buff->pop();
	  if(!isUserPublic(qName)) {
	    accessingPrivateUser = 1;
	  }
	}
	else {
	  pollStdoutMessageBuffer();
	}
      }
      if(rc != 1 && accessingPrivateUser == 1) {
	char stdoutBuffer[MAX_STMT_PATH + 10];
	sprintf(stdoutBuffer, "stdout_%s", stmtId);
	
	buffer* stdoutBuf = bm->lookup(stdoutBuffer);
	Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmtId);
	if(stdoutBuf) {
	  dropStdoutBuf(userName, stdoutBuf);
	}
	rc = 1;
      }
    }
    if(rc == 0) {
      stmts++;
      if(stmtType != QUERY_STMT)
	sm.addStmt(A_Stmt(stmtIsActive, stmtId, stmtUserName));
      else
	sm.addStmt(A_Stmt(0, stmtId, stmtUserName));
      sms.erase(sms.begin()+index);
      sms.insert(sms.begin()+index, sm);
    }
    else if(rc != 0) {
      printf("Error compiling retrieved query %s, continuing\n", stmtId);
      rc = 0;
    }
  }
  
  while(rc == 0) {
    rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_NEXT);
    if(rc == 0) {

      memcpy(&moduleFlag, (char*)data.data, sizeof(int));
      memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
      strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
      strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1);
      strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1 + strlen(moduleId)+1);
      memcpy(&stmtType, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1 + strlen(moduleId)+1 + strlen(userName)+1, 
	     sizeof(int));
      index = findModuleById(userName, sms, moduleId);
      if(index < 0) {
	vector<A_stmt> v;
	modules++;
	if(stmtType != QUERY_STMT)
	  sm = StmtModule(isActive, moduleName, moduleId, v, userName, 
			  stmtType);
	else
	  sm = StmtModule(0, moduleName, moduleId, v, userName, stmtType);
	sms.push_back(sm);
        index = sms.size() - 1;
      }
      else {
	sm = sms[index];
      }
      if(moduleFlag == STMT_FLAG) {
	int moduleLength = sizeof(int) + sizeof(int) + strlen(moduleName)+1 
	  + strlen(moduleId)+1 + strlen(userName)+1 + sizeof(int);
	memcpy(&stmtIsActive, (char*)data.data + moduleLength, sizeof(int));
	strcpy(stmtId, (char*)data.data + moduleLength + sizeof(int));
	strcpy(stmtUserName, (char*)data.data + moduleLength + sizeof(int) + 
	       strlen(stmtId)+1);
	if(stmtType == QUERY_STMT) {
	  int code;
	  int accessingPrivateUser = 0;
	  char qName[4096];
	  int gotMessage = 0;
	  Ios::sendMessageToQueryBuf(SET_USER_NAME, stmtUserName);
	  Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, stmtId);
	  while(!gotMessage) {
	    while(buff->empty()) {
	      usleep(100000);
            }
	    buff->get(code, (char*)qName);
	    
	    if(code == COMPILE_FAILURE) {
	      rc = 1;
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == COMPILE_SUCCESS) {
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == USES_LIBRARY) {
	      buff->pop();
	      if(!isUserPublic(qName)) {
		accessingPrivateUser = 1;
	      }
	    }
	    else {
	      pollStdoutMessageBuffer();
	    }
	  }
	  if(rc != 1 && accessingPrivateUser == 1) {
	    char stdoutBuffer[MAX_STMT_PATH + 10];
	    sprintf(stdoutBuffer, "stdout_%s", stmtId);
	    
	    buffer* stdoutBuf = bm->lookup(stdoutBuffer);
	    Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmtId);
	    if(stdoutBuf) {
	      dropStdoutBuf(userName, stdoutBuf);
	    }
	    rc = 1;
	  }    
	}
	if(rc == 0) {
	  stmts++;
	  if(stmtType != QUERY_STMT)
	    sm.addStmt(A_Stmt(stmtIsActive, stmtId, stmtUserName));
          else
	    sm.addStmt(A_Stmt(0, stmtId, stmtUserName));
	  sms.erase(sms.begin()+index);
	  sms.insert(sms.begin()+index, sm);
	}
	else if(rc != 0) {
	  printf("Error compiling retrieved query %s, continuing\n", stmtId);
	  rc = 0;
	}
      }
    }
  }
  printf("Retreived %d modules and %d stmts from persistent system table %s.\n"
	 , modules, stmts, tableName);
 exit:
  if(modulesTableC != NULL) {
    if((rc = modulesTableC->c_close(modulesTableC)) != 0) {
      printErrorMessage("readPersistentModules, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(modulesTable != NULL && 
     (rc = modulesTable->close(modulesTable, 0)) != 0) {
    printErrorMessage("readPersistentModules, close returned %d. "
		      "Persistence may not work correctly\n", rc);
  }

  return;
}

void Ios::updateModuleIsActive(char* tableName, char* userName1, 
			       char* moduleId1, int isActive1) {
  DB* modulesTable;
  DBC* modulesTableC;
  int rc;
  DBT key, data;
  int moduleFlag;
  int isActive;
  char moduleName[128];
  char moduleId[128];
  char userName[128];

  if ((rc = db_create(&modulesTable, NULL, 0)) != 0) {
    printErrorMessage("updateModuleIsActive, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->set_pagesize(modulesTable, 2048)) != 0) {
    printErrorMessage("updateModuleIsActive, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->open(modulesTable, tableName, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("updateModuleIsActive, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = modulesTable->cursor(modulesTable, NULL, &modulesTableC, 0)) != 0) {
    printErrorMessage("updateModuleIsActive, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;

  memcpy(&moduleFlag, (char*)data.data, sizeof(int));
  if(moduleFlag == MODULE_FLAG) {
    memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
    strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
    strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	   strlen(moduleName)+1);
    if(strcmp(moduleId, moduleId1) == 0) {
      strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1 + strlen(moduleId)+1);
      if(strcmp(userName, userName1) == 0) {
        memcpy((char*)data.data + sizeof(int), &isActive1, sizeof(int));
	if((rc = modulesTableC->c_put(modulesTableC, &key, &data, 
				      DB_CURRENT)) != 0) {
	  printErrorMessage("updateModuleIsActive, cursor put returned %d. "
			    "Persistence may not work correctly\n", rc);
	}
	goto exit;
      }
    }
  }
  
  while(rc == 0) {
    rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_NEXT);
    if(rc == 0) {   
      memcpy(&moduleFlag, (char*)data.data, sizeof(int));
      if(moduleFlag == MODULE_FLAG) {
	memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
	strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
	strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	       strlen(moduleName)+1);
	if(strcmp(moduleId, moduleId1) == 0) {
	  strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
		 strlen(moduleName)+1 + strlen(moduleId)+1);
	  if(strcmp(userName, userName1) == 0) {
	    memcpy((char*)data.data + sizeof(int), &isActive1, sizeof(int));
	    if((rc = modulesTableC->c_put(modulesTableC, &key, &data, 
					  DB_CURRENT)) != 0) {
	      printErrorMessage("updateModuleIsActive, cursor put "
				"returned %d. "
				"Persistence may not work correctly\n", rc);
	    }
	    goto exit;
	  }
	}
      }
    }
  }
 exit:
  if(modulesTableC != NULL) {
    if((rc = modulesTableC->c_close(modulesTableC)) != 0) {
      printErrorMessage("updateModuleIsActive, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(modulesTable != NULL && 
     (rc = modulesTable->close(modulesTable, 0)) != 0) {
    printErrorMessage("updateModuleIsActive, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}

void Ios::updateStmtIsActive(char* tableName, char* userName1, char* qId1, 
			     int isActive1) {
  DB* modulesTable;
  DBC* modulesTableC;
  int rc;
  DBT key, data;
  int moduleFlag;
  int isActive;
  char moduleName[128];
  char moduleId[128];
  char userName[128];
  int stmtType;
  int stmtIsActive;
  char stmtId[128];
  if ((rc = db_create(&modulesTable, NULL, 0)) != 0) {
    printErrorMessage("updateStmtIsActive, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->set_pagesize(modulesTable, 2048)) != 0) {
    printErrorMessage("updateStmtIsActive, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->open(modulesTable, tableName, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("updateStmtIsActive, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = modulesTable->cursor(modulesTable, NULL, &modulesTableC, 0)) != 0) {
    printErrorMessage("updateStmtIsActive, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;

  memcpy(&moduleFlag, (char*)data.data, sizeof(int));
  if(moduleFlag != MODULE_FLAG) {
    memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
    strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
    strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	   strlen(moduleName)+1);
    strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	   strlen(moduleName)+1 + strlen(moduleId)+1);
    if(strcmp(userName, userName1) == 0) {
      memcpy(&stmtType, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1 + strlen(moduleId)+1 + strlen(userName)+1, 
	     sizeof(int));
      int moduleLength = sizeof(int) + sizeof(int) + strlen(moduleName)+1 + 
	strlen(moduleId)+1 + strlen(userName)+1 + sizeof(int);
      memcpy(&stmtIsActive, (char*)data.data + moduleLength, sizeof(int));
      strcpy(stmtId, (char*)data.data + moduleLength + sizeof(int));
      if(strcmp(stmtId, qId1) == 0) {
        memcpy((char*)data.data + moduleLength, &isActive1, sizeof(int));
	if((rc = modulesTableC->c_put(modulesTableC, &key, &data, 
				      DB_CURRENT)) != 0) {
	  printErrorMessage("updateStmtIsActive, cursor put returned %d. "
			    "Persistence may not work correctly\n", rc);
	}
	goto exit;
      }
    }
  }
  
  while(rc == 0) {
    rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_NEXT);
    if(rc == 0) {

      memcpy(&moduleFlag, (char*)data.data, sizeof(int));
      if(moduleFlag != MODULE_FLAG) {
	memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
	strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
	strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	       strlen(moduleName)+1);
	strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	       strlen(moduleName)+1 + strlen(moduleId)+1);
	if(strcmp(userName, userName1) == 0) {
	  memcpy(&stmtType, (char*)data.data + sizeof(int) + sizeof(int) + 
		 strlen(moduleName)+1 + strlen(moduleId)+1 + 
		 strlen(userName)+1, sizeof(int));
	  int moduleLength = sizeof(int) + sizeof(int) + strlen(moduleName)+1+ 
	    strlen(moduleId)+1 + strlen(userName)+1 + sizeof(int);
	  memcpy(&stmtIsActive, (char*)data.data + moduleLength, sizeof(int));
	  strcpy(stmtId, (char*)data.data + moduleLength + sizeof(int));
	  if(strcmp(stmtId, qId1) == 0) {
	    memcpy((char*)data.data + moduleLength, &isActive1, sizeof(int));
	    if((rc = modulesTableC->c_put(modulesTableC, &key, &data, 
					  DB_CURRENT)) != 0) {
	      printErrorMessage("updateStmtIsActive, cursor put returned %d. "
				"Persistence may not work correctly\n", rc);
	    }
	    goto exit;
	  }
	}
      }
    }
  }
 exit:
  if(modulesTableC != NULL) {
    if((rc = modulesTableC->c_close(modulesTableC)) != 0) {
      printErrorMessage("updateStmtIsActive, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(modulesTable != NULL && 
     (rc = modulesTable->close(modulesTable, 0)) != 0) {
    printErrorMessage("updateStmtIsActive, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}

void Ios::deleteStmtInModule(char* moduleId1, char* userName1, char* stmtId1,
			     char* tableName) {
  DB* modulesTable;
  DBC* modulesTableC;
  int rc;
  DBT key, data;
  int moduleFlag;
  int isActive;
  char moduleName[128];
  char moduleId[128];
  char userName[128];
  int stmtType;
  int stmtIsActive;
  char stmtId[128];
  char stmtUserName[128];


  if ((rc = db_create(&modulesTable, NULL, 0)) != 0) {
    printErrorMessage("deleteStmtInModule, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->set_pagesize(modulesTable, 2048)) != 0) {
    printErrorMessage("deleteStmtInModule, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->open(modulesTable, tableName, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("deleteStmtInModule, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = modulesTable->cursor(modulesTable, NULL, &modulesTableC, 0)) != 0) {
    printErrorMessage("deleteStmtInModule, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;

  memcpy(&moduleFlag, (char*)data.data, sizeof(int));
  if(moduleFlag == STMT_FLAG) {
    memcpy(&moduleFlag, (char*)data.data, sizeof(int));
    memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
    strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
    strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	   strlen(moduleName)+1);
    strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	   strlen(moduleName)+1 + strlen(moduleId)+1);
    memcpy(&stmtType, (char*)data.data + sizeof(int) + sizeof(int) + 
	   strlen(moduleName)+1 + strlen(moduleId)+1 + strlen(userName)+1, 
	   sizeof(int));
    int moduleLength = sizeof(int) + sizeof(int) + strlen(moduleName)+1 + 
      strlen(moduleId)+1 + strlen(userName)+1 + sizeof(int);
    memcpy(&stmtIsActive, (char*)data.data + moduleLength, sizeof(int));
    strcpy(stmtId, (char*)data.data + moduleLength + sizeof(int));
    strcpy(stmtUserName, (char*)data.data + moduleLength + sizeof(int) + 
	   strlen(stmtId)+1);
    if(strcmp(moduleId, moduleId1) == 0 && strcmp(userName, userName1) == 0 &&
       strcmp(stmtId, stmtId1) == 0 && strcmp(stmtUserName, userName1) == 0) {
      if((rc = modulesTableC->c_del(modulesTableC, 0)) != 0) {
	printErrorMessage("deleteStmtInModule, delete cursor returned %d. "
			  "Persistence may not work correctly\n", rc);
      }
      goto exit;
    }
  }
  while(rc == 0) {
    rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_NEXT);
    if(rc == 0) {
      memcpy(&moduleFlag, (char*)data.data, sizeof(int));
      if(moduleFlag == STMT_FLAG) {
	memcpy(&moduleFlag, (char*)data.data, sizeof(int));
	memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
	strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
	strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	   strlen(moduleName)+1);
	strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	       strlen(moduleName)+1 + strlen(moduleId)+1);
	memcpy(&stmtType, (char*)data.data + sizeof(int) + sizeof(int) + 
	       strlen(moduleName)+1 + strlen(moduleId)+1 + strlen(userName)+1, 
	       sizeof(int));
	int moduleLength = sizeof(int) + sizeof(int) + strlen(moduleName)+1 + 
	  strlen(moduleId)+1 + strlen(userName)+1 + sizeof(int);
	memcpy(&stmtIsActive, (char*)data.data + moduleLength, sizeof(int));
	strcpy(stmtId, (char*)data.data + moduleLength + sizeof(int));
	strcpy(stmtUserName, (char*)data.data + moduleLength + sizeof(int) + 
	       strlen(stmtId)+1);
	if(strcmp(moduleId, moduleId1) == 0 && 
	   strcmp(userName, userName1) == 0 &&
	   strcmp(stmtId, stmtId1) == 0 && 
	   strcmp(stmtUserName, userName1) == 0) {
	  if((rc = modulesTableC->c_del(modulesTableC, 0)) != 0) {
	    printErrorMessage("deleteStmtInModule, delete cursor returned %d. "
			      "Persistence may not work correctly\n", rc);
	  }
	  goto exit;
	}
      }
    }
  }
 exit:
  if(modulesTableC != NULL) {
    if((rc = modulesTableC->c_close(modulesTableC)) != 0) {
      printErrorMessage("deleteStmtInModule, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(modulesTable != NULL && 
     (rc = modulesTable->close(modulesTable, 0)) != 0) {
    printErrorMessage("deleteStmtInModule, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}

void Ios::deleteModule(char* moduleId1, char* userName1, char* tableName) {
  DB* modulesTable;
  DBC* modulesTableC;
  int rc;
  DBT key, data;
  int moduleFlag;
  int isActive;
  char moduleName[128];
  char moduleId[128];
  char userName[128];

  if ((rc = db_create(&modulesTable, NULL, 0)) != 0) {
    printErrorMessage("deleteModule, db_create returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->set_pagesize(modulesTable, 2048)) != 0) {
    printErrorMessage("deleteModule, set_pagesize returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if ((rc = modulesTable->open(modulesTable, tableName, 
			     NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
    printErrorMessage("deleteModule, open returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
  if((rc = modulesTable->cursor(modulesTable, NULL, &modulesTableC, 0)) != 0) {
    printErrorMessage("deleteModule, cursor returned %d. "
		      "Persistence may not work correctly\n", rc);
    goto exit;
  }

  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_FIRST);
  if(rc != 0)
    goto exit;

  memcpy(&moduleFlag, (char*)data.data, sizeof(int));
  memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
  strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
  strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(moduleName)+1);
  strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	 strlen(moduleName)+1 + strlen(moduleId)+1);
  if(strcmp(moduleId, moduleId1) == 0  && strcmp(userName, userName1) == 0) {
    if((rc = modulesTableC->c_del(modulesTableC, 0)) != 0) {
      printErrorMessage("deleteModule, delete cursor returned %d. "
			"Persistence may not work correctly\n", rc);
      rc = 0;
    }
  }

  while(rc == 0) {
    rc = modulesTableC->c_get(modulesTableC, &key, &data, DB_NEXT);
    if(rc == 0) {
      memcpy(&moduleFlag, (char*)data.data, sizeof(int));
      memcpy(&isActive, (char*)data.data + sizeof(int), sizeof(int));
      strcpy(moduleName, (char*)data.data + sizeof(int) + sizeof(int));
      strcpy(moduleId, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1);
      strcpy(userName, (char*)data.data + sizeof(int) + sizeof(int) + 
	     strlen(moduleName)+1 + strlen(moduleId)+1);
      if(strcmp(moduleId, moduleId1) == 0 && 
	 strcmp(userName, userName1) == 0) {
	if((rc = modulesTableC->c_del(modulesTableC, 0)) != 0) {
	  printErrorMessage("deleteModule, delete cursor returned %d. "
			    "Persistence may not work correctly\n", rc);
	  rc = 0;
	}
      }
    }
  }
 exit:
  if(modulesTableC != NULL) {
    if((rc = modulesTableC->c_close(modulesTableC)) != 0) {
      printErrorMessage("deleteModule, close cursor returned %d. "
			"Persistence may not work correctly\n", rc);
    }
  }
  if(modulesTable != NULL && 
     (rc = modulesTable->close(modulesTable, 0)) != 0) {
    printErrorMessage("deleteModule, close returned %d. "
		      "Persistence may not work correctly\n", rc);
    return;
  }
}


void Ios::run(int port)
{
  myPORT = port;
  int i = 0;
  A_iomodule iter = NULL;
  int ret = 0;
  int count = 0;
  int gotTuples;
  struct timeval tv;
  int size;
  bufferMngr* bm;
  buffer *buff;
  
  iosLog = (char*)malloc(11);
  sprintf(iosLog, "./IOS.log");

  stdoutLog = freopen(iosLog, "w", stdout);
  stderrLog = freopen(iosLog, "a", stderr);

  readPersistentUsers();
  readPersistentIOModules();
  fflush(stdout);

  //externs are handled very similar to aggregate
  //except view detail and defintion because we need two things the
  // decl and cc code file, thus I also put extern defs in the
  // aggr file and thus the rewriteAggregateDecs call below
  readPersistentModules(EXTERNS_TABLENAME, activeExternModules);
  rewriteAggregateDeclares();
  fflush(stdout);

  readPersistentDecModules(DECLARES_TABLENAME, activeDeclareModules, DECLARE_TABLE_STMT);
  rewriteSystemDeclares(DECLARE_TABLE_STMT);

  readPersistentModules(AGGREGATES_TABLENAME, activeAggregateModules);
  rewriteAggregateDeclares();
  fflush(stdout);

  //handled a lot like aggregates
  readPersistentModules(MODELS_TABLENAME, activeModelModules);
  rewriteModelDeclares();
  fflush(stdout);

  readPersistentDecModules(DECLARES_TABLENAME, activeDeclareModules, DECLARE_STREAM_STMT);
  rewriteSystemDeclares(DECLARE_STREAM_STMT);
  fflush(stdout);
  
  readPersistentModules(QUERIES_TABLENAME, activeQueryModules);
  fflush(stdout);

  readPersistentModules(TSQUERIES_TABLENAME, activeTSQueryModules);
  fflush(stdout);

   //ZZZ
   stdoutBuffers->push_back(A_StdoutBuf(strdup("__user__"), 
                            strdup(TOTAL_OUTPUT_TUPLE)));
   stdoutBuffers->push_back(A_StdoutBuf(strdup("__user__"), 
                            strdup(AVG_LATENCY)));
   stdoutBuffers->push_back(A_StdoutBuf(strdup("__user__"), 
                            strdup(MAX_LATENCY)));
   stdoutBuffers->push_back(A_StdoutBuf(strdup("__user__"), 
                            strdup(LAST_TOTAL_BUF_BYTES)));

  /*int temp = readCommand();
  if(temp != -1) {
    printf("StreamMill ready for user requests\n");
  }
  else {
    printf("StreamMill retrieved past definitions, but seems that the"
           " client request port is busy, either wait for a few moments"
           " or restart StreamMill.\n");
  }*/

  while(1)
  {
    gotTuples = 0;
    size = activeIOmodules.size();
    bm = bufferMngr::getInstance();
    buff = bm->lookup("_ioBuffer");

    //bm->displayBufNames();
    
    for(i = 0; i< size; )
    {
      iter = activeIOmodules.operator[](i);
      //printf("i = %d, isActive %d, buf %d\n", i, iter->isActive, iter->buf);
      //fflush(stdout);

      if(iter->isActive == 1 && iter->buf != NULL)
      {   
	//printf("calling getTuple\n");
	//fflush(stdout);
        ret =(*(iter->getTuple))(iter->buf, bm);
        if(iter->buf->empty() != 1) {
	  //printf("back from get tuple, isEmpty %d\n", iter->buf->empty());
	  //fflush(stdout);
	}

        switch(ret)
        {
          case GET_TUPLE_ERROR:
            printf("Error in iomodule %s, getTuple(), continuing\n", iter->buf->name);
            i++;
            break;
          case GET_TUPLE_GOT_DATA:
	    bm->new_arrival = 1;
            gotTuples = 1;
            i++;
          break;
          case GET_TUPLE_CLOSE_CONNECTION:
            (*(iter->closeConnection))();
            i++;
          break;
          case GET_TUPLE_NO_DATA:
            i++;
          break;
          default:
            printf("Error: unknown value returned from getTuple()\n");
        }
	//printf("after switch ret\n");
	//fflush(stdout);
      }
      else
        i++;
    }

    readCommand(); /*Check to see if client made any request, 
                     if so proecess it */
    //printf("here0\n");
    //fflush(stdout);
  
    //just clearing the iobuff incase there are leftover messages
    while(!buff->empty()) {
      buff->pop();
    }

    pollStdoutMessageBuffer();
    //printf("here1\n");
    //fflush(stdout);

    sendOutputToGUIClients();
    //printf("here2\n");
    //fflush(stdout);

    //if(size == 0)
    //  break;
    //simpleTest();
    count++;
    //if (count ==5) break;
    if(!gotTuples)
      usleep(DELAY_USECS);
    fflush(stdout);
    fflush(stderr);
  }
}


int Ios::initListenSock()
{
  int p;                /*general purpose    */
  int op;
  int on_reuse,ret;

  qListenSock.sin_family=AF_INET;
  qListenSock.sin_port=htons((unsigned short)myPORT);
  qListenSock.sin_addr.s_addr=INADDR_ANY;

  /* create socket, terminate in
     case of failure */
  if((qListenSockfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
    qListenSockfd = -1;
    printf("In initListenSock, Error calling socket()\n");
    return -1;
  }
  on_reuse = 0;
  if((ret = setsockopt( qListenSockfd, SOL_SOCKET, SO_REUSEADDR, &on_reuse, sizeof(on_reuse) )) == -1)
  {
	printf("BM Error, setsockopt..\n");
	return -1;
  }

  if(bind(qListenSockfd, (struct sockaddr *)&qListenSock, sizeof(qListenSock)))
  {
    qListenSockfd = -1;
    printf("In initListenSock, Error calling bind()\n");
    return -1;
  }

  /* make socket listening for
   connections, terminate in
   case of failure */
  if(listen(qListenSockfd, 1))
  {
    qListenSockfd = -1;
    printf("In initListenSock, Error calling listen()\n");
    return -1;
  }

  op = fcntl(qListenSockfd, F_GETFL, 0);
  if (op != -1) {
    op |= O_NONBLOCK;
    fcntl(qListenSockfd, F_SETFL, op);
  }

  p=sizeof(qListenSock);
  if((qReadSockfd=accept(qListenSockfd, (struct sockaddr *)&qListenSock, (socklen_t *)&p)) == -1)
  {
    qReadSockfd = -1;
    return 1;
  }

  op = fcntl(qReadSockfd, F_GETFL, 0);
  if (op != -1) {
    op |= O_NONBLOCK;
    fcntl(qReadSockfd, F_SETFL, op);
  }

  return 0;
}

void Ios::acceptConnection()
{
  int p, op;

  p = sizeof(qListenSock);
  if((qReadSockfd=accept(qListenSockfd, (struct sockaddr *)&qListenSock, (socklen_t *)&p)) == -1)
  {
    qReadSockfd = -1;
    return;
  }

  op = fcntl(qReadSockfd, F_GETFL, 0);
  if (op != -1) {
    op |= O_NONBLOCK;
    fcntl(qReadSockfd, F_SETFL, op);
  }

  return;
}

int Ios::addStdoutBuf(char* userName, const buffer* buf) {
  int size = stdoutBuffers->size();

  for(int i = 0; i < size; i++)
  {
    A_stdoutBuf sb = stdoutBuffers->operator[](i);
    if(strcmp(sb->bufName, buf->name) == 0)
    {
      return 0;
    }
  }

  stdoutBuffers->push_back(A_StdoutBuf(strdup(userName), strdup(buf->name)));
  return 0;
}

int Ios::dropStdoutBuf(char* userName, const buffer* buf) {
  int size = stdoutBuffers->size();

  for(int i = 0; i < size; i++)
  {
    A_stdoutBuf sb = stdoutBuffers->operator[](i);
    if(strcmp(sb->bufName, buf->name) == 0)
    {
      stdoutBuffers->erase(stdoutBuffers->begin() + i);
      free(sb);
      return 0;
    }
  }
  return 0;
}

//**************************
//* writeDataToClient
//**************************
/*
void writeDataToClient(vector<char*>* dataTuples, GUIClient* gc) {
  int sockfd, portno, n, rc;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  int dtSize = dataTuples->size();

  //printf("%d size\n", dtSize );

  //if there is no data then do nothing
  if(dtSize <= 0) return;

  portno = gc->getPort();
  
  // Tree: changed
  // sockfd = socket(AF_INET, SOCK_STREAM, 0);
  sockfd = socket(AF_INET,SOCK_DGRAM, 0);

  if(sockfd < 0)
  {
    printf("In writeDataToClient, Could not create socket.\n");
    return;
  }

  server = gethostbyname(gc->getClientIp());
  
  if(server == NULL)
  {
    printf("In writeDataToClient, Could not gethostbyname for %s\n",
gc->getClientIp());
    return;
  }

  //setting up the server side information
  bzero((char*)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;

  bcopy( (char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
         
  serv_addr.sin_port = htons(portno);

  //printf("Pass 1\n");

  //if (connect(sockfd,(struct sockaddr*)(&serv_addr),sizeof(serv_addr)) < 0)
  //{
  //  printf("In writeDataToClient, Could not connect %s, port %d\n",
  // gc->getClientIp(), gc->getPort());
  //   return;
  //}

  for(int k = 0; k < dtSize; k++)
  {
    char* data = dataTuples->operator[](k);
    
    //printf("sending to client %s\n", data);
    //n = write(sockfd, data, strlen(data));
    //if (n < 0)
    //{
     // printf("In writeDataToClient, Could not write\n");
      //return;
    //}
    //printf("writing %s\n", data);
    rc=sendto(sockfd, data, strlen(data), 0, 
              (struct sockaddr *)&serv_addr, sizeof(serv_addr));
                
    if(rc<0) {
      printf("cannot send data \n");
      close(sockfd);
      return;
    }

    // Tree: Testing purpose
    // printf("Data send to %s:%d w/ %s\n", gc->getClientIp(),
    // gc->getPort(), dataTuples->operator[](k) );
                
  }
  
  close(sockfd);
}
*/


void writeDataToClient(vector<char*>* dataTuples, GUIClient* gc)
{
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  
  int dtSize = dataTuples->size();

  if(dtSize <= 0) return;
  
  portno = gc->getPort();
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if(sockfd < 0)
  {
    printf("In writeDataToClient, Could not create socket.\n");
    return;
  }

  server = gethostbyname(gc->getClientIp());
  if(server == NULL)
  {
    printf("In writeDataToClient, Could not gethostbyname for %s\n", gc->getClientIp());
    return;
  }

  bzero((char*)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;

  bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
  serv_addr.sin_port = htons(portno);

  //int dtSize = dataTuples->size();
  if (connect(sockfd,(struct sockaddr*)(&serv_addr),sizeof(serv_addr)) < 0) 
  {
    printf("In writeDataToClient, Could not connect %s, port %d\n", gc->getClientIp(), gc->getPort());
    fflush(stdout);  
    return;
  }
  //int dtSize = dataTuples->size();
  for(int k = 0; k < dtSize; k++)
  {
    char* data = dataTuples->operator[](k);
    n = write(sockfd, data, strlen(data));
    if (n < 0) 
    {
      printf("In writeDataToClient, Could not write\n");
      return;
    }
  }
  close(sockfd);
}

void Ios::sendOutputToGUIClients()
{
  bufferMngr* bm = bufferMngr::getInstance();
  int size = stdoutBuffers->size();
  struct timeval tv;
  cDBT cdbt(4096, &tv);

  for(int i = 0; i < size; i++)
  {
    char* bufName = (stdoutBuffers->operator[](i))->bufName;
    buffer* buf = bm->lookup(bufName);
    //printf("buf1 %s %d, %d\n", bufName, i, size);fflush(stdout);
    if(buf) {
      //printf("buf %s %d, %d, %d\n", bufName, i, size, buf->empty());fflush(stdout);
      vector<char*> dataTuples;
      int rc = 0;
      while(!(buf->empty()) && rc == 0) {
        printf("before malloc\n");fflush(stdout);
	char *data  = (char*)malloc(4096);
        //printf("after malloc\n");fflush(stdout);
	rc = buf->get(&cdbt);
        //printf("after get\n");fflush(stdout);
	if(rc == 0) {
	  buf->pop();
	  strcpy(data, (char*)cdbt.data);
          memset(cdbt.data, 0, 4096);
          dataTuples.push_back(data);
	}
      }
    //printf("buf1 %s %d, %d\n", bufName, i, size);fflush(stdout);
      
      if((stdoutBuffers->operator[](i))->type == sbt_simple) {
	vector<GUIClient*> v;
	buf->getClients(v);
	int clSize = v.size();
	int j;
        //printf("buf name %s, clSize %d, %d dtSize\n", bufName, clSize, dataTuples.size()); fflush(stdout);
	for(j = 0; j < clSize; j++) {
	  GUIClient* gc = v.operator[](j);
	  writeDataToClient(&dataTuples, gc);
	}
      }
      else {
	GUIClient* gc = new GUIClient(strdup((stdoutBuffers->operator[](i))->machine), 
				      (stdoutBuffers->operator[](i))->portNum);
	writeDataToClient(&dataTuples, gc);
	free(gc);
      }
      int dtSize = dataTuples.size();
      for(int k=0; k<dtSize; k++) {
	char* dt = dataTuples.operator[](k);
	free(dt);
      }
    }
  }
}

void Ios::pollStdoutMessageBuffer()
{
  bufferMngr* bm = bufferMngr::getInstance();
  int messageSize = sizeof(int) + MAX_ID_LEN;
  int code;
  char id[2048];
  char userName[40];
  cDBT cdbt(MAX_STR_LEN);

  userName[0] = '\0';
  stdoutMessageBuffer = bm->lookup("_ioBuffer");

  
  //printf("pollStd... here %d\n", stdoutMessageBuffer->empty());
  //fflush(stdout);
  if(!(stdoutMessageBuffer->empty())) {
    //printf("not empty\n");
    //fflush(stdout);
    int rc = stdoutMessageBuffer->get(&cdbt);
    if (rc == 0){
      //printf("calling memcpy\n");
      //fflush(stdout);
      memcpy(&code, cdbt.data, sizeof(int));
      //printf("calling pop()\n");
      //fflush(stdout);
      stdoutMessageBuffer->pop();
      //printf("back from pop\n");
      //fflush(stdout);
    } else fprintf(stderr, "ERROR: could not read stdoutBuffer");;

    //printf("here %d, %s\n", code, id);
    //fflush(stdout);

    if(code == ADD_STDOUT_COMMAND)
    {
      strcpy(id, (char*)cdbt.data+sizeof(int));
      buffer* buf = bm->lookup(id);
      if(!buf)
        return;
      strcpy(userName, (char*)cdbt.data+sizeof(int)+strlen(id)+1);
      addStdoutBuf(userName, buf);
    }
    else if(code == DROP_STDOUT_COMMAND)
    {
      strcpy(id, (char*)cdbt.data+sizeof(int));
      buffer* buf = bm->lookup(id);
      if(!buf)
        return;
      strcpy(userName, (char*)cdbt.data+sizeof(int)+strlen(id)+1);
      dropStdoutBuf(userName, buf);
    }
    else if(code == CONNECT_IOMODULE)
    {
      strcpy(id, (char*)cdbt.data+sizeof(int));
      buffer* buf = bm->lookup(id);
      if(!buf)
        return;
      char modId[messageSize];
      strcpy(modId, (char*)cdbt.data+sizeof(int)+strlen(id)+1);
      updateIOModuleBuffer(modId, id);
    }
    else if(code == ADD_TARGET_STREAM) {
      char machine[256];
      int portNum;
      strcpy(id, (char*)cdbt.data+sizeof(int));
      buffer* buf = bm->lookup(id);
      //printf("got add target stream %s\n", id);
      //fflush(stdout);
      if(!buf)
        return;
      strcpy(machine, (char*)cdbt.data+sizeof(int)+strlen(id)+1);
      memcpy(&portNum,(char*)cdbt.data+sizeof(int)+strlen(id)+1+strlen(machine)+1, sizeof(int));
      strcpy(userName, (char*)cdbt.data+sizeof(int)+strlen(id)+1+strlen(machine)+1+sizeof(int));
      int size = stdoutBuffers->size();
      
      for(int i = 0; i < size; i++) {
	A_stdoutBuf sb = stdoutBuffers->operator[](i);
	if(strcmp(sb->bufName, buf->name) == 0) {
	  sb->machine = strdup(machine);
	  sb->portNum = portNum;
	  return;
	}
      }
      //assign type to this stdout buffer
      //aslo assign it "client" based on machine and portNum
      stdoutBuffers->push_back(A_StdoutBuf(strdup(userName), strdup(buf->name), 
					   sbt_target, strdup(machine), portNum));
      //printf("adding out buffer %s, %s, %d\n", id, machine, portNum);
      //fflush(stdout);
      
    }
  }

  //printf("pollStd-end... here\n");
  //fflush(stdout);
}

void StmtModule::removeStmtAtIndex(int index)
{
  int size = stmtIds.size();
 
  if(size <= index)
    return;

  stmtIds.erase(stmtIds.begin()+index);
}


vector<char*> splitStmts(char* buf)
{
  vector<char*> stmts;
  char* stmt;
  char* next = index(buf, ';');

  while(buf < next)
  {
    stmt = strndup(buf, next-buf+1);
    buf = next + 1;
    stmts.push_back(stmt);
    next = index(buf, ';');
  }

  return stmts;
}

char* strAppend(char* stmt, char* stmtPart)
{
  char* ret = (char*) malloc(MAX_STMT_LEN);

  sprintf(ret, "%s%s", stmt, stmtPart);

  return ret;
}

int getQuoteCount(char* stmt)
{
  int length = strlen(stmt);
  int count = 0;
  
  for(int i = 0; i< length; i++)
  {
    if(stmt[i] == '"')
      count = (count == 0)?1:0;
  }

  return count;
}

vector<char*> parseStmts(char* buf)
{
  vector<char*> sStmts = splitStmts(buf);
  int size = sStmts.size();
  vector<char*> ret;

  for(int i = 0; i < size; i++)
  {
    char* stmt = sStmts.operator[](i);
    if(getQuoteCount(stmt) == 0)
    {
      ret.push_back(stmt);
    }
    else
    {
      int count = 1;
      while(count == 1)
      {
        i++;
        char* stmtPart = sStmts.operator[](i);
        count = count + getQuoteCount(stmtPart);
        stmt = strAppend(stmt, stmtPart);
      }
      ret.push_back(stmt);
    }
  }

  return ret;
}

void Ios::sendObjectAlreadyExistsMessage(int qTempSockfd, char* objName, 
					 char* id) {
  char message[300];

  sprintf(message,
	  "%s by name %s already exists. Please try a different name.\n",
	  objName, id);

  write(qTempSockfd, message, strlen(message));
}

bool Ios::doesModuleExist(char* name, vector<StmtModule> moduleV) {
  int size = moduleV.size();

  for(int i = 0; i < size; i++)
  {
    StmtModule m = moduleV.operator[](i);
    if(strcmp(m.getModuleId(), name) == 0)
      return true;
  }
  return false;

}

void Ios::sendMessageToQueryBuf(int code, char* id)
{
  int messageSize = sizeof(int) + MAX_ID_LEN;
  cDBT cdbt(messageSize);

  memset(cdbt.data, '\0', messageSize);
  memcpy(cdbt.data, &code, sizeof(int));
  strcpy((char*)cdbt.data+sizeof(int), id);

  qBuf->put(&cdbt);
}
int Ios::getMessageFromQueryBuf(int &code, char* id)
{

    int messageSize = sizeof(int) + MAX_ID_LEN;
    cDBT cdbt(messageSize);
    int rc = qBuf->get(&cdbt);
    if (rc == 0){
      memcpy(&code, cdbt.data, sizeof(int));
      //memcpy(id, cdbt.data, strlen(id));
      strcpy(id, (char*)cdbt.data+sizeof(int));
      qBuf->pop();
    }
    return rc;

}

/*
void getToken(char* str, char*& tok)
{
  int separator = 0;
  int i = 0;
  tok = (char*) malloc(100);

  for(i; i < strlen(str); i++) {
    if(str[i] >= 'A')
      break;
    else
      tok[i] = str[i];
  }

  for(i; i < strlen(str); i++) {
    if(str[i] < 'A') {
      tok[i] = str[i];
      separator = 1;
    }
    else if(str[i] >= 'A' && separator == 1) {
      tok[i] = '\0';
      break;
    }
    else
      tok[i] = str[i];
  }
}
*/

int getToken(char* str, char*& tok)
{
  int separator = 0;
  int i = 0, j = 0;
  tok = (char*) malloc(100);

  for(i; i < strlen(str); i++) {
    if(str[i] >= 'A' && str[i] < 'z')
      break;
  }

  for(i; i < strlen(str); i++) {
    if(str[i] >= 'A' && str[i] < 'z') {
      tok[j] = str[i];
      j++;
      separator = 1;
    }
    else if(str[i] > 'z' || str[i] < 'A') {
      tok[j] = '\0';
      break;
    }
  }
  return i;
}


int Ios::isUserPublic(char* userName) {
  int size = users.size();
  
  for(int i = 0; i < size; i++) {
    A_user u = users[i];
    if(strcmp(userName, u->userName) ==0)
      return u->isPublic;
  }
  return 0;
}

int Ios::prependUserName(char* userName, char* stmt, char*& stmtPre)
{
  char* tmpStmt = strdup(stmt);
  char* tmpStmt1 = strdup(stmt);
  char* first;
  char* second;
  char* create = "create";
  int len = 0;

  len = getToken(tmpStmt, first);
  tmpStmt = tmpStmt + len;
  tmpStmt1 = tmpStmt1 + len;

  if(strstr(strlwr(first), create) != NULL) {
    char* temp;
    len = getToken(tmpStmt, temp);
    tmpStmt = tmpStmt + len;
    tmpStmt1 = tmpStmt1 + len;
    strcat(first, " ");
    strcat(first, temp);
    free(temp);
  }  
  len = getToken(tmpStmt, second);
  tmpStmt = tmpStmt + len;
  tmpStmt1 = tmpStmt1 + len;

  char* colon = index(second, '$');
  if(colon == NULL) {
    char* source = strstr(strlwr(tmpStmt1), "source");

    if(source == NULL || strstr(strlwr(tmpStmt1), "port") != NULL
                      || strstr(strlwr(tmpStmt1), "table") != NULL) {
      //stmt =(char*)malloc(strlen(first)+1+strlen(userName)+1+1+strlen(second)+
	//		   strlen(tmpStmt));
      sprintf(stmtPre, "%s %s$%s %s", first, userName, second, tmpStmt);
      return 0;

    }
    else {
      //printf("here source %d, tmpStmt1 %d\n", source, tmpStmt1);
      tmpStmt[(source-tmpStmt1)] = '\0';
      source = source + 6;
    
      //printf("source %s tmpStmt %s\n", source, tmpStmt);

      //stmt = (char*)malloc(strlen(first)+1 + strlen(userName)+1 +
      //			   strlen(second)+1 + strlen(source)+1 + 
      //			   strlen(userName)+1 + strlen(tmpStmt)+ 8);
      //memset(stmt, 0, strlen(first)+1 + strlen(userName)+1 +
	//     strlen(second)+1 + strlen(source)+1 + 
	  //   strlen(userName)+1 + strlen(tmpStmt)+ 8);
      char* singleQt = strsep(&source, "\'");

      //printf("here making stmt 1.%s 2.%s 3.%s 4.%s 5.%s 6.%s\n", first, userName, 
      //     second, tmpStmt, userName, source);
    //fflush(stdout);

      sprintf(stmtPre, "%s %s$%s%s source \'%s$%s", first, userName, second,
	      tmpStmt, userName, source);
      return 0;
    }
  }
  else {
    char* user = strsep(&second, "$");
    if(user == NULL)
      return 1;
    if(strcmp(user, userName) == 0)
      return 0;
    if(isUserPublic(user) == false)
      return 1;
  }
  free(first);
  free(second);
}

int Ios::prependUserNameForModel(char* userName, char* stmt, 
				char* moduleName, char*& stmtPre) {
  char* tmpStmt = strdup(stmt);
  char* first = NULL;
  //char* second = NULL;
  char* third = NULL;
  char* forModName;
  int rc = 0;
  int len = 0;

  len = getToken(tmpStmt, first);
  tmpStmt = tmpStmt + len;
  //fflush(stdout);
  //printf("in prepend\n");
  //fflush(stdout);

  //len = getToken(tmpStmt, second);
  //tmpStmt = tmpStmt + len;
  len = getToken(tmpStmt, third);
  printf("we are here third %s-\n", third);
  tmpStmt = tmpStmt + len;
  forModName = strdup(third);
  char* colon = index(third, '$');
  if(colon == NULL) {
    sprintf(moduleName, "%s__%s", userName, strsep(&forModName, "{"));
    //stmt = (char*)malloc(strlen(first)+1+strlen(second)+1+
	//	     strlen(userName)+1+
	//	     1+strlen(third)+strlen(tmpStmt));
    sprintf(stmtPre, "%s %s$%s %s", first, userName, third, tmpStmt);
  }
  else {
    sprintf(moduleName, "%s", third);
    //stmt = (char*)malloc(strlen(first)+1  +
	//	     strlen(third)+1+ 
	//	     strlen(tmpStmt)+ 8);
    sprintf(stmtPre, "%s %s %s", first, third,
		tmpStmt);
  }
 exit:
  if(first)
    free(first);
  //if(second)
  //  free(second);
  if(third)
    free(third);
  return rc;
}


int Ios::prependUserNameForAggr(char* userName, char* stmt, 
				char* moduleName, int &windowed,
                                char*& stmtPre) {
  char* tmpStmt = strdup(stmt);
  char* first = NULL;
  char* second = NULL;
  char* third = NULL;
  char* forModName;
  int rc = 0;
  int len = 0;

  len = getToken(tmpStmt, first);
  tmpStmt = tmpStmt + len;
  //fflush(stdout);
  //printf("in prepend\n");
  //fflush(stdout);
  if(strcasecmp(first, "window") == 0) {
    windowed = 1;
    len = getToken(tmpStmt, second);
    tmpStmt = tmpStmt + len;
    len = getToken(tmpStmt, third);
    tmpStmt = tmpStmt + len;
  }
  else {
    windowed = 0;
    len = getToken(tmpStmt, third);
    tmpStmt = tmpStmt + len;
    forModName = strdup(third);
    sprintf(moduleName, "%s__%s", userName, strsep(&forModName, "("));
  }
  //fflush(stdout);
  char* colon = index(third, '$');
  if(colon == NULL) {
    if(windowed) {
      forModName = strdup(third);
      sprintf(moduleName, "%s__%s_window", userName, strsep(&forModName, " ("));
    }
    else {
      forModName = strdup(third);
      sprintf(moduleName, "%s__%s", userName, strsep(&forModName, " ("));
    }

    char* source = strstr(tmpStmt, "source");
    //fflush(stdout);
    if(source == NULL) {
      if(windowed) {
	//fflush(stdout);
	//stmt = (char*)malloc(strlen(first)+1+strlen(second)+1+
	//		     strlen(userName)+1+
	//		     1+strlen(third)+strlen(tmpStmt));
	sprintf(stmtPre, "%s %s %s$%s %s", first, second, userName, third, tmpStmt);
      }
      else {
	//fflush(stdout);
	//stmt = (char*)malloc(strlen(first)+1+strlen(userName)+1+
	//		     1+strlen(third)+strlen(tmpStmt));
	sprintf(stmtPre, "%s %s$%s %s", first, userName, third, tmpStmt);
      }
      rc = 0;
      goto exit;
    }
    else {
      tmpStmt[(source-tmpStmt)] = '\0';
      source = source + 6;
      if(windowed) {
	//stmt = (char*)malloc(strlen(first)+1 + strlen(second)+1+
	//		     strlen(userName)+1 +
	//		     strlen(third)+1 + strlen(source)+1 + 
	//		     strlen(userName)+1 + strlen(tmpStmt)+ 8);
	//memset(stmt, 0, strlen(first)+1 + strlen(second)+1+
	//       strlen(userName)+1 +
	//       strlen(third)+1 + strlen(source)+1 + 
	//     strlen(userName)+1 + strlen(tmpStmt)+ 8);
	char* singleQt = strsep(&source, "\'");
	sprintf(stmtPre, "%s%s%s$%s%s source \'%s$%s", first, second, 
		userName, third, tmpStmt, userName, source);
      }
      else {
	//stmt = (char*)malloc(strlen(first)+1 + strlen(userName)+1 +
	//		     strlen(third)+1 + strlen(source)+1 + 
	//		     strlen(userName)+1 + strlen(tmpStmt)+ 8);
	//memset(stmt, 0, strlen(first)+1 + strlen(userName)+1 +
	//       strlen(third)+1 + strlen(source)+1 + 
	//     strlen(userName)+1 + strlen(tmpStmt)+ 8);
	char* singleQt = strsep(&source, "\'");
	sprintf(stmtPre, "%s%s$%s%s source \'%s$%s", first, userName, third,
		tmpStmt, userName, source);
      }
      rc = 0;
      goto exit;
    }
  }
  else {
    char* user = strsep(&second, "$");
    if(user == NULL) {
      rc = 1;
      goto exit;
    }
    if(strcmp(user, userName) == 0) {
      rc = 0;
      if(windowed) {
	forModName = strdup(third);
	sprintf(moduleName, "%s__%s_window", userName, strsep(&forModName, "$"));
      }
      else {
	forModName = strdup(third);
	sprintf(moduleName, "%s__%s", userName, strsep(&forModName, "$"));
      }

      goto exit;
    }
    if(isUserPublic(user) == false) {
      rc = 1;
      goto exit;
    } 
  }
 exit:
  if(first)
    free(first);
  if(second)
    free(second);
  if(third)
    free(third);
  return rc;
}

void Ios::addViewDecModule(char* userName, char* name, char* buf) {
  char fName[MAX_STMT_PATH];
  vector<A_stmt> decs;
  FILE* fdesc;
  
  fName[0] = '\0';
      
  sprintf(fName, "%s%s%s", DECLARE_FILE_PATH, name, DECLARE_FILE_EXT);
  
  fdesc = fopen(fName, "w");
  fprintf(fdesc, "%s", buf);
  fclose(fdesc);
      
  decs.push_back(A_Stmt(0, name, userName));
  activeDeclareModules.push_back(StmtModule(0,name, name, decs, userName, 
		       DECLARE_TABLE_STMT, 1));
  appendDeclaresToSystemFile(decs, DECLARE_TABLE_STMT);
}

void Ios::removeViewDecModule(char* userName, char* name) {
  int index = findModuleById(userName, activeDeclareModules, name);

  if(index == -1)
    return;
  
  int rc = removeDeclareModuleByIndex(index);
  
  if(rc == 0)
  {
    rewriteSystemDeclares(DECLARE_TABLE_STMT);
    rewriteSystemDeclares(DECLARE_STREAM_STMT);
  }
}

int Ios::generateStatements(char* userName, char* buf, char* clientIpAddr,
                            char* path, char* ext, int stmtType,
                            vector<A_stmt> *ret, char*& errors)
{
  int rc = 0;
  struct timeval tv;
  struct timezone tz;
  vector<char*> stmts = parseStmts(buf);
  FILE *fdesc;
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buff = bm->lookup("_ioBuffer");

  int size = stmts.size();
  for(int i = 0; i < size && rc == 0; i++)
  {
    char* stmt = stmts.operator[](i);
    char* stmtPre = (char*)malloc(strlen(stmt)+100);
    if(stmtType == DECLARE_TABLE_STMT || stmtType == DECLARE_STREAM_STMT) {
      rc = prependUserName(userName, stmt, stmtPre);
    } 
    else {
      sprintf(stmtPre, "%s", stmt);
    }
    if(rc == 0) {
      char sName[MAX_ID_LEN];
      char fName[MAX_STMT_PATH];
      char bufName[MAX_ID_LEN];
      int gotStreamCmd = 0;
      bufName[0] = '\0';
      sName[0] = '\0';
      fName[0] = '\0';
      
      gettimeofday(&tv, &tz);

      sprintf(sName, "%s_%d", clientIpAddr, tv.tv_usec);
      sprintf(fName, "%s%s%s", path, sName, ext);
      
      fdesc = fopen(fName, "w");
      fprintf(fdesc, "%s\n", stmtPre);
      fclose(fdesc);
      if(stmtType == DECLARE_TABLE_STMT) {
	//printf("StmtType %d\n", stmtType);
	int code;
	char qName[4096];
	int gotAddedBuffer = 0;
	int messageCt = 0; //look for message ADDED_BUFFER and COMPILE_FAILURE/SUCCESS
	
	appendDeclaresToSystemFile(*ret, stmtType);
	char* temp = strdup(stmtPre);

        strlwr(temp);   //defined above
        if(strstr(temp, " select ") == NULL && strstr(temp, "(select ") == NULL) {
	  //also append the aggregate definitions first
	  //appendAggregatesToSystemFile(stmtType);
	  appendDeclareToSystemFile(fName, stmtType);
	  Ios::sendMessageToQueryBuf(ADD_DECLARE_CMD_CODE, sName);
	  while(messageCt <= 1) {
	    while(buff->empty())
	      usleep(100000);
	    buff->get(code, (char*)qName);
	    if(code == COMPILE_FAILURE) {
	      rc = 1;
	      char* err;
	      int rc = readFileIntoBuffer("./errorsFile", err);
	      if(rc) {
		sprintf(errors, "Error occured in compiling declare %d, unable to retrieve error.\n", i);
	      }
	      else {
		sprintf(errors, "Error in declare %d: %s\n", i, err);
		free(err);
	      }
	      printf("ERROR: %s\n", errors);
	      messageCt = 2;
	      buff->pop();
	    }
	    else if(code == COMPILE_SUCCESS) {
	      messageCt = 2;
	      buff->pop();
	    }
            else if(code == ADDED_BUFFER) {
              strcpy(bufName, qName);
              Ios::sendMessageToQueryBuf(ADDED_BUFFER, bufName);
              gotAddedBuffer = 1;
              messageCt = 1;
              buff->pop();
            }
	    else {
	      pollStdoutMessageBuffer();
	    }
	  }
	  if(gotAddedBuffer == 0) {
	    Ios::sendMessageToQueryBuf((rc==0)?COMPILE_SUCCESS:COMPILE_FAILURE, "");
	  }
	}
	else {
	  int code;
	  int accessingPrivateUser = 0;
	  char qfName[80];
	  char dec[4096];
	  int gotMessage = 0;

	  //put create view in ../cq/sName.cq file.
	  sprintf(qfName, "%s%s%s", QUERY_FILE_PATH, sName, QUERY_FILE_EXT);

	  fdesc = fopen(qfName, "w");
	  fprintf(fdesc, "%s", stmtPre);
	  fclose(fdesc);
	  
	  Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, sName);
	  while(!gotMessage) {
	    while(buff->empty())
	      usleep(100000);
	    buff->get(code, (char*)qName, (char*)dec);
	    
	    if(code == COMPILE_FAILURE) {
	      rc = 1;
	      char* err = NULL;
	      int my_rc = readFileIntoBuffer("./errorsFile", err);
	      if(my_rc) {
		sprintf(errors, "Error occured in compiling statement %d, unable to retrieve error.\n", i);
	      }
	      else {
		sprintf(errors, "Error in statement %d: %s\n", i, strdup(err));
	      }
	      printf("ERROR: %s\n", errors);
	      fflush(stdout);
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == ADD_TABLE_DEC) {
	      sprintf(bufName, "%s", qName);
	      
	      //put qName(which is table dec) in the fName dcl file
	      //printf("writing %s to file %s\n", dec, fName);
	      //fflush(stdout);
	      //printf("got dec %s\n");
	      fdesc = fopen(fName, "w");
	      fprintf(fdesc, "%s", dec);
	      fclose(fdesc);

	      //set variable indicating that display text is set
	      gotStreamCmd = 1;
	      buff->pop();
	    }
	    else if(code == COMPILE_SUCCESS) {
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == USES_LIBRARY) {
	      buff->pop();
	      if(!isUserPublic(qName)) {
		accessingPrivateUser = 1;
	      }
	    }
	    else {
	      pollStdoutMessageBuffer();
	    }
	  }
	  if(gotStreamCmd == 1) {
	    int code;
	    char qName[4096];
	    int gotMessage = 0;
            
	    sendMessageToQueryBuf(ACTIVATE_QUERY_CMD_CODE, sName);
	    while(!gotMessage) {
	      while(buff->empty()) {
		usleep(100000);
	      }
	      buff->get(code, (char*)qName);
	      //printf("here code %d\n", code);
	      if(code == COMMAND_FAILED) {
		rc = 1;
		buff->pop();
		gotMessage = 1;
	      }
	      else if(code == COMMAND_SUCCESSFUL) {
		buff->pop();
		//updateStmtIsActive(QUERIES_TABLENAME, userName, queryId, 1);
		gotMessage = 1;
	      }
	      else if(code == ACTIVATED_QUERY) {
		buff->pop();
		A_stmt stmt = findStmtById(activeQueryModules, qName);
		if(stmt != NULL && stmt->isActive != 1) {
		  stmt->isActive = 1;
		  //updateStmtIsActive(QUERIES_TABLENAME, userName, qName, 1);
		}
		else if(stmt == NULL) {
		  stmt = findStmtById(activeDeclareModules, qName);
		  if(stmt != NULL && stmt->isDirectStream == 1 
		     && stmt->isActive != 1) {
		    stmt->isActive = 1;
		    //updateStmtIsActive(QUERIES_TABLENAME, userName, qName1, 1);
		  }
		}
	      }
	      else {
		  pollStdoutMessageBuffer();
	      }
	    }
	  }
	  if(rc != 1 && accessingPrivateUser == 1) {
	    char stdoutBuffer[MAX_STMT_PATH + 10];
	    sprintf(stdoutBuffer, "stdout_%s", sName);
	    	    
	    buffer* stdoutBuf = bm->lookup(stdoutBuffer);
	    Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, sName);
	    //TODO remove .so and .cc files as well
	    remove(fName);
	    //free(stmt);
	    if(stdoutBuf) {
	      dropStdoutBuf(userName, stdoutBuf);
	    }
	    rc = 1;
	    sprintf(errors, "Error compiling statement %d, "
		  "You attempted using object(s) of a private user.\n", i);
	  }
	  else if(rc != 1 && gotStreamCmd != 1) {
	    Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, sName);
	    //TODO remove .so and .cc files as well
	    remove(qfName);
	    remove(fName);
	    //free(stmt);
	    rc = 1;
	    sprintf(errors, "Error compiling statement %d, "
		  "Invalid create table statement is used.\n", i);
	  }
	}
	rewriteSystemDeclares(stmtType);
      }
      else if(stmtType == DECLARE_STREAM_STMT) {
	int code;
	char qName[4096];
	int messageCt = 0; //look for message ADDED_BUFFER and COMPILE_FAILURE/SUCCESS
	int gotAddedBuffer = 0;
	
	appendDeclaresToSystemFile(*ret, stmtType);
        char* temp = strdup(stmtPre);

        strlwr(temp);   //defined above
        if(strstr(temp, " select ") == NULL 
           && strstr(temp, "(select ") == NULL
           && strstr(temp, " run ") == NULL
           && strstr(temp, "(run ") == NULL) {
	  appendDeclareToSystemFile(fName, stmtType);
	  Ios::sendMessageToQueryBuf(ADD_DECLARE_CMD_CODE, sName);
	  while(messageCt <= 1) {
	    while(buff->empty()) {
	      usleep(100000);
	    }
	    buff->get(code, (char*)qName);

	    //printf("message in buf %d %s %d\n", code, qName, ADD_BUILTIN_IOMOD);
	    //fflush(stdout);
	    if(code == COMMAND_FAILED) {
	      rc = 1;
	      char* err;
              //printf("trying to read file\n");
              //fflush(stdout);
	      int rc1 = readFileIntoBuffer("./errorsFile", err);

              //printf("after read file\n");
              //fflush(stdout);
	      if(rc1) {
		sprintf(errors, "Error occured in compiling stmt %d, unable to retrieve error.\n", i);
	      }
	      else {
		//printf("copying error to read file\n");
		//fflush(stdout);
		sprintf(errors, "Error in stmt %d: %s\n", i, err);

		//printf("freeing err\n");
		//fflush(stdout);
		free(err);

		//printf("done freeing err\n");
		//fflush(stdout);
	      }
	      printf("ERROR: %s\n", errors);
	      fflush(stdout);
	      messageCt = 2;
	      buff->pop();
	      if(gotAddedBuffer == 1) {
		//printf("sending drop declare %s\n", bufName);
		//fflush(stdout);
		Ios::sendMessageToQueryBuf(DROP_DECLARE_CMD_CODE, bufName);
		int rc1 = 0;
		int code1;
	  
		while(buff->empty()) {
		  usleep(100000);
		}
		buff->get(code1);
		
		if(code1 == FAILURE) {
		  rc1 = 1;
		  buff->pop();
		}
		else if(code1 == SUCCESS) {
		  buff->pop();
		}
	      }
	    }
	    else if(code == COMMAND_SUCCESSFUL) {
	      messageCt = 2;
	      buff->pop();
	    }
            else if(code == ADDED_STREAM_BUFFER) {
              strcpy(bufName, qName);
              Ios::sendMessageToQueryBuf(ADDED_BUFFER, bufName);
              gotAddedBuffer = 1;
              messageCt = 1;
              buff->pop();
            }
	    else if(code == ADD_BUILTIN_IOMOD) {
	      //compile IO module
	      char command[1024];
	      char modId[80];
	      command[0] = '\0';
	      sprintf(modId, "%s.so", qName);
	      sprintf(command, "./eslcc %s >& errorsFile", qName);
	      printf("command %s\n", command);
	      int ret = system(command);
	      if(ret != 0) {
		rc = 1;
		int rc1 = readFileIntoBuffer("./errorsFile", errors);
		if(rc1) {
		sprintf(errors, "Error occured in compiling builtin data source, unable to retrieve error.\n");
		}
		printf("ERROR: %s", errors);
		
	      }
	      else {
		char objFName[80];
		sprintf(objFName, "../exe/%s", modId);
		int rc1 = addIOModule(A_IOmodule(NULL, modId, objFName, 0, userName),
				      objFName, errors);
		if(rc1 != 0) {
		  rc = 1;
		  printf("Error: %s", errors);
		}
	      }
	      updateIOModuleBuffer(modId, qName);
	      
	      //printf("here\n");
	      //fflush(stdout);      
	      buff->pop();
	    }
	    else {
	      pollStdoutMessageBuffer();
	    }
	  }

	  if(gotAddedBuffer == 0) {
	    Ios::sendMessageToQueryBuf((rc==0)?COMPILE_SUCCESS:COMPILE_FAILURE, "");
	  }
	}
	else {
	  int code;
	  int accessingPrivateUser = 0;
	  char qfName[80];
	  char dec[4096];
	  int gotMessage = 0;

	  //put create stream in ../cq/sName.cq file.
	  sprintf(qfName, "%s%s%s", QUERY_FILE_PATH, sName, QUERY_FILE_EXT);

	  fdesc = fopen(qfName, "w");
	  //printf("here writing to file %s\n", qfName);
	  //fflush(stdout);
	  fprintf(fdesc, "%s", stmtPre);
	  fclose(fdesc);
	  
	  Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, sName);
	  while(!gotMessage) {
	    while(buff->empty())
	      usleep(100000);
	    buff->get(code, (char*)qName, (char*)dec);
	    
	    if(code == COMPILE_FAILURE) {
	      rc = 1;
	      char* err = NULL;
	      int my_rc = readFileIntoBuffer("./errorsFile", err);
	      if(my_rc) {
		sprintf(errors, "Error occured in compiling query %d, unable to retrieve error.\n", i);
	      }
	      else {
		sprintf(errors, "Error in query %d: %s\n", i, strdup(err));
	      }
	      printf("ERROR: %s\n", errors);
	      fflush(stdout);
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == ADD_STREAM_DEC) {
	      sprintf(bufName, "%s", qName);
	      
	      //put qName(which is stream dec) in the fName dcl file
	      //printf("writing %s to file %s\n", dec, fName);
	      //fflush(stdout);
	      fdesc = fopen(fName, "w");
	      fprintf(fdesc, "%s", dec);
	      fclose(fdesc);

	      //set variable indicating that display text is set
	      gotStreamCmd = 1;
	      buff->pop();
	    }
	    else if(code == COMPILE_SUCCESS) {
	      buff->pop();
	      gotMessage = 1;
	    }
	    else if(code == USES_LIBRARY) {
	      buff->pop();
	      if(!isUserPublic(qName)) {
		accessingPrivateUser = 1;
	      }
	    }
	    else {
	      pollStdoutMessageBuffer();
	    }
	  }
	  if(rc != 1 && accessingPrivateUser == 1) {
	    char stdoutBuffer[MAX_STMT_PATH + 10];
	    sprintf(stdoutBuffer, "stdout_%s", sName);
	    	    
	    buffer* stdoutBuf = bm->lookup(stdoutBuffer);
	    Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, sName);
	  //TODO remove .so and .cc files as well
	    remove(fName);
	    //free(stmt);
	    if(stdoutBuf) {
	      dropStdoutBuf(userName, stdoutBuf);
	    }
	    rc = 1;
	    sprintf(errors, "Error compiling query %d, "
		  "You attempted using object(s) of a private user.\n", i);
	  }
	  else if(rc != 1 && gotStreamCmd != 1) {
	    Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, sName);
	    //TODO remove .so and .cc files as well
	    remove(qfName);
	    remove(fName);
	    //free(stmt);
	    rc = 1;
	    sprintf(errors, "Error compiling query %d, "
		  "Invalid create stream query is used.\n", i);
	  }
	}
	rewriteSystemDeclares(stmtType);
      }
      else if(stmtType == QUERY_STMT) {
	int code;
        int accessingPrivateUser = 0;
	char qName[4096];
	char dec[4096];
	char bufName[4096];
	int gotMessage = 0;
        // printf("message sent\n");
        // fflush(stdout);

	Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, sName);
	while(!gotMessage) {
	  while(buff->empty())
	    usleep(100000);
	  buff->get(code, (char*)qName, (char*)dec);
	  printf("message received %d %s\n", code, qName);
	  fflush(stdout);
	  
	  if(code == COMPILE_FAILURE) {
	    rc = 1;
	    char* err = NULL;
	    int my_rc = readFileIntoBuffer("./errorsFile", err);
            //printf("After read file into\n");
            //fflush(stdout);
	    if(my_rc) {
	      sprintf(errors, "Error occured in compiling query %d, unable to retrieve error.\n", i);
	    }
            else {
              //printf("got err\n");
              //fflush(stdout);
              sprintf(errors, "Error in query %d: %s\n", i, strdup(err));
              //printf("after sprintf\n");
              //fflush(stdout);
              //printf("after free\n");
              //fflush(stdout);
            }
            printf("ERROR: %s\n", errors);
	    fflush(stdout);
            buff->pop();
	    gotMessage = 1;
	  }
	  else if(code == COMPILE_SUCCESS) {
	    buff->pop();
	    gotMessage = 1;
	  }
          else if(code == USES_LIBRARY) {
            buff->pop();
            if(!isUserPublic(qName)) {
              accessingPrivateUser = 1;
	    }
          }
          else if(code == ADD_STREAM_DEC) {
            strcpy(bufName, qName);
            Ios::sendMessageToQueryBuf(ADDED_BUFFER, bufName);
            buff->pop();
          }
          else if(code == ADDED_STREAM_BUFFER) {
            strcpy(bufName, qName);
            Ios::sendMessageToQueryBuf(ADDED_BUFFER, bufName);
            buff->pop();
          }
	  else {
	    pollStdoutMessageBuffer();
	  }
	}
        if(rc != 1 && accessingPrivateUser == 1)
	{
	  char stdoutBuffer[MAX_STMT_PATH + 10];
	  sprintf(stdoutBuffer, "stdout_%s", sName);

	  buffer* stdoutBuf = bm->lookup(stdoutBuffer);
	  Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, sName);
	  //TODO remove .so and .cc files as well
	  remove(fName);
	  //free(stmt);
	  if(stdoutBuf) {
	    dropStdoutBuf(userName, stdoutBuf);
	  }
          rc = 1;
	  sprintf(errors, "Error compiling query %d, "
		  "You attempted using object(s) of a private user.\n", i);
	}
      }
      //printf("almost %s %s %s %d %d\n", sName, userName, bufName, rc, gotStreamCmd);
      //fflush(stdout);

      //before the two cases were diff, but are the same for now
      if(rc == 0 && stmtType == DECLARE_TABLE_STMT)
	ret->push_back(A_Stmt(0, sName, userName, bufName, gotStreamCmd, stmtPre));
      else if(rc == 0 && gotStreamCmd == 1) {
	ret->push_back(A_Stmt(0, sName, userName, bufName, gotStreamCmd, stmtPre));
      }
      else if(rc == 0)
	ret->push_back(A_Stmt(0, sName, userName, bufName));
    }
    free(stmtPre);
  }

  if(rc != 0)
  {
    int badSz = ret->size();
    for(int i = 0; i < badSz; i++)
    {
      A_stmt stmt = ret->operator[](i);
      char* sName = stmt->id;
      char fName[MAX_STMT_PATH];
      fName[0] = '\0';
      sprintf(fName, "%s%s%s", path, sName, ext);

      if(stmtType == DECLARE_TABLE_STMT || stmtType == DECLARE_STREAM_STMT) {
	if(stmt->isDirectStream) {
	  Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmt->id);	

	  // bm->kill(stmt->bufName);
	  //instead need to tell querySchdler some how
	  Ios::sendMessageToQueryBuf(DROP_DECLARE_CMD_CODE, stmt->bufName);
	  int rc1 = 0;
	  int code;
	  
	  while(buff->empty()) {
	    usleep(100000);
	  }
	  buff->get(code);
	  
	  if(code == FAILURE) {
	    rc1 = 1;
	    buff->pop();
	  }
	  else if(code == SUCCESS) {
	    buff->pop();
	  }
	  else {
	    printf("Internal Error: Got unexpected Code in removeDeclare %s %d\n", sName, code);
	    buff->pop();
	  }

	  if(rc1 == 0) {
	    //this is a view //need to COPY
	    //need to remove the stmt from the driver
	    free(stmt);
	    //but the query for the delcare is in the corresponding .cq file ?? YYY
	    remove(fName);
	  }
	  else if(rc1 != 0) {
	    printf("got error in deleting windowed declare, it may not be restored correctly.\n");
	    //YYY
	  }
	}
	else if(stmt->bufName) {
	  Ios::sendMessageToQueryBuf(DROP_DECLARE_CMD_CODE, stmt->bufName);
	  int rc1 = 0;
	  int code;
	  
	  while(buff->empty()) {
	    usleep(100000);
	  }
	  buff->get(code);
	  
	  if(code == FAILURE) {
	    rc1 = 1;
	    buff->pop();
	  }
	  else if(code == SUCCESS) {
	    buff->pop();
	  }
	  else {
	    printf("Internal Error: Got unexpected Code in removeDeclare %s %d\n", fName, code);
	    buff->pop();
	  }
	  
	  if(rc1 == 0) {
	      free(stmt);
	      remove(fName);
	  }
	}
      }
      else if(stmtType == QUERY_STMT) {
	char stdoutBuffer[256];
	sprintf(stdoutBuffer, "stdout_%s", sName);
	    	    
	buffer* stdoutBuf = bm->lookup(stdoutBuffer);
	if(stdoutBuf) {
	  dropStdoutBuf(userName, stdoutBuf);
	}
	Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, sName);
	remove(fName);
	free(stmt);
      }
    }
  }
  //printf("returning\n");
  //fflush(stdout);
  return rc;
}

void replyViewAllMessage(char* userName, vector<StmtModule> activeModules, 
			 int sockfd, int stmtType)
{
  char* message = (char*)malloc(4096);
  message[0] = '\0';
  int size = activeModules.size();

  for(int i = 0; i< size; i++)
  {
    StmtModule sm = activeModules.operator[](i);
    if(strcmp(sm.getUserName(), userName) == 0)
    {
      //printf("module %s, type %d - want %d\n", sm.getModuleName(), sm.getStmtType(), stmtType);
      // don't know why this condition was here, a bug mostly  -> (stmtType != DECLARE_TABLE_STMT) ||
      if(stmtType == sm.getStmtType())
      {
	strcat(message, sm.getModuleName());
	strcat(message, " ");
	strcat(message, sm.getModuleId());
        if(sm.getIsActive() == 1) {
	  strcat(message, " yes");
        }
	else {
	  strcat(message, " no");
	}
	strcat(message, "\n");
	
	vector<A_stmt> stmtIds = sm.getStmtIdsVector();
	int ssize = stmtIds.size();
	for(int j = 0; j < ssize; j++)
	{
          A_stmt st = stmtIds.operator[](j);
	  strcat(message, st->id);
	  if(st->isActive == 1) {
	    strcat(message, " yes");
	  }
	  else {
	    strcat(message, " no");
	  }
	  strcat(message, "\n");
	}
	if(i < size -1)
	  strcat(message, "-\n");
      }
    }
  }

  write(sockfd, message, strlen(message));
  free(message);
}

void replyViewModuleCommand(char* userName, char* buf, 
			    vector<StmtModule> activeModules,
                            int sockfd, char* path, char* ext)
{
  char* mId = strdup(buf);
  char* response = (char*) malloc(10*MAX_STMT_LEN);

  memset(response, '\0', 10*MAX_STMT_LEN);

  int index = findModuleById(userName, activeModules, mId);
  if(index == -1)
    return;

  StmtModule sm = activeModules.operator[](index);

  vector<A_stmt> moduleStmts = sm.getStmtIdsVector();
  int size = moduleStmts.size();

  sprintf(response, "\n\n#Module %s has %d statements", sm.getModuleName(), 
	  size);
 
  for(int i = 0; i< size; i++)
  {
    A_stmt st = moduleStmts.operator[](i);
    char* sName = st->id;
    char* stmtFileName = (char*) malloc(MAX_STMT_PATH);
    char* stmt = (char*) malloc(MAX_STMT_LEN);
      
    sprintf(stmtFileName, "%s%s%s", path, sName, ext);
    strcat(response, "\n#Statement ");
    strcat(response, sName);
    strcat(response, ":\n");
    if(st->isDirectStream == 0) {
      int fd = open(stmtFileName, O_RDONLY);
      int ct = read(fd, stmt, MAX_STMT_LEN);
      stmt[ct] = 0;
      close(fd);
      strcat(response, stmt);
    }
    else
      strcat(response, st->displayText);
    strcat(response, "\n");
    free(stmtFileName);
    free(stmt);
  }
    
  write(sockfd, response, strlen(response));
  free(response);
}

void sendFileText(char* fileName, int sockfd)
{
  char* stmt = (char*) malloc(MAX_STMT_LEN);
  memset(stmt, '\0', MAX_STMT_LEN);
  stmt[0]  = '\0';
  int ct;

  int fd = open(fileName, O_RDONLY);
  while((ct = read(fd, stmt, MAX_STMT_LEN)) > 0) {
    stmt[ct] = 0;
    write(sockfd, stmt, strlen(stmt));
    stmt[0] = '\0';
    usleep(10);
  }
  close(fd);
    
  free(stmt);
}


/*
 * View All command
 */
void Ios::processViewAllCommand(char* userName, int qTempSockfd) {
  /*vector<A_iomodule> activeIOmodules;
    vector<StmtModule> activeQueryModules;
    vector<StmtModule> activeDeclareModules;
    vector<StmtModule> activeAggregateModules; */

  /*
   * void replyViewModuleCommand(char* userName, char* buf, 
   *		            vector<StmtModule> activeModules,
   *                        int sockfd, char* path, char* ext)
   */

  int size = 0;
  char* iomodsHeader = "----------\nIOModules:\n";
  char* declaresHeader = "\n\n\n\n----------\nTable/Stream Modules:\n";
  char* aggregatesHeader = "\n\n\n\n----------\nAggregate Modules:\n";
  char* modelsHeader = "\n\n\n\n----------\nModel Type Modules:\n";
  char* externsHeader = "\n\n\n\n----------\nExtern Modules:\n";
  char* queriesHeader =  "\n\n\n\n----------\nQuery Modules:\n";

  char* iomodPrefix = "\nIOModule: ";

  size = activeIOmodules.size();
  write(qTempSockfd, iomodsHeader, strlen(iomodsHeader));
  for(int i =0; i < size; i ++) {
    A_iomodule aiom = activeIOmodules[i];
    if(strcmp(aiom->userName, userName) == 0) {
      write(qTempSockfd, iomodPrefix, strlen(iomodPrefix));
      write(qTempSockfd, aiom->name, strlen(aiom->name));
    }
  }

  size = activeDeclareModules.size();
  write(qTempSockfd, declaresHeader, strlen(declaresHeader));
  for(int i =0; i < size; i ++) {
    StmtModule sm = activeDeclareModules[i];
    if(strcmp(sm.getUserName(), userName) == 0) {
       replyViewModuleCommand(userName, sm.getModuleId(), 
			      activeDeclareModules, qTempSockfd,
			      DECLARE_FILE_PATH, DECLARE_FILE_EXT);
    }
  }

  size = activeQueryModules.size();
  write(qTempSockfd, queriesHeader, strlen(queriesHeader));
  for(int i =0; i < size; i ++) {
    StmtModule sm = activeQueryModules[i];
    if(strcmp(sm.getUserName(), userName) == 0) {
       replyViewModuleCommand(userName, sm.getModuleId(), 
			      activeQueryModules, qTempSockfd,
			      QUERY_FILE_PATH, QUERY_FILE_EXT);
    }
  }  


  size = activeAggregateModules.size();
  write(qTempSockfd, aggregatesHeader, strlen(aggregatesHeader));
  for(int i =0; i < size; i ++) {
    StmtModule sm = activeAggregateModules[i];
    if(strcmp(sm.getUserName(), userName) == 0) {
      processViewAggregateCommand(userName, qTempSockfd, 
				  sm.getModuleId());
    }
  }  


  size = activeModelModules.size();
  write(qTempSockfd, modelsHeader, strlen(modelsHeader));
  for(int i =0; i < size; i ++) {
    StmtModule sm = activeModelModules[i];
    if(strcmp(sm.getUserName(), userName) == 0) {
      processViewModelCommand(userName, qTempSockfd, 
				  sm.getModuleId());
    }
  }  


  size = activeExternModules.size();
  write(qTempSockfd, externsHeader, strlen(externsHeader));
  for(int i =0; i < size; i ++) {
    StmtModule sm = activeExternModules[i];
    if(strcmp(sm.getUserName(), userName) == 0) {
      processViewExternCommand(userName, qTempSockfd, 
				  sm.getModuleId());
    }
  }  
  printf("Handled ViewAllModules command\n");
}


/*
 * IOModule commands
 */
void Ios::processViewIOModulesCommand(char* userName, int qTempSockfd)
{
  int size = activeIOmodules.size();
  char fullMessage[4096];
  fullMessage[0] = '\0';

  for(int i = 0; i < size; i++)
  {
    A_iomodule m = activeIOmodules.operator[](i);
    if(strcmp(m->userName, userName) == 0)
    {
      if(m->buf != NULL && m->isActive == 1)
      {
	strcat(fullMessage, "yes ");
      }
      else
      {
	strcat(fullMessage, "no ");
      }
      strcat(fullMessage, m->name);
      strcat(fullMessage, "\n");
    }
  }
  write(qTempSockfd, fullMessage, strlen(fullMessage));
  printf("Handled ViewIOModulesCommand\n");
}

void Ios::processViewIOModuleCommand(char* userName, char* buf, 
				     int qTempSockfd)
{
  if(qTempSockfd > 0)
  {
    char* qName = strdup(strsep(&buf, "."));
    char* sourceFileName = (char*) malloc(MAX_STMT_PATH);

    sprintf(sourceFileName, "../exe/%s.cc", qName);

    sendFileText(sourceFileName, qTempSockfd);
    free(sourceFileName);
    printf("Handled ViewIOModule command\n");
  }
}

bool Ios::doesIOModuleExist(char* name)
{
  int size = activeIOmodules.size();

  for(int i = 0; i < size; i++)
  {
    A_iomodule m = activeIOmodules.operator[](i);
    if(strcmp(m->name, name) == 0)
      return true;
  }
  return false;
}

void Ios::updateIOModuleBuffer(char* name, char* buf)
{
  bufferMngr* bm = bufferMngr::getInstance();
  buffer *buff;
  buff = bm->lookup(buf);

  if(!buff)
    return;
  int size = activeIOmodules.size();

  for(int i = 0; i < size; i++)
  {
    A_iomodule m = activeIOmodules.operator[](i);
    if(strcmp(m->name, name) == 0)  //TODO: should also check for user
    {
      m->buf = buff;
      //printf("Connected buffer %s to iomodule %s\n", buf, name);
      break;
    }
  }
}

void Ios::processAddIOModuleCommand(char* userName, int qTempSockfd, char* buf)
{
  compiler *cc = compiler::getInstance();
  char* ioModuleName = strsep(&buf, "\n");
  char fName[MAX_STMT_PATH];
  char objFName[MAX_STMT_PATH];
  char moduleId[MAX_STMT_PATH];
  fName[0] = '\0';
  objFName[0] = '\0';
  moduleId[0] = '\0';

  sprintf(fName, "../exe/%s__%s", userName, ioModuleName);
  char* temp = strdup(ioModuleName);
  char* temp1 = strsep(&temp, ".");

  sprintf(objFName, "../exe/%s__%s.so", userName, temp1);
  sprintf(moduleId, "%s__%s.so", userName, temp1);

  if(!doesIOModuleExist(moduleId)) {
    FILE* fdesc = fopen(fName, "w");

    if(fdesc == NULL) {
      char* errors = "Error opening file\n";
      write(qTempSockfd, errors, strlen(errors));
      return;
    }

    fprintf(fdesc, "%s\n", buf);
    fclose(fdesc);

    //compile IO module
    char command[1024];
    command[0] = '\0';
    sprintf(command, "./eslcc %s__%s >& errorsFile", userName, temp1);

    int ret = system(command);
    if(ret != 0)
    {
      char* errors;
      int rc = readFileIntoBuffer("./errorsFile", errors);
      if(rc) {
        errors = (char*)malloc(200);
        sprintf(errors, "Error occured in compiling data source, unable to retrieve error.\n");
      }
      printf("%s", errors);
      write(qTempSockfd, errors, strlen(errors));
      free(errors);
      remove(fName);
    }
    else
    {
      char* errors = (char*) malloc(200);
      errors[0] = 0;
      int rc = addIOModule(A_IOmodule(NULL, moduleId, objFName, 0, userName),
			   objFName, errors);
      if(rc != 0) {
        printf("Error: %s", errors);
        write(qTempSockfd, errors, strlen(errors));
      }
      else if(strlen(errors) != 0) {
	printf("Error: %s", errors);
      }
      free(errors);
    }
    cc->compile("__dummy__");
  }
  else {
    sendObjectAlreadyExistsMessage(qTempSockfd, "IOModule", ioModuleName);
  }
  printf("Handled AddIOModuleCommand\n");
}

void Ios::processDropIOModuleCommand(char* userName, char* buf)
{
  removeIOModule(userName, buf);
  printf("Handled DropIOModuleCommand\n");
}

void Ios::processActivateIOModuleCommand(char* userName, int qTempSockfd, 
                                         char* buf)
{
  int size = activeIOmodules.size();
  int i = 0;

  for(i; i < size; i++)
  {
    A_iomodule m = activeIOmodules.operator[](i);
    //printf("HERE %s %s\n", m->name, buf);
    //fflush(stdout);
    if(strcmp(m->name, buf) == 0 && m->buf != NULL)
    {
      m->isActive = 1;
      break;
    }
  }
  if(i >= size)
  {
    char* errors = "Error: The IOModule is not associated to any stream.\n"
             "You must define a stream that takes data from this IOModule.\n";
    write(qTempSockfd, errors, strlen(errors));
  }
  printf("Handled ActivateIOModuleCommand\n");
}

void Ios::processDeactivateIOModuleCommand(char* userName, char* buf)
{
  int size = activeIOmodules.size();

  for(int i = 0; i < size; i++)
  {
    A_iomodule m = activeIOmodules.operator[](i);
    if(strcmp(m->name, buf) == 0 && strcmp(m->userName, userName) == 0)
    {
      m->isActive = 0;
      break;
    }
  }
  printf("Handled DeactivateIOModuleCommand\n");
}


/*
 * processTS commands
 */

int Ios::deactivateQuery(char* userName, char* queryId) {
  int gotMessage = 0;
  int code;
  int rc = 0;
  char qName[4096];
  qName[0] = '\0';
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buff = bm->lookup("_ioBuffer");

  sendMessageToQueryBuf(DEACTIVATE_QUERY_CMD_CODE, queryId);
  while(!gotMessage) {
    while(buff->empty())
      usleep(100000);
    buff->get(code, (char*)qName);
    
    if(code == COMMAND_FAILED) {
      rc = 1;
      buff->pop();
      gotMessage = 1;
    }
    else if(code == COMMAND_SUCCESSFUL) {
      buff->pop();
      //updateStmtIsActive(QUERIES_TABLENAME, userName, queryId, 0);
      gotMessage = 1;
    }
    else {
      pollStdoutMessageBuffer();
    }
  }
  return rc;  
}

int Ios::activateQuery(char* userName, char* queryId) {
  int gotMessage = 0;
  int code;
  int rc = 0;
  char qName[4096];
  qName[0] = '\0';
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buff = bm->lookup("_ioBuffer");

  sendMessageToQueryBuf(ACTIVATE_QUERY_CMD_CODE, queryId);
  while(!gotMessage) {
    while(buff->empty()) {
      usleep(100000);
    }
    buff->get(code, (char*)qName);
    printf("here %d %s\n", code, qName);
    if(code == COMMAND_FAILED) {
      rc = 1;
      buff->pop();
      gotMessage = 1;
    }
    else if(code == COMMAND_SUCCESSFUL) {
      buff->pop();
      //updateStmtIsActive(QUERIES_TABLENAME, userName, queryId, 1);
      gotMessage = 1;
    }
    else if(code == ACTIVATED_QUERY) {
      buff->pop();
      A_stmt stmt = findStmtById(activeQueryModules, qName);
      if(stmt != NULL && stmt->isActive != 1) {
	stmt->isActive = 1;
        //updateStmtIsActive(QUERIES_TABLENAME, userName, qName, 1);
      }
      else if(stmt == NULL) {
	stmt = findStmtById(activeDeclareModules, qName);
	if(stmt != NULL && stmt->isDirectStream == 1 
	   && stmt->isActive != 1) {
	  stmt->isActive = 1;
	  //updateStmtIsActive(QUERIES_TABLENAME, userName, qName1, 1);
	}
      }
    }
    else {
      pollStdoutMessageBuffer();
    }
  }
  return rc;
}


void Ios::processAddTSQueryCommand(char* userName, int qTempSockfd, char* buf, 
                                   char* clientIpAddr) {
  struct timeval tv;
  struct timezone tz;
  char wholeBuf[40960];
  wholeBuf[0]=0;
  char* dclBuf;
  vector<A_stmt> queryIds;
  char* moduleName = strsep(&buf, ".");
  char queryName[120];
  char aggrFileName[120];
  char* tsFile = "tmpTSFile.ts";
  queryName[0] = 0;
  aggrFileName[0] = '\0';
  int rc;

    rc = readFileIntoBuffer("../dcl/tables.dcl", dclBuf);
    if (rc) {
      EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "querySchdl::run", "readFileIntoBuffer");
      //      continue;
    }
    if(dclBuf)
    {
      strcpy(wholeBuf, dclBuf);
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

    FILE* tsF = fopen(tsFile, "w");
    strsep(&buf, "\n");
    fprintf(tsF, "%s\n%s", wholeBuf, buf);
    fclose(tsF);;

    sprintf(queryName, "%s__%s_%d", userName, moduleName, 
	    activeTSQueryModules.size());
    
    char command[240];
    command[0] = 0;
    sprintf(command, "./tsadl -a %s_aggr -q ../cq/%s -u %s %s >& errorsFile", 
	    queryName, queryName, userName, tsFile); 
    rc = system(command);
    if(rc != 0) {
      char* errors;
      int rec = readFileIntoBuffer("./errorsFile", errors);
      if(rec) {
	errors = (char*)malloc(200);
	sprintf(errors, "Error occured in translating ts to cq, unable to retrieve error.\n");
      }
      //TODO remove all files
      printf("%s", errors);
      write(qTempSockfd, errors, strlen(errors));
      free(errors);
      //fflush(stdout);
      return;
    }
    
    command[0] = 0;
    sprintf(command, "mv %s_aggr.aggr ./../aggr/", queryName);  
    system(command);
    
    //fflush(stdout);
    //Compiling generated AGGR
    compiler *cc = compiler::getInstance();
    setUserName(userName);
    char aggrFile[200];
    aggrFile[0] = 0;
    sprintf(aggrFile, "%s_aggr", queryName);
    
    
    cc->redirectStdout("./errorsFile", "w", stdoutLog);
    cc->redirectStderr("./errorsFile", "a", stderrLog);
    rc = cc->compile(aggrFile, cmp_aggr);
    
    stdoutLog = freopen(iosLog, "a", cc->getTempStdout());
    stderrLog = freopen(iosLog, "a", cc->getTempStderr());
    cc->resetStdouterr();
    
    bufferMngr* bm = bufferMngr::getInstance();
    buffer* b = bm->lookup("_ioBuffer");
    
    while(b->empty()) {
      usleep(100000);
    }
    int code = 0;
    char qName[4096];
    b->get(code, (char*)qName);
    if(code == COMPILE_FAILURE) {
      //TODO remove all files
      rc = 1;
      b->pop();
      return;
    }
    else if(code == COMPILE_SUCCESS) {
      b->pop();
    }
    
    if(rc != 0) {
      //TODO remove all files
      char* err;
      rc = readFileIntoBuffer("./errorsFile", err);
      if (rc) {
	err = (char*)malloc(200);
	sprintf(err, "Unable to compile translated aggregate. This is most probably an internal error.\n");
      }
    
      write(qTempSockfd, err, strlen(err));
      free(err);
      printf("Handled AddTSQuery command\n");
      return;
    }
    
    
    sprintf(aggrFileName, "./../aggr/%s_aggr.aggr", queryName);
    appendAggregateToCommonAggr(aggrFileName, AGGR_STMT);
    
    
    //Compiling generated query
    int accessingPrivateUser = 0;
    int gotMessage = 0;
    Ios::sendMessageToQueryBuf(ADD_QUERY_CMD_CODE, queryName);
    while(!gotMessage) {
      while(b->empty())
	usleep(100000);
      b->get(code, (char*)qName);
      
      if(code == COMPILE_FAILURE) {
	rc = 1;
	b->pop();
	gotMessage = 1;
      }
      else if(code == COMPILE_SUCCESS) {
	b->pop();
	gotMessage = 1;
      }
      else if(code == USES_LIBRARY) {
	b->pop();
	if(!isUserPublic(qName)) {
	  accessingPrivateUser = 1;
	}
      }
      else {
	pollStdoutMessageBuffer();
      }
    }
    
    char* errors;
    //TODO remove files
    if(rc == 1) {
      rc = readFileIntoBuffer("./errorsFile", errors);
      if (rc) {
      errors = (char*)malloc(200);
      sprintf(errors, "Error occurred in compiling generated query. Unable to retieve error. Most probably an internal error.\n");
      }
      
    }
    else if(rc != 1 && accessingPrivateUser == 1) {
      rc = 1;
      errors = (char*) malloc(200);
      sprintf(errors, "Error compiling query. "
	      "You attempted using object(s) of a private user.\n");
    }
    
    if(rc != 0) {
      char stdoutBuffer[MAX_STMT_PATH + 10];
      sprintf(stdoutBuffer, "stdout_%s", queryName);    
      
      buffer* stdoutBuf = bm->lookup(stdoutBuffer);
      Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, queryName);
      if(stdoutBuf) {
	dropStdoutBuf(userName, stdoutBuf);
      }
      
      rewriteAggregateDeclares();     
      //TODO remove .aggr .cq .so and .cc files as well
      write(qTempSockfd, errors, strlen(errors));
      free(errors);
      return;
    }
    
    vector<A_stmt> declareIds;
    addModule(StmtModule(0, aggrFile, aggrFile, declareIds, userName, 
			 AGGR_STMT), AGGREGATES_TABLENAME, 
	      activeAggregateModules);
    
    addModule(StmtModule(0, queryName, queryName, queryIds, userName, 
			 QUERY_STMT),
	      TSQUERIES_TABLENAME, activeTSQueryModules);
    free(errors);
    printf("Handled AddTSQueryCommand\n");
}


void Ios::processViewAllTSQueriesCommand(char* userName, int qTempSockfd)
{
  if(qTempSockfd > 0)
  {
    replyViewAllMessage(userName, activeTSQueryModules, qTempSockfd, 
			QUERY_STMT);
    printf("Handled ViewAllTSQueries command\n");
  }
}

void Ios::processViewTSQueryCommand(char* userName, int qTempSockfd, char* buf)
{
  if(qTempSockfd > 0)
  {
    char* qName = strdup(buf);
    char* queryFileName = (char*) malloc(MAX_STMT_PATH);
    char* aggrFileName = (char*) malloc(MAX_STMT_PATH);

    sprintf(queryFileName, "%s%s%s", QUERY_FILE_PATH, qName, QUERY_FILE_EXT);
    sprintf(aggrFileName, "%s%s_aggr%s", AGGREGATE_FILE_PATH, qName, 
	    AGGREGATE_FILE_EXT);

    sendFileText(queryFileName, qTempSockfd);
    char* sep = "\n\n/*Pattern Aggregate*/\n";
    write(qTempSockfd, sep, strlen(sep));
    sendFileText(aggrFileName, qTempSockfd);
    free(aggrFileName);
    free(queryFileName);
    printf("Handled ViewQueryDetails command\n");
  }
}

void Ios::processActivateTSQueryCommand(char* userName, int qTempSockfd,
					char* buf) {
  int index = findModuleById(userName, activeTSQueryModules, buf);
  
  if(index < 0) {
    char* errors = "Error: Query/Module not found.\n "
           "This is most probably an internal error.\n"
           "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
    return;
  }
  
  StmtModule sm = activeTSQueryModules[index];
  int rc = 0;

  rc = activateQuery(userName,  activeTSQueryModules[index].getModuleId());
  activeTSQueryModules[index].setIsActive(1);
  
  printf("Handled ActivateTSQuery command\n");
}


void Ios::processDeactivateTSQueryCommand(char* userName, int qTempSockfd,
					char* buf) {
  int index = findModuleById(userName, activeTSQueryModules, buf);
  
  if(index < 0) {
    char* errors = "Error: Query/Module not found.\n "
           "This is most probably an internal error.\n"
           "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
    return;
  }
  
  StmtModule sm = activeTSQueryModules[index];
  int rc = 0;

  rc = deactivateQuery(userName,  activeTSQueryModules[index].getModuleId());
  activeTSQueryModules[index].setIsActive(0);
  
  printf("Handled DeactivateTSQuery command\n");
}

void Ios::processDeleteTSQueryCommand(char* userName, char* buf)
{
  char* moduleId = strdup(buf);
  int index = findModuleById(userName, activeTSQueryModules, moduleId);

  if(index == -1)
    return;

  //TODO: Also need to remove the corresponding aggregate from activeAggrMods

  StmtModule qm = activeTSQueryModules.operator[](index);

  activeTSQueryModules.erase(activeTSQueryModules.begin()+index);  

  char* queryFileName = (char*) malloc(MAX_STMT_PATH);
  char* aggrName = (char*) malloc(MAX_STMT_PATH);
  sprintf(queryFileName, "%s%s%s", QUERY_FILE_PATH, qm.getModuleId(), 
	  QUERY_FILE_EXT);
  sprintf(aggrName, "%s_aggr", qm.getModuleId());

  char stdoutBuffer[MAX_STMT_PATH + 10];
  sprintf(stdoutBuffer, "stdout_%s", qm.getModuleId());
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* stdoutBuf = bm->lookup(stdoutBuffer);
  
  if(stdoutBuf) {
    dropStdoutBuf(userName, stdoutBuf);
  }  

  Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, qm.getModuleId());

  deleteModule(qm.getModuleId(), qm.getUserName(), TSQUERIES_TABLENAME);
  processDeleteAggregateCommand(userName, aggrName);


  //TODO remove both so files
  remove(queryFileName);
  free(aggrName);
  free(queryFileName);

  printf("Handled DeleteTSQuery command\n"); 
}



/*
 * processQuery commands
*/
void Ios::rewriteSystemDeclares(int stmtType)
{
  int size = activeDeclareModules.size();
  FILE* sysFdesc;
  char sysFileName[MAX_STMT_PATH];

  if(stmtType == DECLARE_STREAM_STMT)
  {
    sprintf(sysFileName, "%ssystem%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);
  }
  else if(stmtType == DECLARE_TABLE_STMT)
  {
    sprintf(sysFileName, "%stables%s", DECLARE_FILE_PATH, DECLARE_FILE_EXT);
  }

  sysFdesc = fopen(sysFileName, "w");
  fclose(sysFdesc);

  for(int i = 0; i < size; i++)
  {
    StmtModule sm = activeDeclareModules.operator[](i);
    if(sm.getStmtType() == stmtType)
      appendDeclaresToSystemFile(sm.getStmtIdsVector(), stmtType);
  }
}

void Ios::processAddQueriesCommand(char* userName, int qTempSockfd, char* buf, 
                                   char* clientIpAddr, int stmtType)
{
  struct timeval tv;
  struct timezone tz;
  char* moduleName = strsep(&buf, "\n");
  vector<A_stmt> queryIds;

  char* errors = (char*) malloc(32768);
  int rc = generateStatements(userName, buf, clientIpAddr,
			      QUERY_FILE_PATH, QUERY_FILE_EXT,
			      stmtType, &queryIds, errors);
  
  if(rc != 0) {
    write(qTempSockfd, errors, strlen(errors));
    free(errors);
    return;
  }
  
  gettimeofday(&tv, &tz);
  char moduleId[MAX_ID_LEN];
  moduleId[0] = '\0';
  sprintf(moduleId, "QueryModule_%s_%d", clientIpAddr, tv.tv_sec);
  addModule(StmtModule(0, moduleName, moduleId, queryIds, userName, 
		       stmtType), QUERIES_TABLENAME, activeQueryModules);
  free(errors);
  printf("Handled AddQueries command %d\n", activeQueryModules.size());
}

void Ios::processViewAllQueriesCommand(char* userName, int qTempSockfd)
{
  if(qTempSockfd > 0)
  {
    replyViewAllMessage(userName, activeQueryModules, qTempSockfd, QUERY_STMT);
    printf("Handled ViewAllQueries command\n");
  }
}

void Ios::processViewQueryModuleCommand(char* userName, int qTempSockfd, 
					char* buf)
{
  if(qTempSockfd > 0)
  {
    replyViewModuleCommand(userName, buf, activeQueryModules, qTempSockfd,
                           QUERY_FILE_PATH, QUERY_FILE_EXT);
    printf("Handled ViewQueryModule command\n");
  }
}

void Ios::processViewQueryCommand(char* userName, int qTempSockfd, char* buf)
{
  if(qTempSockfd > 0)
  {
    char* qName = strdup(buf);
    char* queryFileName = (char*) malloc(MAX_STMT_PATH);

    sprintf(queryFileName, "%s%s%s", QUERY_FILE_PATH, qName, QUERY_FILE_EXT);

    sendFileText(queryFileName, qTempSockfd);
    free(queryFileName);
    printf("Handled ViewQueryDetails command\n");
  }
}

void Ios::activateQueryModule(char* userName, int qTempSockfd,
                              int index) {
  StmtModule sm = activeQueryModules[index];
  vector<A_stmt> stmtIds = sm.getStmtIdsVector();
  int size = stmtIds.size();
  int rc = 0;
  int i;

  for (i = 0; i < size; i++) {
    A_stmt stmt = stmtIds[i];
    if(stmt->isActive == 0) {
      rc = activateQuery(userName, stmt->id);
      if(rc != 0)
	break;
      stmt->isActive = 1;
    }
  }

  if(rc != 0) {
    int j = 0;
    for(j; j < i; j++) {
      A_stmt stmt = stmtIds[j];
      if(stmt->isActive == 1) {
        deactivateQuery(userName, stmt->id);
        stmt->isActive = 0;
      }
    }
    char* errors = (char*)malloc(200);
    sprintf(errors, "Error: Unable to activate query %d. The stream used in the query may be deleted.\n", i);
    write(qTempSockfd, errors, strlen(errors));
    free(errors);
  }
  else {
    updateModuleIsActive(QUERIES_TABLENAME, userName, sm.getModuleId(), 1);
    activeQueryModules[index].setIsActive(1);
  }
  printf("Handled ActivateModuleQuery command\n");
}

void Ios::processActivateQueryModuleByNameCommand(char* userName, 
                            int qTempSockfd, char* buf) {
  int index = findModuleByName(userName, activeQueryModules, buf);
  printf("we are here name %s %d\n", buf, index);fflush(stdout);
  
  if(index < 0) {
    char* errors = "Error: Query/Module not found.\n "
           "This is most probably an internal error.\n"
           "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
    return;
  }
  
  activateQueryModule(userName, qTempSockfd, index);
}

void Ios::processActivateQueryModuleCommand(char* userName, int qTempSockfd,
                                            char* buf) {
  int index = findModuleById(userName, activeQueryModules, buf);
  
  if(index < 0) {
    char* errors = "Error: Query/Module not found.\n "
           "This is most probably an internal error.\n"
           "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
    return;
  }
  
  activateQueryModule(userName, qTempSockfd, index);
}

void Ios::processActivateQueryCommand(char* userName, int qTempSockfd,
				      char* buf) {
  A_stmt stmt = findStmtById(activeQueryModules, buf);
  //printf("looking for %s\n", buf);

  if(stmt != NULL) {
    int rc = 0;
    if(stmt->isActive == 0) {
      rc = activateQuery(userName, buf);
    }
    if(rc != 0) {
      char* errors = "Could not activate query. The stream used in the query may be deleted.\n"; 
      write(qTempSockfd, errors, strlen(errors));
    }
    else
      stmt->isActive = 1;
  }
  else {
    char* errors = "Error: Query/Module not found.\n"
      "This is most probably an internal error. \n"
      "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
  }
  printf("Handled ActivateQuery command\n");
}

void Ios::processDeactivateQueryModuleCommand(char* userName, int qTempSockfd,
					      char* buf) {
  int index = findModuleById(userName, activeQueryModules, buf);

  //printf("looking for %s\n", buf);

  if(index < 0) {
    char* errors = "Error: Query/Module not found.\n"
      "This is most probably an internal error. \n"
      "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
    return;
  }
  
  StmtModule sm = activeQueryModules[index];
  vector<A_stmt> stmtIds = sm.getStmtIdsVector();
  int size = stmtIds.size();
  int rc = 0;
  int i;

  for (i = 0; i < size; i++) {
    A_stmt stmt = stmtIds[i];
    if(stmt->isActive == 1) {
      rc = deactivateQuery(userName, stmt->id);
      if(rc != 0)
	break;
      stmt->isActive = 0;
    }
  }

  if(rc != 0) {
    int j = 0;
    for(j; j < i; j++) {
      A_stmt stmt = stmtIds[j];
      if(stmt->isActive == 0) {
        activateQuery(userName, stmt->id);
        stmt->isActive = 1;
      }
    }
    char* errors = (char*)malloc(200);
    sprintf(errors, "Error: Unable to deactivate query %d.\n", i);
    write(qTempSockfd, errors, strlen(errors));
    free(errors);
  }
  else {
    updateModuleIsActive(QUERIES_TABLENAME, userName, sm.getModuleId(), 0);
    activeQueryModules[index].setIsActive(0);
  }
  printf("Handled DeactivateModuleQuery command\n");
}

void Ios::processDeactivateQueryCommand(char* userName, int qTempSockfd,
					char* buf) {
  A_stmt stmt = findStmtById(activeQueryModules, buf);
  if(stmt != NULL) {
    int rc = 0;
    if(stmt->isActive == 1) { 
      rc = deactivateQuery(userName, buf);
    }
    if(rc != 0) {
      char* errors = "Error: Unable to deactivate query.\n";
      write(qTempSockfd, errors, strlen(errors));
    }
    else
      stmt->isActive = 0;
  }
  else {
    char* errors = "Error: Query/Module not found.\n"
      "This is most probably an internal error. \n"
      "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
  }
  printf("Handled DeactivateQuery command\n");
}

void Ios::removeQueryModuleByIndex(char* userName, int index)
{
  bufferMngr* bm = bufferMngr::getInstance();
  StmtModule qm = activeQueryModules.operator[](index);
  deleteModule(qm.getModuleId(), qm.getUserName(), QUERIES_TABLENAME);
  activeQueryModules.erase(activeQueryModules.begin()+index);

  vector<A_stmt> queryIds = qm.getStmtIdsVector();
  int size = queryIds.size();

  for(int i = 0; i < size; i++)
  {
    A_stmt stmt = queryIds.operator[](i);
    char* qName = stmt->id;
    char* qFileName = (char*) malloc(MAX_STMT_PATH);
    char stdoutBuffer[MAX_STMT_PATH + 10];
    sprintf(qFileName, "%s%s%s", QUERY_FILE_PATH, qName, QUERY_FILE_EXT);
    sprintf(stdoutBuffer, "stdout_%s", qName);

    buffer* stdoutBuf = bm->lookup(stdoutBuffer);
    
    Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, qName);
    //TODO remove .so and .cc files as well
    remove(qFileName);
    free(qFileName);
    free(stmt);
    if(stdoutBuf)
    {
      dropStdoutBuf(userName, stdoutBuf);
    }
  }
}

void Ios::processDeleteQueryModuleCommand(char* userName, char* buf)
{
  char* moduleId = strdup(buf);

  int index = findModuleById(userName, activeQueryModules, moduleId);

  if(index == -1)
    return;

  removeQueryModuleByIndex(userName, index);

  printf("Handled DeleteQueryModule command\n");
}

void Ios::removeQueryFromModuleAtIndex(int index, int qIndex)
{
  StmtModule qm = activeQueryModules.operator[](index);

  A_stmt stmt = qm.getStmtIdsVector()[qIndex];
  deleteStmtInModule(qm.getModuleId(), qm.getUserName(), stmt->id, 
		     QUERIES_TABLENAME);
  qm.removeStmtAtIndex(qIndex);
  activeQueryModules.erase(activeQueryModules.begin()+index);
  activeQueryModules.insert(activeQueryModules.begin()+index, qm);
  return;
}

void Ios::processDeleteQueryCommand(char* userName, char* buf)
{
  bufferMngr* bm = bufferMngr::getInstance();
  char* moduleId = strdup(strsep(&buf, " "));
  char* queryId = strdup(buf);

  int index = findModuleById(userName, activeQueryModules, moduleId);

  if(index == -1)
    return;

  StmtModule qm = activeQueryModules.operator[](index);

  vector<A_stmt> moduleQueries = qm.getStmtIdsVector();
  int size = moduleQueries.size();

  for(int i = 0; i< size; i++)
  {
    A_stmt stmt = moduleQueries.operator[](i);
    char* qName = stmt->id;

    if(strcmp(queryId, qName) == 0)
    {
      removeQueryFromModuleAtIndex(index, i);
      char* queryFileName = (char*) malloc(MAX_STMT_PATH);
      sprintf(queryFileName, "%s%s%s", QUERY_FILE_PATH, qName, QUERY_FILE_EXT);
      Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, qName);
      char stdoutBuffer[MAX_STMT_PATH + 10];
      sprintf(stdoutBuffer, "stdout_%s", qName);    

      buffer* stdoutBuf = bm->lookup(stdoutBuffer);
      if(stdoutBuf) {
        dropStdoutBuf(userName, stdoutBuf);
      }
      remove(queryFileName);
      free(queryFileName);
      free(stmt);
      break;
    }
  }
  printf("Handled DeleteQuery command\n"); 
}

/*
 * processAggregate commands
*/
void Ios::appendAggregateToCommonAggr(char* aggregateFile,
				      int stmtType)
{
  FILE* aggregateFdesc;
  FILE* aggrFdesc;
  char aggrFileName[MAX_STMT_PATH];
  char* buf = (char*)malloc(10*MAX_STMT_LEN);

  if(stmtType == MODEL_STMT) {
    sprintf(aggrFileName, "%smodel%s", MODEL_FILE_PATH, ".dcl");
  }
  else if(stmtType == AGGR_STMT) {
    sprintf(aggrFileName, "%saggr%s", AGGREGATE_FILE_PATH, ".dcl");
  }
  else if(stmtType == EXTERN_STMT) {
    sprintf(aggrFileName, "%sext%s", AGGREGATE_FILE_PATH, ".dcl");
  }

  aggrFdesc = fopen(aggrFileName, "a");
  aggregateFdesc = fopen(aggregateFile, "r");
    
  memset(buf, '\0', 10*MAX_STMT_LEN);
  fread(buf, 10*MAX_STMT_LEN, 1, aggregateFdesc);
  fwrite(buf, strlen(buf), 1, aggrFdesc);
  fclose(aggregateFdesc);
  free(buf);
  fclose(aggrFdesc);
}

/*char* getFuncNameFromBuffer(char* buf) {
  char* buf1 = strdup(buf);
  char* temp;
  char* temp1;

  getToken(buf1, &temp);
  
  
  free(temp);
  free(temp1);
  }*/

void Ios::processAddExtAggregateCommand(char* userName, int qTempSockfd, 
				     char* buf, char* clientIpAddr)
{
  struct timeval tv;
  struct timezone tz;
  char moduleName[1024]; //parse buf to findout the name of the aggregate
  vector<A_stmt> declareIds;
  char moduleId[MAX_ID_LEN];
  FILE* fdesc;
  char sName[MAX_ID_LEN];
  char fName[MAX_STMT_PATH];
  char cfName[MAX_STMT_PATH];
  int windowed = 0;
  sName[0] = '\0';
  fName[0] = '\0';
  compiler *cc = compiler::getInstance();

  //the buf has the ESL defnition and the c/c++ code
  //they are delimited by "\n|||\n"
  char* esl_buf1 = strdup(buf);
  char* delim = strstr(esl_buf1, "\n|||\n");
  esl_buf1[delim-esl_buf1] = '\0';
  char* esl_buf = (char*)malloc(strlen(esl_buf1+200));
  delim = delim + 5;
  //thus now esl_buf has the esl def and delim has the c/c++ code

  gettimeofday(&tv, &tz);
  //printf("in add aggregate cmd %s, %s, %d\n", userName, esl_buf1, windowed);
  //fflush(stdout);

  int rc = prependUserNameForAggr(userName, esl_buf1, 
				  (char*)moduleName, windowed, esl_buf);
  //printf("back from prependUserName\n");
  //fflush(stdout);

  sprintf(sName, "%s", moduleName);
  //printf("does module exist\n");
  //fflush(stdout);
  if(!doesModuleExist(sName, activeAggregateModules)) {
    //printf("module does not exist\n");
    //fflush(stdout);
    sprintf(fName, "%s%s%s", AGGREGATE_FILE_PATH, sName, AGGREGATE_FILE_EXT);
    
    fdesc = fopen(fName, "w");
    if(rc != 0) {
      fclose(fdesc);
      remove(fName);
      char* errors = "You must be trying to access a private user's object(s). Please verify your query.\n";
      write(qTempSockfd, errors, strlen(errors));
      printf("Handled AddExtAggregate command\n");
      return;
    }
    
    fprintf(fdesc, "%s\n", esl_buf);
    fclose(fdesc);
    
    //Ios::sendMessageToQueryBuf(ADD_AGGREGATE_CMD_CODE, sName);
    setUserName(userName);
    
    cc->redirectStdout("./errorsFile", "w", stdoutLog);
    cc->redirectStderr("./errorsFile", "a", stderrLog);

    //printf("calling compile\n");
    //fflush(stdout);
    rc = cc->compile(sName, cmp_aggr);
    //printf("back from compile\n");
    //fflush(stdout);
    bufferMngr* bm = bufferMngr::getInstance();
    buffer* b = bm->lookup("_ioBuffer");
    
    while(b->empty()) {
      usleep(100000);
    }

    //need to write delim, which is the c/c++ def in the .cc file and recompile
    stdoutLog = freopen(iosLog, "a", cc->getTempStdout());
    stderrLog = freopen(iosLog, "a", cc->getTempStderr());
    cc->resetStdouterr();

    int code = 0;
    char qName[4096];
    b->get(code, (char*)qName);
    if(code == COMPILE_FAILURE) {
      b->pop();
      rc = 1;
    }
    else if(code == COMPILE_SUCCESS) {
      b->pop();
    }


    if(rc == 0) {
      sprintf(cfName, "../exe/%s.cc", sName);
      fdesc = fopen(cfName, "w");
      
      fprintf(fdesc, "%s\n", delim);
      fclose(fdesc);
    
      const char *compile_cmd={"./eslcc %s >& errorsFile"}; 
      char cmd[256];
      char id[10];
      sprintf(cmd, compile_cmd, sName);
      
      rc = system(cmd);    
    }

    if(rc != 0) {
      char* err;
      rc = readFileIntoBuffer("./errorsFile", err);
    if (rc) {
	err = (char*)malloc(200);
	sprintf(err, "Error occurred in compiling aggregate. Unable to retieve error.\n");
      }
      
    if(err) {
      write(qTempSockfd, err, strlen(err));
      
      free(err);
    }
      goto exit;
    }
    

    appendAggregateToCommonAggr(fName, AGGR_STMT);
  
    moduleId[0] = '\0';
    sprintf(moduleId, "%s", sName);
    addModule(StmtModule(0, moduleName, moduleId, declareIds, userName, 
			 AGGR_STMT, 0), 
	      AGGREGATES_TABLENAME, activeAggregateModules);

  }
  else {
    sendObjectAlreadyExistsMessage(qTempSockfd, "Aggregate", sName);
  }
 exit:
  //printf("done with add aggregae cmd\n");
  //fflush(stdout);
  free(esl_buf);
  printf("Handled AddExtAggregate command\n");
}


/*
 * processModel commands
*/

void Ios::processViewAllModelsCommand(char* userName, int qTempSockfd)
{
  if(qTempSockfd > 0)
  {
    replyViewAllMessage(userName, activeModelModules, qTempSockfd, 
			MODEL_STMT);
    printf("Handled ViewAllModels command\n");
  }
} 

void Ios::processViewModelCommand(char* userName, int qTempSockfd, 
				      char* buf)
{
  if(qTempSockfd > 0)
  {
    char* mName = strdup(buf);
    char* modelFileName = (char*) malloc(MAX_STMT_PATH);

    sprintf(modelFileName, "%s%s%s", MODEL_FILE_PATH, mName, MODEL_FILE_EXT);

    sendFileText(modelFileName, qTempSockfd);
    free(modelFileName);
    printf("Handled ViewModelDetails command\n");
  }
} 

void Ios::rewriteModelDeclares()
{
  int size = activeModelModules.size();
  FILE* modelFdesc;
  char modelFileName[MAX_STMT_PATH];
  char* buf = (char*)malloc(200*MAX_STMT_LEN);

  sprintf(modelFileName, "%smodel%s", MODEL_FILE_PATH, ".dcl");

  modelFdesc = fopen(modelFileName, "w");

  for(int i = 0; i < size; i++)
  {
    StmtModule sm = activeModelModules.operator[](i);
    char* moduleName = sm.getModuleId();
    FILE* modFdesc;
    char modelFile[MAX_STMT_PATH];
    modelFile[0] = '\0';

    sprintf(modelFile, "%s%s%s", MODEL_FILE_PATH, moduleName, MODEL_FILE_EXT);
    modFdesc = fopen(modelFile, "r");
    memset(buf, 0, 10*MAX_STMT_LEN);
   
    fread(buf, 10*MAX_STMT_LEN, 1, modFdesc);

    fwrite(buf, strlen(buf), 1, modelFdesc);
    fclose(modFdesc);
  }
  fclose(modelFdesc);

  free(buf);
}

void Ios::processDeleteModelCommand(char* userName, char* buf)
{
  char* moduleId = strdup(buf);
  char fileName[MAX_STMT_PATH];
  fileName[0] = '\0';

  sprintf(fileName, "%s%s%s", MODEL_FILE_PATH, moduleId, MODEL_FILE_EXT);
  remove(fileName);

  int index = findModuleById(userName, activeModelModules, moduleId);

  if(index == -1)
    return;

  StmtModule sm = activeModelModules[index];
  deleteModule(sm.getModuleId(), sm.getUserName(), MODELS_TABLENAME);
  activeModelModules.erase(activeModelModules.begin()+index);

  rewriteModelDeclares();

  printf("Handled DeleteModelModule command\n");
} 


void Ios::processAddModelCommand(char* userName, int qTempSockfd, 
				     char* buf, char* clientIpAddr)
{
  struct timeval tv;
  struct timezone tz;
  char moduleName[1024]; //parse buf to findout the name of the model
  vector<A_stmt> declareIds;
  char moduleId[MAX_ID_LEN];
  FILE* fdesc;
  char sName[MAX_ID_LEN];
  char fName[MAX_STMT_PATH];
  int windowed = 0;
  sName[0] = '\0';
  fName[0] = '\0';
  compiler *cc = compiler::getInstance();
  char* bufPre = (char*)malloc(strlen(buf)+200);

  gettimeofday(&tv, &tz);

  int rc = prependUserNameForModel(userName, buf, 
				  (char*)moduleName, bufPre);
  //printf("back from prependUserName\n");
  //fflush(stdout);

  sprintf(sName, "%s", moduleName);
  //printf("does module exist\n");
  //fflush(stdout);
  if(!doesModuleExist(sName, activeModelModules)) {
    //printf("module does not exist %s\n", sName);
    //fflush(stdout);
    sprintf(fName, "%s%s%s", MODEL_FILE_PATH, sName, MODEL_FILE_EXT);
    //printf("fName %s\n", fName);
    //fflush(stdout);
    
    fdesc = fopen(fName, "w");
    if(rc != 0) {
      fclose(fdesc);
      remove(fName);
      char* errors = "You must be trying to access a private user's object(s). Please verify your query.\n";
      write(qTempSockfd, errors, strlen(errors));
      printf("Handled AddModel command\n");
      return;
    }
    
    fprintf(fdesc, "%s\n", bufPre);
    fclose(fdesc);
    free(bufPre);
    //printf("Done writing file %s\n", fName);
    //fflush(stdout);
    
    setUserName(userName);
    
    cc->redirectStdout("./errorsFile", "w", stdoutLog);
    cc->redirectStderr("./errorsFile", "a", stderrLog);

    //printf("calling compile\n");
    //fflush(stdout);
    rc = cc->compile("__dummy__");
    //printf("back from compile\n");
    //fflush(stdout);
    bufferMngr* bm = bufferMngr::getInstance();
    buffer* b = bm->lookup("_ioBuffer");
    
    while(b->empty()) {
      usleep(100000);
    }
    
    stdoutLog = freopen(iosLog, "a", cc->getTempStdout());
    stderrLog = freopen(iosLog, "a", cc->getTempStderr());
    cc->resetStdouterr();
    
    int code = 0;
    char qName[4096];
    b->get(code, (char*)qName);
    if(code == COMPILE_FAILURE) {
      b->pop();
      rc = 1;
    }
    else if(code == COMPILE_SUCCESS) {
      b->pop();
    }
    
    if(rc != 0) {
      char* err;
      rc = readFileIntoBuffer("./errorsFile", err);
      if (rc) {
	err = (char*)malloc(200);
	sprintf(err, "Error occurred in compiling model. Unable to retieve error.\n");
      }
      rewriteModelDeclares();
      write(qTempSockfd, err, strlen(err));
      free(err);
      goto exit;
    }
    
    appendAggregateToCommonAggr(fName, MODEL_STMT);
  
    moduleId[0] = '\0';
    sprintf(moduleId, "%s", sName);
    addModule(StmtModule(0, moduleName, moduleId, declareIds, userName, 
			 MODEL_STMT, 0), 
	      MODELS_TABLENAME, activeModelModules);
  }
  else {
    sendObjectAlreadyExistsMessage(qTempSockfd, "Model", sName);
  }
 exit:
  printf("Handled AddModel command\n");
} 

void Ios::processAddAggregateCommand(char* userName, int qTempSockfd, 
				     char* buf, char* clientIpAddr)
{
  struct timeval tv;
  struct timezone tz;
  char moduleName[1024]; //parse buf to findout the name of the aggregate
  vector<A_stmt> declareIds;
  char moduleId[MAX_ID_LEN];
  FILE* fdesc;
  char sName[MAX_ID_LEN];
  char fName[MAX_STMT_PATH];
  int windowed = 0;
  sName[0] = '\0';
  fName[0] = '\0';
  compiler *cc = compiler::getInstance();
  char* bufPre = (char*)malloc(strlen(buf)+200);

  gettimeofday(&tv, &tz);
  //printf("in add aggregate cmd %s, %d\n", userName, windowed);
  //fflush(stdout);

  int rc = prependUserNameForAggr(userName, buf, 
				  (char*)moduleName, windowed, bufPre);
  //printf("back from prependUserName\n");
  //fflush(stdout);

  sprintf(sName, "%s", moduleName);
  //printf("does module exist\n");
  //fflush(stdout);
  if(!doesModuleExist(sName, activeAggregateModules)) {
    //printf("module does not exist %s\n", sName);
    //fflush(stdout);
    sprintf(fName, "%s%s%s", AGGREGATE_FILE_PATH, sName, AGGREGATE_FILE_EXT);
    //printf("fName %s\n", fName);
    //fflush(stdout);
    
    fdesc = fopen(fName, "w");
    if(rc != 0) {
      fclose(fdesc);
      remove(fName);
      char* errors = "You must be trying to access a private user's object(s). Please verify your query.\n";
      write(qTempSockfd, errors, strlen(errors));
      printf("Handled AddAggregate command\n");
      return;
    }
    
    fprintf(fdesc, "%s\n", bufPre);
    fclose(fdesc);
    free(bufPre);
    //printf("Done writing file %s\n", fName);
    //fflush(stdout);
    
    //Ios::sendMessageToQueryBuf(ADD_AGGREGATE_CMD_CODE, sName);
    setUserName(userName);
    
    cc->redirectStdout("./errorsFile", "w", stdoutLog);
    cc->redirectStderr("./errorsFile", "a", stderrLog);

    //printf("calling compile\n");
    //fflush(stdout);
    rc = cc->compile(sName, cmp_aggr);
    //printf("back from compile\n");
    //fflush(stdout);
    bufferMngr* bm = bufferMngr::getInstance();
    buffer* b = bm->lookup("_ioBuffer");
    
    while(b->empty()) {
      usleep(100000);
    }
    
    stdoutLog = freopen(iosLog, "a", cc->getTempStdout());
    stderrLog = freopen(iosLog, "a", cc->getTempStderr());
    cc->resetStdouterr();
    
    int code = 0;
    char qName[4096];
    b->get(code, (char*)qName);
    if(code == COMPILE_FAILURE) {
      b->pop();
      rc = 1;
    }
    else if(code == COMPILE_SUCCESS) {
      b->pop();
    }
    
    if(rc != 0) {
      char* err;
      rc = readFileIntoBuffer("./errorsFile", err);
      if (rc) {
	err = (char*)malloc(200);
	sprintf(err, "Error occurred in compiling aggregate. Unable to retieve error.\n");
      }
      
      write(qTempSockfd, err, strlen(err));
      free(err);
      goto exit;
    }
    
    appendAggregateToCommonAggr(fName, AGGR_STMT);
  
    moduleId[0] = '\0';
    sprintf(moduleId, "%s", sName);
    addModule(StmtModule(0, moduleName, moduleId, declareIds, userName, 
			 AGGR_STMT, 0), 
	      AGGREGATES_TABLENAME, activeAggregateModules);
  }
  else {
    sendObjectAlreadyExistsMessage(qTempSockfd, "Aggregate", sName);
  }
 exit:
  //printf("done with add aggregate cmd\n");
  //fflush(stdout);
  printf("Handled AddAggregate command\n");
} 

void Ios::processViewAllAggregatesCommand(char* userName, int qTempSockfd)
{
  if(qTempSockfd > 0)
  {
    replyViewAllMessage(userName, activeAggregateModules, qTempSockfd, 
			AGGR_STMT);
    printf("Handled ViewAllAggregates command\n");
  }
} 

void Ios::processViewAggregateCommand(char* userName, int qTempSockfd, 
				      char* buf)
{
  if(qTempSockfd > 0)
  {
    char* aName = strdup(buf);
    char* aggregateFileName = (char*) malloc(MAX_STMT_PATH);

    sprintf(aggregateFileName, "%s%s%s", AGGREGATE_FILE_PATH, aName, AGGREGATE_FILE_EXT);

    sendFileText(aggregateFileName, qTempSockfd);
    free(aggregateFileName);
    printf("Handled ViewAggregateDetails command\n");
  }
} 

void Ios::rewriteAggregateDeclares()
{
  int size = activeAggregateModules.size();
  FILE* aggrFdesc;
  char aggrFileName[MAX_STMT_PATH];
  char* buf = (char*)malloc(200*MAX_STMT_LEN);

  sprintf(aggrFileName, "%saggr%s", AGGREGATE_FILE_PATH, ".dcl");

  aggrFdesc = fopen(aggrFileName, "w");

  for(int i = 0; i < size; i++)
  {
    StmtModule sm = activeAggregateModules.operator[](i);
    char* moduleName = sm.getModuleId();
    FILE* aggregateFdesc;
    char aggregateFile[MAX_STMT_PATH];
    aggregateFile[0] = '\0';

    sprintf(aggregateFile, "%s%s%s", AGGREGATE_FILE_PATH, moduleName, AGGREGATE_FILE_EXT);
    aggregateFdesc = fopen(aggregateFile, "r");
    memset(buf, 0, 10*MAX_STMT_LEN);
   
    fread(buf, 10*MAX_STMT_LEN, 1, aggregateFdesc);

    fwrite(buf, strlen(buf), 1, aggrFdesc);
    fclose(aggregateFdesc);
  }
  fclose(aggrFdesc);

  sprintf(aggrFileName, "%sext%s", AGGREGATE_FILE_PATH, ".dcl");

  aggrFdesc = fopen(aggrFileName, "w");

  //externs should be handled same way aggregates are
  size = activeExternModules.size();
  for(int i = 0; i < size; i++)
  {
    StmtModule sm = activeExternModules.operator[](i);
    char* moduleName = sm.getModuleId();
    FILE* aggregateFdesc;
    char aggregateFile[MAX_STMT_PATH];
    aggregateFile[0] = '\0';

    sprintf(aggregateFile, "%s%s%s", AGGREGATE_FILE_PATH, moduleName, AGGREGATE_FILE_EXT);
    aggregateFdesc = fopen(aggregateFile, "r");
    memset(buf, 0, 10*MAX_STMT_LEN);
   
    fread(buf, 10*MAX_STMT_LEN, 1, aggregateFdesc);

    fwrite(buf, strlen(buf), 1, aggrFdesc);
    fclose(aggregateFdesc);
  }

  free(buf);

  fclose(aggrFdesc);
}


void Ios::processDeleteAggregateCommand(char* userName, char* buf)
{
  char* moduleId = strdup(buf);
  char fileName[MAX_STMT_PATH];
  char soFileName[MAX_STMT_PATH];
  fileName[0] = '\0';
  soFileName[0] = '\0';

  sprintf(fileName, "%s%s%s", AGGREGATE_FILE_PATH, moduleId, AGGREGATE_FILE_EXT);
  sprintf(soFileName, "%s%s%s", "../exe/", moduleId, ".so");
  remove(fileName);
  remove(soFileName);

  int index = findModuleById(userName, activeAggregateModules, moduleId);

  if(index == -1)
    return;

  StmtModule sm = activeAggregateModules[index];
  deleteModule(sm.getModuleId(), sm.getUserName(), AGGREGATES_TABLENAME);
  activeAggregateModules.erase(activeAggregateModules.begin()+index);

  rewriteAggregateDeclares();

  printf("Handled DeleteAggregateModule command\n");
} 


/* Externs */
void Ios::processAddExternCommand(char* userName, int qTempSockfd, 
				     char* buf, char* clientIpAddr)
{
  char* buf1 = strdup(buf);
  struct timeval tv;
  struct timezone tz;
  char moduleName[1024];//parse buf to findout the name of the aggregate
  vector<A_stmt> declareIds;
  char moduleId[MAX_ID_LEN];
  FILE* fdesc;
  char sName[MAX_ID_LEN];
  char act_func_name[MAX_ID_LEN];
  char fName[MAX_STMT_PATH];
  char ccfName[MAX_STMT_PATH];
  int windowed;
  sName[0] = '\0';
  fName[0] = '\0';
  compiler *cc = compiler::getInstance();

  gettimeofday(&tv, &tz);

  //printf("check that file %s\n", buf1);

  char* temp = strsep(&buf1, "\n"); //this should be the extern func name
  sprintf(moduleName, "%s__%s", userName, temp);
  sprintf(act_func_name, "%s", temp);
  temp = strsep(&buf1, "\n");  //this has the atlas external declaration

  int rc;

  //rc = prependUserNameForExtern(userName, temp, (char*)moduleName);

  sprintf(sName, "%s", moduleName);
  if(!doesModuleExist(sName, activeExternModules)) {
    sprintf(fName, "%s%s%s", AGGREGATE_FILE_PATH, sName, AGGREGATE_FILE_EXT);
    
    //printf("writing aggr file %s\n", temp);
    fdesc = fopen(fName, "w");
    fprintf(fdesc, "%s\n", temp);
    fclose(fdesc);
    
    //printf("writing cc file %s\n", buf1);
    sprintf(ccfName, "../exe/%s.cc", sName);
    fdesc = fopen(ccfName, "w");
    fprintf(fdesc, "%s\n", buf1);
    fclose(fdesc);
    
    //first see if the cc file complies using eslcc
    char command[1024];
    sprintf(command, "./eslcc %s >& errorsFile", sName);
    rc = system(command);
    if(rc != 0) {
      remove(fName);
      remove(ccfName);
      char* err;
      int rc = readFileIntoBuffer("./errorsFile", err);
      if(rc) {
	err = (char*)malloc(1024);
	sprintf(err, "Error occured in compiling source of the external function.\n");
      }
	      
      write(qTempSockfd, err, strlen(err));
      free(err);
      goto exit;
    }

    //check to make sure the function is avaiable in the file
    char sofName[1024];
    sprintf(sofName, "../exe/%s.so", sName);
    void* handle = dlopen(sofName, RTLD_NOW);
    if(!handle) {
      remove(fName);
      remove(ccfName);
      char err[1024];
	  
      sprintf(err, "The submitted code does not contain the external function %s, remember"
	     " extern \"C\" function declaration is required for c++.", act_func_name);
      write(qTempSockfd, err, strlen(err));
      goto exit;
    }

    void* func = dlsym(handle, act_func_name);
    char* err;
    if((err = dlerror()) != NULL) {
      remove(fName);
      remove(ccfName);
      remove(sofName);
      char error[1024];
	  
      sprintf(error, "The submitted code does not contain the external function %s, remember"
	     " extern \"C\" function declaration is required for c++."
	     " Got dlsym error: %s\n", act_func_name, err);
      write(qTempSockfd, error, strlen(error));
      goto exit;
    }

    dlclose(handle);

    //if so, append extern dec to aggr.dcl file and compile dummy.cq
    appendAggregateToCommonAggr(fName, EXTERN_STMT); //this is for aggr, but I think it will work
                                        //for extern as well

    setUserName(userName);    
    cc->redirectStdout("./errorsFile", "w", stdoutLog);
    cc->redirectStderr("./errorsFile", "a", stderrLog);

    rc = cc->compile("__dummy__");
    bufferMngr* bm = bufferMngr::getInstance();
    buffer* b = bm->lookup("_ioBuffer");
    
    while(b->empty()) {
      usleep(100000);
    }
    
    stdoutLog = freopen(iosLog, "a", cc->getTempStdout());
    stderrLog = freopen(iosLog, "a", cc->getTempStderr());
    cc->resetStdouterr();
    
    int code = 0;
    char qName[4096];
    b->get(code, (char*)qName);
    if(code == COMPILE_FAILURE) {
      b->pop();
      rc = 1;
    }
    else if(code == COMPILE_SUCCESS) {
      b->pop();
    }
    
    if(rc != 0) {
      char* err;
      rc = readFileIntoBuffer("./errorsFile", err);
      if (rc) {
	err = (char*)malloc(200);
	sprintf(err, "Error occurred in compiling extern declaration. Unable to retieve error.\n");
      }
      
      rewriteAggregateDeclares();
      write(qTempSockfd, err, strlen(err));
      free(err);
      goto exit;
    }
    
    moduleId[0] = '\0';
    sprintf(moduleId, "%s", sName);
    addModule(StmtModule(0, moduleName, moduleId, declareIds, userName, 
			 EXTERN_STMT, 0), 
	      EXTERNS_TABLENAME, activeExternModules);
    
  }
  else {
    sendObjectAlreadyExistsMessage(qTempSockfd, "Extern", sName);
  }
 exit:
  printf("Handled AddExtern command\n");
} 

void Ios::processViewAllExternsCommand(char* userName, int qTempSockfd)
{
  if(qTempSockfd > 0)
  {
    replyViewAllMessage(userName, activeExternModules, qTempSockfd, 
			EXTERN_STMT);
    printf("Handled ViewAllExterns command\n");
  }
} 

void Ios::processViewExternCommand(char* userName, int qTempSockfd, 
				      char* buf)
{
  if(qTempSockfd > 0)
  {
    char* eName = strdup(buf);
    char* externFileName = (char*) malloc(MAX_STMT_PATH);
    char* sep = "----------------";
    char* externccFileName = (char*) malloc(MAX_STMT_PATH);

    sprintf(externFileName, "%s%s%s", AGGREGATE_FILE_PATH, eName, AGGREGATE_FILE_EXT);
    sprintf(externccFileName, "../exe/%s.cc", eName);

    sendFileText(externFileName, qTempSockfd);
    write(qTempSockfd, sep, strlen(sep));
    sendFileText(externccFileName, qTempSockfd);
    free(externFileName);
    free(externccFileName);
    printf("Handled ViewExternDetails command\n");
  }
} 

void Ios::processDeleteExternCommand(char* userName, char* buf)
{
  char* moduleId = strdup(buf);
  char fileName[MAX_STMT_PATH];
  char ccFileName[MAX_STMT_PATH];
  char soFileName[MAX_STMT_PATH];
  fileName[0] = '\0';
  soFileName[0] = '\0';

  sprintf(fileName, "%s%s%s", AGGREGATE_FILE_PATH, moduleId, AGGREGATE_FILE_EXT);
  sprintf(ccFileName, "%s%s%s", "../exe/", moduleId, ".cc");
  sprintf(soFileName, "%s%s%s", "../exe/", moduleId, ".so");
  remove(fileName);
  remove(soFileName);
  remove(ccFileName);

  int index = findModuleById(userName, activeExternModules, moduleId);

  if(index == -1)
    return;

  StmtModule sm = activeExternModules[index];
  deleteModule(sm.getModuleId(), sm.getUserName(), EXTERNS_TABLENAME);
  activeExternModules.erase(activeExternModules.begin()+index);

  rewriteAggregateDeclares();

  printf("Handled DeleteExternModule command\n");
} 




/*
 * processDeclare commands
*/

void Ios::processAddDeclaresCommand(char* userName, int qTempSockfd, 
				    char* buf, 
				    char* clientIpAddr, int stmtType)
{
  struct timeval tv;
  struct timezone tz;
  char* moduleName = strsep(&buf, "\n");
  vector<A_stmt> declareIds;
  char* errors = (char*)malloc(32768);
  
  int rc = generateStatements(userName, buf, clientIpAddr,
			      DECLARE_FILE_PATH, 
			      DECLARE_FILE_EXT,
			      stmtType, &declareIds, errors);

  
  if(rc != 0) {
    rewriteSystemDeclares(stmtType);
    write(qTempSockfd, errors, strlen(errors));
    free(errors);
    printf("Handled AddDeclares command\n");

    return;
  }
  
  appendDeclaresToSystemFile(declareIds, stmtType);
  
  gettimeofday(&tv, &tz);
  char moduleId[MAX_ID_LEN];
  moduleId[0] = '\0';
    
  sprintf(moduleId, "DeclareModule_%s_%d", clientIpAddr, tv.tv_sec);
  addModule(StmtModule(0, moduleName, moduleId, declareIds, userName, 
		       stmtType), DECLARES_TABLENAME, activeDeclareModules);
  
  if(errors)
    free(errors);
  printf("Handled AddDeclares command\n");
  fflush(stdout);
}

void Ios::processViewAllTablesCommand(char* userName, int qTempSockfd)
{
  if(qTempSockfd > 0)
  {
    replyViewAllMessage(userName, activeDeclareModules, qTempSockfd, 
			DECLARE_TABLE_STMT);
    printf("Handled ViewAllTables command\n");
  }
}

void Ios::processViewAllStreamsCommand(char* userName, int qTempSockfd)
{
  if(qTempSockfd > 0)
  {
    replyViewAllMessage(userName, activeDeclareModules, qTempSockfd, 
			DECLARE_STREAM_STMT);
    printf("Handled ViewAllStreams command\n");
  }
}

void Ios::processViewDeclareModuleCommand(char* userName, int qTempSockfd, 
					  char* buf)
{
  if(qTempSockfd > 0)
  {
    replyViewModuleCommand(userName, buf, activeDeclareModules, qTempSockfd,
                           DECLARE_FILE_PATH, DECLARE_FILE_EXT);
    printf("Handled ViewDeclareModule command\n");
  }
}

void Ios::processViewDeclareCommand(char* userName, int qTempSockfd, char* buf)
{
  if(qTempSockfd > 0)
  {
    char* dName = strdup(buf);
    int size = activeDeclareModules.size();
    int done = 0;
    
    for(int i = 0; i < size; i++) {
      StmtModule dm = activeDeclareModules.operator[](i);
      if(strcmp(dm.getUserName(), userName) == 0) {
	vector<A_stmt> declareIds = dm.getStmtIdsVector();
	int vecSize = declareIds.size();
	for(int j = 0; j < vecSize; j++) {
	  A_stmt stmt = declareIds.operator[](j);
	  if(strcmp(stmt->id, dName) == 0) {
	    if(stmt->isDirectStream == 1) {
	      done = 1;
	      write(qTempSockfd, stmt->displayText, strlen(stmt->displayText));
	    }
	    goto found; //indicates that we saw the decalre
	  }
	}
      }
    }
    
  found:
    if(done == 0) {  //indicates that the displayText was not set
      char* declareFileName = (char*) malloc(MAX_STMT_PATH);
      
      sprintf(declareFileName, "%s%s%s", DECLARE_FILE_PATH, dName, DECLARE_FILE_EXT);
      
      sendFileText(declareFileName, qTempSockfd);
      free(declareFileName);
    }
    printf("Handled ViewDeclareDetails command\n");
  }
}

int Ios::deactivateStream(A_stmt stmt, char* userName, char* queryId) {
  if(stmt->isDirectStream == 1) {  
    int gotMessage = 0;
    int code;
    int rc = 0;
    char qName[4096];
    qName[0] = '\0';
    bufferMngr* bm = bufferMngr::getInstance();
    buffer* buff = bm->lookup("_ioBuffer");
    
    sendMessageToQueryBuf(DEACTIVATE_QUERY_CMD_CODE, queryId);
    while(!gotMessage) {
      while(buff->empty())
	usleep(100000);
      buff->get(code, (char*)qName);
      
      if(code == COMMAND_FAILED) {
	rc = 1;
	buff->pop();
	gotMessage = 1;
      }
      else if(code == COMMAND_SUCCESSFUL) {
	buff->pop();
	//updateStmtIsActive(QUERIES_TABLENAME, userName, queryId, 0);
	gotMessage = 1;
      }
      else {
	pollStdoutMessageBuffer();
      }
    }
    return rc;  
  }
  else {
   int size = activeIOmodules.size();
    for(int i = 0; i < size; i++) {
      A_iomodule m = activeIOmodules.operator[](i);
      if(m->buf && strcmp(m->buf->name, stmt->bufName) == 0) {
        (*(m->closeConnection))();
	m->isActive = 0;
	break;
      }
    }
  }
  return 0;
}

int Ios::activateStream(A_stmt stmt, char* userName, char* queryId) {
  //printf("got activate stream %s\n", queryId);
  //fflush(stdout);
  if(stmt->isDirectStream == 1) {
    int gotMessage = 0;
    int code;
    int rc = 0;
    char qName[4096];
    qName[0] = '\0';
    bufferMngr* bm = bufferMngr::getInstance();
    buffer* buff = bm->lookup("_ioBuffer");
    
    sendMessageToQueryBuf(ACTIVATE_QUERY_CMD_CODE, queryId);
    while(!gotMessage) {
      while(buff->empty()) {
	usleep(100000);
      }
      buff->get(code, (char*)qName);
      if(code == COMMAND_FAILED) {
	rc = 1;
	buff->pop();
	gotMessage = 1;
      }
      else if(code == COMMAND_SUCCESSFUL) {
	buff->pop();
	//updateStmtIsActive(QUERIES_TABLENAME, userName, queryId, 1);
	gotMessage = 1;
      }
      else if(code == ACTIVATED_QUERY) {
	buff->pop();
	A_stmt stmt = findStmtById(activeQueryModules, qName);
	if(stmt != NULL && stmt->isActive != 1) {
	  stmt->isActive = 1;
	  //updateStmtIsActive(QUERIES_TABLENAME, userName, qName, 1);
	}
	else if(stmt == NULL) {
	  stmt = findStmtById(activeDeclareModules, qName);
	  if(stmt != NULL && stmt->isDirectStream == 1 
	     && stmt->isActive != 1) {
	    stmt->isActive = 1;
	    //updateStmtIsActive(QUERIES_TABLENAME, userName, qName1, 1);
	  }
	}
      }
      else {
	pollStdoutMessageBuffer();
      }
    }
    return rc; 
  }
  else {
    printf("we are here %d\n", activeIOmodules.size());fflush(stdout);
    int size = activeIOmodules.size();
    for(int i = 0; i < size; i++) {
      A_iomodule m = activeIOmodules.operator[](i);
      printf("iomod %s %d %s %s\n", m->name, m->buf, (m->buf)?m->buf->name:"(null)",
      stmt->bufName);
      fflush(stdout);
      if(m->buf && strcmp(m->buf->name, stmt->bufName) == 0) {
	printf("activating IOBuf\n");
	fflush(stdout);
	m->isActive = 1;
	break;
      }
    }
  }
  return 0;
}

void Ios::activateStreamModule(char* userName, int qTempSockfd,
                               int index) {
  StmtModule sm = activeDeclareModules[index];
  vector<A_stmt> stmtIds = sm.getStmtIdsVector();
  int size = stmtIds.size();
  int rc = 0;
  int i;

  for (i = 0; i < size; i++) {
    A_stmt stmt = stmtIds[i];
    if(stmt->isActive == 0) {
      rc = activateStream(stmt, userName, stmt->id);
      if(rc != 0)
	break;
      stmt->isActive = 1;
    }
  }

  if(rc != 0) {
    int j = 0;
    for(j; j < i; j++) {
      A_stmt stmt = stmtIds[j];
      if(stmt->isActive == 1) {
        deactivateStream(stmt, userName, stmt->id);
        stmt->isActive = 0;
      }
    }
    char* errors = (char*)malloc(200);
    sprintf(errors, "Error: Unable to activate query %d.\n", i);
    write(qTempSockfd, errors, strlen(errors));
    free(errors);
  }
  else {
    //updateModuleIsActive(QUERIES_TABLENAME, userName, sm.getModuleId(), 1);
    activeDeclareModules[index].setIsActive(1);
  }
  printf("Handled ActivateStreamModule command\n");
}

void Ios::processActivateStreamModuleByNameCommand(char* userName, 
                            int qTempSockfd, char *buf) {
  int index = findModuleByName(userName, activeDeclareModules, buf);
  
  if(index < 0) {
    char* errors = "Error: Stream/Module not found.\n "
           "This is most probably an internal error.\n"
           "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
    return;
  }
  
  activateStreamModule(userName, qTempSockfd, index);
}

void Ios::processActivateStreamModuleCommand(char* userName, int qTempSockfd,
                                            char* buf) {
  int index = findModuleById(userName, activeDeclareModules, buf);
  
  if(index < 0) {
    char* errors = "Error: Stream/Module not found.\n "
           "This is most probably an internal error.\n"
           "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
    return;
  }
  
  activateStreamModule(userName, qTempSockfd, index);
}

void Ios::processActivateStreamCommand(char* userName, int qTempSockfd,
				      char* buf) {
  A_stmt stmt = findStmtById(activeDeclareModules, buf);

  if(stmt != NULL) {
    int rc = 0;
    if(stmt->isActive == 0) {
      rc = activateStream(stmt, userName, buf);
    }
    if(rc != 0) {
      char* errors = "Could not activate stream.\n"; 
      write(qTempSockfd, errors, strlen(errors));
    }
    else
      stmt->isActive = 1;
  }
  else {
    char* errors = "Error: Stream/Module not found.\n"
      "This is most probably an internal error. \n"
      "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
  }
  printf("Handled ActivateStream command\n");
}

void Ios::processDeactivateStreamModuleCommand(char* userName, int qTempSockfd,
					      char* buf) {
  int index = findModuleById(userName, activeDeclareModules, buf);

  if(index < 0) {
    char* errors = "Error: Stream/Module not found.\n"
      "This is most probably an internal error. \n"
      "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
    return;
  }
  
  StmtModule sm = activeDeclareModules[index];
  vector<A_stmt> stmtIds = sm.getStmtIdsVector();
  int size = stmtIds.size();
  int rc = 0;
  int i;

  for (i = 0; i < size; i++) {
    A_stmt stmt = stmtIds[i];
    if(stmt->isActive == 1) {
      rc = deactivateStream(stmt, userName, stmt->id);
      if(rc != 0)
	break;
      stmt->isActive = 0;
    }
  }

  if(rc != 0) {
    int j = 0;
    for(j; j < i; j++) {
      A_stmt stmt = stmtIds[j];
      if(stmt->isActive == 0) {
        activateStream(stmt, userName, stmt->id);
        stmt->isActive = 1;
      }
    }
    char* errors = (char*)malloc(200);
    sprintf(errors, "Error: Unable to deactivate stream %d.\n", i);
    write(qTempSockfd, errors, strlen(errors));
    free(errors);
  }
  else {
    //updateModuleIsActive(QUERIES_TABLENAME, userName, sm.getModuleId(), 0);
    activeDeclareModules[index].setIsActive(0);
  }
  printf("Handled DeactivateStreamModule command\n");
}

void Ios::processDeactivateStreamCommand(char* userName, int qTempSockfd,
					char* buf) {
  A_stmt stmt = findStmtById(activeDeclareModules, buf);
  if(stmt != NULL) {
    int rc = 0;
    if(stmt->isActive == 1) { 
      rc = deactivateStream(stmt, userName, buf);
    }
    if(rc != 0) {
      char* errors = "Error: Unable to deactivate stream.\n";
      write(qTempSockfd, errors, strlen(errors));
    }
    else
      stmt->isActive = 0;
  }
  else {
    char* errors = "Error: Stream/Module not found.\n"
      "This is most probably an internal error. \n"
      "Please exit the client and try again.\n";
    write(qTempSockfd, errors, strlen(errors));
  }
  printf("Handled DeactivateStream command\n");
}


int Ios::removeDeclareModuleByIndex(int index)
{
  StmtModule dm = activeDeclareModules.operator[](index);
  int rt = 0;
  bufferMngr* bm = bufferMngr::getInstance();
  buffer *buff = bm->lookup("_ioBuffer");

  vector<A_stmt> declareIds = dm.getStmtIdsVector();
  int size = declareIds.size();

  printf("StmtType %d %d\n", dm.getStmtType(), size);
  fflush(stdout);

  for(int i = 0; i < size; i++)
  {
    A_stmt stmt = declareIds.operator[](i);
    char* dName = stmt->id;
    char* dFileName = (char*) malloc(MAX_STMT_PATH);
    sprintf(dFileName, "%s%s%s", DECLARE_FILE_PATH, dName, DECLARE_FILE_EXT);


    if(stmt->isDirectStream) { //denotes derived stream and derived views
      Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmt->id);	

      // bm->kill(stmt->bufName);
      //instead need to tell querySchdler some how
      Ios::sendMessageToQueryBuf(DROP_DECLARE_CMD_CODE, stmt->bufName);
      int rc = 0;
      int code;
      int gotMessage = 0;
      
      while(gotMessage != 1) {
	while(buff->empty()) {
	  usleep(100000);
	}
	buff->get(code);
	//printf("got code %d\n", code);
	//fflush(stdout);
	
	if(code == FAILURE) {
	  rt = 1;
	  rc = 1;
	  gotMessage = 1;
	  buff->pop();
	}
	else if(code == SUCCESS) {
	  gotMessage = 1;
	  buff->pop();
	}
	else if(code == COMMAND_SUCCESSFUL || code == COMMAND_FAILED) {
	  buff->pop();
	}
	else {
          buff->pop();
	  printf("Internal Error: Got unexpected Code in removeDeclare %s %d\n", dName, code);
	}
      }
      if(rc != 0) {
	printf("got error in deleting windowed declare, it may not be restored correctly.\n");
	fflush(stdout);
	//YYY
      }
    }
    else if(stmt->bufName)
    {
      //printf("here sending %s\n", stmt->bufName);
      fflush(stdout);
      Ios::sendMessageToQueryBuf(DROP_DECLARE_CMD_CODE, stmt->bufName);
      int rc = 0;
      int code;
  
      while(buff->empty())
      {
        usleep(100000);
      }
      buff->get(code);

      if(code == FAILURE)
      {
        rt = 1;
        rc = 1;
        buff->pop();
      }
      else if(code == SUCCESS)
      {
        buff->pop();
      }
      else
      {
        printf("Internal Error: Got unexpected Code in removeDeclare %s %d\n", dName, code);
        fflush(stdout);
	buff->pop();
      }
    }
    free(dFileName);
  }

  if(rt == 0) {
    while(declareIds.size() != 0) 
    {
      A_stmt stmt = declareIds.operator[](0);
      char* dName = stmt->id;
      char* dFileName = (char*) malloc(MAX_STMT_PATH);
      sprintf(dFileName, "%s%s%s", DECLARE_FILE_PATH, dName, DECLARE_FILE_EXT);

      removeDeclareFromModuleAtIndex(index, 0);
      free(stmt);
      remove(dFileName);
      declareIds.erase(declareIds.begin());
    }
  }

  if(rt == 0)
  {
    StmtModule sm = activeDeclareModules[index];
    deleteModule(sm.getModuleId(), sm.getUserName(), DECLARES_TABLENAME);
    activeDeclareModules.erase(activeDeclareModules.begin()+index);
  }
  return rt;
}

void Ios::processDeleteDeclareModuleCommand(char* userName, int qTempSockfd, char* buf)
{
  char* moduleId = strdup(buf);

  int index = findModuleById(userName, activeDeclareModules, moduleId);
  printf("moduleId = %s-%d\n", moduleId, index);

  if(index == -1)
    return;

  int rc = removeDeclareModuleByIndex(index);

  
  if(rc == 0)
  {
    rewriteSystemDeclares(DECLARE_TABLE_STMT);
    rewriteSystemDeclares(DECLARE_STREAM_STMT);
  }
  else {
    char* errors = "Could not delete declare module. There may be active queries or windowed table(s) defined on the streams you tried to delete.\n";
    write(qTempSockfd, errors, strlen(errors));
  }

  printf("Handled DeleteDeclareModule command\n");
}

void Ios::removeDeclareFromModuleAtIndex(int index, int dIndex)
{
  StmtModule dm = activeDeclareModules.operator[](index);

  A_stmt stmt = dm.getStmtIdsVector()[dIndex];

  if(stmt->isDirectStream == 1 && dm.getStmtType() == DECLARE_STREAM_STMT)
    Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmt->id);

  deleteStmtInModule(dm.getModuleId(), dm.getUserName(), stmt->id, 
		     DECLARES_TABLENAME);


  dm.removeStmtAtIndex(dIndex);

  activeDeclareModules.erase(activeDeclareModules.begin()+index);
  activeDeclareModules.insert(activeDeclareModules.begin()+index, dm);
  return;
}

void Ios::processDeleteDeclareCommand(char* userName, int qTempSockfd, 
				      char* buf)
{
  char* moduleId = strdup(strsep(&buf, " "));
  char* declareId = strdup(buf);

  int index = findModuleById(userName, activeDeclareModules, moduleId);

  if(index == -1)
    return;

  bufferMngr *bm = bufferMngr::getInstance();
  buffer* buff = bm->lookup("_ioBuffer");

  StmtModule dm = activeDeclareModules.operator[](index);

  vector<A_stmt> moduleDeclares = dm.getStmtIdsVector();
  int size = moduleDeclares.size();
  int rc = 0;

  for(int i = 0; i< size; i++)
  {
    A_stmt stmt = moduleDeclares.operator[](i);
    char* dName = stmt->id;
    
    if(strcmp(declareId, dName) == 0)
    {
      char* declareFileName = (char*) malloc(MAX_STMT_PATH);
      sprintf(declareFileName, "%s%s%s", DECLARE_FILE_PATH, dName, DECLARE_FILE_EXT);

      if(dm.getStmtType() == DECLARE_TABLE_STMT && stmt->isDirectStream) {
	Ios::sendMessageToQueryBuf(DROP_QUERY_CMD_CODE, stmt->id);	

	// bm->kill(stmt->bufName);
	//instead need to tell querySchdler some how
	Ios::sendMessageToQueryBuf(DROP_DECLARE_CMD_CODE, stmt->bufName);
	int code;
	int gotMessage = 0;
	
	while(gotMessage != 1) {
	  while(buff->empty()) {
	    usleep(100000);
	  }
	  buff->get(code);
	  
	  if(code == FAILURE) {
	    rc = 1;
	    gotMessage = 1;
	    buff->pop();
	  }
	  else if(code == SUCCESS) {
	    gotMessage = 1;
	    buff->pop();
	  }
	  else if(code == COMMAND_SUCCESSFUL || code == COMMAND_FAILED) {
	    buff->pop();
	  }
	  else {
	    printf("Internal Error: Got unexpected Code in removeDeclare %s %d\n", dName, code);
	    buff->pop();
	  }
	}
	
	if(rc == 0) {
	    removeDeclareFromModuleAtIndex(index, i);
	    //this is a view //need to COPY
	    //need to remove the stmt from the driver
	    free(stmt);
	    //but the query for the delcare is in the corresponding .cq file ?? YYY
	    remove(declareFileName);
	}
	else if(rc != 0) {
	  printf("got error in deleting windowed declare, it may not be restored correctly.\n");
	  fflush(stdout);
	  //YYY
	}
      }
      else if(stmt->bufName) {
	//cout<<"ios:"<<endl;
	//bm->displayBufs();
        Ios::sendMessageToQueryBuf(DROP_DECLARE_CMD_CODE, stmt->bufName);
        int code;
 
        //because no checks when deleteing streams, instead check at new stream addition time
	while(buff->empty())
          usleep(100000);
	buff->get(code);
	
	if(code == FAILURE) {
	  rc = 1;
	  buff->pop();
	}
	else if(code == SUCCESS) {
	  buff->pop();
	}

        if(rc == 0) {
	  free(stmt);
          removeDeclareFromModuleAtIndex(index, i);
          remove(declareFileName);
        }
      }
      free(declareFileName);
      break;
    }
  }
  if(rc == 0)
  {
    rewriteSystemDeclares(DECLARE_TABLE_STMT);
    rewriteSystemDeclares(DECLARE_STREAM_STMT);
  }
  else {
    char* errors = "Could not delete declare module. There may be active queries or windowed table(s) defined on the streams you tried to delete.\n";
    write(qTempSockfd, errors, strlen(errors));
  }
  printf("Handled DeleteDeclare command\n"); 
}

char* replaceUnderscoreWDot(char* clientIp)
{
  int len = strlen(clientIp);
  for(int i=0; i < len; i++)
  {
    if(clientIp[i] == '_')
       clientIp[i] = '.';
  }
  return strdup(clientIp);
}


/*
 * Component Commands
 */
void Ios::processViewComponentsCommand(int qTempSockfd, char* buf)
{
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* ioBuf = bm->lookup("_ioBuffer");
  cDBT cdbt(MAX_STR_LEN);
  int rCode;
  char reply[MAX_STR_LEN];
  reply[0] = '\0';

  qBuf->put(GET_COMPONENTS);

  while(ioBuf->empty())
    usleep(100000);
  
  int rc = ioBuf->get(rCode, (char*)reply); 
  if(rc == 0)
  {
    ioBuf->pop();
    write(qTempSockfd, reply, strlen(reply));
  }
  else
    printf("ERROR: could not read ioBuffer in processViewComponentsCommand\n");

  printf("Handled ViewComponents command\n");
}

void Ios::processViewComponentDetailsCommand(int qTempSockfd, char* buf)
{
  char* compId = strdup(buf);
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* ioBuf = bm->lookup("_ioBuffer");
  cDBT cdbt(MAX_STR_LEN);
  char reply[MAX_STR_LEN];
  int rCode;

  qBuf->put(VIEW_COMPONENT_DETAILS, compId);

  while(ioBuf->empty())
    usleep(100000);

  int rc = ioBuf->get(rCode, reply);
  if(rc == 0)
  {
    ioBuf->pop();
    write(qTempSockfd, reply, strlen(reply));
  }
  else
    printf("ERROR: could not read ioBuffer in processViewComponentDetailsCommand\n");
  printf("Handled ViewComponentDetails command\n");
}

void Ios::processMoveStatementCommand(int qTempSockfd, char* buf)
{
  char* stmtId = strsep(&buf, " ");
  char* fromCompId = strsep(&buf, " ");
  char* toCompId = strdup(buf);
  int code = MOVE_STMT_TO_COMPONENT;
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* ioBuf = bm->lookup("_ioBuffer");
  cDBT cdbt(4*MAX_STMT_PATH);
  int ret = 0;

  memcpy(cdbt.data, &code, sizeof(int));
  strcpy(cdbt.data+sizeof(int), stmtId);
  strcpy(cdbt.data+sizeof(int)+strlen(stmtId)+1, fromCompId);
  strcpy(cdbt.data+sizeof(int)+strlen(stmtId)+1+strlen(fromCompId)+1, toCompId);

  qBuf->put(&cdbt);

  while(ioBuf->empty())
    usleep(100000);

  int rc = ioBuf->get(ret);
  if(rc == 0)
  {
    ioBuf->pop();
    if(ret == COMMAND_FAILED)
      write(qTempSockfd, "1\n", strlen("1\n"));
  }
  else
    printf("ERROR: could not read ioBuffer in processMoveStatementCommand\n");
  printf("Handled MoveStatement command\n");
}

void Ios::processBreakComponentCommand(int qTempSockfd, char* buf)
{
  char* compId = strsep(&buf, " ");
  char* bufList = strdup(buf);
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* ioBuf = bm->lookup("_ioBuffer");
  int ret = 0;
  int code = BREAK_COMPONENT;

  qBuf->put(code, compId, bufList);
  
  while(ioBuf->empty())
  {
    usleep(100000);
  }
  int rc = ioBuf->get(ret);
  if(rc == 0)
  {
    ioBuf->pop();
    if(ret == COMMAND_FAILED)
      write(qTempSockfd, "1\n", strlen("1\n"));
  }
  else
    printf("ERROR: could not read ioBuffer in processBreakComponentCommand\n");
  printf("Handled BreakComponent command\n");
}

void Ios::processMergeComponentsCommand(int qTempSockfd, char* buf)
{
  char* compId1 = strsep(&buf, " ");
  char* compId2 = strdup(buf);
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* ioBuf = bm->lookup("_ioBuffer");
  int ret = 0;

  qBuf->put(JOIN_COMPONENTS, compId1, compId2);

  while(ioBuf->empty())
    usleep(100000);

  int rc = ioBuf->get(ret);
  if(rc == 0)
  {
    ioBuf->pop();
    if(ret == COMMAND_FAILED)
      write(qTempSockfd, "1\n", strlen("1\n"));
  }
  else
    printf("ERROR: could not read ioBuffer in processMergeComponentsCommand\n");
  printf("Handled MergeComponents command\n");
}

void Ios::processSetComponentPriorityCommand(int qTempSockfd, char* buf) {
  char* compId = strsep(&buf, " ");
  char* priority = strdup(buf);
  int messageSize = sizeof(int) + MAX_ID_LEN + sizeof(int);
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* ioBuf = bm->lookup("_ioBuffer");
  cDBT cdbt(messageSize);
  int code = SET_COMPONENT_PRIORITY;
  int ret = 0;

  memset(cdbt.data, '\0', messageSize);
  memcpy(cdbt.data, &code, sizeof(int));
  strcpy((char*)cdbt.data+sizeof(int), compId);
  memcpy((char*)cdbt.data+sizeof(int)+strlen(compId)+1, priority, 
	 strlen(priority));

  qBuf->put(&cdbt);

  while(ioBuf->empty())
    usleep(100000);
  
  int rc = ioBuf->get(ret);
  if(rc == 0)
  {
    ioBuf->pop();
    if(ret == COMMAND_FAILED)
      write(qTempSockfd, "1\n", strlen("1\n"));
  }
  else
    printf("ERROR: could not read ioBuffer in processSetComponentPriorityCommand\n");
  printf("Handled SetComponentPriority command\n");

}

/*
 * One Tuple Test Command
 */
void Ios::processOneTupleTestCommand(char* userName, int qTempSockfd, 
				     char* buf) {
  char* queryFileName = strsep(&buf, "\n");
  char* queryFilePrefix = strtok(queryFileName, ".");
  char queFile[120];
  queFile[0] = '\0';
  FILE* file = NULL;

  sprintf(queFile, "%s.tq", queryFilePrefix);
  file = fopen(queFile, "w");

  if(!file) {
    write(qTempSockfd, "ERROR executing query, could not write to file\n", 
	  strlen("ERROR executing query, could not write to file\n"));
    perror("ERROR opening file");
    return;
  }

  fprintf(file, "%s\n", buf);
  fclose(file);

  char command[120];
  command[0] = 0;
  sprintf(command, "./eslt %s ../dcl/system ../dcl/tables ../aggr/aggr "
                   "%s > tmpOut",
	  userName, queryFilePrefix);
  int rc = system(command);
  if(rc != 0) {
    write(qTempSockfd, "ERROR executing query\n", 
	  strlen("ERROR executing query\n"));    
  }
  else {
    char* outBuf;
    rc = readFileIntoBuffer("./tmpOut", outBuf);
    if (rc) {
      write(qTempSockfd, "ERROR executing query, could not read output\n", 
	    strlen("ERROR executing query, could not read output\n"));
      return;
    }
    write(qTempSockfd, outBuf, strlen(outBuf));
    printf("Handled OneTupleTest command\n");
  }
  remove("./tmpOut");
}

/*
 * Snapshot Query Command
 */
void Ios::processSnapshotQueryCommand(char* userName, int qTempSockfd, 
				      char* buf) {
  char* queryFileName = strsep(&buf, "\n");
  char fName[MAX_STMT_PATH];
  fName[0] = '\0';

  //put it in the cq folder with cq extension
  sprintf(fName, "../cq/%s__%s.cq", userName, queryFileName);
  char* queryFilePrefix = strtok(queryFileName, ".");
  char qFileId[MAX_STMT_PATH];
  char *dclBuf;
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* ioBuf = bm->lookup("_ioBuffer");
  buffer* qBuf = bm->lookup("_queryBuffer");
  int code;
  char qName[4096];
  char id[10];
  int rc = 0;
  compiler *cc = compiler::getInstance();


  qFileId[0] = '\0';

  sprintf(qFileId, "%s__%s", userName, queryFilePrefix);
  
  FILE* fdesc = fopen(fName, "w");

  if(fdesc == NULL) {
    write(qTempSockfd, "ERROR executing query, could not write to file\n", 
	  strlen("ERROR executing query, could not write to file\n"));
    perror("ERROR opening file");
    return;
  }

  /* the compiler does this so we don't have to
    rc = readFileIntoBuffer("../dcl/tables.dcl", dclBuf);
    if (rc || !dclBuf) {
    EM_error(0, ERR_HISTORY, __LINE__, __FILE__, "ioSchdl::run", "readFileIntoBuffer");
    char* errorStr = "ERROR: Could not read system tables. At least one table/view"
    " must be defined for snapshot queries, if a table is" 
    " defined then this is probably an internal error,"
    " please try again and notify us if this happens again.\n";
    write(qTempSockfd, errorStr, strlen(errorStr));
    remove(fName);
    fclose(fdesc);
    return;
    }
    if(dclBuf) {
    fprintf(fdesc, "%s\n%s\n", dclBuf, buf);
    }
    else
  */

  fprintf(fdesc, "%s\n", buf);
  fclose(fdesc);

  char command[1024];
  command[0] = '\0';

  //instead use eslc with -s option (snap shot for compiler)
  //may be switch a to s and add that in eslc.cc
  //sprintf(command, "./eslc -s -u %s %s__%s >& errorsFile", userName, userName, 
  //queryFilePrefix);

  //printf("executing %s\n", command);
  //fflush(stdout);
  //int ret = system(command);

  
  //also have to play with ioBuf
  int gotMessage = 0;
  int ret = 0;
  char ccFile[MAX_STMT_PATH];
  char objFile[MAX_STMT_PATH];
  char exeFile[MAX_STMT_PATH];
  char outFile[MAX_STMT_PATH];
  exeFile[0] = 0;
  outFile[0] = 0;
  objFile[0] = 0;
  ccFile[0] = '\0';
  
  sprintf(ccFile, "../exe/%s.cc", qFileId);
  sprintf(objFile, "../exe/%s.so", qFileId);
  sprintf(exeFile, "./%s.so", qFileId);
  sprintf(outFile, "./%s.out", qFileId);

  //printf("sending snapshot cmd\n");
  //fflush(stdout);
  Ios::sendMessageToQueryBuf(SNAP_SHOT_CMD_CODE, qFileId);
  //printf("sending message\n");
  //fflush(stdout);
  int fd = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, 0644);
  //chmod(outFile, 0644);
  if(fd > 0)
    close(fd);

  while(gotMessage < 2) {
    while(ioBuf->empty()) {
      usleep(100000);
    }
    ioBuf->get(code, (char*)qName);
    if(code == COMPILE_FAILURE) {
      ret = 1; //error
      ioBuf->pop();
      gotMessage = 1;
    }
    else if(code == COMPILE_SUCCESS) {
      ioBuf->pop();
      //updateStmtIsActive(QUERIES_TABLENAME, userName, queryId, 1);
    }
    /*else if(code == ADHOC_QUERIES) {
      sprintf(id, "%s", qName); //signifies the number of statements in the adhoc query
      ioBuf->pop();
      //updateStmtIsActive(QUERIES_TABLENAME, userName, queryId, 1);
      }*/
    else if(code == USES_LIBRARY) {
      ioBuf->pop();
      if(!isUserPublic(qName)) {
	ret = 1;
      }
    } 
    else if(code == COMMAND_FAILED) {
	ioBuf->pop();
	char* errorStr = "ERROR: Could not execute snapshot query, "
	  "probably  an internal error. Please try again and report to us if "
	  "this happens again.\n";
	printf("%s\n", errorStr);
	//sendFileText(outFile, qTempSockfd);        
	write(qTempSockfd, errorStr, strlen(errorStr)); 
	ret = 1;
	gotMessage = 2;
    }
    else if(code == COMMAND_SUCCESSFUL) {
      ioBuf->pop();
      sendFileText(outFile, qTempSockfd);
      gotMessage = 2;
    }
    else {
      pollStdoutMessageBuffer();
    }
  }
  //remove(fName);
  //commented for testing
  //remove(ccFile);
  //remove(objFile);
  //remove(exeFile);
  //remove(outFile);

  printf("Handled Snapshot Query Command\n");
}


/*
 * Performance commands
 */

void Ios::processViewPerformancesCommand(char* userName, int qTempSockfd, 
				    char* clientIp)
{
  bufferMngr* bm = bufferMngr::getInstance();
  int size = stdoutBuffers->size();
  char fullMessage[4096];
  fullMessage[0] = '\0';

  for(int i = 0; i < size; i++)
  {
    A_stdoutBuf sb = stdoutBuffers->operator[](i);
    if(strcmp(sb->userName, "__user__") == 0)
    {
      char* bufName = sb->bufName;
      buffer* buf = bm->lookup(bufName);
      vector<GUIClient*> v;
      char message[100];
      message[0] = '\0';
      buf->getClients(v);
      int clSize = v.size();
      int j;
      for(j = 0; j < clSize; j++)
      {
	GUIClient* gc = v.operator[](j);
	if(strcmp(gc->getClientIp(), clientIp) == 0)
	{
	  sprintf(message, "yes %s\n", bufName);
	  break;
	}
      }
      if(j >= clSize)
	sprintf(message, "no %s\n", bufName);
      
      strcat(fullMessage, message);
    }
  }
  write(qTempSockfd, fullMessage, strlen(fullMessage));
  printf("Handled ViewPerformances command\n");
}

void Ios::processMonitorPerformancesCommand(char* bufstr, char* clientIp)
{
  char* temp = strdup(bufstr);
  char* bufName = strsep(&temp, " ");
  if(!bufName)
    bufName = bufstr;
  
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buf = bm->lookup(strdup(bufName));
  if(buf) 
    buf->addClient(new GUIClient(clientIp, OUT_PORT));

  vector<GUIClient*> v;
  buf->getClients(v);
  if(v.size()== 1) {
    Ios::sendMessageToQueryBuf(MONITOR_PERFORMANCE_BUFFER, bufName);
  }
  
  printf("Handled MonitorPerformances command\n");
}

void Ios::processUnMonitorPerformancesCommand(char* bufstr, char* clientIp)
{
  char* temp = strdup(bufstr);
  char* bufName = strsep(&temp, " ");
  if(!bufName)
    bufName = bufstr;

  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buf = bm->lookup(strdup(bufName));
  if(buf) {
    GUIClient* gc = new GUIClient(clientIp, OUT_PORT);
    buf->dropClient(gc);
    free(gc);
  }

  vector<GUIClient*> v;
  buf->getClients(v);
  if(v.size()<= 1) {
    Ios::sendMessageToQueryBuf(UNMONITOR_PERFORMANCE_BUFFER, bufName);
  }

  printf("Handled UnMonitorPerformances command\n");
}


/*
 * Buffer commands
 */

void Ios::processViewBuffersCommand(char* userName, int qTempSockfd, 
				    char* clientIp)
{
  bufferMngr* bm = bufferMngr::getInstance();
  int size = stdoutBuffers->size();
  char fullMessage[4096];
  fullMessage[0] = '\0';

  for(int i = 0; i < size; i++)
  {
    A_stdoutBuf sb = stdoutBuffers->operator[](i);
    if(strcmp(sb->userName, userName) == 0 && sb->type == sbt_simple)
    {
      char* bufName = sb->bufName;
      buffer* buf = bm->lookup(bufName);
      vector<GUIClient*> v;
      char message[100];
      message[0] = '\0';
      buf->getClients(v);
      int clSize = v.size();
      int j;
      for(j = 0; j < clSize; j++)
      {
	GUIClient* gc = v.operator[](j);
	if(strcmp(gc->getClientIp(), clientIp) == 0)
	{
	  sprintf(message, "yes %s\n", bufName);
	  break;
	}
      }
      if(j >= clSize)
	sprintf(message, "no %s\n", bufName);
      
      strcat(fullMessage, message);
    }
  }
  write(qTempSockfd, fullMessage, strlen(fullMessage));
  printf("Handled ViewBuffers command\n");
}

void Ios::processMonitorBufferCommand(char* bufstr, char* clientIp)
{
  char* temp = strdup(bufstr);
  char* bufName = strsep(&temp, " ");
  if(!bufName)
    bufName = bufstr;
  
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buf = bm->lookup(strdup(bufName));
  if(buf)
    buf->addClient(new GUIClient(clientIp, OUT_PORT));
  printf("Handled MonitorBuffer command\n");
}

void Ios::processUnMonitorBufferCommand(char* bufstr, char* clientIp)
{
  char* temp = strdup(bufstr);
  char* bufName = strsep(&temp, " ");
  if(!bufName)
    bufName = bufstr;

  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buf = bm->lookup(strdup(bufName));
  if(buf) {
    GUIClient* gc = new GUIClient(clientIp, OUT_PORT);
    buf->dropClient(gc);
    free(gc);
  }
  printf("Handled UnMonitorBuffer command\n");
}

void Ios::processUnMonitorAllOfIPCommand(char* buf1, char* clientIp) {
   //loop through stdoutBuffers
   //for all issue drop Client
   //if userName for stdout buffer == __User__, then it is perf buffer and
   //thus send deactivate if no more clients
  int size = stdoutBuffers->size();
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* buf = NULL;
  GUIClient *gc = new GUIClient(clientIp, OUT_PORT);

  for(int i = 0; i < size; i++)
  {
    A_stdoutBuf sb = stdoutBuffers->operator[](i);
    buf = bm->lookup(strdup(sb->bufName));
    if(buf) {
      buf->dropClient(gc);
      if(strcmp("__user__", sb->userName)==0) {
        vector<GUIClient*> v;
        buf->getClients(v);
        if(v.size()<= 1) {
          Ios::sendMessageToQueryBuf(UNMONITOR_PERFORMANCE_BUFFER, sb->bufName);
        }
      }
    }
  }
  free(gc);
}

/*
 * Username coammnds
 */

char* Ios::getUserPassword(char* userName)
{
  int size = users.size();

  for(int i = 0; i < size; i++)
  {
    A_user user = users[i];
    
    if(strcmp(user->userName, userName) == 0)
      return user->password;
  }
  return NULL;
}

void Ios::processDoesUserNameExistCommand(int qTempSockfd, char* buf)
{
  char* userName = strdup(buf);
  char* password = getUserPassword(userName);

  if(password == NULL)
  {
    write(qTempSockfd, "1\n", strlen("1\n"));
  }
  printf("Handled doesUserNameExist command\n");
}

int Ios::changeUserType(char* user, int isPublic)
{
  int size = users.size();
  for(int i = 0 ; i < size; i++) {
    A_user u = users[i];
    if(strcmp(user, u->userName) == 0) {
      u->isPublic = isPublic;
      changeUserTypeInPersistentTable(user, isPublic);
      return 0;
    }
  }
  return -1;
}

void Ios::processMakeUserPublicCommand(int qTempSockfd, char* buf)
{
  int rc = changeUserType(buf, 1);
  if(rc < 0) {
    write(qTempSockfd, "1\n", strlen("1\n"));
  }
  printf("Handled MakeUserPublic Command\n");
}

void Ios::processMakeUserPrivateCommand(int qTempSockfd, char* buf)
{
  int rc = changeUserType(buf, 0);
  if(rc < 0) {
    write(qTempSockfd, "1\n", strlen("1\n"));
  }
  printf("Handled MakeUserPrivate Command\n");
}


void Ios::processAddNewUserCommand(int qTempSockfd, char* buf)
{
  char* userName = strsep(&buf, " ");
  char* email = strsep(&buf, " ");
  char* password = strdup(buf);
  char errorBuf[1024];

  sprintf(errorBuf, "%s_errors", userName);
  addUser(A_User(userName, email, password, 0)); //TODO for now private user

  bufferMngr* bm = bufferMngr::getInstance();
  bm->create(errorBuf, SHARED);
  stdoutBuffers->push_back(A_StdoutBuf(strdup(userName), 
				       strdup(errorBuf)));

  printf("Added user %s, password %s\n", userName, password);  
  printf("Handled addNewUser command\n");
}

void Ios::processAuthenticateUserCommand(int qTempSockfd, char* buf)
{
  char* userName = strsep(&buf, " ");
  char* password = strdup(buf);

  char* stPassword = getUserPassword(userName);

  printf("Handled authenticateUser command\n");

  if(stPassword == NULL || strcmp(stPassword, password) != 0)
  {
    write(qTempSockfd, "1\n", strlen("1\n"));
    return;
  }
}


void Ios::processViewLibCommand(int qTempSockfd, char* buf)
{
  char reply[2048];
  reply[0] = '\0';
  int size = users.size();
  int first = 1;

  for(int i = 0; i < size; i++) {
    A_user u = users[i];
    if(u->isPublic) {
      if(first) {
	first = 0;
      }
      else {
	strcat(reply, "||");
      }
      strcat(reply, u->userName);
    }
  }
  write(qTempSockfd, reply, strlen(reply));
  printf("Handled ViewLib Command\n");
}


void Ios::processCommand(int qTempSockfd, char* buf, char* clientIp)
{
  bufferMngr* bm = bufferMngr::getInstance();
  buffer* qBuf = bm->lookup("_queryBuffer");
  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  char clientIpAddr[80];
  clientIpAddr[0] = '\0';
  sprintf(clientIpAddr, "a%s", clientIp);

  char* command = strsep(&buf, " ");
  char* userName = strsep(&buf, " ");

  qBuf->put(SET_USER_NAME, userName);
  setUserName(userName);
  printf("Got Command %s from user %s, at %s", command, 
	 userName, ctime(&tv.tv_sec));
  fflush(stdout);
  if(strcmp(command, ADD_QUERIES_COMMAND) == 0)
  {
    processAddQueriesCommand(userName, qTempSockfd, buf, clientIpAddr, 
			     QUERY_STMT);
  }
  else if(strcmp(command, VIEWALL_QUERIES_COMMAND) == 0)
  {
    processViewAllQueriesCommand(userName, qTempSockfd);
  }
  else if(strcmp(command, VIEW_QUERY_MODULE_COMMAND) == 0)
  {
    processViewQueryModuleCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, VIEW_QUERY_COMMAND) == 0)
  {
    processViewQueryCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, ACTIVATE_QUERY_MODULE_BY_NAME_COMMAND) == 0)
  {
    processActivateQueryModuleByNameCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, ACTIVATE_QUERY_MODULE_COMMAND) == 0)
  {
    processActivateQueryModuleCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, ACTIVATE_QUERY_COMMAND) == 0)
  {
    processActivateQueryCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DEACTIVATE_QUERY_MODULE_COMMAND) == 0)
  {
    processDeactivateQueryModuleCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DEACTIVATE_QUERY_COMMAND) == 0)
  {
    processDeactivateQueryCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DELETE_QUERY_MODULE_COMMAND) == 0)
  {
    processDeleteQueryModuleCommand(userName, buf);
  }
  else if(strcmp(command, DELETE_QUERY_COMMAND) == 0)
  {
    processDeleteQueryCommand(userName, buf);
  }
  else if(strcmp(command, ADD_TABLES_COMMAND) == 0)
  {
    processAddDeclaresCommand(userName, qTempSockfd, buf, 
			      clientIpAddr, DECLARE_TABLE_STMT);
  }
  else if(strcmp(command, ADD_STREAMS_COMMAND) == 0)
  {
    processAddDeclaresCommand(userName, qTempSockfd, buf, clientIpAddr, 
			      DECLARE_STREAM_STMT);
  }
  else if(strcmp(command, VIEWALL_TABLES_COMMAND) == 0)
  {
    processViewAllTablesCommand(userName, qTempSockfd);
  }
  else if(strcmp(command, VIEWALL_STREAMS_COMMAND) == 0)
  {
    processViewAllStreamsCommand(userName, qTempSockfd);
  }
  else if(strcmp(command, VIEW_DECLARE_MODULE_COMMAND) == 0)
  {
    processViewDeclareModuleCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, VIEW_DECLARE_COMMAND) == 0)
  {
    processViewDeclareCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, ACTIVATE_STREAM_MODULE_BY_NAME_COMMAND) == 0)
  {
    processActivateStreamModuleByNameCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, ACTIVATE_STREAM_MODULE_COMMAND) == 0)
  {
    processActivateStreamModuleCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, ACTIVATE_STREAM_COMMAND) == 0)
  {
    processActivateStreamCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DEACTIVATE_STREAM_MODULE_COMMAND) == 0)
  {
    processDeactivateStreamModuleCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DEACTIVATE_STREAM_COMMAND) == 0)
  {
    processDeactivateStreamCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DELETE_DECLARE_MODULE_COMMAND) == 0)
  {
    processDeleteDeclareModuleCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DELETE_DECLARE_COMMAND) == 0)
  {
    processDeleteDeclareCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, ADD_MODEL_COMMAND) == 0)
  {
    processAddModelCommand(userName, qTempSockfd, buf, clientIpAddr);
  }
  else if(strcmp(command, VIEWALL_MODELS_COMMAND) == 0)
  {
    processViewAllModelsCommand(userName, qTempSockfd);
  }
  else if(strcmp(command, VIEW_MODEL_COMMAND) == 0)
  {
    processViewModelCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DELETE_MODEL_COMMAND) == 0)
  {
    processDeleteModelCommand(userName, buf);
  }
  else if(strcmp(command, ADD_AGGREGATE_COMMAND) == 0)
  {
    processAddAggregateCommand(userName, qTempSockfd, buf, clientIpAddr);
  }
  else if(strcmp(command, ADD_EXT_AGGREGATE_COMMAND) == 0)
  {
    processAddExtAggregateCommand(userName, qTempSockfd, buf, clientIpAddr);
  }
  else if(strcmp(command, VIEWALL_AGGREGATES_COMMAND) == 0)
  {
    processViewAllAggregatesCommand(userName, qTempSockfd);
  }
  else if(strcmp(command, VIEW_AGGREGATE_COMMAND) == 0)
  {
    processViewAggregateCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DELETE_AGGREGATE_COMMAND) == 0)
  {
    processDeleteAggregateCommand(userName, buf);
  }
  else if(strcmp(command, ADD_EXTERN_COMMAND) == 0)
  {
    processAddExternCommand(userName, qTempSockfd, buf, clientIpAddr);
  }
  else if(strcmp(command, VIEWALL_EXTERNS_COMMAND) == 0)
  {
    processViewAllExternsCommand(userName, qTempSockfd);
  }
  else if(strcmp(command, VIEW_EXTERN_COMMAND) == 0)
  {
    processViewExternCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DELETE_EXTERN_COMMAND) == 0)
  {
    processDeleteExternCommand(userName, buf);
  }
  else if(strcmp(command, VIEW_ALL_COMMAND) == 0)
  {
    processViewAllCommand(userName, qTempSockfd);
  }
  else if(strcmp(command, VIEW_BUFFERS_COMMAND) == 0)
  {
    processViewBuffersCommand(userName, qTempSockfd, 
			      replaceUnderscoreWDot(clientIp));
  }
  else if(strcmp(command, VIEW_PERFORMANCES_COMMAND) == 0) {
    processViewPerformancesCommand(userName, qTempSockfd, 
				   replaceUnderscoreWDot(clientIp));
  }
  else if(strcmp(command, ONE_TUPLE_TEST_COMMAND) == 0) {
    processOneTupleTestCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, SNAPSHOT_QUERY_COMMAND) == 0) {
    processSnapshotQueryCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, MONITOR_BUFFER_COMMAND) == 0)
  {
    processMonitorBufferCommand(buf, replaceUnderscoreWDot(clientIp));
  }
  else if(strcmp(command, UNMONITOR_BUFFER_COMMAND) == 0)
  {
    processUnMonitorBufferCommand(buf, replaceUnderscoreWDot(clientIp));
  }
  else if(strcmp(command, UNMONITOR_ALL_OF_IP_COMMAND) == 0)
  {
    processUnMonitorAllOfIPCommand(buf, replaceUnderscoreWDot(clientIp));
  }
  else if(strcmp(command, MONITOR_PERFORMANCES_COMMAND) == 0)
  {
    processMonitorPerformancesCommand(buf, replaceUnderscoreWDot(clientIp));
  }
  else if(strcmp(command, UNMONITOR_PERFORMANCES_COMMAND) == 0)
  {
    processUnMonitorPerformancesCommand(buf, replaceUnderscoreWDot(clientIp));
  }
  else if(strcmp(command, ADD_IOMODULE_COMMAND) == 0)
  {
    processAddIOModuleCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, VIEW_IOMODULES_COMMAND) == 0)
  {
    processViewIOModulesCommand(userName, qTempSockfd);
  }
  else if(strcmp(command, VIEW_IOMODULE_COMMAND) == 0)
  {
    processViewIOModuleCommand(userName, buf, qTempSockfd);
  }
  else if(strcmp(command, DROP_IOMODULE_COMMAND) == 0)
  {
    processDropIOModuleCommand(userName, buf);
  }
  /*else if(strcmp(command, ACTIVATE_IOMODULE_COMMAND) == 0)
    {
    processActivateIOModuleCommand(userName, qTempSockfd, buf);
    }
    else if(strcmp(command, DEACTIVATE_IOMODULE_COMMAND) == 0)
    {
    processDeactivateIOModuleCommand(userName, buf);
    }*/
  else if(strcmp(command, VIEW_COMPONENTS_COMMAND) == 0)
  {
    processViewComponentsCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, VIEW_COMPONENT_DETAILS_COMMAND) == 0)
  {
    processViewComponentDetailsCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, MOVE_STATEMENT_COMMAND) == 0)
  {
    processMoveStatementCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, BREAK_COMPONENT_COMMAND) == 0)
  {
    processBreakComponentCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, MERGE_COMPONENTS_COMMAND) == 0)
  {
    processMergeComponentsCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, SET_COMPONENT_PRIORITY_COMMAND) == 0)
  {
    processSetComponentPriorityCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, DOES_USERNAME_EXIST_COMMAND) == 0)
  {
    processDoesUserNameExistCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, ADD_NEW_USER_COMMAND) == 0)
  {
    processAddNewUserCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, AUTHENTICATE_USER_COMMAND) == 0)
  {
    processAuthenticateUserCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, MAKE_USER_PUBLIC_COMMAND) == 0)
  {
    processMakeUserPublicCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, MAKE_USER_PRIVATE_COMMAND) == 0)
  {
    processMakeUserPrivateCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, VIEW_LIB_COMMAND) == 0)
  {
    processViewLibCommand(qTempSockfd, buf);
  }
  else if(strcmp(command, ADD_TS_QUERY_COMMAND) == 0)
  {
    processAddTSQueryCommand(userName, qTempSockfd, buf, clientIpAddr);
  }
  else if(strcmp(command, VIEWALL_TS_QUERIES_COMMAND) == 0)
  {
    processViewAllTSQueriesCommand(userName, qTempSockfd);
  }
  else if(strcmp(command, VIEW_TS_QUERY_COMMAND) == 0)
  {
    processViewTSQueryCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, ACTIVATE_TS_QUERY_COMMAND) == 0)
  {
    processActivateTSQueryCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DEACTIVATE_TS_QUERY_COMMAND) == 0)
  {
    processDeactivateTSQueryCommand(userName, qTempSockfd, buf);
  }
  else if(strcmp(command, DELETE_TS_QUERY_COMMAND) == 0)
  {
    processDeleteTSQueryCommand(userName, buf);
  }

  //printf("done here\n");
  //fflush(stdout);

  /* Whatever the command is close the current connection at the end*/
  close(qTempSockfd);
  qTempSockfd = -1;
}

char* replaceDotWUnderscore(char* clientIp)
{
  int len = strlen(clientIp);
  for(int i=0; i < len; i++)
  {
    if(clientIp[i] == '.')
       clientIp[i] = '_';
  }
  return strdup(clientIp);
}

int Ios::readCommand()
{
  int rc = 0;

  if(qReadSockfd < 0 && qListenSockfd < 0)
    rc = initListenSock();

  //printf("HERE rc %d\n", rc);
  //fflush(stdout);

  if(rc == -1)
    return -1;                  // Error establishing connection

  if(qReadSockfd < 0)
    acceptConnection();

  //printf("HERE readSockfd %d\n", qReadSockfd);
  //fflush(stdout);
  if(qReadSockfd < 0)
  {
    return 2;                   //No data
  }

  char* ipAddr = inet_ntoa(qListenSock.sin_addr);
  int qTempSockfd = qReadSockfd;

  char buf[MAX_BUFFER];
  
  int olen = read(qTempSockfd, buf, sizeof(buf));
  
  //printf("HERE olen %d\n", olen);
  //fflush(stdout);

  if (olen == -1 && errno == EAGAIN)
    return 2;                   // No data
  
  if(olen == 0)
  {
    qTempSockfd = -1;
    return 1;                   // Connection Closed
  }
  buf[olen] = '\0';
  
  processCommand(qTempSockfd, buf, replaceDotWUnderscore(ipAddr));
  qReadSockfd = -1;
  return 0;
}

int Ios::simpleTest(){  
  bufferMngr* bm = bufferMngr::getInstance();
  static timeval tv;
  int datasz = MAX_STR_LEN;
  char data[MAX_STR_LEN];
  int i = 1234;
  memcpy(data, &i, sizeof(int));
  gettimeofday(&tv,NULL);
  cDBT cdbt(data, datasz, &tv);
  bm->put("st", &cdbt);
  bm->put("six", &cdbt);
}
