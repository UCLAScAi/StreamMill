#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>

#include <sql/absyn.h>
#include <sql/adl_yacc.h>
#include "lex.h"


#include "list.h"
#include <sql/adl_sys.h>


//I could not include SMLog and use the function 
//FIXME so I had to add this function here
//#include <SMLog.h>
#include <stdarg.h>
#define SMLOG_LEVEL 15
#define SMLOG_FILENAME "./SMLOG.log"
void SMLOG3(int level, const char* Format, ...)
{
	if (level > SMLOG_LEVEL)
		return;
 
/*	va_list Arguments;
	va_start(Arguments, Format);
	printf( Format, Arguments);
	printf("\n");
	va_end(Arguments);
*/	
	//   since SM redirects stdout to different files
	//   it might be kind of risky to prompt everything
	//   into it. so you may have to use following pices 
	//   of code to write them into a file.
	
	FILE *fp;
	if((fp=fopen(SMLOG_FILENAME, "a")) == NULL) 
	{
		printf("Cannot open file: %s \n", SMLOG_FILENAME );
		return;
	}
	//FIXME: add date and time information
	//FIXME: write the log in bunches
	va_list Arguments;
	va_start(Arguments, Format);
	vfprintf(fp, Format, Arguments);
	fprintf(fp, "\n");
	va_end(Arguments);

	fclose(fp);
}




extern YYLTYPE yylloc;

char	*yytext;
char	yyprev[120];
int	yylineno;
char	*lineStartPtr;
static	char 		*tokPtr;
static	char		*tokStart;
static	lex_state_t	prev_state, state, next_state;
int in_sql;
int in_modeltype;


char* tolower( char* str )
{
	if (str==NULL)	
		return str;
	
	for(int i = 0; str[i] != '\0'; i++)
		str[i] = tolower(str[i]);
	
	return str;
}


/*************************************************************
  Macros for handling the scanner's internal pointers
 *************************************************************/

#define yyGet()		(*tokPtr++)
#define yyUnget()	tokPtr--
#define yyPeek()	(*tokPtr)
#define yySkip()	(tokPtr++)

#ifdef STANDALONE
YYSTYPE yylval;
#else
extern YYSTYPE yylval;
#endif

/*************************************************************
  given a char (0-255), state_map is used to speed up state switch
  Generated by genstatemap. DON'T EDIT.
 *************************************************************/
static unsigned char state_map[256]=
{
	17, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 8, 9, 10, 1, 1, 16, 9, 1, 1, 1, 6, 1, 6, 15, 19, 
	13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 1, 1, 8, 8, 8, 1, 
	20, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 18, 1, 1, 2, 
	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 16, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
};

#if 0

static symtab_t ntsym[] = 
{
	{ "compile",	COMPILE_SYM},
	{ "db2",	DB2_SYM},
	{ "exit",	QUIT_SYM},
	{ "help",	HELP_SYM},
	{ "list",	LIST_SYM},
	{ "quit",	QUIT_SYM},
};
#endif

symtab_t funcsym[] = 
{
	{"abs",	BF_ABS},
	{"case",	BF_CASE},
	{"length",	BF_LENGTH},
	{"ltrim",	BF_LTRIM},
	{"max",	BF_MAX},
	{"min",	BF_MIN},
	{"oldest",    BF_OLDEST},
	{"rtrim",	BF_RTRIM},
	{"sqrt",	BF_SQRT},
	{"substr",	BF_SUBSTR},
	{"trim",	BF_TRIM},
	{"xmlattributes", XML_ATTR},
	{"xmlelement", XML_XE},
};
symtab_t aggrsym[] = 
{
	{"avg",	AGGR_AVG},
	{"count",	AGGR_COUNT},
	/*  these two are constructed in builtin-function */
	//{"max",	AGGR_MAX},
	//{"min",	AGGR_MIN},
	{"sum",	AGGR_SUM},
	{"xmlagg",    AGGR_XA},
};

