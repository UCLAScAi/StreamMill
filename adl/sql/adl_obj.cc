#include <stdio.h>
#include <sys/types.h>
#include <sql/adl_obj.h>
#include <sql/adl_sys.h>
#include <sql/util.h>
#include <sql/err.h>
#include <dbug/dbug.h>
#include <setjmp.h>
#include <string.h>

#include "const.h"

#define DEFAULT_NUM_DISPLAY_LEN 10

extern jmp_buf env;

char *object_type_name[] =
{
  /* simple obj : has default_display_len */
  "O_DATE", "O_MONEY", "O_TIME", 
  "O_NUM", "O_LONG_NUM", "O_REAL_NUM", "O_DOUBLE_NUM",

  "",				/* O_SIMPLE_DUMMY */

  /* complex obj */
  "O_DATATYPE", "O_STRUCT_DATATYPE", "O_LIST_TYPE", "O_SET_TYPE", "O_BAG_TYPE",
  "O_STRING", "O_FUNCT", 
  "O_DSTR", "O_LIST", "O_IDENT", "O_SELECT", "O_WITH_ITEM", 
  "O_SELECT_BLOCK",
  "O_SELECT_ITEM", "O_AGGR", "O_VALUES", "O_JOIN_ITEM", "O_COLUMN",
  "O_ORDER_ITEM", "O_TABLE_DEF", "O_UDF", "O_UDF_DEF", "O_UDF_AGGR", "O_UDF_AGGR_DEF",

  "O_CASE", "O_REF",

  "O_DB2_UDA",

  "O_LAM_OR_NODE", "O_SELECT_LAM", "O_TABLE_LAM", "O_FUNCTION_LAM", "O_MEMORY_LAM",
  "O_AGGREGATE_LAM", "O_VAR", "O_BUILTINFUNC", "O_CASE",

  /* dummy obj */
  "O_ANY", "O_STAR", "O_NULL"
};

int simple_type_size[][2] = /* display-size, data-size */
{
  {10, sizeof(int)},		/*O_DATE*/
  {10, sizeof(int)},		/*O_MONEY*/
  {10, sizeof(int)},		/*O_TIME*/
  {DEFAULT_NUM_DISPLAY_LEN, sizeof(int)}, /*O_NUM*/
  {DEFAULT_NUM_DISPLAY_LEN, sizeof(int)}, /*O_LONG_NUM*/
  {DEFAULT_NUM_DISPLAY_LEN, sizeof(float)},/*O_REAL_NUM*/
  {DEFAULT_NUM_DISPLAY_LEN, sizeof(double)}/*O_DOUBLE_NUM*/
};

/* {{{ create object */

nt_obj_t *newObj(object_type t, char* val)
{
  nt_obj_t *new_obj = (nt_obj_t*)ntMalloc(sizeof(nt_obj_t));
  new_obj->type=t;
  new_obj->flag=0;
  new_obj->value=val;
  new_obj->size = 0;
  new_obj->count = 1;
  return new_obj;
}
nt_obj_t *newStrObj(char *val)
{
  nt_obj_t *new_obj = newObj(O_STRING, val);
  new_obj->size=strlen(val)+1;
  return new_obj;
}
nt_obj_t *newNumObj(int n)
{
  nt_obj_t *new_obj = newObj(O_NUM, (char*)0);
  new_obj->size = simple_type_size[O_NUM][0];
  new_obj->value = (char*)malloc(new_obj->size);
  memcpy(new_obj->value, &n, new_obj->size);
  return new_obj;
}
nt_obj_t *newDoubleObj(double f)
{
  nt_obj_t *new_obj = newObj(O_DOUBLE_NUM, (char*)0);
  new_obj->size = simple_type_size[O_DOUBLE_NUM][0];
  new_obj->value = (char*)malloc(new_obj->size);
  memcpy(new_obj->value, &f, new_obj->size);
  return new_obj;
}
nt_obj_t *newFloatObj(float f)
{
  nt_obj_t *new_obj = newObj(O_REAL_NUM, (char*)0);
  new_obj->size = simple_type_size[O_REAL_NUM][0];
  new_obj->value = (char*)malloc(new_obj->size);
  memcpy(new_obj->value, &f, new_obj->size);
  return new_obj;
}
nt_obj_t *newObj_Str(object_type type, unsigned char *textRep, int tokLen)
{
  nt_obj_t *new_obj;

  switch(type) {
  case O_NUM:
  case O_LONG_NUM:
    new_obj = newNumObj(atoi((char*)textRep));
    break;
  case O_REAL_NUM: 
    {
      float f;
      sscanf((char *)textRep ,"%f", &f);
      new_obj = newFloatObj(f);
    }
    break;
  case O_DOUBLE_NUM:
    {
      double f;
      sscanf((char *)textRep ,"%lf", &f);
      new_obj = newDoubleObj(f);
    }
    break;
#ifndef DBUG_OFF
  default:
    EM_error(0, ERR_UNSUPPORTED_CASE_TYPE, __LINE__, __FILE__,
	     object_type_name[type], "newObj_Str");
    exit(1);
#endif    
  }

  return new_obj;
}

