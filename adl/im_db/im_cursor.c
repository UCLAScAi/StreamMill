#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "im_db.h"

/*****************************************************************
 * IM_REL cursor operations on PAGED_ARRAY
 *****************************************************************/
int
im_rel_cursor(IM_REL *relp, IM_RELC **dbcp, u_int32_t flags)
{
    IM_RELC *dbc;
    int ret;

    if((ret = __my_calloc(1, sizeof(IM_RELC), &dbc)) != 0)
	return ret;

    dbc->cur = NULL;

    dbc->rel = relp;
    dbc->c_close = im_rel_cursor_close;
    dbc->c_del = im_rel_cursor_del;
    dbc->c_get = im_rel_cursor_get;
    dbc->c_put = im_rel_cursor_put;
    dbc->sameAs = im_rel_cursor_same;
    *dbcp = dbc;
    return 0;
}

int
im_rel_cursor_close(IM_RELC *dbc)
{
    free(dbc);
    return 0;
}

/* im_rel_cursor_get() has been rewritten. -- Haixun Wang */
int
im_rel_cursor_get(IM_RELC *dbc, DBT *key, DBT *data, u_int32_t flags)
{
    IM_REL *relp = dbc->rel;

    if(relp->head == NULL)
	return DB_NOTFOUND;

    switch(flags) {
    case DB_FIRST:
      {
	dbc->cur = relp->head;

	while (dbc->cur  &&	/* If we found one, */
	       (IS_OID_REL(relp->flags) && 
		TUPLE_DELETED(dbc->cur->status))) /* but it's marked deleted */
	  {
	    /* then we need to get the next one */
	    dbc->cur = dbc->cur->next;
	    /* unless we return to the beginning */
	    if (dbc->cur == relp->head)
	      dbc->cur = NULL;
	  }

	/* next time we will visit dbc->nextaccess */
	dbc->nextaccess = (dbc->cur && dbc->cur->next != relp->head)? dbc->cur->next : NULL;
      }
      break;

    case DB_NEXT:
      {
	//dbc->cur = dbc->nextaccess;
	if (!dbc->cur)
	  dbc->cur = relp->head;
	else if (dbc->cur->next == relp->head)
	  dbc->cur = NULL;
	else
	  dbc->cur = dbc->cur->next;
	
	while (dbc->cur  &&	/* If we found one, */
	       (IS_OID_REL(relp->flags) && 
		TUPLE_DELETED(dbc->cur->status))) /* but it's marked deleted */
	  {
	    /* then we need to get the next one */
	    dbc->cur = dbc->cur->next;
	    /* unless we return to the beginning */
	    if (dbc->cur == relp->head)
	      dbc->cur = NULL;
	  }

	/* next time we will visit dbc->nextaccess */
	dbc->nextaccess = (dbc->cur && dbc->cur->next != relp->head)? dbc->cur->next : NULL;
      }
      break;

    case DB_LAST:
      {
	dbc->cur = relp->head->prev;

	while (dbc->cur  &&	/* If we found one, */
	       (IS_OID_REL(relp->flags) && 
		TUPLE_DELETED(dbc->cur->status))) /* but it's marked deleted */
	  {
	    /* then we need to get the prev one */
	    dbc->cur = dbc->cur->prev;
	    /* unless we return to the beginning */
	    if (dbc->cur == relp->head->prev)
	      dbc->cur = NULL;
	  }

	/* next time we will visit dbc->nextaccess, which is the next
           tuple, provided it's not the last one */
	dbc->nextaccess = (dbc->cur && dbc->cur->next != relp->head->prev)? 
	  dbc->cur->prev : NULL;	
      }
      break;
    case DB_PREV:
      {
	dbc->cur = dbc->nextaccess;

	while (dbc->cur  &&	/* If we found one, */
	       (IS_OID_REL(relp->flags) && 
		TUPLE_DELETED(dbc->cur->status))) /* but it's marked deleted */
	  {
	    /* then we need to get the prev one */
	    dbc->cur = dbc->cur->prev;
	    /* unless we return to the beginning */
	    if (dbc->cur == relp->head->prev)
	      dbc->cur = NULL;
	  }

	/* next time we will visit dbc->nextaccess, which is the next
           tuple, provided it's not the last one */
	dbc->nextaccess = (dbc->cur && dbc->cur->next != relp->head->prev)? 
	  dbc->cur->prev : NULL;	
      }
      break;

    case DB_CURRENT: 
      {
	/*
	  if(dbc->cur == NULL)
	  return EINVAL;
	*/
	if(IS_OID_REL(relp->flags) && TUPLE_DELETED(dbc->cur->status))
	  __im_db_err(NULL, "Internal error!: cursor_get() - invalid current cursor for OID relations");
      }
      break;

    case DB_SET:
      {
	HashTable ht = relp->hidx;

	if (ht) {
	  /* use index */
	  dbc->cur = hash_find(relp->hidx,
			       key->data,
			       key->size);
	  
	  while (dbc->cur  &&	/* If we found one, */
		 (IS_OID_REL(relp->flags) && 
		  TUPLE_DELETED(dbc->cur->status)) )  /* but it's marked deleted */
	    {
	      /* then we need to get the next one */
	      dbc->cur = dbc->cur->hchain;
	    }

	  dbc->nextaccess = (dbc->cur)? dbc->cur->hchain : NULL;	

	} else {
	  /* no index */
	  dbc->cur = relp->head;

	  while (dbc->cur  &&	/* If we found one, */
		 ((IS_OID_REL(relp->flags) && 
		   TUPLE_DELETED(dbc->cur->status))) ||  /* but it's marked deleted, or */
		 (memcmp(dbc->cur->key.data+sizeof(size_t),
			 key->data, key->size) != 0)) /* key does not match */
		 
	    {
	      /* then we need to get the next one */
	      dbc->cur = dbc->cur->next;
	      /* unless we return to the beginning */
	      if (dbc->cur == relp->head)
		dbc->cur = NULL;
	    }

	  dbc->nextaccess = (dbc->cur && dbc->cur->next != relp->head)? dbc->cur->next : NULL;	
	}
      }
      break;

    case DB_GET_BOTH:
      {
	TLL *old_entry;
	HashTable ht = relp->hidx;

	if(ht != NULL)
	{
	  dbc->cur = hash_find(relp->hidx,
			       key->data,
			       key->size);
	  
	  while (dbc->cur  &&	/* If we found one, */
		 ((IS_OID_REL(relp->flags) && 
		   TUPLE_DELETED(dbc->cur->status)) || /* but it's marked deleted, or */
		  (memcmp(dbc->cur->data.data+sizeof(size_t), 
			  data->data, data->size) != 0))) /* data not match */
	    {
	      /* then we need to get the next one */
	      dbc->cur = dbc->cur->hchain;
	    }
	  // for DB_GET_BOTH_NEXT
	  dbc->nextaccess = (dbc->cur)? dbc->cur->hchain : NULL;	
	}
	else
	{
	    /* if there is no index, we have to start from the beginning */
	  dbc->cur = relp->head;

	  while (dbc->cur  &&	/* If we found one, */
		 ((IS_OID_REL(relp->flags) && 
		   TUPLE_DELETED(dbc->cur->status)) || /* but it's marked deleted, or */
		  (memcmp(dbc->cur->data.data+sizeof(size_t), 
			  data->data, data->size) != 0) || /* data not match, or */
		  (memcmp(dbc->cur->key.data+sizeof(size_t), 
			  key->data, key->size) != 0)))   /* key not match */

	    {
	      /* then we need to get the next one */
	      dbc->cur = dbc->cur->next;
	      /* unless we return to the beginning */
	      if (dbc->cur == relp->head)
		dbc->cur = NULL;
	    }
	  // for DB_GET_BOTH_NEXT
	  dbc->nextaccess = (dbc->cur && dbc->cur->next != relp->head)? dbc->cur->next : NULL;	
	}
      }
      break;
    case DB_NEXT_DUP:
      {
	HashTable ht = relp->hidx;

	dbc->cur = dbc->nextaccess;

	if (ht) {
	  
	  while (dbc->cur  &&	/* If we found one, */
		 (IS_OID_REL(relp->flags) && 
		  TUPLE_DELETED(dbc->cur->status)) )  /* but it's marked deleted */
	    {
	      /* then we need to get the next one */
	      dbc->cur = dbc->cur->hchain;
	    }

	  dbc->nextaccess = (dbc->cur)? dbc->cur->hchain : NULL;	

	} else {
	  /* no index */

	  while (dbc->cur  &&	/* If we found one, */
		 ((IS_OID_REL(relp->flags) && 
		   TUPLE_DELETED(dbc->cur->status))) ||  /* but it's marked deleted, or */
		 (memcmp(dbc->cur->key.data+sizeof(size_t),
			 key->data, key->size) != 0)) /* key does not match */
		 
	    {
	      /* then we need to get the next one */
	      dbc->cur = dbc->cur->next;
	      /* unless we return to the beginning */
	      if (dbc->cur == relp->head)
		dbc->cur = NULL;
	    }

	  dbc->nextaccess = (dbc->cur && dbc->cur->next != relp->head)? dbc->cur->next : NULL;	
	}
      }
      break;
    default:
      __im_db_err(NULL, "Internal error!: cursor_get() - mode not supported");
      return EINVAL;
    }

    if (dbc->cur == NULL)
      return DB_NOTFOUND;
    else {
      size_t rkey_size = *(size_t *)(dbc->cur->key.data);
      size_t rdata_size = *(size_t *)(dbc->cur->data.data);

      memcpy(relp->rkey_data, dbc->cur->key.data+sizeof(size_t), rkey_size);
      memcpy(relp->rdata_data, dbc->cur->data.data+sizeof(size_t), rdata_size);

      /* append the OID into rdata_data */
      memcpy(relp->rdata_data + rdata_size, &(dbc->cur), sizeof(int));

      key->data = (relp->rkey).data;
      key->size = rkey_size;
      data->data = (relp->rdata).data;
      data->size = rdata_size;
    }
    
    return 0;
}

