/* test of open/close, print, insert, delete, search */

#include "db.h"
#include "rtree.h"
#include "test.h"

//#define TEST_AMT	100
//Rectangle testr[TEST_AMT];

void
testcase_generation()
{
  int i;

  srandom(1000);
  for(i=0; i<TEST_AMT; i++)
    {
      Rectangle *r = &testr[i];

      r->x_ul = random() % 1000;
      r->x_lr = r->x_ul + random() % 1000;
      r->y_lr = random() % 1000;
      r->y_ul = r->y_lr + random() % 1000;
    }
}

int
main()
{
  RTree *rt;
  char tuple[6] = "hello";
  int i;
  DBT dbt;
  dbt.data = tuple;
  dbt.size = 6;

  //  testcase_generation();
  rtree_create(&rt, "index.db", "data.db",0);
  for(i=0; i<TEST_AMT; i++)
    {
      printf("Inserting i = %d\n", i);
      sprintf(tuple, "%05i", i);
      rt->put(rt, &testr[i], &dbt, DB_APPEND);
    }
  rt->print(rt, 0);
  rt->close(rt, 0);

  /*    rt->open(&rt, "index.db", "data.db", 0);
	for(i=0; i<TEST_AMT/10; i++)
	{
	printf("Deleting i = %d\n", i);
	rt->del(rt, &testr[i], 0);
	if(i % 5 == 0)
	rt->print(rt, 0);
	}
	rt->print(rt, 0);
	rt->close(rt, 0);*/

  rt->open(&rt, "index.db", "data.db", 0);
  for(i=0; i<TEST_AMT; i++)
    {
      /* check if it's there */
      int ret = rt->get(rt, &testr[i], &dbt, 0);
      if(ret != 0)
	printf("Not found\n");
      else
	printf("Found the value in T[%d] is %s\n", i, dbt.data);
    }
  rt->close(rt, 0);

  return 0;
}
