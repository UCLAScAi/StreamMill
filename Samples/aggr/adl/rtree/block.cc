#include <stdio.h>
#include <stdlib.h>
#include "rtree.h"
#include <errno.h>
#include "block.h"
int
update_block_type(RTree *rtree, pageno_t blkno, char node_type)
{
  char buffer[BLOCKSZ];			 /*one for read/write index */
  read_blk(rtree->index, blkno, buffer);
  SET_NODETYPE(buffer, node_type);
  write_blk(rtree->index, blkno, buffer);
  return 0;
}

int
update_block_entry(RTree *rtree, pageno_t blkno, unsigned num,
		   Rect *mbr, long ptr)
{
  char buffer[BLOCKSZ];			 /*one for read/write index */
    char *item;
    read_blk(rtree->index, blkno, buffer);
    GET_FIRSTITEM(item, buffer);
    item += num*ENTRYSZ;
    rect2mem(item, mbr);
    memcpy(item+KEYSZ, &ptr, sizeof(long));
    write_blk(rtree->index, blkno, buffer);
    return 0;
}

/**
 * put the block back into free block list
 * @param flag: 0 the deletion of the split node,
 *		  no need to update leaf chain because it has already
 *		  been updated
 *              1 the normal deletion, need to update leaf chain.
*/
int
free_block(RTree *rtree, pageno_t blkno, int flag)
{
    char nodetype;
    char buffer[BLOCKSZ];			 /*one for read/write index */
    if (blkno == 1) {  // root 
      if (init_rtree(rtree) == -1) {
	myerr("FREE block :INIT RTREE!\n");
	return -1;
      }
 
	dbg(rtree_print(rtree,0));
	return 0;
    }

    if(flag == 1)
    {
	/* update leaf chain */
      read_blk(rtree->index, blkno, buffer);
	GET_NODETYPE(nodetype, buffer);
	if(nodetype == 'l')
	{
	    long prevlptr, nextlptr;

	    GET_PREVLPTR(&prevlptr, buffer);
	    GET_NEXTLPTR(&nextlptr, buffer);
	    if(prevlptr != 0)
	    {
	      read_blk_by_ptr(rtree->index, prevlptr, buffer);
	      SET_NEXTLPTR(buffer, &nextlptr);
	      write_blk_by_ptr(rtree->index, prevlptr, buffer);
	    }
	    else
	    {
		if(nextlptr == 0)
		  rtree->lptr = BLOCKSZ; // the only one leaf being deleted, point back to root;
		else
		    rtree->lptr = nextlptr;
	    }
	    if(nextlptr != 0)
	    {
	      read_blk_by_ptr(rtree->index, nextlptr, buffer);
	      SET_PREVLPTR(buffer, &prevlptr);
	      write_blk_by_ptr(rtree->index, nextlptr, buffer);
	    }
	}
    }

    
    /* maintain the free block chain */
    if(fseek(rtree->index, blkno*BLOCKSZ, SEEK_SET) != 0){
      myerr("free_block:fseek!");
      return -1;
    }
    if(fwrite(&rtree->fb_ind, 1, sizeof(pageno_t), rtree->index)
       != sizeof(pageno_t)){
      myerr("free_block:fwrite!");
	return -1;
    }
    rtree->fb_ind = blkno;
    return 0;
}



/**
 * get parent blk of the given `blkno'
 * @param blkno: return parent block num of this guy.
 * @param curblk: current block in depth-first-search,
 *		  pass 1 (which is the root) in the first call.
 * @return -1 if something wrong, otherwise the parent block num of `blkno'
 */
/*
pageno_t get_parent_block(RTree *rtree, pageno_t blkno){
  
}
*/

pageno_t
get_parent_block(RTree *rtree, pageno_t blkno, pageno_t curblk)
{
    char nodetype;
    unsigned num_items;
    char *item;
    char temp[BLOCKSZ];
    int i;
    if(blkno == 1){
	/* the parent of root is root itself */
      dbg(printf("Get_parent_block(B%d): End searching B%d\n", blkno, curblk));
	return 1;
    }
    /* read the current block at first */
    read_blk(rtree->index, curblk, temp);
    GET_NODETYPE(nodetype, temp);
    if(nodetype == 'l'){
      dbg(printf("Get_parent_block(B%d): End searching B%d\n", blkno, curblk));
      return 0;
    }
    /* now scan each child block */
    GET_FIRSTITEM(item, temp);
    GET_COUNTER(&num_items, temp);
    for(i=0; i<num_items; i++)
    {
	pageno_t b;
	memcpy(&b, item+KEYSZ, sizeof(long));
	b /= BLOCKSZ;
	if(b < 1 || b >= rtree->tb_ind)
	{
	    printf("get_parent_block(B%d): Invalid block number %d in node %d\n", blkno, b, curblk);

	    return UINT_MAX;
	}
	if(b == blkno){
	  dbg(printf("Get_parent_block(B%d): End searching B%d\n", blkno, curblk));
	  return curblk;
	}
	b = get_parent_block(rtree, blkno, b);
	if(b != 0){
	  dbg(printf("Get_parent_block(B%d): End searching B%d\n", blkno, curblk));
	  return b;
	}
	item += ENTRYSZ;
    }
    dbg(printf("Get_parent_block(B%d): End searching B%d\n", blkno, curblk));
    return 0;
}


