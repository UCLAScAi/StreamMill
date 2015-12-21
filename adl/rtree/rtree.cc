#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include "rtree.h"
#include "tupledb.h"
#include "block.h"
#include "rectangle.h"

#include <stack>
using namespace std;
int debug = 1;

/* general-purposed buffer */
char buffer[BLOCKSZ];			 /*one for read/write index */

/* Used in SplitNode and AdjustTree */
static char itembuf[(MAX_ENTRIES+1)*ENTRYSZ];	/* M+1 entries */
static char split0[BLOCKSZ];
static char split1[BLOCKSZ];
static pageno_t blkno0, blkno1;
static Rect coverr0, coverr1;

/* Used in CondenseTree */
char *qbuf;
unsigned qbufsz;

/*
 * R-Tree storage is organized in a block-oriented manner.
 * I tried to keep the storage structure as simple as possible.
 *
 * As discussed with Prof. Zaniolo, we consider clustering in term of
 * fseek().  An R-Tree node may be consist of one or more physical
 * disk block(s) on the hard disk.  As they are consecutive from the
 * viewpoint of fseek(), we consider that they are clustered together.
 *
 * The first node of an R-Tree storage file is always the root node.
 * The second node of an R-Tree storage file is always the first node
 * storing the records.
 */

/*
  empty the rtree:
  1.close rtree;
  2.delete index and record file
  3.create a new rtree
*/
int rtree_remove(RTree* rtree){
 
  //close rtree;
  close_rtree(rtree);
  //  delete index and record file
  remove(rtree->index_fname);
  remove(rtree->record_fname);
  //  create a new rtree
  rtree = open_rtree(rtree->index_fname, rtree->record_fname, NEW_RTREE);   
}

/* Initialize an empty rtree
   Make sure the rtree is empty before calling this function.
*/

int init_rtree(RTree *rtree){
  /* Initialize a superblock and write it to indexdb */
  rtree->fb_ind = 1;
  rtree->tb_ind = 1;
  rtree->lptr = BLOCKSZ;
  rtree->m = 2;
  rtree->M = MAX_ENTRIES;
  rtree->current_tuple = NULL;
  rtree->tplsz = 0;	/* tplsz and current_tuple's initialization
			   is done when first inserting */
}

/**
 * Open an R-Tree storage structure
 *
 * @param indexFileName: the storage file name for index.
 * @param recordFileName: the storage file name for tuples.
 * @param mode: 0 -- open existing storage files.
 *              1 -- create the storage files from the scratch line.
 * @return NULL for errors, otherwise a valid RTree pointer.
 */
RTree *
open_rtree(char *indexFileName, char *recordFileName, int mode)
{
  RTree *rtree;
  if((rtree = (RTree *)malloc(sizeof(RTree))) == NULL)
    return NULL;

  /* set function pointers */
  rtree->cursor = rtree_cursor;
  rtree->open = rtree_open;
  rtree->close = rtree_close;
  rtree->get = rtree_get;
  rtree->put = rtree_put;
  rtree->del = rtree_del;
  rtree->remove = rtree_remove;  
  rtree->print = rtree_print;

  strcpy(rtree->index_fname, indexFileName);
  strcpy(rtree->record_fname, recordFileName);
  /* handle the disk storage */
  if(mode == OLD_RTREE)
    {
      if (!(rtree->index = fopen(indexFileName, "rb+"))){
	if (errno == 2 ) // file not exists
	  mode = NEW_RTREE;
	else{
	  perror("INDEX file");
	  myerr("INDEX file");
	}
      };
    };
  if(mode == NEW_RTREE)
    {
      rtree->index = fopen(indexFileName, "wb+");
    }

  if(rtree->index == NULL)
    {
      fprintf(stderr, "Error in opening the RTREE index file.\n");
      return NULL;
    }

  if(mode == OLD_RTREE)/* existing rtree */
    {
      /* Read the superblock in indexdb */
      if(fread(buffer, 1, BLOCKSZ, rtree->index) != BLOCKSZ)
	{
	  fprintf(stderr, "Error in reading the index superblock.\n");
	  return NULL;
	}

      memcpy(&rtree->fb_ind, buffer, sizeof(pageno_t));
      memcpy(&rtree->tb_ind, buffer+sizeof(pageno_t), sizeof(pageno_t));
      memcpy(&rtree->lptr, buffer+2*sizeof(pageno_t), sizeof(long));
      memcpy(&rtree->m,
	     buffer+2*sizeof(pageno_t)+sizeof(long),
	     sizeof(unsigned));
      memcpy(&rtree->M,
	     buffer+2*sizeof(pageno_t)+sizeof(long)+sizeof(unsigned),
	     sizeof(unsigned));
      memcpy(&rtree->tplsz, 
	     buffer+2*sizeof(pageno_t)+sizeof(long)+2*sizeof(unsigned),
	     sizeof(u_int32_t));
      
      /*initialize a temp space for storing data to retrieve*/
      rtree->current_tuple = (void*)malloc(rtree->tplsz);
      dbg(printf("open:rtree tuple size set to %d \n", rtree->tplsz));
 
    }
  else if(mode == NEW_RTREE) /* new rtree */
    {
      if (init_rtree(rtree) == -1) {
	myerr("RTREE_OPEN:INIT RTREE!\n");
	return NULL;
      }

    }
  if (tuple_init(rtree, mode) != 0){
    myerr("insert_rtree:TUPLE_INIT!\n");
    return NULL;
  }
  return rtree;
}

/** Close the rtree */
int
close_rtree(RTree *rtree)
{
  /* update fb_ind and tb_ind before closing */
  memcpy(buffer, 
	 &rtree->fb_ind, sizeof(pageno_t));
  memcpy(buffer+sizeof(pageno_t), 
	 &rtree->tb_ind, sizeof(pageno_t));
  memcpy(buffer+2*sizeof(pageno_t), 
	 &rtree->lptr, sizeof(long));
  memcpy(buffer+2*sizeof(pageno_t)+sizeof(long),
	 &rtree->m, sizeof(unsigned));
  memcpy(buffer+2*sizeof(pageno_t)+sizeof(long)+sizeof(unsigned),
	 &rtree->M, sizeof(unsigned));
  memcpy(buffer+2*sizeof(unsigned)+sizeof(long)+2*sizeof(unsigned),
	 &rtree->tplsz, sizeof(u_int32_t));
  write_blk(rtree->index, 0, buffer);
  /* close the FILE pointers */
  //Richard 2002/5
  if (rtree->current_tuple) free(rtree->current_tuple);
  //Richard end

  fclose(rtree->index);
  tuple_close(rtree);
  free(rtree);
  return 0;
}