static symtab_t sqlsym[] = 
{
	{ "++",	PLUSPLUS},
	{ "+=",	PLUSEQL},
	{ "--",	MINUSMINUS},
	{ "-=",	MINUSEQL},
	{ "->",	POINTER},
	{ "<=",	LE},
	{ "<>",	NE},
	{ ">=",	GE},
	{ "aggregate",AGGREGATE},
	{ "all",	ALL},
	{ "and",	AND_},
	{ "array",	ARRAY},
	{ "as",	AS},
	{ "asc",	ASC},
	{ "bag",	BAG},
	{ "break",	BREAK},
	{ "btree",	BTREE},
	{ "by",	BY},
	{ "caggregate",CAGGREGATE},
	{ "case",	CASE},
	{ "char",	CHAR_SYM},
	{ "character",CHAR_SYM},
	//  { "clike",	CLIKE},
	{ "create",	SQL_CREATE},
	{ "day",	DAY},
	{ "defferred", DEFFERRED},
	{ "defined",	DEFINED},
	{ "delete",	SQL_DELETE},
	{ "desc",	DESC},
	{ "distinct",	DISTINCT},
	{ "double",	REAL_SYM},
	{ "drop",	SQL_DROP},
	{ "dynamic",  DYNAMIC},
	{ "else",	ELSE},
	{ "end",	END},
	{ "except",	EXCEPT}, 
	{ "exists",	EXISTS},                
	{ "expire",	EXPIRE},
	{ "external",	EXTERNAL},
	{ "false",    FALSE},
	{ "float",	REAL_SYM},
	{ "flow",     FLOW},
	// { "for",	FOR},
	{ "from",	FROM},
	{ "function",	FUNCTION},
	{ "group",	GROUP},
	{ "hash",     HASH},
	{ "having",	HAVING},
	{ "heartbeat",	HEARTBEAT},
	{ "hour",	HOUR},
	{ "iext",     IEXT_SYM},
	// { "if",	IF},
	{ "immediate", IMMEDIATE},
	{ "in",	IN},		// or SQL_IN
	{ "index",	INDEX},
	{ "initialize", INITIALIZE},
	{ "insert",	SQL_INSERT},
	{ "int",	INT_SYM},
	{ "integer",	INT_SYM},
	{ "internal", INTERNAL},
	{ "intersect",INTERSECT},    
	{ "into",	INTO},
	{ "inwindow", INWINDOW},
	{ "is",	IS},
	{ "iterate",	ITERATE},
	{ "let",	LET},
	{ "like",	LIKE},
	{ "list",	LIST},
	{ "load",	SQL_LOAD},
	{ "local",	LOCAL},
	{ "memory",	MEMORY},
	{ "minute",	MINUTE},
	{ "modeltype",MODELTYPE},
	{ "name",	NAME},
	{ "nil",	NIL},
	{ "nop",	NOP},
	{ "not",	NOT},
	{ "null",	NULLSYM},
	{ "of",	OF},
	{ "oid",	OID},
	{ "on",       ON},
	{ "or",	OR},
	{ "order",	ORDER},
	{ "ordered",	ORDERED},
	{ "over",	OVER},
	{ "parameters", PARAMETERS},
	{ "partables",PARTABLES},
	{ "partition",PARTITION},
	{ "port",     PORT},
	{ "preceding",PRECEDING},
	{ "primary",	PRIMARY},
	{ "produce",	PRODUCE},
	{ "range",	RANGE},
	{ "real",	REAL_SYM},
	{ "ref",	REF},
	{ "refresh", REFRESH},
	{ "return",	RETURN},                
	//  { "rlike",	RLIKE},
	{ "rows",	ROWS},
	{ "rtree",	RTREE},
	{ "run",      RUN},
	{ "second",   SECOND},
	{ "select",	SQL_SELECT},
	{ "set",	SET},
	{ "sharedtables", SHAREDTABLES},
	{ "slide",	SLIDE},
	//  { "slike",	SLIKE},
	{ "sorted",	SORTED},
	{ "source",   SOURCE},
	//  { "state",	STATE},
	{ "stdout",	STDOUT},
	{ "stream",	STREAM},
	{ "system",	SYSTEM},
	{ "table",	TABLE},
	{ "target",   TARGET},
	{ "terminate",TERMINATE},
	{ "then",	THEN},
	{ "timestamp",	TIMESTAMP_SYM},
	{ "to",	TO},
	{ "true",     TRUE},
	{ "type",	TYPE},
	{ "uda",      UDA},
	{ "union",	UNION},
	{ "unique",	UNIQUE},
	{ "unlimited", UNLIMITED},
	{ "update",	SQL_UPDATE},
	{ "using",    USING},
	{ "values",	VALUES},
	{ "var",	VAR},
	{ "when",	WHEN},
	{ "where",	WHERE},
	{ "window",	WINDOW},
	{ "with",	WITH},
};

