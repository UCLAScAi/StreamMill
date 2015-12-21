
#include "rtree.h"
#include "tupledb.h"
#include "stdio.h"
#include "stdlib.h"
#include "rectangle.h"
#include "limits.h"

#include "block.h"

int
rtree_create(RTree **rtree, char *indexFileName, char *recordFileName,int flags)
{
  RTree *rt;
  rt = open_rtree(indexFileName, recordFileName, flags);
  if(rt == NULL)
    return -1;

  *rtree = rt;
    
  return 0;
}

int
rtree_open(RTree **rtree, char *indexFileName, char *recordFileName,
	   int flags)
{
  RTree *rt;

  rt = open_rtree(indexFileName, recordFileName, 0);
  if(rt == NULL)
    return -1;

  *rtree = rt;

  return 0;
}

int
rtree_close(RTree *rtree, int flags)
{
  int ret = close_rtree(rtree);

  if(ret == -1)
    return -1;
  return 0;
}


/* Arbitrarily return one tuple matching key->data.
   rtree: underlying RTREE;
   key: key->data, bounding rectangle;
        key->size, fixed to sizeof(Rect);
   dbt: returning tuple, dbt is uninitialized;
   flags: fixed to 0;
 */
int
rtree_get(RTree *rtree,
	  DBT *key, DBT * dbt,int flags)
{
  pageno_t blk;
  unsigned ent;
  tupleno_t tpl;
  Rect *mbr=NULL;
  void *temp[100];
  mbr = (Rect*)key->data;
  dbt->data = rtree->current_tuple; // initialize dbt
  rtree_get_one(&blk, &ent, &tpl, rtree, mbr, 1);
  if(tpl != UINT_MAX)
    {
      if (tuple_get(rtree,dbt->data, &tpl) == 0)
	return 0;
      else
	myerr("rtree_get: tuple_get!\n");
    }
  return DB_NOTFOUND;
}

/* rtree_put: Put data into rtree.
   flags: 0 for normal use;
 */

int
rtree_put(RTree *rtree,
	  DBT *key, DBT *dbt, 
	  int flags)
{
  pageno_t blk;
  unsigned ent;
  tupleno_t tpl;
  Rect *mbr=NULL;
  int rc;
  dbg(printf("puting tuple %d...\n", *((int*)(dbt->data))));
  mbr = (Rect*)key->data;    


  switch(flags)
    {
    case 0:
      break;
    case DB_NODUPDATA:
    case DB_NOOVERWRITE:
      /*Enter the new key/data pair only if the key does not already appear 
	in the database. If the key already appears in the database, DB_KEYEXIST is returned. 
	Even if the database allows duplicates, a call to DB->put with the DB_NOOVERWRITE flag set 
	will fail if the key already exists in the database.
      */
      rtree_get_one(&blk, &ent, &tpl, rtree, mbr, 1); 
      if(tpl != UINT_MAX)
	return DB_KEYEXIST;
    default:
      return -1;
    }
  rc = insert_rtree(rtree, mbr, dbt);
  return rc;
}

int
rtree_del(RTree *rtree, Rect *mbr, int flags)
{
  return delete_rtree(rtree, mbr);
}

void
rtree_print(RTree *rtree, int flags)
{
  printf("super: | fb:%d | tb:%d | m:%d | M:%d | L: %ld | tplSz: %d\n",
	 rtree->fb_ind, rtree->tb_ind, rtree->m, rtree->M,
	 rtree->lptr/BLOCKSZ, rtree->tplsz);
  if(rtree->fb_ind > 1)
    {
      /*
       * Root node exists
       */
      print_node(rtree, BLOCKSZ);
    }
  /* print the free block chain in index */
  if(rtree->fb_ind != rtree->tb_ind)
    {
      pageno_t curfree = rtree->fb_ind;

      printf("  free block chain: ");
      while(curfree < rtree->tb_ind)
	{
	  printf("%ld->", curfree);
	  if(fseek(rtree->index, curfree*BLOCKSZ, SEEK_SET) != 0)
	    return;
	  if(fread(&curfree, 1, sizeof(pageno_t), rtree->index)
	     != sizeof(pageno_t))
	    return;
	}
      printf("%d\n", curfree);
    }
}

