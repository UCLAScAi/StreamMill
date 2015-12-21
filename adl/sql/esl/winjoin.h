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


/* This class is for window-join.  We inherit it from the buffer class.  This is because of the driver's concern. Change the name of "winbuf" to something else */

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
  class winbuf:public buffer{
    IM_REL *b;   // data structure in memory
    IM_RELC *head; // pointing to oldest non-expired tuple in the window
    IM_RELC *temp;  // temporary cursor
    win_mode mode;  // window in memory or disk?
    _adl_win_type wtype;  // logical or physical?
    int size;        // window size -- tuples for physical window and seconds for logical window
    int n; // actual window size -- including expired tuples
  public:
    winbuf(int size_in, _adl_win_type wtype_in = _ADL_WIN_ROW);
    ~winbuf();
    // return codes: 0 if success, DB_NOTFOUND if no more tuple, otherwise fails
    // get the oldest tuple (could be either non-expired or expired)
    int get(dbt*);
    int get(DBT*, timestamp*, DBT*);

    // get the oldest expired tuple
    // return 0 if successful, DB_NOTFOUND if no expired tuples,
    // otherwise error code
    int getExpired(dbt*);

    // put the youngest tuple
    int put(dbt*);
    int put(DBT*, timestamp*, DBT*);

    // pop the oldest tuple (could be either non-expired or expired)
    int pop();

    // buffer empty?
    int empty();    
    
    IM_REL* get_im_rel();
    
    // get the head cursor pointing to the oldest non-expired tuple
    IM_RELC* getWinCursor();
    
    // advance head curosr by one
    int advanceHead();
    
    // get the youngest tuple
    int getTail(dbt* d);
    // get the oldest non-expired tuple in the window, pointed by head
    int getHead(dbt* d);
    
    int print();
    // window has expired?
    bool hasExpired();
  };
}
