#define dbg(x)

#ifndef DEF_RTREE
#define DEF_RTREE

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "db.h"
#include <stack>

using namespace std;

#define EQUALITY 0
#define CONTAINMENT 1

/* Flags.
   Refer BerkeleyDB/include/db.h to avoid conflict
*/
#define DB_NEXT_RANGE 100

//#define R_MAXINT 0x7fffffff
//#define R_MININT 0x80000000
// In sql directory, we use INT_MAX and INT_MIN for R_MAXINT and R_MININT, respectively
#define R_MAXINT INT_MAX
#define R_MININT INT_MIN

//#define MAXCHAR 0x7f
//#define MINCHAR ((char)0x80)

#ifndef DBL_MAX
#define DBL_MAX	1e+37
#endif

/* flags for open rtree: 0--open existing 1-creat a new one */
#define OLD_RTREE 0
#define NEW_RTREE 1

/* Block size for index file */
#define BLOCKSZ		2048
/* Page size for data file, used by BDB's set_pagesize() */
#define PAGESZ          2048

/* KEY is a rectangle in R-tree, 4*4 bytes */
#define KEYSZ 16
/* Item is KEY + OFFSET, 5*4 bytes, plus "deleted" field */
#define ENTRYSZ 21

/* Overhead: 1 char for node type, 1 unsigned for number of items,
   2 potential pageno on root node and leaf nodes for node-links 
*/
#define OVERHEAD	(1+sizeof(unsigned)+2*sizeof(pageno_t))


#define	MAX_ENTRIES	((BLOCKSZ-OVERHEAD)/ENTRYSZ)
/* CAVEATS: In any case, MAX_ENTRIES+1 >= 2*MIN_ENTRIES, otherwise
   a split has no chance to be correct */

#ifndef MIN
#define MIN(x,y)	((x)<(y)? (x):(y))
#endif
#ifndef MAX
#define MAX(x,y)	((x)>(y)? (x):(y))
#endif
#define ABS(x)		((x)<0? -(x): (x))


typedef db_recno_t tupleno_t;

/* We use page-oriented system in index, the actual file offset is
   "pageno * BLOCKSZ" */
typedef unsigned long pageno_t;
typedef pageno_t blkptr;

/* block is a syn of page, we will use "block" in the future, Richard 2002/7 */
typedef pageno_t blkno_t;	


/*
 * R-Tree node operations
 */
#define SET_NODETYPE(node,type)	((node)[0] = (type))
#define SET_COUNTER(node, num)	(memcpy((node)+1, (num), sizeof(unsigned)))
#define SET_NEXTLPTR(node, ptr)	(memcpy((node)+1+sizeof(unsigned), (ptr), sizeof(pageno_t)))
#define SET_PREVLPTR(node, ptr)	(memcpy((node)+1+sizeof(unsigned)+sizeof(pageno_t), (ptr), sizeof(pageno_t)))
#define SET_DEL(node, type) ((node)[ENTRYSZ-1] = (type))
#define GET_NODETYPE(type, node)	((type) = (node)[0])
#define GET_COUNTER(num, node)	(memcpy((num), (node)+1, sizeof(unsigned)))
#define GET_NEXTLPTR(ptr, node)	(memcpy((ptr), (node)+1+sizeof(unsigned), sizeof(pageno_t)))
#define GET_PREVLPTR(ptr, node)	(memcpy((ptr), (node)+1+sizeof(unsigned)+sizeof(pageno_t), sizeof(pageno_t)))
#define GET_FIRSTITEM(cp, node)	((cp) = (node)+OVERHEAD)
#define GET_ITEM(cp, node, i) ((cp) = (node) + OVERHEAD + (i)*ENTRYSZ)
// get current item
#define GET_CURITEM(cp, rtc) ((cp) = (rtc)->curnode+OVERHEAD + (rtc)->curitem*ENTRYSZ)
#define GET_DEL(type, node) ((type) = (node)[ENTRYSZ-1])

extern int debug;	/* normally define this in the main driver */
//extern char buffer[];	/* defined in rtree.c */
extern char *qbuf;	/* defined in rtree.c */
extern unsigned qbufsz;	/* defined in rtree.c */

