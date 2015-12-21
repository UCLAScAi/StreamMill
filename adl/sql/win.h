#ifndef WIN_C
#define WIN_C

#include "adlhash.h"
#include <stdio.h>

/*-----------------------------------------------------------

----
|  |<- last_expired
|  |
|  |<- expired
|  |
|  |
|  |<- current
----

-----------------------------------------------------------*/
typedef struct offset_s win_off; 
struct offset_s
{
  int block;
  int off;
};

typedef struct win_s *win_t;
struct win_s
{
  win_off begin;
  win_off expired;
  win_off current;

  win_off cursor;
  win_off expired_cursor;
};

typedef struct ws_s *ws_t;
struct wins_s
{
  int RecSize;
  int BlockSize;
  char *rec;
  FILE *f;
  struct win_s *w;

  /* key hash */
  hash_t keyhash;

  /* free buffers */
  int *freeBlocks;
  int nFree;
  int nFreeCapacity;
};

ws_t newWS(int size, int keysize, int datasize);
void deleteWS(ws_t w);
win_t putTuple(ws_t ws, char *key, char *data);
int getTuple(win_t win);
int getExpiredTuple(win_t win);

#endif