/* }}} */

/* {{{ object flag routines */
void setFlagObj(nt_obj_t *obj, int f)
{
  obj->flag |= f;
}

void clearFlagObj(nt_obj_t *obj, int f)
{
  obj->flag &= ~f;
}

void clearObj(nt_obj_t *obj)
{
  obj->value= (char*)0;
}

int isBooleanObj(nt_obj_t *obj)
{
#if 0
  /* currently only functors can be boolean */
  return (obj->type == O_FUNCT &&
	  FunctObj(obj)->type >= F_NOT);
#endif
}
/* }}} */

/* {{{ deleteObj */

void deleteObj(nt_obj_t *obj)
{
  if (obj && --obj->count >0) {
    switch(obj->type) {
    case O_DATATYPE:
    case O_NUM:
    case O_LONG_NUM:
    case O_REAL_NUM:
    case O_STAR:
    case O_NULL:
    case O_STRING:
      if (obj->value) 
	ntFree(obj->value);
      break;
      /*    case O_IDENT:
      deleteIdent(IdentObj(obj));
      break;
    case O_FUNCT:
      deleteFunct(FunctObj(obj));
      break;
    case O_CASE:
      deleteCase(CaseObj(obj));
      break;
    case O_BUILTINFUNC:
      deleteBuiltInFunc(BuiltInFuncObj(obj));
      break;
    case O_LIST:
      deleteList(ListObj(obj));
      break;
    case O_SELECT:
      deleteSelect(SelectObj(obj));
      break;
    case O_WITH_ITEM:
      deleteWithItem(WithItemObj(obj));
      break;
    case O_SELECT_BLOCK:
      deleteSelectBlock(SelectBlockObj(obj));
      break;
    case O_SELECT_ITEM:
      deleteSelectItem(SelectItemObj(obj));
      break;
    case O_UDF:
      deleteUdf(UdfObj(obj));
      break;
    case O_UDF_DEF:
      deleteUdfDef(UdfDefObj(obj));
      break;
    case O_AGGR:
      deleteAggr(AggrObj(obj));
      break;
    case O_UDF_AGGR_DEF:
      deleteUdfAggrDef(UdfAggrDefObj(obj));
      break;
#ifdef SQL_AG
    case O_DB2_UDA:
      deleteDb2Uda(Db2UdaObj(obj));
      break;
#endif
    case O_VALUES:
      deleteValues(ValuesObj(obj));
      break;
    case O_JOIN_ITEM:
      deleteJoinItem(JoinItemObj(obj));
      break;
    case O_COLUMN:
      deleteColumn(ColumnObj(obj));
      break;
    case O_TABLE_DEF:
      deleteTableDef(TableDefObj(obj));
      break;
    case O_LAM_OR_NODE:
      deleteLamOrNode(LamOrNodeObj(obj));
      break;
    case O_TABLE_LAM:
      deleteTableLam(TableLamObj(obj));
      break;
    case O_SELECT_LAM:
      deleteSelectLam(SelectLamObj(obj));
      break;
    case O_AGGREGATE_LAM:
      deleteLamAggrNode(LamAggrNodeObj(obj));
      break;
    case O_VAR:
      deleteVar(VarObj(obj));
      break;
    case O_FUNCTION_LAM:
    case O_DATE:
    case O_MONEY:
    case O_TIME:
    case O_ANY:*/
#ifndef DBUG_OFF
    default:
      EM_error(0, ERR_UNSUPPORTED_CASE_TYPE, __LINE__, __FILE__, obj->type, "deleteObj");
      exit(1);
#endif
    }
    ntFree(obj);
  }
}

