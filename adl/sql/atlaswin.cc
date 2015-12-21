#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "atlaswin.h"
#include "ansidecl.h"
extern "C" {
#include <hash-arbitrary.h>
}

#define PAGE_SIZE 4096

void freeBlock(_adl_win_ctrl_t wc, int block_id)
{
  wc->freeBlocks[wc->nFree++]=block_id;
  if (wc->nFree >= wc->nFreeCapacity) {
    wc->nFreeCapacity *= 2;
    wc->freeBlocks = (int*)realloc(wc->freeBlocks, sizeof(int)*wc->nFreeCapacity);
  }
}
int newBlock(_adl_win_ctrl_t wc)
{
  return (wc->nFree > 0)?
    wc->freeBlocks[--wc->nFree]: 
    wc->lastBlock++;
}

static int file_id=0;
_adl_win_ctrl_t _adl_newWinCtrl(_adl_win_type type, 
				int size, 
				int keysize, 
				int datasize)
{
  _adl_win_ctrl_t wc = (_adl_win_ctrl_t)malloc(sizeof(*wc));

  wc->winType = type;
  wc->winSize = size;
  wc->keySize = keysize;
  wc->dataSize = datasize;

  /* size of each block */
  wc->blockSize = (wc->dataSize< PAGE_SIZE - sizeof(int))? 
    PAGE_SIZE : (2*wc->dataSize/PAGE_SIZE)*PAGE_SIZE;
  /* # of records that can fit into one block */
  wc->nRecords = wc->blockSize/wc->dataSize;
  
  /* hash table */
  wc->keyhash = hash_new ();

  /* disk buffer */
  sprintf(wc->fname, "_adl_win_buf_%d.data", file_id++);
  wc->f = fopen(wc->fname, "w+b");
  if (!wc->f) {
    printf("Cannot open file %s\n", wc->fname);
    exit(1);
  }
  wc->lastBlock = 0;

  /* free block management */
  wc->nFreeCapacity = 1024;
  wc->nFree=0;
  wc->freeBlocks = (int*)malloc(sizeof(int)*wc->nFreeCapacity);

  /* input buffer */
  wc->keybuf = (char*)malloc(wc->keySize);
  wc->databuf = (char*)malloc(wc->dataSize);
  return wc;
}

void _adl_deleteWinCtrl(_adl_win_ctrl_t wc)
{
  // TODO: hash traverse kill each win
  hash_die (wc->keyhash);
  fclose(wc->f);
  remove(wc->fname);
  free(wc->freeBlocks);
  free(wc->keybuf);
  free(wc->databuf);
  free(wc);
}
_adl_win_t _adl_newWin(_adl_win_ctrl_t wc)
{
  _adl_win_t win = (_adl_win_t)malloc(sizeof(*win));
  win->wc = wc;

  win->begin.block = newBlock(win->wc);
  win->begin.off   = 0;
  win->expired.block = win->begin.block;
  win->expired.off = 0;
  win->current.block = win->begin.block;
  win->current.off = 0;
  win->count = 0;
  return win;
}
void _adl_deleteWin(_adl_win_t win)
{
  free(win);
}
int _adl_winHasExpired(_adl_win_t win)
{
  return (win->expired.block != win->begin.block ||
	  win->expired.off != win->begin.off);
}
inline long DiskOffset(_adl_win_ctrl_t wc, int block)
{
  return block*wc->blockSize;
}
inline long DiskOffset(_adl_win_ctrl_t wc, _adl_win_off_t off)
{
  return off.block*wc->blockSize + off.off*wc->dataSize;
}
// write the block id to the old block
inline void writeNextBlockId(_adl_win_ctrl_t wc, int block, int next_block)
{
  long offset = DiskOffset(wc, block+1) - sizeof(int);
  fseek(wc->f, offset, SEEK_SET);
  fwrite(&next_block, sizeof(int), 1, wc->f);
}
inline int readNextBlockId(_adl_win_ctrl_t wc, int block)
{
  int new_block;
  long offset = DiskOffset(wc, block+1) - sizeof(int);
  fseek(wc->f, offset, SEEK_SET);
  fread(&new_block, sizeof(int), 1, wc->f);
  return new_block;
}
_adl_win_t _adl_winPutTuple(_adl_win_ctrl_t wc)
{
  return _adl_winPutTuple(wc, wc->keybuf, wc->databuf);
}
_adl_win_t _adl_winPutTuple(_adl_win_ctrl_t wc, char *key, char *data)
{
  _adl_win_t win;
  if ( !(win = (_adl_win_t)hash_find(wc->keyhash, key, wc->keySize)) ) {
    win = _adl_newWin(wc);
    hash_insert(wc->keyhash, key, wc->keySize, win);
  }

  fseek(wc->f, DiskOffset(wc, win->current), SEEK_SET);
  fwrite(data, wc->dataSize, 1, wc->f);

  win->current.off++;
  if (win->current.off >= wc->nRecords) {
    int newblock = newBlock(wc);
    writeNextBlockId(wc, win->current.block, newblock);
    win->current.block = newblock;
    win->current.off = 0;
  }
  win->count++;

  if (wc->winType == _ADL_WIN_TIME) {
    // TIME WINDOW
  } else {
    // ROW WINDOW
    if (win->count > wc->winSize) {
      int expired_block = win->expired.block;

      /* set expired-pointer to begin-pointer */
      win->expired.block = win->begin.block;
      win->expired.off = win->begin.off;
      if (win->expired.off == 0) {
	freeBlock(wc, expired_block);
      }
      
      /* advance begin-pointer */
      win->begin.off++;
      if (win->begin.off >= wc->nRecords) {
	win->begin.block = readNextBlockId(wc, win->begin.block);
	win->begin.off = 0;
      }
      win->count--;
    }
  }

  return win;
}

