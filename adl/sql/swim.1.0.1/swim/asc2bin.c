#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "fptree.h"
#include "memmgr.h" 
#include "timeval.h"

int trans_avg_len = 0;
int no_items = 0;
int min_supp = 0;
int window_size = 0; 

inline int max(int a, int b)
{
	return (a>b ? a : b );
}

int main(int argc, char *argv[]){
	FILE * fin = NULL, * fout = NULL;
	int tid,i,itemName,totalItems = 0,rows = 0;
	int len;
	Trans trans;
	int no_items = 50000;
	trans.items = (int*)malloc(sizeof(int)*no_items);
	
	if (argc != 3){
		fprintf(stderr,"Usage: %s inputfile outputfile\n",argv[0]);
		return -1;
	}
	
//Opening input file	
	if (!(fin = fopen(argv[1],"r"))){
		fprintf(stderr,"Cannot open file for reading: %s\n",argv[1]);
		return 1;
	}

//Opening output file	
	if (!(fout = fopen(argv[2],"wb"))){
		fprintf(stderr,"Cannot open file for writing: %s\n",argv[2]);
		return 1;
	}
	
	while (1){
		if (fscanf(fin,"%d %d %d",&tid,&tid, &len)==EOF)
			goto finish;	
		
		for (i=0; i<len; i++){
			fscanf(fin,"%d",&itemName);
			trans.items[i] = itemName;// -1;// To consider the order and rev_order
		}
		totalItems += len;
		rows ++;
		trans.len = len;
		fwrite(&trans.len,sizeof(trans.len),1,fout);
		//Here we just sort the transaction before insertion,
		sortTran(&trans);
		fwrite(trans.items,sizeof(int),trans.len,fout);
	}

finish:	
	fclose(fout);	
	fclose(fin);
	free(trans.items);
	
	printf("Total %d items, %d rows were written.\n",totalItems,rows);	
	return 0;
}

// vim:ts=2:sw=2:cindent
