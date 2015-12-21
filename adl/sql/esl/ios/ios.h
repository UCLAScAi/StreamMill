#ifndef __IOS_H__
#define __IOS_H__

#include <sys/socket.h>
#include <netinet/in.h>

#include "../dbt.h"
#include "../buffer.h"
#include <adllib.h>
#include <ext/hash_map>

using namespace __gnu_cxx;

#include <vector>
using namespace ESL;
using namespace std;

/*-- return codes. --*/
#define ADD_STDOUT_COMMAND 0
#define DROP_STDOUT_COMMAND 1
#define CONNECT_IOMODULE 2
#define COMPILE_SUCCESS 4
#define COMPILE_FAILURE 5
#define COMMAND_FAILED 6
#define COMMAND_SUCCESSFUL 7
#define USES_LIBRARY 8
#define ACTIVATED_QUERY 9
#define ADD_BUILTIN_IOMOD 10
#define ADD_TABLE_DEC 11
#define ADD_STREAM_DEC 12
#define ADHOC_QUERIES 13
#define ADD_TARGET_STREAM 14

/*-- commands  --*/
#define ADD_QUERY_CMD_CODE 1
#define ACTIVATE_QUERY_CMD_CODE 2
#define ADD_DECLARE_CMD_CODE 3
#define DEACTIVATE_QUERY_CMD_CODE 4
#define DROP_QUERY_CMD_CODE 5
#define DROP_DECLARE_CMD_CODE 6
#define ADD_AGGREGATE_CMD_CODE 7
#define SET_USER_NAME 8
/*
 * Component Commands
 *
 * Ios always sends commands through "_queryBuffer"
 * QuerySchdler always sends commands through "_ioBuffer"
 */

#define GET_COMPONENTS 9        //Ios puts this command
//In response QuerySchdler sends back list of
//  component ids. Separator "||".

#define VIEW_COMPONENT_DETAILS 10 //Ios puts this command
//  with argument c_id (char*).
//In response QuerySchdler sends back list of
//  stmt Ids and buffer Ids in the component
//  Separator "||". Stmts followed by buffers.
//  Stmt and buffer Separator "::"

#define MOVE_STMT_TO_COMPONENT 11 //Ios puts this command
//  with arguments stmtId, c_idFrom,
//  cid_To
//In response, QuerySchdler moves the stmt
//  and sends back SUCCESS or FAILURE

#define BREAK_COMPONENT 12       //Ios puts this command
//  with arguments c_id, bufferList
//  Separator "||"
//In response, QuerySchdler breaks the component
//  and sends back SUCCESS or FAILURE

#define JOIN_COMPONENTS 13      //Ios puts this command
//  with arguments c_id1, c_id2
//In response, QuerySchdler joins the components
//  and sends back SUCCESS or FAILURE
#define SET_COMPONENT_PRIORITY 14

#define SNAP_SHOT_CMD_CODE 15

#define MONITOR_PERFORMANCE_BUFFER 16
#define UNMONITOR_PERFORMANCE_BUFFER 17

#define ADDED_BUFFER 20
#define ADDED_STREAM_BUFFER 21

#define OFFLINE_MODE 22

#define MAX_ID_LEN 50

typedef struct A_iomodule_ *A_iomodule;
struct A_iomodule_ {
  buffer* buf;
  char* name;
  int isActive;
  char* userName;
  char* fileName;
  void* handle;

  int (*getTuple)(buffer*, bufferMngr*);
  int (*closeConnection)();
};

A_iomodule A_IOmodule(buffer* buf, char* name, char* fileName, int isActive,
    char* userName);

typedef struct A_user_ *A_user;
struct A_user_ {
  char* userName;
  char* email;
  char* password;
  int isPublic;
};

A_user A_User(char* userName, char* email, char* password, int isPublic);

typedef struct A_stdoutBuf_ *A_stdoutBuf;
typedef enum {
  sbt_simple = 0, sbt_target
} sb_t;

struct A_stdoutBuf_ {
  char* userName;
  char* bufName;
  sb_t type;
  char* machine;
  int portNum;
};

