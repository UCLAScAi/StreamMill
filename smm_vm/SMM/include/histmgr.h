#include <string.h>

#ifndef HIST_MGR
#define HIST_MGR

#define HIST_BLOCK_SIZE	127*1024
#define BUCKETS	100

struct history{
	int freq[BUCKETS];
};

typedef struct history History;

struct history_block {
	int remains;
	History mem[HIST_BLOCK_SIZE];
	unsigned int fat[HIST_BLOCK_SIZE];
};

typedef struct history_block HistoryBlock;

History * new_history();
void delete_history(History * history);

#endif

// vim:ts=2:sw=2

