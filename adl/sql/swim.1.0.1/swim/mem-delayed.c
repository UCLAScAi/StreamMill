#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "fptree.h"
#include "memmgr.h" 
#include "timeval.h"

#include <mcheck.h>

int trans_avg_len = 0;
int no_items = 0;
int global_min_supp = 0;
int slide_min_supp = 0;
int window_size = 0; 
int slide_size = 0;
int L_delay_max = 0;
int no_slides = 0;
long * archive_fpos;
Trans trans;
int all_trans_len, sum_all_pat_lens = 0, max_needed_mem = 0;
Tree ** localFPs;
int * local_all_trans_len;

inline int max(int a, int b)
{
	return (a>b ? a : b );
}

int ReadChunkToFP(FILE * fin, int index, int chunksize)
{
	int t ;
	trans.support = 1;
	Tree * FP;
//inserting trans of the current window into the tree
	fprintf(stderr,"Starting to ReadChunkToFP.\n");
	FP = localFPs[index];
	local_all_trans_len[index] = 0;
	empty_tree(FP);
	t = chunksize;
	while (t--){
		if (fread(&trans.len,sizeof(trans.len),1,fin)==0){
			if (t == slide_size-1)
				return 1;//End of file
			else
				return 2;//Unexpected end of file
		}
		
		if (fread(trans.items,sizeof(int),trans.len,fin) < trans.len)
			return 3;//Very bad end of file
		//We expect to read in 0..no_items-1 format and also write so
		//fwrite(&trans.len,sizeof(trans.len),1,fout);
		//fwrite(trans.items,sizeof(int),trans.len,fout);
		local_all_trans_len[index] += trans.len+1;
		InsertTrans(FP,trans);
	}
	trans.len = 0;

	return 0;
}

int LoadChunkFromDiskToFP(FILE * fin, int index, Tree * FP)
{
	int t = slide_size;
	if (fseek(fin,archive_fpos[index % (no_slides+1)],SEEK_SET))
	{
		fprintf(stderr,"Invalid pointer.\n");
		return 4;
	}
	trans.support = 1;
	//reading and inserting trans of the current chunk into the tree
	fprintf(stderr,"Starting to LoadChunkFromDiskToFP.\n");
	while (t--){
		if (fread(&trans.len,sizeof(trans.len),1,fin)==0){
			if (t == slide_size-1)
				return 1;//End of file
			else
				return 2;//Unexpected end of file
		}
		
		if (fread(trans.items,sizeof(int),trans.len,fin) < trans.len)
			return 3;//Very bad end of file
		//We expect to read in 0..no_items-1 format and also write so
		
		all_trans_len += trans.len+1;
		InsertTrans(FP,trans);
	}
	trans.len = 0;
	
	return 0;	
}

