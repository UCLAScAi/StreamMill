/*
 *  tabf.so is a shared library
 *  using:  
 *	    gcc -shared -o tabf.so -fPIC -I../BerkeleyDB/include tabf.c
 *  to create tabf.so
 *
 *  tabf.so must be in LD_LIBRARY_PATH
 */

#include <db.h>

struct result {
  int a;
  int b;
};

int fib(int first_entry, struct result *tuple, int k)
{
  static int count;
  static int last;
  static int next;

  if (first_entry == 1) {
    count = 0;
    next=1;
    last=0;
  }
  if (count++ <k) {
    tuple->a = count;
    tuple->b = last;

    last = next;
    next = next+tuple->b;
    return 0;
  } else {
    return DB_NOTFOUND;
  }
}

