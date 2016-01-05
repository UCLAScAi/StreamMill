#include <stdio.h>
#include <string.h>
#include "util.h"
#include <sql/symbol.h>
#include <sql/table.h>
#include "const.h"


#include <vector>
using namespace std;


extern "C" {
#include <dbug/dbug.h>
}

//I could not include SMLog and use the function 
//FIXME so I had to add this function here
//#include <SMLog.h>
#include <stdarg.h>
#define SMLOG_LEVEL 12
#define SMLOG_FILENAME "./SMLOG.log"
void SMLOG1(int level, const char* Format, ...)
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


static S_symbol mksymbol(char * name, S_symbol next)
{
  S_symbol s=(S_symbol)ntMalloc(sizeof(*s));
  s->name=name; s->next=next;
  return s;
}

#define SIZE 109  /* should be prime */

static S_symbol hashtable[SIZE];

void SymInit()
{
  memset(hashtable, 0, sizeof(S_symbol)*SIZE);
}

static unsigned int hash(char *s0)
{
  unsigned int h=0; char *s;
  for(s=s0; *s; s++)  
    h = h*65599 + *s;
  return h;
}
 
static int streq(char * a, char * b)
{
  return !strcmp(a,b);
}

S_symbol S_Symbol(string &s){
  return S_Symbol((char*)(s.c_str()));
}
S_symbol S_Symbol(char * name)
{
  int index= ::hash(name) % SIZE;
  S_symbol syms = hashtable[index], sym;
  for(sym=syms; sym; sym=sym->next) {
    if (streq(sym->name,name)) 
      return sym;
  }
  sym = mksymbol(name,syms);
  hashtable[index]=sym;
  return sym;
}
S_symbol new_Symbol(char *str)
{
  return S_Symbol(copyStr(str));
}
 
char * S_name(S_symbol sym)
{
  // NL: Added the next two lines because the system creashed 
  // when using a mining model. 	
  if (sym == NULL)
	return NULL;
  return sym->name;
}

S_table S_empty(void) 
{ 
  return TAB_empty();
}

void S_enter(S_table t, S_symbol sym, void *value) 
{
//  SMLOG1(10, "symbol.cc::Entering S_enter sym: %s", S_name(sym));
  DBUG_EXECUTE("symbol", printf("enter symbol: %s\n", S_name(sym)););
  TAB_enter(t,sym,value);
}

void *S_look(S_table t, S_symbol sym) 
{
//  SMLOG1(10, "symbol.cc::Entering S_look sym: %s", S_name(sym));
  return TAB_look(t,sym);
}

static struct S_symbol_ marksym = {"<mark>",0};

void S_beginScope(S_table t)
{ 
  S_enter(t,&marksym,NULL);
}

vector<void*> S_endScopeval(S_table t)
{
  S_symbol s;
  void *dummy;
  vector<void*> vals;

  do {
    s= (S_symbol)TAB_pop(t, &dummy);
    DBUG_EXECUTE("symbol", printf("delete symbol: %s\n", S_name(s)););
    if(s != &marksym)
      vals.push_back(dummy);
  } while (s != &marksym);
  return vals;
}

void S_endScope(S_table t)
{
  S_symbol s;
  void *dummy;

  do {
    s= (S_symbol)TAB_pop(t, &dummy);
    DBUG_EXECUTE("symbol", printf("delete symbol: %s\n", S_name(s)););
  } while (s != &marksym);
}

void S_swapScope(S_table t)
{
  void *buf[MAX_STR_LEN]; 
  S_symbol s;
  void *value;
  int i=0;

  do {
    s = (S_symbol)TAB_pop(t, &value);
    buf[i++] = s;
    buf[i++] = value;
  } while (s != &marksym);

  do {
    s = (S_symbol)TAB_pop(t, &value);
  } while (s!= &marksym);

  while (i>0){
    TAB_enter(t, buf[i-2], buf[i-1]);
    i-=2;
  }
}
void S_dump(S_table t, void (*show)(S_symbol sym, void *binding)) 
{
  TAB_dump(t, (void (*)(void *, void *)) show);
}