char *copyToken(char *start, char *end);

static int yytoklen;
	static int
sym_compare(const void *str, const void *sym) 
{
	char buf[100];
	memcpy(buf, str, yytoklen);
	buf[yytoklen]='\0';
	return strcasecmp(buf, ((symtab_t*)sym)->name);
}

#if 0
int getUdfReturnArity(char *fname, int size)
{
	int pos;
	char name[100];
	nt_obj_t *def;

	memcpy(name, fname, size);
	name[size]='\0';
	if ( (def = getUdfDef(name, &pos)) )
		return ListObj(UdfDefObj(def)->return_type_list)->length;
	else if ( (def = getUdaDef(name, &pos)) )
		return ListObj(UdaDefObj(def)->return_type_list)->length;
	else
		return 0;
}
#endif

int findKeyword(char *tok, char *lookahead)
{
	SMLOG3(20, "Entering lex.cc::findKeyword \ntok: %s \n\n", tok);
	
	symtab_t * sym = (symtab_t*)0 ;
	char *s = lookahead;
	int ret_arity;
	int token = 0;

	yytoklen = lookahead - tok;

	while (*s && !isgraph(*s)) s++; 

	if (*s == '(') {
		/* a builtin function? */
		if ((sym = (symtab_t*)bsearch(tok, funcsym, 
						sizeof(funcsym)/sizeof(symtab_t),
						sizeof(symtab_t), sym_compare))) 
		{
			//yylval.ival = sym->tok;
			//	return BUILTINFUNC;
			SMLOG3(20, "\ttok no. (builtin func): %i", sym->tok);
			return sym->tok;
		}
		/* a builtin aggregate ? */
		if ((sym = (symtab_t*)bsearch(tok, aggrsym, 
						sizeof(aggrsym)/sizeof(symtab_t),
						sizeof(symtab_t), sym_compare))) 
		{
			SMLOG3(20, "\ttok no. (builtin aggre): %i", sym->tok);
			return sym->tok;
		}
#if 0
		/* a user-defined function/aggregate ? */
		ret_arity = getUdfReturnArity(tok, lookahead-tok);
		if (ret_arity>0) {
			yytext = copyToken(tok, lookahead);
			yylval.sval = yytext;
			return ret_arity==1 ? SCALAR_FUNC:COMPLEX_FUNC;
		}
#endif
	}

#if 0
	/* a nt command? */
	sym = (symtab_t*)bsearch(tok, ntsym, sizeof(ntsym)/sizeof(symtab_t), 
			sizeof(symtab_t), sym_compare);

#endif

	/* a SQL key word? */
	sym = (symtab_t*)bsearch(tok, sqlsym, sizeof(sqlsym)/sizeof(symtab_t), 
			sizeof(symtab_t), sym_compare);
	if (sym) {
		token = sym->tok;
		if (token == IN && in_sql==1)
			token = SQL_IN;
	}
	SMLOG3(20, "\ttok no.: %i", token);
	return token;
}

char *copyToken(char *start, char *end)
{
	int len = end - start;
	char *token = (char*)malloc(len+1);

	memcpy(token, start, len);
	token[len]='\0';
	return token;
}
char *readTextLiteral(char *tok)
{
	register char c;
	int bail;

	bail = 0;
	while(!bail)    {
		c = yyGet();
		switch(c)
		{
			case 0:
				return(NULL);

			case '\\':
				c = yyGet();
				if (!c)
					return(NULL);
				break;

			case '\'':
				bail=1;
				break;
		}
	}
	return(copyToken(tok+1, tokPtr-1));
}


char *readCCode(char *tok)
{
	register char c;
	int bail;

	bail = 0;
	while(!bail)    {
		c = yyGet();
		switch(c)
		{
			case 0:
				return(NULL);	
			case '@':
				bail=1;
				break;
			case '\n':
				yylineno++;
				lineStartPtr = tokPtr;
				break;  
		}
	}
	return(copyToken(tok+1, tokPtr-1));
}


/************************************************************
  Initialize Scanner
 ************************************************************/
