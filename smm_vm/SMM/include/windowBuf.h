#ifndef __WINDOWBUF_H__
#define __WINDOWBUF_H__

#include <dbt.h>
#include <const.h>
#include <list>
extern "C"{
#include <mm.h>
}
extern "C"{
#include <im_db.h>
}

#include <db.h>
#include <types.h>
#include <GUIClient.h>

#include <vector>
#include <string>
using namespace std;

#include <buffer.h>
#include <types.h>
#include <winbuf.h>

using namespace ESL;
namespace ESL{
  
  class windowBuf:public buffer{
    IM_REL *b;   // data structure in memory
    IM_RELC *temp;  // temporary cursor
    win_mode mode;  // window in memory or disk?
    _adl_win_type wtype;  // logical or physical?
    timestamp first_ts;      // first timestamp, used only for time-based window
    unsigned int tuple_id;      // tuple_id of the youngest tuple.  The yougest tuple may not physically exists in the buffer.  For instance, it's deleted, or in the case of explicit window table, it's not inserted.
    unsigned int size;        // window size -- tuples for physical window and seconds for logical window
    int n; // actual window size -- including expired tuples
    bool first; // first tuple? used only in time-based window
    timestamp current;   // for joins this timestamp will be set equal to the 
                         // the timestamp of the current tuple in joined stream

  public:
    ~windowBuf();
    windowBuf(const char* name_in):buffer(name_in, WINBUF){};

    windowBuf(const char* name_in, unsigned int size_in,    // window size, in millisecond
	      _adl_win_type wtype_in = _ADL_WIN_ROW);

    // put a tuple to the tail of the buffer
    int put(dbt* d);
    int put(DBT* data, timestamp *atime, DBT* key=NULL);
    // put a tuple to the tail of the buffer
    //  int put(DBT* key, DBT* data, timestamp* atime);
    
    _adl_win_type getWinType() {return wtype;};  // logical or physical?
    

    // get a tuple from the head of the buffer
    int get(dbt* d);
    //get timestamp of the tuple on the head of the buffer
    int get(timestamp *atime);
    // get a tuple from the head of the buffer to DBT
    int get(DBT* data, timestamp *atime=NULL, DBT* key=NULL);
    
    // pop a tuple from the head of the buffer
    // int pop();   // already implemented
  
    // if the buffer empty?
    // int empty();  //already implemented may be change it to !full
    
    
    // return codes: 0 if success, DB_NOTFOUND if no more tuple, otherwise fails
    // get the oldest tuple (could be either non-expired or expired)
    //int get(char**);

    // get the oldest expired tuple
    // return 0 if successful, DB_NOTFOUND if no expired tuples,
    // otherwise error code
    int getExpired(DBT* key, DBT* data);
    int getExpired(DBT *data);

    // put the youngest tuple
    // key structure stores the tuple_id
    int put(DBT* key, DBT* data);
    int put(DBT* data);                // for tuple-based window
    //int put(timestamp *ts, DBT* data); // for time-based window

    // pop the oldest tuple (could be either non-expired or expired)
    int pop();

    // buffer empty?
    int empty();    

    //right now returns the actual size including expired tuples. May change later, if needed.
    int bufSize();
    //this is not implemented right now.
    int bufByteSize();

    IM_REL* get_im_rel();
    
    // get the youngest tuple
    // int getTail(DBT*key, DBT *data);
    
    // for debug
    int print();
    static int printTuple(DBT*, DBT*);
    // window has expired?
    bool hasExpired();

    void resetTS(timestamp *ts);
    void getTimestamp(timestamp *atime);    
    unsigned int getTupleID();  // return tuple_id

    // updateTupleID() should be called before put()
    // when ts=NULL, it's a count-based window and we increase tuple_id by one
    // Otherwise it's a time-based window and we increase tuple_id based on the input timestamp
    int updateTupleID(timestamp *ts=NULL);

    void setCurrentTimestamp(timestamp *ts);
};

}
#endif




