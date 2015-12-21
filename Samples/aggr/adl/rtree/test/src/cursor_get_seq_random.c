#include <stdlib.h>
#include <stdio.h>
/* test of open/close, print, and insert */
#include "rtree.h"

#define TEST_AMT 1000


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
  RTree *rt;
  RTreeCursor *rtc;
  char tuple[6] = "hello";
  int i;
  int r;
  int rc;
  DBT dbt;
  int tc = 1;
    testcase_generation(tc);
  
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
	close_rtree(rt);


      }
    //print_rtree("index.db", "data.db");

    rt = open_rtree("index.db", "data.db", OLD_RTREE);
    dbt.data = (void*)malloc(sizeof(int));
    dbt.size = sizeof(int);
    rt->cursor(rt, &rtc, 0);
    key.data = (void*)malloc(sizeof(Rectangle));
    key.size = sizeof(Rectangle);
    int fail = 0;
    int filled[TEST_AMT];
    memset(filled, 0, sizeof(filled));
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
	r = *((int*)dbt.data);
	if (r<0 || r>= TEST_AMT ){
	  printf("FAIL at i=%d r = %d testcase=%d!\n", i, r, tc);
	  fail =1;
	}
	else
	  filled[r] = 1;
	break;
      default:
	myerr("unknown return code!\n");
      }
    }
    for (int i = 0; i < TEST_AMT; i++)
      if (!filled[i]){
	printf("FAIL. Can't find data = %d  testcase=%d!\n", i, tc);
	fail = 1;
      }
    if (!fail) printf("PASS!\n");
    
  free(dbt.data);
  return 0;

}
