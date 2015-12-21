#ifndef _IN_MEM_DB_H_
#define	_IN_MEM_DB_H_

#include <stdio.h>
#include <stdlib.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef linux
#include <linux/types.h>	/* for size_t */
#endif

#include "db.h"
#include "ansidecl.h"
#include "im_db_buffer.h"

/* please check db.h and avoid any conflicts with other DB_... values */
#define DB_GET_OIDOFKEY	100
#define DB_GET_OID	101

/*
 * 0. Ready-made hash table (for hash index)
 */
typedef struct hash_control *HashTable;

/*
 * 1. 
 */
/* the list of tuples: Tuple Linked List */
struct __tll; typedef struct __tll TLL;
struct __tll
{
    /*
     * Storage
     */
    DBT key, data;

    /*
     * Control information
     */
#define TUPLE_TOMBSTONE 1
#define TUPLE_DELETED(x) (((x) & TUPLE_TOMBSTONE) == TUPLE_TOMBSTONE)
    char status;	/* status flag */

    /* links */
    TLL *prev, *next;	/* double link to support fast access and cursors */
    TLL *hchain;	/* the hash chain */
};

/*
 * IM_PAGEDARRAY is a page-oriented array of variable-length tuples;
 * IM_LINKEDLLIST is a linked list of variable-length tuples
 */
typedef enum {IM_LINKEDLIST=1} IM_DBTYPE;

struct __im_db_env; typedef struct __im_db_env IM_DB_ENV;
struct __im_relc; typedef struct __im_relc IM_RELC;
struct __im_rel; typedef struct __im_rel IM_REL;
struct __im_db_env
{
    FILE *db_errfile;		/* Error message file stream. */
    const char *db_errpfx;	/* Error message prefix. */

    void (*err) __P((const IM_DB_ENV *, int, const char *, ...));
    void (*errx) __P((const IM_DB_ENV *, const char *, ...));

    void (*set_errfile) __P((IM_DB_ENV *, FILE *));
    void (*set_errpfx) __P((IM_DB_ENV *, const char *));
};
struct __im_relc
{
    /* Related relation */
    IM_REL *rel;

    /* For LINKED_LIST, TLL pointer */ 
    TLL *cur;

  TLL *nextaccess;

    int (*c_close) __P((IM_RELC *));	/* Methods: public. */
    int (*c_get) __P((IM_RELC *, DBT *, DBT *, u_int32_t));
    int (*c_put) __P((IM_RELC *, DBT *, DBT *, u_int32_t));
    int (*c_del) __P((IM_RELC *, u_int32_t));
  int (*sameAs) __P((IM_RELC *, IM_RELC*));
};
struct __im_rel
{
    /* the db environment */
    IM_DB_ENV *dbenv;

    /* what type it is */
    IM_DBTYPE type;
    /* subtypes */
#define IM_OID 1
#define IM_REL_INDEXED 2
#define IS_OID_REL(x) (((x) & IM_OID) == IM_OID)
#define IS_INDEXED_REL(x) (((x) & IM_INDEXED) == IM_INDEXED)
    u_int32_t flags;

    /* control information */
    /* char *name; */
  char name[80];		/* Haixun Wang */

    /* physical level information for LINKED_LIST */
    TLL *head;

    /* hash index, may be NULL */
    HashTable hidx;

    /* BerkeleyDB emulation for returning data
       (for REL->get and RELC->c_get) */
    DBT rkey, rdata;

  char rkey_data[1024];		/* Haixun Wang */
  char rdata_data[1024];	/* Haixun Wang */

  /* buffer management --Haixun Wang
   * 
   */
  buffer_t buffer;

    int (*cursor) __P((IM_REL *, IM_RELC **, u_int32_t));
    int (*open) __P((IM_REL *, char *, u_int32_t));
    int (*close)__P((IM_REL *, u_int32_t));
    int (*get)	__P((IM_REL *, DBT *, DBT *, u_int32_t));
    int (*put)	__P((IM_REL *, DBT *, DBT *, u_int32_t));
    int (*del)	__P((IM_REL *, DBT *, u_int32_t));

    void (*print) __P((IM_REL *));
};

/**************** function prototypes ****************/
int im_rel_create(IM_REL **, IM_DB_ENV *, IM_DBTYPE, u_int32_t);

int im_rel_open __P((IM_REL *, char *, u_int32_t));
int im_rel_close __P((IM_REL *, u_int32_t));
int im_rel_get __P((IM_REL *, DBT *, DBT *, u_int32_t));
int im_rel_put __P((IM_REL *, DBT *, DBT *, u_int32_t));
int im_rel_del __P((IM_REL *, DBT *, u_int32_t));

/****************************************************************
 * API routines
 ****************************************************************/
/*** from mylib.c ***/
int __my_get_errno __P((void));
void __my_set_errno __P((int));
int __my_strdup __P((const char *, void *));
int __my_calloc __P((size_t, size_t, void *));
int __my_malloc __P((size_t, void *));
int __my_realloc __P((size_t, void *));

/*** from im_cursor.c ***/
int im_rel_cursor __P((IM_REL *, IM_RELC **, u_int32_t));
int im_rel_cursor_close __P((IM_RELC *));
int im_rel_cursor_get __P((IM_RELC *, DBT *, DBT *, u_int32_t));
int im_rel_cursor_put __P((IM_RELC *, DBT *, DBT *, u_int32_t));
int im_rel_cursor_del __P((IM_RELC *, u_int32_t));
int im_rel_cursor_same __P((IM_RELC *, IM_RELC*));

/*** from im_db_env_method.c ***/
int  __im_dbenv_init __P((IM_DB_ENV *));

/*** from im_db_err.c ***/
void __im_db_err __P((const IM_DB_ENV *, const char *, ...));
void __im_db_real_err __P((const IM_DB_ENV *, int, int, const char *, va_list));

/*** from im_debug.c ***/
void im_rel_print(IM_REL *);

/*** from hash-arbitrary.c ***/
struct hash_control;
/* returns control block */
HashTable hash_new __P((void));
void hash_die __P((HashTable));
/* returns previous value */
PTR hash_delete __P((HashTable, const void *, const size_t));
/* returns previous value */
PTR hash_replace __P((HashTable, const void *, const size_t, PTR));
/* returns error string or null */
const char *hash_insert __P((HashTable, const void *, const size_t, PTR));
/* returns value */
PTR hash_find __P((HashTable, const void *, const size_t));
/* returns error text or null (internal) */
const char *hash_jam __P((HashTable, const void *, const size_t, PTR));

#endif /* !_IN_MEM_DB_H_ */
