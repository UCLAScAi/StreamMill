// things which are commented we set during bootstrap
//#define WINDOW_SIZE 30
//#define SLIDE_SIZE 5 
//#define NUMBER_OF_CELLS_ 8 // default number of cells in each dimension
#define ROOM 8 // how much extra room to make (to take care of dimension splits); 
#define ENABLE_ANNCAD 0  // this is just an on/off switch for anncad
#define ENABLE_SPORADIC 0 // for sporadic stuff 
#define ENABLE_AUTOPART 0  // enable auto-partitioning 
#define DISABLE_DUPLICATES 1 // do we allow duplicates? 
// slide logic controls how you want to add the tuples to the 
// main window 
// 1 - we add to main window when plus_summary is full
// 2 - we add to main window when we get SLIDE_CHAR as tuples's x coordinate
// 3 - we add based on time, in that case we use SLIDE_TIME to specify in how many hours
// to add to the window_summary 
#define SLIDE_LOGIC 1 
#define SLIDE_CHAR 999 
#define SLIDE_TIME 60 
//#define CLUSTER_THR 3 
//#define DENSE_THR 10
#define MIN_D 100 

#define DEBUG 0 
#define INFO 0 
#define AA_DEBUG 0 
#define RESULT 1 
#define RESULT_PTS 0 
#define SHOW_TIME 0 

// if this is enab.ed we will output a 
// new file every time we are at the end of our slide
// in order to track evolution of clusters
#define EVOLVE_TRACK 0 

// this is for ANNCAD
#define CL_THR 0.80
#define EMPTY -1
#define MIXED -2
