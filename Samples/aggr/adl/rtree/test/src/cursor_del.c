#include <stdlib.h>
#include <stdio.h>
/* test of open/close, print, and insert */
#include "rtree.h"
#include "stdio.h"
#include "stdlib.h"
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
  int r;
  int rc;
  int j;
  DBT dbt;

    
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
  for (i=0; i<TEST_AMT-3; i++){
    
    key.data=&testr[i];
    rc=0;
    rc = rtc->c_get(rtc, &key, &dbt,DB_SET);

    switch (rc){
    case DB_NOTFOUND:
      printf("NOT FOUND! i = %d\n",i);
      break;
    case -1:
      myerr("cget return -1!\n");
      break;
    case 0:
      r = *((int*)dbt.data);
      printf("Deleting  tuple=%d\n",r);
      rc = rtc->c_del(rtc,0);
      switch (rc){
      case 0:
	
	break;
      case DB_NOTFOUND:
	printf("delete not found\n");
	break;
      default:
	printf("delete error!\n");
      }//end switch
      break;
    default:
      myerr("unknown return code!\n");
    }
    for (j = 0; j < TEST_AMT-3;j++){
      key.data = &testr[j];
      int rcGet = rt->get(rt,&key, &dbt,0);
      switch (rcGet){
      case 0:
	printf("%d\n", *((int*)dbt.data));
	break;
      case DB_NOTFOUND:
	printf("DB_NOTFOUD at %d\n",j);
	break;
      default:
	printf("ERROR!\n");
	break;
      } // end switch
    } // end for j

    //rtree_print(rt,0);
  }
  rtc->c_close(rtc);
  close_rtree(rt);
  free(dbt.data);
  //    printf("PASS!\n");    
  return 0;

}
