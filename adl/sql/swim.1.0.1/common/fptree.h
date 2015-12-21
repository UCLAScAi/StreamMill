#ifdef SWIM
#include "histmgr.h"
#endif

#ifndef FP_TREE_H
#define FP_TREE_H

#define NIL	0
#define OK	1
#define FAIL	2
#define EXPIRED	4

//extern int cost_1, cost_2, cost_3;

struct node{
	short itemNo;
	int support;
	short flag;
	struct node * parent;
	struct node * sibling;
	struct node * leftChild;
	struct node * nextHeader;
	struct node * prevHeader;
#ifdef SWIM
	int * history;
//	History *	history;
//	unsigned char hindex;
//  unsigned char start;
//	unsigned char end;
	unsigned short start;
	unsigned short end;
#endif
	struct node * returnFreqAdr;
};

typedef struct node Node;

struct tree{
	int MaxItems;
	int TotalNodes;
	Node	root; 
	Node ** header; // header table. header[0] is a pointer to items[0]
	int * redundancy; // number of nodes for each node
	int * single_support; // single_support[0] is the single supp of items[0]
};

typedef struct tree Tree;

struct transaction{
	int len;
	int support;
	int * items;
};

typedef struct transaction Trans;

//This function assumes that we'll only have MaxItem numbers
Tree * new_tree(int MaxItem);

void empty_tree(Tree * tree);
void delete_tree(Tree * tree);

//This function assumes that all items in trans are ordered.
int InsertTransAdr(Tree * tree, Trans trans, Node * returnFreqAdr);
int InsertTrans(Tree * tree, Trans trans); // returnFreqAdr = 0


//This function assumes that we'll only have MaxItem numbers);
int RemoveTrans(Tree * tree, Trans trans, int mustExists, int deleteIfZero);

//This function is just InsertTrans, the only difference is that
// it keeps all children sorted
#ifdef SWIM
int InsertPatternSortedly(Tree * tree, Trans trans, int windowid);
#else
int InsertPatternSortedly(Tree * tree, Trans trans);
#endif

#ifdef SWIM
int Mine(Tree * tree, Trans * suffix, int minsupp,int * rev_order, int max ,Tree * patternTree,int doReport, int windowid);
#else
int Mine(Tree * tree, Trans * suffix, int minsupp,int * rev_order, int max ,Tree * patternTree,int doReport);
#endif

void Verify(Tree * fptree, Node * patternTreeRoot, int minsupp,int * rev_order);
void Conditionalize(Tree * fptree, Tree * patternTree , int max_avg_len_trans, int * rev_order);

void Rec_Verify(Tree * fptree, Tree * patternTree, int minsupp, int max_avg_len_trans, int * rev_order, int temp_cond_level);

int atomic_conditionalize(Tree * tree, Tree * result, int itemNo, int minsupp, int max_avg_len_trans, int * rev_order, Node ** witness);

int isSmallEnough(int patTreeNodes, int fpTreeNodes);
inline int estimateExecEdges(int level);

void showSubTree(Node * node, int indent, int * rev_order, int externallycalled);
void showRowSubTree(Node * node, int indent);

int * resetCounts(int MaxItem);

int * PartialDB(int max);
Node ** PartialDBAdr(int max);

int isSinglePath(Tree * tree);

void MineSubsets(Tree * tree,Trans * suffix,int minsupp,int * rev_order);
int MineAllSubsets(Tree * tree,Trans * suffix,int minsupp,int * rev_order);

#ifdef SWIM
int MineIntoPatTree(Tree * tree,Trans * suffix,Tree * PatternTree, int windowid);
#else
int MineIntoPatTree(Tree * tree,Trans * suffix,Tree * PatternTree);
#endif

//sort and remove non-frequents
void sortEliminateTran(Trans * tran, int *freq, int min_supp);
//Just sort items increasingly
void sortTran(Trans * tran);

//retunrs the number of nodes in the tree as well as printing
//more interesting statistics preceded by the given message
int getTreeStatistics(char * message,Tree * tree);
void resetTreeFreq(Tree * tree,int value);
void diffTrees(Node * left, Node * right, int * leftmore, int * common, int * rightmore);

void ExtractAllPatternsDB(Tree * patternTree,char * tablename);
#ifdef SWIM
inline int PruneSubtree(Node * node, Tree * tree, Node * leftSibling, int min_end);
#else
inline int PruneSubtree(Node * node, Tree * tree, Node * leftSibling);
#endif

#ifdef SWIM
inline int PruneSubtreeWithSiblings(Node * node, Tree * tree, Node * leftSibling, int min_end);
#else
inline int PruneSubtreeWithSiblings(Node * node, Tree * tree, Node * leftSibling);
#endif

int UnionTrees(Tree * mainPatTree, Tree * temp_PatTree, int windowid, int max_avg_len_trans);
inline int PruneTree(Tree * tree, int min_allowed_end);
inline int CountNodes(Tree * tree, int min_allowed_start);
int FindStartTime(Tree * tree, Trans trans);
int FindEndTime(Tree * tree, Trans trans);
int checkConsistentcy(Tree * tree);

void copyBackTreeFreq(Tree * tree,int largestItemId);
#endif

//set vim:ts=2:sw=2
