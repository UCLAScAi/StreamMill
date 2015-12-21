#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fptree.h"
#include "memmgr.h"
#include "timeval.h" 

#define MAX_BITS	23
int isGrayInitialized = 0;
unsigned int words[1<<MAX_BITS];
int cost_1 = 0, cost_2 = 0, cost_3 = 0;
//show be deleted all followings
//#ifdef DEVELOP
int num_of_edges = 0;
int temp_inserted_patterns = 0;
int temp_inserted_patterns_flag = 0;
#define TEMP_MAX_INSERTED_PATTERNS	350000
#define TEMP_GARBAGE	2
//#endif

void gray()
{
	register int i,level,width;
	words[0] = 0;
	for (level=0; level<MAX_BITS ; level++){
		width = 1<<level;
		for (i=0 ; i<width ;i++)
			words[2*width-i-1] = words[i] | width;
	}
}

inline void printTrans(char * msg, Trans * trans)
{
	int i;
	fprintf(stderr,"%s [ ",msg);
	for (i=0; i<trans->len; i++)
		fprintf(stderr,"%d ",trans->items[i]);
	fprintf(stderr,"]\n");
}

//This function assumes that we'll only have MaxItems numbers
Tree * new_tree(int MaxItems){
	if (!isGrayInitialized){
		gray();
		isGrayInitialized = 1;
	}	

	Tree * fptree = (Tree *)malloc(sizeof(Tree));
	memset(fptree,0,sizeof(Tree));	
	
	fptree->MaxItems = MaxItems;
	fptree->TotalNodes = 0;
	fptree->root.itemNo = -1;
	
	fptree->header = (Node **)malloc(MaxItems*sizeof(Node *));
	memset(fptree->header,0,MaxItems*sizeof(Node *));

	fptree->redundancy = (int *)malloc(MaxItems*sizeof(int));
	memset(fptree->redundancy,0,MaxItems*sizeof(int));
	
	fptree->single_support = (int *)malloc(MaxItems*sizeof(int));
	memset(fptree->single_support,0,MaxItems*sizeof(int));

	return fptree;
}

void empty_tree(Tree * tree)
{
	int i;
	Node * node, *node2;
	
	tree->TotalNodes = 0;
	tree->root.leftChild = NULL;
	for (i=0; i<tree->MaxItems ; i++ ){
		node = tree->header[i];
		while (node){
			node2 = node->nextHeader;
			delete_node(node);
			node = node2;
		}
	}
//	for (i=0; i<tree->MaxItems ; i++ ){
//		tree->redundancy[i] = tree->single_support[i] = 0;
//		tree->header[i] = NULL;
//	}
	memset(&(tree->root),0,sizeof(Node));
	tree->root.itemNo = -1;
	memset(tree->header,0,tree->MaxItems*sizeof(Node *));
	memset(tree->redundancy,0,tree->MaxItems*sizeof(int));
	memset(tree->single_support,0,tree->MaxItems*sizeof(int));
		
}
void delete_tree(Tree * tree)
{
	empty_tree(tree);
	free(tree->single_support);
	free(tree->redundancy);
	free(tree->header);
	free(tree);
}

//This function assumes that all items in trans are sorted increasingly.
//This function assumes that we'll only have MaxItems numbers);
//It's supposed to return the number of nodes it has created.
int InsertTrans(Tree * tree, Trans trans)
{
	return InsertTransAdr(tree, trans, NULL);
}

//This function assumes that all items in trans are sorted increasingly.
//This function assumes that we'll only have MaxItems numbers);
//It's supposed to return the number of nodes it has created.
int InsertTransAdr(Tree * tree, Trans trans, Node * returnFreqAdr)
//TODO: Make sure about taking care of Header table maintainance
{
	Node * node, * parent, *firstChain;
	int curItmNum = 0,curItem,i;
	int created = 0;
	
	parent = & tree->root;
	tree->root.support += trans.support;
	while (curItmNum < trans.len){//while we have sth to insert
		curItem = trans.items[curItmNum];
		node = parent->leftChild;
		while (node) // to find the proper child
			if (node->itemNo == curItem)
				break;
			else
				node = node->sibling;
		
		if (node){
				node->support += trans.support;
				if (curItmNum == trans.len-1)
					node->returnFreqAdr = returnFreqAdr;			
//			node->end = windowid;
				tree->single_support[curItem] += trans.support;
				parent = node;
				curItmNum++;
		}	else
			break;
	}
	//Now adding all items as a single path under the current parent
	for (i = curItmNum; i<trans.len ; i++){
		curItem = trans.items[i];
		node = new_node();
		created ++;
		node->itemNo = curItem;
		node->support = trans.support;
		if (i	== trans.len-1)
			node->returnFreqAdr = returnFreqAdr;
//		node->start = windowid;
//		node->end = windowid;
		node->parent = parent;
		node->sibling = parent->leftChild;
		node->leftChild = NULL;
			firstChain = tree->header[curItem];
		node->nextHeader = firstChain;
			if (firstChain){
				firstChain->prevHeader = node;
			}	
		node->prevHeader = NULL;
		tree->header[curItem] = node;

		parent->leftChild = node;

		tree->single_support[curItem] += trans.support;
		tree->redundancy[curItem]++;
		
		parent = node;
	}

	tree->TotalNodes += created;
	return created;
}

#ifdef SWIM
inline int CountNodes(Tree * tree, int min_allowed_start)
{
	Node *node;
	int i, num = 0;
	
	for (i=tree->MaxItems -1; i>=0 ; i--){
		node = tree->header[i];
		while (node){
			if (node->start >= min_allowed_start){
				num++;
			}
			node = node->nextHeader;
		}
	}
	return num;
}
#endif

