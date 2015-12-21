#include "swimlib.h"

inline int max(int a, int b)
{
	return (a>b ? a : b );
}

int ReadChunkToFP_b(struct SwimFisStatus * status, int index, int chunksize)
{
	int t, i;
	Tree * FP;
	int readPointer;
//inserting status->trans of the current window into the tree
	status->trans.support = 1;
	fprintf(stderr,"Starting to ReadChunkToFP.\n");
	FP = status->localFPs[index];
	status->local_all_trans_len[index] = 0;
	empty_tree(FP);
	t = chunksize;
	readPointer = 0;	
	while (t--){
		status->trans.len = status->StreamMillBuffer[readPointer++];
		if (readPointer + status->trans.len > status->StreamMillPtr)
			return 2;//Unexpected end of file
		
		for (i=0; i<status->trans.len; ++i)
			status->trans.items[i] = status->StreamMillBuffer[readPointer+i];
		readPointer += status->trans.len;
		//We expect to read in 0..no_items-1 format and also write so
		//fwrite(&status->trans.len,sizeof(status->trans.len),1,fout);
		//fwrite(status->trans.items,sizeof(int),status->trans.len,fout);
		status->local_all_trans_len[index] += status->trans.len+1;
/*
printf("transT %d:",status->trans.len);
for (i=0; i<status->trans.len;++i) printf(" %d",status->trans.items[i]);
printf("\n");
*/
		InsertTrans(FP,status->trans);
	}
	status->trans.len = 0;

	return 0;
}

