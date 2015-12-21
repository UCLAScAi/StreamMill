#include <stdio.h>
#include <sql/adl_sys.h>
#include <assert.h>
#include <sql/table.h>
#include <sql/symbol.h>


#define TABSIZE 127


//I could not include SMLog and use the function 
//FIXME so I had to add this function here
//#include <SMLog.h>
#include <stdarg.h>
#define SMLOG_LEVEL 12
#define SMLOG_FILENAME "./SMLOG.log"
void SMLOG(int level, const char* Format, ...)
{
	if (level > SMLOG_LEVEL)
		return;

/*	va_list Arguments;
	va_start(Arguments, Format);
	printf( Format, Arguments);
	printf("\n");
	va_end(Arguments);
*/	
	//   since SM redirects stdout to different files
	//   it might be kind of risky to prompt everything
	//   into it. so you may have to use following pices 
	//   of code to write them into a file.
	
	FILE *fp;
	if((fp=fopen(SMLOG_FILENAME, "a")) == NULL) 
	{
		printf("Cannot open file: %s \n", SMLOG_FILENAME );
		return;
	}
	//FIXME: add date and time information
	//FIXME: write the log in bunches
	va_list Arguments;
	va_start(Arguments, Format);
	vfprintf(fp, Format, Arguments);
	fprintf(fp, "\n");
	va_end(Arguments);

	fclose(fp);
}




typedef struct binder_ *binder;
struct binder_ 
{
  void *key; 
  void *value; 
  binder next; 
  void *prevtop;
};

struct TAB_table_ {
  binder table[TABSIZE];
  void *top;
};


static binder Binder(void *key, void *value, binder next, void *prevtop)
{
  binder b = (binder)ntMalloc(sizeof(*b));
  b->key = key; b->value=value; b->next=next; b->prevtop = prevtop; 
  return b;
}

TAB_table TAB_empty(void)
{ 
  TAB_table t = (TAB_table)ntMalloc(sizeof(*t));
  int i;
  t->top = NULL;
  for (i = 0; i < TABSIZE; i++)
    t->table[i] = NULL;
  return t;
}

/* The cast from pointer to integer in the expression
 *   ((unsigned)key) % TABSIZE
 * may lead to a warning message.  However, the code is safe,
 * and will still operate correctly.  This line is just hashing
 * a pointer value into an integer value, and no matter how the
 * conversion is done, as long as it is done consistently, a
 * reasonable and repeatable index into the table will result.
 */

void TAB_enter(TAB_table t, void *key, void *value)
{
  int index;
  assert(t && key);
  index = ((unsigned)key) % TABSIZE;
  t->table[index] = Binder(key, value,t->table[index], t->top);
  t->top = key;
}


void *TAB_look(TAB_table t, void *key)
{
  int index;
  binder b;
  assert(t && key);
  index=((unsigned)key) % TABSIZE;
	SMLOG(13, "\tLook for: %s",S_name((S_symbol) key));
  for(b=t->table[index]; b; b=b->next)
	{
	SMLOG(13, "\tLookUp: %s",S_name((S_symbol) b->key));
    if (b->key==key) return b->value;
	}
  return NULL;
}

void *TAB_pop(TAB_table t, void **value) 
{
  void *k; binder b; int index;
  assert (t);
  k = t->top;
  assert (k);
  index = ((unsigned)k) % TABSIZE;
  b = t->table[index];
  assert(b);
  t->table[index] = b->next;
  t->top=b->prevtop;

  *value = b->value;
  return b->key;
}

void TAB_dump(TAB_table t, void (*show)(void *key, void *value)) 
{
  void *k = t->top;
  int index = ((unsigned)k) % TABSIZE;
  binder b = t->table[index];
  if (b==NULL) return;
  t->table[index]=b->next;
  t->top=b->prevtop;
  show(b->key,b->value);
  TAB_dump(t,show);
  assert(t->top == b->prevtop && t->table[index]==b->next);
  t->top=k;
  t->table[index]=b;
}







