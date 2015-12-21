#include <stdlib.h>
#include <time.h>

extern char * _allocateResultSpace(int size);

struct timeval test(struct timeval tv)
{
  struct timeval* t = (struct timeval*)_allocateResultSpace(sizeof(struct timeval));

  t->tv_sec = tv.tv_sec + 10;
  t->tv_usec = tv.tv_usec;
  return *t;
}