/**
 * Insert a tuple into the R-Tree
 *
 * @param rtree: the R-Tree structure.
 * @param tuple: the tuple.  Everything inside must be ready before calling.
 */
int
insert_rtree(RTree *rtree, Rect *mbr, DBT *dbt)
{

  tupleno_t tplno;
  void *tuple = dbt->data;
  if (rtree->tplsz == 0) //initialize rtree->tplsz and rtree->current_tuple
    {

      rtree->tplsz = dbt->size;
      rtree->current_tuple = (void*)malloc(rtree->tplsz);
    }
  dbg(printf("insert_rtree: [(%d,%d),(%d, %d),%d]\n", 
	     mbr->x_ul, 
	     mbr->y_ul, 
	     mbr->x_lr,
	     mbr->y_lr,
	     *((int*)tuple)));
  if (tuple_put(rtree, tuple, &tplno) != 0 ){
    myerr("insert_rtree: tuple_put!");
  }
  if(insert_rtree_aux(rtree, mbr, tplno) == -1){
    myerr("insert rtree!");
    return -1;
  }
  return 0;
}
int
insert_rtree_aux(RTree *rtree, Rect *mbr, tupleno_t tplno)
{
  unsigned temp;
  char node_type;
  unsigned num_items, i;
  char *firstItem, *item;
  pageno_t dest_blk;
  long ptr;
  stack<pageno_t> anc; //ancestors of the leaf node

  dbg(printf("insert_rtree_aux: Inserting [(%ld,%ld)(%ld,%ld), T%ld]\n",
	     mbr->x_ul, mbr->y_ul, mbr->x_lr, mbr->y_lr, tplno));

    if(rtree->fb_ind == 1 && rtree->tb_ind == 1)
      {
	/*
	 * the first insertion: the R-tree index is not there yet.
	 * create one.
	 */
	SET_NODETYPE(buffer, 'l');
	num_items = 0;
	ptr = 0;
	SET_NEXTLPTR(buffer, &ptr);
	SET_PREVLPTR(buffer, &ptr);
	GET_FIRSTITEM(firstItem, buffer);
	SET_DEL(firstItem,0);
	dest_blk = 1;

	/* adjust free block and total block counter */
	rtree->fb_ind = rtree->tb_ind = 2;
      }
    else
      {
	dest_blk = choose_leaf(rtree, mbr, anc);
	/* read the node */
	read_blk(rtree->index, dest_blk, buffer);
	GET_COUNTER(&num_items, buffer);
	GET_FIRSTITEM(firstItem, buffer);
      }

  /* now the destination node is in the buffer */

  if(num_items == MAX_ENTRIES)
    {
      /*
       * Current leaf node is full, split.
       */
      long prevlptr, nextlptr;

      if(dest_blk == 1)
	{
	  /* splitting root node */
	  nextlptr = 0;
	  prevlptr = 0;
	}
      else
	{
	  /* otherwise, splitting a non-root leaf node */
	  GET_NEXTLPTR(&nextlptr, buffer);
	  GET_PREVLPTR(&prevlptr, buffer);
	}

      split_node(rtree, dest_blk, mbr, tplno, 'l', prevlptr, nextlptr);
      if(dest_blk*BLOCKSZ == rtree->lptr)
	{
	  /* if the head of leaf chain is being split,
	     then the head of leaf chain is the first split node. */
	  rtree->lptr = blkno0*BLOCKSZ;
	}
      /* adjust leaf chain */
      if(prevlptr != 0)
	{
	  long blk0ptr = blkno0*BLOCKSZ;
	  read_blk_by_ptr(rtree->index,prevlptr, buffer);
	  SET_NEXTLPTR(buffer, &blk0ptr);
	  write_blk_by_ptr(rtree->index, prevlptr, buffer);
	  GET_COUNTER( &temp, buffer);
 	}
      if(nextlptr != 0)
	{
	  long blk1ptr = blkno1*BLOCKSZ;
	  read_blk_by_ptr(rtree->index, nextlptr, buffer);
	  SET_PREVLPTR(buffer, &blk1ptr);
	  write_blk_by_ptr(rtree->index, nextlptr, buffer);
	  GET_COUNTER( &temp, buffer);
 	}

      /* now the splitted results are in split0 and split1,
	 the covering rectangles are in coverr0 and coverr1,
	 adjust tree */
      adjust_tree_with_split(rtree, dest_blk, anc);
    }
  else
    {
      /*
       * No split: do the insertions to R-Tree index and tupledb
       */
      item = firstItem + (num_items++)*ENTRYSZ;
      rect2mem(item, mbr);
      memcpy(item+KEYSZ, &tplno, sizeof(long));

      /* write the index */
      SET_COUNTER(buffer, &num_items);
      write_blk(rtree->index, dest_blk, buffer);

      /* adjust the parent index nodes */
      adjust_tree_without_split(rtree, dest_blk, mbr, anc);
    }

  return 0;
}

/**
 * Arbitrarily get one tuple whose rectangle equal to MBR.
 *
 * @param mbr: return the block number of the first entry of this MBR
 * @param curblk: current block in depth-first-search,
 *		  pass 1 (which is the root) in the first call.
 * @return *blk = UINT_MAX if something wrong,
 *                otherwise the parent block num of `blkno'.
 *	          0 is impossible, so it means not found.
 *         *ent = entry # in the block
 *         *tpl = tuple #
 */
