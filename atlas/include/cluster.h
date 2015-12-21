#include <sys/time.h>
#include <stdio.h>
#include "linked_list.h"

//  tuple just has x and y coord as well as cellid... it can be easily extended to 3 dimensions of course
typedef struct tuple {
   float* coords;
   int cell_id; 
}tuple;

// this will be used to return a nice struct to a UDA
typedef struct clusterReturn {
   int* myWindowid;
   int* myClassid;
   int mySize;
   double* myx;
   double* myy;;
}clusterReturn;


// this is (primarily for simulation purposes) is used for our to simulate streamMil's input
typedef struct rExtC {
   int length;
   float* pt;
}rExtC;

// same as above but only for integers
typedef struct iExtC {
   int length;
   int* pt;
}iExtC;

// this is a temp point used for testing
rExtC t_pt;

// this is our grid cell
typedef struct grid_cell{
  int tuple_count;
  int part_of_cluster;  // is this cell part of some cluster already?
  int* neightbor; 
  int north_neightbor;
  int south_neightbor;
  int west_neightbor;
  int east_neightbor;
  int cell_id; 
  tuple* tuple_list;
  double* average_coord;
}grid_cell;

// this is going to be used for anncad. 
int** anncad;

// this is for when given a tuple's cell number to find out whch anncad cell it belongs to
int* anncad_locate;

// this we will use for density partiotioning
int NUMBER_OF_CELLS; 

// This will keep track of windowId
int windowId;

//*** Other things we can set during bootstrap
int WINDOW_SIZE; 
int SLIDE_SIZE; 
int CLUSTER_THR; 
int DENSE_THR; 
int NUMBER_OF_CELLS_; 
//** end other things

// this will count the number of the file if EVOLVE_TRACK is enabled
int file_no;
FILE* ofp;
char outputFilename[100];

// this is our grid
typedef struct grid_str{
  grid_cell* cells;
}grid_str;

grid_str grid; 


// this will hold summary over the whole window
typedef struct summary {
  int tuple_count;
  int cluster_count;
  int cluster_center;
  tuple* tuples;
  tuple** clusters;
  grid_str grid; 
}summary;

// This will hold our dimension variables
typedef struct dim {
  int length;
  double partition;
  int cell_count;
}dim;

dim* dimensions;
int d; // hold dimension

// this is used to keep track of which cell was split so we can undo it
int SPLIT_CLUSTER;

int* cell_num; // this will hold where the point falls in each dimension of the grid

int duplicate;   // this will check for duplicates, if duplicates are disabled this will always be true

int number_of_cells; // holds total number of cells in a grid
int number_of_cells_old; // holds total number of cells in a grid
tuple temp;
int* number_of_cells_in_d; // holds total number of cells in the first dimension
// declare 3 different summaries we will have
summary window_summary;
summary plus_summary;
summary minus_summary;

// this will return where does a given point fall in the cell
int map_to_cell_in_l(tuple t, int level);

// this will get the cluster of a cell in a given level
int compute_cell_cluster_in_l(int cell, int level);

// Function to add summary for incoming new tuples
int fn_plus();

// will return true if we already have the point in grid
int point_exists(tuple t);

// Function to remove summary for old tuples
int fn_minus();

// This will reset the plus summary
void reset_plus_summary();

// This will reset the plus summary
void testDummyMethod();

// this will store our buffer and check variable for auto_dimensions
tuple* bufferTuple;
int buffer_size;
float* new_max;
float* old_max;
int points_inserted_in_buffer;

// This will add stuff to buffer
void add_to_buffer(tuple t);

// this will re-partition stuff based on newly arrived tuples
int re_adjust_dimension();
 
// this will check if out logic plus_summary condition is satisfied
int logic_window_check(tuple t);
// this is used inside of logic_window_checl
time_t start,now;
int plus_summary_tc2;
struct timeval start_s;
struct timeval end_s;

// this simply prints out all of the clusters 
void print_clusters();

// overloaded method
void print_clustersESL(clusterReturn* tmp);

// this will print all of the points of our cluster 
void print_points_of_clusters(int i);

// initialize the grid and variables
void initCluster(rExtC pt);

// initialize anncad 
void init_anncad();

// this wil track if we already called init_anncad or not
int _init_anncad;

// adds a tuples to the current window
void add_to_window(tuple t);

// This assigns a tuple to a specific cell
void add_t_to_plus_summary(tuple t);

// this will find out if we have any new clusters
void explore_clusters();

// this will find out if we have any new clusters
void explore_clusters_sporadic();


// this will ask user for dimension input
void ask_dimension();

// this will compute average coordinates
void compute_average_coord();

// this will ask user for tuple input based in dimension
void iterate(rExtC pnt);

// overloaded function
void iterateESL(rExtC pnt, clusterReturn* tmp);

// this will simulate input from stream mil by reading in input and converting to rExt 
void simulate_input();

// given two cells which are clusters try to merge them
int try_to_merge(int c, int dir);

// this will calculate into which cell to place a tuple
int which_cell(tuple t);

// here we reset all clusters
void reset_clusters();

// this will only reset clusters of the cell's neightbors
void reset_cluster(int cid);

// this will compute total number of cells given cell cunt in one row (all rows have the same cell cnt)./
int comp_number_of_cells(int cellcnt);
