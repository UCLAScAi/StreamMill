#include "rtree.h"

/*
 * Test if r1 overlaps r2
 *
 * @return 1 overlaps
 *	   0 otherwise
 */
int
overlap(Rect *r1, Rect *r2)
{
  if (r1->x_lr < r2->x_ul ||
      r2->x_lr < r1->x_ul ||
      r1->y_ul < r2->y_lr ||
      r2->y_ul < r1->y_lr)
    return 0;
  return 1;
  
}

/*
 * Test if r1 is contained in r2
 *
 * @return -1 not contained in
 *	    0 equal to
 *          1 properly contained in
 */
int
containedIn(Rect *r1, Rect *r2)
{
	if (r1 == r2) return 0;
	if (!r1) return 1;//NULL is contained by any rectangle
    if(r1->x_ul == r2->x_ul &&
       r1->y_ul == r2->y_ul &&
       r1->x_lr == r2->x_lr &&
       r1->y_lr == r2->y_lr)
	return 0;
    if(r1->x_ul >= r2->x_ul &&
       r1->y_ul <= r2->y_ul &&
       r1->x_lr <= r2->x_lr &&
       r1->y_lr >= r2->y_lr)
	return 1;
    return -1;
}

int
cmp_lowside(Rect *r1, Rect *r2)
{
    if(r1->y_lr == r2->y_lr)
	return 0;
    if(r1->y_lr > r2->y_lr)
	return 1;
    return -1;
}
int
cmp_highside(Rect *r1, Rect *r2)
{
    if(r1->y_ul == r2->y_ul)
	return 0;
    if(r1->y_ul > r2->y_ul)
	return 1;
    return -1;
}

double
area(Rect *r)
{
    return (double)(r->y_ul - r->y_lr) * (double)(r->x_lr - r->x_ul);
}

Rect
compose(Rect *r1, Rect *r2)
{
    Rect r;

    r.x_ul = MIN(r1->x_ul, r2->x_ul);
    r.y_ul = MAX(r1->y_ul, r2->y_ul);
    r.x_lr = MAX(r1->x_lr, r2->x_lr);
    r.y_lr = MIN(r1->y_lr, r2->y_lr);
    return r;
}

/**
 * Convert 4*4=16 bytes memory into a rectangle
 */
void
mem2rect(Rect *dst, char *src)
{
    memcpy(&dst->x_ul, src, sizeof(long));
    memcpy(&dst->y_ul, src+sizeof(long), sizeof(long));
    memcpy(&dst->x_lr, src+2*sizeof(long), sizeof(long));
    memcpy(&dst->y_lr, src+3*sizeof(long), sizeof(long));
}
/**
 * Convert a rectangle into 4*4=16 bytes memory
 */
void
rect2mem(char *dst, Rect *src)
{
    memcpy(dst, &src->x_ul, sizeof(long));
    memcpy(dst+sizeof(long), &src->y_ul, sizeof(long));
    memcpy(dst+2*sizeof(long), &src->x_lr, sizeof(long));
    memcpy(dst+3*sizeof(long), &src->y_lr, sizeof(long));
}

/* Print Rect
   param:
     mbr: the mbr to print;
     crlf: if crlf != 0, print a line-feed
 */
void rectPrint(Rect *mbr, int crlf = 0){
  dbg(printf(" [(%d, %d), (%d, %d)] ", mbr->x_ul, mbr->y_ul, mbr->x_lr, mbr->y_lr));
  if (crlf) { dbg(printf("\n"));}
}
