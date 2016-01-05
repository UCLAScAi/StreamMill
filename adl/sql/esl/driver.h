#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "buffer.h"
#include "stmt.h"
#include <iostream>
#include <string>
#include <ext/hash_map>
#include <list>

using namespace std;
using namespace __gnu_cxx;

namespace ESL{
int driver_test ();

typedef enum {
  act_none, //sink
  act_cjump,  //normal
  //act_back,
  act_merge_jump, //union output to normal
  act_cfork, //fork input
  act_jump,  //fork dummy
  act_merge, //union output to sink
  act_merge_cfork //union output to fork input 
} d_action;

static char* d_action_strs[] = {"None", "CJump", /*"Back",*/ "MergeJump", "CFork", "Jump", "Merge", "MergeCFork"};

typedef enum {
  t_normal,
  t_source,
  t_fork,
  t_source_fork,
  t_merge,
  t_sink,
  t_merge_sink,
  t_merge_fork,
  t_fork_dummy,
  //t_merge_dummy,
  t_none          //used for null return value
} buffer_ty;

static char* buffer_t_strs[] = {"Normal", "Source", "Fork", "Source-Fork", "Merge", "Sink", "Merge-Sink", "Merge-Fork", "Fork-Dummy" /*, "Merge-Dummy"*/};

union fw_entry 
{
  char* forward_buf;
  int fork_count;
};

union bk_entry 
{
  char* back_buf; //for normal back buffer entry
  stmt* union_stmt; //for union output buffer, pointer to its union statement (not the statement that follows, which is referred to in the main table.)
  bool processed; //for fork dummy buffers, indicate if this fork branch has been processed for the current tuple
};

typedef enum {
  m_down,
  m_up,
  m_break
} mark_t;

typedef struct buffer_entry
{
  buffer_ty b_type;
  stmt* statement;
  buffer* buf;
  bk_entry back;
  char* inplace;
  fw_entry forward;
  d_action action;

  mark_t mark; //for graph traversal marking, when set breakpoints
  int pop(); //this pop is created to handle delayed pop -- in the case of fork buffers.

  buffer_entry():statement(NULL),buf(NULL),inplace(NULL) {}

  buffer_entry(buffer_entry* entry);

} *b_entry;

typedef enum {
  run_success,
  run_failure,
  run_no_input
  /*run_tl_union*/     //return from a timestamp-less union, needs to be re-entered by scheduler.
} run_rc;

struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;    
  }  
};

class Driver;

typedef hash_multimap<char*, Driver*, std::hash<char*>, eqstr> BufMapType;
typedef hash_map<char*, b_entry, std::hash<char*>, eqstr> BufStateTblType;
typedef hash_map<char*, Driver*, std::hash<char*>, eqstr > StmtMapType;

class Strategy;
class Monitor;
class ScheduleStrategy;

class Driver
{
  friend class DrvMgr;

 public:

  int getId();
  void setId(int id);
  int getPriority();
  void setPriority(int priority);
  BufStateTblType* getBufGraph();
  list<stmt*>* getStmts();
  list<buffer*>* getBufs();

  int addStmt(stmt* s);
  
  //drop stmt, also need to modify the bufToDrvMap in DrvMgr,
  //since only driver knows when a buffer is used only in one statement thus requires removal
  int dropStmt(stmt* s);  // note: free the stmt memory in the query scheduler, not here.

  /* no longer used: driver now only keeps the original query graph, segmented graphs are kept by strategies.
  //set a set of buffers to be break points
  //this set needs to break the buffer flow graph into 2. Later, need to add in verification code.
  int setBreakPnt(list<buffer*>* bufs);
  */

  //nTuples need to be passed in, usually it can be set to 0 so that "priority" of a driver will be used.
  //A value is returned to *nTuples, if run_rc is set to code run_tl_union
  //which will prompt scheduler to re-schedule this driver for *nTuples number of tuples.
  run_rc run(int* nTuples);
  
  void printStateTable(BufStateTblType* tbl = NULL, ostream* os = NULL); //use parameter so that it can be conveniently used for table printing for strategies.

