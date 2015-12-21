#include <semant.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "util.h"
#include <sql/adl_sys.h>
#include "const.h"
extern "C" {
#include <dbug/dbug.h>
}
#include <string>
#include <buffer.h>


#include <ios/ios.h>
using namespace ESL;

using namespace std;
extern system_t *ntsys;
extern char queryName[1024];

/************************************************************
                      MEMORY
************************************************************/
static inline char *tmpAlloc(int size)
{
  char *ret = ntsys->freespace;
  ntsys->freespace += intAlign(size);
  if (ntsys->freespace - ntsys->memory >= NTSQL_MEMORY_POOL) {
    EM_error(0, ERR_OUT_OF_MEMORY, __LINE__, __FILE__, "temporary space");
    exit(1);
  }
  return ret;
}
char *myMalloc(int size, char *file, int line)
{
  char *ret;
  ntsys->permanent = 1;
  //  if (ntsys->permanent) 
    ret = ((char *)malloc(size));
    //  else {
    //    ret = tmpAlloc(size);
    //  }

  DBUG_PRINT("adlmem", ("%x %s MALLOC SIZE %d AT %s(%d)\n", 
			ret, ntsys->permanent? "":"TEMP_POOL", size, file, line));

  return ret;
}
char *myCalloc(int size, char *file, int line)
{
  char *ret;
  if (ntsys->permanent)
    ret = ((char *)calloc(1,size));
  else {
    ret = tmpAlloc(size);
    memset(ret, 0, size);
  }

  DBUG_PRINT("adlmem", ("%x CALLOC size %d AT %s(%d)\n", 
			ret, size, file, line));

  return ret;
}
void myFree(char *ptr, char *file, int line)
{
  if (ntsys->permanent)
    free(ptr);

  DBUG_PRINT("adlmem", ("%x FREE AT %s(%d)\n", 
			ptr, file, line));
}
char *myRealloc(char *p, int size, char *file, int line)
{
  char *ret;
  if (ntsys->permanent)
    ret = ((char*)realloc(p, size));
  else {
    ret = tmpAlloc(size);
    memmove(ret, p, size);
  }

  DBUG_PRINT("adlmem", ("%x REALLOC TO %x size %d AT %s(%d)\n", 
			p, ret, size, file, line));

  return ret;
}
/************************************************************
                      STRING
************************************************************/
#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif

void lowerCaseStr(char *s)
{
  while ((*s=tolower(*s)));
}
void upperCaseStr(char *s)
{
  while ((*s=toupper(*s)));
}
char *makeStr(char c, int size)
{
  static char buf[MAX_STR_LEN];
  int len = MIN(size, 1023);
  memset(buf, c, len);
  buf[len]='\0';
  return buf;
}
char *copyStrLen(char *str, int len)
{
  char *s = (char*)ntCalloc(len+1);
  if (str)
    memcpy(s, str, MIN(strlen(str), len));
  return s;
}
char *copyStr (const char *str)
{
  char *s;

  if (str == (char*)0)
    return ((char *) 0);
  s = ntMalloc (strlen (str) + 1);
  (void) strcpy (s, str);
  return (s);
}

struct timeval* copyTimeval(const struct timeval* tm)
{
  struct timeval* t;

  if(tm == NULL)
    return (struct timeval*)NULL;

  t = (struct timeval*)ntMalloc(sizeof(struct timeval));
  t->tv_sec = tm->tv_sec;
  t->tv_usec = tm->tv_usec;
  return t;
}

int equalStr(const char *s1, const char *s2)
{
  if (s1==s2)
    return 1;
  if (s1==(char*)0 || s2==(char*)0)
    return 0;
  return (strcmp(s1,s2)==0);
}
char *ltrim(char *string)
{
  register char *s;
  for (s = string; whitespace (*s); s++);
  return s;
}
char *rtrim(char *string)
{
  register char *s;

  if (*string == '\0')
    return string;

  s = string + strlen(string)-1;

  while (s > string && whitespace (*s))
    s--;
  *++s = '\0';

  return string;
}
char *trim(char *string)
{
  return rtrim(ltrim(string));
}
/************************************************************
                      MISC
************************************************************/

// change '\' in filename to '//'
char *os_filename(char *file)
{
#ifdef __CYGWIN__
  static char buffer[MAX_STR_LEN];
  char *t = buffer;
  char *s = file;
  while ( (*t++ = *s++) ) {
    if (*(s-1) == '\\') {
      *(t-1) = '/';
      *t++ = '/';
    } 
  }
  return buffer;
#else
  return file;
#endif
} 

char *getUniqueName(char *prefix)
{
  static int unique = 0;
  char *buf= (char*)ntMalloc(strlen(prefix)+12);
  sprintf(buf, "%s_%04d", prefix, unique++);
  return buf;
}
int getUId(int *id)
{
  static int buf[10240];
  static int count=0;
  int i;
  
  for (i=0; i<count; i++)
    if (buf[i]==(int)id)
      return i;

  buf[i]=(int)id;
  count++;

  if (count>=10240) {
    fprintf(stderr, "getUniqueId() buffer overflow\n");
    exit(1);
  }

  return i;
}




bool isESL(){
  if(isAdHoc())
    return false;
  return setQueryName();
};


