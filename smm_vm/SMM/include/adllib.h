#ifndef _ADL_H
#define _ADL_H

extern "C" {
#include <im_db.h>
}
#include <rtree.h>
#include <sys/types.h>
#include <stdio.h>
#include <db.h>
#include <stdarg.h>
#include "err.h"

#include "atlaswin.h"


struct eqstrTab
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};

struct iExt_ {
  int length;
  int *pt;
};
typedef struct iExt_* iExt;

struct rExt_ {
  int length;
  double *pt;
};
typedef struct rExt_* rExt;

struct cExt_ {
  int length;
  char **pt;
};
typedef struct cExt_* cExt;

struct tExt_ {
  int length;
  timestamp *pt;
};
typedef struct tExt_* tExt;

#include <buffer.h>
#include <winbuf.h>
void adlabort(int rc, char *fmt, ...);
void adlabortESL(ESL::buffer* errorBuf, int rc, char *fmt, ...);
void adlabortESLAggr(ESL::buffer* errorBuf, int rc, char *fmt, ...);
void adlerror(int rc, char *fmt, ...);
void adltrace(char *fun, int status, int flag);

typedef struct A_timeexp_ *A_timeexp;
struct A_timeexp_ {
  bool deleted;
  ESL::winbuf* wbuf;
  };
A_timeexp A_Timeexp(ESL::winbuf* wbuf);

/*************************************************************/
/*                     TEMPORARY STORAGE                     */
/*************************************************************/
int tempdb_init();
int tempdb_delete();
int insertTemp(int sid, int rid, DBT *key, DBT *data);
int mvTemp(int sid, int rid, DB *db);
int mvTemp(int sid, int rid, RTree *db);
int mvTemp(int sid, int rid, IM_REL *db);
void tmpStore(DBT *key, DBT *data);
void tmpRecover(DBT *key, DBT *data);
void kdPush(DBT *key, DBT *data);
void kdPop(DBT *key, DBT *data);

/*************************************************************/
/*                     HASH FOR GB                           */
/*************************************************************/
int hash_put(int sid, int rid, char *key, int size, void *data);
int hash_get(int sid, int rid, char *&key, int size, char **data);
int hashgb_delete(int sid, int rid);
int hashgb_init(void);

/*************************************************************/
/*                     Dynamic Library Loader                */
/*************************************************************/
void _adl_dlm_init(void);
void _adl_dlm_delete(void);
void *_adl_dlm(char *libname, char *funname);
// external functions allocate return space
extern "C" 
char * _allocateResultSpace(int size);
char * _allocateResultSpacePlus(int size);
/*************************************************************/
/*                     SORT                                  */
/*************************************************************/
int adl_sort_init();
int adl_sort_put(char *tuple, int size);
int adl_sort_sort(int (*cmp)(const void *, const void *));
int adl_sort_get(char *tuple);
int adl_sort_cleanup();
/*************************************************************/
/*                     BUILTIN FUNCTIONS                     */
/*************************************************************/
struct timeval __timeofday();
char *csvtok(char *str, char *delim);
/*************************************************************/
/*            BTREE COMPARISON FUNCTIONS                     */
/*************************************************************/
/* Set BTREE comparison functions for type int, real
 */
int _int_comp(DB* dbp, const DBT *a, const DBT *b);
int _double_comp(DB *dbp, const DBT *a, const DBT *b);

char * _XMLElement (char * Name, ...);
char * _XMLAttributes (char * firstValue, char * firstName, ...);
int _verticalize(int first_entry, void* tuple, int args, char* name, ...);
int _iExtVert(int first_entry, void* tuple, struct iExt_ ext);
int _buildIExt(int first_entry, void* t, int i);
struct iExt_ _newiext(int num, ...);
int _deleteiext(struct iExt_);
int _rExtVert(int first_entry, void* tuple, struct rExt_ ext);
struct rExt_ _newrext(int num, ...);
int _deleterext(struct rExt_);
int _cExtVert(int first_entry, void* tuple, struct cExt_ ext);
struct cExt_ _newcext(int num, ...);
int _deletecext(struct cExt_);
int _tExtVert(int first_entry, void* tuple, struct tExt_ ext);
struct tExt_ _newtext(int num, ...);
int _deletetext(struct tExt_);
int _fetchtbl(int first_entry, void* tuple, char* name, char* typeName);


struct timeval
A_Timeval(long tv_sec, long tv_usec);

double
timeval_subtract (struct timeval x, struct timeval y);

int
timeval_cmp (struct timeval x, struct timeval y);

struct timeval
timeval_add(struct timeval t, int secs);

struct timeval
timeval_subtract(int secs, struct timeval t);

struct timeval
timeval_subtract(struct timeval t, int secs);

struct timeval
timeval_add(struct timeval t, double secs);

struct timeval
timeval_subtract(double secs, struct timeval t);

struct timeval
timeval_subtract(struct timeval t, double secs);

struct timeval
getTimeval(char* timest);


int __stringToInt(char* str);
double __stringToReal(char* str);
char* __intToString(int a);
char* __realToString(double a);
char* __strcat(const char* s1, const char* s2);
char* __timetostring(struct timeval t);

static char __modelId__[1024];
void setModelId(const char*s);
char* getModelId();

#endif
