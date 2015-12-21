#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sys/time.h> 
#include "dbscan.h"
using namespace std;

int main( int argc, char **argv )
{
        /* start and end timers */
        struct timeval start_s;
        struct timeval end_s;
        gettimeofday(&start_s, NULL);
	/* total number of points */
	int N = atoi(argv[1]);
	/* Eps */
	double Eps = atof(argv[2]);
	/* min number of pts */
	int Minpts = atoi(argv[3]);

	Dbscan dbscan;
	dbscan.set(N,Eps,Minpts);  

        

        /* start reading file with points */
	string line;
	ifstream myfile (argv[4]);
	if (myfile.is_open())
	{
        int i=0;	
    	       while (! myfile.eof() )
		{
			getline (myfile,line);
		        dbscan.p[i].x = atof(line.c_str());	
			getline (myfile,line);
		        dbscan.p[i].y = atof(line.c_str());	
                        //cout << "point " << i << " x: "<< dbscan.p[i].x << " y " << dbscan.p[i].y << endl;
        dbscan.Db_scan();
	                i++;	
                }
		myfile.close();
	}
	else { cout << "Unable to open file"; return 0; } 

        dbscan.print_clusters();

        /* report time */
        gettimeofday(&end_s, NULL);
        double t1=start_s.tv_sec+(start_s.tv_usec/1000000.0);
        double t2=end_s.tv_sec+(end_s.tv_usec/1000000.0);
        printf(" t1 %f t2 %f \n",t1,t2);
        printf("Total time elapsed: %.61f \n",
                t2-t1);
 
	return 1;
}