#ifdef SWIM
inline int PruneTree(Tree * tree, int min_allowed_end)
{
	Node *parent, *node, *next;
	int i, deleted = 0, barzantest = 0;
	
	for (i=tree->MaxItems -1; i>=0 ; i--){
		node = tree->header[i];
		while (node){
			if (node->end < min_allowed_end){
				node->flag = EXPIRED;
				barzantest++;
			}
			node = node->nextHeader;
		}
	}
//Now those which should be deleted are marked
//Now we must fix leftChild, sibling
	for (i=tree->MaxItems -1; i>=0 ; i--){
		parent = tree->header[i];
		while (parent){
			if (parent->flag != EXPIRED){
				//first fixing leftChild
				node = parent->leftChild;
				while (node && (node->flag == EXPIRED)){
					node = node->sibling;
				}
				parent->leftChild = node;		
				//Then fixing sibling
				while (node){
					next = node->sibling;
					while (next && (next->flag == EXPIRED)){
						next = next->sibling;
					}
					node->sibling = next;
					node = next;
				}
			}
			parent = parent->nextHeader;
		}
	}	
//Also fix first layer children! 
	node = tree->root.leftChild;
	while (node && (node->flag == EXPIRED)){
		node = node->sibling;
	}
	tree->root.leftChild = node;		
	//Then fixing sibling
	while (node){
		next = node->sibling;
		while (next && (next->flag == EXPIRED)){
			next = next->sibling;
		}
		node->sibling = next;
		node = next;
	}
	
//Removing EXPIRED flags and taking care of nextHeader and prevHeader
	for (i=tree->MaxItems -1; i>=0 ; i--){
		node = tree->header[i];
		while (node){
////
			if (node->flag != EXPIRED){
				node = node->nextHeader;
				continue;
			}
			if (node->prevHeader){
				node->prevHeader->nextHeader = node->nextHeader;
			}else{
#ifdef DEBUG
				assert(tree->header[i] == node);
#endif
				tree->header[i] = node->nextHeader;
			}
			
			if (node->nextHeader)
			{
				node->nextHeader->prevHeader = node->prevHeader;
			}
			
			tree->single_support[i] -= node->support;
			tree->redundancy[i]--;
			tree->TotalNodes --;
			next = node;
			node = node->nextHeader;
			delete_node(next);
			deleted ++;
////	
		}
	}
	if (barzantest != deleted)
		fprintf(stderr,"Serious error probable=%d or %d ?\n",barzantest,deleted);
		
	return deleted;	
}
#endif

//This function is supposed to delete node and all his children
//when its leftSibling is given
//It should take of its next sibling, if any and its parent's pointers
//Also header table + prevHeader and nextHeader
//Also decreasing header table + redundancy + single_support + redundancy 
//All things whose end =< min_end will be removed
#ifdef SWIM
inline int PruneSubtree(Node * node, Tree * tree, Node * leftSibling, int min_end)
#else
inline int PruneSubtree(Node * node, Tree * tree, Node * leftSibling)
#endif
{
	int itemNo, deleted = 0;
	if (!node || node->itemNo == -1){
		fprintf(stderr,"Horrible mistake!!node=%x\n",(int)node);
		fflush(stderr);
		return 0;
	}

#ifdef SWIM
	if (node->end > min_end)
		return 0;
#endif
	
	if (node->leftChild){
#ifdef SWIM
		deleted += PruneSubtreeWithSiblings(node, tree, NULL, min_end);
#else
		deleted += PruneSubtreeWithSiblings(node, tree, NULL);
#endif
	}
	
	itemNo = node->itemNo;
	
	if (leftSibling){
		leftSibling->sibling = node->sibling;
	}else{//I'm the first child of my parent:-)
#ifdef DEBUG
		assert(node->parent->leftChild == node);
#endif
		node->parent->leftChild = node->sibling;
	}
	
	if (node->prevHeader){
		node->prevHeader->nextHeader = node->nextHeader;
	}else{
#ifdef DEBUG
		assert(tree->header[itemNo] == node);
#endif
		tree->header[itemNo] = node->nextHeader;
	}
	
	if (node->nextHeader)
	{
		node->nextHeader->prevHeader = node->prevHeader;
	}
	
	tree->single_support[itemNo] -= node->support;
	tree->redundancy[itemNo]--;
	tree->TotalNodes --;
	delete_node(node);
	return deleted +1;
}

//All things whose end =< min_end will be removed
#ifdef SWIM
inline int PruneSubtreeWithSiblings(Node * node, Tree * tree, Node * leftSibling, int min_end)
#else
inline int PruneSubtreeWithSiblings(Node * node, Tree * tree, Node * leftSibling)
#endif
{
	int deleted = 0;
	if (!node || node->itemNo == -1){
		fprintf(stderr,"Horrible mistake!!node=%x\n",(int)node);
		fflush(stderr);
		return 0;
	}

	if (node->sibling){
#ifdef SWIM
		deleted += PruneSubtreeWithSiblings(node->sibling, tree, node, min_end);
#else
		deleted += PruneSubtreeWithSiblings(node->sibling, tree, node);
#endif
	}
	
#ifdef SWIM	
	deleted += PruneSubtree(node, tree, leftSibling, min_end);
#else
	deleted += PruneSubtree(node, tree, leftSibling);
#endif
	
	return deleted;

}
	