/*
int main(int argc, char *argv[]){
	int i,windowid = 0,err, hist, deleted;
	Tree * status->tree, * status->patternTree, * status->temp_patternTree;
	int * status->order,*status->rev_order;
	FILE * instream;
	int status->temp_pat_num;
	int status->temp_created;
	int status->index, status->new_index, status->expired_index;
	
//mtrace ();
	
	if (argc != 8){
		fprintf(stderr,"Job: Memory-based SWIM.\n");
		fprintf(stderr,"Usage: %s trans_avg_len window_size slide_size L_delay_max no_items min_supp_10000 instream\n",argv[0]);
		return -1;
	}
//printf("Node size=%d,Block size=%d,int=%d,short=%d,int *=%d,unsigned char=%d\n",sizeof(Node),sizeof(Block),sizeof(int),sizeof(short),sizeof(int *),sizeof(unsigned char));

	status->trans_avg_len = atoi(argv[1]);
	status->window_size = atoi(argv[2]);
	status->slide_size = atoi(argv[3]);
	status->L_delay_max = atoi(argv[4]);
	status->no_items = atoi(argv[5]);
	status->global_min_supp = ceil(atoi(argv[6]) * status->window_size / 10000.0);
	status->slide_min_supp = ceil(atoi(argv[6]) * status->slide_size / 10000.0);
	printf("# status->global_min_supp=%d,status->slide_min_supp=%d,status->window_size=%d,status->slide_size=%d,status->L_delay_max=%d\n",status->global_min_supp,status->slide_min_supp,status->window_size,status->slide_size,status->L_delay_max);

	status->no_slides = status->window_size / status->slide_size;
	if (status->window_size != (status->no_slides*status->slide_size) || status->L_delay_max < 0 || status->L_delay_max >= status->no_slides){
		fprintf(stderr,"Error:status->slide_size must be a divisor of status->window_size.\n");
		return 1;	
	}
//Opening file for first scan	
	if (!(instream = fopen(argv[7],"rb"))){
		fprintf(stderr,"Cannot open file To read %s\n",argv[7]);
		return 1;
	}
	
//Allocating memories and initialization	
	status->trans.items = (int*)malloc(sizeof(int)*status->no_items);
	status->archive_fpos = (long*)malloc(sizeof(long)*(status->no_slides+1));
	status->localFPs = (Tree **)malloc((status->no_slides+1)*sizeof(Tree *));
	for (i=0; i<=status->no_slides; i++)
		status->localFPs[i] = new_tree(status->no_items);
	status->local_all_trans_len = (int *)malloc((status->no_slides+1)*sizeof(int));
//itemName is what we read/write from/in the input/output
//But itemNo is our ordering over single items
// In SWIM implementations we assumet the single item numbers exatly like the output of IBM 
// generator:	
//Basically, status->order[itemName] = itemNo	
//For the first scan, itemNo = status->order[itemName] = itemName
//Also recall that the status->order's status->index is always between 0 and status->no_items-1
//And that's why we do not say malloc(sizeof(int)*(status->no_items+1)
	status->order = (int*)malloc(sizeof(int)*(status->no_items));
	status->rev_order = (int*)malloc(sizeof(int)*(status->no_items));
	for (i=0; i<status->no_items; i++){
		status->order[i] = i;
		status->rev_order[i] = i;
	}

//Notice that 0<= itemNo <= status->no_items-1
	status->patternTree = new_tree(status->no_items);
	status->temp_patternTree = new_tree(status->no_items);

//************************************************
//Initializing the window by reading 0..n-1 slides
	printf("#slide_time\tlast_trans\tpatTree_size\tnew_pats\tpruned_pats\tneed_history\n");
	for (windowid=0; windowid<status->no_slides ; windowid++)
	{
		status->new_index = windowid%(status->no_slides+1);
		resetTime();
		if ((err = ReadChunkToFP_b(status, status->new_index, status->slide_size)))
		{
			if (err == 1)//End of file
				break;
			else{
				fprintf(stderr,"Bad error occured. Code=%d\n",err);
				return 1;
			}
		}
		status->tree = status->localFPs[status->new_index];
		status->all_trans_len = status->local_all_trans_len[status->new_index];
		fprintf(stderr,"Time spent to create FP-status->tree and save to FP: %d\n",howlong());
		//Now current slice is ready for mining
		fprintf(stderr,"Slice %d was read (status->all_trans_len=%d): >>>>>>>>>>\n",windowid,status->all_trans_len);
		//resetTime();
//TODO: update last to windowid, and start to 0
		status->sum_all_pat_lens += Mine(status->tree, &status->trans, status->slide_min_supp, status->rev_order, status->all_trans_len, status->patternTree,0,windowid);//Insert pattern in a status->tree
		printf("%d",howlong());
		status->temp_pat_num = getTreeStatistics(NULL,status->patternTree);
		printf("\t%d\t%d\t0\t0\t0\n",(windowid+1)*status->slide_size, status->temp_pat_num);
//printf("FP:id=%d,status->index=%d.\n",windowid, status->new_index);
//		showSubTree(&status->tree->root,0,status->rev_order,1);
//printf("Total PT:id=%d,status->index=%d.\n",windowid, status->new_index);
//		showSubTree(&status->patternTree->root,0,status->rev_order,1);
	}//Next window

	fprintf(stderr,"Main window starts moving....\n");
//************************************************
//TODO: After this line every thing needs re-evaluation!!!!!!!
//Start the main loop from slices n...forever
	for (windowid=status->no_slides; 1 ; windowid++)
	{
//Mining new arrived
		status->all_trans_len = 0;
		status->new_index = windowid%(status->no_slides+1);
		status->expired_index = (windowid - status->no_slides)%(status->no_slides+1);
		resetTime();
		if ((err = ReadChunkToFP_b(status, status->new_index, status->slide_size)))
		{
			if (err == 1)//End of file
				break;
			else{
				fprintf(stderr,"Bad error occured. Code=%d\n",err);
				return 1;
			}
		}
		status->tree = status->localFPs[status->new_index];
		status->all_trans_len = status->local_all_trans_len[status->new_index];
		fprintf(stderr,"Slice %d was read (status->all_trans_len=%d): >>>>>>>>>>\n",windowid,status->all_trans_len);
		status->sum_all_pat_lens += Mine(status->tree, &status->trans, status->slide_min_supp, status->rev_order, status->all_trans_len, status->temp_patternTree, 0, windowid);//Insert pattern in a status->tree

//printf("FP:id=%d,status->index=%d.\n",windowid,status->new_index);
//		showSubTree(&status->tree->root,0,status->rev_order,1);
//printf("new PT:id=%d,status->index=%d.\n",windowid, status->new_index);
//		showSubTree(&status->temp_patternTree->root,0,status->rev_order,1);
		fprintf(stderr,"Time to Mine FP-status->tree and import into pattern status->tree: %d\n",howlong());
		for (hist=windowid-1; hist>= windowid-(status->no_slides -status->L_delay_max -1); hist--)
		{
			status->index = hist%(status->no_slides+1);
			status->tree = status->localFPs[status->index];
			status->all_trans_len = status->local_all_trans_len[status->index];
			fprintf(stderr,"Slice %d was re-accessed for less delay(status->all_trans_len=%d): >>>>>>>>>>\n",hist,status->all_trans_len);
			status->max_needed_mem = max(status->all_trans_len, status->sum_all_pat_lens);
			Rec_Verify(status->tree,status->temp_patternTree,0,status->max_needed_mem,status->rev_order,0);
			//Verify(status->tree,& status->patternTree->root, 0,status->rev_order);
//printf("exp FP:id=%d,status->index=%d.\n",windowid,status->expired_index);
//		showSubTree(&status->tree->root,0,status->rev_order,1);
//printf("expired PT:id=%d,status->index=%d.\n",windowid, status->expired_index);
//		showSubTree(&status->temp_patternTree->root,0,status->rev_order,1);
		}

//printf("Uninon1\n");
//showSubTree(&status->patternTree->root, 0,status->rev_order, 1);
//printf("Uninon2\n");
//showSubTree(&status->temp_patternTree->root, 0, status->rev_order, 1);
		status->temp_created = UnionTrees(status->patternTree,status->temp_patternTree,windowid,status->max_needed_mem);
//printf("Uninon result:\n");
//showSubTree(&status->patternTree->root, 0,status->rev_order, 1);
		empty_tree(status->temp_patternTree);	
//Reading expired chunk
		status->tree=status->localFPs[status->expired_index];
//printf("The status->tree to expire\n");
//showSubTree(&status->tree->root, 0,status->rev_order, 1);
		status->all_trans_len=status->local_all_trans_len[status->expired_index];
		
		fprintf(stderr,"Slice %d was re-accessed for expiration (status->all_trans_len=%d): >>>>>>>>>>\n",windowid - status->no_slides,status->all_trans_len);
		status->max_needed_mem = max(status->all_trans_len, status->sum_all_pat_lens);
//Verifying status->patternTree against expired
		Rec_Verify(status->tree,status->patternTree,0,status->max_needed_mem,status->rev_order,0);
		//Verify(status->tree,& status->patternTree->root, 0,status->rev_order);

//Pruning
		deleted = PruneTree(status->patternTree,windowid - status->no_slides +1);
		printf("%d",howlong());
		status->temp_pat_num = getTreeStatistics(NULL,status->patternTree);
		printf("\t%d\t%d\t%d\t%d",(windowid+1)*status->slide_size, status->temp_pat_num, status->temp_created,deleted);
		status->temp_pat_num = CountNodes(status->patternTree,windowid-status->no_slides+2);
		printf("\t%d\n",status->temp_pat_num);
	}//Next window


	//Finish up
	fclose(instream);
	return 0;
	fprintf(stderr, "Releasing the memory\n");	fflush(stderr);
	free(status->trans.items);
	free(status->archive_fpos);
	free(status->localFPs);
	free(status->local_all_trans_len);
	free(status->order);
	free(status->rev_order);
	release_memory();

}
*/ //main commented out