/* This is in-memory data structure for a rectangle.
   Only the coordinates for upperleft corner and lowerright corner are
   recorded.
   In storage, consecutive 4*4=20 bytes record a rectangle */
typedef struct rectangle
{
    long x_ul,y_ul,x_lr,y_lr;
} Rect;

/* This is in-memory data structure for a item in R-tree node.
   In storage, consecutive 5*4=20 bytes record a rectangle */
typedef struct index_entry
{
    Rect entry;
    pageno_t  pageno;
} IndexEntry;

/* This is in-memory data structure for an R-tree.
   In storage, two file names handle the index and the record */


typedef struct rtree_cursor RTreeCursor;
typedef struct rtree RTree;

typedef struct cbuffertype{
  pageno_t blkptr;// block ptr in index
  unsigned ent;// entry no.in index
}cBufferType;

typedef struct pathtype{
  char buf[BLOCKSZ];
  int iItem;                   //last item no. that found
  struct pathtype *next;
  struct pathtype *prev;
}PathType;

struct rtree_cursor
{
  
  RTree *rtree;            /* Related rtree */
  char curnode[BLOCKSZ];
  pageno_t curBlkPtr;	   /* pointers to index file (index.db) */
  unsigned curitem;
  tupleno_t curtpl;        /*tuple number */

  stack<cBufferType> *dfsStack; /* stack for DFS, NOTE: MUST INIT in rtree_cursor */
  int dfsType;              // DFS search type, EQUALITY OR CONTAINMENT
  DBC *dbc;                 // Underlying BerkeleyDB's Cursor
  
  PathType *pathHead;      //head of path list, used in cursor range get (DB_SET_RANGE)
  PathType *pathTail;      //tail of path list used in cursor range get 
  /* methods */
  int (*c_close)(RTreeCursor *);
  int (*c_get)(RTreeCursor *, DBT *, DBT *, int);
  int (*c_put)(RTreeCursor *, DBT *, DBT *, int);
  int (*c_del)(RTreeCursor *, int);
};

struct rtree
/* NOTE: Don't change variables order !!! */
{
  /* index: free block head and total blocks in use */
  pageno_t fb_ind, tb_ind;
  long lptr;		/* leaf pointer, lptr/BLOCKSZ = leaf block */
  unsigned m, M;	/* m: minimum; M: maximum; m <= M/2 */

  /* size of super block in index file */
#define indexSbSz (sizeof(pageno_t)*2 + sizeof(long) + sizeof(unsigned)*2)


  /* records: free tuple head and total tuples in use */

  
  /*Richard 2002/4, save tuple size coz it's fixed,instead
    of variable in a rtree.  the value will be assigned during first rtree_put.  
    use the initial value 0 (initialized in create_rtree) to decide whether it's the first rtree_put*/
  u_int32_t tplsz; 

#define recordSbSz (sizeof(tupleno_t)*2 + sizeof(u_int32_t) *2) /* the size of super block in data file */
 

  void *current_tuple;/*Richard 2002/5, used for get() and c_get(), to avoid dynamically allocating memory, assuming there is no multiple accesses to database*/

  FILE *index;

  /* store the filename for rtree removal */
  char index_fname[255];
  char record_fname[255];

  /* handle for Berkeley DB */
  DB *db;

  int (*cursor)(RTree *, RTreeCursor **, int);
  int (*open)(RTree **, char *, char *, int);
  int (*close)(RTree *, int);
  int (*get)(RTree *, DBT *, DBT *, int);
  int (*put)(RTree *, DBT *, DBT *, int);
  int (*del)(RTree *, Rect *, int);
  int (*remove)(RTree *);	// empty the rtree
  
  void (*print)(RTree *, int);


};



/*
 * function prototypes
 */
int init_rtree(RTree *rtree);
RTree *open_rtree(char *indexFileName, char *recordFileName, int mode);
int close_rtree(RTree *rtree);
int insert_rtree(RTree *rtree, Rect *mbr, DBT *);
int insert_rtree_aux(RTree *rtree, Rect *mbr, tupleno_t tplptr);
int print_node(RTree *rtree, long node_offset);
int print_one_node(RTree *rtree, long node_offset);
int print_superblk(RTree *rtree);
int split_node(RTree *rtree, pageno_t curblk, Rect *mbr, long tupleptr,
	       char nodetype, long prevlptr, long nextlptr);