//This function assumes that all items in trans are sorted increasingly.
//This function assumes that we'll only have MaxItems numbers);
//It's supposed to return the number of nodes it has deleted.
//mustExists = 1 => if this trans is not there ALARM!!!!
//mustExists = 0 => if this trans is not there, don't worry
//deleteIfZero = 1 => Subtract this trans from the tree according to its support, whenever it causes some node's support to become zero remove them
//deleteIfZero = 0 => find this transaction and remove the longest prefix of that which is a single path regardless of support
//Since it just removes items, so if siblings are already sorted they'll be still so after this function
//Return value: This function returns the number of nodes it has deleted, and -1 if there's no such path exists
int RemoveTrans(Tree * tree, Trans trans, int mustExists, int deleteIfZero)
//TODO: Make sure about taking care of Header table maintainance
{
	int deleted = 0;
	return deleted;	
}
//This function is just InsertTrans, the only difference is that
// it keeps all children sorted
// For now, it sums the supports when inserting a new transaction
// but it should not be so.
// Hence I've made sure I'm calling it only when trans->support ==0
#ifdef SWIM
int InsertPatternSortedly(Tree * tree, Trans trans, int windowid)
#else
int InsertPatternSortedly(Tree * tree, Trans trans)
#endif
//TODO: Make sure about taking care of Header table maintainance
{
#ifdef SWIM
	Node * node=NULL, * parent, *leftsibling;
#else
	Node * node=NULL, * parent, *leftsibling, *firstChain;
#endif
	int curItmNum = 0,curItem,i;
	int created = 0;

	parent = & tree->root;
	while (curItmNum < trans.len){//while we have sth to insert
		curItem = trans.items[curItmNum];
		node = parent->leftChild;
		while (node){ // to find the proper child
			if (node->itemNo == curItem)
				break;
			else if (node->itemNo > curItem)
				node = NULL; //means you should add it as first child
			else{ //node->itemNo < curItem
				if (!node->sibling)
					break; //this node is the left sibling
				else if (node->sibling->itemNo <= curItem)
					node = node->sibling; //still should search
				else //node->sibling->itemNo > curItem
					break; //this node is the left sibling
			}
		}
		//if node is null you should add,
		//if not null, and equals next
		//if not null and not equal, insert as node's sibling
		if (node && node->itemNo == curItem){
				node->support += trans.support;
#ifdef SWIM
				node->end = windowid;
#endif
				tree->single_support[curItem] += trans.support;
				parent = node;
				curItmNum++;
		}	else
			break;
	}
	//Now adding all items as a single path under the current "parent"
	leftsibling = node;//it may be null in case we don't have leftsibling
	for (i = curItmNum; i<trans.len ; i++){
		curItem = trans.items[i];
		node = new_node();
		created++;
		tree->TotalNodes++;
		node->itemNo = curItem;
		node->support = trans.support;
#ifdef SWIM
		node->start = windowid;
		node->end = windowid;
#endif
		node->parent = parent;

		if (leftsibling){
			node->sibling = leftsibling->sibling;	
			leftsibling->sibling = node;
			leftsibling = NULL;
		}
		else{
			node->sibling = parent->leftChild;
			parent->leftChild = node;
		}
		node->leftChild = NULL;
//
#ifdef SWIM
		node->nextHeader = tree->header[curItem];
		if (node->nextHeader){
			node->nextHeader->prevHeader = node;
		}	
#else
			firstChain = tree->header[curItem];
		node->nextHeader = firstChain;
			if (firstChain){
				firstChain->prevHeader = node;
			}	
#endif
		node->prevHeader = NULL;
		tree->header[curItem] = node;
//
		tree->single_support[curItem] += trans.support;
		tree->redundancy[curItem]++;
		
		parent = node;
	}	
	return created;	
}


void showRowSubTree(Node * node, int indent)
{
	char * tabs;
	Node * child;
	tabs = (char*)malloc(sizeof(char) * indent +1);
	memset(tabs,'\t',indent);
	tabs[indent] = 0;
	
	if (node->itemNo == -1)
		fprintf(stderr,"%sROOT (%d)\n",
				tabs,node->support);
	else
		fprintf(stderr,"%s[%d] (%d)\n",
				tabs,node->itemNo,node->support);
	child = node->leftChild;
	while (child){
		showRowSubTree(child,indent+1);
		child = child->sibling;
	}
	
	free(tabs);
	if (!indent)
		fprintf(stderr,"====================================\n");
}

void showSubTree(Node * node, int indent, int * rev_order, int externallycalled)
{
	char * tabs;
	Node * child;
	tabs = (char*)malloc(sizeof(char) * (indent +1));
	memset(tabs,'\t',indent);
	tabs[indent] = 0;
	
	if (node->itemNo == -1)
		fprintf(stderr,"%sROOT (%d)\n",
				tabs,node->support);
	else
		fprintf(stderr,"%s%d (%d)\n",
				tabs,rev_order[node->itemNo],node->support);
	child = node->leftChild;
	while (child){
		showSubTree(child,indent+1,rev_order,0);
		child = child->sibling;
	}
	
	free(tabs);
	if (externallycalled)
		fprintf(stderr,"====================================\n");
}


//Assumes that suffix->items has enough space for tree->MaxItems
// the last argument is avg_trans_len * trans_number
//rev_order[itemNo] = rev_order[itemName], needed for output
//it returns the sum of all patterns' length
#ifdef SWIM
int Mine(Tree * tree, Trans * suffix, int minsupp, int * rev_order, int max_avg_len_trans,Tree * patternTree,int doReport, int windowid)
#else
int Mine(Tree * tree, Trans * suffix, int minsupp, int * rev_order, int max_avg_len_trans,Tree * patternTree,int doReport)
#endif
{
	int i, mine = 0, Ptr;
	Node * target, * node;
	int curItem;
	Tree * con_tree;
	static Trans trans;
	int * buf,* count;
	int temp;
	int sum_all_pat_lens = 0;

/*fprintf(stderr,"Mine below with suffix (supp=%d): ",suffix->support);
for (i=0;i<suffix->len ; i++)
	fprintf(stderr,"%d ",suffix->items[i]);
fprintf(stderr,"\n");*/

	if (isSinglePath(tree)){
		if (doReport)
			sum_all_pat_lens = MineAllSubsets(tree,suffix,minsupp,rev_order);
		else{
#ifdef SWIM
			sum_all_pat_lens =  MineIntoPatTree(tree,suffix,patternTree,windowid);
#else
			sum_all_pat_lens =  MineIntoPatTree(tree,suffix,patternTree);
#endif
		}
		return sum_all_pat_lens;
	}

/* showSubTree(&tree->root,0,rev_order);
fflush(stdout); fflush(stderr); */

	//So it's not a single path
	con_tree = new_tree(tree->MaxItems);
	
	for (i=tree->MaxItems-1 ; i>=0 ; i--){
		if (!(target = tree->header[i]))
			continue;
		if (tree->single_support[i] < minsupp)
			continue;
		curItem = target->itemNo;
//		printf("ItemNo=%d ,single_support=%d,redundancy=%d\n",curItem,tree->single_support[i],tree->redundancy[i]);
		count = resetCounts(tree->MaxItems); //
		buf = PartialDB(max_avg_len_trans);//
		Ptr = 0; // Ptr is our buffer pointer for partial database
		while (target){
			node = target->parent;
			while (node){// each non-NULL node is a parent
				if (node->itemNo >=0 ){
					buf[Ptr++] = node->itemNo;
					count[node->itemNo] += target->support;
				} else {
					buf[Ptr++] = - target->support -1;//Notice: I have changed this
				}
				node = node->parent;
			}
				
			target = target->nextHeader;
		}
		//Now partial DB is in biuf[0] to buf[Ptr-1]
		//Now going to create a new FP_TREE
		if (!trans.items){
			trans.items = (int*)malloc(sizeof(int)*tree->MaxItems);
			mine = 1;
		}
		empty_tree(con_tree);
		trans.len = 0;
		while (--Ptr >= 0){
			if (buf[Ptr] < 0){
				if (trans.len){
					InsertTrans(con_tree,trans);
				}
				trans.len = 0;
				trans.support = - buf[Ptr] -1;
			} else {
				if (count[buf[Ptr]] >= minsupp)
					trans.items[trans.len++] = buf[Ptr];
			}				
		}
		if (trans.len){
			InsertTrans(con_tree,trans);
		}
		//Now everything is in con_tree
		temp = suffix->support;
		suffix->support = tree->single_support[i];
		suffix->items[suffix->len++] = curItem;
#ifdef SWIM
		sum_all_pat_lens += Mine(con_tree,suffix,minsupp,rev_order,max_avg_len_trans,patternTree,doReport,windowid);
#else
		sum_all_pat_lens += Mine(con_tree,suffix,minsupp,rev_order,max_avg_len_trans,patternTree,doReport);
#endif
		suffix->support = temp;
		suffix->len --;
	}// current header table's entry was processed, go to the next
	
	delete_tree(con_tree);
	if (mine){
		free (trans.items);
		trans.items = NULL;
	}
	return sum_all_pat_lens;
}

