#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "include/cluster.h"
#include "include/config.h"

/* We don't add main, because we have main in the adl program
int main(int argc, char* argv[]) {
        // if this is the case we set everything to default. 
        if (argc != 7) 
        {
           printf("Setting things to default [WINDOW_SIZE = 30] [SLIDE_SIZE = 5] [CLUSTER_THR = 3] [DENSE_THR = 10] [NUMBER_OF_CELLS] [DIMENSION]\n");
           WINDOW_SIZE=30;    
           SLIDE_SIZE=5;    
           CLUSTER_THR=3;    
           DENSE_THR=10;    
           NUMBER_OF_CELLS_=8;    
	   t_pt.length = 2;
        } 
        else {
           WINDOW_SIZE=atoi(argv[1]);    
           SLIDE_SIZE=atoi(argv[2]);    
           CLUSTER_THR=atoi(argv[3]);    
           DENSE_THR=atoi(argv[4]);    
           NUMBER_OF_CELLS_=atoi(argv[5]);    
	   t_pt.length = atoi(argv[6]);
        }	
        ask_dimension();  // record dimension       
	init(t_pt);           // init all data structures
	simulate_input();      // start the tuple input
}
*/
// adds a tuple to window, then places tuple in a cell and checks for clusters
void add_to_window(tuple t) {
	int i;
	t.cell_id = which_cell(t);   // find out where the tuple lies (which cell) 
	if (DEBUG) 
		printf("DEBUG: in add-to_window and length of window is %i and t.coords[0] is %f and partition is %f \n",dimensions[0].length,t.coords[0],dimensions[0].partition);
	//if (t.coords[0] <= dimensions[0].length) { 
	add_t_to_plus_summary(t);    
	for (i=0; i<d;i++){ 
		if (DEBUG) 
			printf("%f ",t.coords[i]);
	}
	if (DEBUG)
		printf(" was put in WINDOW \n");
	if (DEBUG) 
		printf("DEBUG: in cell %i \n",t.cell_id);
	if (ENABLE_ANNCAD) 
		map_to_cell_in_l(t,2);
	//}
	//  printf(" This point is classified as cluster %i \n",map_to_cell_in_l(t,2));
	// if plus_summary is full, then we have to merge that summary with the main window and find clusters
	if (plus_summary.tuple_count == (SLIDE_SIZE) && logic_window_check(t)) {
		// see if we have to call fn_minus
		if (window_summary.tuple_count == WINDOW_SIZE)
			fn_minus();
		fn_plus();           // merge new tuples with the main window
	        if (ENABLE_SPORADIC)
                    explore_clusters_sporadic();
                else {	
                    explore_clusters();  // new clusters?
                }
	} 
}

// calculate into which cell the tuple is supposed to go
int which_cell(tuple t) {
	int i;
	int which_cell_num = 0;
	int nociltd = 1; // This will hold number of cells in previous dimensions.  
	// this is to find out where the point maps in each dimension
	if (AA_DEBUG) 
		printf("AA_DEBUG: dimensions[i].partition is %i number_of_cells_in_d[0] is %i \n",dimensions[0].partition,number_of_cells_in_d[0]); 
	for (i=0; i<d;i++){
		if (DEBUG) 
			printf("tuple[%i] is %f \n",i,t.coords[i]);
		cell_num[i] = (t.coords[i]/dimensions[i].partition);
		cell_num[i] %= number_of_cells_in_d[i];  
		if (DEBUG)
			printf("DEBUG: cell_num[%i] is %i \n",i,cell_num[i]); 
	}
	// and now we find out the cell number
	which_cell_num = cell_num[0]; 
	// here we simply need to improve this by dividing first by dimensions[i] and then
	// and then oring both results.   
	for (i=1; i<d;i++){ 
		which_cell_num = (which_cell_num)|(cell_num[i] << (int)(log(number_of_cells_in_d[i-1])/log(2))); 
	}
	return which_cell_num;
}
// ask for user to enter dimension
void ask_dimension() {
	int i; 
        // this will be provided by the Stream Mill algorithm, but for now we hard-code it in.

	/*
	   for (i=0;i<d;i++) 
	   scanf("%f",&t_pt.pt[i]);
	 */
	/*
	   char buf[10]; 
	   int c = 0;
	   printf("Enter grid dimension: ");
	   scanf("%s",&buf);
	   d = 1;
	   while (buf[c]) 
	   {
	   if (buf[c]==',')
	   d++;
	   c++;
	   }
	   if (DEBUG)
	   printf("Dimension : %i\n",d); 
	   dimensions = (dim*)malloc(d*sizeof(dim)); 
	 */ 
	/*for (i=0; i<d; i++) {
	  printf("Length for dimension %i: ",i+1);
	  scanf("%d",&dimensions[i].length);
	  printf("Partition for dimension %i: ",i+1);
	  scanf("%d",&dimensions[i].partition);
	  dimensions[i].cell_count = (dimensions[i].length)/(dimensions[i].partition);
	  }*/ 
}

