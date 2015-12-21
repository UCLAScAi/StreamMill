/*
select a,b, aggr1(x,y), aggr2(x)
from t
group by a,b

aggregate aggr1(x int, y int)
{
table current(c int);
table t(x int);
init: { 
	insert into current values(1);
      }
iterate: {
	    update current set c=c+1;
            insert into t select x from current where c mod 3 =0;
	  }
terminate: { insert into return 
	    select * from t union all
	    select aggr1(x) from t;}
}
*/

struct aggr1_status {
  DB *current;
  DB *t;
  DB *ret;
};

aggr_init(aggr_status *&status, x, y)
{
  DBopen (status->current);
  DBopen (status->t);
  DBopen (status->ret);
  /* init routine */
}

aggr_iterate(aggr_status *status, x, y)
{

}

aggr_terminate(aggr_staus *stauts)
{
  /* term routine */
}




DB *hashtable;
int index_aggr=0;

while (rc==0)
{
  /* ... */

  char key[MAX_STR_LEN];
  int terminating=0;
  int first_entry_aggr1=1;
  int first_entry_aggr2=1;
  struct gb_status {
    aggr1_status aggr1;
    aggr2_status aggr2;
    /* ... */
  };
  struct gb_status *gbstatus;

  while (index_aggr>=0 && index_aggr <3) {
    switch(index_aggr) {
    case 0: 
      if (terminating == 0) {
	/* source  */
	getCursor2C(first_entry_x, "");
	/* compute all aggrs */
	if (rc==0) {
	  gbstatus = (struct gb_status*)0;
	  sprintf(key, "%s%s",a,b);
	  rc = hash_get(key, &gbstatus);
	  if (rc==DB_NOTFOUND) {
	    /* begin of create gbstatus */
	    gbstatus = (struct gb_stauts *)malloc(sizeof(*gbstatus));
	    gbstatus->aggr1 = (struct aggr1_status*)malloc(sizeof(*gbstatus->aggr1));
	    gbstatus->aggr2 = (struct aggr2_status*)malloc(sizeof(*gbstatus->aggr2));

	    /* end of create gbstatus */
	    aggr_init(gbstatus->aggr1,x,y);
	    aggr_init(gbstatus->aggr2,x,y);

	    hash_put(key, gbstatus);

	  } else if (rc==0) {
	    /* begin of clear ret table */
	    DBclear(gbstatus->aggr1->ret);
	    DBclear(gbstatus->aggr2->ret);
	    /* end of clear ret table */
	    aggr_iterate(gbstatus->aggr1,x,y);
	    aggr_iterate(gbstatus->aggr2,x,y);
	  } else {
	    goto exit;
	  }
	} else if (rc==DB_NOTFOUND) {
	  terminating = 1;
	} else 
	  goto exit;
      }
      if (terminating == 1) {
	rc = DB_get(key, &gbstatus);
	/* assign key */
	if (rc==0) {
	  aggr_terminate(gbstaus->aggr1);
	  aggr_terminate(gbstaus->aggr2);
	} else if (rc==DB_NOTFOUND) {
	} else
	  goto exit;
      }
      break;
    case 1: /* first aggr */
      rc = getTuple(gbstatus->aggr1->ret);
      break;
    case 2: /* second aggr */
      rc = getTuple(gbstatus->aggr2->ret);
      /* ... */
      break;
    }
    if (rc==0) {
      index_aggr++;
    }
    if (rc=DB_NOTFOUND) {
      index_aggr--;
      if (terminating && index_aggr==1) {
	/* close all status */
	/* --- for aggr1 */
	DBclose (gbstatus->aggr1->current);
	DBclose (gbstatus->aggr1->t);
	DBclose (gbstatus->aggr1->ret);
	free(gbstatus->aggr1);
	/* --- for aggr2 */
	/* ... */
	free(gbstatus);
      }
    }
  }
  if (rc==0) {
    index_aggr--;
  }
  /* ... */
}




