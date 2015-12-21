#include "rtree.h"

int debug = 1;

#if 0
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
    char tuple[TUPLESZ] = "hello";
    int i;

    for(i=0; i<TEST_AMT; i++)
    {
	if(i!=0)
	    rt = open_rtree("index.db", "data.db", 2, 0);
	else
	    rt = open_rtree("index.db", "data.db", 2, 1);
	insert_rtree(rt, &testr[i], tuple, 6);
	close_rtree(rt);
	print_rtree("index.db", "data.db");
    }

    return 0;
}
#else
#define TEST_AMT	1000
Rectangle testr[TEST_AMT];

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
    char tuple[TUPLESZ] = "hello";
    int i;

    testcase_generation();
    for(i=0; i<TEST_AMT; i++)
    {
	printf("Inserting i = %d\n", i);
	if(i!=0)
	    rt = open_rtree("index.db", "data.db", 0);
	else
	    rt = open_rtree("index.db", "data.db", 1);
	insert_rtree(rt, &testr[i], tuple, 6);
	close_rtree(rt);
    }

    print_rtree("index.db", "data.db");

    rt = open_rtree("index.db", "data.db", 0);
    for(i=0; i<TEST_AMT/10; i++)
    {
	printf("Deleting i = %d\n", i);
	delete_rtree(rt, &testr[i]);
    }
    close_rtree(rt);
    print_rtree("index.db", "data.db");

    rt = open_rtree("index.db", "data.db", 0);
    for(i=0; i<TEST_AMT; i++)
    {
	/* check if it's there */
	pageno_t b;
	unsigned e;
	tupleno_t t;
	search_rtree(&b, &e, &t, rt, &testr[i], 1);
	if(b < 0 || b >= rt->tb_ind)
	{
	    fprintf(stderr, "Invalid block number B%d\n", b);
	    exit(-1);
	}
	if(b == 0)
	    printf("Not found\n");
	else
	    printf("Found in block %ld entry %d at tuple %ld\n", b, e, t);
	sequential_search_rtree(&b, &e, &t, rt, &testr[i]);
	if(b < 0 || b >= rt->tb_ind)
	{
	    fprintf(stderr, "Invalid block number B%d\n", b);
	    exit(-1);
	}
	if(b == 0)
	    printf("Not found\n");
	else
	    printf("Found in block %ld entry %d at tuple %ld\n", b, e, t);
    }
    close_rtree(rt);

    print_rtree("index.db", "data.db");
    return 0;
}
#endif
