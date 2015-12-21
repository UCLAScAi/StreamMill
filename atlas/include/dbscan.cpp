// Dbscan.cpp: 

#include "dbscan.h"
#include <math.h> 
#include <time.h> 
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
using namespace std;

// Constructor
Dbscan::Dbscan()
{
	dis = NULL;
	C = NULL;
	p = NULL;
}


void Dbscan::set(int _n,double _Eps,int _Minpts)
{
	dis = new int * [_n];
	for(int i = 0; i < _n; i++)
		dis[i] = new int [_n];
	for(int j = 0; j <_n; j++)
		for(int k = 0; k <_n; k++)
			dis[j][k] = 0 ;
	Eps = _Eps;
	N = _n;
	Minpts = _Minpts;
	p = new pointDB[_n];
	C = new int[_n];
	for(int l = 0 ; l < _n ; l++)
		C[l] = -2;
	Class = 0;
}



void Dbscan::init_dis()
{
	int i,j;
	for(i = 0 ; i < N; i++) 
		for(j = 0 ; j < N; j++){ 
                      if( distance(p[i],p[j]) <= Eps){  
                           dis[i][j] = 1;
                        }else
                           dis[i][j] = 0;
      }
}

double Dbscan::distance(pointDB p1, pointDB p2) {
        return sqrt(((p1.x-p2.x)*(p1.x-p2.x))+((p1.y-p2.y)*(p1.y-p2.y))); 
}

bool Dbscan::Core(int i)
{
	int sum = 0 ;
	for(int j = 0 ; j < N; j++)
		sum += dis[i][j];
	if(sum >= Minpts )
		return true;
	return false;
}


int Dbscan::visit_next()
{
	for(int i = 0; i < N ; i++)
		if( C[i] == UNCLASSIFIED )
			return i;
	return -1 ;
}

void Dbscan::print_clusters() {
    pointDB temp;
    for (int i=0; i<N; i++) {
       if (C[i] >= 0) {
          temp = p[i];
          cout << "Point (" << temp.x << ", " << temp.y << ") is part of the cluster " << C[i] << endl; 
       }  
   } 
}

void Dbscan::Db_scan()
{
	init_dis();
	time_t t;  
	srand((unsigned) time(&t));  
	int p = rand()%N;
	C_n q(N);
	do{
		if( Core(p) )
		{
			q.i = 0; 
			q.j = 0 ;
			q.Id[ q.j++ ] = p;
			C[p] = Class;
			for(int i = 0 ; i < N ; i ++)
				if( dis[p][i] == 1 && C[i] < 0 )
				{
				        //cout << "***********" << endl;	
                                        q.Id[ q.j++ ] = i;
					C[i] = Class;
				} 
			while( q.i < q.j )
			{
				if( Core( q.Id[ q.i ] ) )
					for( int j = 0 ; j < N; j++)
						if( dis[ q.Id[ q.i ] ][j] == 1 && C[ j ]< 0 )
						{
							q.Id[ q.j++ ] = j;
							C[ j ] = Class;
						}
				q.i++;
			}
			Class++;
		}
		else
			C[ p ] = NOISE;  //noise point
		p = visit_next();
		if( p < 0 )
			break;
	}while( true );
}
