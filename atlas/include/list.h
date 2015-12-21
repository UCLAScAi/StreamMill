#ifndef _NT_LIST_H_
#define _NT_LIST_H_

#include <stdlib.h>
#include <string.h>

#include <adl_obj.h>
#include <abstypes.h>

#define DEFAULT_LIST_SIZE 10

struct A_list_ {
  A_pos pos;
  adl_t type;
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */

  nt_obj_t **elements;
  int size;
  int length;
  int collect_type;		/* list,set,bag */
};

typedef struct A_list_ *A_list;

A_list A_List(A_pos pos=0, adl_t type=A_UNKNOWN);
A_list copyAList(A_list alist);
int A_ListEmpty(A_list a);
// append an element to the list, calling appendElementList()
// looks like a wrapper funtion of appendElementList()
void appendAList(A_list alist, void* obj);

#define ListObj(o) ((A_list)((o)->value))

A_list newList(void);
A_list readList(nt_obj_t *stream);
A_list copyList(A_list list);
void deleteList(A_list list);

void clearList(A_list list);
int isEmptyList(A_list list);

void displayList(nt_obj_t *stream, A_list list, char *del);

int memberPList(A_list list, nt_obj_t *obj);
int memberEqualPList(A_list list, nt_obj_t *obj);
int equalList(A_list l1, A_list l2);
nt_obj_t* getNthElementList(A_list list, int position);
int insertElementList(A_list list, int position, nt_obj_t *obj);
void overwriteElementList(A_list list, int position, nt_obj_t *obj);
void prependElementList(A_list list, nt_obj_t *obj);
void appendElementList(A_list list, nt_obj_t *obj);
void addElementList(A_list list, nt_obj_t *obj);
void removeElementList(A_list list, nt_obj_t *obj);
nt_obj_t *removeNthElementList(A_list list, int position);
nt_obj_t *getFirstElementList(A_list list);
nt_obj_t *getLastElementList(A_list list);
void exchangeElementsList(A_list list, int position1, int position2);
void pushElementList(A_list list, nt_obj_t *obj);
nt_obj_t *popElementList(A_list list);
void appendListList(A_list list, A_list alist);

int isTypeList(nt_obj_t* obj, object_type this_type);
int walkList(A_list list, int (*f)());

#endif /* _NT_LIST_H_ */