//TODO:
void Rec_Verify(Tree * fptree, Tree * patternTree, int minsupp, int max_avg_len_trans, int * rev_order, int temp_cond_level)
{
	int i, first;
	Tree * con_fp_tree, * con_pat_tree;
	int size_1, size_2;
#ifdef DEVELOP		
	unsigned int garbages,time_1,time_2;
	
	if (temp_cond_level == 0)	
		num_of_edges = 0;
#endif
	//First find the last item of patternTree
	first = patternTree->MaxItems-1;
	while (first>=0 && !patternTree->header[first])
		first--;
	if (first<0)
		return;

	con_fp_tree = new_tree(first+1);
	con_pat_tree = new_tree(first+1);

//printf("FP=%d\n",temp_cond_level);	
//showSubTree(&fptree->root,temp_cond_level,rev_order,1);
//printf("PT=%d\n",temp_cond_level);	
//showSubTree(&patternTree->root,temp_cond_level,rev_order,1);

	for (i=first; i>=0 ; i--){//going trough header table of patternTree up
		if (!patternTree->header[i])
			continue;
	
		con_fp_tree->MaxItems = i+1;
		con_pat_tree->MaxItems = i+1;	
//fprintf(stderr,"calling cond. on item (%d).\n",i);
		//Notice: for patternTree all nodes' supports are 1
		//No prunning for patternTree!
		size_1 = atomic_conditionalize(patternTree, con_pat_tree, i, 0, max_avg_len_trans, rev_order, NULL);

		if (!size_1 || !fptree->header[i]){
			if (fptree->header[i])
				con_pat_tree->root.support = fptree->single_support[i];
			goto Count;
		}
#ifdef DEVELOP
		num_of_edges++;
#endif
		//should be also pruned according to patternTree
		size_2 = atomic_conditionalize(fptree, con_fp_tree, i, minsupp, max_avg_len_trans, rev_order, patternTree->header);	

		if (!size_2)
			goto Count;

		//if (isSmallEnough(size_1, size_2))
		if (temp_cond_level >= 1)
			Verify(con_fp_tree, &con_pat_tree->root, minsupp, rev_order);
		else {
			#ifdef DEVELOP
			garbages= tellMeTime();
			#endif
			Rec_Verify(con_fp_tree, con_pat_tree, minsupp, max_avg_len_trans, rev_order,temp_cond_level+1);
			#ifdef DEVELOP
			time_1 = tellMeTime() - garbages;
			#endif
		}
						#ifdef DEVELOP
						if (temp_cond_level == TEMP_GARBAGE){
							garbages= tellMeTime();
							Verify(con_fp_tree, &con_fp_tree->root, minsupp, rev_order);
							time_2 = tellMeTime() - garbages;
							garbages = estimateExecEdges(temp_cond_level);
						}
						#endif
		//Now time to return the fruits!
Count:
		copyBackTreeFreq(con_pat_tree, i-1);
		patternTree->root.support = fptree->root.support;

//printf("processing %i\n",i);
//printf("FP_con=%d\n",temp_cond_level);	
//showSubTree(&con_fp_tree->root, temp_cond_level,rev_order,1);
//printf("After PT_con=%d\n",temp_cond_level);	
//showSubTree(&con_pat_tree->root, temp_cond_level,rev_order,1);
//printf("After PT=%d\n",temp_cond_level);	
//showSubTree(&patternTree->root, temp_cond_level,rev_order,1);

	}// current header table's entry was processed, go to the next

	
	con_fp_tree->MaxItems = first+1;
	con_pat_tree->MaxItems = first+1;	
		
	delete_tree(con_fp_tree);
	delete_tree(con_pat_tree);
	
}//end of Rec_Verify