void
rtree_get_one(pageno_t *blk, unsigned *ent, tupleno_t *tpl,
	      RTree *rtree, Rect *mbr, pageno_t curblk)
{
  char nodetype;
  unsigned num_items;
  char *item;
  char temp[BLOCKSZ];
  int i;
  dbg(printf("rtree_get_one:\n"));
  dbg(printf("searching B%d fb=%d\n", curblk, rtree->fb_ind));
  *blk = UINT_MAX;
  *ent = UINT_MAX;
  *tpl = UINT_MAX;
  if (rtree->fb_ind == 1){ //RTREE is empty
    dbg(printf("RTREE is empty!\n"));
    *blk = 0;
    return ;
  }
  /* read the current block at first */
  read_blk(rtree->index, curblk, temp);
  GET_NODETYPE(nodetype, temp);
  GET_FIRSTITEM(item, temp);
  GET_COUNTER(&num_items, temp);

  /* now scan each child block */
  for(i=0; i<num_items; i++)
    {
      Rect r;
      pageno_t b;

      mem2rect(&r, item);
      /* get the logical tuple number */
      memcpy(&b, item+KEYSZ, sizeof(long));
      if(nodetype == 'l')
	{

	  dbg(printf("examing Rec.%d\n", b));

	  /* leaf node */
	  if(containedIn(mbr, &r) == 0)   /* equal, got it! */
	    {
	      // check if deleted
	      char deleted;
	      GET_DEL(deleted, item);
	      if (!deleted){
		dbg(printf("got it! cb = %d  b=%d item = %d\n", curblk, b, i));
		*blk = curblk;
		*ent = i;
		*tpl = b;
		dbg(printf("End searching B%d\n", curblk));
		return;
	      }
	    }
	}
      else	/* non-leaf node */
		{
		  dbg(printf("examing B%d\n", b/BLOCKSZ));
		  if(containedIn(mbr, &r) >= 0)
		    {
		      b /= BLOCKSZ;
			if(b < 1 || b >= rtree->tb_ind)
			  {
			    rtree->print(rtree,0);
			    printf("Invalid block number %d in B%d\n", b, curblk);
			    *blk = UINT_MAX;
			    return ;
			  }
			rtree_get_one(&b, ent, tpl, rtree, mbr, b);
			if(b != 0)
			  {
			    *blk = b;
			    dbg(printf("End searching B%d\n", curblk));
			    return;
			  }
		    }
		}
      item += ENTRYSZ;
    }
  *blk = 0;
  dbg(printf("End searching B%d\n", curblk));
  return;
}

/**
 * Jiejun Kong's extension on R-Tree.
 * Sequential scan is supported by linking all leaf nodes together.
 */
void
sequential_search_rtree(pageno_t *blk, unsigned *ent, tupleno_t *tpl,
			RTree *rtree, Rect *mbr)
{
  long blkptr=rtree->lptr;

  while(blkptr != 0)
    {
      char *item;
      unsigned num_items;
      int i;
      read_blk_by_ptr(rtree->index, blkptr, buffer);
      GET_FIRSTITEM(item, buffer);
      GET_COUNTER(&num_items, buffer);

      for(i=0; i<num_items; i++)
	{
	  Rect r;

	  mem2rect(&r, item);
	  if(containedIn(mbr, &r) == 0)
	    {
	      *blk = blkptr/BLOCKSZ;
	      *ent = i;
	      memcpy(tpl, item+KEYSZ, sizeof(tupleno_t));
	      return;
	    }
	  item += ENTRYSZ;
	}

      GET_NEXTLPTR(&blkptr, buffer);
    }
}

/**
 * Guttman's routine CondenseTree()
 *
 * @return -1: something wrong; 0: ok.
 */
int
condense_tree(RTree *rtree, pageno_t blkno)
{
  long blkptr = blkno * BLOCKSZ;
  char temp[BLOCKSZ];
  pageno_t pblk;
  char *item;
  unsigned num_items;
  int i;

  /* know the parent block */
  pblk = get_parent_block(rtree, blkno, 1);
  if(pblk < 1 || pblk > rtree->tb_ind)
    {
      fprintf(stderr, "Internal error! Impossible parent block B%d\n", pblk);
      exit(-1);
    }
  /* read the parent block */
  read_blk(rtree->index, pblk, temp);
  /* delete the entry pointing to `blkno' */
  GET_FIRSTITEM(item, temp);
  GET_COUNTER(&num_items, temp);
  for(i=0; i<num_items; i++)
    {
      long ptr;

      memcpy(&ptr, item+KEYSZ, sizeof(long));
      if(blkptr == ptr)
	{
	  /* delete this entry */
	  int j;

	  /* move entries [i+1..num_items-1] to [i..num_items-2] */
	  for(j=0; j<num_items-i-1; j++)
	    memcpy(item+j*ENTRYSZ, item+(j+1)*ENTRYSZ, ENTRYSZ);
	  /* write the block */
	  num_items--;
	  SET_COUNTER(temp, &num_items);
	  write_blk(rtree->index, pblk, temp);
	  if(num_items < rtree->m)
	    {
	      if(pblk == 1)
		{
		  if(num_items == 1)
		    {
		      GET_FIRSTITEM(item, temp);
		      memcpy(&ptr, item+KEYSZ, sizeof(long));
		      make_root(rtree, ptr);
		      return 0;
		    }
		  else
		    return 0;
		}
	      else
		{
		  fillin_q(rtree, pblk);
		  return condense_tree(rtree, pblk);
		}
	    }

	  return 0;
	}
      item += ENTRYSZ;
    }

  printf("condense_tree(): Internal error! It's impossible that the parent node B%d doesn't have a pointer pointing to node B%d.\n", pblk, blkno);
  myerr("condense_tree()");
}


/**
   Delete all tuples bounded by mbr.
*/
int
delete_rtree(RTree *rtree, Rect *mbr)// header modified by Richard 2002/4
{
  dbg(printf("Deleting all tuples bounded by "));
  rectPrint(mbr);
  dbg(printf("\n"));
  cBufferType cbt;
  stack<cBufferType> *dfsStack = new stack<cBufferType>;
  char buffer[BLOCKSZ];
  char *item;
  tupleno_t curtpl;
  cbt.blkptr = 1 * BLOCKSZ; // root node
  cbt.ent = 0;       // first entry to examine
  dfsStack -> push(cbt);
  int rc = 0;
  while (rc == 0){
    rc = rtree_dfs(rtree,
		   dfsStack,
		   mbr,
		   curtpl,  //no use here
		   CONTAINMENT,
		   buffer);
    if (rc == 0) {
      cbt = dfsStack->top();
      // set delete mark
      item = buffer + OVERHEAD + (cbt.ent-1) * ENTRYSZ;
      SET_DEL(item, 1);
      // write back
      write_blk_by_ptr(rtree->index, cbt.blkptr, buffer);
    }
  } // end while rc =0;
  
  return 0;
}
/* According to Antonin Guttman, update equals
   a deletion followed by an insertion, thus
   it is omitted here.
   void
   update_rtree(RTree *rtree, Rect *mbr)
   {
   }
*/