A_stdoutBuf A_StdoutBuf(char* userName, char* bufName, sb_t type = sbt_simple,
    char* machine = NULL, int portNum = 0);

typedef struct A_stmt_ *A_stmt;
struct A_stmt_ {
  int isActive;
  char* id;
  char* userName;
  char* bufName;
  int isDirectStream;
  char* displayText;
};

A_stmt A_Stmt(int isActive, char* id, char* userName, char* buf = NULL,
    int isDirectStream = 0, char* displayText = NULL);

class StmtModule {
  int isActive;
  char* moduleName;
  char* moduleId;
  char* userName;
  vector<A_stmt> stmtIds;
  int stmtType;
  int hidden;

public:
  StmtModule();
  StmtModule(int isActive, char* moduleName, char* moduleId,
      vector<A_stmt> stmtIds, char* userName, int stmtType = 0, int hidden = 0);
  int getIsActive();
  void setIsActive(int);
  char* getModuleName();
  char* getModuleId();
  char* getUserName();
  int getStmtType();
  int isHidden();
  vector<A_stmt> getStmtIdsVector();
  void removeStmtAtIndex(int index);
  void addStmt(A_stmt stmt);

};

/*File IO*/
// Writes to the log of a particular user.
static void WriteToLog(const char* to_write, const char* fileName) {
  FILE* fdesc;
  fdesc = fopen(fileName, "a");
  fwrite(to_write, sizeof(char) * strlen(to_write), 1, fdesc);
  fclose(fdesc);
}

class Ios {

  // Is set to true if the SMM server is running in a standalone mode (i.e.
  // without discovery).
  bool isStandalone;

  // Represents a discovery server address.
  char* myDiscovery;

  // Will contains the IP address of the localhost.
  char localhost_addr[128];

  char* currently_loggedin_user;

  char* iosLog;

  FILE* stdoutLog;
  FILE* stderrLog;

  vector<A_user> users;
  vector<char*> hosts;
  hash_map<char*, char*> discovery_users_map;

  vector<A_iomodule> activeIOmodules;
  int myPORT;

  int qReadSockfd;
  int qListenSockfd;
  struct sockaddr_in qListenSock;
  static buffer* qBuf;
  static buffer* stdoutMessageBuffer;

  vector<StmtModule> activeQueryModules;
  vector<StmtModule> activeDeclareModules;
  vector<StmtModule> activeAggregateModules;
  vector<StmtModule> activeModelModules;
  vector<StmtModule> activeExternModules;
  vector<StmtModule> activeTSQueryModules;
  vector<A_stdoutBuf>* stdoutBuffers;

  static Ios* _instance;

  int init();

  // Copies the address of a localhost into addr.
  void getLocalHostAddr(char* addr);

  void sendObjectAlreadyExistsMessage(int qTempSockFd, char* objName, char* id);
  bool doesModuleExist(char* name, vector<StmtModule> moduleV);
  void addViewDecModule(char* userName, char* name, char* buf);
  void removeViewDecModule(char* userName, char* name);

  int addStdoutBuf(char*, const buffer*);
  int dropStdoutBuf(char*, const buffer*);
  void pollStdoutMessageBuffer();
  void sendOutputToGUIClients();

  void printErrorMessage(char*, int);

  /* Persistance */
  void addUser(A_user user);
  void removeHostPersistentTable(char* host_to_remove);
  void printHosts();
  void readPersistentUsers();
  void readPersistentHosts();
  int addIOModule(A_iomodule, char*, char*&);
  int prepareIOModule(A_iomodule, char*);
  void readPersistentIOModules();
  void removeIOModuleFromPersistentTable(char*, char*);
  void removeIOModule(char*, char*);
  void changeUserTypeInPersistentTable(char*, int);
  void addModule(StmtModule, char*, vector<StmtModule>&);
  void readPersistentModules(char*, vector<StmtModule>&);
  void readPersistentDecModules(char*, vector<StmtModule>&, int);
  void updateModuleIsActive(char*, char*, char*, int);
  void updateStmtIsActive(char*, char*, char*, int isActive);
  void deleteStmtInModule(char*, char*, char*, char*);
  void deleteModule(char*, char*, char*);

