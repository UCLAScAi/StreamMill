/*
 * types.c - 
 *
 * All types and functions declared in this header file begin with "Ty_"
 * Linked list types end with "..list"
 */

#include <stdio.h>
#include "util.h"
#include <sql/list.h>
#include <sql/symbol.h>
#include <sql/types.h>

static Ty_ty tynil = NULL;

Ty_ty Ty_Nil(void) {
  if(tynil == NULL) {
    tynil = (Ty_ty)malloc(sizeof(Ty_ty_));
    tynil->kind = Ty_nil;
    tynil->pp = (void*)0;
  }
  return tynil;
}

static Ty_ty tyint = NULL;
Ty_ty Ty_Int(void) {
  if(tyint == NULL) {
    tyint = (Ty_ty)malloc(sizeof(Ty_ty_));
    tyint->kind = Ty_int;
    tyint->pp = (void*)0;
  }
  return tyint;
}

static Ty_ty tyref = NULL;
Ty_ty Ty_REF(void) {
  if(tyref == NULL) {
    tyref = (Ty_ty)malloc(sizeof(Ty_ty_));
    tyref->kind = Ty_ref;
    tyref->pp = (void*)0;
  }
  return tyref;
}

static Ty_ty tyreal = NULL;
Ty_ty Ty_Real(void) {
  if(tyreal == NULL) {
    tyreal = (Ty_ty)malloc(sizeof(Ty_ty_));
    tyreal->kind = Ty_real;
    tyreal->pp = (void*)0;
  }
  return tyreal;
}

static Ty_ty tystring = NULL;
Ty_ty Ty_String(void) {
  if(tystring == NULL) {
    tystring = (Ty_ty)malloc(sizeof(Ty_ty_));
    tystring->kind = Ty_string;
    tystring->pp = (void*)0;
  }
  return tystring;
}

static Ty_ty tytimestamp = NULL;
Ty_ty Ty_Timestamp(void) {
  if(tytimestamp == NULL) {
    tytimestamp = (Ty_ty)malloc(sizeof(Ty_ty_));
    tytimestamp->kind = Ty_timestamp;
    tytimestamp->pp = (void*)0;
  }
  return tytimestamp;
}

static Ty_ty tyiext = NULL;
Ty_ty Ty_IExt(void) {
  if(tyiext == NULL) {
    tyiext = (Ty_ty)malloc(sizeof(Ty_ty_));
    tyiext->kind = Ty_iext;
    tyiext->pp = (void*)0;
  }
  return tyiext;
}

static Ty_ty tyrext = NULL;
Ty_ty Ty_RExt(void) {
  if(tyrext == NULL) {
    tyrext = (Ty_ty)malloc(sizeof(Ty_ty_));
    tyrext->kind = Ty_rext;
    tyrext->pp = (void*)0;
  }
  return tyrext;
}

static Ty_ty tycext = NULL;
Ty_ty Ty_CExt(void) {
  if(tycext == NULL) {
    tycext = (Ty_ty)malloc(sizeof(Ty_ty_));
    tycext->kind = Ty_cext;
    tycext->pp = (void*)0;
  }
  return tycext;
}

static Ty_ty tytext = NULL;
Ty_ty Ty_TExt(void) {
  if(tytext == NULL) {
    tytext = (Ty_ty)malloc(sizeof(Ty_ty_));
    tytext->kind = Ty_text;
    tytext->pp = (void*)0;
  }
  return tytext;
}

static Ty_ty tyvoid = NULL;
Ty_ty Ty_Void(void) {
  if(tyvoid == NULL) {
    tyvoid = (Ty_ty)malloc(sizeof(Ty_ty_));
    tyvoid->kind = Ty_void;
    tyvoid->pp = (void*)0;
  }
  return tyvoid;
}

Ty_ty Ty_Record(A_list fields)
{
  Ty_ty p = (Ty_ty)ntMalloc(sizeof(*p));
  p->kind=Ty_record;
  p->u.record=fields;
  return p;
}
/* convert a record type to ref type */
Ty_ty Ty_Ref(Ty_ty record_ty)
{
  Ty_ty p = (Ty_ty)ntMalloc(sizeof(*p));
  p->kind=Ty_ref;
  p->u.record=record_ty->u.record;
  return p;
}
Ty_ty Ty_Ref(A_list fields)
{
  Ty_ty p = (Ty_ty)ntMalloc(sizeof(*p));
  p->kind=Ty_ref;
  p->u.record=fields;
  return p;
}

