#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fptree.h"
#include "memmgr.h"



Block * all_blocks[1000]; //should be changed
int current_block_number = -1;

int temp_max_nodes_num = 0, temp_cur_nodes_num = 0;

Node * new_node()
{
	static Block * block;
	int i;
	Node * node;
	temp_cur_nodes_num ++;
	if (temp_cur_nodes_num > temp_max_nodes_num)
		temp_max_nodes_num = temp_cur_nodes_num;

	for (i=current_block_number ; i>=0 ; i--){
		if (all_blocks[i]->remains)
			break;
	}
	if (i == -1){ // no free space in the current blocks
		current_block_number ++;
		all_blocks[current_block_number] = (Block *)malloc(sizeof(Block));
		block = all_blocks[current_block_number];
		block->remains = BLOCK_SIZE;
		memset(block->mem,0,BLOCK_SIZE*sizeof(Node));
		for (i=0; i<BLOCK_SIZE ; i++){
			block->fat[i]=BLOCK_SIZE - i -1;///
		}
		i = current_block_number;
//		fprintf(stderr,"New block created. current_block_number = %d, start=%x, end=%x.\n",current_block_number,all_blocks[current_block_number]->mem,all_blocks[current_block_number]->mem+BLOCK_SIZE-1);
	}
	block = all_blocks[i];
	
	node = &(block->mem[ block->fat[ --block->remains ] ]);
	memset(node,0,sizeof(Node));

//	fprintf(stderr,"Node %d from block %d allocated.\n",block->fat[block->remains],i);
	//fprintf(stderr,"A %d-%d\n",block->fat[block->remains],i);
	return node;
	
}

void delete_node(Node * node){
	int i;
	Block * block;
	temp_cur_nodes_num --;
	for (i=current_block_number ; i>=0 ; i--){
		block = all_blocks[i];
		if ( node >= &(block->mem[0]) && node < &(block->mem[0])+BLOCK_SIZE)
			break;	
	}

	if (i == -1){
		fprintf(stderr,"This is an invalid pointer to be deleted. Address=%x\n",node);
		return;
	}

	block->fat[block->remains] = (node - &(block->mem[0])); // to be checked
	block->remains ++;
//	fprintf(stderr,"Node %d from block %d deleted.\n",(node - &(block->mem[0])),i);
//	fprintf(stderr,"D %d-%d\n",(node - &(block->mem[0])),i);
}

void release_memory()
{
	int i;
	for (i=0; i<=current_block_number ; i++ )
		free(all_blocks[i]);
}

void debug_memory()
{
	int i;
	int sum_of_frees = 0;
	for (i=0 ; i<=current_block_number ; i++){
		sum_of_frees += all_blocks[i]->remains;
//		fprintf(stderr,"Block %d: free %d \n",i,all_blocks[i]->remains);
	}
	fprintf(stderr,"---------------------------------\n");
	fprintf(stderr,"Total blocks=%d,total nodes=%d,total free=%d\n",
			current_block_number+1,(current_block_number+1)*BLOCK_SIZE-sum_of_frees,sum_of_frees);
	
}

// vim:ts=2:sw=2
