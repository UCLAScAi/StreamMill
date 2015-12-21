#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "histmgr.h"

HistoryBlock * all_hist_blocks[1000]; //should be changed
int current_hist_block_number = -1;

int temp_max_hist_num = 0, temp_cur_hist_num = 0;

History * new_history()
{
	static HistoryBlock * histblock;
	int i;
	History * history;
	temp_cur_hist_num ++;
	if (temp_cur_hist_num > temp_max_hist_num)
		temp_max_hist_num = temp_cur_hist_num;

	for (i=current_hist_block_number ; i>=0 ; i--){
		if (all_hist_blocks[i]->remains)
			break;
	}
	if (i == -1){ // no free space in the current blocks
		current_hist_block_number ++;
		all_hist_blocks[current_hist_block_number] = (HistoryBlock *)malloc(sizeof(HistoryBlock));
		histblock = all_hist_blocks[current_hist_block_number];
		histblock->remains = HIST_BLOCK_SIZE;
		memset(histblock->mem,0,HIST_BLOCK_SIZE*sizeof(History));
		for (i=0; i<HIST_BLOCK_SIZE ; i++){
			histblock->fat[i]=HIST_BLOCK_SIZE - i -1;///
		}
		i = current_hist_block_number;
//		fprintf(stderr,"New block created. current_hist_block_number = %d, start=%x, end=%x.\n",current_hist_block_number,all_hist_blocks[current_hist_block_number]->mem,all_hist_blocks[current_hist_block_number]->mem+HIST_BLOCK_SIZE-1);
	}
	histblock = all_hist_blocks[i];
	
	history = &(histblock->mem[ histblock->fat[ --histblock->remains ] ]);
//?need it?
	memset(histblock,0,sizeof(History));

//	fprintf(stderr,"History %d from histblock %d allocated.\n",histblock->fat[histblock->remains],i);
	
	return history;
}

void delete_history(History * history){
	int i;
	HistoryBlock * histblock;
	temp_cur_hist_num --;
	for (i=current_hist_block_number ; i>=0 ; i--){
		histblock = all_hist_blocks[i];
		if ( history >= &(histblock->mem[0]) && history < &(histblock->mem[0])+HIST_BLOCK_SIZE)
			break;	
	}

	if (i == -1){
		fprintf(stderr,"This is an invalid pointer to be deleted. Address=%x\n",history);
		return;
	}

	histblock->fat[histblock->remains] = (history - &(histblock->mem[0])); // to be checked
	histblock->remains ++;
//	fprintf(stderr,"History %d from histblock %d deleted.\n",(history - &(histblock->mem[0])),i);
}

// vim:ts=2:sw=2