/* }}} */

/* {{{ readObj */

nt_obj_t *readObj(nt_obj_t *stream)
{
  int type = ntReadNum(stream);
  nt_obj_t *obj;

  if (type == -1)
    return (nt_obj_t*)0;

  obj = newObj((object_type)type, (char*)0);

  switch(obj->type) {
#if 0
  case O_DATATYPE:
    DataTypeObj(obj)=readDataType(stream);
    break;
#endif
  case O_NUM:
  case O_LONG_NUM: 
  case O_REAL_NUM:
  case O_DOUBLE_NUM: 
    {
      int size = simple_type_size[obj->type][1];
      obj->value = (char*)ntMalloc(size);
      ntRead(obj->value, stream, size);
    }
    break;
  case O_STRING:
    {
      int len = ntReadNum(stream);
      obj->value = ntReadStr(stream, len); /* reads 0 */
      obj->size = len;
    }
    break;
    /*  case O_IDENT:
    IdentObj(obj) = readIdent(stream);
    break;
  case O_CASE:
    CaseObj(obj) = readCase(stream);
    break;
  case O_FUNCT:
    FunctObj(obj) = readFunct(stream);
    break;
  case O_LIST:
    ListObj(obj) = readList(stream);
    break;
  case O_SELECT:
    SelectObj(obj) = readSelect(stream);
    break;
  case O_WITH_ITEM:
    WithItemObj(obj) = readWithItem(stream);
    break;
  case O_SELECT_BLOCK:
    SelectBlockObj(obj) = readSelectBlock(stream);
    break;
  case O_SELECT_ITEM:
    SelectItemObj(obj) = readSelectItem(stream);
    break;
  case O_BUILTINFUNC:
    BuiltInFuncObj(obj)=readBuiltInFunc(stream);
    break;
  case O_UDF:
    UdfObj(obj) = readUdf(stream);
    break;
  case O_UDF_DEF:
    UdfDefObj(obj) = readUdfDef(stream);
    break;
  case O_UDF_AGGR_DEF:
    UdfAggrDefObj(obj) = readUdfAggrDef(stream);
    break;
  case O_VALUES:
    ValuesObj(obj) = readValues(stream);
    break;
  case O_JOIN_ITEM:
    JoinItemObj(obj) = readJoinItem(stream);
    break;
  case O_COLUMN:
    ColumnObj(obj) = readColumn(stream);
    break;
  case O_ORDER_ITEM:
    OrderItemObj(obj) = readOrderItem(stream);
    break;
  case O_TABLE_DEF:
    TableDefObj(obj) = readTableDef(stream);
    break;
  case O_REF:
    RefObj(obj) = readRef(stream);
    break;
  case O_NULL:
  case O_STAR:
  case O_DATE:
  case O_MONEY:
  case O_TIME:
  case O_ANY:*/
#ifndef DBUG_OFF
  default:
    EM_error(0, ERR_UNSUPPORTED_CASE_TYPE, __LINE__, __FILE__, object_type_name[obj->type],
	       "readObj");
    exit(1);
#endif
  }
  return obj;
}

/* }}} */

/* {{{ displayObj */

int displayFixedLengthObj(nt_obj_t *stream, nt_obj_t *obj)
{
  char buf[MAX_STR_LEN];
  register int size = obj->size;

  switch (obj->type) {
  case O_STRING:
    {
      int len = strlen(StrObj(obj));
      if (len>size)
	ntWrite(stream, StrObj(obj), size);
      else {
	char empty[MAX_STR_LEN];
	memset(empty, ' ', size-len);
	ntWrite(stream, StrObj(obj), len);
	ntWrite(stream, empty, size-len);
      }
    }
    break;
  case O_NUM:
    sprintf(buf, "%*d", size, NumObj(obj));
    ntWrite(stream, buf,  size);
    break;
  case O_REAL_NUM:
    sprintf(buf, "%*f", size, RealNumObj(obj));
    ntWrite(stream, buf, size);
    break;
  case O_DOUBLE_NUM:
    sprintf(buf, "%*lf", size, DoubleNumObj(obj));
    ntWrite(stream, buf, size);
    break;
  case O_LIST:
    {
      nt_obj_t *s = makeStream(O_STRING, buf, DISPLAY_VAR_VALUE);
      displayObj(s,obj);
      size=MIN(StrObj(s) - buf,40);
      ntWrite(stream, buf, size);
      closeStream(s);
    }
    break;
#ifndef DBUG_OFF
  default:
    EM_error(0, ERR_UNSUPPORTED_CASE_TYPE, __LINE__, __FILE__, 
	       object_type_name[obj->type], "displayFixedLengthObj");
    exit(1);
#endif
  }
  return size;
}