int _adl_winGetTuple(_adl_win_t win, DBT *key, DBT *data, u_int32_t flags)
{
}
int _adl_winGetExpiredTuple(_adl_win_t win, DBT *key, DBT *data, u_int32_t flags)
{
}

void _adl_winDisplay(_adl_win_t win, void (*displayData)(char *))
{
  _adl_win_ctrl_t wc = win->wc;
  _adl_win_off_t p;
  char *data = (char*)malloc(wc->dataSize);

  p.block = win->expired.block;
  p.off = win->expired.off;
  printf("Expired:\n");
  while (p.block<win->current.block || 
	 (p.block==win->current.block && p.off <win->current.off)) {
    if (p.block==win->begin.block && p.off == win->begin.off) {
      printf("Begin:\n");
    } 
    fseek(wc->f, DiskOffset(wc, p), SEEK_SET);
    fread(data, wc->dataSize, 1, wc->f);
    displayData(data);
    p.off++;
    if (p.off>=wc->nRecords) {
      p.block=readNextBlockId(wc, p.block);
      p.off=0;
    }
  }
  printf("Current:\n");
  free(data);
}


int winc_get(_adl_winc_t winc, DBT *key, DBT *data, u_int32_t flags)
{
  int ret;
  _adl_win_ctrl_t wc = winc->win->wc;

  switch(flags) {
  case DB_FIRST:
    {
      winc->cursor.block = winc->cursor_beg->block;
      winc->cursor.off = winc->cursor_beg->off;
      fseek(wc->f, DiskOffset(wc, winc->cursor.block), SEEK_SET);
      fread(winc->block, wc->blockSize, 1, wc->f);
    }
  case DB_NEXT:
    {
      if (winc->cursor.block < winc->cursor_end->block ||
	  (winc->cursor.block == winc->cursor_end->block &&
	   winc->cursor.off <winc->cursor_end->off)) 
	{
	  data->size = wc->dataSize;
	  data->data = winc->block+winc->cursor.off*wc->dataSize;
	  winc->cursor.off++;
	  if (winc->cursor.off>=wc->nRecords) {
	    winc->cursor.off = 0;
	    winc->cursor.block = *(winc->block+wc->blockSize-sizeof(int));
	    fseek(wc->f, DiskOffset(wc, winc->cursor.block), SEEK_SET);
	    fread(winc->block,wc->blockSize, 1, wc->f);
	  }
	  ret = 0;
	} else {
	  ret = DB_NOTFOUND;
	}
    }
    break;
  case DB_LAST:
  case DB_PREV:
  case DB_CURRENT:
  case DB_SET:
  case DB_GET_BOTH:
  case DB_NEXT_DUP:
  default:
    printf("Window cursor access method not implemented");
    exit(1);
  }
  return ret;
}
int winc_close(_adl_winc_t winc)
{
  free(winc->block);
  free(winc);
  return 0;
}
int _adl_winCursor(_adl_win_t win, _adl_winc_t *cursorp, int expired_p)
{
  _adl_winc_t c=(_adl_winc_t)malloc(sizeof(*c));

  c->win = win;
  c->c_get = winc_get;
  c->c_close = winc_close;
  c->block = (char*)malloc(win->wc->blockSize);

  if (expired_p) {
    c->cursor_beg = &(c->win->expired);
    c->cursor_end = &(c->win->begin);
  } else {
    c->cursor_beg = &(c->win->begin);
    c->cursor_end = &(c->win->current);
  } 

  (*cursorp) = c;

  return 0;
}