void sqlInitScanner(char *buf)
{
	tokPtr = buf;
	next_state = START_STATE;	/* no following signed number */

	in_sql = 0;
	in_modeltype = 0;
	yylineno = 1;
	lineStartPtr = tokPtr;
	yylloc.first_line = 1;
	yylloc.first_column = 1;
}

/************************************************************
  lex
 ************************************************************/
int last_token_is_keyword;
int filename_p = 0;

int yylex2(void)
{
	SMLOG3(20, "Entering lex.cc::yylex2 state: %i", state);
	/* parser has difficulty shifting the last token returned by yylex(). */
	if (last_token_is_keyword) {
		yytext = copyToken(tokStart, tokPtr);
		yylval.sval = yytext;
		last_token_is_keyword = 0;
					//added by Hamid  7/22/2009
					//FIXME: I believe that the language must not be case sensitive
					if (in_sql)
						yytext = tolower(yytext);
		return in_sql? SQL_ID: IDENT;
	}
	return 0;
}
int yylex(void)
{
	register	char	c;
	static	char EOI[] = "*end of input*";
	//  static char *EOI = strlen(tokPtr)+tokPtr;
	int tokval;
	/*  yylval = 0;*/
	prev_state = state = next_state; 
	next_state = ALLOW_SIGNED_NUMBER;
	tokStart = tokPtr;

	last_token_is_keyword = 0;

	SMLOG3(20, "Entering lex.cc::yylex state: %i", state);

	while(1) {
		switch(state) {
			case ALLOW_SIGNED_NUMBER:
			case START_STATE:		
				c = yyGet();
				while (c && !isgraph(c)) {
					if (c == '\n') {
						yylineno++;
						lineStartPtr = tokPtr;
					}
					c = yyGet();
				}
				/* Set Start of token */
				tokStart = tokPtr - 1;	
				yylloc.first_line = yylineno;
				yylloc.first_column = tokPtr - lineStartPtr + 1;

				/* filename */
				if (filename_p) {
					while (c && (isalnum(c) || c=='.' || c=='/' || c=='-' || c=='_'))
						c=yyGet();
					yyUnget();

					yytext = copyToken(tokStart, tokPtr);
					yylval.sval = yytext;

					filename_p = 0;
					//added by Hamid  7/22/2009
					//FIXME: I believe that the language must not be case sensitive
					//if (in_sql)
					//	yytext = tolower(yytext);
					return in_sql ? SQL_ID : IDENT;
				}

				state = (lex_state_t)state_map[c];
				break;

			case IDENT_STATE:		/* keyword or ident */
				while (isalnum(c=yyGet()) || c == '_' || c == '$');
				yyUnget();

				next_state = START_STATE; /* no following signed number */

				if ((tokval = findKeyword(tokStart, tokPtr))) {
					last_token_is_keyword = 1;
				} else {
					yytext = copyToken(tokStart, tokPtr);
					yylval.sval = yytext;
					tokval = in_sql ? SQL_ID : IDENT;
					//added by Hamid  7/22/2009
					//FIXME: I believe that the language must not be case sensitive
					//if (in_sql)
					//	yytext = tolower(yytext);
				}
				return tokval;
			case NUMBER_STATE:		/* Incomplete real or int number */
				while (isdigit((c=yyGet())));
				if (c== 'e' || c== 'E') {
					if ((c=(yyGet())) == '+' || c == '-')
					{			
						if (isdigit(yyPeek()))	
						{
							while (isdigit(yyGet())) ;
							yyUnget();
							yytext = copyToken(tokStart, tokPtr);
							yylval.rval = atof(yytext);
							next_state = START_STATE;
							return in_sql? SQL_REAL_NUM : REAL_NUM;
						}
					}
					yyUnget();
				} else if (c == 'x' && tokPtr - tokStart == 2 && *tokStart == '0') {
					/* hex */
				}
				state = INT_OR_REAL_STATE;
				break;

			case SIGNED_NUMBER_STATE:
				//        if (c== '-') {
				//  	if (yyGet()=='>')
				//  	  return POINTER;
				//  	else
				//  	  yyUnget();
				//        }
				if (c=='+' || c== '-') {
					/* for ++, --, +=, -=, and -> */
					if ((tokval = findKeyword(tokStart, tokStart+2))) {
						yyGet();
						return tokval;
					}
				}
				if (prev_state != ALLOW_SIGNED_NUMBER) {
					state = CHAR_STATE;
					break;
				}
				if (!isdigit(c=yyGet())) {
					if (c != '.') {
						state = CHAR_STATE; /* Return sign as single char */
						break;
					}
					yyUnget();			
				}
				while (isdigit(c=yyGet())) ; /* Incomplete real or int number */

			case INT_OR_REAL_STATE:	/* Complete integer/real number */
				if (c!='.') {
					yyUnget();
					yytext = copyToken(tokStart, tokPtr);
					yylval.ival = atoi(yytext);
					next_state = START_STATE;
					return in_sql ? SQL_NUM : NUM;
				}
			case REAL_STATE:		/* Incomplete real number */
				while (isdigit(c=yyGet()));

				if(c == 'e' || c == 'E') {
					c = yyGet();
					if (c != '-' && c != '+') {
						state = CHAR_STATE;
						break;
					}
					if (!isdigit(yyGet())) {
						state = CHAR_STATE;
						break;
					}
					while (isdigit(yyGet()));
				}
				yyUnget();
				yytext = copyToken(tokStart, tokPtr);
				yylval.rval = atof(yytext);
				next_state = START_STATE;
				return in_sql ? SQL_REAL_NUM : REAL_NUM;

			case CMP_OP_STATE:	/* Incomplete comparison operator */
				c = yyGet();
				if (state_map[c] != CMP_OP_STATE)
					yyUnget();

				if ((tokval = findKeyword(tokStart, tokPtr))) {
					/* 	last_token_is_keyword = 1; */
					return tokval;
				}

				state = CHAR_STATE;
				break;

			case STRING_STATE:	/* Incomplete text string */
				yytext = readTextLiteral(tokStart);
				yylval.sval = yytext;
				if (yytext) {
					next_state = START_STATE;
					return in_sql ? SQL_STRING : STRING;
				}
				else
					state = CHAR_STATE;
				break;

			case C_CODE_STATE:
				yytext = readCCode(tokStart);
				yylval.sval = yytext;	    
				if (yytext) {
					next_state = START_STATE;
					return CCODE;
				}
				else
					state = CHAR_STATE;
				break;

			case COMMENT_STATE:	/* line Comment (starting with '#') */
				while ((c = yyGet()) != '\n' && c);
				yyUnget();
				state = START_STATE;
				break;
			case LONG_COMMENT_STATE: /* c style Comment */
				if (yyPeek() != '*')
				{
					state=CHAR_STATE;               // Probable division
					break;
				}
				yyGet();
				while (tokPtr != EOI &&
						((c=yyGet()) != '*' || yyPeek() != '/') && c)
				{
					if (c == '\n') {
						yylineno++;
						lineStartPtr = tokPtr;
					}
				}
				if (tokPtr != EOI && c) {
					yyGet();                        // remove last '/'
					state = START_STATE;              // Try again
				} else {
					return END_OF_INPUT;
				}

				break;
			case CHAR_STATE:	/* Unknown token.  Revert to single char */
				tokPtr = tokStart;
				if (c== ')') next_state = START_STATE;
				c = yyGet();
				return (int)c;
			case REAL_OR_POINT_STATE:
				if (isdigit(yyPeek()))
					state = REAL_STATE;		/* Real */
				else
				{
					state = CHAR_STATE;		/* return '.' */
				}
				break;
			case EOL_STATE:	/*  End Of Input */
				yytext = EOI;
				yylval.sval = EOI;
				next_state = END_STATE;
				return END_OF_INPUT;
			case END_STATE:
				return 0;
		}
	}
}


#ifdef STANDALONE
#undef malloc
#undef free

char *Malloc(size,file,line)
	int	size;
	char	*file;
	int	line;
{
	return((char *)malloc(size));
}


void Free(ptr,file,line)
	char	*ptr;
	char	*file;
	int	line;
{
	free(ptr);
}



main()
{
	char	*p, tmpBuf[10 * MAX_STR_LEN];

	bzero(tmpBuf,sizeof(tmpBuf));
	read(fileno(stdin),tmpBuf,sizeof(tmpBuf));
	sqlInitScanner(tmpBuf);
	while(p = (char *) yylex())
	{
		printf("token id : %d   value [%s] \n", p, 
				yylval?yylval:(char *)"(null)");
	}
}

#endif