Ty_ty Ty_Array(Ty_ty ty)
{
  Ty_ty p = (Ty_ty)ntMalloc(sizeof(*p));
  p->kind=Ty_array;
  p->u.array=ty;
  return p;
}

Ty_ty Ty_Name(S_symbol sym, Ty_ty ty)
{
  Ty_ty p = (Ty_ty)ntMalloc(sizeof(*p));
  p->kind=Ty_name;
  p->u.name.sym=sym;
  p->u.name.ty=ty;
  return p;
}


Ty_ty copyTy(Ty_ty ty) {
  Ty_ty t = (Ty_ty)0;
  switch(ty->kind) {
  case Ty_record:
    t = Ty_Record(copyAList(ty->u.record));
    break;
  case Ty_nil:
    t = Ty_Nil();
    break;
  case Ty_int:
    t = Ty_Int();
    break;
  case Ty_real:
    t = Ty_Real();
    break;
  case Ty_string:
    t = Ty_String();
    break;
  case Ty_array:
    t = Ty_Array(copyTy(ty->u.array));
    break;
  case Ty_name:
    t = Ty_Name(ty->u.name.sym, copyTy(ty->u.name.ty));
    break;
  case Ty_void:
    t = Ty_Void();
    break;
  case Ty_ref:
    t = Ty_REF();;
    break;
  case Ty_timestamp:
    t = Ty_Timestamp();
    break;
  default:
    break;
  }
  return t;
}


/*
Ty_tyList Ty_TyList(Ty_ty head, Ty_tyList tail)
{Ty_tyList p = ntMalloc(sizeof(*p));
 p->head=head;
 p->tail=tail;
 return p;
}
*/
Ty_field Ty_Field(S_symbol name, S_symbol ref, Ty_ty ty)
{
  Ty_field p = (Ty_field)ntMalloc(sizeof(*p));
  p->name = name;
  p->ty = ty;
  p->ref = ref;
  p->iskey = 0;
  p->isTimekey = 0;
  p->implicit = 0;
  p->size = 0;		// reference type is a pointer
  return p;
}
Ty_field Ty_Field(S_symbol name, Ty_ty ty, int size)
{
  Ty_field p = (Ty_field)ntMalloc(sizeof(*p));
  p->size = size;
  p->name = name;
  p->ty = ty;
  p->iskey = 0;
  p->isTimekey = 0;
  p->implicit = 0;
  p->ref = (S_symbol)0;
  return p;
}

/*
Ty_fieldList Ty_FieldList(Ty_field head, Ty_fieldList tail)
{Ty_fieldList p = ntMalloc(sizeof(*p));
 p->head=head;
 p->tail=tail;
 return p;
}
*/
/* printing functions - used for debugging */
static char str_ty[][13] = {
   "ty_record", "ty_nil", "ty_int", "ty_real", "ty_timestamp", "ty_string", 
   "ty_array", "ty_name", "ty_void", "ty_ref"};

/* This will infinite loop on mutually recursive types */
void Ty_print(Ty_ty t)
{
  if (t == NULL) printf("null");
  else { 
    printf("%s", str_ty[t->kind]);
    if (t->kind == Ty_name) {
      printf(", %s", S_name(t->u.name.sym)); }
  }
}

static int typeSize[][2] = {
  {10,	sizeof(int)}, /* Ty_record,   size=10 for reference type */
  {20,	sizeof(int)}, /* Ty_nil,        */
  {10,	sizeof(int)}, /* Ty_int,        */
  {10,	sizeof(double)}, /* Ty_real,       */
  {20,	0},/* Ty_string,   size is usually kept in Ty_field  */
  {0,	0},/* Ty_array,      */
  {0,	0},/* Ty_name,       */
  {0,	0},/* Ty_void,        */
  {10,  sizeof(int)},	/* Ty_ref,       currently not in use */
  {10,	sizeof(struct timeval)} /* Ty_timestamp       */
};
int getDisplaySize(Ty_ty t)
{ 
  return typeSize[t->kind][0]; 
}
int getStorageSize(Ty_ty t)
{
  return typeSize[t->kind][1];
}

void TyList_print(A_list list)
{
  if (list == NULL) printf("null");
  else {
    printf("TyList( ");
    for (int i=0; i<list->length; i++) {
      Ty_ty f = (Ty_ty)getNthElementList(list, i);
      Ty_print(f);
      printf(", ");
    }
    printf(")");
  }
}