// this will simulate input from stream mil by reading in input and converting to rExt 
void simulate_input() {
	int i;
	int j;
	rExtC tpl;
	int point_count = 0;
	tpl.pt = (float*)malloc(d*sizeof(float));
	char temp_check[100];
	if (SHOW_TIME)printf("Enter tuple coordinated for %d dimensions \n",d); 
	while(1){
		if (DEBUG)
			printf("Reading point #%i: prev pt read : %f \n", point_count,tpl.pt[0]);
		for (i=0;i<d;i++) {
			if (DEBUG) 
				printf("Coordinate %i: ",i+1);
			if(scanf("%f",&tpl.pt[i])==EOF) {
				gettimeofday(&end_s, NULL);
				double t1=start_s.tv_sec+(start_s.tv_usec/1000000.0);
				double t2=end_s.tv_sec+(end_s.tv_usec/1000000.0);
				if (SHOW_TIME) {printf(" t1 %f t2 %f \n",t1,t2);  
				    printf("Total time elapsed: %.61f \n", 
						t2-t1);
                                }
				exit(EXIT_SUCCESS); 
			} 
		}
		point_count++; 
		iterate(tpl);
	}
}

// overloaded method which asks for user input
void iterateESL(rExtC pnt, clusterReturn* tmp) {
	int i;
	int j;
	char temp_check;
        if (DEBUG)
   		printf("In iterate \n");
	for (i = 0; i<d; i++) 
		temp.coords[i] = pnt.pt[i];
	if (DEBUG) {
		printf("Inserting -- ");
		for (i=0; i<d;i++) 
			printf("%f ",temp.coords[i]);
		printf(" --- \n");
	}
	if (buffer_size != WINDOW_SIZE) {
		add_to_buffer(temp);

        }
	else { 
                // print_clustersESL(tmp); 
		re_adjust_dimension(); 
		if (DEBUG)  
			printf("Points inserted in buffer is %i \n", points_inserted_in_buffer); 
		for (i=0; i< points_inserted_in_buffer; i++){  
			for (j=0; j<d; j++){  
				temp.coords[j] = bufferTuple[(buffer_size-points_inserted_in_buffer)+i].coords[j];
			}
			if (DEBUG)  
				printf("Adding buffer point to window \n ");
			add_to_window(temp);
		}
		// clean-up
		points_inserted_in_buffer = 0;
		buffer_size -= SLIDE_SIZE;
                //print_clustersESL(tmp);
	}
}


// ask for user input
void iterate(rExtC pnt) {
	int i;
	int j;
	char temp_check;
        if (DEBUG)
   		printf("In iterate \n");
	for (i = 0; i<d; i++) 
		temp.coords[i] = pnt.pt[i];
	if (DEBUG) {
		printf("Inserting -- ");
		for (i=0; i<d;i++) 
			printf("%f ",temp.coords[i]);
		printf(" --- \n");
	}
	if (buffer_size != WINDOW_SIZE)
		add_to_buffer(temp);
	else { 
		re_adjust_dimension(); 
		if (DEBUG)  
			printf("Points inserted in buffer is %i \n", points_inserted_in_buffer); 
		for (i=0; i< points_inserted_in_buffer; i++){  
			for (j=0; j<d; j++){  
				temp.coords[j] = bufferTuple[(buffer_size-points_inserted_in_buffer)+i].coords[j];
			}
			if (DEBUG)  
				printf("Adding buffer point to window \n ");
			add_to_window(temp);
		}
		// clean-up
		points_inserted_in_buffer = 0;
		buffer_size -= SLIDE_SIZE;
	}
}

// if new max is detected we re-adjust demensions
int re_adjust_dimension() {
	int different = 0;
	int i=0;
	for (i=0; i<d; i++) {
		if (new_max[i] != old_max[i]) {
			different = 1;  
			new_max[i] = old_max[i]; 
		}
	}

	// if max (old) is the same as max(new) then we don't do anything. 
	if (different == 0)
		return 1;
	else {
		// TODO: first we must assign the new maximum to the dimension i (which is divisible by 100) 
		// then we assign a partition such that we have (4) cells in dimension i
		for (i=0; i<d; i++){ 
			dimensions[i].partition = floor(new_max[i])/NUMBER_OF_CELLS; 
			if (AA_DEBUG)
				printf("AA_DEBUG: deminsions[%i].partition is %i \n",i,dimensions[i].partition);   
		}
		// now we can call init anncad
		if (_init_anncad && ENABLE_ANNCAD){ 
			init_anncad();
			_init_anncad = 0;
		} 
	}
}

void add_to_buffer(tuple t) {
	int i;
	/*int i;
	  tuple b_tuple;
	  b_tuple.cell_id = t.cell_id;
	 */
	for (i=0; i<d; i++){ 
		bufferTuple[buffer_size].coords[i] = t.coords[i];
		if (old_max[i] < t.coords[i])
			old_max[i] = t.coords[i];
	}
	buffer_size++;
	points_inserted_in_buffer++;
	if (AA_DEBUG) { 
		printf("AA_DEBUG: Added tuple ");
		for (i=0; i<d;i++)
			printf("%f ",bufferTuple[buffer_size-1].coords[i]);
		printf("AA_DEBUG: to buffer buffer_size is %i \n",buffer_size);
	}
}

