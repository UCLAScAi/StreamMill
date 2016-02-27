#ifndef SWIMLIB_H
#define SWIMLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "fptree.h"
#include "memmgr.h" 
#include "timeval.h"
//#include "adllib.h"

struct SwimFisStatus{
		int trans_avg_len;
		int no_items;
		int global_min_supp;
		int slide_min_supp;
		int window_size; 
		int slide_size;
		int L_delay_max;
		int no_slides;
		long * archive_fpos;
		Trans trans;
		int all_trans_len, sum_all_pat_lens, max_needed_mem;
		Tree ** localFPs;
		int * local_all_trans_len;
		int * StreamMillBuffer;
		int StreamMillPtr; 

		//other auxiliary variables.
		int err, hist, deleted;
		Tree * tree, * patternTree, * temp_patternTree;
		int * order,*rev_order;
		int temp_pat_num;
		int temp_created;
		int index, new_index, expired_index;

};

inline int max(int a, int b);
int ReadChunkToFP(FILE * fin, int index, int chunksize);
int LoadChunkFromDiskToFP(FILE * fin, int index, Tree * FP);
void cleanUp(struct SwimFisStatus * status);
void ProcessIterate(int windowid, struct SwimFisStatus * status);
void ProcessExpire(int windowid, struct SwimFisStatus * status);
int ProcessMain(int windowid, struct SwimFisStatus * status);
int addTuple(int length, int * pt, int * buf, int bufPtr);







#endif

