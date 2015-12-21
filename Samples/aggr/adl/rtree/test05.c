/* test of open/close, print, insert, delete, search, and sequential search */

#include "rtree.h"

#define TEST_AMT	100
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
    char tuple[6] = "hello";
    int i;
    DBT dbt;
    dbt.data = tuple;
    dbt.size = 6;
    

    testcase_generation();
    for(i=0; i<TEST_AMT; i++)
    {
	printf("Inserting i = %d\n", i);
	if(i!=0)
	    rt = open_rtree("index.db", "data.db", 0);
	else
	    rt = open_rtree("index.db", "data.db", 1);
	rt->put(rt, &testr[i], &dbt, 0);
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