  stmt* getStmtByName(char* name);
  buffer* getBufByName(char* name);
  
  bool isDirty() {return dirty;}
  void resetDirty() {dirty = false;}
  void setDirty() {dirty = true;}

  void setStrategy(Strategy* strategy);
  Strategy* getStrategy() {return this->strategy;}

  //quick hack for now, to deal with old and new type of strategies.
  void setStrategy(ScheduleStrategy* strategy);
  ScheduleStrategy* getStrategy(bool weighted) {if (weighted) return this->weightedStrategy;}

  buffer_ty getBufType(buffer*);
  buffer* getBuffer(char* bufferName);

  ~Driver();

  //moved over from Strategy class, to support strategy switch from one to another for this driver
  //flag used for interaction between runNextUnit and RunState. RunState may trigger statetables rebuild
  //when rebuid happens, iteration in runNextUnit on the statetable has to terminate to avoid exceptions.
  bool stateTableChanged; 

 protected:
  Driver(int roundcnt = DEFAULT_TUPLE_PER_PROMPT);

  void resetMarks();
  void setSink(b_entry entry);
  
  //last flag to indicate whether this is already a backtrack operation from a union with no timestamp
  run_rc runState(b_entry state, int* nTuples, bool unionNoTime = false);

  void sendErrorForStmt(stmt* s);
  
 private:
  int id;
  BufStateTblType* bufStateTable;
  //the number of tuples to run at a time, (before exit to check whether there is any user command
  //this can be tied to relative importance of drivers
  //also at some point too high number will hurt the interactivity of program - user command take longer to be responded when there are tuples in system.
  int priority;  
  list<stmt*>* stmts;
  list<buffer*>* bufs;
  //buffer* last_run;
  char last_run_name[255];
  //buffer* entryPoint;
  bool dirty; //flag to indicate modifications, used by scheduling strategies

  Strategy* strategy; //scheduling algorithm to be used when running it
  ScheduleStrategy* weightedStrategy; //new version strategy. Keep both this and above for now.

};


class DrvMgr 
{
  friend class Driver;
  friend class Monitor;
  
 public:
  
  static DrvMgr* getInstance();
  static void destroy();  
  list<Driver*>*  getDrivers();

  /*drv should be passed in as a null pointer.
    if a new driver is created, then it is returned in drv, which can be used to set priority.
    if the stmt is added to an existing driver, that driver is also returned */
  int addStmt(stmt* s, Driver* &drv);
  int addStmt(stmt* s);

  //same as add above, the driver which was dropped from is returned
  //need to check returned drv for NULL, since it may have become empty and deleted
  int dropStmt(stmt* s, Driver* &drv); // note: free the stmt memory in the query scheduler, not here.
  
  int setBreakPnt(list<buffer*>* bufs);
  
  //reset a set of buffers to be not a break point
  //this operation should merge two Drivers to one, needs code later to verify this.
  //do this by remove a driver and add back the statements to the other one, is much simpler than implementing this function.
  //int resetBreakPnt(list<buffer*>* bufs);  

  //run the driver manager, which will call optimizer if necessary.
  //nTuples can be set to 0, which will lead to use the priority of the driver
  //if not set to 0, then priority is overwritten (for example, when re-enter a timeless union)
  run_rc run(Driver* drv, int* nTuples);
  //nTuples default to 0
  run_rc run(Driver* drv);

  stmt* getStmtByName(char* name);
  buffer* getBufByName(char* name);  
  bool bufferInUse(char* bname);
  bool stmtInUse(const char* name);
  Driver* getDrvById(int id);

  void setMonitor(Monitor* monitor) {this->monitor = monitor;}
  Monitor* getMonitor() {return this->monitor;}
  
  ~DrvMgr();
  
 protected:
  DrvMgr();
  BufMapType* bufToDrvMap;
  list<Driver*>* drivers;
  StmtMapType* stmtToDrvMap;
  static int id_count;
  
 private:
  static DrvMgr* _instance;
  Monitor* monitor; //statistics monitor
};
} // end of namespace ESL
#endif
