/*
 * im_ll_rel.c: for hash indexing, purely tuple-by-tuple linked list
 *		easy for clustering hash buckets
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "im_db.h"

int
im_rel_create(IM_REL **relpp,	/* result */
	      IM_DB_ENV *dbenv,	/* currently NULL */
	      IM_DBTYPE type,   /* for later extensions */
	      u_int32_t flags)	/* currently NULL */
{
    IM_REL *relp;
    int ret;

    /* Allocate the IM_REL */
    if((ret = __my_calloc(1, sizeof(*relp), &relp)) != 0)
	return ret;

    /* Initialize the environement */
    if(dbenv == NULL)
    {
	if((ret = im_db_env_create(&dbenv, 0)) != 0)
	    return ret;
    }
    else
	relp->dbenv = dbenv;

    /* the type */
    relp->type = type;
    relp->flags = flags;

    /* SC.INDEXED? */
    if((flags & IM_REL_INDEXED) == IM_REL_INDEXED)
    {
	relp->hidx = hash_new();
	if(relp->hidx == NULL)
	    return ENOMEM;
    }

    /* set function pointers */
    switch(type)
    {
    case IM_LINKEDLIST:
	relp->cursor = im_rel_cursor;
	relp->open = im_rel_open;
	relp->close = im_rel_close;
	relp->get = im_rel_get;
	relp->put = im_rel_put;
	relp->del = im_rel_del;

	relp->print = im_rel_print;

	relp->rkey.data = relp->rkey_data; /* Haixun Wang */
	relp->rdata.data = relp->rdata_data; /* Haixun Wang */

	break;
    default:
	return EINVAL;
    }

    *relpp = relp;
    return 0;
}

/* this is actually not necessary, since right now, one in-memory
   database can only hold one table --Haixun Wang*/
int
im_rel_open(IM_REL *relp, char *name, u_int32_t flags)
{
    int ret;

    /* store name */
    memcpy(relp->name, name, 78);
    relp->name[79]='\0';
/*      if((ret = __my_strdup(name, &relp->name)) != 0) */
/*  	return ret; */

    /* initialize storage */
    relp->head = NULL;

    return 0;
}

int
im_rel_close(IM_REL *rel, u_int32_t flags)
{
    TLL *cur, *next;
    IM_REL *rp, *prev_rp;

#if 0
    /* release space held by the tuples */
    cur = rel->head;
    if(cur != NULL)
    {
	cur = cur->next;
	while(cur != rel->head)
	{
	    next = cur->next;
	    free(cur);
	    cur = next;
	}
	free(rel->head);
    }
#else
    if (rel->buffer) {
      deleteBuffer(rel->buffer);
    }
#endif

    /* release space held by hash index */
    if(rel->hidx != NULL)
	hash_die(rel->hidx);

#if 0
    /* for BerkeleyDB emulation, return area of REL->get and RELC->c_get */
    if((rel->rkey).data)
	free((rel->rkey).data);
    if((rel->rdata).data)
	free((rel->rdata).data);
#endif
    free(rel); /* Seems like this was not being freed: Hetal & Yijian */
    return 0;
}

