#include <sys/types.h>
#include <stdio.h>
#include <db.h>

int main()
{
  int rc = 0;

  // initialization
  // decs
  // exp
 exit:
  return rc;
}

/* translation of declarations

   table t(a int primary key, b int, c char(15));
*/

DB *t;
DBT t_key, t_data;
typedef struct t_tuple_ {
  int a;
  int b;
  char c[15];
} t_tuple;

if ((rc = db_create(&t, NULL, 0)) != 0) {
  fprintf(stderr, "db_create: %s\n", db_strerror(rc));
  goto exit;
}

if ((rc = t->open(dbp, "t", NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
  dbp->err(t, ret, "%s", "t");
  goto exit;
}

/* translation of UNION box to C code

     [SELECT1]    
   UNION 
     [SELECT2]
*/

for (;;) {
  switch(stage) {
  case 0:
    // SELECT1
    if (*resultp==NULLP)
      stage++;
    else 
      goto next;
    break;
  case 1:
    // SELECT2
    goto next;
  }
}
next:
/* translation of UPDATE box to C code

   UPDATE t 
   SET a=100
   WHERE C(a,b);
*/
{
  DBC *t_cursor;
  rc = t->cursor(t, NULL, &t_cursor, 0);

  if (rc) {
    dbp->err(t, rc, "DB->cursor");
    goto exit;
  }

  for(;;)
    {
      // get tuple from t 
      rc = t_cursor->c_get(t_cursor, t_key, t_data, DB_NEXT);
      if (rc) {
	if (rc == DB_NOTFOUND) {
	  rc = t_cursor->close(t_cursor);
	  if (rc) {
	    t->err(t, rc, "%s", "t");
	    goto exit;
	  }
	} else {
	  t->err(t, rc, "%s", "t");
	  goto exit;
	}
      }

      /*  printf("db: %s: key retrieved: data was %s.\n",
	  (char *)t_key.data, (char *)t_data.data);
      */
      if (C(tuple.a,tuple.b)) {
	tuple.a = 100;
	*(int*)(&t_key.data[0]) = 100;
	rc = t_cursor->c_put(t_cursor, t_key, t_data, DB_CURRENT);
	if (rc) {
	  t->err(t, rc, "%s", "t");
	  goto exit;
	}
      }
    }
}

/* translation of DEL box to C code

   DELETE FROM t 
   WHERE C(a,b);
*/
{
  DBC *t_cursor;
  rc = t->cursor(t, NULL, &t_cursor, 0);

  if (rc) {
    dbp->err(t, rc, "DB->cursor");
    goto exit;
  }

  for(;;)
    {
      // get tuple from t 
      rc = t_cursor->c_get(t_cursor, t_key, t_data, DB_NEXT);
      if (rc) {
	if (rc == DB_NOTFOUND) {
	  rc = t_cursor->close(t_cursor);
	  if (rc) {
	    t->err(t, rc, "%s", "t");
	    goto exit;
	  }
	  goto next;
	} else {
	  t->err(t, rc, "%s", "t");
	  goto exit;
	}
      }

      /*  printf("db: %s: key retrieved: data was %s.\n",
	  (char *)t_key.data, (char *)t_data.data);
      */
      if (C(tuple.a,tuple.b)) {
	rc = t_cursor->c_del(t_cursor, 0);
	if (rc) {
	  t->err(t, rc, "%s", "t");
	  goto exit;
	}
      }
    }
 next:
}

/* translation of INS box to C code

   INSERT INTO t 
    SELECT A(a) a, B(x) b
    FROM g1, g2;
    WHERE C(a,x);
*/

{
  do {
    // select
    if (*resultp==SUCCESS) {
      insertTuple();
    }
  } while (*resultp==SUCCESS);
}

/* translation of SEL box to C code

   SELECT A(a) a, B(x) b
   FROM g1, g2;
   WHERE C(a,x);
*/

typedef struct {
  int a;
  int b;
} main_tuple_t;

typedef struct {
  int stage;			// runtime stage
  g1_stack_t *g1_stack;		// stack of subnodes
  g1_tuple_t g1_tuple;
  g2_stack_t *g2_stack;		// stack of subnodes
  g2_tuple_t g2_tuple;
} main_stack_t;

int main_gettuple(int depth, main_stack_t* &stack, main_tuple_t *tuple, int *resultp)
{
  int rc;

  if (!stack) {
    stack = hashStack(sizeof(main_stack_t),depth);
    stack->stage=0;
    stack->g1_stack=NULLP;
    stack->g2_stack=NULLP;
  }
  for (;;) {
    switch (stack->stage) {
    case 0:
      rc = g1_gettuple(depth+1, stack->g1_stack, &(stack->g1_tuple), resultp);
      if (*resultp==NULLP) {
	deallocate(stack);
	goto exit;
      } else {
	stack->stage++;
      }
      break;
    case 1:
      rc = g2_gettuple(depth+1, stack->g2_stack, &(stack->g2_tuple), resultp);
      if (*resultp==NULLP) {
	stack->stage--;
      } else {
	if (C(stack->g1_tuple.a, stack->g2_tuple.x)) {
	  tuple->a = A(x);
	  tuple->b = B(y);
	  goto exit;
	}
      }
    }
  }
 exit:
  return rc;
}
/***************************************************************************
f is an aggregate:

select a A, b B, f(x,y) C
from g
group by a,b

***************************************************************************/
typedef struct {
  int A;
  int B;
  int C;
} main_tuple_t;

typedef struct {
  int stage;			// runtime stage
  f_stack_t *fstack;		// stack of aggregate
  f_tuple_t ftuple;
  g_stack_t *gstack;
  g_tuple_t gtuple;
} main_stack_t;

int main_gettuple(int depth, main_stack_t* &stack, main_tuple_t *tuple, int *resultp)
{
  int rc;

  if (stack==NULLP) {
    stack = hashStack(sizeof(main_stack_t), depth);
    stack->fstack=NULLP;
    stack->gstack=NULLP;
    stack->stage=0;
  }

  for (;;) {
    switch (stack->stage) {     
    case 0:
      rc = g_gettuple(depth+1, stack->gstack, &(stack->gtuple), resultp);
      if (*resultp == NULLP) {
	stack->stage=2;
      } else {
	stack->fstack = hashStack(sizeof(f_stack_t), depth, g_result.a, g_result.b);
	stack->stage=1;
      }
      break;
    case 1:
      f(depth+1, 0, stack->fstack, stack->gtuple.x, stack->gtuple.y, &(stack->ftuple), resultp);
      if (*resultp==SUCCESS) {
	tuple->A = stack->gtuple.a;
	tuple->B = stack->gtuple.b;
	tuple->C = stack->ftuple; // ??
	goto exit;
      }
      else
	stack->stage=0;
      break;
    case 2:
      getNextGroup(depth,result)=>(stack->a,stack->b,stack->fstack);
      if (*resultp==NULLP) {
	deallocate(stack);
	goto exit;
      } else
	stack->stage=3;
      break;
    case 3:
      f(depth+1, TERMINATE, stack->fstack, _, _, &(stack->ftuple), resultp);
      if (*resultp==SUCCESS) {
	tuple->A = stack->gtuple.a;
	tuple->B = stack->gtuple.b;
	tuple->C = stack->ftuple; // ??
	goto exit;
      } else
	stack->stage=2;
      break;
    }
  };

 exit:
  return rc;
}

/*
aggr f(x,y):t
{
    table a(i: int);

    init  : init_exp
    multi : multi_exp
    terminate: term_exp
}
*/

typedef struct {
  table_t *a;

  init_stack_t *initstack;
  multi_stack_t *multistack;
  term_stack_t *termstack;
} f_stack_t;

typedef struct {
  f_stack_t *prev;		// previous stack
} term_stack_t;

typedef struct {
  
} f_tuple_t;

f(int depth, int flag, f_stack_t *stack, int x, int y, f_tuple_t *tuple, int *resultp)
{
  if (termflag == 1) {
    term_getTuple(depth);
  } 
    else if (!stack) init_exp(depth) => stack;
    else multi_exp(depth) => stack;
    return r; 
}