/*
 * R-Tree index debugging routines
 */
/* Given an index file offset, print the node*/

int print_one_node(RTree *rtree, pageno_t blkno){
  long node_offset = blkno * BLOCKSZ;
  int i, num_items;
  char node_type;
  char *firstItem;
  long lptr, lptr2;
  pageno_t pblk;
  char buffer[BLOCKSZ];
  void *tuple[10000];
  read_blk_by_ptr(rtree->index, node_offset, buffer);
  GET_NODETYPE(node_type, buffer);
  GET_COUNTER(&num_items, buffer);
  GET_FIRSTITEM(firstItem, buffer);
  pblk = get_parent_block(rtree, node_offset/BLOCKSZ, 1);
  printf("  [B%ld]:|'%c'|N=%d|P=%ld|",
	 node_offset/BLOCKSZ, node_type, num_items, pblk);
  if(node_type == 'l')	/* leaf nodes */
    {
      GET_NEXTLPTR(&lptr, buffer);
      GET_PREVLPTR(&lptr2, buffer);
      printf("L=p%ld,n%ld|", lptr2/BLOCKSZ, lptr/BLOCKSZ);
    }

  for(i=0; i<num_items; i++)
    {
      char *item = firstItem + i*ENTRYSZ;
      Rect r;
      long ptr;

      mem2rect(&r, item);
      memcpy(&ptr, item+4*sizeof(long), sizeof(off_t));
      printf(" [(%ld,%ld)(%ld,%ld),", r.x_ul, r.y_ul, r.x_lr, r.y_lr);
      if(node_type == 'l')
	{
	  tupleno_t tplno = ptr;
	  if (tuple_get(rtree, tuple, &tplno) == 0) 
	    printf("T%ld=%d]", tplno, *((int*)tuple));
	  else
	    printf("T%ld=ERROR]", tplno);
	}
      else
	{
	  pageno_t blkno = ptr/BLOCKSZ;
	  if(blkno < 1 || blkno >= rtree->tb_ind)
	    {
	      rtree->print(rtree,0);		
	      printf("Invalid block number %ld in B%ld\n",
		     blkno, node_offset/BLOCKSZ);
	     
	    }
	  printf("B%ld]", blkno);
	}
    }
  printf("\n");
}

/*
 * R-Tree index debugging routines
 */
/* Given an index file offset, print the node and all subsidiary nodes */
int
print_node(RTree *rtree, long node_offset)
{
  int i, num_items;
  char node_type;
  char *firstItem;
  long lptr, lptr2;
  pageno_t pblk;
  char buffer[BLOCKSZ];
  void *tuple[10000];
  read_blk_by_ptr(rtree->index, node_offset, buffer);
  GET_NODETYPE(node_type, buffer);
  GET_COUNTER(&num_items, buffer);
  GET_FIRSTITEM(firstItem, buffer);
  pblk = get_parent_block(rtree, node_offset/BLOCKSZ, 1);
  printf("  [B%ld]:|'%c'|N=%d|P=%ld|",
	 node_offset/BLOCKSZ, node_type, num_items, pblk);
  if(node_type == 'l')	/* leaf nodes */
    {
      GET_NEXTLPTR(&lptr, buffer);
      GET_PREVLPTR(&lptr2, buffer);
      printf("L=p%ld,n%ld|", lptr2/BLOCKSZ, lptr/BLOCKSZ);
    }

  for(i=0; i<num_items; i++)
    {
      char *item = firstItem + i*ENTRYSZ;
      Rect r;
      long ptr;
      char deleted;

      GET_DEL(deleted, item);
      mem2rect(&r, item);
      memcpy(&ptr, item+KEYSZ, sizeof(pageno_t));
      printf(" [(%ld,%ld)(%ld,%ld),", r.x_ul, r.y_ul, r.x_lr, r.y_lr);
      if(node_type == 'l')
	{
	  tupleno_t tplno = ptr;
	  if (tuple_get(rtree, tuple, &tplno) == 0) 
	    printf("T%ld=%d", tplno, *((int*)tuple));
	  else
	    printf("T%ld=ERROR", tplno);
	  if (deleted)
	    printf(", D]");
	  else 
	    printf("]");
	}
      else
	{
	  pageno_t blkno = ptr/BLOCKSZ;
	  if(blkno < 1 || blkno >= rtree->tb_ind)
	    {
	      rtree->print(rtree,0);		
	      printf("Invalid block number %ld in B%ld\n",
		     blkno, node_offset/BLOCKSZ);
	     
	    }
	  printf("B%ld]", blkno);
	}
    }
  printf("\n");

  if(node_type != 'l')
    for(i=0; i<num_items; i++)
      {
	char *item = firstItem + i*ENTRYSZ;
	long ptr;

	memcpy(&ptr, item+4*sizeof(long), sizeof(off_t));
	print_node(rtree, ptr);
      }
}

int print_superblk(RTree *rtree){
  printf("super: | fb:%d | tb:%d | m:%d | M:%d | L: %ld | tplSz: %d\n",
	 rtree->fb_ind, rtree->tb_ind, rtree->m, rtree->M,
	 rtree->lptr/BLOCKSZ, rtree->tplsz);
  return 0;
}

/*
int
print_rtree(char *indexFileName, char *recordFileName)
{
  RTree *rtree;

  if((rtree = (RTree *)malloc(sizeof(RTree))) == NULL)
    return -1;

  rtree->index = fopen(indexFileName, "rb");
  if(fread(buffer, 1, BLOCKSZ, rtree->index) != BLOCKSZ)
    {
      fprintf(stderr, "Error in reading the index superblock.\n");
      return -1;
    }

  memcpy(&rtree->fb_ind, buffer, sizeof(pageno_t));
  memcpy(&rtree->tb_ind, buffer+sizeof(pageno_t), sizeof(pageno_t));
  memcpy(&rtree->lptr, buffer+2*sizeof(pageno_t), sizeof(long));
  memcpy(&rtree->m,
	 buffer+2*sizeof(pageno_t)+sizeof(long),
	 sizeof(unsigned));
  memcpy(&rtree->M,
	 buffer+2*sizeof(pageno_t)+sizeof(long)+sizeof(unsigned),
	 sizeof(unsigned));

  printf("super: | fb:%d | tb:%d | m:%d | M:%d | L: %ld | tplSz: %d\n",
	 rtree->fb_ind, rtree->tb_ind, rtree->m, rtree->M,
	 rtree->lptr/BLOCKSZ, rtree->tplsz);
  if(rtree->fb_ind > 1)
    {
      /*
       * Root node exists
       
      print_node(rtree, BLOCKSZ);
    }
  /* print the free block chain in index 
  if(rtree->fb_ind != rtree->tb_ind)
    {
      pageno_t curfree = rtree->fb_ind;

      printf("  free block chain: ");
      while(curfree < rtree->tb_ind)
	{
	  printf("%ld->", curfree);
	  if(fseek(rtree->index, curfree*BLOCKSZ, SEEK_SET) != 0){
	    myerr("print_rtree:fseek!");
	    return -1;
	  }
	  if(fread(&curfree, 1, sizeof(pageno_t), rtree->index)
	     != sizeof(pageno_t)){
	    myerr("print_rtree: fread!");
	    return -1;
	  }
	}
      printf("%d\n", curfree);
    }

  fclose(rtree->index);
  free(rtree);
}

*/

