#include <config.h>
#include <sys/types.h>
#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <stdio.h>
#include <db.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#ifdef HAVE_LIBDL
#include <dlfcn.h>
#else
#include <windows.h>
#endif
#include <math.h>
#include <limits.h>
#include <rtree.h>
#include "const.h"
#include "err.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "adllib.h"
using namespace ESL;
#include <buffer.h>


#include <vector>
#include <ext/hash_map>
using namespace __gnu_cxx;


// for compatibility issue between c and c++
int _ADL_NULL = INT_MAX;
char *_ADL_NULL_STR = "_this_is_a_null_str_";

void adlabort(int rc, char *fmt, ...)
{
  char buf[MAX_STR_LEN];
  va_list ap;
  va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
  fprintf(stderr, "BERKELEY/IM DB ERR %s : %s\n", buf, db_strerror(rc));
  va_end(ap);

  exit(1);
}
void adlerror(int rc, char *fmt, ...)
{
  char buf[MAX_STR_LEN];
  va_list ap;
  va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
  fprintf(stderr, "BERKELEY/IM DB ERR %s : %s\n", buf, db_strerror(rc));
  va_end(ap);
}

void adlabortESL(buffer* errorBuf, int rc, char *fmt, ...)
{
  char buf[MAX_STR_LEN];
  char buf1[MAX_STR_LEN];
  va_list ap;
  va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
  if(rc != 0) {
    sprintf(buf1, "%s: %s", buf, db_strerror(rc));
  }
  else {
    sprintf(buf1, "%s", buf);
  }
  va_end(ap);

  //TODO: put the data in the buffer
  if(errorBuf) {
    char errStr[4096];
    sprintf(errStr, "o||%s||%s$\n", errorBuf->name, buf1);
    cDBT cdbt(errStr, 4096);
    errorBuf->put(&cdbt);
  }
  printf("Error occured: %s\n", buf1);
  fflush(stdout);
}

void adlabortESLAggr(buffer* errorBuf, int rc, char *fmt, ...)
{
  char buf[MAX_STR_LEN];
  char buf1[MAX_STR_LEN];
  va_list ap;
  va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
  if(rc != 0) {
    sprintf(buf1, "%s: %s", buf, db_strerror(rc));
  }
  else {
    sprintf(buf1, "%s", buf);
  }
  va_end(ap);

  //TODO: put the data in the buffer
  if(errorBuf) {
    char errStr[4096];
    sprintf(errStr, "o||%s||%s$\n", errorBuf->name, buf1);
    cDBT cdbt(errStr, 4096);
    errorBuf->put(&cdbt);
  }
  printf("Error occured: %s\n", buf1);
  fflush(stdout);
}


void adltrace(char *fun, int status, int flag)
{
  static int indent=0;
  int i;

  if (flag == 0) {
    for (i=0; i<indent; i++) fprintf(stderr, "  ");
    fprintf(stderr, "ENTERING %s STATUS %d\n", fun, status); 
    indent++;
  } else {
    indent--;
    for (i=0; i<indent; i++) fprintf(stderr, "  ");
    fprintf(stderr, "LEAVING %s STATUS %d\n", fun, status); 
  }
}

A_timeexp A_Timeexp(winbuf* wbuf) {
  A_timeexp a = (A_timeexp)malloc(sizeof(*a));
  a->wbuf = wbuf;
  a->deleted = false;
  return a;
}

void dump_data(char *format, char *data)
{
  char *p = format;
  char buf[MAX_STR_LEN];
  int w = 0, offset=0;
  
  if (p) {
    while (*p) {
      if (*p>='0' && *p <='9') {
	w = w*10+*p-'0';
      } else {
	if (*p == 'd') {
	  fprintf(stderr, "%d", *(int*)(data+offset));
	  offset+=4;
	} else if (*p == 's') {
	  memcpy(buf, data+offset, w);
	  buf[w] = '\0';
	  fprintf(stderr, buf);
	  offset+=w;
	} else if (*p == ' ') {
	  fprintf(stderr, " ");
	}
	w = 0;
      }
      p++;
    }
  }
}
void dump_db(DB *db, char *keyformat, char *dataformat)
{
  DBT key, data;
  DBC *tempc;
  int rc =0; 
  if ((rc = db->cursor(db, NULL, &tempc, 0)) != 0) {
    fprintf(stderr, "dump_db() DB->cursor: %s\n", db_strerror(rc));
    exit (1);
  }
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  rc = tempc->c_get(tempc, &key, &data, DB_FIRST);
  while (rc==0) {
    dump_data(keyformat, (char*)key.data);
    fprintf(stderr, "  ");
    dump_data(dataformat, (char*)data.data);
    fprintf(stderr, "\n");
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    rc = tempc->c_get(tempc, &key, &data, DB_NEXT);
  }
  rc = tempc->c_close(tempc);
}

/*************************************************************/
/*                     TEMPORARY STORAGE                     */
/*************************************************************/
static DB *temp;
DBT mykey, mydata;
int pairbuf[MAX_STR_LEN];
char tempname[128];