void displayObj(nt_obj_t *s, nt_obj_t *obj)
{
  if (obj == (nt_obj_t*)0) {
    if (s->flag & BINARY_STREAM)
      ntWriteNum(s, -1); 
    return;
  }
  
  if (s->flag & BINARY_STREAM) {
    ntWriteNum(s, (int)obj->type);
    if (isSimpleObj(obj)) {
      ntWrite(s, obj->value, simple_type_size[obj->type][1]);
      return;
    }
  }
  
  switch(obj->type) {
#if 0
  case O_DATATYPE:
    displayDataType(s, DataTypeObj(obj));
    break;
#endif
  case O_NUM:
    ntPrintf(s, "%d", NumObj(obj));
    break;
  case O_LONG_NUM:
    ntPrintf(s, "%d", LongNumObj(obj));
    break;
  case O_REAL_NUM:
    ntPrintf(s, "%f", RealNumObj(obj));
    break;
  case O_DOUBLE_NUM:
    ntPrintf(s, "%lf", DoubleNumObj(obj));
    break;
  case O_STRING:
    {
      if (s->flag & BINARY_STREAM) {
	int len = strlen(obj->value)+1;	/* writes 0 */
	ntWriteNum(s, len);
	ntWrite(s, obj->value, len); 
      } else 
	ntPrintf(s, "%s", StrObj(obj));
    }
    break;
    /*  case O_IDENT:
    displayIdent(s, IdentObj(obj));
    break;*/
  case O_STAR:
    ntPrintf(s, "*");
    break;
  case O_NULL:
    ntPrintf(s, "NULL");
    break;
  case O_DATE:
    break;
  case O_MONEY:
    break;
  case O_TIME:
    break;
  case O_ANY:
    break;

#if 0

  case O_CASE:
    displayCase(s, CaseObj(obj));
    break;
  case O_FUNCT:
    displayFunct(s, FunctObj(obj));
    break;
  case O_LIST:
    displayList(s, ListObj(obj), ",");
    break;
  case O_SELECT:
    displaySelect(s, SelectObj(obj));
    break;
  case O_WITH_ITEM:
    displayWithItem(s, WithItemObj(obj));
    break;
  case O_SELECT_BLOCK:
    displaySelectBlock(s, SelectBlockObj(obj));
    break;
  case O_SELECT_ITEM:
    displaySelectItem(s, SelectItemObj(obj));
    break;
  case O_UDF_DEF:
    displayUdfDef(s, UdfDefObj(obj));
    break;
  case O_UDF:
    displayUdf(s, UdfObj(obj));
    break;
  case O_UDF_AGGR_DEF:
    displayUdfAggrDef(s, UdfAggrDefObj(obj));
    break;
  case O_UDF_AGGR:
    displayUdfAggr(s, UdfAggrObj(obj));
    break;
#ifdef SQL_AG
  case O_DB2_UDA:
    displayDb2Uda(s, Db2UdaObj(obj));
    break;
#endif
  case O_VALUES:
    displayValues(s, ValuesObj(obj));
    break;
  case O_JOIN_ITEM:
    displayJoinItem(s, JoinItemObj(obj));
    break;
  case O_COLUMN:
    displayColumn(s, ColumnObj(obj));
    break;
  case O_TABLE_DEF:
    displayTableDef(s, TableDefObj(obj));
    break;
  case O_TABLE_LAM:
    displayPred(s, TableLamObj(obj)->pred);
    break;
  case O_LAM_OR_NODE:
    displayList(s, LamOrNodeObj(obj)->nodes,"\n");
    break;
  case O_AGGREGATE_LAM:
    displayUdaRoutines(s, LamAggrNodeObj(obj));
    /* fall through */
  case O_SELECT_LAM:
    {
      list_t *list = newList();
      appendElementList(list, obj);
      while (list->length>0) {
	nt_obj_t *lam = removeNthElementList(list, 0);
	displayLamRules(s, lam, list);
      } 
      deleteList(list);
    }
    break;
  case O_VAR:
    displayVar(s, VarObj(obj));
    break;
  case O_BUILTINFUNC:
    displayBuiltInFunc(s, BuiltInFuncObj(obj));
    break;
  case O_AGGR:			/* standard builtin aggregate */
    displayAggr(s, AggrObj(obj));
    break;
  case O_REF:
    displayRef(s, RefObj(obj));
    break;
  case O_FUNCTION_LAM:
#endif
#ifndef DBUG_OFF
  default:
    EM_error(0, ERR_UNSUPPORTED_CASE_TYPE, __LINE__, __FILE__,
	       object_type_name[obj->type], "displayObj");
    exit(1);
#endif
  }
}

