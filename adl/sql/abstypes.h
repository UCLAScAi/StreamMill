#ifndef _ABSTYPES_H
#define _ABSTYPES_H

typedef int A_pos;

typedef enum {
  A_UNKNOWN,
  A_EXP,
  /*  A_EFIELD, */
  A_DEC,
  A_SELECT_ITEM,
  A_QUN,
  A_SYMBOL,
  A_MODELITEM,
  A_FLOW,
  A_FIELD,
  A_LIST,
  A_ORDERITEM,
  A_TABLE_COLUMN,
  A_TY,
  A_VAR,
  A_REF
} adl_t;

typedef enum {
  A_simple_var_spec, A_field_var_spec, A_subscript_var_spec,
  A_simpleVar, A_fieldVar, A_subscriptVar, A_refVar,
  A_varRef, A_refRef,
  A_varExp,
  A_refExp /*10*/,
  A_nilExp,
  A_intExp,
  A_realExp,
  A_stringExp,
  A_timestampExp, 
  A_callExp,
  A_opExp,
  A_recordExp,
  A_seqExp,
  A_assignExp/*20*/,
  A_ifExp,
  A_whileExp,
  A_forExp,
  A_breakExp,
  A_letExp,
  A_arrayExp,
  A_selectExp,
  A_functionDec, A_aggregateDec, A_externDec,/*30*/
  A_modelTypeDec,
  A_varDec, A_tabVarDec, A_dynamicVarDec, A_typeDec,
  A_nameTy, A_recordTy, A_arrayTy,
  A_sqloprExp,
  A_runtaskExp/*40*/,
  A_createviewExp,
  A_createstreamExp,
  A_streamVarDec
} absyn_t;

#endif
