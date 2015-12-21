#ifndef SQL_LEX_H
#define SQL_LEX_H

typedef struct symtab_s {
  char	*name;
  int	tok;
} symtab_t;

typedef enum 
{ 
  START_STATE,			/* 0 */
  CHAR_STATE,			/* 1 */ 
  IDENT_STATE,			/* 2 */ 
  IDENT_SEP_STATE,		/* 3 */ 
  IDENT_START_STATE,		/* 4 */ 
  COMPLETE_IDENT_STATE,		/* 5 */ 
  SIGNED_NUMBER_STATE,		/* 6 */ 
  REAL_STATE,			/* 7 */ 
  CMP_OP_STATE,			/* 8 */ 
  STRING_STATE,			/* 9 */ 
  COMMENT_STATE,		/* 10 */
  END_STATE,			/* 11 */
  ALLOW_SIGNED_NUMBER,		/* 12 */
  NUMBER_STATE,			/* 13 */
  INT_OR_REAL_STATE,		/* 14 */
  REAL_OR_POINT_STATE,		/* 15 */
  BOOL_STATE,			/* 16 */
  EOL_STATE,			/* 17 */
  ESCAPE_STATE,			/* 18 */
  LONG_COMMENT_STATE,		/* 19 */
  C_CODE_STATE                  /* 20 */
} lex_state_t;			

#endif