int tempdb_init()
{
  int rc;

  //  sprintf(tempname, ".adltemp_%d", (int)getpid());
  if ((rc = db_create(&temp, NULL, 0)) != 0) {
    fprintf(stderr, "db_create: %s\n", db_strerror(rc));
    goto exit;
  }
  if ((rc = temp->set_flags(temp, DB_DUP)) != 0) {
    fprintf(stderr, "set_flags: %s\n", db_strerror(rc));
    //    temp->err(temp, rc, "set_flags: %s", tempname);
    goto exit;
   }

  if ((rc = temp->open(temp, NULL /*tempname*/, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
    fprintf(stderr, "open: %s\n", db_strerror(rc));
    //    temp->err(temp, rc, "open: %s", tempname);
    goto exit;
  }
 exit:
  return rc;
}
int tempdb_delete()
{
  int rc;

  if ((rc = temp->close(temp, 0)) != 0) 
    goto exit;

  //  (void)unlink(tempname);

 exit:
  return rc;
}
int insertTemp(int sid, int rid, DBT *key, DBT *data)
{
  int rc; 
  int ids[2] = {sid, rid};
  
  if(temp == NULL)
    tempdb_init();

  mykey.data = &ids;
  mykey.size = sizeof(int)*2;

  mydata.data = pairbuf;
  pairbuf[0]=key->size;

  memcpy((char*)mydata.data+sizeof(int), key->data, key->size);

  if (data->size>0) 
    memcpy((char*)mydata.data+sizeof(int)+key->size, data->data, data->size);

  mydata.size = sizeof(int)+key->size+data->size;

  if ((rc = temp->put(temp, NULL, &mykey, &mydata, 0))!=0) {
    temp->err(temp, rc, tempname);
    exit(rc);
  }

 exit:
  return rc;
}

/* 
 * mvTemp: move tuples in the tempory table to db
 */
int _mvTemp(int sid, int rid, void* db, int flag);
int mvTemp(int sid, int rid, DB *db)
{
  return _mvTemp(sid, rid, (void*)db, 0);
}
int mvTemp(int sid, int rid, IM_REL *db)
{
  return _mvTemp(sid, rid, (void*)db, 1);
}
int mvTemp(int sid, int rid, RTree *db)
{
  return _mvTemp(sid, rid, (void*)db, 2);
}
int _mvTemp(int sid, int rid, void* db, int flag)
{
  int rc;
  DBT key, data;
  DBC *tempc;
  unsigned char rtree_key_buf[MAX_STR_LEN];
  int ids[2] = {sid, rid};

  if(temp == NULL)
    tempdb_init();

  if ((rc = temp->cursor(temp, NULL, &tempc, 0)) != 0) {
    fprintf(stderr, "mvTemp() DB->cursor: %s %s\n", db_strerror(rc), tempname);
    exit (1);
  }
  
  memset(&mykey, 0, sizeof(mykey));
  memset(&mydata, 0, sizeof(mydata));

  mykey.data = &ids;
  mykey.size = sizeof(int)*2;

  rc = tempc->c_get(tempc, &mykey, &mydata, DB_SET);
  while (rc==0) {
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    key.size = *(int*)mydata.data;
    key.data = (char*)mydata.data+sizeof(int);
    data.size = mydata.size - sizeof(int)-key.size;
    data.data =(char*)mydata.data+sizeof(int)+key.size;

    if (flag==0) {
      /* Berkeley DB */
      DB* dbp = (DB*)db;
      rc = dbp->put(dbp, NULL, &key, &data, 0);
    } else if (flag==1) {
      /* In Memory DB */
      IM_REL *dbp = (IM_REL*)db;
      rc = dbp->put(dbp, &key, &data, DB_APPEND);
    } else {
      /* Rtree */
      RTree *dbp = (RTree*)db;
      rc = dbp->put(dbp, &key, &data, 0);

    }
    if (rc) {
      fprintf(stderr, "mvTemp() DB->put: %s\n", db_strerror(rc));
      exit (1);
    }

    if ((rc = tempc->c_del(tempc, 0)) != 0) {
      fprintf(stderr, "mvTemp() DBC->c_del: %s\n", db_strerror(rc));
      exit (1);
    }

    rc = tempc->c_get(tempc, &mykey, &mydata, DB_NEXT_DUP);    
  }

  if (rc != DB_NOTFOUND) {
    fprintf(stderr, "mvTemp() DBC->c_get: %s %s\n", db_strerror(rc), tempname);
    exit (1);
  }

  return rc;
}

DBT kStack[MAX_STR_LEN], dStack[MAX_STR_LEN];
int kdPointer=0;
void kdPush(DBT *key, DBT *data)
{
  memcpy(&kStack[kdPointer], key, sizeof(DBT));
  memcpy(&dStack[kdPointer], data, sizeof(DBT));  
  kdPointer++;
}
void kdPop(DBT *key, DBT *data)
{
  kdPointer--;
  memcpy(key, &kStack[kdPointer], sizeof(DBT));
  memcpy(data, &dStack[kdPointer], sizeof(DBT));
}

int tmpkeysize, tmpdatasize;
char tmpkeydata[MAX_STR_LEN], tmpdatadata[MAX_STR_LEN];
void tmpStore(DBT *key, DBT *data)
{
  tmpkeysize = key->size;
  tmpdatasize = data->size;
  memcpy(tmpkeydata, key->data, tmpkeysize);
  memcpy(tmpdatadata, data->data, tmpdatasize);  
}
void tmpRecover(DBT *key, DBT *data)
{
  key->size = tmpkeysize;
  data->size = tmpdatasize;
  key->data = tmpkeydata;
  data->data = tmpdatadata;
}
/*************************************************************/
/*                     HASH FOR GB                           */
/*************************************************************/

/* 
 * For each Group-By operation, we need a table to hold all the group
 * by columns. 
 */
#define GBHASHSIZE	109
#define SLOTEMPTY	(-1)
#define SLOTDELETED	(-2)

struct _gbhashdb {
  /* we are now using in-memory tables for hash */
  IM_REL *db;
  IM_RELC *ac;
  int sid;
  int rid;
};
struct _gbhashdb gbhashdb[GBHASHSIZE];

/*
 * For each (sid, rid), gethashhandle returns a slot in gbhashdb[],
 * which holds all the group by column for that instance of gropu-by
 * operation.
 *
 *	sid: static id, same for each SQL statement in the source code. 
 *	rid: runtime id, differentiate each recursive calling of the *
 *	same SQL statement 
 */
int gethashhandle(int sid, int rid)
{
  int rc = 0;
  int rh = (sid*65599+rid) % GBHASHSIZE;
  int h;
  int foundslot = GBHASHSIZE;	// not found
  IM_REL *dbh;

  h = rh;


  do {
    if (gbhashdb[h].sid == sid && gbhashdb[h].rid == rid) {
      return h;
    } else if (gbhashdb[h].sid == SLOTEMPTY) {
      if (foundslot == GBHASHSIZE) foundslot = h;
      break;
    } else if (gbhashdb[h].sid == SLOTDELETED) {
      if (foundslot == GBHASHSIZE) foundslot = h;
      /* continue to scan ! */
    } 

    h++;
    if (h == GBHASHSIZE) h=0;

  } while (h != rh);

  /*
  while ( !(gbhashdb[h].sid == sid && gbhashdb[h].rid == rid) &&
	 gbhashdb[h].sid != -1 &&
	 ((h+1) % GBHASHSIZE) != rh) {
    h = (h+1) % GBHASHSIZE;
  }
  */

  if (h==GBHASHSIZE) {
    fprintf(stderr, "hashdb handle overflow\n");
    goto exit;
  } 

  if ((rc = im_rel_create(&dbh, NULL, IM_LINKEDLIST, 0)) != 0) {
    fprintf(stderr, "gethashhandle() im_rel_create: %s\n", db_strerror(rc));
    goto exit;
  }

  if ((rc = dbh->open(dbh, "gbhash", 0)) != 0) {
    fprintf(stderr, "gethashhandle() open: %s\n", tempname);
    goto exit;
  }

  gbhashdb[h].sid = sid;
  gbhashdb[h].rid = rid;
  gbhashdb[h].db = dbh;

  return h;

 exit:
  exit(1);
}

/*
 * If key is not NULL, hash_get() retrieves the key/data pair.
 * If key is NULL, hash_get() uses a cursor to get any key/data pair 
 * and delete that pair from the table. 
 */
int hash_get(int sid, int rid, char *&key, int size, char **data)
{
  int rc;
  int h = gethashhandle(sid, rid);
  DBT hkey, hdata;
  IM_REL *dbh = gbhashdb[h].db;

  memset(&hkey, 0, sizeof(hkey));
  memset(&hdata, 0, sizeof(hdata));

  if (key != (char*)0) {

    /* we need to allocate memory for hdata since we are now using in-memory database */
    /* NOT ANYMORE!!! SINCE JJ HAS CHANGED HIS IMPLEMENTATION */
    /* hdata.data = malloc(4);
    /* hdata.size = 4; */

    hkey.data = key; 
    hkey.size = size;

    /* im_rel->get(...) */
    rc = dbh->get(dbh, &hkey, &hdata, 0);

    if (rc == 0) {
      *(int*)data = *(int*)hdata.data;
    } else if (rc== DB_NOTFOUND) {
      *(int*)data = 0;
    } else {
      adlabort(rc, "hash_get() IM_REL->get");
    }

    /* free the memory allocated above */
    /* free (hdata.data); */

  } else {
    IM_RELC *ac = gbhashdb[h].ac;
    int flag = DB_NEXT;

    if (ac == (IM_RELC*)0) {

      if ((rc = dbh->cursor(dbh, &ac, 0)) != 0) {
	adlabort(rc, "hash_get() IM_REL->cursor()");
      }
      gbhashdb[h].ac = ac;

      flag = DB_FIRST;
    }

    /* For debugging purposes 
    printf("handler: %x flag %d\n", ac, flag);
    */

    rc = ac->c_get(ac, &hkey, &hdata, flag);

    if (rc == 0) {
      key = (char*)hkey.data;
      *(int*)data = *(int*)hdata.data;
    } else if (rc == DB_NOTFOUND) {
      int ret;

      if ((ret = ac->c_close(ac)) != 0)
	adlabort(ret, "hash_get() IM_RELC->c_close");

      gbhashdb[h].ac = (IM_RELC*)0;
    } else {
      adlabort(rc, "hash_get() IM_RELC->c_get");
    }
  }
  //For debugging purposes 
  /*	printf("hash_get [%s] %d\n", key, size); */
	
  return rc;
}

int hash_put(int sid, int rid, char *key, int size, void *data)
{
  int rc;
  int h = gethashhandle(sid, rid);
  DBT hkey, hdata;
  IM_REL *dbh = gbhashdb[h].db;

  memset(&hkey, 0, sizeof(hkey));
  memset(&hdata, 0, sizeof(hdata));
  hkey.data = key; 
  hkey.size = size;
  hdata.data = data;
  hdata.size = sizeof(int);

  if ((rc = dbh->put(dbh, &hkey, &hdata, 0))!=0) {
    /* no err function is defined in in-memory database
    dbh->err(dbh, rc, "hash_put()"); */
    printf("Err: hash_put [%s] %d\n", key, size);
    exit(rc);
  }
  /* For debugging purposes */
  return rc;
}
int hashgb_delete(int sid, int rid)
{
  int h = gethashhandle(sid, rid);
  IM_REL *dbh = gbhashdb[h].db;
  char tempname[80];
  
  dbh->close(dbh, 0);
  sprintf(tempname, ".gbhash.%d.%d.db", sid, rid);

  remove(tempname);

  gbhashdb[h].sid = SLOTDELETED;
  gbhashdb[h].db = (IM_REL*)0;
}
int hashgb_init(void)
{
  int h;
  for (h=0; h< GBHASHSIZE; h++) {
    gbhashdb[h].sid = SLOTEMPTY;
    gbhashdb[h].rid = -1;
    gbhashdb[h].db = (IM_REL*)0;
    gbhashdb[h].ac = (IM_RELC*)0;
  }
  return 0;
}

/*************************************************************/
/*                     Dynamic Library Loader                */
/*************************************************************/


static char *_ext_buf[EXT_COUNT];    // Buffers for return string 
static int _ext_cnt = 0;

struct dlnode_ {
  char *name;
  void *handle;
} dlnodes[16];
static int nextnode;


// external functions allocate return space
extern "C" 
char * _allocateResultSpace(int size)
{
  if (_ext_buf[_ext_cnt])  // this is the reused buffer
    free(_ext_buf[_ext_cnt]);
  _ext_buf[_ext_cnt] = (char*)malloc(size);
  if (!_ext_buf[_ext_cnt]){
    perror("external function return space");
    return NULL;
  }
  char* r = _ext_buf[_ext_cnt];
  _ext_cnt++;
  
  if (_ext_cnt  >= EXT_COUNT) {
    /* reuse the buffer */
    _ext_cnt = 0;
  }
  return r;
}
// external functions allocate return space for C++
char * _allocateResultSpacePlus(int size){
  return _allocateResultSpace(size);
};


// ext functions init
void _adl_dlm_init(void)
{
  _ext_cnt = 0;
  memset(_ext_buf, 0, sizeof _ext_buf);
  nextnode = 0;
}
void _adl_dlm_delete(void)
{
  int i;
  // free the result buffers
  for (i = 0; i < EXT_COUNT; i++){
    if (_ext_buf[i])
      free(_ext_buf[i]);
  }
  for (i=0; i<nextnode; i++) {
#ifdef HAVE_LIBDL
    dlclose(dlnodes[i].handle);
#else
    if (!FreeLibrary((HMODULE)dlnodes[i].handle)){
      LPVOID lpMsgBuf;
      if (!FormatMessage( 
			 FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			 FORMAT_MESSAGE_FROM_SYSTEM | 
			 FORMAT_MESSAGE_IGNORE_INSERTS,
			 NULL,
			 GetLastError(),
			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			 (LPTSTR) &lpMsgBuf,
			 0,
			 NULL ))
	{
	  // Handle the error.
	  fprintf(stderr, "FormatMessage(): code=%d\n", GetLastError());
	  exit(1);
	}
      
      fprintf(stderr, "FreeLibrary : %s\n", lpMsgBuf);
      exit(1);      
    
    }
#endif
  }
}
void *_adl_dlm(char *libname, char *funname)
{
  int i;
  void *handle, *fun;

  for (i=0; i<nextnode; i++) {
    if (strcmp(libname, dlnodes[i].name)==0)
      break;
  }
  if (i>=nextnode) {
    dlnodes[nextnode].name = libname;
#ifdef HAVE_LIBDL
    if (!(handle = dlopen (libname, RTLD_LAZY))) {
      fprintf(stderr, "dlopen: %s\n", dlerror());
      exit(1);
    }
#else
    if (!(handle = LoadLibrary(libname))){
      LPVOID lpMsgBuf;
      if (!FormatMessage( 
			 FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			 FORMAT_MESSAGE_FROM_SYSTEM | 
			 FORMAT_MESSAGE_IGNORE_INSERTS,
			 NULL,
			 GetLastError(),
			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			 (LPTSTR) &lpMsgBuf,
			 0,
			 NULL ))
	{
	  // Handle the error.
	  fprintf(stderr, "FormatMessage(): code=%d\n", GetLastError());
	  exit(1);
	}
      
      fprintf(stderr, "LoadLibrary : %s LIB Name=%s FUNC Name=%s\n", lpMsgBuf, libname, funname);
      exit(1);      
    }
    /*
  printf("DL (dlopen, etc) libraries not supported\n");
  exit(1);
    */
#endif

    dlnodes[nextnode].handle = handle;
    nextnode++;
  } else {
    handle = dlnodes[i].handle;
  }
  
#ifdef HAVE_LIBDL
  if (!(fun = dlsym(handle, funname))) {
    fprintf(stderr, "dlsym: %s\n", dlerror());
    exit(1);
  }
#else
  if (!(fun = (void*)GetProcAddress((HMODULE)handle, funname))){
      LPVOID lpMsgBuf;
      if (!FormatMessage( 
			 FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			 FORMAT_MESSAGE_FROM_SYSTEM | 
			 FORMAT_MESSAGE_IGNORE_INSERTS,
			 NULL,
			 GetLastError(),
			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			 (LPTSTR) &lpMsgBuf,
			 0,
			 NULL ))
	{
	  // Handle the error.
	  fprintf(stderr, "FormatMessage(): code=%d\n", GetLastError());
	  exit(1);
	}
      
      fprintf(stderr, "GetProcAddress() : %s DLL Name=%s FUNC Name=%s\n", lpMsgBuf, libname, funname);
      exit(1);      
    }
#endif
  return fun;
}
/*************************************************************/
/*                     SORT                                  */
/*************************************************************/
static char *sortbuf;
static int bufsize;
static int nitems;
static int itemsize;
static int sort_get_index;
int adl_sort_init()
{
  nitems = 0;
  sort_get_index = 0;
  bufsize = 4096;
  sortbuf = (char*)malloc(bufsize);
  return (!sortbuf)? 1: 0;
}
int adl_sort_put(char *tuple, int size)
{
  itemsize = size;

  if (bufsize - nitems*itemsize < itemsize) {
    bufsize*=2;
    sortbuf = (char*)realloc(sortbuf, bufsize);
    if (!sortbuf) return 1;
  }

  memcpy(sortbuf+nitems*itemsize, tuple, itemsize);
  nitems++;
  return 0;
}
int adl_sort_sort(int (*cmp)(const void *, const void *))
{
  qsort(sortbuf, nitems, itemsize, cmp);
}
int adl_sort_get(char *tuple)
{
  if (sort_get_index<nitems) {
    memcpy(tuple, sortbuf+sort_get_index*itemsize, itemsize);
    sort_get_index++;
    return 0;
  } 

  return DB_NOTFOUND;
}
int adl_sort_cleanup()
{
  if (sortbuf) free(sortbuf);
}

/* builtin function */
struct timeval __timeofday()
{
  //static char buf[20];
  /* elapsed time since Jan 1, 1970 */
  struct timeval tv;

#ifdef HAVE_GETTIMEOFDAY

  gettimeofday(&tv, NULL);
  //sprintf(buf, "%d", tv.tv_sec);

#else

  time_t seconds;
  seconds = time(NULL);
  tv.tv_sec = seconds;
  tv.tv_usec = 0;
  //sprintf(buf, "%d", seconds);

#endif

  return tv;
}




/* Set BTREE comparison functions for type int, real;
   We only consider the inequlity condition for the first key,
   other keys are stored as String.
 */
int
_int_comp(DB* dbp, const DBT *a, const DBT *b){
   int ai, bi, res, ss;
   memcpy(&ai, a->data, sizeof(int));
   memcpy(&bi, b->data, sizeof(int));
   res = ai - bi;
   if (res != 0) return res;
   ss = a->size - b->size;
   if (ss != 0) return ss;
   return strncmp((char*)a->data+sizeof(int),
   (char*)b->data+sizeof(int),
   a->size - sizeof(int));
};
int
_double_comp(DB *dbp, const DBT *a, const DBT *b){
   double ai, bi;
   int res, ss;
   memcpy(&ai, a->data, sizeof(double));
   memcpy(&bi, b->data, sizeof(double));
   res = (int)(ai - bi);
   if (res != 0) return res;
   ss = a ->size - b->size;
   if (ss != 0) return ss;
   return strncmp((char*)a->data + sizeof(double),
   (char*)b->data + sizeof(double),
   a->size - sizeof(double));
};

char *csvtok(char *str, char *delim) {
    char *ret, *match, *end;
    static char *start;

    if (str) {
       start = str;
    }
    ret = start;

    if (start == NULL) {
       return NULL;
    } else {
       if (start[0] == '"') { /* It's an open quote */
          ret = start + 1;
          end = strpbrk(ret, "\"");
          end[0] = '\0';
          start = end+2;
          return ret;
       } else {
          if ( (match = strpbrk(start, delim)) != NULL) {
             match[0] = '\0';
             start = match+1;
          } else {
             ret = start;
             start = NULL;
          }
          return ret;
       }
    }
}

#define End "._end"
#define BufferSize MAX_STR_LEN
#define IsAttr "._IsAttr"

char * _XMLAttributes (char * firstValue, char * firstName, ...)
{
	char * result;
	char * nextValue;
	char * nextName
	  ;
	va_list marker;

	result = _allocateResultSpace(BufferSize);
  
	strcpy (result, "");
	strcat (result, IsAttr);
	strcat (result, firstName);
	strcat (result, "=\"");
	strcat (result, firstValue);
	strcat (result, "\"");

	va_start( marker, firstName );     /* Initialize variable arguments. */  
	nextValue = va_arg (marker, char *);

	while (strcmp (nextValue , End) != 0)
	{
		nextName = va_arg (marker, char *);	
		strcat (result, " ");
		strcat (result, nextName);
		strcat (result, "=\"");
		strcat (result, nextValue);
		strcat (result, "\"");

		nextValue = va_arg (marker, char *);
	}

	va_end( marker );
	return result;

}

char * _XMLElement (char * Name, ...)
{
	char * result;
	char * child;
	va_list marker; 
	char * Attr_offset;
	result = _allocateResultSpace(BufferSize);

	strcpy (result , "");
  
	va_start( marker, Name );     /* Initialize variable arguments. */  
	child = va_arg (marker, char *);
  
  
	strcat (result, "<");
	strcat (result, Name);
  
	if (strcmp (child, End) != 0)
	{
		Attr_offset = strstr (child, IsAttr);
		if (Attr_offset == child)
		{
			Attr_offset += strlen (IsAttr);
			strcat (result, " ");
			strcat (result, Attr_offset);
			child = va_arg (marker, char *);
		}
	}
  
	strcat (result, ">");
  
  
	while (strcmp (child , End) != 0)
	{
		strcat (result, child);
		child = va_arg (marker, char *);
	}
	va_end( marker );              /* Reset variable arguments.      */
	strcat (result, "</");
	strcat (result, Name);
	strcat (result, ">");
	
	return result;
  
}

struct buildIExtResult {
  int tid;
  struct iExt_ val;
};


struct iExtVResult {
  int col;
  int val;
};

iExtVResult* IExtVResult(int col, int val) {
  iExtVResult* v = (iExtVResult*)_allocateResultSpace(sizeof(iExtVResult));
  v->col = col;
  v->val = val;
  return v;
}

struct rExtVResult {
  int col;
  double val;
};

rExtVResult* RExtVResult(int col, double val) {
  rExtVResult* v = (rExtVResult*)_allocateResultSpace(sizeof(rExtVResult));
  v->col = col;
  v->val = val;
  return v;
}

struct cExtVResult {
  int col;
  char val[100];
};

cExtVResult* CExtVResult(int col, char* val) {
  cExtVResult* v = (cExtVResult*)_allocateResultSpace(sizeof(cExtVResult));
  v->col = col;
  sprintf(v->val, val);
  return v;
}

struct tExtVResult {
  int col;
  timestamp val;
};

tExtVResult* TExtVResult(int col, timestamp val) {
  tExtVResult* v = (tExtVResult*)_allocateResultSpace(sizeof(tExtVResult));
  v->col = col;
  v->val = val;
  return v;
}

struct vresult {
  int id;
  int col;
  double val;
  int lbl;
};

vresult* Vresult(int id, int col, double val, int lbl) {
  vresult* v = (vresult*)_allocateResultSpace(sizeof(vresult));
  v->id = id;
  v->col = col;
  v->val = val;
  v->lbl = lbl;
  return v;
}

vresult* Vresult(int col, double val) {
  vresult* v = (vresult*)_allocateResultSpace(sizeof(vresult));
  v->col = col;
  v->val = val;
  return v;
}

struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};

struct rangeAssignment {
  double low;
  double high;
  double val;
};

rangeAssignment* RangeAssignment(double low, double high, double val) {
  rangeAssignment* r = (rangeAssignment*)_allocateResultSpace(sizeof(rangeAssignment));
  r->low = low;
  r->high = high;
  r->val = val;
  return r;
}

struct columnConfig {
  int cIndex;
  char* type;
  char* datatype;
  vector<rangeAssignment*>* rangeAssign;
  hash_map<const char*, double, std::hash<const char*>, eqstr>* catMap;
};

void addRange(columnConfig* cc, double low, double high, double val) {
  if(cc->rangeAssign != NULL) {
    cc->rangeAssign->push_back(RangeAssignment(low, high, val));
  }
}

double getRangeVal(columnConfig*cc, double val) {
  double ret = 0;
  if(cc->rangeAssign != NULL) {
    int length = cc->rangeAssign->size();
    for(int i = 0; i < length; i++) {
      if(val > (*cc->rangeAssign)[i]->low 
            && val <= (*cc->rangeAssign)[i]->high) {
        ret = (*cc->rangeAssign)[i]->val;
        break;
      }
    }
  }

  return ret;
}

void addCatMap(columnConfig* cc, char* dataVal, double val) {
  if(cc->catMap != NULL) {
    (*cc->catMap)[strdup(dataVal)] = val;
  }
}

double getCatMapVal(columnConfig* cc, char* val) {
  double ret = 0;
  if(cc->catMap != NULL) {
    ret = (*cc->catMap)[val];
  }
  
  return ret;
}

columnConfig* ColumnConfig(int index, char* t, char* dt) {
  columnConfig *c = (columnConfig*)malloc(sizeof(columnConfig));
      /*(columnConfig*)_allocateResultSpace(sizeof(columnConfig));*/

  c->cIndex = index;
  c->type = strdup(t);
  c->datatype = strdup(dt);
  if(strcmp(c->type, "d") == 0) {
    c->rangeAssign = new vector<rangeAssignment*>;
    c->catMap = NULL;
  }
  else if(strcmp(c->type, "c") == 0) {
    c->catMap = new hash_map<const char*, double, std::hash<const char*>, eqstr>;
    c->rangeAssign = NULL;
  }
  return c;
}

void deleteColumnConfig(columnConfig* ccp) {
  if(ccp->rangeAssign!=NULL) {
    while(!ccp->rangeAssign->empty()) {
      rangeAssignment* ra = (*ccp->rangeAssign)[0];
      ccp->rangeAssign->erase(ccp->rangeAssign->begin());
      free(ra);
    }
  }
  if(ccp->catMap!=NULL) {
    ccp->catMap->clear();
  }

  free(ccp);
}

void printColumnConfig(columnConfig* cc) {
  if(strcmp(cc->type, "a") == 0) {
    printf("index %d, type %s, datatype %s\n", 
             cc->cIndex, cc->type, cc->datatype);
  } 
  if(strcmp(cc->type, "c") == 0) {
    printf("index %d, type %s, datatype %s, categorical %d\n", 
             cc->cIndex, cc->type, cc->datatype, cc->catMap->size());
  } 
  if(strcmp(cc->type, "d") == 0) {
    printf("index %d, type %s, datatype %s, discrete %d\n", 
             cc->cIndex, cc->type, cc->datatype, cc->rangeAssign->size());
  } 
}

int ordercmp(const void *s1, const void *s2)
{
   int ia, ib;
   double ra, rb;
   struct timeval *ta, *tb;
   int rc;
   ia = *(int*)((char*)s1+0);
   ib = *(int*)((char*)s2+0);
   if (ia != ib) return  (ia>ib);
   ra = *(double*)((char*)s1+4);
   rb = *(double*)((char*)s2+4);
   if (ra != rb) return  (ra>rb);
   return 0;
}


vector<columnConfig*>* getColumnConfig(char* tablename) {
  DB* ccTable;
  DBC* ccTable1;
  DBT key, data;
  vector<columnConfig*>* ret = new vector<columnConfig*>;
  int rc = 0;
  int fEntry = 1;

  int col;
  char type[2];
  char datatype[11];
  double low;
  double high;
  char dataVal[21];
  double val;
  char _order_buf[MAX_STR_LEN];

  if ((rc = db_create(&ccTable, NULL, 0)) != 0) {
     adlabort(rc, "db_create()");
  }
  if ((rc = ccTable->set_pagesize(ccTable, 2048)) != 0) {
     adlabort(rc, "set_pagesize()");
  }
  if ((rc = ccTable->set_flags(ccTable, DB_DUP)) != 0) {
     adlabort(rc, "set_flags()");
  }
  /*if ((rc=ccTable->set_bt_compare(ccTable, _mytable_cmp)) != 0){
     adlabort(rc, "IM_REL->put()");
  }*/
  if ((rc = ccTable->open(ccTable, tablename, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
     adlabort(rc, "open()");
  }

  if((rc = ccTable->cursor(ccTable, NULL, &ccTable1, 0)) != 0) {
    adlabort(rc, "DB->cursor()");
  }
  adl_sort_init();
  while(rc == 0) {
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    rc = ccTable1->c_get(ccTable1, &key, &data, (fEntry)?DB_FIRST:DB_NEXT);
    if(rc == 0) {
      fEntry = 0;

      /* put col and low first, as they are order by attrs */
      memcpy(&col, (char*)data.data+0, sizeof(int));       /* col */
        //printf("1 column value col  %d\n", col);
      memcpy(_order_buf, &col, sizeof(int));       /* col */
      memcpy(&low, (char*)data.data+15, sizeof(double)); /* low */
        //printf("1 column value low  %f\n", low);
      memcpy(_order_buf+4, &low, sizeof(double)); /* low */
      memcpy(type, (char*)data.data+4, 1);              /* type */
      type[1]=0;
        //printf("1 column value type %s\n", type);
      memcpy(_order_buf+12, type, 1);              /* type */
      _order_buf[12+1]=0;
      memcpy(datatype, (char*)data.data+5, 10);             /* datatype */
      datatype[10]=0;
        //printf("1 column value datatype %s\n", datatype);
      memcpy(_order_buf+16, datatype, 10);             /* datatype */
      _order_buf[16+10]=0;
      memcpy(&high, (char*)data.data+23, sizeof(double));/* high */
        //printf("1 column value high %f\n", high);
      memcpy(_order_buf+28, &high, sizeof(double));/* high */
      memcpy(dataVal, (char*)data.data+31, 20);            /* dataval */
      dataVal[20]=0;
        //printf("1 column value dataVal %s\n", dataVal);
      memcpy(_order_buf+36, dataVal, 20);            /* dataval */
      _order_buf[36+20]=0;
      memcpy(&val, (char*)data.data+51, sizeof(double));/* val */
        //printf("1 column value val %f\n", val);
      memcpy(_order_buf+58, &val, sizeof(double));/* val */

      adl_sort_put(_order_buf, 66);
    }
  }

  /* now sort entries */
  adl_sort_sort(ordercmp);
  rc = 0;
  int curCol = -1;
  while(rc == 0) {
    memset(_order_buf, 0, MAX_STR_LEN);
    
    rc=adl_sort_get(_order_buf);
    if(rc==0) {
      memcpy(&col, _order_buf, sizeof(int));               /* col */
        //printf("column value col  %d\n", col);
      memcpy(&low, _order_buf+4, sizeof(double));          /* low */
        //printf("column value low  %f\n", low);
      memcpy(type, _order_buf+12, 1);                      /* type */
      type[1] = 0;
        //printf("column value type %s\n", type);
      memcpy(datatype,_order_buf+16, 10);                  /* datatype */
      datatype[10] = 0;
        //printf("column value datatype %s\n", datatype);
      memcpy(&high,_order_buf+28, sizeof(double));         /* high */
        //printf("column value high %f\n", high);
      memcpy(&dataVal,_order_buf+36, 20);                  /* dataval */
      dataVal[20] = 0;
        //printf("column value dataVal %s\n", dataVal);
      memcpy(&val, _order_buf+58, sizeof(double));         /* val */
        //printf("column value val %f\n", val);


      if(curCol != col) {
        ret->push_back(ColumnConfig(col, type, datatype));
        curCol = col;
      }
      if(strcmp(type, "d") == 0) {
        addRange(ret->back(), low, high, val);
      }
      if(strcmp(type, "c") == 0) {
        addCatMap(ret->back(), dataVal, val);
      }
        //printf("config %d, %s\n", col, type);
    }
    else { /* rc == DB_NOTFOUND */
      //printf("here %d\n", rc);
      adl_sort_cleanup();
    }
  }
  
  if((rc = ccTable1->c_close(ccTable1)) != 0) {
    adlabort(rc, "DBC->c_close()");
  }

  return ret;
}

int _buildIExt(int first_entry, void* t, int i) {
  const int MAX_ITEMS = 100;
  static int tid = 0;
  static int count = 0;
  struct buildIExtResult* tuple = (struct buildIExtResult*)t;
  static int* pt;
  int mybuf[MAX_ITEMS];
  
  if(tid == 0 && count == 0) {
    pt = (int*)malloc(MAX_ITEMS*sizeof(int));
  }

  if(i == 0 && count != 0) {
    tuple->tid = tid;
    tuple->val.pt = mybuf; //(int*)malloc(count*sizeof(int));
    memcpy(tuple->val.pt, pt, count*sizeof(int));
    tuple->val.length = count;
    tid++; 
    count = 0;
    return 0;
  }
  else if(i == 0 && count == 0) {
    return DB_NOTFOUND;
  }
  else {
    pt[count] = i;
    count++;
    return DB_NOTFOUND;
  }
}

int _iExtVert(int first_entry, void* t, struct iExt_ ext) {
  int i = 0;
  static int count=0;
  struct iExtVResult* tuple = (struct iExtVResult*)t;

  if(first_entry == 1)
    count = 0;

  if(count < ext.length) {
    tuple->col = count+1;
    tuple->val = ext.pt[count];
  }
  else {
    count = 0;
    return DB_NOTFOUND;
  }
  count++;
  return 0;
}

struct iExt_ _newiext(int num, ...) {
  int i = 0;
  va_list marker;
  struct iExt_ ext;
  ext.length = num;
  ext.pt = (int*)malloc(num*sizeof(int));

  va_start(marker, num);     /* Initialize variable arguments. */  
  for(i = 0 ; i < num; i++) {
    ext.pt[i] = (int)va_arg(marker, int);
  }
  va_end(marker);
  
  return ext;
}

int _deleteiext(struct iExt_ ext) {
  free(ext.pt);
  return 0;
}

int _rExtVert(int first_entry, void* t, struct rExt_ ext) {
  int i = 0;
  static int count=0;
  struct rExtVResult* tuple = (struct rExtVResult*)t;

  if(first_entry == 1)
    count = 0;

  if(count < ext.length) {
    tuple->col = count+1;
    tuple->val = ext.pt[count];
  }
  else {
    count = 0;
    return DB_NOTFOUND;
  }
  count++;
  return 0;
}

struct rExt_ _newrext(int num, ...) {
  int i = 0;
  va_list marker;
  struct rExt_ ext;
  ext.length = num;
  ext.pt = (double*)malloc(num*sizeof(double));

  va_start(marker, num);     /* Initialize variable arguments. */  
  for(i = 0 ; i < num; i++) {
    ext.pt[i] = (double)va_arg(marker, double);
  }
  va_end(marker);
  
  return ext;
}

int _deleterext(struct rExt_ ext) {
  free(ext.pt);
  return 0;
}

int _cExtVert(int first_entry, void* t, struct cExt_ ext) {
  int i = 0;
  static int count=0;
  struct cExtVResult* tuple = (struct cExtVResult*)t;

  if(first_entry == 1)
    count = 0;

  if(count < ext.length) {
    tuple->col = count+1;
    sprintf(tuple->val, ext.pt[count]);
    //tuple->val = strdup(ext.pt[count]);
    //printf("here we assigned %s\n", tuple->val);
  }
  else {
    count = 0;
    return DB_NOTFOUND;
  }
  count++;
  return 0;
}

struct cExt_ _newcext(int num, ...) {
  int i = 0;
  va_list marker;
  struct cExt_ ext;
  ext.length = num;
  ext.pt = (char**)malloc(num*sizeof(char*));

  va_start(marker, num);     /* Initialize variable arguments. */  
  for(i = 0 ; i < num; i++) {
    ext.pt[i] = strdup((char*)va_arg(marker, char*));
    //printf("here we are with %s\n", ext.pt[i]);
  }
  va_end(marker);
  
  return ext;
}

int _deletecext(struct cExt_ ext) {
  for(int i =0; i < ext.length; i++) {
    free(ext.pt[i]);
  }
  free(ext.pt);
  return 0;
}

int _tExtVert(int first_entry, void* t, struct tExt_ ext) {
  int i = 0;
  static int count=0;
  struct tExtVResult* tuple = (struct tExtVResult*)t;

  if(first_entry == 1)
    count = 0;

  if(count < ext.length) {
    tuple->col = count+1;
    tuple->val = ext.pt[count];
  }
  else {
    count = 0;
    return DB_NOTFOUND;
  }
  count++;
  return 0;
}

struct tExt_ _newtext(int num, ...) {
  int i = 0;
  va_list marker;
  struct tExt_ ext;
  ext.length = num;
  ext.pt = (timestamp*)malloc(num*sizeof(timestamp));

  va_start(marker, num);     /* Initialize variable arguments. */  
  for(i = 0 ; i < num; i++) {
    ext.pt[i] = (timestamp)va_arg(marker, timestamp);
  }
  va_end(marker);
  
  return ext;
}

int _fetchtbl(int first_entry, void* t, char* name, char* typeName) {

  return 0;
}

int _deletetext(struct tExt_ ext) {
  free(ext.pt);
  return 0;
}

int _verticalize(int first_entry, void* t, int args, char* name, ...) {
  static int count = 0;
  static char* tablename = NULL;
  static va_list marker;
  struct vresult* tuple = (struct vresult*)t;
  static int id;
  static int lbl;
  static vector<columnConfig*>* cc;
  static vector<vresult*> vt;

        //printf("count %d, %s, %s\n", count, tablename, name);

  args = args - 2; /* three arguments tablename, id and classLbl are constant
                      here we only want the number of other columns, thus ...
		   */

  if(tablename == NULL || strcmp(tablename, name) != 0) {
    /* new table specified */

    /*deallocate previous cc*/
    while(cc!=NULL && !cc->empty()) {
      columnConfig* ccp = (*cc)[0];
      cc->erase(cc->begin());
      deleteColumnConfig(ccp);
    }

    /* re-read table and column configs */
    tablename = strdup(name);
    cc = getColumnConfig(tablename);
    /*for(int j = 0; j < cc->size(); j++) {
      printColumnConfig((*cc)[j]);
    }*/
  }

  if(cc->size() != args) {
    return DB_NOTFOUND;
  }
  if(count == cc->size()-1) {
    count = 0;
    return DB_NOTFOUND;
  }
  else if(count == 0) {
    int i = 0;
    char* datatype;
    double val;

    vt.clear();
    va_start(marker, name);     /* Initialize variable arguments. */  
    /* here iterate through the va_list, get all values */
    id = (int)va_arg(marker, double);
    
    while(i < cc->size()) {
      datatype = (*cc)[i]->datatype;
      //printf("type %s,%d\n", type, i);

      /* Remember everything comes as 8-bytes here
         if datatype is int or real, then we check for discrete type 
           - if discrete the discretize, if no match then assign 0
           - if not discretize, then keep as is
         if datatype is char, then data type must be categorical
           - if not we give default value 0
           - if categorical then assign categories, 0 is default again
      */

      if(strcmp(datatype, "int") == 0) {
        val = (int)va_arg(marker, double);
        //va_arg(marker, int);
        //printf("int %d\n", val);

        if(strcmp((*cc)[i]->type, "d") == 0) {
          val = getRangeVal((*cc)[i], val);
        }
      }
      else if(strcmp(datatype, "char") == 0) {
	int first4 = va_arg(marker, int);
        int second4 = va_arg(marker, int);
        char mv[sizeof(double)+1];

        memcpy(mv, &first4, sizeof(int));
        memcpy(mv+sizeof(int), &second4, sizeof(int));
        mv[sizeof(double)] = 0;
        //printf("char %s\n", mv);

        if(strcmp((*cc)[i]->type, "c") == 0) {
          val = getCatMapVal((*cc)[i], mv);
        }
        else {
          val = 0;
        }
      }
      else if(strcmp(datatype, "real") == 0) {
        val = va_arg(marker, double);
        //printf("double %f\n", val);
        if(strcmp((*cc)[i]->type, "d") == 0) {
          val = getRangeVal((*cc)[i], val);
        }
      }

      //printf("vt %d, %d\n", i+1, val);
      if(i == cc->size()-1) {
	lbl = val;
      }
      else {
        vt.push_back(Vresult(i+1, val));
      }
      i++;
    }
    //lbl = (int)va_arg(marker, double);
    //va_arg(marker, int);
      //printf("lbl %d\n", lbl);
    va_end(marker);
  }

  /* don't come here if already done with tuple */
  //printf("here %d\n", count);
  tuple->id = id;
  tuple->col = vt[count]->col;
  tuple->val = vt[count]->val;
  tuple->lbl = lbl;
   
  count++;
  return 0;
}

int __stringToInt(char* str) {
  return atoi(str);
}

double __stringToReal(char* str) {
  return atof(str);
}

char* __intToString(int a) {
  char* result = (char*)_allocateResultSpace(100);
  sprintf(result, "%d", a);
  return result;
}

char* __realToString(double a) {
  char* result = (char*)_allocateResultSpace(100);
  sprintf(result, "%f", a);
  return result;
}


char* __strcat(const char* s1, const char* s2) {
  char* dest = (char*)malloc(100*sizeof(char));
  sprintf(dest, "%s%s", s1, s2);
  dest[99] = '\0';
  return dest;
}

struct timeval
A_Timeval(long tv_sec, long tv_usec)
{
  struct timeval* result = (struct timeval*)_allocateResultSpace(sizeof(struct timeval));

  result->tv_sec = tv_sec;
  result->tv_usec = tv_usec;
  return *result;
}

double
timeval_subtract(struct timeval x, struct timeval y)
{
  double result = x.tv_sec;
  result = result + ((double)x.tv_usec/1000000);
  result = result - y.tv_sec;
  result = result - ((double)y.tv_usec/1000000);

  return result;
}

struct timeval
timeval_add(struct timeval t, int secs)
{
  struct timeval* tv = (struct timeval*)_allocateResultSpace(sizeof(struct timeval));

  tv->tv_sec = t.tv_sec + secs;
  tv->tv_usec = t.tv_usec;
  return *tv;
}

char* __timetostring(struct timeval t) {
  char* tbuf = (char*)malloc(50*sizeof(char));
  sprintf(tbuf, "%s", ctime(&(t.tv_sec)));
  tbuf[strlen(tbuf)-1] = '\0';
  return tbuf;
}

struct timeval
timeval_subtract(int secs, struct timeval t)
{
  struct timeval* tv = (struct timeval*)_allocateResultSpace(sizeof(struct timeval));

  tv->tv_sec = secs - t.tv_sec - ((t.tv_usec == 0)?0:1);
  tv->tv_usec = 1000000 - t.tv_usec;
  return *tv;
}

struct timeval
timeval_subtract(struct timeval t, int secs)
{
  struct timeval* tv = (struct timeval*)_allocateResultSpace(sizeof(struct timeval));

  tv->tv_sec = t.tv_sec - secs;
  tv->tv_usec = t.tv_usec;
  return *tv;
}

struct timeval
timeval_add(struct timeval t, double secs)
{
  double res = t.tv_sec;
  char resChar[25];
  char* temp;
  struct timeval* tv = (struct timeval*)_allocateResultSpace(sizeof(struct timeval));

  res = res + ((double)t.tv_usec/1000000);
  res = res + secs;
  sprintf(resChar, "%f", res);
  
  tv->tv_sec = atol(strtok(resChar, "."));
  temp = strtok(NULL, ".");
  if(strlen(temp) > 6)
  {
    temp[6] = '\0';
    tv->tv_usec = atol(temp);
  }
  else if(strlen(temp) == 6)
    tv->tv_usec = atol(temp);
  else if(strlen(temp) < 6)
  {
    tv->tv_usec = (long)(atol(temp) * pow((double)10, (double)(6-strlen(temp))));
  }
    
  return *tv;
}

struct timeval
timeval_subtract(double secs, struct timeval t)
{
  double res = t.tv_sec;
  char resChar[25];
  char* temp;
  struct timeval* tv = (struct timeval*)_allocateResultSpace(sizeof(struct timeval));

  res = res + ((double)t.tv_usec/1000000);
  res = secs - res;
  sprintf(resChar, "%f", res);
  
  tv->tv_sec = atol(strtok(resChar, "."));
  temp = strtok(NULL, ".");
  if(strlen(temp) > 6)
  {
    temp[6] = '\0';
    tv->tv_usec = atol(temp);
  }
  else if(strlen(temp) == 6)
    tv->tv_usec = atol(temp);
  else if(strlen(temp) < 6)
  {
    tv->tv_usec = (long)(atol(temp) * pow((double)10, (double)(6-strlen(temp))));
  }
    
  return *tv;
}

struct timeval
timeval_subtract(struct timeval t, double secs)
{
  double res = t.tv_sec;
  char resChar[25];
  char* temp;
  struct timeval* tv = (struct timeval*)_allocateResultSpace(sizeof(struct timeval));

  res = res + ((double)t.tv_usec/1000000);
  res = res - secs;
  sprintf(resChar, "%f", res);
  
  tv->tv_sec = atol(strtok(resChar, "."));
  temp = strtok(NULL, ".");
  if(strlen(temp) > 6)
  {
    temp[6] = '\0';
    tv->tv_usec = atol(temp);
  }
  else if(strlen(temp) == 6)
    tv->tv_usec = atol(temp);
  else if(strlen(temp) < 6)
  {
    tv->tv_usec = (long)(atol(temp) * pow((double)10, (double)(6-strlen(temp))));
  }
    
  return *tv;
}

int
timeval_cmp(struct timeval x, struct timeval y)
{
  long result = x.tv_sec - y.tv_sec;

  //printf("comparing %d.%d and %d.%d\n", x.tv_sec, x.tv_usec, y.tv_sec, y.tv_usec);
  //fflush(stdout);
 
  if(result != 0)
    return (int)result;

  result = x.tv_usec - y.tv_usec;
  return (int)result;
}

struct timeval
getTimeval(char* timest)
{
  struct timeval* t;
  struct tm* tm = (struct tm*)malloc(sizeof(struct tm));

  t = (struct timeval*)_allocateResultSpace(sizeof(struct timeval));

  if(tm == NULL || timest == NULL)
  {
    t->tv_sec = 0;
    t->tv_usec = 0;
    return *t;
  }

  memset(tm, 0, sizeof(struct tm));
  char* ret = strptime(timest, TIMESTAMP_FORMAT, tm);

  if(ret == NULL)
  {
    t->tv_sec = 0;
    t->tv_usec = 0;
    free(tm);
    return *t;
  }

  t->tv_sec = mktime(tm);
  t->tv_usec = (long)0;

  /* For daylight savings, not completely sure but it seems to work */
  if(tm->tm_isdst > 0)
    t->tv_sec = t->tv_sec - 3600;

  free(tm);

  if(ret[0] == '.')
  {
    ret ++;
    if(strlen(ret) > 6)
    {
      ret[6] = '\0';
      t->tv_usec = atol(ret);
    }
    else if(strlen(ret) == 6)
      t->tv_usec = atol(ret);
    else
    {
      t->tv_usec = (long)(atol(ret) * pow((double)10, (double)(6-strlen(ret))));
    }
  }

  return *t;
}

void setModelId(const char* s) {
  __modelId__[0] = '\0';
  sprintf(__modelId__, "%s", s);
  //printf("setModelId %s\n", __modelId__);fflush(stdout);
}

char* getModelId() {
  //printf("getModelId %s\n", __modelId__);fflush(stdout);
  return __modelId__;
}