int
rtree_cursor(RTree *rtree, RTreeCursor **rtcp, int flags)
{
  RTreeCursor *rtc;
  unsigned num_items;
  char *item;
  int ret;

  if((rtc = (RTreeCursor *)calloc(1, sizeof(RTreeCursor))) == NULL){
    printf("Memory Allocate error in Cursor Open!\n");      
    return -1;
  }

  if (rtree->fb_ind !=1) { //RTREE is not empty
    /* Initialization: goto the first leaf node and read it */
    if(fseek(rtree->index, rtree->lptr, SEEK_SET) != 0){
      printf("fseek error in Cursor Open!\n");
      return -1;
    }
    if(fread(rtc->curnode, 1, BLOCKSZ, rtree->index) != BLOCKSZ){
      printf("fread error in Cursor Open!\n");
      return -1;
    }
    rtc->curBlkPtr = rtree->lptr/BLOCKSZ;
    rtc->curitem = 0;
    GET_COUNTER(&num_items, rtc->curnode);
    if(num_items > 0)
      {
	GET_FIRSTITEM(item, rtc->curnode);
	memcpy(&rtc->curtpl, item+KEYSZ, sizeof(long));
      }
    else
      rtc->curtpl = UCHAR_MAX;
  } // end if
  else { // RTREE is empty
    rtc->curtpl = UINT_MAX;
    rtc->curBlkPtr = UINT_MAX;
    rtc->curitem = 0;
  }
  rtc->pathHead = rtc->pathTail = NULL;
  rtc->rtree = rtree;
  rtc->c_close = rtree_cursor_close;
  rtc->c_del = rtree_cursor_del;
  rtc->c_get = rtree_cursor_get;
  rtc->c_put = rtree_cursor_put;

  // init underlying tuple database
  if (tuple_cursor_init(rtc) !=0){
    myerr("rtree_cursor:tuple_cursor_init!");
  }

  rtc->dfsStack = new stack<cBufferType>;

  *rtcp = rtc;
  return 0;
}

int
rtree_cursor_close(RTreeCursor *rtc)
{
  /* delete all tuples marked in last block */
  tuple_cursor_close(rtc);
  free(rtc);
  return 0;
}

/* rtree_cursor_next_item(): Move the cursor to next item.
   param:
     rtc: underlying cursor;
     deleted: whether the result tuple is deleted (deleted=1)
   Return value:
     0: successful
     DB_NOTFOUND: End of the database
 */
int rtree_cursor_next_item(RTreeCursor *rtc, char &deleted)//cursor move to next item
{
  unsigned num_items;
  long nextlptr;
  pageno_t blk;
  unsigned ent;
  tupleno_t tpl;
  RTree *rtree = rtc->rtree;	
  //move to next item
  GET_COUNTER(&num_items, rtc->curnode);
  rtc->curitem++;

  if(rtc->curitem >= num_items)
    {
      GET_NEXTLPTR(&nextlptr, rtc->curnode);
      if(nextlptr == 0)
	{
	  /* the last entry in the entire index */
	  rtc->curBlkPtr = UINT_MAX;
	  rtc->curitem = UINT_MAX;
	  rtc->curtpl = UINT_MAX;
	  deleted = 0;
	  return DB_NOTFOUND;
	}
      /* otherwise, read the next block in the leaf chain */
      read_blk_by_ptr(rtree->index, nextlptr, rtc->curnode);
      rtc->curBlkPtr = nextlptr;
      rtc->curitem = 0;
    }

  char *curItem;
  GET_CURITEM(curItem, rtc);
  GET_DEL(deleted, curItem);

  dbg(printf("Cursor moved to B%d[%d(th)](%s)\n", rtc->curBlkPtr/BLOCKSZ, rtc->curitem, deleted?"DELETED":""));
  return 0;
};

/* moveToNextNonDel(): Move the cursor to next undeleted tuples
   Return Value:
     0: successful;
     DB_NOTFOUND: not found any more non-deleted tuples
 */
int moveToNextNonDel(RTreeCursor *rtc){
  RTree *rtree=rtc->rtree;
  while (1){
    char deleted;
    int rc = rtree_cursor_next_item(rtc, deleted);
    if (rc == DB_NOTFOUND)
      return DB_NOTFOUND;
    if (!deleted)
      return 0;
  }
  
};

