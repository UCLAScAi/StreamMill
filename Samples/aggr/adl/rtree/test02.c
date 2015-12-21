/* test of open/close, print */
#include "rtree.h"
#include "db.h"
#include "test.h"

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
    DBT dbt;
        dbt.data = tuple;
    dbt.size = 6;

    for(i=0; i<TEST_AMT; i++)
    {
	if(i!=0)
	    rt = open_rtree("index.db", "data.db", 0);
	else
	    rtree_create(&rt, "index.db", "data.db", 0);
			rt->put(rt, &testr[i], &dbt,0);
		close_rtree(rt);
		print_rtree("index.db", "data.db");
    }

    return 0;
}
