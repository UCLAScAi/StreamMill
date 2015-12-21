#include <stdlib.h>
#include <stdio.h>
/* test random data of open/close, print, and insert */
#include "rtree.h"


/* please refer to generator.c */
#define TEST_AMT 1000//10000 running time is about 3`30``

Rectangle testr[TEST_AMT] ;
void testcase_generation(int tc){
    char filename[256];
    char ext[] = ".data";
    char prefix[] = "src/";
    strcpy(filename,prefix);
      filename[strlen(prefix)]= tc +'0';
      filename[strlen(prefix)+1] = 0;
      strcat(filename, ext);
      freopen(filename, "r", stdin);
    //read the data
    for (int i = 0; i < TEST_AMT; i++){
      scanf("%d ", 
	    &(testr[i].x_ul));
      scanf("%d ",
	    &(testr[i].y_ul));
      scanf("%d ",
	    &(testr[i].x_lr));
      scanf("%d ",
	    &(testr[i].y_lr));
    }
}
int
main()
{
  for (int tc = 0; tc < 10; tc++){

    testcase_generation(tc);
    RTree *rt;
    int i;
    int r;
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
	dbt.size = sizeof(int);
	rt->put(rt, &key, &dbt, 0);
	//rtree_print(rt,0);
	close_rtree(rt);

	
    }

    rt = open_rtree("index.db", "data.db", OLD_RTREE);
    dbt.data = (void*)malloc(sizeof(int));
    //rtree_print(rt,0);
    int fail = 0;
    for (i=0; i<TEST_AMT; i++){
      key.data=&testr[i];
      rt->get(rt, &key, &dbt,0);
      r = *((int*)dbt.data);
      if (i != r){
	printf("FAIL at i=%d r = %d testcase=%d!\n", i, r, tc);
	fail = 1;
      };
    }
    if (!fail) printf("PASS!\n");
    free(dbt.data);
  }
    return 0;
}