/* rtree_cursor_get: Cursor Get.
   Description:
     If found, rtc->(curBlkPtr, curitem, curnode) should be set.
   flags:
     DB_FIRST:DB_NEXT:
       return all tuples;
     DB_SET:DB_NEXT_DUP: 
       return all tuples whose rectangle equal to key->data;
     DB_SET_RANGE: DB_NEXT_RANGE:
       return all tuples whose rectangle is bounded by key->data;  
       the key->data may be changed to the returned key value;
       
     For the last 2 cases (equality and containment), we have to perform a 
     DFS.  Every time the cursor is advanced (DB_NEXT/DB_NEXT_RANGE), we will
     move the cursor to "next" match tuple.  Therefore we have to use stack to
     record the intermediatee status in the whole DFS process.
 */

int
rtree_cursor_get(RTreeCursor *rtc, 
		 DBT *key,
		 DBT *dbt,
		 int flags)//modified by Richard 2002/1 to implement cursor get
{
  RTree *rtree = rtc->rtree;
  char *firstItem;
  unsigned num_items;
  char found = 0;
  char *s_item;
  char *sCurItem;
  Rect re;		
  int i_ret;
  int rc;
  unsigned i;
  char nodetype;
  char* item;
  cBufferType cbt;
  char deleted;

  dbg(printf("Entering rtree_cursor_get:\n"));

  //added by Richard 2002/4
  dbt->size = rtree->tplsz;
  dbt->data = rtree->current_tuple;
  key->size = sizeof(Rect);

  //end Richard
  switch(flags)
    {
    case DB_FIRST:
      dbg(printf("\tDB_FIRST\n"));
      /* Initialization: goto the first leaf node and read it */
      if (rtree->fb_ind == 1) { // RTREE is empty
	  rtc->curBlkPtr = UINT_MAX;
	  rtc->curitem = UINT_MAX;
	  rtc->curtpl = UINT_MAX;
	  return DB_NOTFOUND;
      }

      /* Get the first non-deleted item */
      deleted=1;
      int rc;
      read_blk_by_ptr(rtree->index, rtree->lptr, rtc->curnode);
      rtc->curBlkPtr = rtree->lptr;
      rtc->curitem = 0;
      GET_COUNTER(&num_items, rtc->curnode);
      if (num_items ==0){
	rtc->curBlkPtr = UINT_MAX;
	rtc->curitem = UINT_MAX;
	rtc->curtpl = UINT_MAX;	
	return DB_NOTFOUND;
      }
      GET_FIRSTITEM(firstItem, rtc->curnode);
      GET_DEL(deleted, firstItem);
      if (deleted){
	rc = moveToNextNonDel(rtc);
      }
      if (rc == DB_NOTFOUND) return DB_NOTFOUND; // couldn't find any non-deleted tuples



      /* get the item */
      GET_CURITEM(s_item, rtc);

      /* copy the item(string) to the key struct to be returned */
      key->size = sizeof(Rect);
      memcpy(key->data, s_item, key->size);


      /* set current tuple pointer rtc->curtpl */
      memcpy(&rtc->curtpl, s_item+KEYSZ, sizeof(pageno_t));

      dbg(printf("Cursor first set to B%d[%d(th)](%s)->T%d\n", rtc->curBlkPtr/BLOCKSZ, rtc->curitem, deleted?"DELETED":"", rtc->curtpl));

      /* read the tuple from underlying database ;
	 We should use DB_SET coz the leaf sequence in RTREE 
	 is different from that in Berkeley DB's RECNO database;
       */
      rc = tuple_cursor_get(rtc, dbt->data, DB_SET);
      switch (rc){
      case 0:
	break;
      case DB_NOTFOUND:
	return DB_NOTFOUND;
	break;
      default:
	myerr("rtree_cursor_get:DB_FIRST:tuple_get");
	break;
      } // end switch
      break;  
    case DB_NEXT:
      dbg(printf("\tDB_NEXT:\n"));
       /* move the cursor to next item */
      i_ret = moveToNextNonDel(rtc);
      if (i_ret == DB_NOTFOUND)
	return DB_NOTFOUND;

      /* get the number of items of current node */
      GET_COUNTER(&num_items,rtc->curnode);

      /* get the item */
      GET_CURITEM(s_item, rtc);

      /* copy the item(string) to the key struct to be returned */
      memcpy(key->data, s_item, key->size);
      key->size = sizeof(Rect);

      /* set current tuple pointer rtc->curtpl */
      memcpy(&rtc->curtpl, s_item+KEYSZ, sizeof(pageno_t));

      /* read the tuple from underlying database ;
	 We should use DB_SET coz the leaf sequence in RTREE 
	 is different from that in Berkeley DB's RECNO database;
       */
      rc = tuple_cursor_get(rtc, dbt->data, DB_SET);
 
      switch (rc){
      case DB_NOTFOUND:
	dbg(printf("\trtree_cursor_get:DB_NEXT:DB_NOTFOUND\n"));
	return DB_NOTFOUND;
	break;
      case 0:
	break;
      default:
	myerr("cursor_get: tuple_cursor_get!");
	break;
      }
      break;  
    case DB_SET:
      dbg(printf("\tDB_SET\n"));
      cbt.blkptr = 1 * BLOCKSZ;  // root node
      cbt.ent = 0;  // first entry
      rtc->dfsStack->push(cbt);
      rtc->dfsType = EQUALITY;

      // type is EQUALITY, search the first match
      // rtc->curtpl is set to the tuple no of the match
      rc = rtree_dfs(rtc->rtree,
		     rtc->dfsStack, 
		     (Rect*)key->data, 
		     rtc->curtpl, 
		     rtc->dfsType,
		     rtc->curnode); 
      switch (rc){
      case DB_NOTFOUND:
	rtc->curBlkPtr = UINT_MAX;
	rtc->curitem = UINT_MAX;
	return DB_NOTFOUND;
      case 0:
	cbt=rtc->dfsStack->top();
	rtc->curBlkPtr = cbt.blkptr;
	rtc->curitem = cbt.ent - 1; // cbt.ent is the next entry to test
	break;
      case -1:
      default:
	myerr("rtree_cursor_get:DB_SET");
      }// end switch rc

      /* read the tuple from underlying database ;
	 We should use DB_SET coz the leaf sequence in RTREE 
	 is different from that in Berkeley DB's RECNO database;
       */
      rc = tuple_cursor_get(rtc, dbt->data, DB_SET);
 
      switch (rc){
      case DB_NOTFOUND:
	dbg(printf("\trtree_cursor_get:DB_SET:DB_NOTFOUND\n"));
	return DB_NOTFOUND;
	break;
      case 0:
	break;
      default:
	myerr("cursor_get: tuple_cursor_get!");
	break;
      }
      break;  
    case DB_NEXT_DUP:
    case DB_NEXT_RANGE:
      dbg(printf("DB_NEXT_DUP/DB_NEXT_RANGE:\n"));
      // type is EQUALITY, search the next match
      // rtc->curtpl is set to the tuple no of the match
      rc = rtree_dfs(rtc->rtree,
		     rtc->dfsStack, 
		     (Rect*)key->data, 
		     rtc->curtpl, 
		     rtc->dfsType,
		     rtc->curnode); 
      switch (rc){
      case DB_NOTFOUND:
	rtc->curBlkPtr = UINT_MAX;
	rtc->curitem = UINT_MAX;
	return DB_NOTFOUND;
      case 0:
	cbt=rtc->dfsStack->top();
	rtc->curBlkPtr = cbt.blkptr;
	rtc->curitem = cbt.ent - 1; // cbt.ent is the next entry to test
	break;
      case -1:
      default:
	myerr("rtree_cursor_get:DB_SET");
      }// end switch rc

      
      
      /* read the tuple from underlying database ;
	 We should use DB_SET coz the leaf sequence in RTREE 
	 is different from that in Berkeley DB's RECNO database;
       */
      rc = tuple_cursor_get(rtc, dbt->data, DB_SET);
 
      switch (rc){
      case DB_NOTFOUND:
	dbg(printf("\trtree_cursor_get:DB_NEXT_DUP:DB_NOTFOUND\n"));
	return DB_NOTFOUND;
	break;
      case 0:
	break;
      default:
	myerr("cursor_get: tuple_cursor_get!");
	break;
      }
      break;  
    case DB_SET_RANGE:
      /* return the first tuple whose rectangle is bounded by key->data
       */
      dbg(printf("DB_SET_RANGE:"));
      cbt.blkptr = 1 * BLOCKSZ; // root
      cbt.ent = 0; // first entry
      rtc->dfsStack->push(cbt);
      rtc->dfsType = CONTAINMENT;
      // type is CONTAINMENT, search the first match
      // rtc->curtpl is set to the tuple no of the match
      rc = rtree_dfs(rtc->rtree,
		     rtc->dfsStack, 
		     (Rect*)key->data, 
		     rtc->curtpl, 
		     rtc->dfsType,
		     rtc->curnode); 
      switch (rc){
      case DB_NOTFOUND:
	rtc->curBlkPtr = UINT_MAX;
	rtc->curitem = UINT_MAX;
	return DB_NOTFOUND;
      case 0:
	cbt=rtc->dfsStack->top();
	rtc->curBlkPtr = cbt.blkptr;
	rtc->curitem = cbt.ent - 1; // cbt.ent is the next entry to test
	break;
      case -1:
      default:
	myerr("rtree_cursor_get:DB_SET_RANGE");
      }// end switch rc

      /* read the tuple from underlying database ;
	 We should use DB_SET coz the leaf sequence in RTREE 
	 is different from that in Berkeley DB's RECNO database;
       */
      rc = tuple_cursor_get(rtc, dbt->data, DB_SET);
 
      switch (rc){
      case DB_NOTFOUND:
	dbg(printf("\trtree_cursor_get:DB_SET_RANGE:DB_NOTFOUND\n"));
	return DB_NOTFOUND;
	break;
      case 0:
	break;
      default:
	myerr("cursor_get: tuple_cursor_get!");
	break;
      }
      break;  
    default:
      /* report error:  not-supported flags*/
      printf("RTREE Cursor Get Error: not all flags are supported in this version!\n");
      printf("You are using flag = %d\n",flags);
      return -1;
    }//end switch
    return 0;
}