/* }}} */

/* {{{ copyObj */

nt_obj_t* copyObj(nt_obj_t *obj)
{
  nt_obj_t *new_obj = (nt_obj_t*)0;

  if (obj != (nt_obj_t*)0) {
    switch(obj->type)
      {
#if 0
      case O_REF:
	new_obj = newObj(O_REF, (char*)copyRef(RefObj(obj)));
	break;
      case O_BUILTINFUNC:
	new_obj = newObj(O_BUILTINFUNC, 
		     (char*)copyBuiltInFunc(BuiltInFuncObj(obj)));
	break;
      case O_DATATYPE:
	new_obj = newObj(O_DATATYPE, (char*)copyDataType(DataTypeObj(obj)));
	break;
#endif
      case O_NUM:
      case O_LONG_NUM:
      case O_REAL_NUM:
      case O_DOUBLE_NUM:
	{
	  int size = simple_type_size[obj->type][1];
	  new_obj = (nt_obj_t*)ntMalloc(sizeof(nt_obj_t));
	  new_obj->type = obj->type;
	  new_obj->value = (char*)ntMalloc(size);
	  new_obj->size = obj->size;
	  memcpy(new_obj->value, obj->value, size);
	}
	break;
      case O_STRING:
	{
	  new_obj = newObj(O_STRING, (char*)0);
	  new_obj->value = (char*)ntMalloc(obj->size);
	  strcpy(new_obj->value, StrObj(obj));
	  new_obj->size = obj->size;
	}
	break;
#if 0
      case O_IDENT:
	new_obj = newObj(O_IDENT, (char*)copyIdent(IdentObj(obj)));
	break;
      case O_NULL:
	new_obj = newObj(O_NULL, (char*)0);
	break;
      case O_CASE:
	new_obj = newObj(O_CASE, (char*)copyCase(CaseObj(obj)));
	break;
      case O_FUNCT:
	new_obj = newObj(O_FUNCT, (char*)copyFunct(FunctObj(obj)));
	break;
      case O_LIST:
	new_obj = newObj(O_LIST, (char*)copyList(ListObj(obj)));
	break;
      case O_STAR:
	new_obj = newObj(O_STAR, (char*)0);
	break;
      case O_SELECT:
	new_obj = newObj(O_SELECT, (char*)copySelect(SelectObj(obj)));
	break;
      case O_WITH_ITEM:
	new_obj = newObj(O_WITH_ITEM, (char*)copyWithItem(WithItemObj(obj)));
	break;
      case O_SELECT_BLOCK:
	new_obj = newObj(O_SELECT_BLOCK, (char*)copySelectBlock(SelectBlockObj(obj)));
	break;
      case O_SELECT_ITEM:
	new_obj = newObj(O_SELECT_ITEM, (char*)copySelectItem(SelectItemObj(obj)));
	break;
      case O_UDF_DEF:
	new_obj = newObj(O_UDF_DEF, (char*)copyUdfDef(UdfDefObj(obj)));
	break;
      case O_AGGR:
	new_obj = newObj(O_AGGR, (char*)copyAggr(AggrObj(obj)));
	break;
      case O_UDF_AGGR:
	new_obj = newObj(O_UDF_AGGR, (char*)copyUdfAggr(UdfAggrObj(obj)));
	break;
      case O_UDF_AGGR_DEF:
	new_obj = newObj(O_UDF_AGGR_DEF, (char*)copyUdfAggrDef(UdfAggrDefObj(obj)));
	break;
      case O_VALUES:
	new_obj = newObj(O_VALUES, (char*)copyValues(ValuesObj(obj)));
	break;
      case O_JOIN_ITEM:
	new_obj = newObj(O_JOIN_ITEM, (char*)copyJoinItem(JoinItemObj(obj)));
	break;
      case O_COLUMN:
	new_obj = newObj(O_COLUMN, (char*)copyColumn(ColumnObj(obj)));
	break;
      case O_ORDER_ITEM:
	new_obj = newObj(O_ORDER_ITEM, (char*)copyOrderItem(OrderItemObj(obj)));
	break;
      case O_TABLE_DEF:
	new_obj = newObj(O_TABLE_DEF, (char*)copyTableDef(TableDefObj(obj)));
	break;
      case O_VAR:		/* vars have same name but not sharing obj */
	new_obj = newObj(O_VAR, (char*)copyVar(VarObj(obj)));
	break;
      case O_UDF:
      case O_DATE:
      case O_MONEY:
      case O_TIME:
      case O_ANY:
#endif
#ifndef DBUG_OFF
      default:
	EM_error(0, ERR_UNSUPPORTED_CASE_TYPE, __LINE__, __FILE__,
		   object_type_name[obj->type], "copyObj");
	exit(1);
#endif
      }
  }
  if (new_obj)
    new_obj->flag=obj->flag;
  return new_obj;
}

