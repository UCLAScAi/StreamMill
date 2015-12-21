/* test of open/close, print, insert, and cursor scan */

#include "rtree.h"

#define TEST_AMT 16
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
    int i;
    DBT dbt;
    dbt.size = sizeof(int);
    key.size = sizeof(Rectangle);

    for(i=0; i<TEST_AMT; i++)
    {
      dbt.data = &i;
      key.data = &testr[i];
      rt->put(rt, &key, &dbt, 0);
    }
    //    rt->get(rt, &testr[10], &dbt, 0);
    //    printf("dbt.data = %s\n", dbt.data);
    rt->close(rt, 0);
    
    rt->open(&rt, "index.db", "data.db", 0);
    rt->cursor(rt, &rtc, 0);
    for (i = 0; i < TEST_AMT; i++)
	if (rtc->c_get(rtc,&testr[i], &dbt, DB_FIRST)!=DB_NOTFOUND)
	    printf("First Fetched %s  size = %d\n", dbt.data, dbt.size);
	else
		printf("No Record Fetched.\n");
    rtc->c_close(rtc);
    rt->close(rt, 0);
    
    return 0;
}
