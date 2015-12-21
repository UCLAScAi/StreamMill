/*
 * Test if r1 overlaps r2
 *
 * @return 1 overlaps
 *	   0 otherwise
 */
int
overlap(Rect *r1, Rect *r2);
/*
 * Test if r1 is contained in r2
 *
 * @return -1 not contained in
 *	    0 equal to
 *          1 properly contained in
 */
int
containedIn(Rect *r1, Rect *r2);
int
cmp_lowside(Rect *r1, Rect *r2);
int
cmp_highside(Rect *r1, Rect *r2);
double
area(Rect *r);
Rect
compose(Rect *r1, Rect *r2);
/**
 * Convert 4*4=16 bytes memory into a rectangle
 */
void
mem2rect(Rect *dst, char *src);
/**
 * Convert a rectangle into 4*4=16 bytes memory
 */
void
rect2mem(char *dst, Rect *src);
/* Print Rect
 */
void rectPrint(Rect *mbr, int crlf = 0);
