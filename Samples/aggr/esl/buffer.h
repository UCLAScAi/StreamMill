#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <dbt.h>
#include <const.h>
#include <deque>
extern "C"{
#include <mm.h>
}
#include <db.h>
#include <types.h>
#include <GUIClient.h>
#include <types.h>

#include <vector>
#include <string>
#include <ext/hash_map>

using namespace std;

namespace ESL{

typedef enum{
  NOT_SHARED=1,
    SHARED,
    BUF_STDOUT,
    BUF_BTREE,
    BUF_RECNO,
    WINBUF,
}buf_t;

typedef enum{
  TS_INTERNAL,
    TS_EXTERNAL,
    TS_LATENT
}ts_type;
 
class buffer{
 protected:
  vector<GUIClient*> clients; // clients which monitor this buffer

 public:
  int type;               // buffer in shared memory?
  ts_type timestamp_type;
  
  char name[MAX_NAME];      // buffer name
  struct timeval* mant;         //current MANT state for the buffer
  
  buffer(const char* name_in,
	 buf_t shared_in=NOT_SHARED);
  virtual ~buffer()=0;

  // get a tuple from the head of the buffer
  virtual int get(dbt* d)=0;

  // get timestamp of the tuple on the head of the buffer
  virtual int get(timestamp *atime)=0;
  
  /* We set function get(DBT*, timestamp*, DBT*) to virtual due to
   * performance concerns.  We don't want to call get(dbt*) again.
   */
  virtual int get(DBT* data, timestamp *atime=NULL, DBT* key=NULL)=0;
  int get(int &code, char *s1=NULL, char *s2=NULL);

  // put a tuple to the tail of the buffer  
  virtual int put(DBT* data, timestamp *atime=NULL, DBT* key=NULL)=0;

  /* We set function put(DBT*, timestamp*, DBT*) to virtual due to
   * performance concerns.  We don't want to call get(dbt*) again.
   */
  virtual int put(dbt* d)=0;
  int put(int code, const char *s1=NULL, const char *s2=NULL);



  // pop a tuple from the head of the buffer
  virtual int pop()=0;
  
  // if the buffer empty?
  virtual int empty()=0;

  virtual int bufSize()=0;
  virtual int bufByteSize()=0;

  // add client to the monitor list
  int addClient(GUIClient* c);

  // drop client from the monitor list
  int dropClient(GUIClient *c);

  // get the monitor clients list
  int getClients(vector<GUIClient*> &v);

  bool isTable();

  //set the MANT state to the value passed in, allocate memory if mant pointer was NULL
  int setMant(const timestamp *mtime);
  //get the MANT state, return -1 if mant state was never set (this->mant is still NULL)
  int getMant(timestamp *mtime);
};

class commonBuf:public buffer{
  deque<cDBT*> b;                  // actual buffer space in heap, compression optimazation is possible for fixed-size tuple, i.e. remove keysz and datasz from actural storage.  
 public:
  ~commonBuf(){};
  commonBuf(const char* name_in):buffer(name_in){};

  // put a tuple to the tail of the buffer
  int put(dbt* d);
  int put(DBT* data, timestamp *atime=NULL, DBT* key=NULL);
  // put a tuple to the tail of the buffer
  //  int put(DBT* key, DBT* data, timestamp* atime);

  // get a tuple from the head of the buffer
  int get(dbt* d);
  //get timestamp of the tuple on the head of the buffer
  int get(timestamp *atime);
  // get a tuple from the head of the buffer to DBT
  int get(DBT* data, timestamp *atime=NULL, DBT* key=NULL);

  // pop a tuple from the head of the buffer
  int pop();
  
  // if the buffer empty?
  int empty();

  int bufSize();
  int bufByteSize();
};

class sharedBuf:public buffer{
  int sz;            // buffer size, in bytes

  sDBT* tail;
  int n;                    // current # of tuples in the buffer
  
  // is the buffer full?
  int full();

  // increase buffer size when full
  int increase();

 public:
  sDBT* head;
  //  static MM* buf_mm;