//should return the number of nodes in the result tree
int atomic_conditionalize(Tree * tree, Tree * result, int itemNo, int minsupp, int max_avg_len_trans, int * rev_order, Node ** witness)
{
	int mine = 0, Ptr, PtrAdr;
	Node * target, * node;
	static Trans trans;
	int * buf,* count;
	Node ** bufAdr;
	int size = 0;
	int seenSomething;

	if (!witness)
		witness = tree->header;

	target = tree->header[itemNo];
	count = resetCounts(tree->MaxItems);
	buf = PartialDB(max_avg_len_trans);
	bufAdr = PartialDBAdr(tree->redundancy[itemNo]+1);
	Ptr = 0; // Ptr is our buffer pointer for partial database
	PtrAdr = 0;
	while (target){
		node = target->parent;
		while (node){// each non-NULL node is a parent
			if (node->itemNo >=0 ){
if (node->itemNo >= itemNo) printf("Wow!! %d,%d\n",node->itemNo,itemNo);
				buf[Ptr++] = node->itemNo;
				count[node->itemNo] += target->support;
			} else {
				buf[Ptr++] = - target->support -1;
			}
			node = node->parent;
		}
			
		bufAdr[PtrAdr++] = target;
		target = target->nextHeader;
	}
	//Now partial DB resides in buf[0] to buf[Ptr-1]
	if (!trans.items){
		trans.items = (int*)malloc(sizeof(int)*tree->MaxItems);
		mine = 1;
	}

	empty_tree(result);

	seenSomething = 0;	
	trans.len = 0;
	while (--Ptr >= 0){
		if (buf[Ptr] < 0){
			if (trans.len){
//printTrans("Inserting",&trans);	
				size += InsertTransAdr(result, trans, bufAdr[--PtrAdr]);
			} else if (seenSomething)
				result->root.returnFreqAdr = bufAdr[--PtrAdr];
			seenSomething = 1;
			trans.len = 0;
			trans.support = - buf[Ptr] -1;
		} else {
			seenSomething = 1;
			if ((count[buf[Ptr]] >= minsupp)&&(witness[buf[Ptr]]))
				trans.items[trans.len++] = buf[Ptr];
		}				
	}
	if (trans.len){
//printTrans("Inserting",&trans);	
		size += InsertTransAdr(result, trans, bufAdr[--PtrAdr]);
	} else
		result->root.returnFreqAdr = bufAdr[--PtrAdr];			

	if (PtrAdr != 0)
		fprintf(stderr,"Weird results: ha ha ha\n");
	
	if (mine){
		free(trans.items);
		trans.items = NULL;
	}
	return size;	
}

int isSmallEnough(int patTreeNodes, int fpTreeNodes)
{
	return 0;
	if (patTreeNodes && ((fpTreeNodes / patTreeNodes)>200))
		return 1;
	else
		return 0;
}

float Delta[]={857.0000,10.6126,1.0804,0.9695,0.7847,0.6273,0.4809,0.3477,0.2296,0.1278,0.0435,0,0,0,0,0,0,0,0,0,0,0,0,0};

inline int estimateExecEdges(int level)
{
	int sum = 0, i;
	float mult = 1;
	i = level+1;
	while (Delta[i]){
		mult *= Delta[i];
		sum += mult;
		i++;
	}
	return sum;	
}


//So it means that we should have only items 0 to item_no-1 
// in the conditionalized tree
//int inton = 0;
void Conditionalize(Tree * fptree, Tree * patternTree, int max_avg_len_trans, int * rev_order)
{
	int i, mine = 0, Ptr;
	Node * target, * node;
	int curItem;
	Tree * con_fp_tree, * con_pat_tree;
	static Trans trans;
	int * buf;

//fprintf(stderr,"Pattree before cond.\n");
//showSubTree(&patternTree->root,0,rev_order,1);
	//First find the last item of patternTree
	i=patternTree->MaxItems-1;
	while (i>=0 && !patternTree->header[i])
		i--;
	if (i<0)
		return;
		
	con_fp_tree = new_tree(i+1);
	con_pat_tree = new_tree(i+1);
	
//inton++;
	for ( ; i>=0 ; i--){//going trough header table of patternTree up
		if (!(target = patternTree->header[i]))
			continue;
		curItem = target->itemNo;
//		printf("ItemNo=%d ,single_support=%d,redundancy=%d\n",curItem,tree->single_support[i],tree->redundancy[i]);
		buf = PartialDB(max_avg_len_trans);

//for (j=0; j<inton ; j++)
//	fprintf(stderr,"   ");
//fprintf(stderr,"Cond %d\n",curItem);		
//first conditionalizing patternTree
		Ptr = 0; // Ptr is our buffer pointer for partial database
		while (target){
			node = target->parent;
			while (node){// each non-NULL node is a parent
				if (node->itemNo >=0 ){
					buf[Ptr++] = node->itemNo;
				} else {
					if (!target->support)
						target->support = 1;
					buf[Ptr++] = - target->support -1;
				}
				node = node->parent;
			}
			target = target->nextHeader;
		}
		//Now partial DB is in buf[0] to buf[Ptr-1]
		//Now going to create a new FP_TREE
		if (!trans.items){
			trans.items = (int*)malloc(sizeof(int)*patternTree->MaxItems);
			mine = 1;
		}
		empty_tree(con_pat_tree);
		trans.len = 0;
		if (Ptr){//otherwise conditionalized patternTree is empty
			while (--Ptr >= 0){
				if (buf[Ptr] < 0){
					if (trans.len){
						InsertTrans(con_pat_tree,trans);
					}
					trans.len = 0;
					trans.support = - buf[Ptr] -1;
				} else {
					trans.items[trans.len++] = buf[Ptr];
				}				
			}
			if (trans.len){
				InsertTrans(con_pat_tree,trans);
			}
//fprintf(stderr,"Pattree after cond.\n");
//showSubTree(&con_pat_tree->root,0,rev_order,1);
//fprintf(stderr,"FPtree before cond.\n");
//showSubTree(&fptree->root,0,rev_order,1);
			
			//Now going to conditionalizing FP-tree
			if ((target = fptree->header[i])){
				Ptr = 0; // Ptr is our buffer pointer for partial database
				while (target){
					node = target->parent;
					while (node){// each non-NULL node is a parent
						if (node->itemNo >=0 ){
							buf[Ptr++] = node->itemNo;
						} else {
							buf[Ptr++] = - target->support -1;
						}
						node = node->parent;
					}
					target = target->nextHeader;
				}
				//Now partial DB is in buf[0] to buf[Ptr-1]
				//Now going to create a new FP_TREE
				if (!trans.items){
					trans.items = (int*)malloc(sizeof(int)*patternTree->MaxItems);
					mine = 1;
				}
				empty_tree(con_fp_tree);
				trans.len = 0;
				if (Ptr){//otherwise conditionalized patternTree is empty
					while (--Ptr >= 0){
						if (buf[Ptr] < 0){
							if (trans.len){
								InsertTrans(con_fp_tree,trans);
							}
							trans.len = 0;
							trans.support = - buf[Ptr] -1;
						} else {
							trans.items[trans.len++] = buf[Ptr];
						}				
					}
					if (trans.len){
						InsertTrans(con_fp_tree,trans);
					}
//fprintf(stderr,"FPtree after cond.\n");
//showSubTree(&con_fp_tree->root,0,rev_order,1);
					Conditionalize(con_fp_tree,con_pat_tree,max_avg_len_trans,rev_order);
				}
			}//ending conditionalizing FP-tree

		}//after this point we should not do anything
	}// current header table's entry was processed, go to the next
//	inton--;	
	delete_tree(con_fp_tree);
	delete_tree(con_pat_tree);
	if (mine){
		free (trans.items);
		trans.items = NULL;
	}
}