int
im_rel_get(IM_REL *rel, DBT *key, DBT *data, u_int32_t flags)
{
    TLL *cur = rel->head;
    HashTable ht = rel->hidx;
    void *rkey_data, *rdata_data;
    size_t rkey_size, rdata_size;
    int ret;

    if(cur == NULL)
	return DB_NOTFOUND;

    switch(flags)
    {
    case 0:
    case DB_GET_OIDOFKEY:
      /* DB_GET_OIDOFKEY is not in use any more. Everytime a tuple is
         returned, we append the tuple's OID to the data part.  --
         Haixun Wang */
    {
	if(ht != NULL)
	{
	    /* if there is a hash index, use it */

	    if((cur = hash_find(rel->hidx, key->data, key->size)) == NULL)
		/* if not found */
		return DB_NOTFOUND;

	    /* else cur is at the right place */
	    if(memcmp(cur->key.data+sizeof(size_t), key->data, key->size) != 0)
	    {
		__im_db_err(NULL, "Internal error!: im_rel_get() - hash index pointing to wrong place.");
	    }
	    if(IS_OID_REL(rel->flags))
	    {
		while(1)
		{
		    if(!TUPLE_DELETED(cur->status))
			break;
		    else
			/* if TOMBSTONED, go to next in hash chain */
			cur = cur->hchain;
		    if(cur == NULL)
			return DB_NOTFOUND;
		}
	    }
	    /* return DATA */
	    if(flags == DB_GET_OIDOFKEY)
	    {
		if(!IS_OID_REL(rel->flags))
		    return EINVAL;

		rdata_data = (rel->rdata).data;
		rdata_size = sizeof(void *);

		if(rdata_data)
		    free(rdata_data);
		if((ret = __my_malloc(rdata_size, &rdata_data)) != 0)
		    return ENOMEM;
		*(void **)rdata_data = cur;
	    }
	    else
	    {
	      //		rdata_data = (rel->rdata).data;
		rdata_size = *(size_t *)(cur->data.data);
		/*	if(rdata_data)
		    free(rdata_data);
		if((ret = __my_malloc(rdata_size, &rdata_data)) != 0)
		return ENOMEM;
		*/
		memcpy((rel->rdata).data, cur->data.data+sizeof(size_t), rdata_size);
	    }
	    data->data = (rel->rdata).data;//rdata_data;
	    data->size = rdata_size;
	    return 0;
	}
	else
	{
	    /* if there is no index, have to start from the beginning */
	    cur = rel->head;
	    while(1)
	    {
		if(!(IS_OID_REL(rel->flags) && TUPLE_DELETED(cur->status)) &&
		   memcmp(cur->key.data+sizeof(size_t), key->data, key->size) == 0)
		{
		    /* well, it qualifies */
		    /* printf("Matched.\n"); */
		    /* return DATA */
		    if(flags == DB_GET_OIDOFKEY)
		    {
		      //rdata_data = (rel->rdata).data;
			rdata_size = sizeof(void *);
			/*
			if(rdata_data)
			    free(rdata_data);
			if((ret = __my_malloc(rdata_size, &rdata_data)) != 0)
			    return ENOMEM;
			*/
			*(void **)((rel->rdata).data) = cur;
		    }
		    else
		    {
			/* return DATA */
		      // rdata_data = (rel->rdata).data;
			rdata_size = *(size_t *)(cur->data.data);
			/* if(rdata_data)
			    free(rdata_data);
			if((ret = __my_malloc(rdata_size, &rdata_data)) != 0)
			    return ENOMEM;
			*/
			memcpy((rel->rdata).data, cur->data.data+sizeof(size_t), rdata_size);
		    }
		    data->data = (rel->rdata).data;//rdata_data;
		    data->size = rdata_size;
		    return 0;
		}
		cur = cur->next;

		if(cur == rel->head)
		    /* meet the head again, end */
		    break;
	    }
	    return DB_NOTFOUND;
	}
	/* no break need here */
    }

    case DB_GET_BOTH:
	if(ht != NULL)
	{
	    /* if there is a hash index, use it */

	    if((cur = hash_find(rel->hidx, key->data, key->size)) == NULL)
		/* if not found */
		return DB_NOTFOUND;

	    /* else cur is at the right key place, check the hash chain */
	    while(cur)
	    {
		if(memcmp(cur->key.data+sizeof(size_t), key->data, key->size) != 0)
		{
		    __im_db_err(NULL, "Internal error!: im_rel_get() - hash index pointing to wrong place.");
		}
		if(!(IS_OID_REL(rel->flags) && TUPLE_DELETED(cur->status)) &&
		   memcmp(cur->data.data+sizeof(size_t), data->data, data->size) == 0)
		{
		    /* well, it qualifies */
		    /* printf("Matched.\n"); */
		    return 0;
		}
		cur = cur->hchain;
	    }
	    return DB_NOTFOUND;
	}
	else
	{
	    /* if there is no index, have to start from the beginning */
	    cur = rel->head;
	    while(1)
	    {
		if(!(IS_OID_REL(rel->flags) && TUPLE_DELETED(cur->status)) &&
		   memcmp(cur->key.data+sizeof(size_t), key->data, key->size) == 0 &&
		   memcmp(cur->data.data+sizeof(size_t), data->data, data->size) == 0)
		{
		    /* well, it qualifies */
		    /* printf("Matched.\n"); */
		    return 0;
		}
		cur = cur->next;

		if(cur == rel->head)
		    /* meet the head again, end */
		    break;
	    }
	    return DB_NOTFOUND;
	}
	break;
    case DB_SET_RECNO:
    {
	register db_recno_t i=1, recno = *(db_recno_t *)key->data;

	while(1)
	{
	    if(i == recno)
	    {
	      //rkey_data = (rel->rkey).data;
		rkey_size = *(size_t *)(cur->key.data);
		//rdata_data = (rel->rdata).data;
		rdata_size = *(size_t *)(cur->data.data);

		/*		
                if(rkey_data)
		    free(rkey_data);
		if(rdata_data)
		    free(rdata_data);
		if((ret = __my_malloc(rkey_size, &rkey_data)) != 0)
		    return ENOMEM;
		if((ret = __my_malloc(rdata_size, &rdata_data)) != 0)
		    return ENOMEM;
		*/
		memcpy((rel->rkey).data, cur->key.data+sizeof(size_t), rkey_size);
		memcpy((rel->rdata).data, cur->data.data+sizeof(size_t), rdata_size);

		/* append the OID into rdata_data. --Haixun Wang */
		memcpy((rel->rdata).data + rdata_size, &cur, sizeof(int));

		key->data = (rel->rkey).data; //rkey_data;
		key->size = rkey_size;
		data->data = (rel->rdata).data; //rdata_data;
		data->size = rdata_size;
		return 0;
	    }
	    if(!(IS_OID_REL(rel->flags) && TUPLE_DELETED(cur->status)))
		i++;
	    cur = cur->next;

	    if(cur == rel->head)
		/* meet the head again, end */
		break;
	}
	/* otherwise, the record# is too big, no such record# */
	return DB_NOTFOUND;
    }
#if 0
    case DB_GET_OID:
	cur = *(void **)key->data;
	{
	    void *rkey_data = (rel->rkey).data;
	    void *rdata_data = (rel->rdata).data;

	    rkey_size = *(size_t *)(cur->key.data);
	    rdata_size = *(size_t *)(cur->data.data);
	    if(rkey_data)
		free(rkey_data);
	    if(rdata_data)
		free(rdata_data);
	    if((ret = __my_malloc(rkey_size, &rkey_data)) != 0)
		return ENOMEM;
	    if((ret = __my_malloc(rdata_size, &rdata_data)) != 0)
		return ENOMEM;
	    memcpy(rkey_data, cur->key.data+sizeof(size_t), rkey_size);
	    memcpy(rdata_data, cur->data.data+sizeof(size_t), rdata_size);

	    key->data = rkey_data;
	    key->size = rkey_size;
	    data->data = rdata_data;
	    data->size = rdata_size;
	    return 0;
	}
#endif
    default:
	__im_db_err(NULL, "Internal error!: im_rel_get() - mode not supported");
	return EINVAL;
    }
}

