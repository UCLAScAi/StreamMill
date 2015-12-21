#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <dbug/dbug.h>
#include "heap.h"
#include <sql/util.h>
#include <sql/io.h>
#include <sql/err.h>

heap_t *createHeap(char *name, int rec_size, void (*f)())
{
  int handle = open(name, O_RDWR | O_CREAT, 0644);
  
  if (handle) {
    heap_t *heap = (heap_t*)ntMalloc(sizeof (heap_t));
    strcpy(heap->name, name);
    heap->fhandle=handle;
    heap->rec_size=rec_size;
    heap->size=0;
    heap->record = (char*)ntMalloc(heap->rec_size);
    heap->displayRec = f;
    return heap;
  }
  return (heap_t*)0;
}

heap_t *openHeap(char *name, int rec_size)
{
  heap_t *new_obj = (heap_t*)0;
  int handle;

  if ((handle=open(name, O_RDWR))>0) {
    new_obj = (heap_t*)ntMalloc(sizeof(heap_t));
    new_obj->rec_size = rec_size;
    new_obj->fhandle=handle;
    new_obj->size = lseek(new_obj->fhandle, 0, SEEK_END);
    new_obj->record = (char*)ntMalloc(new_obj->rec_size);
  }
  return new_obj;
}
void closeHeap(heap_t *heap)
{
  close(heap->fhandle);
  free(heap->record);
  free(heap);
}
int appendRecHeap(heap_t *heap, char *record)
{
  if (heap && heap->fhandle>=0) {
    bzero(heap->record, heap->rec_size);
    memcpy(heap->record, record, heap->rec_size);
    if (lseek(heap->fhandle, heap->size, SEEK_SET)>=0 &&
	write(heap->fhandle, heap->record, heap->rec_size)>=0) {
      heap->size+=heap->rec_size;
      return 1;
    }
  }
  return 0;
}
char* 
searchHeap(heap_t *heap, char *key, 
	   int (*fcmp)(const char*, const char*), int *pos)
{
  if (heap && heap->fhandle && 
      lseek(heap->fhandle, *pos, SEEK_SET)>=0) {
    while (*pos < heap->size*heap->rec_size && 
	   read(heap->fhandle, heap->record, heap->rec_size)>=0) {
      *pos+=heap->rec_size;
      if (fcmp(heap->record, key)==0)
	return heap->record;
    }
  }
  return (char*)0;
}
int removeRecHeap(heap_t *h)
{
  int pos;
  if (h && h->fhandle) {
    if (h->size==0)
      return 0;
    if (h->size==h->rec_size) {
      h->size=0;
      return 1;
    }
    if ( (pos = lseek(h->fhandle, 0, SEEK_CUR)) >=0 && /* current pos */
	 (lseek(h->fhandle, h->size-h->rec_size, SEEK_CUR)>=0) && 
	 read(h->fhandle, h->record, h->rec_size)>0 &&
	 lseek(h->fhandle, pos - h->rec_size, SEEK_SET)>=0 &&
	 write(h->fhandle, h->record, h->rec_size)>0) {
      h->size-=h->rec_size;
      ftruncate(h->fhandle, h->size);
      return 1;
    }
  }

  return 0;
}
int getTupleHeap(heap_t *heap, int pos)
{
  int offset = pos * heap->rec_size;

  DBUG_PRINT("info",("getTupleHeap: handle %d, offset %d, size %d", 
		     heap->fhandle, offset, heap->size));

  if (offset>=heap->size) 
    return 0;

  if (lseek(heap->fhandle, offset, SEEK_SET)<0 ||
      read(heap->fhandle, heap->record, heap->rec_size)<=0) {
    displayErr(ERR_NTSQL_INTERNAL, strerror(errno));
    return 0;
  }
  return 1;
}
void displayHeap(nt_obj_t *stream, heap_t *heap)
{
  if (heap && heap->fhandle) {
    int pos=0;
    while (pos<heap->size) {
      read(heap->fhandle, heap->record, heap->rec_size);
      pos+=heap->rec_size;
      heap->displayRec(stream, heap->record);
    }
  }
}