int * resetCounts(int MaxItems)
{
	static int * count = NULL;
	static int size = 0;
	int i;
	
	if (MaxItems > size || !count){
		if (count)
			free(count);
		size = MaxItems;
		count = (int*)malloc (sizeof(int)*size);
	}
	for (i=0; i<MaxItems ; i++)
		count[i] = 0;	
	
	return count;
}

int isSinglePath(Tree * tree)
{
	Node * node;
	node = & tree->root;
	
	while (node){
		if (node->sibling)
			return 0;
		node = node->leftChild;
	}
	return 1;
}

int * PartialDB(int max)//max = avg_trans_len * no_of_trans
{
	static int * buf = NULL;
	static int size = 0;
	
	if (max > size || !buf){
		if (buf)
			free(buf);
		size = max;
		buf = (int*)malloc (sizeof(int)*size);
	}
	
	return buf;
}


Node ** PartialDBAdr(int max)//max = header size of itemNo 
{
	static Node ** bufAdr = NULL;
	static int size = 0;

//	printf("sizeof(%d),sizeof(%d).\n", sizeof(int), sizeof(Node *));
	
	if (max > size || !bufAdr){
		if (bufAdr)
			free(bufAdr);
		size = max;
		bufAdr = (Node **)malloc (sizeof(Node *)*size);
	}
	
	return bufAdr;
}


void MineSubsets(Tree * tree,Trans * suffix,int minsupp,int * rev_order)
{
	int i;
	Node * node = tree->root.leftChild;

//	fprintf(stdout,"mined: ");
	for (i=0; i<suffix->len ; i++)
		fprintf(stdout,"%d[%d] ",suffix->items[i],suffix->support);
	while (node){
		fprintf(stdout,"%d(%d) ",node->itemNo, node->support);
		node = node->leftChild;
	}
	fprintf(stdout,"\n");
}

//TODO: This function is supposed to create all gray codes
int MineAllSubsets(Tree * tree,Trans * suffix,int minsupp,int * rev_order)
{
	register int i,j;
	int all;
	Node * node = tree->root.leftChild;
	
	//fprintf(stdout,"mined: ");
	all = suffix->len;
	while (node){
		suffix->items[suffix->len++] = node->itemNo;
		//fprintf(stdout,"%d(%d) ",node->itemNo, node->support);
		node = node->leftChild;
	}
//fprintf(stdout,"all=%d but suffix->len=%d\n",all,suffix->len);
	for (i=0; i< (1<< (suffix->len - all)) ; i++){
		fprintf(stdout,"[%d] ",suffix->support);
		for (j=0; j<all ; j++)
			fprintf(stdout,"%d ",rev_order[suffix->items[j]]);
		for (j=all; j<suffix->len ; j++)
			if (words[i] & (1<<(j-all)))
				fprintf(stdout,"%d ",rev_order[suffix->items[j]]);
//fprintf(stdout,"{%d}",words[i]);
		fprintf(stdout,"\n");
	}
	suffix->len = all;
	//TODO: The following output is wrong
	printf("TODO: Don't use function!"); fflush(stdout);
	return 0;
}

#ifdef SWIM
int MineIntoPatTree(Tree * tree,Trans * suffix,Tree * patternTree, int windowid)
#else
int MineIntoPatTree(Tree * tree,Trans * suffix,Tree * patternTree)
#endif
{
	register int i,j;
	int all;
	Node * node = tree->root.leftChild;
	int sum_all_pat_len = 0;
	//The following stuff should be fixed!
	Trans help;
		
	all = suffix->len;
	
	while (node){
		suffix->items[suffix->len++] = node->itemNo;
		node = node->leftChild;
	}
	
	help.items = (int*)malloc(sizeof(int)*suffix->len);
	help.support = 0;//to make sure pattern tree won't be wrong!
	
	for (i=0; i< (1<< (suffix->len - all)) ; i++){
		for (j=0; j<all ; j++)
			help.items[j] = suffix->items[j];
		help.len = all;
		for (j=all; j<suffix->len ; j++)
			if (words[i] & (1<<(j-all)))
				help.items[help.len ++] = suffix->items[j];
		
		sortTran(&help);
		sum_all_pat_len += help.len +1;
		//#ifndef DEVELOP
		//InsertPatternSortedly(patternTree,help);
		//#endif
		
		//#ifdef DEVELOP
		if (temp_inserted_patterns < TEMP_MAX_INSERTED_PATTERNS){
#ifdef SWIM
			temp_inserted_patterns += InsertPatternSortedly(patternTree,help,windowid);
#else
			temp_inserted_patterns += InsertPatternSortedly(patternTree,help);
#endif
		}else{
			temp_inserted_patterns_flag = 1;
			suffix->len = all;
			free(help.items);
			return sum_all_pat_len;
		}
		//#endif
	}
	
	suffix->len = all;
	free(help.items);
	return sum_all_pat_len;
}


//Eliminate non-frequent ones according to freq[]
//, and sort others increasingly 
void sortEliminateTran(Trans * tran, int *freq, int min_supp)
{	
	register int i, j, temp;
	
	//Here we eliminate all non-freq single items
	for (i=0; i<tran->len ;){
		if (freq[tran->items[i]] < min_supp){
			tran->items[i] = tran->items[--tran->len];	
		}else
			i++;
	}
	//Now insertion sort
	for (j=1; j<tran->len ; j++){
		temp = tran->items[j];
		i = j-1;
		while ((i>=0)&&( tran->items[i] > temp) ){
			tran->items[i+1] = tran->items[i];
			i--;
		}
		tran->items[i+1] = temp;
	}
}

//Just sort items increasingly 
void sortTran(Trans * tran)
{	
	register int i, j, temp;
	
	//Now insertion sort
	for (j=1; j<tran->len ; j++){
		temp = tran->items[j];
		i = j-1;
		while ((i>=0)&&( tran->items[i] > temp) ){
			tran->items[i+1] = tran->items[i];
			i--;
		}
		tran->items[i+1] = temp;
	}
	
}