int Ty_compatible(Ty_ty ty1, Ty_ty ty2)
{
  if (ty1->kind != ty2->kind &&
      !((ty1->kind == Ty_int && ty2->kind == Ty_real) ||
	(ty1->kind == Ty_real && ty2->kind == Ty_int)))
    return 0;

  if (ty1->kind = Ty_record) {
    A_list list1 = ty1->u.record;
    A_list list2 = ty2->u.record;
  
    if (list1->length != list2->length)
      return 0;
    
    for (int i=0; i<list1->length; i++) {
      Ty_field f1 = (Ty_field)getNthElementList(list1,i);
      Ty_field f2 = (Ty_field)getNthElementList(list2,i);
      if (!Ty_compatible(f1->ty, f2->ty)) 
	return 0;
    }
  }
  return 1;
}

/* an implicit field is a field not declared in the table declaration,
but phisically stored. Currently, an in-memory table has an OID
column, which stores the pointer to each tuple */
int getNumOfExplictFields(A_list fields)
{
  int n = fields->length;

  /* check last field */
  Ty_field f = (Ty_field) getNthElementList(fields, n-1);

  return (f->implicit)? n-1: n;
}

/*
 *  extend the type of source within the limit of target
 */
int extendType(Ty_ty &source, Ty_ty target)
{
  //printf("here %d %d\n", source->kind, target->kind);
  if (source->kind != target->kind) {
    /* change type */
    if (source->kind == Ty_int && target->kind == Ty_real) 
      source = Ty_Real();
    else if (target->kind == Ty_ref &&
	     (source->kind == Ty_nil || source->kind == Ty_int || source->kind == Ty_record)) {
      /* internally, ref type is represented by an integer (pointer) */
    }
    else
      return 0;
  }
  if (source->kind == Ty_record) {
    A_list list1 = source->u.record;
    A_list list2 = target->u.record;

    //printf("source length %d, dest length %d\n", list1->length, getNumOfExplictFields(list2));

    if (list1 != list2) {
      if (list1->length != getNumOfExplictFields(list2)) {
	return 0;
      }

      for (int i=0; i<list1->length; i++) {
	Ty_field f1 = (Ty_field)getNthElementList(list1,i);
	Ty_field f2 = (Ty_field)getNthElementList(list2,i);

	if (!extendType(f1->ty, f2->ty)) 
	  return 0;
	/* change size */
	f1->size = f2->size;
      }
    }
  }
  return 1;
}
/*
 *  grow the type/size of the source to accomodate the target
 */
int makeCompatibleType(Ty_ty source, Ty_ty target)
{
  //printf("kinds %d, %d\n", source->kind, target->kind);
  if((source->kind == Ty_nil && target->kind == Ty_ref) || 
     (source->kind == Ty_ref && target->kind == Ty_nil)) {
    return 1;
  }
  if (source->kind != target->kind) {
    /* grow type */
    if (source->kind == Ty_int && target->kind == Ty_real ||
	source->kind == Ty_real && target->kind == Ty_int)
      source->kind = Ty_real;
    else 
      return 0;
  }
  
  if (source->kind == Ty_record) {
    A_list list1 = source->u.record;
    A_list list2 = target->u.record;
    if (list1->length != list2->length)
      return 0;
    for (int i=0; i<list1->length; i++) {
      Ty_field f1 = (Ty_field)getNthElementList(list1,i);
      Ty_field f2 = (Ty_field)getNthElementList(list2,i);

      if (!makeCompatibleType(f1->ty, f2->ty)) 
	return 0;

      /* grow size */
      if (f1->size<f2->size)
	f1->size = f2->size;
    }
  }
  return 1;
}

int isFieldNameP(A_list alist, S_symbol name)
{
  for (int i=0; i<alist->length; i++) {
    Ty_field f = (Ty_field)getNthElementList(alist, i);
    if (f->name == name) {
      return 1;
    }
  }
  return 0;
}

void displayTyField(Ty_field f)
{
  fprintf(stderr, "%s ", S_name(f->name));
  displayType(f->ty);
}

void displayType(Ty_ty ty)
{
  fprintf(stderr, "%s ", str_ty[ty->kind]);

  switch (ty->kind) {
  case Ty_record:
  case Ty_ref:
    fprintf(stderr, ": \n");
    for (int i=0; i<ty->u.record->length; i++) {
      Ty_field f = (Ty_field)getNthElementList(ty->u.record, i);
      displayTyField(f);
    }
    break;
  case Ty_nil:
  case Ty_int:
  case Ty_real:
  case Ty_string:
  case Ty_timestamp:
  case Ty_array:
  case Ty_void:
    fprintf(stderr, "\n");
    break;
  case Ty_name:
    fprintf(stderr, "%s ", S_name(ty->u.name.sym));
    displayType(ty->u.name.ty);
    break;
  }
}