/*******************************************
 * Guttman's routines.
 *******************************************/

/**
 * Guttman's routine PickSeeds()
 * Select two entries to be the first elements of the groups
 * @param mItems + mbr:  the M+1 entries
 * @return the 2 rectangles in res[2]
 */
void
pick_seeds(unsigned res[2], char *items, unsigned num_items)
{
  int i, j;
  unsigned er1_ind, er2_ind;
  long x_ul, x_lr;
  double maxd = -DBL_MAX;

  for(i=0; i+1<num_items; i++)
    {
      char *item1 = items + i*ENTRYSZ;
      Rect r1;

      mem2rect(&r1, item1);

      for(j=i+1; j<num_items; j++)
	{
	  char *item2 = items + j*ENTRYSZ;
	  Rect r2, cr;
	  double d;

	  mem2rect(&r2, item2);
	  cr = compose(&r1, &r2);
	  d = area(&cr) - area(&r1) - area(&r2);
	  if(d > maxd)
	    {
	      maxd = d;
	      er1_ind = i;
	      er2_ind = j;
	    }
	}
    }

  if(debug)
    {
      Rect er1, er2;

      mem2rect(&er1, items+er1_ind*ENTRYSZ);
      mem2rect(&er2, items+er2_ind*ENTRYSZ);
      dbg(printf("\tExtreme rectangles(d=%lg): [%d(%ld,%ld)(%ld,%ld) %d(%ld,%ld)(%ld,%ld)]\n",	     maxd,
		 er1_ind, er1.x_ul, er1.y_ul, er1.x_lr, er1.y_lr,
		 er2_ind, er2.x_ul, er2.y_ul, er2.x_lr, er2.y_lr);)
	}

  res[0] = er1_ind;
  res[1] = er2_ind;
}

/**
 * Guttman's routine PickNext()
 * Select one remaining entry for classification in a group.
 * @param cr[2]:  the two covering rectangles.
 * @param entry_flag: I don't use linked list.  So this flag array indicates
 *			whether the corresponding entry is already processed.
 * @param items: the placeholder of M+1 entries.
 * @param num_items: the value M+1.
 * @return: the index of the next item.
 */
int
pick_next(Rect *coverr0, Rect *coverr1,
	  char *entry_flag, char *items, unsigned num_items)
{
  int i;
  int res=-1;
  double maxdiff=-DBL_MAX;

  for(i=0; i<num_items; i++)
    if(entry_flag[i] == 'o')
      {
	double d[2], diff;
	Rect r, incr[2];
	char *item = items + i*ENTRYSZ;

	mem2rect(&r, item);

	incr[0] = compose(coverr0, &r);
	incr[1] = compose(coverr1, &r);
	d[0] = area(incr) - area(coverr0);
	d[1] = area(incr+1) - area(coverr1);
	diff = ABS(d[0]-d[1]);
	if(diff > maxdiff)
	  res = i;
      }

  if(res != -1)
    entry_flag[res] = 'X';
  return res;
}

/**
 * Guttman's routine SplitNode()
 * @param node: the node being split
 * @param mbr: the entry being inserted
 * @param tupleptr: the tuple pointer being inserted in the index
 *	mbr + tupleptr = complete entry
 * @param nodetype: 'l' to leaf, 'n' to non-leaf
 * @param pptr: the parent pointer for both split nodes
 * @param ptr0: For nodetype=='l', this is the previous leaf pointer for split0
 *		For nodetype=='n', this is the previous split0
 * @param ptr1: For nodetype=='l', this is the the next leaf pointer for split1
 *		For nodetype=='n', this is the previous split1
 * @param anc:  Ancestors of current node
 * Implicit global arguments: if nodetype=='n', then coverr0, coverr1
 * hold the cover rectangles of the two nodes just split.
 */