/* cursor put:
   The flags can be only set to DB_CURRENT in this version.
   Return value: 0-success  
*/
int
rtree_cursor_put(RTreeCursor *rtc, 
		 DBT *key, 
		 DBT *dbt, 
		 int flags)
{

    
  RTree *rtree = rtc->rtree;
  switch (flags){
  case DB_CURRENT:
    /* write the tuple from record file */
      if (tuple_cursor_put(rtc, dbt->data, flags) == 0)
	return 0;
      else
	myerr("rtree_cursor_put: tuple_cursor_put!\n");
    break;
  default:
    /* not supported flags */
    printf("RTREE Cursor Put Error: not all flags are supported in this version!\n");
    printf("You are using code %d\n",flags);
    myerr("rtree_cursor_put");
    return -1;
  }//end switch
  

  return 0;
}





/*Richard 2003/5
  Cursor_Dele1te() :  deletes the key/data pair to which the cursor refers.

  Description: 
    Delete the item and call condense_rtree() to transmit the deletion upward.
    
  Optimization:
    We delete to-be-deleted items all by once per block.  Therefore first we record the current cursor position and delay the deletion, then we compare current cursor position with the previous one.  If they are of different block, we execute the deletion operation on the last block.
    In cursor_close(), we execute deletion on the very last block.
*/



int
rtree_cursor_del(RTreeCursor *rtc, int flags)
{
  RTree *rtree = rtc->rtree;
  long prevlptr, nextlptr;
  char nodetype;
  unsigned num_items;
  char *sItem;

  dbg(printf("deleting tuple pointed by cursor...\n"));

  if(rtc->curBlkPtr == UINT_MAX){
    return DB_NOTFOUND;
  }
  // set deleted mark
  GET_CURITEM(sItem, rtc);
  SET_DEL(sItem, 1);

  //write back
  write_blk_by_ptr(rtree->index, rtc->curBlkPtr, rtc->curnode);

  return 0;
}
