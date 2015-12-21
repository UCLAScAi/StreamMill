/**
 * This file is a sample tuple db where we
 * put fixed length tuples into a linear storage.
 * The first tuple in this linear storage is a super-tuple where
 * some control information are stored.
 * The other tuples are data tuples and their offsets are
 * used as the pointer in Rtree index db
 *
 * In the real application, use whatever means to replace this
 * simple tuple db.  For example, application developer may use
 * a Berkeley DB's B-tree to implement the tuple db, then use
 * the primary key of the tuple db as the pointer in Rtree index.
 */

#include "rtree.h"
#include "db.h"
/** Initialize the Berkeley DB RECNO database as RTREE underlying storage method
 * @param rtree: underlying RTREE
 * @param flags: only NEW_RTREE is supported in this version
 */

int tuple_init(RTree *rtree, int flags){
  int rc = 0;

  switch (flags){
  case NEW_RTREE:
    if ((rc = db_create(&rtree->db, NULL, 0)) != 0) {
      myerr("tuple_init: db_create()\n");
    }
    if ((rc = rtree->db->set_pagesize(rtree->db, PAGESZ)) != 0) {
      myerr("set_pagesize()");
    }
    (void)remove(rtree->record_fname);
    if ((rc = rtree->db->open(rtree->db, rtree->record_fname, NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
      rtree->db->err(rtree->db,rc, "%s", rtree->record_fname);
      myerr("tuple_init: open()");
    }
    break;
  case OLD_RTREE:
    if ((rc = db_create(&rtree->db, NULL, 0)) != 0) {
      myerr("tuple_init: db_create()\n");
    }
    if ((rc = rtree->db->set_pagesize(rtree->db, PAGESZ)) != 0) {
      myerr("set_pagesize()");
    }
    if ((rc = rtree->db->open(rtree->db, rtree->record_fname, NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
      rtree->db->err(rtree->db,rc, "%s", rtree->record_fname);
      myerr("tuple_init: open()");
    }
   
    break;
  } // end of switch
  return 0;
}

/** Get the specified tuple
 * @param rtree: underlying rtree;
 * @param tuple: the buffer to which the tuple is written;
 * @param tplno: the tuple no.;
 * @return 0 if successful, otherwise -1;
 *
 */

int tuple_get(RTree *rtree, void* tuple, tupleno_t *tplno){
  DBT data;
  DBT key;
  int rc;
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  key.size = sizeof(tupleno_t);
  key.data = tplno;
  rc = rtree->db->get(rtree->db, NULL, &key, &data, 0);
  switch (rc){
  case DB_NOTFOUND:
    return DB_NOTFOUND;
    break;
  case 0:
    memcpy(tuple, data.data, data.size);
    break;
  default:
    rtree->db->err(rtree->db, rc, "%s", "tuple_get: Berkeley DB get!");
    return rc;
  }
  return 0;
}

/** tuple_put(): put the tuple into database
 * @param rtree: underlying rtree;
 * @param tuple: the buffer in which the tuple is stored;
 * @param tplno: the tuple no.;
 * @return 0 if successful, otherwise Error Code;
 *
 */

int tuple_put(RTree *rtree, void* tuple, tupleno_t *tplno){
  DBT data;
  DBT key;
  int rc;
  dbg(printf("tuple_put: putting %d\n", tuple));
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  data.size = rtree->tplsz;
  data.data = tuple;
  if ((rc = rtree->db->put(rtree->db, NULL, &key, &data, DB_APPEND)) != 0){
    rtree->db->err(rtree->db, rc, "%s", "tuple_put");
    return rc;
  }
  *tplno = *((tupleno_t*)key.data);
  dbg(printf("tuple_put: leaving\n"));
  return 0;
}
/** tuple_del(): delete the tuple
 * @param rtree: underlying rtree;
 * @param tplno: the tuple no.;
 * @return 0 if successful, otherwise Error Code;
 *
 */

int tuple_del(RTree *rtree, tupleno_t *tplno){
  DBT key;
  int rc;
  memset(&key, 0, sizeof(key));
  key.size = sizeof(tupleno_t);
  key.data = tplno;
  rc = rtree->db->del(rtree->db, NULL, &key, 0);
  switch (rc){
  case DB_NOTFOUND:
    return DB_NOTFOUND;
    break;
  case 0:
    break;
  default:
    rtree->db->err(rtree->db, rc, "%s", "tuple_del: Berkeley DB del!");
    return rc;
  }
  return 0;
}

int tuple_close(RTree *rtree){
  int rc;
  if ((rc=rtree->db->close(rtree->db, 0)) != 0){
    rtree->db->err(rtree->db, rc, "%s", "Berkeley DB close!");
    return rc;
  }
}

/* tuple_cursor_init: initialize the underlying BerkeleyDB's cursor.
 */
int tuple_cursor_init(RTreeCursor *rtc){
  DB *db = rtc->rtree->db;
  int rc;
  if ((rc = db->cursor(db, NULL, &rtc->dbc, 0)) != 0) {
    db->err(db, rc, "%s", "tuple_cursor_init");
  }
  return 0;
}

/* tuple_cursor_put: cursor_put tuple into underlying BerkeleyDB
 */
int tuple_cursor_put(RTreeCursor *rtc, 
		     void *tuple, 
		     int flags){
  DB *db = rtc->rtree->db;
  DBC* dbc = rtc->dbc;
  DBT data;
  DBT key;
  int rc;
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  data.size = rtc->rtree->tplsz;
  data.data = tuple;
  dbg(printf("tuple_cursor_put: putting tuple = %d data.size = %d\n", *((int*)data.data), data.size));
  if ((rc = dbc->c_put(dbc, &key, &data, flags)) != 0){
    db->err(db, rc, "%s", "tuple_cursor_put");
    return rc;
  }
  return 0;
}



/* tuple_cursor_get: cursor_get tuple from underlying BerkeleyDB
   The tuple no. is rtc->curtpl;
 */
int tuple_cursor_get(RTreeCursor *rtc, 
		     void *tuple, 
		     int flags){
  DB *db = rtc->rtree->db;
  DBC* dbc = rtc->dbc;
  DBT data;
  DBT key;
  int rc;
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  switch (flags){
  case DB_SET:
    key.data = &rtc->curtpl;
    key.size = sizeof(tupleno_t);
    rc = dbc->c_get(dbc, &key, &data, flags);
    break;
  default:
    // not supported flags
    myerr("tuple_cursor_get:Not all flags supported yet!");
    break;
  } // end switch flags
  switch(rc){
  case DB_NOTFOUND:
    dbg(printf("tuple_curosr_get: DB_NOTFOUND!\n"));
    return DB_NOTFOUND;
    break;
  case 0:
    break;
  default:
    db->err(db, rc, "%s", "tuple_cursor_get");
    return rc;
    break;

  }
  memcpy(tuple, data.data, data.size);
  dbg(printf("tuple_cursor_get: got tuple = %d\n", *((int*)tuple)));
  return 0;
}
/* tuple_cursor_close: close BerkeleyDB's cursor
 */
int tuple_cursor_close(RTreeCursor *rtc){
  DB *db = rtc->rtree->db;
  DBC* dbc = rtc->dbc;
  int rc;
  if ((rc = dbc->c_close(dbc)) != 0){
    db->err(db, rc, "%s", "tuple_cursor_close");
    return rc;
  }
  return 0;
}