/* fill all LEAFs of node `blkno' into qbuf */
void
fillin_q(RTree *rtree, pageno_t blkno)
{
    char node[BLOCKSZ];
    char *item;
    char nodetype;
    unsigned num_items;
    int i;

    /* read the block */
    read_blk(rtree->index, blkno, node);
    GET_NODETYPE(nodetype, node);
    GET_FIRSTITEM(item, node);
    GET_COUNTER(&num_items, node);

    if(nodetype == 'l')
    {
	unsigned tbufsz = qbufsz + num_items;
	char *tbuf = (char *)malloc(tbufsz*ENTRYSZ);

	/* this equals realloc() qbuf into tbuf,
	   I doubt realloc() is not robust on certain platforms, thus ... */
	memcpy(tbuf, qbuf, qbufsz*ENTRYSZ);
	memcpy(tbuf+qbufsz*ENTRYSZ, item, num_items*ENTRYSZ);
	free(qbuf);
	qbuf = tbuf;
	qbufsz = tbufsz;

	free_block(rtree, blkno, 1);
	return;
    }

    /* non-leaf node, fill all sub-blocks into qbuf */
    for(i=0; i<num_items; i++)
    {
	pageno_t ptr;

	memcpy(&ptr, item+KEYSZ, sizeof(long));
	fillin_q(rtree, ptr/BLOCKSZ);
	item += ENTRYSZ;
    }
    free_block(rtree, blkno, 1);
}

/**
 * Make blkno as the root node
 */
void
make_root(RTree *rtree, pageno_t blkptr)
{
  char buffer[BLOCKSZ];			 /*one for read/write index */
  pageno_t blkno = blkptr / BLOCKSZ;
  dbg(printf("Converting B%ld into root\n", blkno));
    
  if(rtree->lptr == blkptr)
    /* if this block is the head of the leaf chain */
    rtree->lptr = BLOCKSZ;
  
  /* read the block */
  read_blk_by_ptr(rtree->index, blkptr, buffer);
  /* write to the root block */
  write_blk(rtree->index, 1, buffer);
  free_block(rtree, blkno, 1);

}


/* Write block in buffer into the index file
   @param
     f: the index filename;
     blkno: the destination block no. to write
     buffer: block content
   @return: 
     0 successful, otherwise err code
 */
int write_blk(FILE *f, pageno_t blkno, void* buffer){
  if(fseek(f, blkno * BLOCKSZ, SEEK_SET) != 0){
    myerr("write_blk:fseek!\n");
    return -1;
  }
  if(fwrite(buffer, BLOCKSZ, 1, f) != 1){
    myerr("write_blk: fwrite!\n");
    return -1;
  }
  return 0;
}

/* Read block into buffer from the index file
   @param
     f: the index filename;
     blkno: the block no. to read
     buffer: to store the block content
   @return: 
     0 successful, otherwise err code
 */
int read_blk(FILE *f, pageno_t blkno, void* buffer){
  
  if(fseek(f, blkno * BLOCKSZ, SEEK_SET) != 0){
    printf("read_blk error: blk_no=%d\n", blkno);
    myerr("read_blk:fseek!\n"); 
    return -1;
  }
  if(fread(buffer, BLOCKSZ, 1, f) != 1){
    printf("read_blk error: blk_no=%d\n", blkno);
    myerr("read_blk: fread!\n");
    return -1;
  }
  return 0;
}
/* Write block pointed by ptr in buffer into the index file
   @param
     f: the index filename;
     ptr: the destination block ptr to write
     buffer: block content
   @return: 
     0 successful, otherwise err code
 */
int write_blk_by_ptr(FILE *f, pageno_t ptr, void* buffer){
  if(fseek(f, ptr, SEEK_SET) != 0){
    myerr("write_blk_ptr:fseek!\n");
    return -1;
  }
  if(fwrite(buffer, BLOCKSZ, 1, f) != 1){
    myerr("write_blk_ptr: fwrite!\n");
    return -1;
  }
  return 0;
}

/* Read block pointed by ptr into buffer from the index file
   @param
     f: the index filename;
     ptr: the block ptr to read
     buffer: to store the block content
   @return: 
     0 successful, otherwise err code
 */
int read_blk_by_ptr(FILE *f, pageno_t ptr, void* buffer){
  if(fseek(f, ptr, SEEK_SET) != 0){
    myerr("read_blk_ptr:fseek!\n"); 
    return -1;
  }
  if(fread(buffer, BLOCKSZ, 1, f) != 1){
    myerr("read_blk_ptr: fread!\n");
    return -1;
  }
  return 0;
}
