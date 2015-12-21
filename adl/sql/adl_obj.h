#ifndef NT_OBJ_H
#define NT_OBJ_H

#include <sys/types.h>

/***********************************************************************
                              NT OBJECT TYPE
***********************************************************************/

typedef enum 
{
  /* simple obj : has default_display_len */
  O_DATE, O_MONEY, O_TIME, 
  O_NUM, O_LONG_NUM, O_REAL_NUM, O_DOUBLE_NUM,

  O_SIMPLE_DUMMY, 

  /* complex obj */
  O_DATATYPE, O_STRUCT_DATATYPE, O_LIST_TYPE, O_SET_TYPE, O_BAG_TYPE,
  O_STRING, O_FUNCT, 
  O_DSTR, O_LIST, O_IDENT, O_SELECT, O_WITH_ITEM, 
  O_SELECT_BLOCK,
  O_SELECT_ITEM, O_AGGR, O_VALUES, O_JOIN_ITEM, O_COLUMN,
  O_ORDER_ITEM, O_TABLE_DEF, O_UDF, O_UDF_DEF, O_UDF_AGGR, O_UDF_AGGR_DEF,

  O_CASE, O_REF,

  O_DB2_UDA,

  O_LAM_OR_NODE, O_SELECT_LAM, O_TABLE_LAM, O_FUNCTION_LAM, O_MEMORY_LAM,
  O_AGGREGATE_LAM, O_VAR, O_BUILTINFUNC, 

  /* dummy obj */
  O_ANY, O_STAR, O_NULL
} object_type;

/* for obj flags */
#define BINARY_STREAM		0x0001
#define INPUT_STREAM		0x0002
#define HAS_AGGREGATE		0x0004
#define RECURSIVE		0x0008
#define DISPLAY_VAR_VALUE	0x0010
#define OBJ_TMP_FLAG		0x8000

#ifdef SQL_AG

#define NO_UDA		0
#define EARLY_UDA	1
#define FINAL_UDA	2
#define EARLY_FINAL_UDA 3
#define CONFLICT_UDA	4

#endif

typedef struct obj_s {
  object_type type;
  char *value;
  int size;
  int flag;
  int count;			/* object sharing */
} nt_obj_t;


int isBooleanObj(nt_obj_t *obj);

/* simple obj does not have indepedent structure */
#define isSimpleObj(obj)	isSimpleType((obj)->type)
#define isSimpleType(type)	((type)<O_SIMPLE_DUMMY)

#define isConstObj(obj) (obj && (obj->type == O_NUM || \
				 obj->type == O_LONG_NUM || \
				 obj->type == O_REAL_NUM || \
				 obj->type == O_DOUBLE_NUM || \
				 obj->type == O_STRING))
#define isNumericType(type) ((type)>=O_NUM && (type)<=O_DOUBLE_NUM)

#define LongNumObj(o) (*((long*)((o)->value)))
#define NumObj(o) (*((int*)((o)->value)))
#define RealNumObj(o) (*((float*)((o)->value)))
#define DoubleNumObj(o) (*((double*)((o)->value)))
#define StrObj(o) ((char*)((o)->value))


nt_obj_t* newObj(object_type type, char* obj);
nt_obj_t* newNumObj(int n);
nt_obj_t* newFloatObj(float f);
nt_obj_t* newDoubleObj(double d);
nt_obj_t* newStrObj(char * d);
nt_obj_t* newObj_Str(object_type type, char* text, int tokLen);

nt_obj_t *getObjValue(nt_obj_t *obj);
object_type getObjDataType(nt_obj_t *obj);

void displayObj(nt_obj_t* stream, nt_obj_t *obj);
int displayFixedLengthObj(nt_obj_t *stream, nt_obj_t *obj);
nt_obj_t *readObj(nt_obj_t* stream);

nt_obj_t* copyObj(nt_obj_t *obj);
nt_obj_t* replaceObj(nt_obj_t *obj, nt_obj_t *obj1, nt_obj_t *obj2, 
		     int option, int (*fp)());
void deleteObj(nt_obj_t *obj);

void setFlagObj(nt_obj_t *obj, int flag);
void clearFlagObj(nt_obj_t *obj, int flag);

typedef struct pair_s {
  char *name;
  char *value;
} pair_t;

pair_t *newPair(char* name, char *value);
void deletePair(pair_t *pair);

#endif