/* }}} */
#if 0
/* {{{ getObjDataType */
object_type getObjDataType(nt_obj_t *obj)
{
  return getObjValue(obj)->type;
}
/* }}} */

/* {{{ getObjValue */
nt_obj_t *getObjValue(nt_obj_t *obj)
{
  switch (obj->type) {
  case O_LAM_OR_NODE:		/* subquery: a>(select ..) */
    {
      pred_t *p = getLamNodePred(obj);
      if (p->args->length==1) {
	nt_obj_t *arg = getNthElementList(p->args, 0);
	if (arg->type == O_VAR) 
	  arg=VarObj(arg)->obj;
	return arg;
      } else {
	EM_error(0, ERR_SUBQUERY, __LINE__, __FILE__, "wrong return arity");
	longjmp(env,0);
      }
    }
    break;
  case O_NUM:
  case O_LONG_NUM:
  case O_REAL_NUM:
  case O_DOUBLE_NUM:
  case O_STRING:
    return obj;
  case O_VAR:
    return VarObj(obj)->obj;
  case O_FUNCT:
    return FunctObj(obj)->value;
  case O_BUILTINFUNC:
    return BuiltInFuncObj(obj)->value;
  case O_CASE:
    return CaseObj(obj)->value[0];
  case O_UDF:
#ifndef DBUG_OFF
  default:
    EM_error(0, ERR_UNSUPPORTED_CASE_TYPE, __LINE__, __FILE__, 
	       object_type_name[obj->type], "getObjValue");
    exit(1);
#endif
  }
}
/* }}} */

/* {{{ equalObj */

int equalObj(nt_obj_t *obj1, nt_obj_t *obj2)
{
  if (obj1==obj2)
    return 1;
  if (obj1==(nt_obj_t*)0 || obj2==(nt_obj_t*)0)
    return 0;
  if (obj1->type != obj2->type)
    return 0;
  switch (obj1->type) {
  case O_NUM:
  case O_LONG_NUM:
  case O_REAL_NUM:
  case O_DOUBLE_NUM:
    return memcmp(obj1->value, obj2->value, 
		  simple_type_size[obj1->type][1])==0;
  case O_STRING:
    return (strcmp(StrObj(obj1), StrObj(obj2))==0);
  case O_NULL:
  case O_STAR:
    return 1;
  case O_FUNCT:
    return equalFunct(FunctObj(obj1), FunctObj(obj2));
  case O_LIST:
    return equalList(ListObj(obj1), ListObj(obj2));
  case O_IDENT:
    return equalIdent(IdentObj(obj1), IdentObj(obj2));
  case O_SELECT:
  case O_WITH_ITEM:
  case O_SELECT_BLOCK:
  case O_SELECT_ITEM:
  case O_JOIN_ITEM:
  case O_VALUES:
    return 0;
#ifdef SQL_AG
  case O_DB2_UDA:
    return equalDb2Uda(Db2UdaObj(obj1), Db2UdaObj(obj2));
#endif
  case O_UDF_AGGR:
    return equalUdfAggr(UdfAggrObj(obj1), UdfAggrObj(obj2));
  case O_AGGR:
    return equalAggr(AggrObj(obj1), AggrObj(obj2));
  case O_UDF_DEF:
  case O_DATE:
  case O_MONEY:
  case O_TIME:
  case O_ANY:
  case O_COLUMN:
  case O_ORDER_ITEM:
  case O_TABLE_DEF:
#ifndef DBUG_OFF
  default:
    EM_error(0, ERR_UNSUPPORTED_CASE_TYPE, __LINE__, __FILE__, 
	       object_type_name[obj1->type], "equalUdf");
    exit(1);
#endif
  }
}

