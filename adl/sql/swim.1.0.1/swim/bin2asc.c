#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "fptree.h"
#include "memmgr.h" 
#include "timeval.h"


int main(int argc, char *argv[]){
	FILE * fin = NULL, * fout = NULL;
	int i,totalItems = 0,rows = 0;
	Trans trans;
	int no_items = 1000;
	trans.items = (int*)malloc(sizeof(int)*no_items);
	
	if (argc != 3){
		fprintf(stderr,"Usage: %s inputfile outputfile\n",argv[0]);
		return -1;
	}
	
//Opening input file	
	if (!(fin = fopen(argv[1],"rb"))){
		fprintf(stderr,"Cannot open file for reading: %s\n",argv[1]);
		return 1;
	}

//Opening output file	
	if (!(fout = fopen(argv[2],"w"))){
		fprintf(stderr,"Cannot open file for writing: %s\n",argv[2]);
		return 1;
	}
	
	while (1){
		if (fread(&trans.len,sizeof(trans.len),1,fin)==0)
			goto finish;
		
		fread(trans.items,sizeof(int),trans.len,fin);
		totalItems += trans.len;
		rows ++;
		
		fprintf(fout,"%d %d %d",rows,rows,trans.len);
		for (i=0; i<trans.len; i++){
			fprintf(fout," %d",trans.items[i]);//+1);//to consider order & rev_order
		}
		fprintf(fout,"\n");
	}

finish:	
	fclose(fout);	
	fclose(fin);
	free(trans.items);
	
	printf("Total %d items, %d rows were written.\n",totalItems,rows);	
	return 0;
}

// vim:ts=2:sw=2:cindent
