#include <stdlib.h>
#include <iostream.h>
#include <queue>

#define NOISE 0
#define UNCLASSIFIED -1
#define NUM_OF_POINT 7
#define EPS 1
#define MINPTS 3  // minimal # of neighbours

typedef struct{
  int x,y,ClID;
}point;

int ClusterID;  //  current cluster ID
point SetOfPoints[NUM_OF_POINT]={
  {1,1,UNCLASSIFIED},
  {1,3,UNCLASSIFIED},
  {2,2,UNCLASSIFIED},
  {3,1,UNCLASSIFIED},
  {3,2,UNCLASSIFIED},
  {3,3,UNCLASSIFIED},
  {4,2,UNCLASSIFIED}

}; // Global points set.  Points are identified as index

int numOfPoints = NUM_OF_POINT;

/** regionQuery: find the neighbours of a given ponit, excluding itself
 * i: index of given point
 * eps: Neighbour distance
 * result: set of neighbours, excluding itself
 */
void regionQuery(int i, int eps, std::queue<int> &result){

  cout<<"Regional Query Result for "<<i<<":"<<endl;
  for (int j = 0; j < numOfPoints; j ++){
    if (j!= i && (SetOfPoints[j].x -SetOfPoints[i].x)*(SetOfPoints[j].x - SetOfPoints[i].x) +
	(SetOfPoints[j].y - SetOfPoints[i].y)*(SetOfPoints[j].y - SetOfPoints[i].y) <=eps * eps){
					  result.push(j);
					  cout<<j<<':'<<SetOfPoints[j].ClID<<endl;
    }
  }// for numOfPoints

}

bool ExpandCluster(int i, int ClID, int eps, int MinPts){
  std::queue<int> seeds;
  regionQuery(i, eps, seeds);
  if (seeds.size() < MINPTS ){
    SetOfPoints[i].ClID = NOISE;
    return false;
  } // if size < MINPTS
  else{ // all points in seeds are density-reachable from Point
    SetOfPoints[i].ClID = ClID;
    while (seeds.size() > 0){
      int currentP = seeds.front();
      SetOfPoints[currentP].ClID = ClID;
      seeds.pop();
      std::queue<int> result;
      regionQuery(currentP, eps, result);
      if (result.size() >= MinPts) {
	while (result.size() > 0){
	  int resultP = result.front();
	  if (SetOfPoints[resultP].ClID != UNCLASSIFIED || SetOfPoints[resultP].ClID !=NOISE) 
	    continue;
	  if (SetOfPoints[resultP].ClID == UNCLASSIFIED) 
	    seeds.push(resultP);
	  SetOfPoints[resultP].ClID = ClID;
	}// while 
      }// if result.size() >=MinPts
    } // while seeds.size()>0
    return true;
  }// else
};

int main(){
  ClusterID = NOISE +1;
  for (int i = 0; i < numOfPoints; i++){
    if (SetOfPoints[i].ClID == UNCLASSIFIED){
      if (ExpandCluster(i, ClusterID, EPS, MINPTS))
	ClusterID ++;
    }
    
  }
  /* output the result */
  for (int i = 0 ; i < numOfPoints; i++){
    cout<<SetOfPoints[i].x<<' '<<SetOfPoints[i].y<<' '<<SetOfPoints[i].ClID<<endl;
  }

};