int adjust_tree_with_split(RTree *rtree, pageno_t curblk, stack<pageno_t> &anc);
int adjust_tree_without_split(RTree *rtree, pageno_t curblk, Rect *mbr, stack<pageno_t> &anc);
pageno_t get_newblock(RTree *rtree);
pageno_t choose_leaf(RTree *rtree, Rect *mbr, stack<pageno_t> &anc);
/**
 * Search all tuples whose rectangle equal to MBR.
 *
 * @param mbr: return the block number of the first entry of this MBR
 * @param curblk: current block in depth-first-search,
 *		  pass 1 (which is the root) in the first call.
 * @param head, tail: the head and tail of result buffer.  
 head should be already allocated memory before the call.
 tail->blk is set to -1.
 * @return -1 if something wrong, 0 upon success;
 *         tail is set to the tail of the result buffer
 *         head -> blk = -1 if no tuple found
 */
int
search_rtree(cBufferType **head, cBufferType **tail, RTree *rtree, Rect *mbr, pageno_t curblk);
/**
 * Arbitrarily get one tuple whose rectangle equal to MBR.
 *
 * @param mbr: return the block number of the first entry of this MBR
 * @param curblk: current block in depth-first-search,
 *		  pass 1 (which is the root) in the first call.
 * @return *blk = -1 if something wrong,
 *                otherwise the parent block num of `blkno'.
 *	          0 is impossible, so it means not found.
 *         *ent = entry # in the block
 *         *tpl = tuple #
 */
void
rtree_get_one(pageno_t *blk, unsigned *ent, tupleno_t *tpl,
	      RTree *rtree, Rect *mbr, pageno_t curblk);
/**
   Delete all tuples bounded by mbr.
*/
int
delete_rtree(RTree *rtree, Rect *mbr);
/* rtree_dfs: (Continue to) DFS the rtree for the first match tuple.  Intermediate nodes are recorded in the stack rtc->dfsStack, in order to do the next match
   param:
     rtc: underlying cursor;
     mbr: mbr to query
     type: EQUALITY : mbr is equal to some tuple?
           CONTAINMENT: some tuple is contained in  mbr?
   return value:
     0 : successful, rtc->curtpl is set to the tuple no of match
     -1: fail
     DB_NOTFOUND: DFS is completed. NOT FOUND any more match.
*/
int rtree_dfs(RTree *rtree, 
	      stack <cBufferType> *dfsStack, 
	      Rect *mbr, 
	      tupleno_t &curtpl, 
	      int type, 
	      char* temp);


/* from rectangle.c */
double area(Rect *r);
Rect compose(Rect *r1, Rect *r2);
int containedIn(Rect *r1, Rect *r2);
void mem2rect(Rect *dst, char *src);
void rect2mem(char *dst, Rect *src);

/* from block.c */
int update_block_type(RTree *rtree, pageno_t blkno, char node_type);
int update_block_entry(RTree *rtree, pageno_t blkno, unsigned num,
		       Rect *mbr, long ptr);
int free_block(RTree *rtree, pageno_t blkno, int flag);
pageno_t get_parent_block(RTree *rtree, pageno_t blkno, pageno_t curblk);

/* from interface.c */
int rtree_create(RTree **rtree, char *indexFileName, char *recordFileName, int flags);
int rtree_open(RTree **rtree, char *indexFileName, char *recordFileName,
	       int flags);
int rtree_close(RTree *rtree, int flags);
int rtree_get(RTree *rtree,DBT*, DBT*,int flags);
int rtree_put(RTree *rtree,DBT*, DBT*,int flags);
int rtree_del(RTree *rtree, Rect *mbr, int flags);
void rtree_print(RTree *rtree, int flags);
int rtree_cursor(RTree *rtree, RTreeCursor **rtcp, int flags);
int rtree_cursor_close(RTreeCursor *rtc);
int rtree_cursor_get(RTreeCursor *rtc, DBT*, DBT *, int flags);
int rtree_cursor_put(RTreeCursor *rtc, DBT*, DBT *, int flags);
int rtree_cursor_next_item(RTreeCursor *);
int rtree_cursor_del(RTreeCursor *rtc, int flags);

void myerr(char *s); // error exit

#endif
