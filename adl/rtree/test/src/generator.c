#include <stdlib.h>
#include <stdio.h>
#define TEST_AMT 1000
#include "rtree.h"
int main()
{
    int i;
    srandom(1000);
    for (int j = 0; j < 10; j++){
      char filename[256];
      filename[0]=j+'0';
      filename[1] = 0;
      strcat(filename+1, ".data");
    freopen(filename, "w", stdout);
    for(i=0; i<TEST_AMT; i++)
    {
	Rectangle r;

	r.x_ul=random() % 1000;
	r.x_lr = r.x_ul + random() % 1000;
	r.y_lr = random() % 1000;
	r.y_ul = r.y_lr + random() % 1000;
	printf("%d %d %d %d\n", 
	       r.x_ul,
	       r.y_ul,
	       r.x_lr,
	       r.y_lr);
    }
    }
}
