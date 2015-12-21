#ifndef __WINBUF_H__
#define __WINBUF_H__
extern "C"{
#include <im_db.h>
}

#include <buffer.h>
#include <list>
#include <dbt.h>
#include <const.h>
#include <types.h>
/*-----------------------------------------------------------

----
|  |<- expired
|  |
|  |
|  |
|  |<- head
|  |
----<- tail

-----------------------------------------------------------*/


/* This class is for window over aggr */

using namespace std;
using namespace ESL;
namespace ESL{
  typedef enum{
    win_memory,
      win_disk,
      }win_mode; // window in memory or disk?
  typedef list<dbt*> ldbt;
  typedef ldbt::iterator pos;


  // Winbuf: buffer for window.  Now we only use memory
  // Later, we can detect system overloaded and use some paging algorithms
  //  , such as LFD or LIFO alg.
  // to spill pages onto disk, 
  class winbuf{
    IM_REL *b;   // data structure in memory
    IM_RELC *temp;  // temporary cursor
    win_mode mode;  // window in memory or disk?
    _adl_win_type wtype;  // logical or physical?
    timestamp first_ts;      // first timestamp, used only for time-based window
    unsigned int tuple_id;      // tuple_id of the youngest tuple.  The yougest tuple may not physically exists in the buffer.  For instance, it's deleted, or in the case of explicit window table, it's not inserted.
    unsigned int size;        // window size -- tuples for physical window and seconds for logical window
    int n; // actual window size -- including expired tuples
    bool first; // first tuple? used only in time-based window
  public:
    int datasize;
    int keysize;
    int keybegin;
    int plidbegin;
    winbuf(unsigned int size_in,    // window size, in millisecond
	   int datasize,
	   int keysize,
	   _adl_win_type wtype_in = _ADL_WIN_ROW);
    ~winbuf();
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

    //for partition by to store the key at the end
    int put(DBT* data, char* gbkey, int id); 
    //int put(timestamp *ts, DBT* data); // for time-based window

    // pop the oldest tuple (could be either non-expired or expired)
    int pop();

    // buffer empty?
    int empty();    
    
    IM_REL* get_im_rel();
    
    // get the head cursor pointing to the oldest non-expired tuple
    IM_RELC* getWinCursor();
    
    
    // get the youngest tuple
    // int getTail(DBT*key, DBT *data);
    
    // for debug
    int print();
    int printTuple(DBT*, DBT*);
    // window has expired?
    bool hasExpired();

    void resetTS(timestamp *ts);
    void getTimestamp(timestamp *atime);    
    unsigned int getTupleID();  // return tuple_id

    // updateTupleID() should be called before put()
    // when ts=NULL, it's a count-based window and we increase tuple_id by one
    // Otherwise it's a time-based window and we increase tuple_id based on the input timestamp
    int updateTupleID(timestamp *ts=NULL);

    //funcs to enable windowed min max
    int getHeadInt();
    void deleteLesserInt(int testVal);
    void deleteGreaterInt(int testVal);
    double getHeadReal();
    void deleteLesserReal(double testVal);
    void deleteGreaterReal(double testVal);
  };
}


#endif

