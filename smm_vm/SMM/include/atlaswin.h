#ifndef ATLASWIN_H
#define ALTASWIN_H

#include <stdio.h>
#include <types.h>
#include <db.h>
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
/*
 * For each OVER(PARTITION ... SORT ... ROWS ..) struct
 * there is a ws_t structure.
 */

typedef struct _adl_win_ctrl_s *_adl_win_ctrl_t;
struct _adl_win_ctrl_s
{
  /* window size */
  int keySize;
  int dataSize;
  int winSize;			/* 1. # of rows, or
				   2. time span (in seconds) */
  _adl_win_type winType;

  /* disk buffer */
  char fname[240];
  FILE *f;
  int lastBlock;

  /* key hash */
  struct hash_control *keyhash;

  /* input buffer */
  char *keybuf;
  char *databuf;

  /* free buffers */
  int *freeBlocks;
  int nFree;
  int nFreeCapacity;

  int nRecords;
  int blockSize;
};

/*
 * offset in a file
 */
typedef struct _adl_win_off_s _adl_win_off_t; 
struct _adl_win_off_s
{
  int block;
  int off;
};

/*
 * This structure is passed into each UDA so that the UDA can access
 * the tuples in the window as well as the Expired tuple.
 */
typedef struct _adl_win_s *_adl_win_t;
struct _adl_win_s
{
  _adl_win_ctrl_t wc;
  int count;
  _adl_win_off_t begin;
  _adl_win_off_t expired;
  _adl_win_off_t current;
};


typedef struct _adl_winc_s *_adl_winc_t;
struct _adl_winc_s
{
  _adl_win_t win;
  _adl_win_off_t *cursor_beg;
  _adl_win_off_t *cursor_end;
  _adl_win_off_t cursor;
  char *block;
  int (*c_get)(_adl_winc_t winc, DBT *key, DBT *data, u_int32_t flags);
  int (*c_close)(_adl_winc_t winc);
};

_adl_win_ctrl_t _adl_newWinCtrl(_adl_win_type type, 
				int size, 
				int keysize, 
				int datasize);

void _adl_deleteWinCtrl(_adl_win_ctrl_t wc);
_adl_win_t _adl_newWin(_adl_win_ctrl_t wc);
void _adl_deleteWin(_adl_win_t win);
_adl_win_t _adl_winPutTuple(_adl_win_ctrl_t wc, char *key, char *data);
//_adl_win_t _adl_winPutTuple(_adl_win_ctrl_t wc);
int _adl_winGetTuple(_adl_win_t win, DBT *key, DBT *data, u_int32_t flags);
int _adl_winGetExpiredTuple(_adl_win_t win, DBT *key, DBT *data, u_int32_t flags);
int _adl_winHasExpired(_adl_win_t win);
void _adl_winDisplay(_adl_win_t win, void (*displayData)(char *));
int _adl_winCursor(_adl_win_t win, _adl_winc_t *cursor, int expired_p);
#endif
