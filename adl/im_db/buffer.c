#include "im_db_buffer.h"
#include <stdlib.h>

buffer_t newBuffer(int size) 
{
  buffer_t p = (buffer_t)malloc(sizeof(struct _buf));

  p->item_size = size;
  p->page_size = size * 256;

  p->n_page = 1;
  p->current_page = 0;
  p->page_offset = 0; 
  p->pages = (char **)malloc(sizeof(char*));
  p->pages[0] = (char*)malloc(p->page_size);

  p->n_free_page=1;
  p->current_free_page=0;
  p->free_page_offset=0;
  p->free_pages=(int**)malloc(sizeof(int*));
  p->free_pages[0] = (int *)malloc(1024*sizeof(int));
  return p;
}
void deleteBuffer(buffer_t p)
{
  int i;
  for (i=0; i<=p->current_page; i++) {
    free(p->pages[i]);
  }
  free(p->pages);
  for (i=0; i<=p->current_free_page; i++) {
    free(p->free_pages[i]);
  }
  free(p->free_pages);
  free(p);
}
char *checkFreeBuffer(buffer_t p)
{
  if (p->free_page_offset>0) {
    p->free_page_offset--;
  } else if (p->current_free_page>0) {
    free(p->free_pages[p->current_free_page]);
    p->current_free_page--;
    p->free_page_offset=1023;
  } else 
    return 0;

  return (char*)(*(p->free_pages[p->current_free_page]+p->free_page_offset));
}
int allocateBuffer(buffer_t p, char **tuple)
{
  int rc = 0;

  if ((*tuple = checkFreeBuffer(p))) {
    goto exit;
  }

  if (p->page_offset == p->page_size) {
    /* we need a new page */
    if (p->current_page == p->n_page-1) {
      /* we need to expand page slots */
      p->n_page *= 2;

      if (!(p->pages = (char **)realloc(p->pages, p->n_page * sizeof(char *)))) {
	rc = 1;
	goto exit;
      }
    }
    p->current_page++;
    if (!(p->pages[p->current_page] = (char *)malloc(p->page_size))) {
      rc = 1;
      goto exit;
    }
    p->page_offset = 0;
  }
  *tuple = p->pages[p->current_page]+p->page_offset;
  p->page_offset += p->item_size;

 exit:
  return rc;
}

int freeBuffer(buffer_t p, char *tuple)
{
  int rc = 0;

  if (p->free_page_offset == 1024) {
    if (p->current_free_page == p->n_free_page-1) {
      p->n_free_page*=2;
      if (!(p->free_pages = (int **)realloc(p->free_pages, p->n_free_page * sizeof(int)))) {
	rc = 1;
	goto exit;
      }
    }
    p->current_free_page++;
    if (!(p->free_pages[p->current_free_page] = (int *)malloc(1024*sizeof(int)))) {
      rc = 1;
      goto exit;
    }
    p->free_page_offset = 0;
  }

  *(p->free_pages[p->current_free_page]+p->free_page_offset) = (int)tuple;
  p->free_page_offset++;

 exit:
  return rc;
}

#ifdef TEST
int main()
{
  buffer_t p = newBuffer(76);
  char *buf;
  int i,j, rc;
  char *result[4096];
  int c=0;

  //  for (j=0; j<1000; j++) {
  printf("allocate 3072 free 1024\n");
  p = newBuffer(76);
  for (i=0; i<3072; i++) {
    rc = allocateBuffer(p, &buf);

    if (i%3==0) {
      freeBuffer(p, buf);
    } else {
      result[c++]=buf;
    }

  }

  printf("cur page %d, cur off %d = %d items\n", p->current_page, p->page_offset,
	 p->current_page*256+p->page_offset/76);
  printf("cur page %d, cur off %d = %d items\n", p->current_free_page, p->free_page_offset,
	 p->current_free_page*1024+p->free_page_offset);

  printf("free 2048\n");
  while (c>0) {
    freeBuffer(p,result[--c]);
  }
  printf("allocate 2047\n");
  for (i=0;i<2047;i++) {
    allocateBuffer(p, &buf);
  }
  printf("cur page %d, cur off %d = %d items\n", p->current_page, p->page_offset,
	 p->current_page*256+p->page_offset/76);
  printf("cur page %d, cur off %d = %d items\n", p->current_free_page, p->free_page_offset,
	 p->current_free_page*1024+p->free_page_offset);


  deleteBuffer(p);
  //  }
}
#endif 
