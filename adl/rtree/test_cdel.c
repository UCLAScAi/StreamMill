#include "rtree.h"

#define TEST_AMT 1
int
main()
{
    RTree *rt;
    RTreeCursor *rtc;
    Rectangle testr[TEST_AMT] =
	{
	    {10,60,20,50},	/* R8 */

};
    Rectangle re;
    char tuple[6];
    int i;
    int rc = 0;
    DBT dbt;
    DBT *newDbt;
     dbt.data = tuple;
    dbt.size = 6;
   

printf("tuple=%s\n", tuple);

    rtree_create(&rt, "index.db", "data.db", 0);
    for(i=0; i<TEST_AMT; i++)
    {
	sprintf(tuple, "%05d", i+1);
	rt->put(rt, &testr[i], &dbt,DB_APPEND);
	//rt->print(rt, 0);
    }
    rt->close(rt, 0);

    rt->open(&rt, "index.db", "data.db", 0);
    rt->cursor(rt, &rtc, 0);

    	if (rtc->c_get(rtc,&re, newDbt,DB_FIRST)!=DB_NOTFOUND)
      {
	    printf("First Fetched %s\n", dbt.data);
	    rc=rtc->c_del(rtc,0);
	    switch (rc){
	      case 0:
		printf("delete successful!\n");
		break;
	  case DB_NOTFOUND:
	      printf("delete not found\n");
	    break;
	    default:
	      printf("delete error!\n");
	    }//end switch
	      }


    rtc->c_close(rtc);
    rt->close(rt, 0);

    return 0;
}
