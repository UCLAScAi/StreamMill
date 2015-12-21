#include <stdio.h>
#include <sql/list.h>
#include <sql/err.h>
#include <sql/util.h>
#include <sql/io.h>

A_list A_List(A_pos pos, adl_t type)
{
  A_list list = (A_list)ntMalloc(sizeof(*list));
  register int	i;

  list->pos = pos;
  list->type = type;

  list->elements = (nt_obj_t **)ntMalloc(DEFAULT_LIST_SIZE*sizeof(nt_obj_t*));
  list->size = DEFAULT_LIST_SIZE;
  list->length = 0;

  for (i = 0; i < list->size; i++)
    list->elements[i] = (nt_obj_t*)0;
  
  list->collect_type = 0;
  return list;
}

int A_ListEmpty(A_list l)
{
  return (!l || l->length == 0) ? 1:0;
}


A_list readList(nt_obj_t *stream)
{
  int i;
  A_list  list = (A_list )ntMalloc(sizeof(*list));
  list->size=ntReadNum(stream);
  list->length=ntReadNum(stream);
  list->collect_type=ntReadNum(stream);
  list->elements = (nt_obj_t **)ntMalloc(list->size*sizeof(nt_obj_t*));
  for (i=0; i<list->length; i++)
    list->elements[i]=readObj(stream);
  for (;i<list->size; i++)
    list->elements[i]=(nt_obj_t*)0;
  return list;
}

void deleteList(A_list list)
{
  int i;
  for (i=0; i<list->length; i++) {
    deleteObj(list->elements[i]);
  }
  ntFree(list->elements);
  ntFree(list);
}

void clearList(A_list list)
{
  register int	i;

  list->length = 0;

  for (i = 0; i < list->size; i++)
    list->elements[i] = (nt_obj_t*)0;
}

int walkList(A_list list, int (*f)(nt_obj_t *))
{
  int i;
  int bl = 1;
  for (i=0; bl && i<list->length; i++)
    bl = f(list->elements[i]);
  return bl;
}
static void displayListElements(nt_obj_t *s, A_list list, char *del)
{
  int i;
  if (list->length>0)
    displayObj(s, list->elements[0]);

  for (i=1; i<list->length; i++) {
    if (del && !(s->flag & BINARY_STREAM))
      ntPrintf(s, "%s", del);
    displayObj(s, list->elements[i]);
  }
}
void displayList(nt_obj_t *s, A_list list, char *del)
{
  if (s->flag & BINARY_STREAM) {
    ntPrintf(s, "%d%d%d", list->size, list->length, list->collect_type);
    displayListElements(s, list, del);
  } else {
    switch (list->collect_type) {
    case O_SET_TYPE:
      ntPrintf(s, "{");
      displayListElements(s, list, del);
      ntPrintf(s, "}");
      break;
    case O_BAG_TYPE:
      ntPrintf(s, "{");
      displayListElements(s, list, del);
      ntPrintf(s, "}");
      break;
    case O_LIST_TYPE:
      ntPrintf(s, "[");
      displayListElements(s, list, del);
      ntPrintf(s, "]");
      break;
    default:
      displayListElements(s, list, del);
      break;
    }
  }
}

static int doubleSizeList(A_list list)
{
  int		status = 0;
  int		new_size;
  nt_obj_t**	new_elements;

  if (list->size)
    new_size = 2 * list->size;
  else
    new_size = 1;

  if (new_elements = (nt_obj_t **) ntRealloc(list->elements, 
					     new_size * sizeof(nt_obj_t *)))
    {
      list->size = new_size;
      list->elements = new_elements;
      status = 1;
    }
  else
    status = 0;

  return (status);
}

nt_obj_t *getNthElementList(A_list list, int position)
{
  return(position < list->length ? list->elements[position] : (nt_obj_t*)0);
}

void overwriteElementList(A_list list, int position, nt_obj_t *obj)
{
  if ((position >= 0) && (position < list->length))
    list->elements[position]=obj;
}