//u is the root of our pattern tree
//initially all supports in this tree should be zero
//by setting minsupp to zero we can force it to compute everything
void Verify(Tree * fptree,Node * u, int minsupp,int * rev_order)
{
	Node * child, *node, *t;
	
	if (u->itemNo == -1) { //u is the root
		u->support = fptree->root.support;
		fptree->root.flag = FAIL;
		for (child = u->leftChild; child ; child=child->sibling){//in pat tree
			//Following is needed if pattree is not initialized
			//child->support = 0;
			for (node = fptree->header[child->itemNo]; node;
						node = node->nextHeader){ //nodes of fptree	
				node->flag = OK;
				child->support += node->support;					
			}
			if (child->support >= minsupp)
				Verify(fptree,child,minsupp,rev_order);
		}
	}	else {// u is not root of pat tree
		for (child = u->leftChild; child ; child = child->sibling){//in pat tree
			//Following is needed if pattree is not initialized
			//child->support = 0;
			for (node = fptree->header[child->itemNo]; node;
						node = node->nextHeader){ //nodes of fptree
				//find lowest t here
				cost_2 ++;
				t = node->parent;
				//while (t->itemNo > u->itemNo){ //to show marking importance
				while ((t->itemNo > u->itemNo)&& (!t->flag)){
					t = t->parent;
					cost_1++;
				}
				
				if (t->itemNo < u->itemNo)
					node->flag = FAIL;
				else {
					node->flag = t->flag;
					if (node->flag == OK)
						child->support += node->support;
				}			
			}// all nodes
			if (child->support >= minsupp)
				Verify(fptree,child,minsupp,rev_order);
		}
	}
	//either u is root or not
	for (child = u->leftChild; child ; child = child->sibling)
		for (node = fptree->header[child->itemNo]; node;
				            node = node->nextHeader){
			//cost_3 ++;
			node->flag = NIL;
		}
	
}


//retunrs the number of nodes in the tree as well as printing
//more interesting statistics preceded by the given message
int getTreeStatistics(char * message,Tree * tree)
{
	int num_of_nodes = 0;
	int num_of_items = 0;
//	int avg_degree = 0;// avg num of children per node
	int avg_depth = 0;
	int num_of_leaves = 0;
//the following means the sum of nodes' supports divided by number of nodes 
//	int percent_of_total_saving = 0; 
//the following means the sum of length of maximal paths from root 
//down to a leaves divided by the number of nodes
//	int percent_of_overlaps = 0;
//Temp variables
	Node * node, *oldnode;
	int i, curDepth = 0;
	
//Now calculating the above parameters
	for (i=0; i<tree->MaxItems ; i++){
		if (tree->header[i])
			num_of_items++;
		num_of_nodes += tree->redundancy[i];
	}
#ifdef SWIM
	if (!message){
		return num_of_nodes;	
	}else{
		fprintf(stderr,"Debug meeeeeee!!!!!\n");
	}
#endif
	
	node = &tree->root;
	do{
		oldnode = node;
		node->flag = 1;
		if (node->leftChild){
			if (!node->leftChild->flag){
				curDepth ++;
				node = node->leftChild;
			} else
				node = node->sibling;
		}else{
			num_of_leaves++;
			avg_depth += curDepth;
			node = node->sibling;
		}
		if (!node){
			node = oldnode->parent;
			curDepth --;
		}
	} while (node);
//Clearing flags
	for (i=0; i<tree->MaxItems ; i++){
		node = tree->header[i];
		while (node){
			node->flag = 0;
			node = node->nextHeader;
		}
	}

//Now printing them
	if (message){
		fprintf(stderr,"Statistics: %s\n",message);
		fprintf(stderr,"#items=%d , #nodes=%d , avg_length_of_header_siblings=%d\n",num_of_items,num_of_nodes,num_of_nodes/num_of_items);
		fprintf(stderr,"Average depth of tree = %d , #leaves(different paths)=%d\n",avg_depth/num_of_leaves, num_of_leaves);
		fprintf(stderr,"========================\n");
	}	
	return num_of_nodes;
}

void resetTreeFreq(Tree * tree,int value)
{
	Node * node;
	int i;
	
//Now calculating the above parameters
	for (i=0; i<tree->MaxItems ; i++){
		node = tree->header[i];
		while (node){
			node->support = value;
			node = node->nextHeader;
		}
	}
}


void diffTrees(Node * left, Node * right, int * leftmore, int * common, int * rightmore)
{
	Node * node, *node2;
	int tleft,tright,tcommon;
	
	if (!leftmore || !common || !rightmore){
		fprintf(stderr,"diffTrees: NULL inputs..\n");
		fflush(stderr);
		return;
	}
	*leftmore = 0; *rightmore = 0; *common = 0;

//Both empty	
	if (!left && !right){
		return;
	}
//Only right tree
	if (!left && right){
		for (node2=right->leftChild; node2; node2=node2->sibling){
			*rightmore = *rightmore + 1;
			diffTrees(NULL,node2, &tleft, &tcommon, &tright);
			*rightmore = *rightmore+tright;
		}

		return;
	}
//Only left tree
	if (left && !right){
		for (node=left->leftChild; node; node=node->sibling){
			*leftmore = *leftmore + 1;
			diffTrees(node,NULL, &tleft, &tcommon, &tright);
			*leftmore = *leftmore+tleft;
		}

		return;
	}
	
//now we know both trees are not empty
	for (node=left->leftChild; node; node=node->sibling){
		for (node2=right->leftChild; node2; node2=node2->sibling)
			if (node2->itemNo == node->itemNo)
				break;
		if (node2){
			node2->itemNo = node2->itemNo - 20000;//Should be changed if more than 20000 single items! 
			*common = *common +1;
		} else
			*leftmore = *leftmore +1;
			
		diffTrees(node,node2, &tleft, &tcommon, &tright);
		*leftmore = *leftmore+tleft; *common = *common+tcommon; *rightmore = *rightmore+tright;
	}
//Counting not-visited nodes of right tree
	for (node2=right->leftChild; node2; node2=node2->sibling)
		if (node2->itemNo >= 0){
			*rightmore = *rightmore +1;
			diffTrees(NULL,node2, &tleft, &tcommon, &tright);
			*rightmore = *rightmore+tright;
		}
//Undoing right tree marks				
	for (node2=right->leftChild; node2; node2=node2->sibling)
		if (node2->itemNo < 0)
			node2->itemNo = node2->itemNo + 20000;
}

