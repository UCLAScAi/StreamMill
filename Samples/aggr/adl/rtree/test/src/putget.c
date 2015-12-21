#include <stdlib.h>
#include <stdio.h>
/* test of open/close, print, and insert */
#include "rtree.h"

#define TEST_AMT 16



int
main()
{
    RTree *rt;
    Rectangle testr[TEST_AMT] =
	{
	    {10,60,20,50},	/* R8 */
		{35,80,40,70},	/* R9 */
		{50,85,55,20},	/* R11 */
		{40,45,52,35},	/* R12 */
		{35,65,40,55},	/* R10 */
		{65,80,70,35},	/* R13 */
		{60,60,65,50},	/* R14 */
		{0,10,10,0},	/* R15 */
		{15,30,50,7},	/* R16 */
		{75,35,100,20},	/* R17 */
		{80,40,85,7},	/* R18 */
		{82,25,95,15},	/* R19 */
		{75,20,95,7},
		{0,30,50,10},
	    	{0,30,50,10},
	    	{0,30,50,10},
	};
    char tuple[6] = "hello";
    int i;
    int r;
    int rc;
    DBT dbt;
    dbt.size = 6;

    
    DBT key;
    key.size = sizeof(Rectangle);
    dbt.size = sizeof(int);
    for(i=0; i<TEST_AMT; i++)
    {
	if(i!=0)
	    rt = open_rtree("index.db", "data.db", OLD_RTREE);
	else
	    rt = open_rtree("index.db", "data.db", NEW_RTREE);
	key.data = &testr[i];
	dbt.data = &i;
	rt->put(rt, &key, &dbt, 0);
	//rtree_print(rt,0);
	close_rtree(rt);
    }

    rt = open_rtree("index.db", "data.db", OLD_RTREE);
    for (i=0; i<TEST_AMT - 3; i++){
      key.data=&testr[i];
      rc=rt->get(rt, &key, &dbt,0);
      switch (rc){
      case 0:
      r = *((int*)dbt.data);
	printf("i=%d, r= %d!\n", i, r);
      if (i != r){
	printf("FAIL at i=%d, r= %d!\n", i, r);
      };
      break;
      case DB_NOTFOUND:
	printf("DB_NOTFOUND!\n");
	break;
      }
    }
    //    printf("PASS!\n");    
    return 0;
}
