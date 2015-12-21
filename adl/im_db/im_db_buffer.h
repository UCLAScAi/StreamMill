#ifndef BUFFER_H
#define BUFFER_H



typedef struct _buf
{
  int item_size;		/* in terms of bytes */
  int page_size;

  int n_page;			/* total # of pages */
  int current_page;		/* current page */
  int page_offset;		/* next empty slot on the current page */

  int n_free_page;
  int current_free_page;
  int free_page_offset;
  
  char **pages;
  int **free_pages;
}*buffer_t;

//typedef struct _buf *buffer_t;

buffer_t newBuffer(int size);
void deleteBuffer(buffer_t p);
int allocateBuffer(buffer_t p, char **tuple);

#endif