void ExtractAllPatternsDB(Tree * patternTree,char * tablename)
{
	Node * target, * node;
	Trans trans;
	int i,k;

	trans.items = (int*)malloc(sizeof(int)*patternTree->MaxItems);
	

	for (i=patternTree->MaxItems-1 ; i>=0 ; i--){
		target = patternTree->header[i];
		while (target){
			node = target;
			trans.len = 0;
			while (node->itemNo != -1){
				trans.items[trans.len ++]=node->itemNo;
				node = node->parent;
			}
			printf("insert into %s%d values(%d,",tablename,trans.len,target->support);
			printf("%d",trans.items[0]);
			for (k=1; k<trans.len; k++)
				printf(",%d",trans.items[k]);
			printf(");\n");
			
			target = target->nextHeader;
		}			
	}
	free(trans.items);
}

//Returns the number of new nodes created, as a result of this union
#ifdef SWIM
int UnionTrees(Tree * mainPatTree, Tree * temp_PatTree, 
							int windowid, int max_avg_len_trans)
{
	int mine = 0, Ptr;
	Node * target, * node;
	static Trans trans;
	int * buf;
	int i;
	int size = 0;


	buf = PartialDB(max_avg_len_trans);
	
	for (i=temp_PatTree->MaxItems - 1; i>=0 ; i--){
		if (!(target = temp_PatTree->header[i]))
			continue;
		Ptr = 0; // Ptr is our buffer pointer for partial database
		while (target){
			node = target;
			while (node){// each non-NULL node is a parent
				if (node->itemNo >=0 ){
					buf[Ptr++] = node->itemNo;
				} else {
					buf[Ptr++] = - target->support -1;
				}
				node = node->parent;
			}
			target = target->nextHeader;
		}
		//Now partial DB resides in buf[0] to buf[Ptr-1]
		if (!trans.items){
			trans.items = (int*)malloc(sizeof(int)*temp_PatTree->MaxItems);
			mine = 1;
		}

		trans.len = 0;
		while (--Ptr >= 0){
			if (buf[Ptr] < 0){
				if (trans.len){
					size += InsertPatternSortedly(mainPatTree, trans, windowid);
				}
				trans.len = 0;
				trans.support = - buf[Ptr] -1;
			} else {
				trans.items[trans.len++] = buf[Ptr];
			}				
		}
		if (trans.len){
			size += InsertPatternSortedly(mainPatTree, trans, windowid);
		}
	}
	
	if (mine){
		free(trans.items);
		trans.items = NULL;
	}
	return size;	
}
#endif
//Finds the start timestamp of a given trans in the given tree which is usually a pattern tree
//Returns the start time, if the patterns doesn't exist return -1 and prints error message
#ifdef SWIM
int FindStartTime(Tree * tree, Trans trans)
{
	Node * node=NULL, * parent;
	int curItmNum = 0,curItem;

	parent = & tree->root;
	while (curItmNum < trans.len){//while we have sth to find 
		curItem = trans.items[curItmNum];
		node = parent->leftChild;
		while (node){ // to find the proper child
			if (node->itemNo == curItem)
				break;
			else if (node->itemNo > curItem)
				node = NULL; //means you should add it as first child
			else{ //node->itemNo < curItem
				if (!node->sibling)
					break; //this node is the left sibling
				else if (node->sibling->itemNo <= curItem)
					node = node->sibling; //still should search
				else //node->sibling->itemNo > curItem
					break; //this node is the left sibling
			}
		}
		//if node is null you should add,
		if (!node || node->itemNo != curItem){
			fprintf(stderr,"?????????Error! transaction not found!\n");
			return -1;
		}
		//if not null, and equals next
		if (node && node->itemNo == curItem){
				parent = node;
				curItmNum++;
		}
	}
	if (curItmNum < trans.len){
		fprintf(stderr,"?????????Error! transaction not found!\n");
		return -1;
	}
	
	return parent->start;
}
#endif

#ifdef SWIM
int FindEndTime(Tree * tree, Trans trans)
{
	Node * node=NULL, * parent;
	int curItmNum = 0,curItem;

	parent = & tree->root;
	while (curItmNum < trans.len){//while we have sth to find 
		curItem = trans.items[curItmNum];
		node = parent->leftChild;
		while (node){ // to find the proper child
			if (node->itemNo == curItem)
				break;
			else if (node->itemNo > curItem)
				node = NULL; //means you should add it as first child
			else{ //node->itemNo < curItem
				if (!node->sibling)
					break; //this node is the left sibling
				else if (node->sibling->itemNo <= curItem)
					node = node->sibling; //still should search
				else //node->sibling->itemNo > curItem
					break; //this node is the left sibling
			}
		}
		//if node is null you should add,
		if (!node || node->itemNo != curItem){
			fprintf(stderr,"FindEndTime:??????Error! transaction not found!\n");
			return -1;
		}
		//if not null, and equals next
		if (node && node->itemNo == curItem){
				parent = node;
				curItmNum++;
		}
	}
	if (curItmNum < trans.len){
		fprintf(stderr,"FindEndTime:?????????Error! transaction not found!\n");
		return -1;
	}
	
	return parent->end;
}
#endif

#ifdef SWIM
int checkConsistentcy(Tree * tree)
{
	int p;
	Node * node;
	for (p=0; p<tree->MaxItems ; p++){
		node = tree->header[p];
		while (node){
			if (node->itemNo <= node->parent->itemNo){
				fprintf(stderr,"Violated:%x,%d,%d\n",node,node->itemNo,node->parent->itemNo);
				return 1;
			}
			node = node->nextHeader;
		}
	}
	return 0;
}
#endif

void copyBackTreeFreq(Tree * tree,int largestItemNo)
{
	Node * node;
	int i;
	
	if (tree->root.returnFreqAdr)
		tree->root.returnFreqAdr->support = tree->root.support;
//Now calculating the above parameters
	for (i=0; i<=largestItemNo && i<tree->MaxItems; i++){
		node = tree->header[i];
		while (node){
			if (node->returnFreqAdr)
				node->returnFreqAdr->support = node->support;
			node = node->nextHeader;
		}
	}
	
}