int
split_node(RTree *rtree, pageno_t curblk, Rect *mbr, long tupleptr,
	   char nodetype, long ptr0, long ptr1)
{
  unsigned er_ind[2];
  unsigned num_items[2];
  char *firstItem;
  char entry_flag[MAX_ENTRIES+1];
  int i;
  char *item, *destitem;
  long ptr;

  /* read the node being split */
  read_blk(rtree->index, curblk, buffer);
  /* construct the M+1 entries ==> itembuf.
     [0] is the one being inserted. [1..M] is the M entries in NODE */
  GET_FIRSTITEM(firstItem, buffer);
  rect2mem(itembuf, mbr);
  memcpy(itembuf+KEYSZ, &tupleptr, sizeof(long));
  memcpy(itembuf+ENTRYSZ, firstItem, MAX_ENTRIES*ENTRYSZ);
  /*
  if(nodetype == 'n')
    /* if ptr0 and ptr1 are previously split nodes,
    update the pointers in this node to point to them 
    for(i=0; i<MAX_ENTRIES+1; i++)
      {
	Rect r;
	char *item = itembuf+i*ENTRYSZ;
	mem2rect(&r, item);
	if(containedIn(&r, &coverr0) == 0){
	  dbg(printf("blkno0=\n", ptr0/BLOCKSZ));
	  memcpy(item+KEYSZ, &ptr0, sizeof(long));
	}
	if(containedIn(&r, &coverr1) == 0){
	  dbg(printf("blkno1=\n", ptr1/BLOCKSZ));
	  memcpy(item+KEYSZ, &ptr1, sizeof(long));
	}
      }
  */
  /* pick the two seeds ==> er_ind[2] */
  pick_seeds(er_ind, itembuf, MAX_ENTRIES+1);

  for(i=0; i<MAX_ENTRIES+1; i++)
    entry_flag[i] = 'o';	/* not grouped yet */

  num_items[0] = num_items[1] = 1;
  /* group 0 */
  entry_flag[er_ind[0]] = 'X';
  GET_FIRSTITEM(item, split0);
  SET_DEL(item, 0);
  memcpy(item, itembuf+er_ind[0]*ENTRYSZ, ENTRYSZ);
  SET_COUNTER(split0, &num_items[0]);
  /* group 1 */
  entry_flag[er_ind[1]] = 'X';
  GET_FIRSTITEM(item, split1);
  memcpy(item, itembuf+er_ind[1]*ENTRYSZ, ENTRYSZ);
  SET_DEL(item, 0);
  SET_COUNTER(split1, &num_items[1]);


  /*
   * Then let's insert the remaining entries one by one
   * ordered by PickNext().
   */
  item = itembuf+er_ind[0]*ENTRYSZ;
  mem2rect(&coverr0, item);
  item = itembuf+er_ind[1]*ENTRYSZ;
  mem2rect(&coverr1, item);

  for(i=0; i<MAX_ENTRIES-1; i++)
    {
      double enlarge[2];
      Rect r, cr[2];
      int next;
      int flag = -1;

      next = pick_next(&coverr0, &coverr1, entry_flag, itembuf, MAX_ENTRIES+1);
      if(next == -1)
	{
	  fprintf(stderr, "Internal error! Picked next is -1\n");
	  exit(-1);
	}

      /* append the NEXT into either group0 or group1 */
      item = itembuf+next*ENTRYSZ;
      mem2rect(&r, item);
      dbg(printf("\tsplit(): grouping [%d(%ld,%ld)(%ld,%ld)]\n",
		 next, r.x_ul, r.y_ul, r.x_lr, r.y_lr);)

	cr[0] = compose(&coverr0, &r);
      cr[1] = compose(&coverr1, &r);
      enlarge[0] = area(cr)-area(&coverr0);
      enlarge[1] = area(cr+1)-area(&coverr1);
      if(enlarge[0] < enlarge[1] ||
	 (enlarge[0] == enlarge[1] && area(cr) < area(cr+1)) ||
	 (enlarge[0] == enlarge[1] && area(cr) == area(cr+1) &&
	  num_items[0] < num_items[1]))
	flag = 0;
      else
	flag = 1;
      if((rtree->m-num_items[0]) == (MAX_ENTRIES-1-i) &&
	 flag == 1)
	flag = 0;
      if((rtree->m-num_items[1]) == (MAX_ENTRIES-1-i) &&
	 flag == 0)
	flag = 1;
      if(flag == 0)
	{
	  /* insert the next entry to group 0 */
	  GET_FIRSTITEM(destitem, split0);
	  SET_DEL(item, 0);
	  destitem += (num_items[0]++)*ENTRYSZ;
	  memcpy(&coverr0, &cr[0], sizeof(Rect));
	  dbg(printf("\t\t(enlarge to group0=%lg) coverr=[(%ld,%ld)(%ld,%ld)]\n",
		     enlarge[0],
		     coverr0.x_ul, coverr0.y_ul, coverr0.x_lr, coverr0.y_lr);)
	    }
      else
	{
	  /* insert the next entry to group 1 */
	  GET_FIRSTITEM(destitem, split1);
	  SET_DEL(item, 0);
	  destitem += (num_items[1]++)*ENTRYSZ;
	  memcpy(&coverr1, &cr[1], sizeof(Rect));

	  dbg(printf("\t\t(enlarge to group1=%lg) coverr=[(%ld,%ld)(%ld,%ld)]\n",
		     enlarge[1],
		     coverr1.x_ul, coverr1.y_ul,
		     coverr1.x_lr, coverr1.y_lr);)
	    }
      memcpy(destitem, item, ENTRYSZ);
    }

  /* now the entries are split into node (split0) and node (split1),
     setup other contents and write them */
  blkno0 = get_newblock(rtree);
  blkno1 = get_newblock(rtree);
  dbg(printf("\tSplit to B%ld coverred by [(%ld,%ld)(%ld,%ld)] and B%ld coverred by [(%ld,%ld)(%ld,%ld)]\n",
	     blkno0, coverr0.x_ul, coverr0.y_ul, coverr0.x_lr, coverr0.y_lr,
	     blkno1, coverr1.x_ul, coverr1.y_ul, coverr1.x_lr, coverr1.y_lr);)

    /* set up split0 */
    SET_NODETYPE(split0, nodetype);
  SET_COUNTER(split0, &num_items[0]);
  ptr = blkno1*BLOCKSZ;
  if(nodetype == 'l')
    {
      SET_NEXTLPTR(split0, &ptr);	/* next leaf is split1 */
      SET_PREVLPTR(split0, &ptr0);	/* adjusting leaf node chain */
    }
  /* write split0 */
  write_blk(rtree->index, blkno0, split0);
  /* set up split1 */
  SET_NODETYPE(split1, nodetype);
  SET_COUNTER(split1, &num_items[1]);
  ptr = blkno0*BLOCKSZ;
  if(nodetype == 'l')
    {
      SET_NEXTLPTR(split1, &ptr1);	/* adjusting leaf node chain */
      SET_PREVLPTR(split1, &ptr);	/* prev leaf is split0 */
    }
  /* write split1 */
  write_blk(rtree->index, blkno1, split1);
}

/**
 * Guttman's routine AdjustTree():
 *
 * @param curblk: the parent block/node being adjusted
 * Implicit global arguments:
 * @param split0, split1: hold the two split groups
 * @param coverr0, coverr1: hold the two covering rectangle for the two groups
 * @param blkno0, blkno1: hold the block numbers of split0 and split1
 * @param anc: ancestors
 */