int main(int argc, char *argv[]){
	int i,windowid = 0,err, hist, deleted;
	Tree * tree, * patternTree, * temp_patternTree;
	int * order,*rev_order;
	FILE * instream;
	int temp_pat_num;
	int temp_created;
	int index, new_index, expired_index;
	
//mtrace ();
	
	if (argc != 8){
		fprintf(stderr,"Job: Memory-based SWIM.\n");
		fprintf(stderr,"Usage: %s trans_avg_len window_size slide_size L_delay_max no_items min_supp_10000 instream\n",argv[0]);
		return -1;
	}
//printf("Node size=%d,Block size=%d,int=%d,short=%d,int *=%d,unsigned char=%d\n",sizeof(Node),sizeof(Block),sizeof(int),sizeof(short),sizeof(int *),sizeof(unsigned char));

	trans_avg_len = atoi(argv[1]);
	window_size = atoi(argv[2]);
	slide_size = atoi(argv[3]);
	L_delay_max = atoi(argv[4]);
	no_items = atoi(argv[5]);
	global_min_supp = ceil(atoi(argv[6]) * window_size / 10000.0);
	slide_min_supp = ceil(atoi(argv[6]) * slide_size / 10000.0);
	printf("# global_min_supp=%d,slide_min_supp=%d,window_size=%d,slide_size=%d,L_delay_max=%d\n",global_min_supp,slide_min_supp,window_size,slide_size,L_delay_max);

	no_slides = window_size / slide_size;
	if (window_size != (no_slides*slide_size) || L_delay_max < 0 || L_delay_max >= no_slides){
		fprintf(stderr,"Error:slide_size must be a divisor of window_size.\n");
		return 1;	
	}
//Opening file for first scan	
	if (!(instream = fopen(argv[7],"rb"))){
		fprintf(stderr,"Cannot open file To read %s\n",argv[7]);
		return 1;
	}
	
//Allocating memories and initialization	
	trans.items = (int*)malloc(sizeof(int)*no_items);
	archive_fpos = (long*)malloc(sizeof(long)*(no_slides+1));
	localFPs = (Tree **)malloc((no_slides+1)*sizeof(Tree *));
	for (i=0; i<=no_slides; i++)
		localFPs[i] = new_tree(no_items);
	local_all_trans_len = (int *)malloc((no_slides+1)*sizeof(int));
//itemName is what we read/write from/in the input/output
//But itemNo is our ordering over single items
// In SWIM implementations we assumet the single item numbers exatly like the output of IBM 
// generator:	
//Basically, order[itemName] = itemNo	
//For the first scan, itemNo = order[itemName] = itemName
//Also recall that the order's index is always between 0 and no_items-1
//And that's why we do not say malloc(sizeof(int)*(no_items+1)
	order = (int*)malloc(sizeof(int)*(no_items));
	rev_order = (int*)malloc(sizeof(int)*(no_items));
	for (i=0; i<no_items; i++){
		order[i] = i;
		rev_order[i] = i;
	}

//Notice that 0<= itemNo <= no_items-1
	patternTree = new_tree(no_items);
	temp_patternTree = new_tree(no_items);

//************************************************
//Initializing the window by reading 0..n-1 slides
	printf("#slide_time\tlast_trans\tpatTree_size\tnew_pats\tpruned_pats\tneed_history\n");
	for (windowid=0; windowid<no_slides ; windowid++)
	{
		new_index = windowid%(no_slides+1);
		resetTime();
		if ((err = ReadChunkToFP(instream, new_index, slide_size)))
		{
			if (err == 1)//End of file
				break;
			else{
				fprintf(stderr,"Bad error occured. Code=%d\n",err);
				return 1;
			}
		}
		tree = localFPs[new_index];
		all_trans_len = local_all_trans_len[new_index];
		fprintf(stderr,"Time spent to create FP-tree and save to FP: %d\n",howlong());
		//Now current slice is ready for mining
		fprintf(stderr,"Slice %d was read (all_trans_len=%d): >>>>>>>>>>\n",windowid,all_trans_len);
		//resetTime();
//TODO: update last to windowid, and start to 0
		sum_all_pat_lens += Mine(tree, &trans, slide_min_supp, rev_order, all_trans_len, patternTree,0,windowid);//Insert pattern in a tree
		printf("%d",howlong());
		temp_pat_num = getTreeStatistics(NULL,patternTree);
		printf("\t%d\t%d\t0\t0\t0\n",(windowid+1)*slide_size, temp_pat_num);
//printf("FP:id=%d,index=%d.\n",windowid, new_index);
//		showSubTree(&tree->root,0,rev_order,1);
//printf("Total PT:id=%d,index=%d.\n",windowid, new_index);
//		showSubTree(&patternTree->root,0,rev_order,1);
	}//Next window

	fprintf(stderr,"Main window starts moving....\n");
//************************************************
//TODO: After this line every thing needs re-evaluation!!!!!!!
//Start the main loop from slices n...forever
	for (windowid=no_slides; 1 ; windowid++)
	{
//Mining new arrived
		all_trans_len = 0;
		new_index = windowid%(no_slides+1);
		expired_index = (windowid - no_slides)%(no_slides+1);
		resetTime();
		if ((err = ReadChunkToFP(instream, new_index, slide_size)))
		{
			if (err == 1)//End of file
				break;
			else{
				fprintf(stderr,"Bad error occured. Code=%d\n",err);
				return 1;
			}
		}
		tree = localFPs[new_index];
		all_trans_len = local_all_trans_len[new_index];
		fprintf(stderr,"Slice %d was read (all_trans_len=%d): >>>>>>>>>>\n",windowid,all_trans_len);
		sum_all_pat_lens += Mine(tree, &trans, slide_min_supp, rev_order, all_trans_len, temp_patternTree, 0, windowid);//Insert pattern in a tree

//printf("FP:id=%d,index=%d.\n",windowid,new_index);
//		showSubTree(&tree->root,0,rev_order,1);
//printf("new PT:id=%d,index=%d.\n",windowid, new_index);
//		showSubTree(&temp_patternTree->root,0,rev_order,1);
		fprintf(stderr,"Time to Mine FP-tree and import into pattern tree: %d\n",howlong());
		for (hist=windowid-1; hist>= windowid-(no_slides -L_delay_max -1); hist--)
		{
			index = hist%(no_slides+1);
			tree = localFPs[index];
			all_trans_len = local_all_trans_len[index];
			fprintf(stderr,"Slice %d was re-accessed for less delay(all_trans_len=%d): >>>>>>>>>>\n",hist,all_trans_len);
			max_needed_mem = max(all_trans_len, sum_all_pat_lens);
			Rec_Verify(tree,temp_patternTree,0,max_needed_mem,rev_order,0);
			//Verify(tree,& patternTree->root, 0,rev_order);
//printf("exp FP:id=%d,index=%d.\n",windowid,expired_index);
//		showSubTree(&tree->root,0,rev_order,1);
//printf("expired PT:id=%d,index=%d.\n",windowid, expired_index);
//		showSubTree(&temp_patternTree->root,0,rev_order,1);
		}

//printf("Uninon1\n");
//showSubTree(&patternTree->root, 0,rev_order, 1);
//printf("Uninon2\n");
//showSubTree(&temp_patternTree->root, 0, rev_order, 1);
		temp_created = UnionTrees(patternTree,temp_patternTree,windowid,max_needed_mem);
//printf("Uninon result:\n");
//showSubTree(&patternTree->root, 0,rev_order, 1);
		empty_tree(temp_patternTree);	
//Reading expired chunk
		tree=localFPs[expired_index];
//printf("The tree to expire\n");
//showSubTree(&tree->root, 0,rev_order, 1);
		all_trans_len=local_all_trans_len[expired_index];
		
		fprintf(stderr,"Slice %d was re-accessed for expiration (all_trans_len=%d): >>>>>>>>>>\n",windowid - no_slides,all_trans_len);
		max_needed_mem = max(all_trans_len, sum_all_pat_lens);
//Verifying patternTree against expired
		Rec_Verify(tree,patternTree,0,max_needed_mem,rev_order,0);
		//Verify(tree,& patternTree->root, 0,rev_order);

//Pruning
		deleted = PruneTree(patternTree,windowid - no_slides +1);
		printf("%d",howlong());
		temp_pat_num = getTreeStatistics(NULL,patternTree);
		printf("\t%d\t%d\t%d\t%d",(windowid+1)*slide_size, temp_pat_num, temp_created,deleted);
		temp_pat_num = CountNodes(patternTree,windowid-no_slides+2);
		printf("\t%d\n",temp_pat_num);
	}//Next window


	//Finish up
	fclose(instream);
	return 0;
	fprintf(stderr, "Releasing the memory\n");	fflush(stderr);
	free(trans.items);
	free(archive_fpos);
	free(localFPs);
	free(local_all_trans_len);
	free(order);
	free(rev_order);
	release_memory();

}

// vim:ts=2:sw=2:cindent
