// Dbscan.h: interface for the Dbscan class.


#include "points.h"
#define NOISE -1  
#define UNCLASSIFIED -2
#define NULL 0

struct C_n
{
	int i,  //searched 
	    j,  //total
	    *Id;
	public:
	C_n(int n)
	{
		Id = new int [n];
		i = 1 ;
		j = 0 ;
	}
	~C_n()
	{
		delete []Id;
	}
};

class Dbscan  
{
	public:
		int N;  // the total number of the points
		int **dis;  /////////////////
		double Eps;
                int Minpts;
		pointDB *p;
		int Class;
		int *C;
	public:
		Dbscan() ;
		void set(int n,double Eps,int Minpts);
		//virtual ~Dbscan();
		double distance(pointDB p1,pointDB p2);
	        void print_clusters();	
                void init_dis();
		bool Core(int i);
		int visit_next();
		void Db_scan();
};
