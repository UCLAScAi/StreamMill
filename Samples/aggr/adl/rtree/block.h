int update_block_type(RTree *rtree, pageno_t blkno, char node_type);
int update_block_entry(RTree *rtree, pageno_t blkno, unsigned num,
		       Rect *mbr, long ptr);
/**
 * put the block back into free block list
 * @param flag: 0 the deletion of the split node,
 *		  no need to update leaf chain because it has already
 *		  been updated
 *              1 the normal deletion, need to update leaf chain.
*/
int
free_block(RTree *rtree, pageno_t blkno, int flag);
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
get_parent_block(RTree *rtree, pageno_t blkno, pageno_t curblk);

/**
 * Make blkno as the root node
 */
void
make_root(RTree *rtree, pageno_t blkptr);
/* fill all LEAFs of node `blkno' into qbuf */
void
fillin_q(RTree *rtree, pageno_t blkno);

/* Write block in buffer into the index file
   @param
     f: the index filename;
     blkno: the destination block no. to write
     buffer: block content
   @return: 
     0 successful, otherwise err code
 */
int write_blk(FILE *f, pageno_t blkno, void* buffer);

/* Read block into buffer from the index file
   @param
     f: the index filename;
     blkno: the block no. to read
     buffer: to store the block content
   @return: 
     0 successful, otherwise err code
 */
int read_blk(FILE *f, pageno_t blkno, void* buffer);
/* Write block pointed by ptr in buffer into the index file
   @param
     f: the index filename;
     ptr: the destination block ptr to write
     buffer: block content
   @return: 
     0 successful, otherwise err code
 */
int write_blk_by_ptr(FILE *f, pageno_t ptr, void* buffer);
/* Read block pointed by ptr into buffer from the index file
   @param
     f: the index filename;
     ptr: the block ptr to read
     buffer: to store the block content
   @return: 
     0 successful, otherwise err code
 */
int read_blk_by_ptr(FILE *f, pageno_t ptr, void* buffer);
