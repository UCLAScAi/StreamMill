#include "db.h"
#include "rtree.h"
/* We only handle fixed-length tuples as an example.
   It is database designer's job to design the internal
   structure of the file storing data tuples.

   Here the TUPLESZ is the maximal size.  It is okay to have smaller tuples.

   TUPLESZ < BLOCKSZ.
*/
//#define TUPLESZ 40
//#define TUPLEPTR(x)	(TUPLESZ*(x))




/** tuple_del(): delete the tuple
 * @param rtree: underlying rtree;
 * @param tplno: the tuple no.;
 * @return 0 if successful, otherwise Error Code;
 *
 */

int tuple_del(RTree *rtree, tupleno_t *tplno);
/** Initialize the Berkeley DB QUEUE database as RTREE underlying storage method
 * @param rtree: underlying RTREE
 * @param flags: only NEW_RTREE is supported in this version
 */
int tuple_init(RTree *rtree, int flags);

/** tuple_get(): Get the specified tuple
 * @param rtree: underlying rtree;
 * @param tuple: the buffer to which the tuple is written;
 * @param tplno: the tuple no.;
 * @return 0 if successful, otherwise -1;
 *
 */
int tuple_get(RTree *rtree, void* tuple, tupleno_t *tplno);

/** tuple_put(): put the tuple into database
 * @param rtree: underlying rtree;
 * @param tuple: the buffer in which the tuple is stored;
 * @param tplno: the tuple no.;
 * @return 0 if successful, otherwise Error Code;
 *
 */
int tuple_put(RTree *rtree, void* tuple, tupleno_t *tplno);

/* Close the data file */
int tuple_close(RTree *rtree);
/* tuple_cursor_init: initialize the underlying BerkeleyDB's cursor.
 */
int tuple_cursor_init(RTreeCursor *rtc);
/* tuple_cursor_put: cursor_put tuple into underlying BerkeleyDB
 */
int tuple_cursor_put(RTreeCursor *rtc, 
		     void *tuple, 
		     int flags);
/* tuple_cursor_get: cursor_get tuple from underlying BerkeleyDB
   The tuple no. is rtc->curtpl;
 */
int tuple_cursor_get(RTreeCursor *rtc, 
		     void *tuple, 
		     int flags);
/* tuple_cursor_close: close BerkeleyDB's cursor
 */
int tuple_cursor_close(RTreeCursor *rtc);