int
im_rel_put(IM_REL *rel, DBT *key, DBT *data, u_int32_t flags)
{
    int ret;
    TLL *cur, *head = rel->head;
    HashTable ht;

    if(flags == 0 && head != NULL)	/* Overwritten */
    {
	int ovwt_flag = 0;
	TLL *hprev = head->prev;

	cur = head;
	while(1)
	{
	    if(!(IS_OID_REL(rel->flags) && TUPLE_DELETED(cur->status)) &&
	       memcmp(key->data, cur->key.data+sizeof(size_t), key->size) == 0)
	    {
		ovwt_flag = 1;
		break;
	    }
	    if(cur == hprev)
		break;
	    cur = cur->next;
	}
	if(ovwt_flag)
	{
	    /* free old data */
	    free(cur->data.data);
	    /* allocate new data */
	    if((ret = __my_malloc(data->size+sizeof(size_t), &cur->data.data)) != 0)
		return ret;
	    /* set new data */
	    *(size_t *)(cur->data.data) = data->size;
	    memcpy(cur->data.data+sizeof(size_t), data->data, data->size);

	    return 0;
	}
	/* else DB_APPEND */
    }

#if 0
    /* allocate the TLL */
    if((ret = __my_calloc(1, sizeof(TLL), &cur)) != 0)
	return ret;
    /* allocate the key */
    if((ret = __my_malloc(key->size+sizeof(size_t), &cur->key.data)) != 0)
	return ret;
    /* allocate the data */
    if((ret = __my_malloc(data->size+sizeof(size_t), &cur->data.data)) != 0)
	return ret;
#else
    if (rel->buffer == (buffer_t)0) {
      rel->buffer = newBuffer(sizeof(TLL)+ key->size+sizeof(size_t)+data->size+sizeof(size_t));
    }
    ret = allocateBuffer(rel->buffer, (char**)(&cur));
    if (ret) {
      printf("err in allocate Buffer in im_rel.c\n");
      exit(1);
    }

    cur->key.data = (char*)cur + sizeof(TLL);
    cur->data.data = (char*)cur + sizeof(TLL) + key->size + sizeof(size_t);
#endif

    /* set KEY & DATA: prepending SIZE to each field */
    *(size_t *)(cur->key.data) = key->size;
    memcpy(cur->key.data+sizeof(size_t), key->data, key->size);
    *(size_t *)(cur->data.data) = data->size;
    memcpy(cur->data.data+sizeof(size_t), data->data, data->size);
    /* misc */
    cur->status = 0;
    cur->hchain = NULL;

    /* set double linkage */
    if(head == NULL)
    {
	/* the 1st insertion, let head point to itself */
	cur->prev = cur;
	cur->next = cur;
	rel->head = cur;
    }
    else
    {
	TLL *hprev = head->prev;

	if(hprev == head)
	{
	    /* the 2nd insertion, head is pointing to itself */
	    cur->prev = cur->next = head;
	    head->next = head->prev = cur;
	}
	else
	{
	    /* All other cases: append it to the end of table */
	    cur->prev = hprev, hprev->next = cur;
	    cur->next = head, head->prev = cur;
	}
    }

    /* maintain the hash index and hash chain */
    ht = rel->hidx;
    if(ht != NULL)
    {
	TLL *old_entry;

	/* if the in-memory relation is hash indexed */
	if((old_entry = hash_find(ht, key->data, key->size)) != NULL)
	{
	    /* if the key exists in the hash index,
	       cluster it in the bucket */
#if 1
	  /* insert at the beginning of the hash_chain */
	  hash_replace(ht, cur->key.data+sizeof(size_t), key->size, cur);
	  cur->hchain = old_entry;
#else
	  /* insert at the end of the hash_chain */
	  while(old_entry->hchain)
	    old_entry = old_entry->hchain;
	  old_entry->hchain = cur;
	  cur->hchain = NULL;
#endif
	}
	else
	    /*
	     * if the key doesn't exist in the hash index,
	     * insert it into hash index.
	     *
	     * Remember the hash implementation points to the key directly,
	     * so I have to strdup it.
	     */

	  // since we already creates new space for the tuple, we don't need to strdup!!!
	  //  --- Haixun
	  // here : memcpy(cur->key.data+sizeof(size_t), key->data, key->size);
	  //
	  //hash_insert(ht, strdup(key->data), key->size, cur);
	  hash_insert(ht, (cur->key.data+sizeof(size_t)), key->size, cur);
	//	printf("Hash %d insert %d, %d\n", ht, *(int*)(cur->key.data+sizeof(size_t)), key->size);
    }
    else
    {
	/* If there is a hash index, maintain the hash chain via the index.
	   Otherwise, have to maintain it by finding the last duplicate key.

	   It's interesting to remark that tombstoned tuples do not matter
	   here because they can be part of the chain even after they are
	   tombstoned.

	   Therefore, here it is enough to find the most immediate
	   previous duplicate key. */
	TLL *p = cur->prev;

	if(cur != rel->head)
	{
	    /* a head-only relation has no hash chain at all */

	    /* otherwise */
            /* Removing this while(1) below changed nothing as far as the test
               suit is concerned and gives dramatic improvements when using 
               large memory tables (especially large windows) Yijian - Hetal*/
	    while(1)
	    {
		if(memcmp(p->key.data+sizeof(size_t), key->data, key->size) == 0)
		{
		    p->hchain = cur;
		    break;
		}
		if(p == rel->head)
		    break;
		p = p->prev;
	    }
	}
    }

    return 0;
}

