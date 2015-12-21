#ifndef NTSQL_TYPES_H
#define NTSQL_TYPES_H

#include <symbol.h>
#include <list.h>
#include <const.h>
/*
 * types.h - 
 *
 * All types and functions declared in this header file begin with "Ty_"
 * Linked list types end with "..list"
 * Make sure modify Ty_size and Ty_sname as well when modifying ty_t!!!
 */

typedef enum {
  Ty_record,     
  Ty_nil,        
  Ty_int,        
  Ty_real,       
  Ty_string,     
  Ty_array,      
  Ty_name,       
  Ty_void, 
  Ty_ref,
  Ty_timestamp,
  Ty_iext,
  Ty_rext,
  Ty_cext,
  Ty_text
} ty_t;


typedef struct Ty_ty_ *Ty_ty;
/*typedef struct Ty_tyList_ *Ty_tyList;*/
typedef struct Ty_field_ *Ty_field;
/*  typedef struct Ty_fieldList_ *Ty_fieldList; */

struct Ty_ty_ 
{
  void *pp;			/* points to its parent structure */
  adl_t ppt;			/* type of its parent structure */

  ty_t kind;

  union { 
    A_list record;		/* memeber is of type Ty_field */
    Ty_ty array;
    struct {
      S_symbol sym; 
      Ty_ty ty;
    } name;
  } u;
};

Ty_ty copyTy(Ty_ty);

struct Ty_field_ 
{
  S_symbol name; 
  Ty_ty ty; 
  S_symbol ref;			/* reference type */
  int implicit;			/* implicit field: e.g., OID field of a table */
  int iskey;
  int isTimekey;              // stream has at most one Time Key (TK)
  int size;
};

Ty_ty Ty_Nil(void);
Ty_ty Ty_Int(void);
Ty_ty Ty_REF(void);
Ty_ty Ty_Real(void);
Ty_ty Ty_Timestamp(void);
Ty_ty Ty_IExt(void);
Ty_ty Ty_RExt(void);
Ty_ty Ty_CExt(void);
Ty_ty Ty_TExt(void);
Ty_ty Ty_String(void);
Ty_ty Ty_Void(void);
Ty_ty Ty_Ref(A_list fields);
Ty_ty Ty_Ref(Ty_ty record_ty);
Ty_ty Ty_Record(A_list fields);
Ty_ty Ty_Array(Ty_ty ty);
Ty_ty Ty_Name(S_symbol sym, Ty_ty ty);

Ty_field Ty_Field(S_symbol name, Ty_ty ty, int size);
Ty_field Ty_Field(S_symbol name, S_symbol ref, Ty_ty ty);

void Ty_print(Ty_ty t);
void TyList_print(A_list list);
int Ty_compatible(Ty_ty ty1, Ty_ty ty2);
int getNumOfExplictFields(A_list fields);

int extendType(Ty_ty &source, Ty_ty target);
int makeCompatibleType(Ty_ty source, Ty_ty target);
void displayType(Ty_ty ty);
void displayTyField(Ty_field f);
/* check if there is a field named "name" in a list of fields */
int isFieldNameP(A_list alist, S_symbol name);

int getDisplaySize(Ty_ty t);
int getStorageSize(Ty_ty t);

typedef struct timeval timestamp;
typedef enum {
  _ADL_WIN_ROW, 
  _ADL_WIN_TIME
} _adl_win_type;

#endif /* NTSQL_TYPES_H */