  /* Discovery commands */
  void processAddUserToDiscoveryCommand(char*, char*);
  void processRemoveUserFromDiscoveryCommand(int, char*);
  void processAddNewHostCommand(int, char*);
  void processRemoveHostCommand(char*);
  void addUserToDiscovery(char* user, char* host);
  void removeUserFromDiscovery(char* user);
  void readPersistentDiscoveryUsers();
  void printUsersInDiscovery();
  // pingHost is used to verify the existence of a host, so that in case
  // we are trying to add a user which is already in the DB, we first check
  // if the corresponding host is functioning and is on the network, and if
  // not then we replace the dead host / user with the new host / user.
  bool pingHost(char* host);
  void addHost(char* host);
  // This method returns the least loaded server when regestering
  // a new user.
  void processPickLeastLoadedServer(int, char*);
  // This method returns the host which contains the regiestered user.
  void processGetHostWithUsername(int, char*);

  /* UserName commands*/
  char* getUserPassword(char*);
  void processDoesUserNameExistCommand(int, char*);
  void processAddNewUserCommand(int, char*);
  void processAuthenticateUserCommand(int, char*);
  int prependUserName(char*, char*, char*&);
  int prependUserNameForAggr(char*, char*, char*, int&, char*&);
  int prependUserNameForModel(char*, char*, char*, char*&);
  int isUserPublic(char*);
  void processMakeUserPublicCommand(int, char*);
  void processMakeUserPrivateCommand(int, char*);
  int changeUserType(char*, int);

  void processViewLibCommand(int, char*);

  /*Component commands*/
  void processViewComponentsCommand(int, char*);
  void processViewComponentDetailsCommand(int, char*);
  void processMoveStatementCommand(int, char*);
  void processBreakComponentCommand(int, char*);
  void processMergeComponentsCommand(int, char*);
  void processSetComponentPriorityCommand(int, char*);

  /*IOMOdule commands*/
  void processViewIOModulesCommand(char*, int);
  void processViewIOModuleCommand(char*, char*, int);
  void processAddIOModuleCommand(char*, int, char* buf);
  void processDropIOModuleCommand(char*, char* buf);
  void processActivateIOModuleCommand(char*, int, char* buf);
  void processDeactivateIOModuleCommand(char*, char* buf);
  bool doesIOModuleExist(char*);
  void updateIOModuleBuffer(char*, char*);

  /* for listening on query port */
  int initListenSock();
  void acceptConnection();
  void processCommand(int, char*, char*);

  /*Buffers*/
  void processViewBuffersCommand(char*, int, char* clientIp);
  void processMonitorBufferCommand(char* buf, char* clientIp);
  void processMonitorAllOfIPCommand(char* buf, char* clientIp);
  void processUnMonitorBufferCommand(char* buf, char* clientIp);
  void processUnMonitorAllOfIPCommand(char* buf, char* clientIp);

  /*Performances*/
  void processViewPerformancesCommand(char*, int, char* clientIp);
  void processMonitorPerformancesCommand(char* buf, char* clientIp);
  void processUnMonitorPerformancesCommand(char* buf, char* clientIp);

  /*OneTupleTest Queries*/
  void processOneTupleTestCommand(char*, int, char*);

  /*Snapshot Queries*/
  void processSnapshotQueryCommand(char*, int, char*);

  /*Queries*/
  void activateQueryModule(char*, int, int);
  void processAddQueriesCommand(char*, int, char*, char*, int);
  void processViewAllQueriesCommand(char*, int);
  void processViewQueryModuleCommand(char*, int, char*);
  void processViewQueryCommand(char*, int, char*);
  void processActivateQueryModuleByNameCommand(char*, int, char*);
  void processActivateQueryModuleCommand(char*, int, char*);
  void processActivateQueryCommand(char*, int, char*);
  void processDeactivateQueryModuleCommand(char*, int, char*);
  void processDeactivateQueryCommand(char*, int, char*);
  void processDeleteQueryModuleCommand(char*, char*);
  void processDeleteQueryCommand(char*, char*);
  int activateQuery(char*, char*);
  int deactivateQuery(char*, char*);
  void removeQueryFromModuleAtIndex(int index, int qIndex);
  void removeQueryModuleByIndex(char* userName, int index);
  void processOfflineModeCommand(char*, int, char*, char*);