void cleanUp(struct SwimFisStatus * status)
{
	fprintf(stderr, "Releasing the memory\n");	fflush(stderr);
	if (status->StreamMillBuffer) 
		free(status->StreamMillBuffer);
	if (status->trans.items)
		free(status->trans.items);
	if (status->archive_fpos)
		free(status->archive_fpos);
	if (status->localFPs)
		free(status->localFPs);
	if (status->local_all_trans_len)
		free(status->local_all_trans_len);
	if (status->order)
		free(status->order);
	if (status->rev_order)
		free(status->rev_order);
	release_memory();
}

//struct iExt_ {
//	  int length;
//	  int *pt;
//};
//return the number of characters that were read
int addTuple(int length, int * pt, int * buf, int bufPtr)
{
	int i;
	buf[bufPtr] = length;
	for (i=0; i<length; ++i)
	{
		buf[bufPtr+1+i] = pt[i];		
	}

	return length + 1;
}

int ProcessMain(int windowid, struct SwimFisStatus * status)
{
	int hist, err, deleted;
//Initializing the window by reading 0..n-1 slides
/*int i;
for (i=0; i<status->no_slides+1; ++i){
printf("Tree info %d:\n",i);
showSubTree(&status->localFPs[i]->root, 0, status->rev_order, 1);
}
*/
	printf("#slide_time\tlast_trans\tpatTree_size\tnew_pats\tpruned_pats\tneed_history\n");
	if ((windowid >= 0) && (windowid < status->no_slides))
	{
		status->new_index = windowid%(status->no_slides+1);
		resetTime();
		if ((err = ReadChunkToFP_b(status, status->new_index, status->slide_size)))
		{
			if (err == 1)//End of file
				return 10;//break;
			else{
				fprintf(stderr,"Bad error occured. Code=%d\n",err);
				return 1;
			}
		}
		status->tree = status->localFPs[status->new_index];
		status->all_trans_len = status->local_all_trans_len[status->new_index];
		fprintf(stderr,"Time spent to create FP-status->tree and save to FP: %d\n",howlong());
		//Now current slice is ready for mining
		fprintf(stderr,"Slice %d was read (status->all_trans_len=%d): >>>>>>>>>>\n",windowid,status->all_trans_len);
		//resetTime();
//TODO: update last to windowid, and start to 0
		status->sum_all_pat_lens += Mine(status->tree, &status->trans, status->slide_min_supp, status->rev_order, status->all_trans_len, status->patternTree,0,windowid);//Insert pattern in a status->tree
		printf("%d",howlong());
		status->temp_pat_num = getTreeStatistics(NULL,status->patternTree);
		printf("\t%d\t%d\t0\t0\t0\n",(windowid+1)*status->slide_size, status->temp_pat_num);
//printf("FP:id=%d,status->index=%d.\n",windowid, status->new_index);
//		showSubTree(&status->tree->root,0,status->rev_order,1);
//printf("Total PT:id=%d,status->index=%d.\n",windowid, status->new_index);
//		showSubTree(&status->patternTree->root,0,status->rev_order,1);
	} 
	else if (windowid >= status->no_slides) 
	{
		fprintf(stderr,"Main window has started moving....\n");
	//************************************************
	//TODO: After this line every thing needs re-evaluation!!!!!!!
	
	//Mining new arrived
		status->all_trans_len = 0;
		status->new_index = windowid%(status->no_slides+1);
		status->expired_index = (windowid - status->no_slides)%(status->no_slides+1);
		resetTime();
		if ((err = ReadChunkToFP_b(status, status->new_index, status->slide_size)))
		{
			if (err == 1)//End of file
				return 10; //break;
			else{
				fprintf(stderr,"Bad error occured. Code=%d\n",err);
				return 1;
			}
		}
		status->tree = status->localFPs[status->new_index];
		status->all_trans_len = status->local_all_trans_len[status->new_index];
		fprintf(stderr,"Slice %d was read (status->all_trans_len=%d): >>>>>>>>>>\n",windowid,status->all_trans_len);
		status->sum_all_pat_lens += Mine(status->tree, &status->trans, status->slide_min_supp, status->rev_order, status->all_trans_len, status->temp_patternTree, 0, windowid);//Insert pattern in a status->tree

//printf("FP:id=%d,status->index=%d.\n",windowid,status->new_index);
//		showSubTree(&status->tree->root,0,status->rev_order,1);
//printf("new PT:id=%d,status->index=%d.\n",windowid, status->new_index);
//		showSubTree(&status->temp_patternTree->root,0,status->rev_order,1);
		fprintf(stderr,"Time to Mine FP-status->tree and import into pattern status->tree: %d\n",howlong());
		for (hist=windowid-1; hist>= windowid-(status->no_slides -status->L_delay_max -1); --hist)
		{
			status->index = hist%(status->no_slides+1);
			status->tree = status->localFPs[status->index];
			status->all_trans_len = status->local_all_trans_len[status->index];
			fprintf(stderr,"Slice %d was re-accessed for less delay(status->all_trans_len=%d): >>>>>>>>>>\n",hist,status->all_trans_len);
			status->max_needed_mem = max(status->all_trans_len, status->sum_all_pat_lens);
			Rec_Verify(status->tree,status->temp_patternTree,0,status->max_needed_mem,status->rev_order,0);
			//Verify(status->tree,& status->patternTree->root, 0,status->rev_order);
//printf("exp FP:id=%d,status->index=%d.\n",windowid,status->expired_index);
//		showSubTree(&status->tree->root,0,status->rev_order,1);
//printf("expired PT:id=%d,status->index=%d.\n",windowid, status->expired_index);
//		showSubTree(&status->temp_patternTree->root,0,status->rev_order,1);
		}

//printf("Uninon1\n");
//showSubTree(&status->patternTree->root, 0,status->rev_order, 1);
//printf("Uninon2\n");
//showSubTree(&status->temp_patternTree->root, 0, status->rev_order, 1);

		status->temp_created = UnionTrees(status->patternTree,status->temp_patternTree,windowid,status->sum_all_pat_lens);
//printf("Uninon result:\n");
//showSubTree(&status->patternTree->root, 0,status->rev_order, 1);
		empty_tree(status->temp_patternTree);	
//Reading expired chunk
		status->tree=status->localFPs[status->expired_index];
//printf("The status->tree to expire\n");
//showSubTree(&status->tree->root, 0,status->rev_order, 1);
		status->all_trans_len=status->local_all_trans_len[status->expired_index];
		
		fprintf(stderr,"Slice %d was re-accessed for expiration (status->all_trans_len=%d): >>>>>>>>>>\n",windowid - status->no_slides,status->all_trans_len);
		status->max_needed_mem = max(status->all_trans_len, status->sum_all_pat_lens);
//Verifying status->patternTree against expired
		Rec_Verify(status->tree,status->patternTree,0,status->max_needed_mem,status->rev_order,0);
		//Verify(status->tree,& status->patternTree->root, 0,status->rev_order);

//Pruning
		deleted = PruneTree(status->patternTree,windowid - status->no_slides +1);
		printf("%d",howlong());
		status->temp_pat_num = getTreeStatistics(NULL,status->patternTree);
		printf("\t%d\t%d\t%d\t%d",(windowid+1)*status->slide_size, status->temp_pat_num, status->temp_created,deleted);
		status->temp_pat_num = CountNodes(status->patternTree,windowid-status->no_slides+2);
		printf("\t%d\n",status->temp_pat_num);
	}


	return 0;
}

//void ProcessExpire(int windowid, struct SwimFisStatus * status)
//{
	
//}

//Libraries and other functions ended here 