void prependElementList(A_list list, nt_obj_t *obj)
{
  insertElementList(list, 0, obj);
}

void appendElementList(A_list list, nt_obj_t *obj)
{
  insertElementList(list, list->length, obj);
}

nt_obj_t * getFirstElementList(A_list list)
{
  return (getNthElementList(list, 0));
}

nt_obj_t * getLastElementList(A_list list)
{
  return (getNthElementList(list, list->length-1));
}

void pushElementList(A_list list, nt_obj_t *obj)
{
  appendElementList(list, obj);
}

nt_obj_t * popElementList(A_list list)
{
  return (removeNthElementList(list, list->length-1));
}

int isEmptyList(A_list list)
{
  return (list->length <= 0 ? 1 : 0);
}


int insertElementList(A_list list, int position, nt_obj_t* obj)
{
  register int i;

  if (position >= 0 && position <= list->length)
    if (list->size > list->length)
      {
	for (i = list->length; i > position; i--)
	  list->elements[i] = list->elements[i-1];

	list->elements[position] = obj;
	list->length++;
      }
    else if (doubleSizeList(list))
      insertElementList(list, position, obj);
    else
      return -ERR_OUT_OF_MEMORY;
}

#if 0
int equalList(A_list l1, A_list l2)
{
  register int	i;
  if (l1->length != l2->length) 
    return 0;
  for (i=0; i<l1->length; i++) {
    if (!equalObj(l1->elements[i], l2->elements[i]))
      return 0;
  }
  return 1;
}

int memberEqualPList(A_list list, nt_obj_t* obj)
{
  register int	i;
  int index=-1;

  for (i = 0; i < list->length; i++)
    if (equalObj(list->elements[i],obj))
      {
	index = i;
	break;
      }

  return (index);
}
#endif 
int memberPList(A_list list, nt_obj_t* obj)
{
  register int	i;
  int index=-1;

  for (i = 0; i < list->length; i++)
    if (list->elements[i] == obj)
      {
	index = i;
	break;
      }

  return (index);
}


void removeElementList(A_list list, nt_obj_t* obj)
{
  register int i;
  register int j;

  for (i = 0; i < list->length; i++)
    if (list->elements[i] == obj)
      break;

  if (i < list->length)		
    {
      for (j = i; j < list->length; j++)
	list->elements[j] = list->elements[j+1];
      
      list->length--;
      list->elements[list->length] = (nt_obj_t*)0;
    }
}


nt_obj_t* removeNthElementList(A_list list, int position)
{
  if (position >= 0 && position < list->length)
    {
      register int	i;
      nt_obj_t*	obj = list->elements[position];

      for (i = position; i < list->length-1; i++) 
	list->elements[i] = list->elements[i+1];
      
      list->length--;
      list->elements[list->length] = (nt_obj_t *)0;

      return (obj);
    }

  return (nt_obj_t*)0;
}

void exchangeElementsList(A_list list, int position1, int position2)
{
  if (position1 > 0 && position1 < list->length &&
      position2 > 0 && position2 < list->length)
    {
      nt_obj_t* temp_element;
      
      temp_element = list->elements[position1];
      list->elements[position1] = list->elements[position2];
      list->elements[position2] = temp_element;
    }
}

void appendListList(A_list list, A_list alist)
{
  register int i;

  for (i = 0; i < alist->length; i++)
    appendElementList(list, getNthElementList(alist, i));
}

void addElementList(A_list list, nt_obj_t* obj)
{
  register int	i;

  for (i = 0; i < list->length; i++)
    if (obj == getNthElementList(list, i))
      break;

  if (i >= list->length)
    appendElementList(list, obj);
}

int isTypeList(nt_obj_t* obj, object_type this_type)
{
  if (obj->type == O_LIST)
    {
      A_list list = ListObj(obj);
      if (list->length>0 && getNthElementList(list, 0)->type == this_type)
	return 1;
    }
  return 0;
}