int
adjust_tree_with_split(RTree *rtree, pageno_t curblk, stack<pageno_t> &anc)
{
  pageno_t curptr=curblk * BLOCKSZ;
  long ptr;
  char *item;

  if(curblk == 1)
    {
      /*
       * Root splits.
       * Create a new root to acommodate split0 and split1.
       */
      unsigned num_items = 2;

      SET_NODETYPE(buffer, 'n');	/* non-leaf */
      SET_COUNTER(buffer, &num_items);

      /* entry 0 */
      GET_FIRSTITEM(item, buffer);
      rect2mem(item, &coverr0);
      SET_DEL(item, 0);

      ptr = blkno0 * BLOCKSZ;
      memcpy(item+KEYSZ, &ptr, sizeof(long));
      /* entry 1 */
      item += ENTRYSZ;
      rect2mem(item, &coverr1);
      SET_DEL(item, 0);
      ptr = blkno1 * BLOCKSZ;
      memcpy(item+KEYSZ, &ptr, sizeof(long));

      write_blk_by_ptr(rtree->index, BLOCKSZ, buffer);
    }
  else
    {
      /*
       * split of a non-root node
       */
      pageno_t pblk, ppblk;
      unsigned num_items;
      int i;
      if(anc.empty())
	{
	  fprintf(stderr, "Internal error! The parent of B%d doesn't exist.\n",
		  curblk);
	  myerr("adjust_tree_with_split");
	}
      /* Read the parent node */
      pblk = anc.top();
      anc.pop();
      dbg(printf("adjust_tree_with_split():Get B%d's parent B%d\n", curblk, pblk));
      read_blk(rtree->index, pblk, buffer);
      GET_FIRSTITEM(item, buffer);
      GET_COUNTER(&num_items, buffer);
      for(i=0; i<num_items; i++)
	{
	  memcpy(&ptr, item+4*sizeof(long), sizeof(long));

	  if(ptr == curptr)
	    {
	      dbg(printf("\tBingo.  Inserting the 2 split nodes into %d(th) item in B%d\n",
			 i, pblk);)


		if(num_items == MAX_ENTRIES)
		  {
		    /*
		     * Already full, need further split
		     */
		    /* Caveats:
		       before this point is reached, the split0 and split1
		       must have been written into hard disk, so I can
		       re-use them right here */
		    long ptr0=blkno0*BLOCKSZ, ptr1=blkno1*BLOCKSZ;
		    Rect mbr = coverr1;

		    dbg(printf("\tNode %d is full. Non-leaf node split.\n", pblk);)
		      update_block_entry(rtree, pblk, i, &coverr0, ptr0);
		    split_node(rtree, pblk, &mbr, ptr1, 'n', ptr0, ptr1);
		    adjust_tree_with_split(rtree, pblk, anc);
		  }
		else
		  {
		    int j;
		    Rect r;

		    /* move entries [i+1..num_items-1] to [i+2..num_items] */
		    for(j=num_items-i; j>1; j--)
		      memcpy(item+j*ENTRYSZ, item+(j-1)*ENTRYSZ, ENTRYSZ);
		    /* now entries [i] and [i+1] are left open */
		    /* put coverr0 and coverr1 there */
		    /* [i] */
		    rect2mem(item, &coverr0);
		    ptr = blkno0 * BLOCKSZ;
		    memcpy(item+KEYSZ, &ptr, sizeof(long));
		    /* [i+1] */
		    item += ENTRYSZ;
		    rect2mem(item, &coverr1);
		    ptr = blkno1 * BLOCKSZ;
		    memcpy(item+KEYSZ, &ptr, sizeof(long));

		    num_items++;
		    SET_COUNTER(buffer, &num_items);
		    /* update it */
		    write_blk(rtree->index, pblk, buffer);
		    r = compose(&coverr0, &coverr1);
		    adjust_tree_without_split(rtree, pblk, &r, anc);
		  }
	      break;
	    }
	  else
	    item += ENTRYSZ;
	}
      /* put the node being split (curblk) back into free block list */
      free_block(rtree, curblk, 0);

      if(i == num_items)
	{
	  printf("adjust_tree_with_split(): Internal error! It's impossible that the parent node doesn't have a pointer pointing to node %d.\n", curblk);
	    myerr("adjust_tree_with_split()");
	}
      else
	return 0;
    }
}
int
adjust_tree_without_split(RTree *rtree, pageno_t curblk, Rect *mbr, stack<pageno_t> &anc)
{
  pageno_t curptr = curblk * BLOCKSZ;
  long ptr;
  char *item;
  unsigned num_items;
  Rect r, coverr;
  pageno_t pblk;
  int i;

  if(curblk == 1)
    /* root node */
    return 0;
  if(anc.empty())
    {
      fprintf(stderr, "Internal error! The parent of B%d doesn't exist.\n",
	      curblk);
      myerr("adjust_tree_without_split");
    }
  /* Read the parent node */
  pblk = anc.top();
  anc.pop();
  dbg(printf("adjust_tree_without_split(): Get B%d's parent B%d\n", curblk, pblk));

  read_blk(rtree->index, pblk, buffer);
  GET_FIRSTITEM(item, buffer);
  GET_COUNTER(&num_items, buffer);
  for(i=0; i<num_items; i++)
    {
      memcpy(&ptr, item+4*sizeof(long), sizeof(long));

      if(ptr == curptr)
	{
	  dbg(printf("\tBingo.  Enlarging %d(th) item in B%d\n",
		     i, pblk);)
	    mem2rect(&r, item);
	  coverr = compose(&r, mbr);

	  if(containedIn(&coverr, &r) == 0)
	    {
	      /* the MBR is already properly contained in
		 existing rectangle */
	      dbg(printf("\tNo change hence.\n");)
		return 0;
	    }

	  rect2mem(item, &coverr);
	  write_blk(rtree->index, pblk, buffer);
	  break;
	}
      else
	item += ENTRYSZ;
    }

  if(i == num_items)
    {
      printf("adjust_tree_without_split(): Internal error! It's impossible that the parent node B%d doesn't have a pointer pointing to node B%d.\n", pblk, curblk);
      myerr("adjust_tree_without_split()");
    }
  else
    return adjust_tree_without_split(rtree, pblk, &coverr, anc);
}

/**
 * Auxiliary routine: allocate a new disk block.
 * If there are freed blocks in the middle, use them,
 * otherwise use the new block at the end of the file.
 */
pageno_t
get_newblock(RTree *rtree)
{
  pageno_t res;

  if(rtree->fb_ind == rtree->tb_ind)
    {
      res = rtree->fb_ind;
      rtree->tb_ind = rtree->fb_ind = res+1;
    }
  else
    {
      /*
       * some free block in the middle, put it there
       */
      /* first to know the future free block head */
      read_blk(rtree->index, rtree->fb_ind, buffer);
      res = rtree->fb_ind;
      memcpy(&rtree->fb_ind, buffer, sizeof(pageno_t));
    }

  return res;
}

