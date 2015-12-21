#ifndef HEAP_H
#define HEAP_H

#include <nt_obj.h>

typedef struct heap_s
{
  char name[32];
  char *record;
  int size;			/* total size of heap */
  int rec_size;			/* length of record */
  int fhandle;
  void (*displayRec)();
} heap_t;

heap_t *createHeap(char *name, int rec_size, void (*f)());
heap_t *openHeap(char *name, int rec_size);
void closeHeap(heap_t *heap);
int appendRecHeap(heap_t *heap, char *record);
char* searchHeap(heap_t *heap, char *key, int (*fcmp)(const char*,const char*), int *pos);
int removeRecHeap(heap_t *heap);
void displayHeap(nt_obj_t *stream, heap_t *heap);
int getTupleHeap(heap_t *heap, int pos);
#endif