int
im_rel_del(IM_REL *rel, DBT *key, u_int32_t flags)
{
    register int i;
    TLL *cur = rel->head;
    TLL *tofree = NULL;
    int ret=0, head_deleted = 0;

    if(cur == NULL)
	return DB_NOTFOUND;

    /* start from head */
    while(1)
    {
	if(memcmp(cur->key.data+sizeof(size_t), key->data, key->size) == 0)
	{
	    /* well, it qualifies */
	    if(IS_OID_REL(rel->flags))
	    {
		/* I am an object-referenced relation */
		/* No deletion. Set tombstone */
		cur->status |= TUPLE_TOMBSTONE;
		/* Actually TOMBSTONE means no ACCESS!
		   All link pointer and hash chains are still in use */
	    }
	    else
	    {
		/* I am a value-referenced relation */
		/* Deletion okay */
		if(cur == rel->head)
		    head_deleted = 1;
		else
		{
		    cur->prev->next = cur->next;
		    cur->next->prev = cur->prev;
                    tofree = cur;
		}
	    }
	    ret++;
	}
	cur = cur->next;
        if(tofree) {
          //free(tofree); //somehow this freeing causes segfault, Hetal
          tofree = NULL;
        }

	if(cur == rel->head)
	    /* meet the head again, end */
	    break;
    }

    if(head_deleted)
    {
	TLL *head = rel->head;
	if(head->next == head)
	    rel->head = NULL;
	else
	{
	    rel->head = head->next;
	    head->prev->next = rel->head;
	    rel->head->prev = head->prev;
	}
	free(head);
    }

    if(ret > 0)
	return 0;
    return DB_NOTFOUND;
}






