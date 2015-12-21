#include <string.h>
#include "fptree.h"

#ifndef MEM_MGR
#define MEM_MGR

#define BLOCK_SIZE	4095*1024
extern int temp_max_nodes_num, temp_cur_nodes_num; 
struct memory_block {
	int remains;
	Node mem[BLOCK_SIZE];
	unsigned int fat[BLOCK_SIZE];
};

typedef struct memory_block Block;

Node * new_node();
void delete_node(Node * node);
void release_memory();
void debug_memory();

#endif

// vim:ts=2:sw=2