  // is the buffer full?
  //static MM* createSM();
  //static void destroySM();

  ~sharedBuf();
  sharedBuf(const char* name_in
	    );

  // put a tuple to the tail of the buffer
  int put(dbt* d);
  int put(DBT* data, timestamp *atime=NULL, DBT* key=NULL);

  // get a tuple from the head of the buffer
  int get(dbt* d);
  //get timestamp of the tuple on the head of the buffer
  int get(timestamp *atime);
  // get a tuple from the head of the buffer to DBT
  int get(DBT* data, timestamp *atime=NULL, DBT* key=NULL);

  // pop a tuple from the head of the buffer
  int pop();
  
  // if the buffer empty?
  int empty();

  int bufSize();
  int bufByteSize();

  void printAll();
};
/*
 class stdoutBuf:public buffer{
 public:   
   int get(dbt*);
   int get(DBT*, timestamp*, DBT*);
   int put(dbt*);
   int put(DBT*, timestamp*, DBT*);
   int pop();
   int empty();
   stdoutBuf(const char *name);
   ~stdoutBuf();

 };
*/

 class table:public buffer{
 public:   
   DB *db;
   int get(dbt*);
   //get timestamp of the tuple on the head of the buffer
   int get(timestamp *atime);
   int get(DBT*, timestamp*, DBT*);
   int put(dbt*);
   int put(DBT*, timestamp*, DBT*);
   int pop();
   int empty();
   int bufSize();
   int bufByteSize();
   table(const char *name, buf_t, const char* dbname = NULL);
   ~table();
 };

class bufMap{
 public:
  char name[MAX_NAME];
  buffer* b;
  static int cmp(const void * m1, const void* m2){
    return strcmp(((bufMap*)m1)->name, 
		  ((bufMap*)m2)->name);
  };
};

// Buffer Manager
// NOTE: Buffer Manager is sitting on shared memory using MM library.  Thus, try to use only C and avoid C++, such as dynamic cast.
class bufferMngr{ 
  bufMap dict[MAX_BUF]	;    // name-buffer dictionary
  int n;                     // # of buffers
  static bufferMngr * _instance;
  //  static stdoutBuf *bStdout;
  // given the name, return the bufMap pointer, used in kill()
  bufMap* lookupBufMap(const char* name);

  // given the name, return the index in dict, used in kill()
  int lookupBufMapIdx(const char* name);

 public:
  int new_arrival;
  bufferMngr();
  ~bufferMngr();
  int init();
  int destory();
  static bufferMngr * getInstance();

  // create a buffer
  int create(const char *name,        // buffer name
	     buf_t shared=NOT_SHARED,     // buffer in the shared memory?  yes for i/o buffers
	     unsigned int size_in = 0,
	     _adl_win_type wtype_int = _ADL_WIN_ROW,
	     const char* dbname = NULL);
  int create(const string name, 
	     buf_t shared=NOT_SHARED,	     
	     unsigned int size_in = 0,
	     _adl_win_type wtype_in = _ADL_WIN_ROW,
	     const char* dbname = NULL){ 
    return create(name.c_str(), shared, size_in, wtype_in, dbname);};


  // delete a buffer
  int kill(const char* name);

  // put a tuple to the tail of the specified buffer
  int put(const char* name,
	  dbt *d);

  // get a tuple from the head of the specified buffer
  int get(const char* name,
	  dbt * d);

  //get timestamp of the tuple on the head of the specified buffer
  int get(const char* name,
	  timestamp *atime);


  // get the DB pointer when the buffer is a btree
  int get(const char* name,
	  DB *&db);

  // pop a tuple from the head of the buffer
  int pop(const char *name);



  // if the buffer empty?
  int empty(const char* name);

  // given the buffer name, return the actual buffer address pointer
  // or NULL if failed;
  buffer* lookup(const char* name);
  buffer* lookup(const string name){ return lookup(name.c_str());};

  void displayBufNames();

  // Display all buffers
  void displayBufs();

  // return the stdout buffer pointer
  //stdoutBuf *getStdout();
  
};
};  // end of namespace ESL
#endif




