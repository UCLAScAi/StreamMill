#ifndef __STMT_H__
#define __STMT_H__

#include "buffer.h"
#include <list>
//#include <hash_map.h>
#include <ext/hash_map>
#include "adllib.h"

//using namespace ESL;
using namespace std;
using namespace __gnu_cxx;

namespace ESL{

typedef enum {
  s_failure=-2,
  s_no_input,
  s_no_output,
  s_success
} stmt_rc;

//typedef enum {
//  s_success=0,
//  s_failure,
//  s_no_input,
//  s_no_output
//} stmt_rc;

typedef enum {
  stmt_normal,
  stmt_join,
  stmt_t_union,
  stmt_tl_union,
  stmt_coll
} stmt_t;

typedef union {
  long quota;
  double priority;
  int weight;
} rank_t;

class Monitor;
class Driver;
struct StmtEntry;
 
// Statements
class stmt
{
  friend class querySchdl;
  friend class Monitor;
  friend class Strategy;
  friend struct StmtEntry;
  
  //this may be temporary, for now, to allow exsiting code to work.
  friend class Driver;

 public:
  stmt();
  /*
  stmt(char *name,
       buffer* i,
       buffer* o=NULL,  // when NULL, output is STDOUT
       stmt_t t = stmt_normal);
  */
  
  //int id;
  stmt_t type;
  
  buffer* in;  //single input buffer, or the first buffer to use in a window-join operation
  char inBufName[256]; 
  buffer* out;
  char outBufName[256]; 
  int (*func)(bufferMngr*, int, buffer*, 
	      hash_map<const char*, void*, hash<const char*>, eqstrTab>* inMemTables); // function entry
  char name[256]; // DLL file name, unique key for stmt
  bool sub_stmt; //flag to indicate whether this statement is a substatement

  //maximum deadline of delay for the output of this statement, in seconds
  //ideally can only be applied at sink level, and calculated backwards.
//!!!!!not done yet, temporarily just use source stmt to directly assign  
  int deadline; 
  
  bool valid;

  //double priority; //assigned by scheduling strategy, when applicable.
  rank_t rank;
  bool rankSet; //flag to indicate whether rank was ever set

  //user defined importance weights. Only the sink statements are by the user
  //Other stmts take the max of the downstream stmts, all the way up to sources
  int weight; 
  bool weightSet; //used when weights are calculated for non-sink stmts
  
  virtual ~stmt();

  bool monitor_src_stat;  //flag set when stat need to be collected on this statement for source info
  bool monitor_sink_stat;  //flag set when stat need to be collected on this statement for output info
  //bool monitor_stmt_stat;  //flag set when stat need to be collected on this statement for general stmt info

  //-----for new strategy scheme ------------
  //union {
  int quota; //assigned quota
  //double time_quota;
  //};
    
  //union {  
  int quota_left; //quota still left to be used, in this round.
  //double time_quota_left;
  //};

  int deadline_quota; //to be used when handling quota increase for deadline, save last increase information to achieve exponential increase
  
  //-----end for new strategy scheme --------
  virtual void print() = 0;
  
 protected:
  //make this protected, to make sure that it can only be called through Monitor
  virtual stmt_rc exe(int freeVar = 0) = 0;
  StmtEntry* stmtEntry;
  
};

// normal Statements, and sub statements in UNION
class nStmt:public stmt
{
 public:
  //nStmt();
  nStmt(char *name,
	buffer* i,
	buffer* o=NULL,  // when NULL, output is STDOUT
	bool sub_stmt=false);
  
  virtual ~nStmt();

 protected:
  stmt_rc exe(int freeVar = 0);
  void print();
};

//Many optimizations require that one statement is broken into
//two statements. Similarly flows may require generating multiple 
//statements from a single statement. Thus we create this place-holder
//class. Only the original substatements are added to the driver.
// collection of statements
class cStmt:public stmt
{
 public:
  list<stmt*> sub_stmts;
  void addSubStmt(stmt* s); 
  cStmt();
  cStmt(char* name);
  cStmt(char* name, stmt*);
  virtual ~cStmt();
  void set_in_buf(buffer* i) {in = i;}
  void set_out_buf(buffer* i) {out = i;}
  void print();

 protected:
  // execute the statement
  stmt_rc exe(int freeVar = 0) { return s_failure; }
};

//union Statements, abstract
class uStmt:public stmt
{
 public:

  list<buffer*> union_bufs; //all buffers in a union operation, the in-buffer is not used here.

  list<stmt*> sub_stmts; //all sub statements in a union operation.
  
  uStmt();
  
  void addSubStmt(stmt* s)  { sub_stmts.push_back(s); union_bufs.push_back(s->out); }
  
  virtual ~uStmt();
  void print();

 protected:
  //execute the statement, but set the in-buffer to be the correct buffer to be poped, so the calling code can pop the right buffer! Or set it to be the empty buffer, when there is a need to backtrack.
  stmt_rc exe(int freeVar = 0) { if (freeVar == 0) return exe_set_in(); else return s_success;}

  //execute the statement, but set the in-buffer to be the correct buffer to be poped, so the calling code can pop the right buffer!
  virtual stmt_rc exe_set_in() = 0;
};

//union Statement consider timestamp
class utStmt:public uStmt
{
 public:
  //utStmt();
  utStmt(char *name,
	 //buffer* i, 
	 buffer* o=NULL,  // when NULL, output is STDOUT
	 bool sub_stmt=false);

  virtual ~utStmt();
  
 protected:
  stmt_rc exe_set_in();
};

//union Statement, does not consider timestamp
class utlStmt:public uStmt
{
 public:
  //utlStmt();
  utlStmt(char *name,
	  //buffer* i,
	  buffer* o=NULL,  // when NULL, output is STDOUT
	  bool sub_stmt=false);
  
  virtual ~utlStmt();

 protected:
  stmt_rc exe_set_in();
};



// join Statements
class jStmt:public stmt
{
 public:

  list<buffer*> window_bufs; //the rest of buffers in a window-join operation
  buffer* back_buf; //specifically used in join to tell the calling code which buffer to backtrack to.
  list<stmt*> sub_stmts; //all sub statements in a join operation.

  
  //jStmt();
  jStmt(char *name,
	buffer* i,
	buffer* o=NULL,  // when NULL, output is STDOUT
	bool sub_stmt=false);

  void print();

  void addSubStmt(stmt* s)  { sub_stmts.push_back(s); window_bufs.push_back(s->out); }
  
  virtual ~jStmt();

 protected:
  // execute the statement
  stmt_rc exe(int freeVar = 0) { back_buf = NULL; return exe_set_backbuf(freeVar); }
  stmt_rc exe_set_backbuf(int freeVar = 0);
};

} // end of namespace ESL
#endif

