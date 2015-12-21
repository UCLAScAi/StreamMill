#ifndef UTIL_H
#define UTIL_H

#include <time.h>

/************************************************************
                  MEMORY ALLOCATION
************************************************************/
#define ntFree(x) myFree((char*)x, __FILE__, __LINE__)
#define ntMalloc(size) myMalloc(size, __FILE__, __LINE__)
#define ntCalloc(size) myCalloc(size, __FILE__, __LINE__)
#define ntRealloc(x, size) myRealloc((char*)x, size, __FILE__, __LINE__)

char *myMalloc(int size, char *file, int line);
char *myCalloc(int size, char *file, int line);
void myFree(char *ptr, char *file, int line);
char *myRealloc(char *p, int size, char *file, int line);

/************************************************************
                  STRING
************************************************************/
char *copyStr (const char *str);
struct timeval* copyTimeval (const struct timeval* tm);
char *copyStrLen(char *str, int len);
int equalStr (const char *str1, const char *str2);
char *ltrim(char *str);
char *rtrim(char *str);
char *trim(char *str);
char *makeStr(char c, int size);
/************************************************************
                  MISC
************************************************************/
char* os_filename(char *file);
char *getUniqueName(char *prefix);

// Given a pointer x, get an unique ID for this pointer
// First invocation assign a new UID while subsequent invocations return this same UID
#define UID(x) getUId((int*)(x))
int getUId(int *id);

/* align at integer (4 bytes) boundary */
#define intAlign(len) ( (1 + ((len-1)>>2)) << 2)


// MIN and MAX are changed to upper case to avoid conflicts with iostream
#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

// is the compiler used in ESL?
bool isESL();

#endif