void print_points_of_clusters(int i) {
        printf("in print_point_of_clusters\n");
        exit(1);
	int j = 0;
	int k = 0;
	char str[100];
        char fn[100] = "./output/"; 
        sprintf(fn,"./output/evolve/matlab%i.dat", file_no);  
        if (EVOLVE_TRACK)
              ofp = fopen(fn, "w");
         

	for (j=0; j < grid.cells[i].tuple_count; j++) {
		for (k =0; k<d;k++) {
			printf("%f ", grid.cells[i].tuple_list[j].coords[k]);
                        if (EVOLVE_TRACK)
                              fprintf(ofp, "%f ", grid.cells[i].tuple_list[j].coords[k]); 
                }
                if (EVOLVE_TRACK)
                        fprintf(ofp, "\n");
                printf("\n");	
                }
        if (EVOLVE_TRACK)
        {   fclose(ofp);
            file_no++;
        }
}

void print_clusters() {
	int i;

	if (RESULT || RESULT_PTS){ 
		for (i=0; i< number_of_cells; i++)
			if (grid.cells[i].part_of_cluster != -1) {
				if (RESULT)
					printf("RESULT: Cell %i with tuple count %i is part of cluster #%i and window tuple count is %i \n",i,grid.cells[i].tuple_count,grid.cells[i].part_of_cluster,window_summary.tuple_count);
				if (RESULT_PTS) {
					print_points_of_clusters(i);
	                        }		
                }
	}
}

// overloade print_clusters function
void print_clustersESL(clusterReturn* tmp) {
        /*** FILE OUTPUT UFF **/
        char fn[100] = "./output/";
        sprintf(fn,"gnudata.dat", file_no);
        ofp = fopen(fn, "w");
        int i, j;
        windowId++;
        /** DONE WITH FILE OUTPUT STUFF **/
        /*if ( newi < number_of_cells) {
            if (grid.cells[newi].part_of_cluster != -1) {
                 if (newj < grid.cells[newi].tuple_count) {
                      printf("recieved i:%i and j%i and number of cells is %i cells in grid %i\n", newi, newj, number_of_cells, grid.cells[newi].tuple_count);       
                      (*tmp).myWindowid = windowId;
                      (*tmp).myClassid = grid.cells[newi].part_of_cluster;
                      (*tmp).myx = grid.cells[newi].tuple_list[newj].coords[0];
                      (*tmp).myy = grid.cells[newi].tuple_list[newj].coords[1];
                 }
                 if (newj >= grid.cells[newi].tuple_count) {
                      (*tmp).myClassid = -1;
                 }
            }
            
        }
        if (newi >= number_of_cells) {
            (*tmp).myClassid = -2;
        } 
	*/ 
if ((RESULT || RESULT_PTS)){ 
 
		for (i=0; i < number_of_cells; i++){
			if (grid.cells[i].part_of_cluster != -1) {
				if (RESULT){
					//printf("RESULT: Cell %i with tuple count %i is part of cluster #%i and window tuple count is %i \n",i,grid.cells[i].tuple_count,grid.cells[i].part_of_cluster,window_summary.tuple_count);
                                          for (j=0; j < grid.cells[i].tuple_count; j++) {
                                          (*tmp).myWindowid[((*tmp).mySize)%WINDOW_SIZE] = windowId;
                                          (*tmp).myClassid[((*tmp).mySize)%WINDOW_SIZE] = grid.cells[i].part_of_cluster;
					  (*tmp).myx[((*tmp).mySize)%WINDOW_SIZE] = grid.cells[i].tuple_list[j].coords[0];
                                          (*tmp).myy[((*tmp).mySize)%WINDOW_SIZE] = grid.cells[i].tuple_list[j].coords[1];
                                          (*tmp).mySize++;  // change to ++ .. this is just to test 
				//	            printf("windowId=%i x=%f y=%f class=%i \n",windowId,grid.cells[i].tuple_list[j].coords[0], grid.cells[i].tuple_list[j].coords[1], grid.cells[i].part_of_cluster);
                                                    fprintf(ofp,"%f %f\n", grid.cells[i].tuple_list[j].coords[0], grid.cells[i].tuple_list[j].coords[1]);
                              //(*tmp).myWindowid = windowId;
			      //(*tmp).myClassid = grid.cells[i].part_of_cluster;
			      //(*tmp).myx = grid.cells[i].tuple_list[j].coords[0];
			      //(*tmp).myy = grid.cells[i].tuple_list[j].coords[1];
					           //sprintf(*tmp,"windowId=%i x=%f y=%f class=%i",windowId,grid.cells[i].tuple_list[j].coords[0], grid.cells[i].tuple_list[j].coords[1], grid.cells[i].part_of_cluster);
                      //printf("%f ", grid.cells[i].tuple_list[j].coords[k]);
                                          }
                                   }}
				if (RESULT_PTS) {
					print_points_of_clusters(i);
	                        }		
                }
	}
      
}
