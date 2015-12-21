#include <stdlib.h>
#include <stdio.h>
/* test of open/close, print, and insert */
#include "rtree.h"

#define TEST_AMT 16
#define DATASZ 1000


int
main()
{
    RTree *rt;
    RTreeCursor *rtc;
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
    char tuple[DATASZ] = "hello world!";
    int i;
    int r;
    int rc;
    DBT dbt;

    
    DBT key;
    key.size = sizeof(Rectangle);
    dbt.size = DATASZ;

    for(i=0; i<TEST_AMT; i++)
    {
	if(i!=0)
	    rt = open_rtree("index.db", "data.db", OLD_RTREE);
	else
	    rt = open_rtree("index.db", "data.db", NEW_RTREE);

	key.data = &testr[i];
	dbt.data = tuple;
	rt->put(rt, &key, &dbt, 0);
	close_rtree(rt);


    }
    //print_rtree("index.db", "data.db");

    rt = open_rtree("index.db", "data.db", OLD_RTREE);
    dbt.data = (void*)malloc(sizeof(DATASZ));
    rt->cursor(rt, &rtc, 0);
    key.data = (void*)malloc(sizeof(Rectangle));
    key.size = sizeof(Rectangle);
    for (i=0; i<TEST_AMT; i++){

     rc = rtc->c_get(rtc, 
		     &key, 
		     &dbt, 
		     i==0?DB_FIRST:DB_NEXT);

      switch (rc){
      case DB_NOTFOUND:
	printf("NOT FOUND! i = %d\n",i);
	break;
      case -1:
	myerr("cget return -1!\n");
	break;
      case 0:
	printf("%s\n",(char*)dbt.data);
	break;
      default:
	myerr("unknown return code!\n");
      }

    }
    free(dbt.data);
    //    printf("PASS!\n");    
    return 0;

}