/* }}} */

/* {{{ replaceObj */

nt_obj_t* 
replaceObj(nt_obj_t *obj, nt_obj_t *old, nt_obj_t *new_obj, 
	   int option, int (*fp)())
{
  if (!obj)
    return obj;

  if (equalObj(obj, old) && fp(obj, old)) {
    deleteObj(obj);
    return copyObj(new_obj);
  }
    
  switch(obj->type)
    {
    case O_UDF_AGGR:
    case O_DATE:
    case O_MONEY:
    case O_TIME:
    case O_ANY:
    case O_NUM:
    case O_LONG_NUM:
    case O_REAL_NUM:
    case O_NULL:
    case O_IDENT:
    case O_STAR:
    case O_STRING:
      break;
    case O_FUNCT: 
      {
	funct_t *f = FunctObj(obj);
	f->args[0]=replaceObj(f->args[0], old, new_obj, option, fp);
	f->args[1]=replaceObj(f->args[1], old, new_obj, option, fp);
      }
    break;
    case O_LIST:
      {
	int i;
	list_t *list = ListObj(obj);
	for (i=0;i<list->length;i++)
	  list->elements[i]=replaceObj(list->elements[i], old, new_obj, option, fp);
      }
      break;
    case O_SELECT:
      {
	select_t *s =SelectObj(obj);
	s->with_list=replaceObj(s->with_list, old, new_obj, option, fp);
	s->query_list=replaceObj(s->query_list, old, new_obj, option, fp);
	s->order_list=replaceObj(s->order_list, old, new_obj, option, fp);
      }
      break;
    case O_WITH_ITEM:
      {
	with_item_t *w =WithItemObj(obj);
	w->opt_columns=replaceObj(w->opt_columns, old, new_obj, option, fp);
	w->select_block_list=replaceObj(w->select_block_list, old, new_obj, option, fp);
      }
      break;
    case O_SELECT_BLOCK:
      {
	select_block_t *s =SelectBlockObj(obj);
	s->select_list=replaceObj(s->select_list,old, new_obj, option, fp);
	s->join_table_list=replaceObj(s->join_table_list,old, new_obj, option, fp);
	s->where_cond=replaceObj(s->where_cond,old, new_obj, option, fp);
	s->group_by_list=replaceObj(s->group_by_list,old, new_obj, option, fp);
	s->having_cond=replaceObj(s->having_cond,old, new_obj, option, fp);
      }
      break;
    case O_SELECT_ITEM:
      {
	select_item_t *s = SelectItemObj(obj);
	s->expr=replaceObj(s->expr,old, new_obj, option, fp);
      }
      break;
    case O_DB2_UDA:
      {
	db2_uda_t *u = Db2UdaObj(obj);
	u->args=replaceObj(u->args,old, new_obj, option, fp);
      }
      break;
    case O_VALUES:

      break;
    }

  return obj;
}

/* }}} */

/* {{{ pair */
pair_t *newPair(char* name, char *value)
{
  pair_t *new_obj = (pair_t*)ntMalloc(sizeof(pair_t));
  new_obj->name = name;
  new_obj->value = value;
  return new_obj;
}
void deletePair(pair_t *pair)
{
  ntFree(pair);
}
/* }}} */
#endif