  /*Logs*/
  void processViewIOSLogsCommand(char*, int, char*);
  void processViewQSLogsCommand(char*, int, char*);

  /*Time series*/
  void processAddTSQueryCommand(char*, int, char*, char*);
  void processViewAllTSQueriesCommand(char*, int);
  void processViewTSQueryCommand(char*, int, char*);
  void processActivateTSQueryCommand(char*, int, char*);
  void processDeactivateTSQueryCommand(char*, int, char*);
  void processDeleteTSQueryCommand(char*, char*);

  /*Declares*/
  void rewriteSystemDeclares(int);
  void initializeSystemTables();
  void initializeSystemStreams();
  void processAddDeclaresCommand(char*, int, char*, char*, int);
  void processViewAllTablesCommand(char*, int);
  void processViewAllStreamsCommand(char*, int);
  void processViewDeclareModuleCommand(char*, int, char*);
  void processViewDeclareCommand(char*, int, char*);

  void activateStreamModule(char*, int, int);
  void processActivateStreamModuleByNameCommand(char*, int, char*);
  void processActivateStreamModuleCommand(char*, int, char*);
  void processActivateStreamCommand(char*, int, char*);
  void processDeactivateStreamModuleCommand(char*, int, char*);
  void processDeactivateStreamCommand(char*, int, char*);

  int activateStream(A_stmt, char*, char*);
  int deactivateStream(A_stmt, char*, char*);

  void processDeleteDeclareModuleCommand(char*, int, char*);
  void processDeleteDeclareCommand(char*, int, char*);

  void removeDeclareFromModuleAtIndex(int index, int qIndex);
  int removeDeclareModuleByIndex(int index);

  /*Aggregates*/
  void initializeAggregateDeclares();
  void appendAggregateToCommonAggr(char*, int);
  void processAddAggregateCommand(char*, int, char*, char*);
  void processAddExtAggregateCommand(char*, int, char*, char*);
  void processViewAllAggregatesCommand(char*, int);
  void processViewAggregateCommand(char*, int, char* buf);
  void rewriteAggregateDeclares();
  void processDeleteAggregateCommand(char*, char*);

  /*Model*/
  //void initializeModelDeclares();
  //void appendModelToCommonModel(char*, int);
  void processAddModelCommand(char*, int, char*, char*);
  void processViewAllModelsCommand(char*, int);
  void processViewModelCommand(char*, int, char* buf);
  void rewriteModelDeclares();
  void processDeleteModelCommand(char*, char*);

  /* Externs*/
  //void initializeAggregateDeclares();
  //void appendAggregateToCommonAggr(char*);
  void processAddExternCommand(char*, int, char*, char*);
  void processViewAllExternsCommand(char*, int);
  void processViewExternCommand(char*, int, char* buf);
  //void rewriteAggregateDeclares();
  void processDeleteExternCommand(char*, char*);

  void processViewAllCommand(char*, int);

  static void sendMessageToQueryBuf(int code, char* id);
  static int getMessageFromQueryBuf(int &code, char* id);
  int generateStatements(char* userName, char* buf, char* clientIpAddr,
      char* path, char* ext, int stmtType, vector<A_stmt>*, char*&);
  int simpleTest();

  int executeUNIXCommand(char *cmd, std::string &s);

  Ios();
public:

  int readCommand();
  static Ios* getInstance();
  void run(int, char*);

  // API used to send commands to discovery such as 'addUser, addHost' etc...
  bool sendCommandToDiscovery(char* command);

  // Executes a system command and captures the output as a nice string.
  string exec(char* cmd);
};

vector<char*> parseStmts(char* buf);

#endif