int
im_rel_cursor_put(IM_RELC *dbc, DBT *key, DBT *data, u_int32_t flags)
{
    IM_REL *rel = dbc->rel;
    TLL *cur = dbc->cur;

    if(cur == NULL)
	return DB_NOTFOUND;
    switch(flags)
    {
    case DB_CURRENT:
	/* KEY is ignored, DATA overwrites current data */
	*(size_t *)cur->data.data = data->size;
	memcpy(cur->data.data+sizeof(size_t), data->data, data->size);
	break;
    default:
	__im_db_err(NULL, "Internal error!: cursor_get() - mode not supported");
	return EINVAL;
    }
    return 0;
}

int
im_rel_cursor_del(IM_RELC *dbc, u_int32_t flags)
{
    IM_REL *rel = dbc->rel;
    TLL *cur = dbc->cur;

    if(cur == NULL)
	return DB_NOTFOUND;
    if(IS_OID_REL(rel->flags))
    {
	/* I am an object-referenced relation */
	/* No deletion! Set tombstone */
	cur->status |= TUPLE_TOMBSTONE;
	for(cur = cur->next;
	    (cur != rel->head) && TUPLE_DELETED(cur->status);
	    cur = cur->next);
	if(cur == rel->head)
	    dbc->cur = NULL;
	else
	    dbc->cur = cur;
	/* Actually TOMBSTONE means no ACCESS!
	   All link pointer and hash chains are still in use */
    }
    else
    {
	/* I am a value-referenced relation */
	/* Deletion okay */
	TLL *next = cur->next, *prev = cur->prev;

	if(cur == rel->head)
	{
	    if(cur->next == rel->head)
	    {
		/* deleting the last tuple */
	      rel->head = NULL;
	    }
	    else
	    {
		rel->head = next;
		prev->next = next;
		next->prev = prev;
		//free(cur);
		freeBuffer(rel->buffer, cur);
		dbc->cur = NULL;
	    }
	} else {
	  prev->next = next;
	  next->prev = prev;
	  //free(cur);
	  freeBuffer(rel->buffer, cur);
	}

	// You forgot hash chain! --Haixun Wang
	if (rel->hidx) {
	  TLL *old_entry;
	  char *keydata = cur->key.data+sizeof(size_t);
	  int keysize = *(int*)cur->key.data;

	  old_entry = hash_find(rel->hidx, keydata, keysize);

	  if (!old_entry) {
	    printf("cannot find key %d in the hash index\n", *(int*)keydata);
	    hash_display(rel->hidx);
	    exit(1);
	  }
	  
	  if (old_entry == cur) {
	    // remove it from hash
	    if (cur->hchain) {
	      hash_replace(rel->hidx, 
			   cur->hchain->key.data+sizeof(size_t), keysize, 
			   cur->hchain);
			   
	    } else {
	      if (!hash_delete(rel->hidx, keydata, keysize)) {
		printf("cannot delete tuple in the hash index \n");
		exit(1);
	      }
	      // printf("delete key %d it table %d\n", *(int*)keydata, rel->hidx);
	    }
	  } else {
	    while (old_entry->hchain && old_entry->hchain != cur) {
	      old_entry = old_entry->hchain;
	    }
	    if (!old_entry) {
	      /* we have a problem here */
	      printf("cannot find tuple in the hash chain \n");
	      exit(1);
	    }
	    old_entry->hchain = cur->hchain;
	  }
	}
    }
    return 0;
}


int im_rel_cursor_same(IM_RELC *self, IM_RELC *other){
  return self->cur == other->cur;
}