/**
 * Guttman's routine ChooseLeaf():
 * Select a leaf node in which to place a new index entry mbr.
 * @param mbr: the MBR of the object being inserted.
 * @param anc: ancestor stack which records all ancestors of 
               the result leaf node, excluding itself.
 */
pageno_t
choose_leaf(RTree *rtree, Rect *mbr, stack<pageno_t> &anc)
{
  char node_type;
  unsigned num_items, i;
  char *firstItem, *item;
  pageno_t res;

  /*
   * Search the R-Tree index
   */
  /* seek to the root node of the index */
  read_blk_by_ptr(rtree->index, BLOCKSZ, buffer);
  GET_NODETYPE(node_type, buffer);
  GET_COUNTER(&num_items, buffer);
  GET_FIRSTITEM(firstItem, buffer);
  res = 1;
  /* non-leaf node: scan the existing items */
  while(node_type != 'l')
    {
      double least = DBL_MAX;	/* the least enlargement */
      unsigned ind = UINT_MAX;
      long ptr;
      // record the result into ancestor stack
      anc.push(res);

      item = firstItem;
      for(i=0; i<num_items; i++)
	{
	  Rect r, cr;
	  double enlarge;

	  mem2rect(&r, item);

	  cr = compose(mbr, &r);

	  enlarge = area(&cr) - area(&r);
	  if(enlarge < least)
	    {
	      least = enlarge;
	      ind = i;
	    }

	  item += ENTRYSZ;

	}
      if (ind == UINT_MAX) {
	myerr("ChooseLeaf: no node to split!");
	return UINT_MAX;
      }

      /* seek to the node with least expansion */
      memcpy(&ptr, firstItem+ind*ENTRYSZ+KEYSZ, sizeof(long));
      if(debug)
	{
	  Rect r;

	  item = firstItem + ind*ENTRYSZ;
	  mem2rect(&r, item);
	  dbg(printf("\tChooseLeaf: %d[(%d %d) (%d %d)] -> B%ld\n",
		     ind, r.x_ul, r.y_ul, r.x_lr, r.y_lr, ptr/BLOCKSZ);)
	    }
      read_blk_by_ptr(rtree->index, ptr, buffer);
      GET_NODETYPE(node_type, buffer);
      GET_COUNTER(&num_items, buffer);
      GET_FIRSTITEM(firstItem, buffer);
      res = ptr/BLOCKSZ;
    }
  dbg(printf("\tChooseLeaf: Leaving\n"));
  return res;
}

void myerr(char *s){
  fprintf(stderr,"RTREE ERROR:");
  fprintf(stderr, s);
  fprintf(stderr,"\n");
  perror(s);
  exit(1);
}

/* rtree_dfs: (Continue to) DFS the rtree for the first match tuple.  Intermediate nodes are recorded in the stack rtc->dfsStack, in order to do the next match
   param:
     dfsStack   : stack for DFS, initial value could be the root; When a match 
                  is found, the stack stores the path from root to leaf (inclusive).
     mbr        : mbr to query, for CONTAINMENT search, mbr stores the result rectangle
     curtpl     : result tuple no, UINT_MAX if not found;
     type       : EQUALITY : mbr is equal to some tuple?
                  CONTAINMENT: some tuple is contained in  mbr?
     temp       : stores the leaf block if found match, this is useful to set
                  rtc->curnode while doing cursor_get with flag DB_SET/DB_SET_RANGE
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
	      char *temp){
  char buffer[BLOCKSZ];
  if (!temp) temp = buffer;
  cBufferType cbt;
  dbg(printf("Entering rtree_dfs\n"));
  if (!dfsStack->empty()){
    cbt = dfsStack->top();
  }
  else{
    dbg(printf("Stack is empty!\n"));
  }
  while (!dfsStack->empty()){

    char nodetype;
    char *item;
    unsigned num_items;
    dbg(printf("DFS: B%d entry=%d\n", cbt.blkptr/BLOCKSZ, cbt.ent));
    /* read the current block */
    read_blk_by_ptr(rtree->index, cbt.blkptr, temp);
    GET_NODETYPE(nodetype, temp);
    GET_ITEM(item, temp, cbt.ent);
    GET_COUNTER(&num_items, temp);
    
    /* now scan each child block */
    int found;
    found = 0;
    for(int i=cbt.ent; i<num_items; i++){
      Rect r;
      pageno_t b;

      mem2rect(&r, item);
      /* Read the tuple no. for leaf, or child block ptr for non-leaf */
      memcpy(&b, item+KEYSZ, sizeof(long));
      if (nodetype == 'l'){ // leaf
	dbg(printf("Testing T%d entry=%d mbr =", b, i));
	rectPrint(mbr);
	dbg(printf("T%d=", i));
	rectPrint(&r,1);
	int got;
	switch (type){
	case EQUALITY:
	  got = (containedIn(mbr, &r) == 0);
	  break;
	case CONTAINMENT:
	  got = (containedIn(&r, mbr) >= 0);
	  break;
	default:
	  myerr("rtree_dfs: Unknown search type!");
	}// end switch type
	if (got){
	  dbg(printf("Got leaf: T%d %d(th) item\n", b, i));
	  // check deleted 	  
	  char deleted;
	  GET_DEL(deleted, item);
	  if (!deleted){
	    /* write the result */
	    curtpl = b;
	    if (type == CONTAINMENT)
	      memcpy(mbr, item, sizeof(Rect));
	    
	    /* update the stack*/
	    dfsStack->pop();
	    cbt.ent = i + 1;
	    dfsStack->push(cbt);
	    return 0;
	  }
	}
      }
      else {  // non-leaf
	dbg(printf("Examing B%d\n", b/BLOCKSZ));
	if (overlap(mbr, &r) >= 0){
	  found = 1;
	  /* update the stack */
	  dfsStack->pop();
	  cbt.ent = i + 1;
	  dfsStack->push(cbt);
	  
	  cbt.blkptr = b;
	  cbt.ent = 0;
	  dfsStack->push(cbt);
	  break;
	}

      } // end if nodetype
      item += ENTRYSZ;
    } // end for each children
    if (!found){
      dfsStack->pop();
      if (!dfsStack->empty())
	cbt= dfsStack->top();
    }
    else { // found
    }
  } // end while
  curtpl = UINT_MAX;
  return DB_NOTFOUND;
}
