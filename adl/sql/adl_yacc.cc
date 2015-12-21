
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 1



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "adl_yacc.grm"

  /* {{{ c code */

#define YYDEBUG 1
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <config.h>

#include <sql/util.h>
#include <sql/const.h>
#include <sql/absyn.h>
#include <sql/list.h>
#include <sql/adl_sys.h>
#include <buffer.h>
#include <env.h>

using namespace ESL;
#include <ios/ios.h>

using namespace std;
extern system_t *ntsys;

extern char *yytext;
extern int yylineno;
extern int filename_p;

extern A_exp abs_adl;

char errMsg[1024];
extern int in_sql;
extern int in_modeltype;

int yylex2(void);
int yylex(void);

void yyerror(char *s)
{
  fprintf(stderr, "%s near '%s' at line %d\n", s, yytext ? (char*) yytext : "",
	     yylineno);
}

void mkInternalName(char* name, char*& internalName, int notify)
{
  if(strcasecmp(name, "inwindow") == 0) {
    internalName = strdup(name);
    return;
  }
  if(strcasecmp(name, "instream") == 0) {
    internalName = strdup(name);
    return;
  }
  if(strcasecmp(name, "outstream") == 0) {
    internalName = strdup(name);
    return;
  }
  if(strcmp(getUserName(), "__user__") == 0) {
    /*if(in_modeltype == 1) {
      internalName = (char*)malloc(strlen(getModelName())+strlen(name)+4);
      sprintf(internalName, "%s%s", getModelName(), name);
    }
    else {*/
      internalName = strdup(name);
    //}
    return;
  }
  char* localName = strdup(name);
  char* underScore = strchr(localName, '_');
  if(underScore != NULL && underScore[1] == '_') {
    char* first = strsep(&localName, "_");
    //first++;  //skip the next '_' too
    if(strcmp(getUserName(), first) != 0 && notify) {
      bufferMngr* bm = bufferMngr::getInstance();
      buffer* iob = bm->lookup("_ioBuffer");
      iob->put(USES_LIBRARY, first);
    }
    /*if(in_modeltype == 1) {
      internalName = (char*)malloc(strlen(first)+strlen(getModelName())+strlen(name)+4);
      sprintf(internalName, "%s__%s%s", first, getModelName(), name);
    }
    else*/ 
      internalName = strdup(name);
    return;
  }


  char* colon = strrchr(localName, '$');

  if(colon == NULL) {
    if(in_modeltype == 1) {
      internalName = strdup(name);
      return;
    }
    char* second = localName;
    internalName = (char*)malloc(strlen(getUserName())+strlen(getModelName())+strlen(second)+4);
    sprintf(internalName, "%s__%s%s", getUserName(), getModelName(), second);
  }
  else {
    char* first = strsep(&localName, "$");
    if(strcmp(getUserName(), first) != 0 && notify) {
      bufferMngr* bm = bufferMngr::getInstance();
      buffer* iob = bm->lookup("_ioBuffer");
      iob->put(USES_LIBRARY, first);
    }

    char* second = localName;
    if(in_modeltype == 1) {
      internalName = (char*)malloc(strlen(first)+strlen(second)+4);
      sprintf(internalName, "%s__%s", first, second);
      return;
    }
    internalName = (char*)malloc(strlen(first)+strlen(getModelName())+strlen(second)+4);
    sprintf(internalName, "%s__%s%s", first, getModelName(), second);
  }
}

void prependUserNameIfESL(char* name, char*& internalName) {
  if(strcmp(getUserName(), "__user__") == 0) {
    internalName = strdup(name);
    return;
  }

  internalName = (char*)malloc(512);
  sprintf(internalName, "../exe/%s__%s.so", getUserName(), name);
}

/* }}} */



/* Line 189 of yacc.c  */
#line 205 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENT = 258,
     SQL_ID = 259,
     STRING = 260,
     SQL_STRING = 261,
     CCODE = 262,
     NUM = 263,
     SQL_NUM = 264,
     DAY = 265,
     HOUR = 266,
     MINUTE = 267,
     SECOND = 268,
     TIMESTAMP_SYM = 269,
     REAL_NUM = 270,
     SQL_REAL_NUM = 271,
     END_OF_INPUT = 272,
     LIST_SYM = 273,
     QUIT_SYM = 274,
     HELP_SYM = 275,
     DB2_SYM = 276,
     COMPILE_SYM = 277,
     NIL = 278,
     OF = 279,
     IF = 280,
     THEN = 281,
     ELSE = 282,
     WHILE = 283,
     DO = 284,
     FOR = 285,
     TO = 286,
     BREAK = 287,
     LET = 288,
     IN = 289,
     END = 290,
     TYPE = 291,
     ARRAY = 292,
     VAR = 293,
     FUNCTION = 294,
     AGGREGATE = 295,
     CAGGREGATE = 296,
     INT_SYM = 297,
     CHAR_SYM = 298,
     REAL_SYM = 299,
     IEXT_SYM = 300,
     TRUE_ = 301,
     FALSE_ = 302,
     SQL_CREATE = 303,
     SQL_SELECT = 304,
     SQL_DROP = 305,
     SQL_INSERT = 306,
     SQL_DELETE = 307,
     SQL_UPDATE = 308,
     SQL_LOAD = 309,
     MODELTYPE = 310,
     FLOW = 311,
     UDA = 312,
     SHAREDTABLES = 313,
     PARAMETERS = 314,
     PARTABLES = 315,
     WITH = 316,
     AS = 317,
     UNION = 318,
     INTERSECT = 319,
     EXCEPT = 320,
     ALL = 321,
     DISTINCT = 322,
     REFRESH = 323,
     IMMEDIATE = 324,
     DEFFERRED = 325,
     FROM = 326,
     WHERE = 327,
     GROUP = 328,
     PARTITION = 329,
     OVER = 330,
     ROWS = 331,
     RANGE = 332,
     SLIDE = 333,
     HEARTBEAT = 334,
     PRECEDING = 335,
     BY = 336,
     UNLIMITED = 337,
     HAVING = 338,
     ORDER = 339,
     UNIQUE = 340,
     RUN = 341,
     USING = 342,
     ON = 343,
     ORDERED = 344,
     SORTED = 345,
     LOCAL = 346,
     SOURCE = 347,
     MEMORY = 348,
     BTREE = 349,
     HASH = 350,
     RTREE = 351,
     TARGET = 352,
     OID = 353,
     IS = 354,
     SYSTEM = 355,
     DEFINED = 356,
     REF = 357,
     VALUES = 358,
     EXISTS = 359,
     CASE = 360,
     WHEN = 361,
     TABLE = 362,
     STREAM = 363,
     INDEX = 364,
     CLIKE = 365,
     RLIKE = 366,
     SLIKE = 367,
     INTO = 368,
     DYNAMIC = 369,
     ASC = 370,
     DESC = 371,
     SET = 372,
     LIST = 373,
     BAG = 374,
     COMPLEX_FUNC = 375,
     SCALAR_FUNC = 376,
     BUILTINFUNC = 377,
     PORT = 378,
     NULLSYM = 379,
     EXTERNAL = 380,
     NAME = 381,
     INWINDOW = 382,
     WINDOW = 383,
     RETURN = 384,
     STDOUT = 385,
     STATE = 386,
     INITIALIZE = 387,
     ITERATE = 388,
     EXPIRE = 389,
     PRODUCE = 390,
     TERMINATE = 391,
     NOP = 392,
     PRIMARY = 393,
     KEY = 394,
     AGGR_AVG = 395,
     AGGR_SUM = 396,
     AGGR_MIN = 397,
     AGGR_MAX = 398,
     AGGR_COUNT = 399,
     AGGR_XA = 400,
     BF_ABS = 401,
     BF_CASE = 402,
     BF_LENGTH = 403,
     BF_LTRIM = 404,
     BF_MAX = 405,
     BF_MIN = 406,
     BF_RTRIM = 407,
     BF_SQRT = 408,
     BF_SUBSTR = 409,
     BF_TRIM = 410,
     XML_XE = 411,
     XML_ATTR = 412,
     BF_OLDEST = 413,
     INTERNAL = 414,
     OR = 415,
     AND_ = 416,
     BETWEEN = 417,
     SQL_IN = 418,
     LIKE = 419,
     NE = 420,
     LE = 421,
     GE = 422,
     NEG = 423,
     NOT = 424,
     POINTER = 425,
     MINUSEQL = 426,
     PLUSEQL = 427,
     PLUSPLUS = 428,
     MINUSMINUS = 429
   };
#endif
/* Tokens.  */
#define IDENT 258
#define SQL_ID 259
#define STRING 260
#define SQL_STRING 261
#define CCODE 262
#define NUM 263
#define SQL_NUM 264
#define DAY 265
#define HOUR 266
#define MINUTE 267
#define SECOND 268
#define TIMESTAMP_SYM 269
#define REAL_NUM 270
#define SQL_REAL_NUM 271
#define END_OF_INPUT 272
#define LIST_SYM 273
#define QUIT_SYM 274
#define HELP_SYM 275
#define DB2_SYM 276
#define COMPILE_SYM 277
#define NIL 278
#define OF 279
#define IF 280
#define THEN 281
#define ELSE 282
#define WHILE 283
#define DO 284
#define FOR 285
#define TO 286
#define BREAK 287
#define LET 288
#define IN 289
#define END 290
#define TYPE 291
#define ARRAY 292
#define VAR 293
#define FUNCTION 294
#define AGGREGATE 295
#define CAGGREGATE 296
#define INT_SYM 297
#define CHAR_SYM 298
#define REAL_SYM 299
#define IEXT_SYM 300
#define TRUE_ 301
#define FALSE_ 302
#define SQL_CREATE 303
#define SQL_SELECT 304
#define SQL_DROP 305
#define SQL_INSERT 306
#define SQL_DELETE 307
#define SQL_UPDATE 308
#define SQL_LOAD 309
#define MODELTYPE 310
#define FLOW 311
#define UDA 312
#define SHAREDTABLES 313
#define PARAMETERS 314
#define PARTABLES 315
#define WITH 316
#define AS 317
#define UNION 318
#define INTERSECT 319
#define EXCEPT 320
#define ALL 321
#define DISTINCT 322
#define REFRESH 323
#define IMMEDIATE 324
#define DEFFERRED 325
#define FROM 326
#define WHERE 327
#define GROUP 328
#define PARTITION 329
#define OVER 330
#define ROWS 331
#define RANGE 332
#define SLIDE 333
#define HEARTBEAT 334
#define PRECEDING 335
#define BY 336
#define UNLIMITED 337
#define HAVING 338
#define ORDER 339
#define UNIQUE 340
#define RUN 341
#define USING 342
#define ON 343
#define ORDERED 344
#define SORTED 345
#define LOCAL 346
#define SOURCE 347
#define MEMORY 348
#define BTREE 349
#define HASH 350
#define RTREE 351
#define TARGET 352
#define OID 353
#define IS 354
#define SYSTEM 355
#define DEFINED 356
#define REF 357
#define VALUES 358
#define EXISTS 359
#define CASE 360
#define WHEN 361
#define TABLE 362
#define STREAM 363
#define INDEX 364
#define CLIKE 365
#define RLIKE 366
#define SLIKE 367
#define INTO 368
#define DYNAMIC 369
#define ASC 370
#define DESC 371
#define SET 372
#define LIST 373
#define BAG 374
#define COMPLEX_FUNC 375
#define SCALAR_FUNC 376
#define BUILTINFUNC 377
#define PORT 378
#define NULLSYM 379
#define EXTERNAL 380
#define NAME 381
#define INWINDOW 382
#define WINDOW 383
#define RETURN 384
#define STDOUT 385
#define STATE 386
#define INITIALIZE 387
#define ITERATE 388
#define EXPIRE 389
#define PRODUCE 390
#define TERMINATE 391
#define NOP 392
#define PRIMARY 393
#define KEY 394
#define AGGR_AVG 395
#define AGGR_SUM 396
#define AGGR_MIN 397
#define AGGR_MAX 398
#define AGGR_COUNT 399
#define AGGR_XA 400
#define BF_ABS 401
#define BF_CASE 402
#define BF_LENGTH 403
#define BF_LTRIM 404
#define BF_MAX 405
#define BF_MIN 406
#define BF_RTRIM 407
#define BF_SQRT 408
#define BF_SUBSTR 409
#define BF_TRIM 410
#define XML_XE 411
#define XML_ATTR 412
#define BF_OLDEST 413
#define INTERNAL 414
#define OR 415
#define AND_ 416
#define BETWEEN 417
#define SQL_IN 418
#define LIKE 419
#define NE 420
#define LE 421
#define GE 422
#define NEG 423
#define NOT 424
#define POINTER 425
#define MINUSEQL 426
#define PLUSEQL 427
#define PLUSPLUS 428
#define MINUSMINUS 429




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 132 "adl_yacc.grm"

  int pos;
  int ival;
  double rval;
  char *sval;

  A_list list;
  A_ref ref;
  A_exp exp;
  A_dec dec;
  A_ty ty;
  A_field field;
  A_namety namety;
  A_range range;
  A_slide slide;
  A_win win;
//    A_fundec fundec;
//    A_aggrdec aggrdec;
  A_selectitem selectitem;
  A_qun qun;
  A_tablecolumn tablecolumn;
  S_symbol symval;
  A_index index;
  A_modelitem modelitem;
  A_flow flow;



/* Line 214 of yacc.c  */
#line 618 "y.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 643 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
	     && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  38
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1417

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  193
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  131
/* YYNRULES -- Number of rules.  */
#define YYNRULES  334
/* YYNRULES -- Number of states.  */
#define YYNSTATES  777

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   429

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,   179,   174,     2,
     189,   190,   177,   176,   161,   175,   192,   178,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   191,   160,
     167,   165,   166,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   187,   173,   188,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   162,   163,   164,   168,   169,
     170,   171,   172,   180,   181,   182,   183,   184,   185,   186
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,     9,    13,    14,    16,    18,    22,
      23,    25,    27,    29,    31,    33,    35,    37,    41,    42,
      44,    46,    48,    50,    54,    55,    57,    59,    64,    68,
      72,    74,    77,    83,    89,    91,    95,   101,   110,   116,
     121,   129,   135,   142,   145,   148,   149,   152,   154,   158,
     159,   164,   169,   174,   183,   192,   193,   196,   197,   201,
     205,   211,   212,   214,   217,   219,   225,   226,   230,   233,
     235,   236,   239,   251,   261,   264,   265,   273,   282,   286,
     288,   293,   294,   296,   300,   304,   310,   319,   321,   326,
     331,   336,   343,   348,   353,   358,   363,   368,   373,   381,
     390,   397,   398,   404,   406,   410,   414,   420,   421,   424,
     426,   430,   432,   434,   436,   438,   441,   444,   446,   449,
     454,   459,   461,   465,   469,   473,   477,   481,   484,   488,
     491,   495,   499,   503,   507,   511,   515,   519,   523,   527,
     531,   536,   540,   545,   549,   554,   557,   561,   565,   569,
     574,   576,   583,   589,   594,   598,   599,   601,   603,   607,
     612,   618,   620,   624,   631,   632,   633,   635,   639,   641,
     645,   649,   651,   653,   655,   661,   680,   685,   686,   697,
     702,   719,   733,   745,   764,   780,   781,   787,   790,   791,
     797,   798,   804,   805,   811,   812,   816,   820,   823,   824,
     828,   831,   832,   836,   838,   840,   842,   844,   846,   848,
     850,   852,   854,   856,   859,   860,   861,   865,   870,   872,
     873,   877,   882,   885,   886,   888,   890,   894,   896,   905,
     906,   907,   913,   914,   922,   923,   932,   933,   943,   944,
     949,   951,   953,   957,   961,   963,   965,   967,   968,   970,
     971,   973,   975,   979,   981,   989,   995,   998,  1001,  1005,
    1007,  1011,  1013,  1014,  1016,  1019,  1023,  1025,  1027,  1030,
    1031,  1035,  1037,  1039,  1043,  1047,  1052,  1056,  1059,  1063,
    1068,  1069,  1071,  1073,  1075,  1077,  1079,  1081,  1083,  1085,
    1088,  1095,  1106,  1115,  1120,  1121,  1124,  1125,  1128,  1129,
    1133,  1134,  1137,  1138,  1146,  1148,  1150,  1154,  1158,  1159,
    1162,  1163,  1164,  1174,  1183,  1186,  1189,  1190,  1195,  1196,
    1201,  1203,  1205,  1207,  1208,  1215,  1216,  1223,  1224,  1231,
    1233,  1237,  1241,  1245,  1246
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     194,     0,    -1,   197,   195,    17,    -1,   196,    -1,   195,
     160,   196,    -1,    -1,   199,    -1,   198,    -1,   197,   160,
     198,    -1,    -1,   203,    -1,   222,    -1,   220,    -1,   249,
      -1,   247,    -1,   258,    -1,   201,    -1,   200,   160,   201,
      -1,    -1,   204,    -1,   208,    -1,   249,    -1,   203,    -1,
     202,   160,   203,    -1,    -1,   204,    -1,   208,    -1,    36,
       3,   165,   205,    -1,   187,   207,   188,    -1,    37,    24,
       3,    -1,     3,    -1,     3,     3,    -1,     3,   102,   189,
       3,   190,    -1,     3,     3,   189,     8,   190,    -1,   206,
      -1,   207,   161,   206,    -1,    38,     3,   191,   165,   233,
      -1,    38,     3,   191,     3,   191,   165,   233,   160,    -1,
     128,     3,   189,   207,   190,    -1,   210,    62,   114,     3,
      -1,   210,   189,   207,   190,   213,   216,   211,    -1,   209,
     189,   207,   190,   219,    -1,   209,   189,   207,   190,   215,
     217,    -1,   108,     3,    -1,   107,     3,    -1,    -1,    62,
     262,    -1,     3,    -1,   212,   161,     3,    -1,    -1,    96,
     189,   212,   190,    -1,    94,   189,   212,   190,    -1,    95,
     189,   212,   190,    -1,    94,   189,     3,   189,   212,   190,
     214,   190,    -1,    95,   189,     3,   189,   212,   190,   214,
     190,    -1,    -1,   161,     8,    -1,    -1,    84,    81,     3,
      -1,    84,    81,   159,    -1,    84,    81,   159,    62,     3,
      -1,    -1,    91,    -1,    92,     5,    -1,    93,    -1,    93,
      98,    99,   100,   101,    -1,    -1,    92,   107,     3,    -1,
      92,   218,    -1,     5,    -1,    -1,    97,     5,    -1,    39,
       3,   189,   225,   190,   191,   238,   187,   202,   195,   188,
      -1,    39,     3,   189,   225,   190,   187,   202,   195,   188,
      -1,    34,     5,    -1,    -1,   125,   224,     3,   189,   225,
     190,   221,    -1,   125,    43,   223,     3,   189,   225,   190,
     221,    -1,   189,     8,   190,    -1,     3,    -1,   107,   189,
     225,   190,    -1,    -1,   207,    -1,   226,   182,     4,    -1,
       4,   182,     4,    -1,     4,   192,     4,   182,     4,    -1,
     228,    75,   189,   287,   263,   288,   291,   190,    -1,   229,
      -1,   141,   189,   233,   190,    -1,    38,   189,   233,   190,
      -1,     4,   189,   235,   190,    -1,     4,   189,   235,   190,
     182,     4,    -1,   143,   189,   233,   190,    -1,   142,   189,
     233,   190,    -1,   144,   189,   177,   190,    -1,   144,   189,
     233,   190,    -1,   140,   189,   233,   190,    -1,   145,   189,
     233,   190,    -1,   156,   189,   126,   233,   161,   236,   190,
      -1,   156,   189,   126,   233,   230,   161,   236,   190,    -1,
     156,   189,   126,   233,   230,   190,    -1,    -1,   161,   157,
     189,   231,   190,    -1,   233,    -1,   231,   161,   233,    -1,
     233,    62,     6,    -1,   231,   161,   233,    62,     6,    -1,
      -1,   192,     4,    -1,     4,    -1,     4,   192,     4,    -1,
     226,    -1,   124,    -1,     9,    -1,    16,    -1,     9,   292,
      -1,    16,   292,    -1,     6,    -1,    14,     6,    -1,   158,
     189,   190,   232,    -1,   153,   189,   233,   190,    -1,   234,
      -1,   233,   176,   233,    -1,   233,   175,   233,    -1,   233,
     177,   233,    -1,   233,   178,   233,    -1,   233,   179,   233,
      -1,   233,   186,    -1,   233,   183,   233,    -1,   233,   185,
      -1,   233,   184,   233,    -1,   233,   165,   233,    -1,   233,
     170,   233,    -1,   233,   167,   233,    -1,   233,   171,   233,
      -1,   233,   166,   233,    -1,   233,   172,   233,    -1,   233,
     163,   233,    -1,   233,   162,   233,    -1,   233,   169,   233,
      -1,   233,   181,   169,   233,    -1,   233,   168,   233,    -1,
     233,   181,   168,   233,    -1,   233,    99,   124,    -1,   233,
      99,   181,   124,    -1,   104,   233,    -1,   181,   104,   233,
      -1,   189,   233,   190,    -1,   189,   262,   190,    -1,   187,
     202,   195,   188,    -1,   228,    -1,   105,   233,   237,    27,
     233,    35,    -1,   105,   237,    27,   233,    35,    -1,   105,
     233,   237,    35,    -1,   105,   237,    35,    -1,    -1,   236,
      -1,   233,    -1,   236,   161,   233,    -1,   106,   233,    26,
     233,    -1,   237,   106,   233,    26,   233,    -1,     3,    -1,
     189,   207,   190,    -1,   127,   189,   207,   190,   213,   160,
      -1,    -1,    -1,   244,    -1,   240,   161,   244,    -1,   245,
      -1,   241,   161,   245,    -1,   242,   161,     3,    -1,     3,
      -1,    46,    -1,    47,    -1,    56,     3,   189,   195,   190,
      -1,     3,   189,    57,     3,   161,   128,   243,   161,    60,
     189,   242,   190,   161,    59,   189,   225,   190,   190,    -1,
      58,   189,   242,   190,    -1,    -1,    55,     3,   187,   246,
     161,   241,   161,   248,   240,   188,    -1,    55,     3,    62,
       3,    -1,   128,    40,     3,   189,   225,   190,   191,   238,
     187,   239,   200,   255,   256,   254,   257,   188,    -1,    40,
       3,   189,   225,   190,   191,   238,   187,   200,   255,   256,
     257,   188,    -1,    40,     3,   189,   225,   190,   191,   238,
     187,   200,   195,   188,    -1,   128,    41,     3,   189,   225,
     190,   191,   238,   187,     7,   239,   200,     7,   250,   251,
     252,   253,   188,    -1,    41,     3,   189,   225,   190,   191,
     238,   187,     7,   200,     7,   250,   251,   253,   188,    -1,
      -1,   132,   191,   187,     7,   188,    -1,   132,   191,    -1,
      -1,   133,   191,   187,     7,   188,    -1,    -1,   134,   191,
     187,     7,   188,    -1,    -1,   136,   191,   187,     7,   188,
      -1,    -1,   134,   191,   233,    -1,   132,   191,   233,    -1,
     132,   191,    -1,    -1,   133,   191,   233,    -1,   133,   191,
      -1,    -1,   136,   191,   233,    -1,   300,    -1,   259,    -1,
     310,    -1,   314,    -1,   318,    -1,   322,    -1,   316,    -1,
     308,    -1,   306,    -1,   271,    -1,   260,   262,    -1,    -1,
      -1,    61,   261,   266,    -1,   262,   278,   279,   273,    -1,
     273,    -1,    -1,    84,    81,   264,    -1,   264,   161,   233,
     265,    -1,   233,   265,    -1,    -1,   115,    -1,   116,    -1,
     266,   161,   267,    -1,   267,    -1,     4,   189,   207,   190,
      62,   189,   262,   190,    -1,    -1,    -1,    87,   269,   189,
     320,   190,    -1,    -1,    75,   189,   287,   263,   288,   291,
     190,    -1,    -1,    86,   272,     4,   232,    88,     4,   270,
     268,    -1,    -1,    49,   274,   280,   281,   296,   297,   298,
     299,   263,    -1,    -1,   103,   275,   276,   297,    -1,   271,
      -1,   277,    -1,   276,   161,   277,    -1,   189,   281,   190,
      -1,    63,    -1,    64,    -1,    65,    -1,    -1,    66,    -1,
      -1,    66,    -1,    67,    -1,   281,   161,   282,    -1,   282,
      -1,   158,   189,     4,   192,     4,   190,   284,    -1,   233,
      62,   189,   283,   190,    -1,   227,   284,    -1,   233,   284,
      -1,     4,   192,   177,    -1,   177,    -1,   283,   161,     4,
      -1,     4,    -1,    -1,     4,    -1,    62,     4,    -1,   285,
     161,   295,    -1,   295,    -1,     4,    -1,    62,     4,    -1,
      -1,    74,    81,   236,    -1,   289,    -1,   290,    -1,    76,
       9,    80,    -1,    77,     9,    80,    -1,    77,     9,   292,
      80,    -1,    77,    82,    80,    -1,    78,     9,    -1,    78,
       9,   292,    -1,    78,     9,   292,    79,    -1,    -1,    13,
      -1,    12,    -1,    11,    -1,    10,    -1,   127,    -1,     4,
      -1,   293,    -1,   294,    -1,   293,   286,    -1,     4,    75,
     189,   289,     4,   190,    -1,   107,   189,     4,    75,   189,
     289,     4,   190,   190,   286,    -1,   107,   189,     4,   189,
     235,   190,   190,   286,    -1,   189,   262,   190,   286,    -1,
      -1,    71,   285,    -1,    -1,    72,   233,    -1,    -1,    73,
      81,   236,    -1,    -1,    83,   233,    -1,    -1,    48,   301,
     107,     4,   189,   302,   190,    -1,   109,    -1,   303,    -1,
     302,   161,   303,    -1,   206,   304,   305,    -1,    -1,   181,
     124,    -1,    -1,    -1,    48,   108,     3,    62,   189,   262,
     190,   307,   215,    -1,    48,   107,     3,    62,   189,   273,
     190,   309,    -1,    68,    69,    -1,    68,    70,    -1,    -1,
      50,   311,    39,     4,    -1,    -1,    50,   312,   107,     4,
      -1,   294,    -1,   130,    -1,   129,    -1,    -1,    51,   315,
     113,   313,   262,   263,    -1,    -1,    54,    71,     5,   113,
     317,     4,    -1,    -1,    53,   319,   294,   117,   320,   297,
      -1,   321,    -1,   320,   161,   321,    -1,     4,   165,   233,
      -1,   226,   165,   233,    -1,    -1,    52,   323,    71,   294,
     297,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   257,   257,   264,   270,   280,   283,   289,   296,   307,
     310,   314,   318,   322,   326,   332,   339,   346,   356,   359,
     367,   371,   377,   384,   394,   397,   405,   454,   460,   464,
     468,   474,   478,   485,   491,   496,   503,   507,   511,   536,
     554,   591,   614,   697,   712,   734,   737,   744,   749,   785,
     788,   793,   798,   803,   820,   841,   844,   851,   854,   858,
     862,   871,   874,   878,   889,   893,   900,   903,   916,   923,
     941,   944,   949,   955,   962,   970,   975,   992,  1027,  1032,
    1036,  1042,  1045,  1059,  1063,  1069,  1079,  1113,  1118,  1128,
    1138,  1203,  1218,  1228,  1238,  1248,  1258,  1284,  1294,  1301,
    1309,  1320,  1321,  1331,  1339,  1347,  1355,  1366,  1369,  1377,
    1381,  1390,  1394,  1398,  1402,  1406,  1410,  1414,  1418,  1422,
    1445,  1452,  1456,  1460,  1464,  1468,  1472,  1476,  1480,  1484,
    1488,  1492,  1496,  1500,  1504,  1508,  1512,  1516,  1520,  1524,
    1528,  1532,  1536,  1540,  1544,  1548,  1552,  1556,  1560,  1564,
    1568,  1575,  1579,  1583,  1587,  1594,  1597,  1603,  1608,  1615,
    1621,  1642,  1651,  1657,  1678,  1685,  1688,  1696,  1704,  1712,
    1720,  1725,  1734,  1738,  1745,  1751,  1770,  1788,  1786,  1802,
    1821,  1854,  1881,  1904,  1940,  1974,  1977,  1981,  1988,  1991,
    1999,  2002,  2010,  2013,  2020,  2023,  2034,  2038,  2045,  2048,
    2052,  2059,  2062,  2069,  2074,  2079,  2084,  2089,  2094,  2099,
    2104,  2109,  2114,  2127,  2151,  2152,  2152,  2157,  2171,  2179,
    2180,  2186,  2192,  2201,  2202,  2203,  2207,  2212,  2220,  2234,
    2237,  2237,  2244,  2247,  2253,  2253,  2279,  2279,  2296,  2296,
    2305,  2311,  2315,  2337,  2350,  2352,  2354,  2360,  2361,  2367,
    2368,  2370,  2375,  2380,  2388,  2407,  2411,  2418,  2425,  2429,
    2436,  2441,  2449,  2450,  2451,  2455,  2460,  2467,  2476,  2506,
    2507,  2511,  2515,  2521,  2525,  2529,  2535,  2541,  2545,  2549,
    2554,  2560,  2562,  2564,  2566,  2572,  2576,  2588,  2597,  2601,
    2605,  2624,  2648,  2667,  2681,  2682,  2689,  2690,  2697,  2698,
    2703,  2704,  2716,  2716,  2720,  2725,  2730,  2737,  2744,  2745,
    2750,  2762,  2762,  2783,  2798,  2802,  2815,  2815,  2818,  2818,
    2830,  2834,  2839,  2847,  2847,  2873,  2873,  2898,  2898,  2918,
    2923,  2929,  2935,  2947,  2947
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENT", "SQL_ID", "STRING",
  "SQL_STRING", "CCODE", "NUM", "SQL_NUM", "DAY", "HOUR", "MINUTE",
  "SECOND", "TIMESTAMP_SYM", "REAL_NUM", "SQL_REAL_NUM", "END_OF_INPUT",
  "LIST_SYM", "QUIT_SYM", "HELP_SYM", "DB2_SYM", "COMPILE_SYM", "NIL",
  "OF", "IF", "THEN", "ELSE", "WHILE", "DO", "FOR", "TO", "BREAK", "LET",
  "IN", "END", "TYPE", "ARRAY", "VAR", "FUNCTION", "AGGREGATE",
  "CAGGREGATE", "INT_SYM", "CHAR_SYM", "REAL_SYM", "IEXT_SYM", "TRUE_",
  "FALSE_", "SQL_CREATE", "SQL_SELECT", "SQL_DROP", "SQL_INSERT",
  "SQL_DELETE", "SQL_UPDATE", "SQL_LOAD", "MODELTYPE", "FLOW", "UDA",
  "SHAREDTABLES", "PARAMETERS", "PARTABLES", "WITH", "AS", "UNION",
  "INTERSECT", "EXCEPT", "ALL", "DISTINCT", "REFRESH", "IMMEDIATE",
  "DEFFERRED", "FROM", "WHERE", "GROUP", "PARTITION", "OVER", "ROWS",
  "RANGE", "SLIDE", "HEARTBEAT", "PRECEDING", "BY", "UNLIMITED", "HAVING",
  "ORDER", "UNIQUE", "RUN", "USING", "ON", "ORDERED", "SORTED", "LOCAL",
  "SOURCE", "MEMORY", "BTREE", "HASH", "RTREE", "TARGET", "OID", "IS",
  "SYSTEM", "DEFINED", "REF", "VALUES", "EXISTS", "CASE", "WHEN", "TABLE",
  "STREAM", "INDEX", "CLIKE", "RLIKE", "SLIKE", "INTO", "DYNAMIC", "ASC",
  "DESC", "SET", "LIST", "BAG", "COMPLEX_FUNC", "SCALAR_FUNC",
  "BUILTINFUNC", "PORT", "NULLSYM", "EXTERNAL", "NAME", "INWINDOW",
  "WINDOW", "RETURN", "STDOUT", "STATE", "INITIALIZE", "ITERATE", "EXPIRE",
  "PRODUCE", "TERMINATE", "NOP", "PRIMARY", "KEY", "AGGR_AVG", "AGGR_SUM",
  "AGGR_MIN", "AGGR_MAX", "AGGR_COUNT", "AGGR_XA", "BF_ABS", "BF_CASE",
  "BF_LENGTH", "BF_LTRIM", "BF_MAX", "BF_MIN", "BF_RTRIM", "BF_SQRT",
  "BF_SUBSTR", "BF_TRIM", "XML_XE", "XML_ATTR", "BF_OLDEST", "INTERNAL",
  "';'", "','", "OR", "AND_", "BETWEEN", "'='", "'>'", "'<'", "SQL_IN",
  "LIKE", "NE", "LE", "GE", "'|'", "'&'", "'-'", "'+'", "'*'", "'/'",
  "'%'", "NEG", "NOT", "POINTER", "MINUSEQL", "PLUSEQL", "PLUSPLUS",
  "MINUSMINUS", "'{'", "'}'", "'('", "')'", "':'", "'.'", "$accept",
  "program", "statements", "statement", "fdecs", "fdec", "sexp", "adecs",
  "adec", "decs", "dec", "tydec", "ty", "tyfield", "tyfields", "vardec",
  "stream_dec_id", "table_dec_id", "tabinit_opt", "keylist",
  "table_index_opt", "opt_num", "stream_timekey_opt", "table_scope_opt",
  "stream_source_opt", "stream_source", "stream_target_opt", "fundec",
  "opt_in_string", "externdec", "opt_arr_index", "extern_ty",
  "option_tyfields", "ref", "func_exp_window", "func_exp", "xml_func_exp",
  "opt_xml_attr", "xml_attr_list", "opt_sqlid", "sql_exp", "case_exp",
  "sql_explist_opt", "sql_explist", "when_exp_list", "adlfun_ret_ty",
  "window_opt", "flows", "modelitems", "tabledcls", "truefalse", "flow",
  "modelitem", "sharedtables", "modeldec", "$@1", "aggrdec",
  "c_initialize_opt", "c_iterate_opt", "c_expire_opt", "c_terminate_opt",
  "expire_opt", "initialize_opt", "iterate_opt", "terminate_opt", "sql",
  "retrieve", "opt_with_clause", "$@2", "query", "opt_order_clause",
  "order_list", "order_dir", "with_list", "with_item", "opt_params", "$@3",
  "opt_window", "run_task", "$@4", "query_block", "$@5", "$@6",
  "values_list", "values", "query_set_op", "set_op_distinct",
  "sel_op_distinct", "select_list", "select_item", "ident_list",
  "opt_item_alias", "qun_list", "join_alias", "opt_partition_clause",
  "func_range", "range", "unlimited_range", "opt_slide", "unit",
  "simple_qun", "std_qun", "qun", "opt_from_clause", "opt_where_clause",
  "opt_group_by_clause", "opt_having_clause", "create", "$@7",
  "table_column_list", "table_column", "opt_default", "opt_key_type",
  "create_stream", "$@8", "create_view", "view_mode", "drop", "$@9",
  "$@10", "insert_target", "insert", "$@11", "load", "$@12", "update",
  "$@13", "setlist", "setitem", "delete", "$@14", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
      59,    44,   415,   416,   417,    61,    62,    60,   418,   419,
     420,   421,   422,   124,    38,    45,    43,    42,    47,    37,
     423,   424,   425,   426,   427,   428,   429,   123,   125,    40,
      41,    58,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   193,   194,   195,   195,   196,   196,   197,   197,   198,
     198,   198,   198,   198,   198,   199,   200,   200,   201,   201,
     201,   201,   202,   202,   203,   203,   203,   204,   205,   205,
     205,   206,   206,   206,   207,   207,   208,   208,   208,   208,
     208,   208,   208,   209,   210,   211,   211,   212,   212,   213,
     213,   213,   213,   213,   213,   214,   214,   215,   215,   215,
     215,   216,   216,   216,   216,   216,   217,   217,   217,   218,
     219,   219,   220,   220,   221,   221,   222,   222,   223,   224,
     224,   225,   225,   226,   226,   226,   227,   228,   228,   228,
     228,   228,   228,   228,   228,   228,   228,   229,   229,   229,
     229,   230,   230,   231,   231,   231,   231,   232,   232,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   234,   234,   234,   234,   235,   235,   236,   236,   237,
     237,   238,   238,   239,   239,   240,   240,   240,   241,   241,
     242,   242,   243,   243,   244,   245,   246,   248,   247,   247,
     249,   249,   249,   249,   249,   250,   250,   250,   251,   251,
     252,   252,   253,   253,   254,   254,   255,   255,   256,   256,
     256,   257,   257,   258,   258,   258,   258,   258,   258,   258,
     258,   258,   258,   259,   260,   261,   260,   262,   262,   263,
     263,   264,   264,   265,   265,   265,   266,   266,   267,   268,
     269,   268,   270,   270,   272,   271,   274,   273,   275,   273,
     273,   276,   276,   277,   278,   278,   278,   279,   279,   280,
     280,   280,   281,   281,   282,   282,   282,   282,   282,   282,
     283,   283,   284,   284,   284,   285,   285,   286,   286,   287,
     287,   288,   288,   289,   289,   289,   290,   291,   291,   291,
     291,   292,   292,   292,   292,   293,   293,   294,   295,   295,
     295,   295,   295,   295,   296,   296,   297,   297,   298,   298,
     299,   299,   301,   300,   300,   302,   302,   303,   304,   304,
     305,   307,   306,   308,   309,   309,   311,   310,   312,   310,
     313,   313,   313,   315,   314,   317,   316,   319,   318,   320,
     320,   321,   321,   323,   322
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     1,     3,     0,     1,     1,     3,     0,
       1,     1,     1,     1,     1,     1,     1,     3,     0,     1,
       1,     1,     1,     3,     0,     1,     1,     4,     3,     3,
       1,     2,     5,     5,     1,     3,     5,     8,     5,     4,
       7,     5,     6,     2,     2,     0,     2,     1,     3,     0,
       4,     4,     4,     8,     8,     0,     2,     0,     3,     3,
       5,     0,     1,     2,     1,     5,     0,     3,     2,     1,
       0,     2,    11,     9,     2,     0,     7,     8,     3,     1,
       4,     0,     1,     3,     3,     5,     8,     1,     4,     4,
       4,     6,     4,     4,     4,     4,     4,     4,     7,     8,
       6,     0,     5,     1,     3,     3,     5,     0,     2,     1,
       3,     1,     1,     1,     1,     2,     2,     1,     2,     4,
       4,     1,     3,     3,     3,     3,     3,     2,     3,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       4,     3,     4,     3,     4,     2,     3,     3,     3,     4,
       1,     6,     5,     4,     3,     0,     1,     1,     3,     4,
       5,     1,     3,     6,     0,     0,     1,     3,     1,     3,
       3,     1,     1,     1,     5,    18,     4,     0,    10,     4,
      16,    13,    11,    18,    15,     0,     5,     2,     0,     5,
       0,     5,     0,     5,     0,     3,     3,     2,     0,     3,
       2,     0,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     0,     0,     3,     4,     1,     0,
       3,     4,     2,     0,     1,     1,     3,     1,     8,     0,
       0,     5,     0,     7,     0,     8,     0,     9,     0,     4,
       1,     1,     3,     3,     1,     1,     1,     0,     1,     0,
       1,     1,     3,     1,     7,     5,     2,     2,     3,     1,
       3,     1,     0,     1,     2,     3,     1,     1,     2,     0,
       3,     1,     1,     3,     3,     4,     3,     2,     3,     4,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       6,    10,     8,     4,     0,     2,     0,     2,     0,     3,
       0,     2,     0,     7,     1,     1,     3,     3,     0,     2,
       0,     0,     9,     8,     2,     2,     0,     4,     0,     4,
       1,     1,     1,     0,     6,     0,     6,     0,     6,     1,
       3,     3,     3,     0,     5
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       9,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   214,     7,    10,    25,    26,     0,     0,    12,
      11,    14,    13,     0,     0,     0,     0,     0,     0,    44,
      43,    79,     0,     0,     0,     0,     0,     0,     1,     0,
     316,   323,   333,   327,     0,   215,   234,   304,     9,     0,
       3,     6,    15,   204,     0,   212,   203,   211,   210,   205,
     206,   209,   207,   208,     0,     0,     0,     0,     0,    81,
      81,    81,     0,     0,     0,     0,    81,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     8,     2,     5,   236,   238,   213,   240,   218,
       0,    34,     0,     0,     0,    30,     0,     0,    27,     0,
       0,    82,     0,     0,     0,   179,     0,     0,     0,     0,
       0,    81,     0,    81,    81,     0,     0,     0,     0,     0,
       0,     0,   286,   285,   287,     0,     0,     0,   216,   227,
     107,     4,   249,     0,   244,   245,   246,   247,    31,     0,
       0,    57,    39,    49,     0,     0,     0,   109,   117,   113,
       0,   114,     0,     0,     0,   112,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    24,     0,   111,   150,
      87,    36,   121,     0,     0,     0,     0,     0,    78,    81,
      80,     0,    38,     0,     0,     0,     0,     0,   317,   319,
     322,   321,   320,     0,   296,     0,   325,     0,     0,     0,
       0,   250,   251,     0,     0,   296,   241,   248,     0,     0,
       0,    35,     0,     0,    66,    41,     0,     0,     0,    61,
      29,    28,     0,     0,   155,     0,   284,   283,   282,   281,
     115,   118,   116,     0,   145,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   214,
      22,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   129,   127,    24,     0,     0,     0,   171,
       0,     0,     0,   168,     0,    75,     0,     0,     0,     0,
       0,   219,     0,   334,     0,     0,   296,   329,     0,     0,
     226,   108,     0,   109,     0,   259,   262,   150,   262,   294,
     253,     0,     0,   239,   217,     0,     0,     0,    71,     0,
      42,     0,     0,     0,    62,     0,    64,    45,     0,    84,
     157,     0,   156,   110,     0,     0,     0,     0,   154,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   107,
     146,    24,     0,   147,   148,    83,   143,     0,   138,   137,
     131,   135,   133,   141,   139,   132,   134,   136,   123,   122,
     124,   125,   126,     0,     0,   128,   130,   214,   161,     0,
       0,     0,     0,     0,   176,     0,   177,    75,     0,    76,
       0,     0,     0,     0,   308,     0,   305,     0,   324,   297,
       0,     0,     0,     0,   328,   326,     0,   232,     0,     0,
     263,     0,   256,     0,     0,   257,     0,     0,   296,   243,
     242,    33,    32,    58,    59,    69,     0,    68,    47,     0,
      47,     0,    47,     0,    63,     0,     0,    40,    37,    90,
       0,     0,    89,     0,     0,   153,     0,     0,    96,    88,
      93,    92,    94,    95,    97,   120,   101,   119,    23,   149,
     144,   142,   140,     0,     0,    24,    18,     0,   170,     0,
     169,   165,    77,    74,     0,     0,     0,   311,     0,   310,
       0,   303,     0,   331,     0,   332,   330,     0,     0,   229,
     258,     0,   264,   269,     0,   286,     0,     0,   295,   287,
     288,   266,   252,   298,     0,    67,     0,     0,    51,     0,
      52,    50,     0,    46,     0,   158,    85,   159,     0,   152,
       0,     0,     0,    73,   162,   214,   214,    16,    19,    20,
      21,    18,     0,     0,     0,   166,   164,     0,     0,   313,
      57,   309,   307,   306,   223,   220,     0,   269,   230,   235,
       0,     0,   219,   261,     0,     0,     0,     0,     0,   267,
       0,   289,     0,   300,    60,     0,    48,     0,     0,    91,
     151,   160,     0,     0,     0,   100,     0,     0,    18,     0,
     198,     0,     0,     0,     0,   178,     0,    18,   164,   314,
     315,   312,   224,   225,   222,     0,     0,   219,     0,     0,
       0,     0,     0,   255,     0,     0,     0,   265,   268,     0,
       0,   219,    55,    55,    65,     0,    98,     0,    72,   197,
      17,   182,     0,   201,   185,     0,     5,   167,     0,     0,
      18,   223,   228,     0,     0,   262,   270,     0,     0,   280,
     271,   272,   260,     0,     0,     0,   155,   293,   299,   301,
     237,     0,     0,     0,     0,   103,    99,   196,   200,     0,
       0,     0,   188,   172,   173,     0,     0,     0,   198,     0,
     221,   280,     0,   254,     0,     0,     0,     0,     0,     0,
       0,     0,    56,    53,    54,     0,   102,     0,   199,     0,
     181,   187,     0,   192,     0,   174,    49,   194,   185,     0,
     231,   273,   274,     0,   276,   277,    86,   290,     0,     0,
     104,   105,   202,     0,     0,     0,     0,     0,     0,     0,
     201,   188,   233,   275,   278,     0,     0,     0,     0,     0,
       0,   184,     0,   163,     0,     0,   190,   279,     0,   292,
     106,   186,     0,     0,     0,   195,   180,     0,   192,     0,
     189,     0,     0,     0,     0,   291,   193,     0,     0,   183,
       0,     0,    81,   191,     0,     0,   175
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    11,    49,    50,    12,    13,    51,   536,   537,   259,
     260,    15,   108,   101,   111,    16,    17,    18,   447,   439,
     229,   662,   224,   337,   330,   437,   225,    19,   399,    20,
      75,    34,   112,   178,   316,   179,   180,   532,   664,   210,
     340,   182,   341,   342,   247,   390,   597,   544,   292,   290,
     675,   545,   293,   117,    21,   481,   540,   672,   703,   758,
     726,   730,   590,   633,   670,    52,    53,    54,    90,    97,
     408,   555,   604,   138,   139,   559,   608,   499,    98,    91,
      99,   142,   143,   215,   216,   147,   218,   213,   319,   320,
     564,   422,   508,   571,   562,   649,   650,   651,   688,   240,
     134,   510,   511,   428,   303,   573,   621,    56,    83,   405,
     406,   489,   552,    57,   550,    58,   549,    59,    84,    85,
     203,    60,    86,    61,   308,    62,    88,   306,   307,    63,
      87
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -581
static const yytype_int16 yypact[] =
{
     319,    67,    79,    82,    99,   151,   175,   197,   290,    44,
     265,   235,    23,  -581,  -581,  -581,  -581,   106,   -34,  -581,
    -581,  -581,  -581,   134,   110,   176,   179,   184,   -37,  -581,
    -581,  -581,   201,   256,   372,   273,   402,   427,  -581,   -62,
     202,  -581,  -581,  -581,   365,  -581,  -581,  -581,   319,    32,
    -581,  -581,  -581,  -581,   185,  -581,  -581,  -581,  -581,  -581,
    -581,  -581,  -581,  -581,   437,   350,   437,    21,    24,   437,
     437,   437,   463,   421,   459,   483,   437,   299,   437,   307,
     312,   503,   507,   411,   481,   418,   413,   458,    53,   525,
     527,   529,  -581,  -581,   212,  -581,  -581,   430,  -581,  -581,
      86,  -581,    55,   531,    57,  -581,   517,   437,  -581,   351,
     617,   382,   354,   355,   356,  -581,   361,   393,   368,   366,
     371,   437,    62,   437,   437,   501,   502,   561,   562,   563,
      68,    53,  -581,  -581,  -581,   453,   460,   386,   410,  -581,
     388,  -581,   166,   389,  -581,  -581,  -581,   508,   394,   396,
     437,   -45,  -581,   409,   574,   237,   424,   246,  -581,   448,
     580,   448,   401,   617,   276,  -581,   403,   405,   406,   407,
     412,   414,   422,   423,   425,   493,   200,   208,   416,  -581,
    -581,  1183,  -581,   -64,   419,   428,   596,   597,  -581,   437,
    -581,   426,  -581,   434,   435,   429,   438,   440,  -581,  -581,
    -581,  -581,  -581,   185,   530,   609,  -581,   437,   527,   611,
     542,  -581,  -581,   395,   395,   -43,  -581,  -581,   185,   624,
     631,  -581,   554,   632,   546,  -581,   450,   451,   454,   431,
    -581,  -581,   617,   640,   617,   642,  -581,  -581,  -581,  -581,
    -581,  -581,  -581,   617,   291,   617,  1127,    90,   617,   617,
     617,   617,   464,   617,   617,   523,   462,   617,   647,    43,
    -581,   812,     3,   650,   -83,   617,   617,   617,   617,   617,
     617,   617,   617,   617,   617,   617,   617,   617,   617,   617,
      80,   617,   617,  -581,  -581,   200,    18,    18,    18,  -581,
     113,   469,   515,  -581,   478,   655,   490,   491,   185,   185,
     437,   385,   617,  -581,   221,  -104,    49,  -581,   686,   120,
    -581,  -581,   687,   264,   504,  -581,    58,   619,   494,   -27,
    -581,   128,   389,  -581,  -581,   505,   513,    28,  -581,    74,
    -581,   689,   694,   695,  -581,   699,   607,   644,  1076,  -581,
    1183,   519,   549,   532,   838,   616,   232,   617,  -581,   617,
     868,   900,   926,   956,   521,   988,  1014,  1044,   617,   388,
     291,   200,   -96,  -581,  -581,  -581,  -581,   583,  1208,  1231,
     306,   306,   306,   306,   306,   306,   306,   306,   330,   330,
     291,   291,   291,   617,   617,  -581,  -581,    43,  -581,   437,
     526,   533,   536,   709,  -581,   659,   597,   655,   712,  -581,
      18,    18,   528,    36,   538,   130,  -581,   645,  -581,  1183,
     617,   721,   617,   609,  -581,  -581,   665,   653,    22,    19,
    -581,   725,  -581,   544,    16,  -581,    26,   395,   530,  -581,
    -581,  -581,  -581,  -581,   669,  -581,   731,  -581,   547,   135,
     548,   139,  -581,   180,  -581,   636,   185,  -581,  -581,   556,
     617,   736,  -581,   617,   617,  -581,   673,   648,  -581,  -581,
    -581,  -581,  -581,  -581,  -581,  -581,  1157,  -581,  -581,  -581,
    -581,   306,   306,    42,   186,   200,   286,   735,  -581,   740,
    -581,   688,  -581,  -581,   558,   559,   680,  -581,   625,  -581,
     437,  -581,   617,  1183,   532,  1183,  -581,   564,   565,   663,
    -581,   560,  -581,   677,   751,   681,   575,   185,   602,   215,
    -581,  -581,  -581,   692,   763,  -581,   695,   764,  -581,   695,
    -581,  -581,   668,   430,   765,  1183,  -581,  1183,   704,  -581,
     617,   543,   193,  -581,  -581,    43,   283,  -581,  -581,  -581,
    -581,   286,   610,   771,   263,  -581,   649,   770,   173,  -581,
     696,  -581,  -581,  -581,  1102,   628,   185,   677,  -581,  -581,
     786,   715,   723,  -581,   195,   623,   801,    47,    26,  -581,
     804,  -581,   741,   738,  -581,   206,  -581,   216,   727,  -581,
    -581,  1183,   641,   217,   617,  -581,   109,   646,   286,   183,
     720,    25,   732,   666,   688,  -581,   672,   286,   649,  -581,
    -581,  -581,  -581,  -581,  -581,   617,    50,   723,   674,   675,
     617,   207,   858,  -581,   240,   -42,   215,  -581,  -581,   617,
     617,   723,   703,   703,  -581,   617,  -581,   241,  -581,   617,
    -581,  -581,   700,   742,   745,   408,   212,  -581,   437,   231,
     286,  1102,  -581,   207,   609,    58,   549,   859,    34,   808,
    -581,  -581,  -581,   883,   889,   705,   617,  -581,   549,  1183,
    -581,   887,   706,   707,   247,   747,  -581,  1183,   617,   708,
     710,   711,   767,  -581,  -581,   743,  -100,   249,   720,    30,
    -581,   808,   251,  -581,   821,   243,   823,   896,   716,   717,
     240,   718,  -581,  -581,  -581,   617,  -581,   914,  1183,   617,
    -581,   734,   744,   791,   869,  -581,   409,   800,   745,   746,
    -581,  -581,  -581,   860,  -581,   448,  -581,  -581,   934,   749,
     785,  -581,  1183,   935,   754,   752,   756,   757,   789,   768,
     742,   767,  -581,  -581,   866,   775,   215,   952,   784,   966,
     798,  -581,   596,  -581,   617,   788,   852,  -581,   802,  -581,
    -581,  -581,   806,  1004,   262,  1183,  -581,   827,   791,   215,
    -581,   824,   865,   833,   839,  -581,  -581,   970,  1025,  -581,
     853,   862,   437,  -581,   851,   867,  -581
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -581,  -581,  -249,   954,  -581,  1008,  -581,  -458,   471,  -269,
      11,  -413,  -581,  -128,   -59,  -411,  -581,  -581,  -581,  -315,
     358,   457,   510,  -581,  -581,  -581,  -581,  -581,   664,  -581,
    -581,  -581,   -70,  -202,  -581,  -199,  -581,  -581,  -581,   714,
    -108,  -581,   443,  -340,   828,  -180,   484,  -581,  -581,   364,
    -581,   506,   719,  -581,  -581,  -581,    88,   400,   383,  -581,
     359,  -581,   497,   442,   399,  -581,  -581,  -581,  -581,  -168,
    -349,  -581,   489,  -581,   930,  -581,  -581,  -581,    -8,  -581,
    -176,  -581,  -581,  -581,   822,  -581,  -581,  -581,   931,   722,
    -581,  -305,  -581,  -580,   590,   509,  -564,  -581,   467,  -153,
    -392,   -50,   593,  -581,  -203,  -581,  -581,  -581,  -581,  -581,
     678,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,  -581,
    -581,  -581,  -581,  -581,  -581,  -581,  -581,   518,   774,  -581,
    -581
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -319
static const yytype_int16 yytable[] =
{
     113,   114,   181,   305,    55,   102,   120,   104,   242,   262,
     362,    14,   323,   425,   317,   317,   387,   441,   443,   122,
     502,   388,   221,   501,   105,    72,   343,   109,    65,   302,
     505,   433,   634,   655,   509,   301,   657,   708,   135,   222,
      -5,   366,   324,   685,   426,    81,    82,    31,   155,    93,
     654,   191,   223,   193,   194,   244,   246,   132,   106,    14,
      94,   412,   420,   538,    94,   539,   144,   145,   146,   261,
      23,    39,   132,    40,    41,    42,    43,    44,   263,   435,
     202,   204,    24,   591,    45,    25,    55,    32,    22,   148,
     705,    39,   469,    40,    41,    42,    43,    44,   367,   144,
     145,   146,    26,   414,    45,   318,   318,   391,   392,    46,
     144,   145,   146,   144,   145,   146,   686,   347,   322,   294,
     421,   302,   402,   285,   338,   348,   718,   286,   538,    46,
     539,   403,    47,   506,   427,   344,    22,   345,   473,   639,
     350,   351,   352,   353,   355,   356,   357,   656,   309,   360,
      73,    33,    47,   133,    27,    66,   749,   368,   369,   370,
     371,   372,   373,   374,   375,   376,   377,   378,   379,   380,
     381,   382,   404,   385,   386,   538,   509,   539,    28,   765,
     133,   436,   679,    48,   538,   588,   539,   434,   149,   110,
     588,   583,    94,   364,   409,   133,   349,   200,   201,   500,
      29,   575,    94,   361,   577,   504,   535,   389,   107,   359,
     413,   305,   157,   611,   158,   507,   150,   159,   150,   569,
     484,   485,   160,   150,   161,   513,   487,   538,   317,   539,
     533,    -5,   211,   212,    95,    38,     1,   616,     2,   456,
     642,   457,   599,   600,   627,   151,   162,   153,   383,   384,
     466,    55,   192,   236,   237,   238,   239,    95,   643,   454,
      39,  -214,    40,    41,    42,    43,    44,   455,    35,    94,
     646,    46,   660,    45,   393,   471,   472,   570,   523,   658,
     157,   150,   158,   647,   648,   159,   586,   589,    96,   427,
     160,   490,   161,    30,    46,    64,   517,   628,    46,    67,
     517,    68,   493,   394,   495,    36,    37,     7,     8,  -318,
     416,    96,   163,   164,   162,  -214,   647,   653,   429,   318,
     491,    47,     1,   712,     2,   518,     4,     5,   258,   520,
     474,    39,   165,    40,    41,    42,    43,    44,   349,   567,
     683,   517,   525,    94,    45,   527,   528,   150,   166,   167,
     168,   169,   170,   171,   584,     1,   612,     2,     3,     4,
       5,   172,   404,   587,   173,    69,   174,   517,    70,    46,
     521,   631,   468,    71,     6,    77,   534,   517,   450,    55,
     163,   164,   245,   585,   554,   613,   410,   676,   606,   175,
      74,   588,    47,     7,     8,   176,   622,   177,   150,   313,
     165,   158,   450,   233,   159,    79,   623,   626,   695,   160,
     150,   161,   413,   411,    10,   587,   166,   167,   168,   169,
     170,   171,   581,   393,   594,   231,     7,     8,   233,   172,
      80,   666,   173,   162,   174,   234,    89,   696,   235,   706,
     100,   710,   305,   588,     9,    76,   233,    10,   144,   145,
     146,   595,   762,   234,   673,   674,   418,   175,   236,   237,
     238,   239,    78,   176,   103,   177,   115,   118,   157,   407,
     158,    -5,   280,   159,   281,   282,   283,   284,   160,   116,
     161,   275,   276,   277,   278,   279,   119,   280,   121,   281,
     282,   283,   284,   144,   145,   146,   123,   641,   420,   163,
     164,   124,   162,   226,   227,   228,   125,   277,   278,   279,
     126,   280,   659,   281,   282,   283,   284,   665,   127,   165,
     128,   667,   334,   335,   336,   129,   130,    55,    55,   131,
     136,   137,   713,   140,   152,   166,   167,   168,   169,   170,
     171,   154,   156,   150,   183,   184,   185,   157,   172,   158,
     186,   173,   159,   314,   187,   189,   424,   160,   188,   161,
     698,   190,   734,   195,   196,   197,   198,   199,   163,   164,
     205,   208,   315,   206,   217,   207,   175,   230,   214,   677,
     209,   162,   176,   219,   177,   220,   241,   720,   165,   232,
     243,   722,   248,   264,   249,   250,   251,   257,   263,   289,
     291,   252,   302,   253,   166,   167,   168,   169,   170,   171,
     287,   254,   255,   304,   256,   311,   295,   172,   298,   288,
     173,   157,   174,   158,   296,   297,   159,   299,    55,   300,
     312,   160,   325,   161,   326,   327,   755,   328,   329,   331,
     332,   354,   453,   333,   339,   175,   343,   163,   164,   358,
      35,   176,   359,   177,   365,   162,   265,   266,   395,   267,
     268,   269,   270,   271,   272,   273,   274,   165,   397,   275,
     276,   277,   278,   279,   530,   280,   396,   281,   282,   283,
     284,   400,   401,   166,   167,   168,   169,   170,   171,   398,
     415,   417,   438,   419,   423,   431,   172,   440,   442,   173,
     582,   174,   774,   432,   444,   445,   446,   470,   529,   449,
     450,   462,   478,   475,   451,   264,   479,   483,   486,   488,
     476,   163,   164,   477,   175,   494,   492,   497,   498,   502,
     176,   514,   177,   503,   515,   522,   516,   519,   524,   580,
     526,   165,   541,   542,   543,   546,   547,   264,   548,   551,
     558,   561,   560,   556,   557,   563,   565,   166,   167,   168,
     169,   170,   171,   568,   566,   572,   574,   576,   578,   579,
     172,   592,   264,   173,   593,   174,   596,   598,   265,   266,
     222,   267,   268,   269,   270,   271,   272,   273,   274,   605,
     609,   275,   276,   277,   278,   279,   610,   280,   175,   281,
     282,   283,   284,   264,   176,   615,   177,   407,   618,   697,
     265,   266,   614,   267,   268,   269,   270,   271,   272,   273,
     274,   620,   619,   275,   276,   277,   278,   279,   624,   280,
     625,   281,   282,   283,   284,   265,   266,   629,   267,   268,
     269,   270,   271,   272,   273,   274,   264,   737,   275,   276,
     277,   278,   279,   632,   280,   636,   281,   282,   283,   284,
     635,   638,   652,   644,   661,   645,   265,   266,   684,   267,
     268,   269,   270,   271,   272,   273,   274,   671,   669,   275,
     276,   277,   278,   279,   264,   280,   687,   281,   282,   283,
     284,   668,   685,   689,   690,   692,   693,   694,   700,   699,
     702,   711,   701,   714,   704,   715,   716,   717,   719,   265,
     266,   264,   267,   268,   269,   270,   271,   272,   273,   274,
     721,   723,   275,   276,   277,   278,   279,   725,   280,   727,
     281,   282,   283,   284,   729,   724,   732,   264,   735,   736,
     733,   739,   738,   740,   741,   747,   742,   265,   266,   743,
     267,   268,   269,   270,   271,   272,   273,   274,   750,   744,
     275,   276,   277,   278,   279,   748,   280,   264,   281,   282,
     283,   284,   751,   752,   265,   266,   756,   267,   268,   269,
     270,   271,   272,   273,   274,   753,   757,   275,   276,   277,
     278,   279,   759,   280,   760,   281,   282,   283,   284,   264,
     265,   266,   363,   267,   268,   269,   270,   271,   272,   273,
     274,   761,   766,   275,   276,   277,   278,   279,   763,   280,
     768,   281,   282,   283,   284,   264,   767,   769,   452,   770,
     265,   266,   771,   267,   268,   269,   270,   271,   272,   273,
     274,   775,   772,   275,   276,   277,   278,   279,   141,   280,
     773,   281,   282,   283,   284,   264,    92,   776,   458,   630,
     601,   482,   265,   266,   728,   267,   268,   269,   270,   271,
     272,   273,   274,   467,   346,   275,   276,   277,   278,   279,
     663,   280,   640,   281,   282,   283,   284,   264,   265,   266,
     459,   267,   268,   269,   270,   271,   272,   273,   274,   691,
     637,   275,   276,   277,   278,   279,   754,   280,   731,   281,
     282,   283,   284,   264,   746,   480,   460,   764,   265,   266,
     707,   267,   268,   269,   270,   271,   272,   273,   274,   745,
     680,   275,   276,   277,   278,   279,   678,   280,   310,   281,
     282,   283,   284,   264,   430,   321,   461,   607,   709,   512,
     265,   266,   681,   267,   268,   269,   270,   271,   272,   273,
     274,   617,   682,   275,   276,   277,   278,   279,   553,   280,
       0,   281,   282,   283,   284,   264,   265,   266,   463,   267,
     268,   269,   270,   271,   272,   273,   274,   496,     0,   275,
     276,   277,   278,   279,     0,   280,     0,   281,   282,   283,
     284,   264,     0,     0,   464,     0,   265,   266,     0,   267,
     268,   269,   270,   271,   272,   273,   274,   602,   603,   275,
     276,   277,   278,   279,     0,   280,   264,   281,   282,   283,
     284,     0,     0,   245,   465,     0,   448,     0,   265,   266,
       0,   267,   268,   269,   270,   271,   272,   273,   274,     0,
       0,   275,   276,   277,   278,   279,   264,   280,     0,   281,
     282,   283,   284,     0,   265,   266,     0,   267,   268,   269,
     270,   271,   272,   273,   274,     0,     0,   275,   276,   277,
     278,   279,   264,   280,     0,   281,   282,   283,   284,   265,
     266,     0,   267,   268,   269,   270,   271,   272,   273,   274,
       0,     0,   275,   276,   277,   278,   279,   264,   280,     0,
     281,   282,   283,   284,     0,     0,     0,     0,   531,   265,
     266,     0,   267,   268,   269,   270,   271,   272,   273,   274,
     264,     0,   275,   276,   277,   278,   279,     0,   280,     0,
     281,   282,   283,   284,     0,   265,   266,     0,   267,   268,
     269,   270,   271,   272,   273,   274,     0,     0,   275,   276,
     277,   278,   279,     0,   280,     0,   281,   282,   283,   284,
       0,   266,     0,   267,   268,   269,   270,   271,   272,   273,
     274,     0,     0,   275,   276,   277,   278,   279,     0,   280,
       0,   281,   282,   283,   284,     0,   267,   268,   269,   270,
     271,   272,   273,   274,     0,     0,   275,   276,   277,   278,
     279,     0,   280,     0,   281,   282,   283,   284
};

static const yytype_int16 yycheck[] =
{
      70,    71,   110,   205,    12,    64,    76,    66,   161,   177,
     259,     0,   215,   318,   213,   214,   285,   332,   333,    78,
       4,     3,   150,     4,     3,    62,     4,     3,    62,    72,
       4,     3,     7,    75,   426,   203,   616,     7,    88,    84,
      17,   124,   218,     9,    71,   107,   108,     3,   107,    17,
     614,   121,    97,   123,   124,   163,   164,     4,    37,    48,
     160,   165,     4,   476,   160,   476,    63,    64,    65,   177,
       3,    48,     4,    50,    51,    52,    53,    54,   182,     5,
     130,   131,     3,   541,    61,     3,    94,    43,     0,     3,
     190,    48,   188,    50,    51,    52,    53,    54,   181,    63,
      64,    65,     3,   306,    61,   213,   214,   287,   288,    86,
      63,    64,    65,    63,    64,    65,    82,    27,   161,   189,
      62,    72,   298,   187,   232,    35,   690,   191,   541,    86,
     541,   299,   109,   107,   161,   243,    48,   245,   387,   597,
     248,   249,   250,   251,   252,   253,   254,   189,   207,   257,
     187,   107,   109,   127,     3,   189,   736,   265,   266,   267,
     268,   269,   270,   271,   272,   273,   274,   275,   276,   277,
     278,   279,   300,   281,   282,   588,   568,   588,     3,   759,
     127,   107,   640,   160,   597,   160,   597,   159,   102,   165,
     160,   531,   160,   190,   302,   127,   106,   129,   130,   177,
       3,   516,   160,   160,   519,   189,   475,   189,   187,   190,
     161,   413,     4,   562,     6,   189,   161,     9,   161,     4,
     400,   401,    14,   161,    16,   428,   190,   640,   427,   640,
     188,   188,    66,    67,    49,     0,    36,   190,    38,   347,
     190,   349,    69,    70,   584,   190,    38,   190,   168,   169,
     358,   259,   190,    10,    11,    12,    13,    49,   607,    27,
      48,    49,    50,    51,    52,    53,    54,    35,     3,   160,
     610,    86,   621,    61,   161,   383,   384,    62,   446,   619,
       4,   161,     6,    76,    77,     9,   535,   536,   103,   161,
      14,   161,    16,     3,    86,   189,   161,   188,    86,   165,
     161,   191,   410,   190,   412,    40,    41,   107,   108,   107,
     190,   103,   104,   105,    38,   103,    76,    77,   190,   427,
     190,   109,    36,    80,    38,   190,    40,    41,   128,   190,
     389,    48,   124,    50,    51,    52,    53,    54,   106,   507,
     645,   161,   450,   160,    61,   453,   454,   161,   140,   141,
     142,   143,   144,   145,   161,    36,   161,    38,    39,    40,
      41,   153,   490,   132,   156,   189,   158,   161,   189,    86,
     190,   188,   361,   189,    55,     3,   190,   161,   161,   387,
     104,   105,   106,   190,   492,   190,   165,   636,   556,   181,
     189,   160,   109,   107,   108,   187,   190,   189,   161,     4,
     124,     6,   161,   182,     9,     3,   190,   190,   161,    14,
     161,    16,   161,   192,   128,   132,   140,   141,   142,   143,
     144,   145,   530,   161,   161,   188,   107,   108,   182,   153,
       3,   190,   156,    38,   158,   189,    71,   190,   192,   190,
       3,   190,   644,   160,   125,   189,   182,   128,    63,    64,
      65,   188,   190,   189,    46,    47,   192,   181,    10,    11,
      12,    13,   189,   187,   114,   189,     3,     8,     4,    84,
       6,   188,   181,     9,   183,   184,   185,   186,    14,    58,
      16,   175,   176,   177,   178,   179,     3,   181,   189,   183,
     184,   185,   186,    63,    64,    65,   189,   605,     4,   104,
     105,   189,    38,    94,    95,    96,     3,   177,   178,   179,
       3,   181,   620,   183,   184,   185,   186,   625,   107,   124,
      39,   629,    91,    92,    93,   107,   113,   535,   536,    71,
       5,     4,   685,     4,     3,   140,   141,   142,   143,   144,
     145,    24,   191,   161,   190,   190,   190,     4,   153,     6,
     189,   156,     9,   158,   161,   189,    62,    14,   190,    16,
     668,   190,   715,    62,    62,     4,     4,     4,   104,   105,
     117,   161,   177,   113,    66,   189,   181,     3,   189,   638,
     192,    38,   187,   189,   189,   189,     6,   695,   124,   165,
     189,   699,   189,    99,   189,   189,   189,   104,   182,     3,
       3,   189,    72,   189,   140,   141,   142,   143,   144,   145,
     191,   189,   189,     4,   189,     4,   190,   153,   189,   191,
     156,     4,   158,     6,   190,   190,     9,   189,   636,   189,
      88,    14,     8,    16,     3,    81,   744,     5,    92,   189,
     189,   177,    26,   189,     4,   181,     4,   104,   105,   126,
       3,   187,   190,   189,     4,    38,   162,   163,   189,   165,
     166,   167,   168,   169,   170,   171,   172,   124,   190,   175,
     176,   177,   178,   179,    26,   181,   161,   183,   184,   185,
     186,   191,   191,   140,   141,   142,   143,   144,   145,    34,
       4,     4,     3,   189,    75,   190,   153,     3,     3,   156,
     157,   158,   772,   190,     5,    98,    62,   124,    35,   190,
     161,   190,     3,   187,   182,    99,    57,     5,   190,   181,
     187,   104,   105,   187,   181,     4,    81,    62,    75,     4,
     187,    62,   189,   189,     3,    99,   189,   189,   182,    35,
       4,   124,     7,     3,    56,   187,   187,    99,    68,   124,
      87,    74,   192,   189,   189,     4,    75,   140,   141,   142,
     143,   144,   145,   161,   189,    73,     3,     3,   100,     4,
     153,   161,    99,   156,     3,   158,   127,     7,   162,   163,
      84,   165,   166,   167,   168,   169,   170,   171,   172,   161,
       4,   175,   176,   177,   178,   179,    81,   181,   181,   183,
     184,   185,   186,    99,   187,     4,   189,    84,     4,    62,
     162,   163,   189,   165,   166,   167,   168,   169,   170,   171,
     172,    83,    81,   175,   176,   177,   178,   179,   101,   181,
     189,   183,   184,   185,   186,   162,   163,   191,   165,   166,
     167,   168,   169,   170,   171,   172,    99,    62,   175,   176,
     177,   178,   179,   133,   181,   189,   183,   184,   185,   186,
     128,   189,     4,   189,   161,   190,   162,   163,     9,   165,
     166,   167,   168,   169,   170,   171,   172,   132,   136,   175,
     176,   177,   178,   179,    99,   181,    78,   183,   184,   185,
     186,   191,     9,     4,   189,     8,   190,   190,   188,   191,
     133,    80,   191,    80,   161,     9,   190,   190,   190,   162,
     163,    99,   165,   166,   167,   168,   169,   170,   171,   172,
       6,   187,   175,   176,   177,   178,   179,   136,   181,    60,
     183,   184,   185,   186,   134,   191,   190,    99,     4,   190,
      80,   187,     7,   191,   188,    79,   189,   162,   163,   160,
     165,   166,   167,   168,   169,   170,   171,   172,     6,   191,
     175,   176,   177,   178,   179,   190,   181,    99,   183,   184,
     185,   186,   188,     7,   162,   163,   188,   165,   166,   167,
     168,   169,   170,   171,   172,   187,   134,   175,   176,   177,
     178,   179,   190,   181,   188,   183,   184,   185,   186,    99,
     162,   163,   190,   165,   166,   167,   168,   169,   170,   171,
     172,     7,   188,   175,   176,   177,   178,   179,   191,   181,
     187,   183,   184,   185,   186,    99,   161,   188,   190,    59,
     162,   163,     7,   165,   166,   167,   168,   169,   170,   171,
     172,   190,   189,   175,   176,   177,   178,   179,    94,   181,
     188,   183,   184,   185,   186,    99,    48,   190,   190,   588,
     550,   397,   162,   163,   706,   165,   166,   167,   168,   169,
     170,   171,   172,   359,   246,   175,   176,   177,   178,   179,
     623,   181,   598,   183,   184,   185,   186,    99,   162,   163,
     190,   165,   166,   167,   168,   169,   170,   171,   172,   656,
     594,   175,   176,   177,   178,   179,   742,   181,   708,   183,
     184,   185,   186,    99,   731,   396,   190,   758,   162,   163,
     678,   165,   166,   167,   168,   169,   170,   171,   172,   730,
     641,   175,   176,   177,   178,   179,   639,   181,   208,   183,
     184,   185,   186,    99,   322,   214,   190,   557,   681,   427,
     162,   163,   643,   165,   166,   167,   168,   169,   170,   171,
     172,   568,   644,   175,   176,   177,   178,   179,   490,   181,
      -1,   183,   184,   185,   186,    99,   162,   163,   190,   165,
     166,   167,   168,   169,   170,   171,   172,   413,    -1,   175,
     176,   177,   178,   179,    -1,   181,    -1,   183,   184,   185,
     186,    99,    -1,    -1,   190,    -1,   162,   163,    -1,   165,
     166,   167,   168,   169,   170,   171,   172,   115,   116,   175,
     176,   177,   178,   179,    -1,   181,    99,   183,   184,   185,
     186,    -1,    -1,   106,   190,    -1,   160,    -1,   162,   163,
      -1,   165,   166,   167,   168,   169,   170,   171,   172,    -1,
      -1,   175,   176,   177,   178,   179,    99,   181,    -1,   183,
     184,   185,   186,    -1,   162,   163,    -1,   165,   166,   167,
     168,   169,   170,   171,   172,    -1,    -1,   175,   176,   177,
     178,   179,    99,   181,    -1,   183,   184,   185,   186,   162,
     163,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      -1,    -1,   175,   176,   177,   178,   179,    99,   181,    -1,
     183,   184,   185,   186,    -1,    -1,    -1,    -1,   161,   162,
     163,    -1,   165,   166,   167,   168,   169,   170,   171,   172,
      99,    -1,   175,   176,   177,   178,   179,    -1,   181,    -1,
     183,   184,   185,   186,    -1,   162,   163,    -1,   165,   166,
     167,   168,   169,   170,   171,   172,    -1,    -1,   175,   176,
     177,   178,   179,    -1,   181,    -1,   183,   184,   185,   186,
      -1,   163,    -1,   165,   166,   167,   168,   169,   170,   171,
     172,    -1,    -1,   175,   176,   177,   178,   179,    -1,   181,
      -1,   183,   184,   185,   186,    -1,   165,   166,   167,   168,
     169,   170,   171,   172,    -1,    -1,   175,   176,   177,   178,
     179,    -1,   181,    -1,   183,   184,   185,   186
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    36,    38,    39,    40,    41,    55,   107,   108,   125,
     128,   194,   197,   198,   203,   204,   208,   209,   210,   220,
     222,   247,   249,     3,     3,     3,     3,     3,     3,     3,
       3,     3,    43,   107,   224,     3,    40,    41,     0,    48,
      50,    51,    52,    53,    54,    61,    86,   109,   160,   195,
     196,   199,   258,   259,   260,   271,   300,   306,   308,   310,
     314,   316,   318,   322,   189,    62,   189,   165,   191,   189,
     189,   189,    62,   187,   189,   223,   189,     3,   189,     3,
       3,   107,   108,   301,   311,   312,   315,   323,   319,    71,
     261,   272,   198,    17,   160,    49,   103,   262,   271,   273,
       3,   206,   207,   114,   207,     3,    37,   187,   205,     3,
     165,   207,   225,   225,   225,     3,    58,   246,     8,     3,
     225,   189,   207,   189,   189,     3,     3,   107,    39,   107,
     113,    71,     4,   127,   293,   294,     5,     4,   266,   267,
       4,   196,   274,   275,    63,    64,    65,   278,     3,   102,
     161,   190,     3,   190,    24,   207,   191,     4,     6,     9,
      14,    16,    38,   104,   105,   124,   140,   141,   142,   143,
     144,   145,   153,   156,   158,   181,   187,   189,   226,   228,
     229,   233,   234,   190,   190,   190,   189,   161,   190,   189,
     190,   225,   190,   225,   225,    62,    62,     4,     4,     4,
     129,   130,   294,   313,   294,   117,   113,   189,   161,   192,
     232,    66,    67,   280,   189,   276,   277,    66,   279,   189,
     189,   206,    84,    97,   215,   219,    94,    95,    96,   213,
       3,   188,   165,   182,   189,   192,    10,    11,    12,    13,
     292,     6,   292,   189,   233,   106,   233,   237,   189,   189,
     189,   189,   189,   189,   189,   189,   189,   104,   128,   202,
     203,   233,   262,   182,    99,   162,   163,   165,   166,   167,
     168,   169,   170,   171,   172,   175,   176,   177,   178,   179,
     181,   183,   184,   185,   186,   187,   191,   191,   191,     3,
     242,     3,   241,   245,   225,   190,   190,   190,   189,   189,
     189,   262,    72,   297,     4,   226,   320,   321,   317,   207,
     267,     4,    88,     4,   158,   177,   227,   228,   233,   281,
     282,   281,   161,   297,   273,     8,     3,    81,     5,    92,
     217,   189,   189,   189,    91,    92,    93,   216,   233,     4,
     233,   235,   236,     4,   233,   233,   237,    27,    35,   106,
     233,   233,   233,   233,   177,   233,   233,   233,   126,   190,
     233,   160,   195,   190,   190,     4,   124,   181,   233,   233,
     233,   233,   233,   233,   233,   233,   233,   233,   233,   233,
     233,   233,   233,   168,   169,   233,   233,   202,     3,   189,
     238,   238,   238,   161,   190,   189,   161,   190,    34,   221,
     191,   191,   273,   262,   206,   302,   303,    84,   263,   233,
     165,   192,   165,   161,   297,     4,   190,     4,   192,   189,
       4,    62,   284,    75,    62,   284,    71,   161,   296,   190,
     277,   190,   190,     3,   159,     5,   107,   218,     3,   212,
       3,   212,     3,   212,     5,    98,    62,   211,   160,   190,
     161,   182,   190,    26,    27,    35,   233,   233,   190,   190,
     190,   190,   190,   190,   190,   190,   233,   232,   203,   188,
     124,   233,   233,   195,   207,   187,   187,   187,     3,    57,
     245,   248,   221,     5,   238,   238,   190,   190,   181,   304,
     161,   190,    81,   233,     4,   233,   321,    62,    75,   270,
     177,     4,     4,   189,   189,     4,   107,   189,   285,   293,
     294,   295,   282,   297,    62,     3,   189,   161,   190,   189,
     190,   190,    99,   262,   182,   233,     4,   233,   233,    35,
      26,   161,   230,   188,   190,   202,   200,   201,   204,   208,
     249,     7,     3,    56,   240,   244,   187,   187,    68,   309,
     307,   124,   305,   303,   233,   264,   189,   189,    87,   268,
     192,    74,   287,     4,   283,    75,   189,   262,   161,     4,
      62,   286,    73,   298,     3,   212,     3,   212,   100,     4,
      35,   233,   157,   236,   161,   190,   195,   132,   160,   195,
     255,   200,   161,     3,   161,   188,   127,   239,     7,    69,
      70,   215,   115,   116,   265,   161,   262,   287,   269,     4,
      81,   263,   161,   190,   189,     4,   190,   295,     4,    81,
      83,   299,   190,   190,   101,   189,   190,   236,   188,   191,
     201,   188,   133,   256,     7,   128,   189,   244,   189,   200,
     239,   233,   190,   263,   189,   190,   236,    76,    77,   288,
     289,   290,     4,    77,   289,    75,   189,   286,   236,   233,
     263,   161,   214,   214,   231,   233,   190,   233,   191,   136,
     257,   132,   250,    46,    47,   243,   195,   207,   255,   200,
     265,   288,   320,   284,     9,     9,    82,    78,   291,     4,
     189,   235,     8,   190,   190,   161,   190,    62,   233,   191,
     188,   191,   133,   251,   161,   190,   190,   256,     7,   291,
     190,    80,    80,   292,    80,     9,   190,   190,   289,   190,
     233,     6,   233,   187,   191,   136,   253,    60,   213,   134,
     254,   250,   190,    80,   292,     4,   190,    62,     7,   187,
     191,   188,   189,   160,   191,   257,   251,    79,   190,   286,
       6,   188,     7,   187,   242,   233,   188,   134,   252,   190,
     188,     7,   190,   191,   253,   286,   188,   161,   187,   188,
      59,     7,   189,   188,   225,   190,   190
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, Location); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (yylocationp);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, yylocationp)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    YYLTYPE const * const yylocationp;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yylsp, yyrule)
    YYSTYPE *yyvsp;
    YYLTYPE *yylsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       , &(yylsp[(yyi + 1) - (yynrhs)])		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, yylsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, yylocationp)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    YYLTYPE *yylocationp;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Location data for the lookahead symbol.  */
YYLTYPE yylloc;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.
       `yyls': related to locations.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[2];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;
  yylsp = yyls;

#if YYLTYPE_IS_TRIVIAL
  /* Initialize the default location before parsing starts.  */
  yylloc.first_line   = yylloc.last_line   = 1;
  yylloc.first_column = yylloc.last_column = 1;
#endif

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	YYLTYPE *yyls1 = yyls;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);

	yyls = yyls1;
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
	YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
{
int dodefault = (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yytoken);
int newlookahead; 
if (((dodefault && yydefact[yystate]==0) ||
(!dodefault && (yytable[yyn] == YYPACT_NINF || yytable[yyn] == 0))) &&
(newlookahead = yylex2())) {
yychar = newlookahead;	goto yybackup;	
}	
}
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;
  *++yylsp = yylloc;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location.  */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 258 "adl_yacc.grm"
    {
      abs_adl = A_LetExp((yylsp[(1) - (3)]).first_line, (yyvsp[(1) - (3)].list), A_SeqExp((yylsp[(2) - (3)]).first_line, (yyvsp[(2) - (3)].list)));
    }
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 265 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, /*list,*/ A_EXP);
      if ((yyvsp[(1) - (1)].exp) != (A_exp)0) 
	 appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].exp));
    }
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 271 "adl_yacc.grm"
    {
      if ((yyvsp[(3) - (3)].exp)  !=  (A_exp)0)
	 appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].exp));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 280 "adl_yacc.grm"
    {
	(yyval.exp) = (A_exp)0;
    }
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 284 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 290 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, /*list,*/ A_DEC);
      if ((yyvsp[(1) - (1)].dec) != (A_dec)0) {
	 appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].dec));
      }
    }
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 297 "adl_yacc.grm"
    {
      if ((yyvsp[(3) - (3)].dec) != (A_dec)0) {
	appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].dec));
      }
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 307 "adl_yacc.grm"
    {
	(yyval.dec) = (A_dec)0;
    }
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 311 "adl_yacc.grm"
    { 
      (yyval.dec)=(yyvsp[(1) - (1)].dec);
    }
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 315 "adl_yacc.grm"
    {
      (yyval.dec)=(yyvsp[(1) - (1)].dec);
    }
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 319 "adl_yacc.grm"
    { 
      (yyval.dec) = (yyvsp[(1) - (1)].dec); 
    }
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 323 "adl_yacc.grm"
    { 
      (yyval.dec) = (yyvsp[(1) - (1)].dec); 
    }
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 327 "adl_yacc.grm"
    {
      (yyval.dec) = (yyvsp[(1) - (1)].dec); 
    }
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 333 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 340 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_DEC);
      if ((yyvsp[(1) - (1)].dec) != (A_dec)0) {
	appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].dec));
      }
    }
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 347 "adl_yacc.grm"
    {
      if ((yyvsp[(3) - (3)].dec) != (A_dec)0) 
	appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].dec));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 356 "adl_yacc.grm"
    {
	(yyval.dec) = (A_dec)0;
    }
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 360 "adl_yacc.grm"
    { 
      /* to be implemented 
	 A_TypeDec(@1.first_line, $1);
      */

      (yyval.dec)=(A_dec)0; 
    }
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 368 "adl_yacc.grm"
    { 
      (yyval.dec)=(yyvsp[(1) - (1)].dec);
    }
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 372 "adl_yacc.grm"
    { 
      (yyval.dec) = (yyvsp[(1) - (1)].dec); 
    }
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 378 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_DEC);
      if ((yyvsp[(1) - (1)].dec) != (A_dec)0) {
	appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].dec));
      }
    }
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 385 "adl_yacc.grm"
    {
      if ((yyvsp[(3) - (3)].dec) != (A_dec)0) 
	appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].dec));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 394 "adl_yacc.grm"
    {
	(yyval.dec) = (A_dec)0;
    }
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 398 "adl_yacc.grm"
    { 
      /* to be implemented 
	 A_TypeDec(@1.first_line, $1);
      */

      (yyval.dec)=(A_dec)0; 
    }
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 406 "adl_yacc.grm"
    { 
      (yyval.dec)=(yyvsp[(1) - (1)].dec);
    }
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 455 "adl_yacc.grm"
    {
      (yyval.namety)=A_Namety(S_Symbol((yyvsp[(2) - (4)].sval)), (yyvsp[(4) - (4)].ty));
    }
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 461 "adl_yacc.grm"
    {
      (yyval.ty) = A_RecordTy((yylsp[(1) - (3)]).first_line, (yyvsp[(2) - (3)].list));
    }
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 465 "adl_yacc.grm"
    {
      (yyval.ty) = A_ArrayTy((yylsp[(1) - (3)]).first_line, S_Symbol((yyvsp[(3) - (3)].sval)));
    }
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 469 "adl_yacc.grm"
    {
      (yyval.ty) = A_NameTy((yylsp[(1) - (1)]).first_line, S_Symbol((yyvsp[(1) - (1)].sval)));
    }
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 475 "adl_yacc.grm"
    {
      (yyval.field) = A_Field((yylsp[(1) - (2)]).first_line, S_Symbol((yyvsp[(1) - (2)].sval)), S_Symbol((yyvsp[(2) - (2)].sval)));
    }
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 479 "adl_yacc.grm"
    {
      (yyval.field) = A_Field((yylsp[(1) - (5)]).first_line, 
		   S_Symbol((yyvsp[(1) - (5)].sval)), 
		   S_Symbol((yyvsp[(4) - (5)].sval)), 
		   -1 /* -1 indicates a ref type */);
    }
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 486 "adl_yacc.grm"
    {
      (yyval.field) = A_Field((yylsp[(1) - (5)]).first_line, S_Symbol((yyvsp[(1) - (5)].sval)), S_Symbol((yyvsp[(2) - (5)].sval)), (yyvsp[(4) - (5)].ival));
    }
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 492 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_FIELD);
      appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].field));
    }
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 497 "adl_yacc.grm"
    {
      appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].field));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 504 "adl_yacc.grm"
    {
      (yyval.dec) = A_VarDec((yylsp[(2) - (5)]).first_line, S_Symbol((yyvsp[(2) - (5)].sval)), (S_symbol)0, (yyvsp[(5) - (5)].exp));
    }
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 508 "adl_yacc.grm"
    {
      (yyval.dec) = A_VarDec((yylsp[(2) - (8)]).first_line, S_Symbol((yyvsp[(2) - (8)].sval)), S_Symbol((yyvsp[(4) - (8)].sval)), (yyvsp[(7) - (8)].exp));
    }
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 512 "adl_yacc.grm"
    {
      A_ty ty = A_RecordTy((yylsp[(4) - (5)]).first_line, (yyvsp[(4) - (5)].list));

      int index=0; /* B_TREE */
      S_symbol s = NULL;
      char* iName;
      mkInternalName((yyvsp[(2) - (5)].sval), (iName), 0); //Don't notify since this 
					// dec may not even be used
      s = S_Symbol(strdup(iName));
      free(iName);

      //if ($6) {
	// index  = (int)popElementList($6);
      //}	

      (yyval.dec) = A_TabVarDec((yylsp[(2) - (5)]).first_line, 
		       s,	// name of the table        
		       ty,		// tuple type               
		       (A_index)0,  	// keys to the index 
		       1,	// MEMORY    
		       (A_exp)0,		// init
		       1
		       ); 
    }
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 537 "adl_yacc.grm"
    {
      S_symbol s = NULL;
      char* iName;
      mkInternalName((yyvsp[(1) - (4)].sval), (iName), 0); //Don't notify since this 
					// dec may not even be used
      s = S_Symbol(strdup(iName));
      free(iName);
      S_symbol s1 = NULL;
      char* iName1;
      mkInternalName((yyvsp[(4) - (4)].sval), (iName1), 0); //Don't notify since this 
					// dec may not even be used
      s1 = S_Symbol(strdup(iName1));
      free(iName1);

      (yyval.dec) = A_DynamicVarDec((yylsp[(1) - (4)]).first_line, s, s1, S_Symbol((yyvsp[(4) - (4)].sval)));

    }
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 555 "adl_yacc.grm"
    {
      A_ty ty = A_RecordTy((yylsp[(1) - (7)]).first_line, (yyvsp[(3) - (7)].list));

      S_symbol s = NULL;
      char* iName;
      mkInternalName((yyvsp[(1) - (7)].sval), (iName), 0); //Don't notify since this 
					// dec may not even be used
      s = S_Symbol(strdup(iName));
      free(iName);
      
      //if ($5) {
	// index  = (int)popElementList($5);
      //}	 

      (yyval.dec) = A_TabVarDec((yylsp[(1) - (7)]).first_line, 
		       s,	// name of the table        
		       ty,		// tuple type               
		       (yyvsp[(5) - (7)].index),		// index  specification
		       (yyvsp[(6) - (7)].ival),		// global, local, memory, oid    
		       (yyvsp[(7) - (7)].exp)		// init
		       ); 
    }
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 592 "adl_yacc.grm"
    {
	A_ty ty = A_RecordTy((yylsp[(1) - (5)]).first_line, (yyvsp[(3) - (5)].list));
	timekey_t tk = tk_none;
	
        S_symbol s = NULL;
        
	char* iName = NULL;
        mkInternalName((yyvsp[(1) - (5)].sval), iName, 0);  //do not notify since this dec
					 // may not even be used
        s = S_Symbol(strdup(iName));
        free(iName);
        
      	(yyval.dec) = A_StreamVarDec((yylsp[(1) - (5)]).first_line, 
		       s,		// name of the table        
		       ty,		// tuple type               
		       NULL,		// source
		       (yyvsp[(5) - (5)].symval),		// target	
		       tk,              // time key flag
  		       0,		// time key
		       NULL		// init
		       ); 
    }
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 615 "adl_yacc.grm"
    {
	A_ty ty = A_RecordTy((yylsp[(1) - (6)]).first_line, (yyvsp[(3) - (6)].list));
 	char *timekey = (yyvsp[(5) - (6)].sval);
	timekey_t tk;
  	if (timekey){  //external or internal timestamp
	    if (strstr(timekey, ITIME_COLUMN)){  // internal timekey
	      tk = tk_internal;
	      if (strlen(timekey) > strlen(ITIME_COLUMN)){
		// remove ITME_COLUMN
		timekey[strlen(timekey) - strlen(ITIME_COLUMN)] = 0;
      		}
	    }
		else{ // external timekey
			tk = tk_external;
		    }
	  }
	  else{  // no timestamp
	    tk = tk_none;
	  }

        S_symbol s = NULL;
        
	char* iName = NULL;
        mkInternalName((yyvsp[(1) - (6)].sval), iName, 0);  //do not notify since this dec
					 // may not even be used
        s = S_Symbol(strdup(iName));
        free(iName);
        
      	(yyval.dec) = A_StreamVarDec((yylsp[(1) - (6)]).first_line, 
		       s,	// name of the table        
		       ty,		// tuple type               
		       (yyvsp[(6) - (6)].symval),		// source
		      	NULL,		// target	
			tk,             // time key flag
		       timekey?S_Symbol(timekey):0,	// time key
			NULL		// init
		       ); 
    }
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 698 "adl_yacc.grm"
    { 
      (yyval.sval) = (yyvsp[(2) - (2)].sval);
    }
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 713 "adl_yacc.grm"
    { 
      (yyval.sval) = (yyvsp[(2) - (2)].sval);
    }
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 734 "adl_yacc.grm"
    {
	(yyval.exp) = (A_exp)0;
    }
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 738 "adl_yacc.grm"
    {
	(yyval.exp) = (yyvsp[(2) - (2)].exp);
	in_sql = 0;
    }
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 745 "adl_yacc.grm"
    {	 
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_SYMBOL);
      appendAList((yyval.list), (void*)S_Symbol((yyvsp[(1) - (1)].sval)));
    }
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 750 "adl_yacc.grm"
    {
      appendAList((yyvsp[(1) - (3)].list), (void*)S_Symbol((yyvsp[(3) - (3)].sval)));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 785 "adl_yacc.grm"
    {
      (yyval.index) = (A_index)0; /* DB_BTREE (default) defined in absyn.h */
    }
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 789 "adl_yacc.grm"
    {
      //appendAList($3, (void*)5);  /* DB_RTREE defined in absyn.h */
      (yyval.index) = A_Index((yylsp[(3) - (4)]).first_line, (tabindex_t)5, (yyvsp[(3) - (4)].list));
    }
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 794 "adl_yacc.grm"
    {
      //appendAList($3, (void*)1);  /* DB_BTREE (default) defined in absyn.h */
      (yyval.index) = A_Index((yylsp[(3) - (4)]).first_line, (tabindex_t)1, (yyvsp[(3) - (4)].list));
    }
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 799 "adl_yacc.grm"
    {
      //appendAList($3, (void*)1);  /* same as BTREE - when we have memory we do hash*/
      (yyval.index) = A_Index((yylsp[(3) - (4)]).first_line, (tabindex_t)1, (yyvsp[(3) - (4)].list));
    }
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 804 "adl_yacc.grm"
    {
      if(strcmp((yyvsp[(3) - (8)].sval), "ditto") != 0) {
        S_symbol s = NULL;
        
	char* iName = NULL;
        mkInternalName((yyvsp[(3) - (8)].sval), iName, 0);  //do not notify since this dec
					 // may not even be used
        s = S_Symbol(strdup(iName));
        free(iName);
	
        (yyval.index) = A_Index((yylsp[(5) - (8)]).first_line, (tabindex_t)1, (yyvsp[(5) - (8)].list), s, (yyvsp[(7) - (8)].ival));
      }
      else {
        (yyval.index) = A_Index((yylsp[(5) - (8)]).first_line, (tabindex_t)1, (yyvsp[(5) - (8)].list), (S_symbol)0, (yyvsp[(7) - (8)].ival));
      }
    }
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 821 "adl_yacc.grm"
    {
     
      if(strcmp((yyvsp[(3) - (8)].sval), "ditto") != 0) {
        S_symbol s = NULL;
        
	char* iName = NULL;
        mkInternalName((yyvsp[(3) - (8)].sval), iName, 0);  //do not notify since this dec
					 // may not even be used
        s = S_Symbol(strdup(iName));
        free(iName);
	
        (yyval.index) = A_Index((yylsp[(5) - (8)]).first_line, (tabindex_t)1, (yyvsp[(5) - (8)].list), s, (yyvsp[(7) - (8)].ival));
      }
      else {
        (yyval.index) = A_Index((yylsp[(5) - (8)]).first_line, (tabindex_t)1, (yyvsp[(5) - (8)].list), (S_symbol)0, (yyvsp[(7) - (8)].ival));
      }
    }
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 841 "adl_yacc.grm"
    {
	(yyval.ival) = -1;
    }
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 845 "adl_yacc.grm"
    {
	(yyval.ival) = (yyvsp[(2) - (2)].ival);
    }
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 851 "adl_yacc.grm"
    {
		(yyval.sval) = (char*)0;
	}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 855 "adl_yacc.grm"
    {
		(yyval.sval) = (yyvsp[(3) - (3)].sval);
	}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 859 "adl_yacc.grm"
    {
		(yyval.sval) = ITIME_COLUMN;
	}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 863 "adl_yacc.grm"
    {
		string s((yyvsp[(5) - (5)].sval));
		s += ITIME_COLUMN;
		(yyval.sval) = (char*)s.c_str();
	}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 871 "adl_yacc.grm"
    {
      (yyval.ival) = 0;
    }
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 875 "adl_yacc.grm"
    {
      (yyval.ival) = 0;
    }
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 879 "adl_yacc.grm"
    {
      S_symbol s = NULL;
      char* iName;
      mkInternalName((yyvsp[(2) - (2)].sval), (iName), 0); //Don't notify since this 
					// may not even be used
      s = S_Symbol(strdup(iName));
      free(iName);
      
      (yyval.ival) = (int)s;
    }
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 890 "adl_yacc.grm"
    {
      (yyval.ival) = 2;
    }
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 894 "adl_yacc.grm"
    {
      (yyval.ival) = 2;
    }
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 900 "adl_yacc.grm"
    {
      (yyval.symval) = 0;
    }
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 904 "adl_yacc.grm"
    {
        S_symbol s = NULL;
        char* iName;
        char tName[256];
        mkInternalName((yyvsp[(3) - (3)].sval), (iName), 0); //Don't notify since this
                                        // may not even be used
        sprintf(tName, "table%s", iName);
        s = S_Symbol(strdup(tName));
        free(iName);

        (yyval.symval) = s;
    }
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 917 "adl_yacc.grm"
    {
      (yyval.symval) = (yyvsp[(2) - (2)].symval);
    }
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 924 "adl_yacc.grm"
    {
      if(strstr(strlwr((yyvsp[(1) - (1)].sval)), "port") == (yyvsp[(1) - (1)].sval)) {
        (yyval.symval) = S_Symbol((yyvsp[(1) - (1)].sval));
      }
      else {
        S_symbol s = NULL;
        char* iName;
        mkInternalName((yyvsp[(1) - (1)].sval), (iName), 0); //Don't notify since this 
					// may not even be used
        s = S_Symbol(strdup(iName));
        free(iName);
      
        (yyval.symval) = s;
      }
    }
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 941 "adl_yacc.grm"
    {
      (yyval.symval) = 0;
    }
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 945 "adl_yacc.grm"
    {
      (yyval.symval) = S_Symbol((yyvsp[(2) - (2)].sval));
    }
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 951 "adl_yacc.grm"
    {
      A_exp body = ((yyvsp[(10) - (11)].list))? A_SeqExp((yylsp[(10) - (11)]).first_line, (yyvsp[(10) - (11)].list)) : (A_exp)0;
      (yyval.dec) = A_Fundec((yylsp[(1) - (11)]).first_line, S_Symbol((yyvsp[(2) - (11)].sval)), (yyvsp[(4) - (11)].list), (yyvsp[(7) - (11)].ty), (yyvsp[(9) - (11)].list), body);
    }
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 956 "adl_yacc.grm"
    {
      A_exp body = ((yyvsp[(8) - (9)].list))? A_SeqExp((yylsp[(8) - (9)]).first_line, (yyvsp[(8) - (9)].list)) : (A_exp)0;
      (yyval.dec) = A_Fundec((yylsp[(1) - (9)]).first_line, S_Symbol((yyvsp[(2) - (9)].sval)), (yyvsp[(4) - (9)].list), (A_ty)0, (yyvsp[(7) - (9)].list), body);
    }
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 963 "adl_yacc.grm"
    {
        char* iName2 = NULL;
        mkInternalName((yyvsp[(2) - (2)].sval), iName2, 1);
	(yyval.sval) = strdup(iName2);
        free(iName2);
    }
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 970 "adl_yacc.grm"
    {
	(yyval.sval)= NULL;
    }
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 976 "adl_yacc.grm"
    {
     	S_symbol s3 = NULL;
          char* iName3 = NULL;
          mkInternalName((yyvsp[(3) - (7)].sval), iName3, 1);
          s3 = S_Symbol(strdup(iName3));
          free(iName3);
      if((yyvsp[(7) - (7)].sval) == NULL) {
        char* sourceName;
	prependUserNameIfESL((yyvsp[(3) - (7)].sval), sourceName);
        (yyval.dec) = A_Externdec((yylsp[(1) - (7)]).first_line, s3, (yyvsp[(5) - (7)].list), (yyvsp[(2) - (7)].ty), S_Symbol(strdup(sourceName)), 0, S_Symbol((yyvsp[(3) - (7)].sval)));
	free(sourceName);
      }
      else {
        (yyval.dec) = A_Externdec((yylsp[(1) - (7)]).first_line, S_Symbol((yyvsp[(3) - (7)].sval)), (yyvsp[(5) - (7)].list), (yyvsp[(2) - (7)].ty), S_Symbol((yyvsp[(7) - (7)].sval)));
      }
    }
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 993 "adl_yacc.grm"
    {
     	S_symbol s4 = NULL;
      	char* iName4 = NULL;
        mkInternalName((yyvsp[(4) - (8)].sval), iName4, 1);
        s4 = S_Symbol(strdup(iName4));
        free(iName4);
      if((yyvsp[(3) - (8)].ival) == 0)
      {
	if((yyvsp[(8) - (8)].sval) == NULL) {
          char* sourceName;
	  prependUserNameIfESL((yyvsp[(4) - (8)].sval), sourceName);
          (yyval.dec) = A_Externdec((yylsp[(1) - (8)]).first_line, s4, (yyvsp[(6) - (8)].list), A_NameTy((yylsp[(2) - (8)]).first_line, S_Symbol((char*)"char")), S_Symbol(strdup(sourceName)), 0, S_Symbol((yyvsp[(4) - (8)].sval)));
          free(sourceName);
        }
	else {
          (yyval.dec) = A_Externdec((yylsp[(1) - (8)]).first_line, s4, (yyvsp[(6) - (8)].list), A_NameTy((yylsp[(2) - (8)]).first_line, S_Symbol((char*)"char")), S_Symbol((yyvsp[(8) - (8)].sval)), 0, S_Symbol((yyvsp[(4) - (8)].sval)));
        }
      }
      else
      {
	if((yyvsp[(8) - (8)].sval) == NULL) {
          char* sourceName;
	  prependUserNameIfESL((yyvsp[(4) - (8)].sval), sourceName);
          (yyval.dec) = A_Externdec((yylsp[(1) - (8)]).first_line, s4, (yyvsp[(6) - (8)].list), A_NameTy((yylsp[(2) - (8)]).first_line, S_Symbol((char*)"char")), S_Symbol(strdup(sourceName)), (yyvsp[(3) - (8)].ival), S_Symbol((yyvsp[(4) - (8)].sval)));
	  free(sourceName);
        }
	else {
	   (yyval.dec) = A_Externdec((yylsp[(1) - (8)]).first_line, s4, (yyvsp[(6) - (8)].list), A_NameTy((yylsp[(2) - (8)]).first_line, S_Symbol((char*)"char")), S_Symbol((yyvsp[(8) - (8)].sval)), (yyvsp[(3) - (8)].ival), S_Symbol((yyvsp[(4) - (8)].sval)));
        }
      }
    }
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 1028 "adl_yacc.grm"
    {
      (yyval.ival) = (yyvsp[(2) - (3)].ival);
    }
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 1033 "adl_yacc.grm"
    {
      (yyval.ty) = A_NameTy((yylsp[(1) - (1)]).first_line, S_Symbol((yyvsp[(1) - (1)].sval)));
    }
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 1037 "adl_yacc.grm"
    {
      (yyval.ty) = A_RecordTy((yylsp[(1) - (4)]).first_line, (yyvsp[(3) - (4)].list));
    }
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 1042 "adl_yacc.grm"
    {
      (yyval.list) = (A_list)0;
    }
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 1046 "adl_yacc.grm"
    {
      (yyval.list) = (yyvsp[(1) - (1)].list);
    }
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 1060 "adl_yacc.grm"
    {
      (yyval.ref) = A_Ref((yylsp[(1) - (3)]).first_line, (yyvsp[(1) - (3)].ref), S_Symbol((yyvsp[(3) - (3)].sval)));
    }
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 1064 "adl_yacc.grm"
    {
      (yyval.ref) = A_Ref((yylsp[(1) - (3)]).first_line, 
		 A_SimpleVar((yylsp[(1) - (3)]).first_line, S_Symbol((yyvsp[(1) - (3)].sval))),
		 S_Symbol((yyvsp[(3) - (3)].sval)));
    }
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 1070 "adl_yacc.grm"
    {
      (yyval.ref) = A_Ref((yylsp[(1) - (5)]).first_line, 
		 A_FieldVar((yylsp[(1) - (5)]).first_line, 
			    A_SimpleVar((yylsp[(1) - (5)]).first_line, S_Symbol((yyvsp[(1) - (5)].sval))),
			    S_Symbol((yyvsp[(3) - (5)].sval))),
		 S_Symbol((yyvsp[(5) - (5)].sval)));
    }
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 1080 "adl_yacc.grm"
    {
	(yyval.exp) = (yyvsp[(1) - (8)].exp);
        if((yyval.exp)->kind == A_opExp) {
	  A_win win1 = A_Win((yylsp[(1) - (8)]).first_line, (yyvsp[(4) - (8)].list), (yyvsp[(5) - (8)].list), (yyvsp[(6) - (8)].range), (yyvsp[(7) - (8)].slide));
	  A_win win2 = A_Win((yylsp[(1) - (8)]).first_line, (yyvsp[(4) - (8)].list), (yyvsp[(5) - (8)].list), (yyvsp[(6) - (8)].range), (yyvsp[(7) - (8)].slide));
          A_SetCallExpWindow((yyval.exp)->u.op.left, win1);
          A_SetCallExpWindow((yyval.exp)->u.op.right, win2);
        }
	//all windowed aggregate names are appended with "_window"
	//but not when unlimited window
	else if((!((yyvsp[(6) - (8)].range)->type == TIME_RANGE && (yyvsp[(6) - (8)].range)->size == -1)) &&
            !((yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("sum") ||
              (yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("count") ||
              (yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("min") ||
              (yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("max") ||
              (yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("minr") ||
              (yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("maxr") ||
              (yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("sumr") ||
              (yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("avg") ||
              (yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("countr") ||
              (yyvsp[(1) - (8)].exp)->u.call.func == S_Symbol("var"))) {
	  char fixAggrName[1024];
 	  sprintf(fixAggrName, "%s_window", S_name((yyvsp[(1) - (8)].exp)->u.call.func));
	  (yyvsp[(1) - (8)].exp)->u.call.func = S_Symbol(fixAggrName);
        }
        if((yyval.exp)->kind == A_callExp) {
	  A_win win = A_Win((yylsp[(1) - (8)]).first_line, (yyvsp[(4) - (8)].list), (yyvsp[(5) - (8)].list), (yyvsp[(6) - (8)].range), (yyvsp[(7) - (8)].slide));
	  A_SetCallExpWindow((yyval.exp), win);
        }
    }
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 1114 "adl_yacc.grm"
    { 
		(yyval.exp) = (yyvsp[(1) - (1)].exp);
	}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 1119 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)(yyvsp[(3) - (4)].exp));
      
      (yyval.exp) = A_CallExp((yylsp[(2) - (4)]).first_line, 
		     S_Symbol("sum"), 
		     list,
		     (S_symbol)0);
    }
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 1129 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)(yyvsp[(3) - (4)].exp));
      
      (yyval.exp) = A_CallExp((yylsp[(2) - (4)]).first_line, 
		     S_Symbol("var"), 
		     list,
		     (S_symbol)0);
    }
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 1139 "adl_yacc.grm"
    {
      if(strcasecmp((yyvsp[(1) - (4)].sval), "sum") != 0 &&
 	 strcasecmp((yyvsp[(1) - (4)].sval), "avg") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "max") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "min") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "maxr") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "minr") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "sumr") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "countr") != 0 &&
     strcasecmp((yyvsp[(1) - (4)].sval), "count") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "oldest") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "XMLAgg") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "XMLElement") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "XMLAttributes") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "var") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "print") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "srand") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "pow") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "log") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "ceil") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "timeofday") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "rand") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "flush") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "getchar") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "ord") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "char") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "size") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "substring") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "concat") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "gettimeval") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "timetostring") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "not") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "sqrt") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "vert") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "iExtvert") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "buildiExt") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "newiExt") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "deleteiext") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "rExtvert") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "newrExt") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "deleterext") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "cExtvert") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "newcExt") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "deletecext") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "tExtvert") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "newtExt") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "deletetext") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "inttostring") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "realtostring") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "stringtoint") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "stringtoreal") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "inttoreal") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "realtoint") != 0 &&
	 strcasecmp((yyvsp[(1) - (4)].sval), "exit") != 0)
      {
        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(1) - (4)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
      
      	(yyval.exp) = A_CallExp((yylsp[(1) - (4)]).first_line, s, (yyvsp[(3) - (4)].list), (S_symbol)0);
      }
      else
	(yyval.exp) = A_CallExp((yylsp[(1) - (4)]).first_line, S_Symbol((yyvsp[(1) - (4)].sval)), (yyvsp[(3) - (4)].list), (S_symbol)0);
    }
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1204 "adl_yacc.grm"
    {
        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(1) - (6)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
      
        S_symbol s1 = NULL;
        char* iName1 = NULL;
        mkInternalName((yyvsp[(6) - (6)].sval), iName1, 1);
        s1 = S_Symbol(strdup(iName1));
        free(iName1);
      (yyval.exp) = A_CallExp((yylsp[(1) - (6)]).first_line, s, (yyvsp[(3) - (6)].list), s1);
    }
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1219 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)(yyvsp[(3) - (4)].exp));

      (yyval.exp) = A_CallExp((yylsp[(2) - (4)]).first_line, 
		     S_Symbol("max"), 
		     list, 
		     (S_symbol)0);
    }
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 1229 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)(yyvsp[(3) - (4)].exp));

      (yyval.exp) = A_CallExp((yylsp[(2) - (4)]).first_line, 
		     S_Symbol("min"), 
		     list,
		     (S_symbol)0);
    }
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1239 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);            
      appendAList(list, (void*)A_IntExp((yylsp[(2) - (4)]).first_line, 1));
                  
      (yyval.exp) = A_CallExp((yylsp[(2) - (4)]).first_line, 
		     S_Symbol("count"), 
		     list,
		     (S_symbol)0);
    }
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1249 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)(yyvsp[(3) - (4)].exp));
      
      (yyval.exp) = A_CallExp((yylsp[(2) - (4)]).first_line, 
		     S_Symbol("count"), 
		     list,
		     (S_symbol)0);
    }
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1259 "adl_yacc.grm"
    {
      A_exp sum_exp, cnt_exp;

      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)(yyvsp[(3) - (4)].exp));

      sum_exp = A_CallExp((yylsp[(2) - (4)]).first_line, 
			  S_Symbol("sumr"), 
			  list,
			  (S_symbol)0);

      list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)A_IntExp((yylsp[(2) - (4)]).first_line, 1));

      cnt_exp = A_CallExp((yylsp[(2) - (4)]).first_line,
			  S_Symbol("count"),
			  list,
			  (S_symbol)0);			  

      (yyval.exp) = A_OpExp((yylsp[(2) - (4)]).first_line, A_divideOp, sum_exp, cnt_exp);
    }
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1285 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)(yyvsp[(3) - (4)].exp));

      (yyval.exp) = A_CallExp((yylsp[(2) - (4)]).first_line, 
		     S_Symbol("XMLAgg"), 
		     list, 
		     (S_symbol)0);
    }
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 1295 "adl_yacc.grm"
    {
		A_list list=A_List((yylsp[(4) - (7)]).first_line,A_EXP);
		appendAList(list, (void*)(yyvsp[(4) - (7)].exp));
		appendListList(list, (yyvsp[(6) - (7)].list));
		(yyval.exp) = A_CallExp((yylsp[(1) - (7)]).first_line, S_Symbol("XMLElement"), list, (S_symbol)0);		
	}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1302 "adl_yacc.grm"
    {
		A_list list=A_List((yylsp[(4) - (8)]).first_line,A_EXP);
		appendAList(list, (yyvsp[(4) - (8)].exp));
		if ((yyvsp[(5) - (8)].exp)) appendAList(list, (void*)(yyvsp[(5) - (8)].exp));
		appendListList(list, (yyvsp[(7) - (8)].list));
		(yyval.exp) = A_CallExp((yylsp[(1) - (8)]).first_line, S_Symbol("XMLElement"), list, (S_symbol)0);		
	}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1310 "adl_yacc.grm"
    {
		A_list list=A_List((yylsp[(4) - (6)]).first_line, A_EXP);
		appendAList(list, (void*)(yyvsp[(4) - (6)].exp));
		if ((yyvsp[(5) - (6)].exp)) appendAList(list, (yyvsp[(5) - (6)].exp));
		(yyval.exp) = A_CallExp((yylsp[(1) - (6)]).first_line, S_Symbol("XMLElement"), list, (S_symbol)0);		
	}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1320 "adl_yacc.grm"
    { (yyval.exp)=0;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1322 "adl_yacc.grm"
    {
      	      (yyval.exp) = A_CallExp((yylsp[(1) - (5)]).first_line,
			  S_Symbol("XMLAttributes"),
			  (yyvsp[(4) - (5)].list),
			  (S_symbol)0);	
	}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1332 "adl_yacc.grm"
    {
	      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_EXP);
	      appendAList((yyval.list), (yyvsp[(1) - (1)].exp));
	      A_exp exp = A_StringExp((yylsp[(1) - (1)]).first_line, getVarNameSuffix((yyvsp[(1) - (1)].exp)->u.var)->name);
	      appendAList((yyval.list), (void*)exp);

	}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1340 "adl_yacc.grm"
    {
		appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].exp));
	      A_exp exp = A_StringExp((yylsp[(3) - (3)]).first_line, getVarNameSuffix((yyvsp[(3) - (3)].exp)->u.var)->name);	 
	      appendAList((yyvsp[(1) - (3)].list), (void*)exp);
		(yyval.list) = (yyvsp[(1) - (3)].list);
	}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1348 "adl_yacc.grm"
    {
	      (yyval.list) = A_List((yylsp[(1) - (3)]).first_line, A_EXP);
	      appendAList((yyval.list), (yyvsp[(1) - (3)].exp));
	      A_exp exp = A_StringExp((yylsp[(3) - (3)]).first_line, (yyvsp[(3) - (3)].sval));
	      appendAList((yyval.list), (void*)exp);

	}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1356 "adl_yacc.grm"
    {
		appendAList((yyvsp[(1) - (5)].list), (void*)(yyvsp[(3) - (5)].exp));
	      A_exp exp = A_StringExp((yylsp[(5) - (5)]).first_line, (yyvsp[(5) - (5)].sval));	 
	      appendAList((yyvsp[(1) - (5)].list), (void*)exp);
		(yyval.list) = (yyvsp[(1) - (5)].list);
	}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1366 "adl_yacc.grm"
    {
      (yyval.sval) = (char*)0;
    }
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1370 "adl_yacc.grm"
    { 
      (yyval.sval)=(yyvsp[(2) - (2)].sval); 
    }
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1378 "adl_yacc.grm"
    {
      (yyval.exp) = A_SqlVarExp((yylsp[(1) - (1)]).first_line, (S_symbol)0, S_Symbol((yyvsp[(1) - (1)].sval)));
    }
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1382 "adl_yacc.grm"
    {
        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(1) - (3)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
      (yyval.exp) = A_SqlVarExp((yylsp[(1) - (3)]).first_line, s, S_Symbol((yyvsp[(3) - (3)].sval)));
    }
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1391 "adl_yacc.grm"
    {
      (yyval.exp) = A_RefExp((yylsp[(1) - (1)]).first_line, (yyvsp[(1) - (1)].ref));
    }
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1395 "adl_yacc.grm"
    {
      (yyval.exp) = A_NilExp((yylsp[(1) - (1)]).first_line);
    }
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1399 "adl_yacc.grm"
    {
      (yyval.exp) = A_IntExp((yylsp[(1) - (1)]).first_line, (yyvsp[(1) - (1)].ival));
    }
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1403 "adl_yacc.grm"
    {
      (yyval.exp) = A_RealExp((yylsp[(1) - (1)]).first_line, (yyvsp[(1) - (1)].rval));
    }
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1407 "adl_yacc.grm"
    {
      (yyval.exp) = A_IntExp((yylsp[(1) - (2)]).first_line, (yyvsp[(1) - (2)].ival)*(yyvsp[(2) - (2)].ival));
    }
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1411 "adl_yacc.grm"
    {
      (yyval.exp) = A_RealExp((yylsp[(1) - (2)]).first_line, (yyvsp[(1) - (2)].rval)*(yyvsp[(2) - (2)].ival));
    }
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1415 "adl_yacc.grm"
    {
      (yyval.exp) = A_StringExp((yylsp[(1) - (1)]).first_line, (yyvsp[(1) - (1)].sval));
    }
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1419 "adl_yacc.grm"
    {
      (yyval.exp) = A_TimestampExp((yylsp[(1) - (2)]).first_line, (yyvsp[(2) - (2)].sval));
    }
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1423 "adl_yacc.grm"
    {
      A_exp v;
      if((yyvsp[(4) - (4)].sval) == (char*)NULL) {
        v = A_VarExp((yylsp[(1) - (4)]).first_line, A_FieldVar((yylsp[(1) - (4)]).first_line, 
			   A_SimpleVar((yylsp[(3) - (4)]).first_line, S_Symbol("inwindow")),
			   (S_symbol)0));
      }
      else {
        v = A_VarExp((yylsp[(1) - (4)]).first_line, A_FieldVar((yylsp[(1) - (4)]).first_line, 
			   A_SimpleVar((yylsp[(3) - (4)]).first_line, S_Symbol("inwindow")),
			   S_Symbol((yyvsp[(4) - (4)].sval))));
      }


      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)v);

      (yyval.exp) = A_CallExp((yylsp[(2) - (4)]).first_line, 
		     S_Symbol("oldest"), 
		     list, 
		     (S_symbol)0);
    }
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1446 "adl_yacc.grm"
    {	
      A_list list = A_List((yylsp[(3) - (4)]).first_line, A_EXP);
      appendAList(list, (void*)(yyvsp[(3) - (4)].exp));

      (yyval.exp) = A_CallExp((yylsp[(1) - (4)]).first_line, S_Symbol("sqrt"), list, (S_symbol)0);
    }
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1453 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1457 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_plusOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1461 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_minusOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1465 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_timesOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1469 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_divideOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1473 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_modOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1477 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (2)]).first_line, A_minuseqOp, (yyvsp[(1) - (2)].exp), 0);
    }
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1481 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_minuseqOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1485 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (2)]).first_line, A_pluseqOp, (yyvsp[(1) - (2)].exp), 0);
    }
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1489 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_pluseqOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1493 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_eqOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1497 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_neqOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1501 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_ltOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1505 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_leOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1509 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_gtOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1513 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_geOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1517 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_andOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1521 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_orOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1525 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_likeOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1529 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (4)]).first_line, A_nlikeOp, (yyvsp[(1) - (4)].exp), (yyvsp[(4) - (4)].exp));
    }
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1533 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_inOp, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp));
    }
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1537 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (4)]).first_line, A_ninOp, (yyvsp[(1) - (4)].exp), (yyvsp[(4) - (4)].exp));
    }
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1541 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_isnullOp, (yyvsp[(1) - (3)].exp), 0);
    }
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1545 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (4)]).first_line, A_isnullOp, (yyvsp[(1) - (4)].exp), 0);
    }
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1549 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (2)]).first_line, A_existOp, (yyvsp[(2) - (2)].exp), 0);
    }
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1553 "adl_yacc.grm"
    {
      (yyval.exp) = A_OpExp((yylsp[(1) - (3)]).first_line, A_notexistOp, (yyvsp[(3) - (3)].exp), 0);
    }
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1557 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(2) - (3)].exp);
    }
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1561 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(2) - (3)].exp);
    }
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1565 "adl_yacc.grm"
    {
      (yyval.exp) = A_LetExp((yylsp[(1) - (4)]).first_line, (yyvsp[(2) - (4)].list), A_SeqExp((yylsp[(3) - (4)]).first_line, (yyvsp[(3) - (4)].list)));
    }
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1569 "adl_yacc.grm"
    {
	(yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1576 "adl_yacc.grm"
    {
      (yyval.exp) = A_CaseExp((yylsp[(1) - (6)]).first_line, (yyvsp[(2) - (6)].exp), (yyvsp[(3) - (6)].list), (yyvsp[(5) - (6)].exp));
    }
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1580 "adl_yacc.grm"
    {
      (yyval.exp) = A_CaseExp((yylsp[(1) - (5)]).first_line, (A_exp)0, (yyvsp[(2) - (5)].list), (yyvsp[(4) - (5)].exp));
    }
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1584 "adl_yacc.grm"
    {
      (yyval.exp) = A_CaseExp((yylsp[(1) - (4)]).first_line, (yyvsp[(2) - (4)].exp), (yyvsp[(3) - (4)].list), (A_exp)0);
    }
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1588 "adl_yacc.grm"
    {
      (yyval.exp) = A_CaseExp((yylsp[(1) - (3)]).first_line, (A_exp)0, (yyvsp[(2) - (3)].list), (A_exp)0);
    }
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1594 "adl_yacc.grm"
    {
	(yyval.list) = (A_list)0;
    }
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1598 "adl_yacc.grm"
    {
	(yyval.list) = (yyvsp[(1) - (1)].list);
    }
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1604 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_EXP);
      appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].exp));
    }
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1609 "adl_yacc.grm"
    {
      appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].exp));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1616 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (4)]).first_line, A_EXP);
      appendAList((yyval.list), (void*)(yyvsp[(2) - (4)].exp));
      appendAList((yyval.list), (void*)(yyvsp[(4) - (4)].exp));
    }
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1622 "adl_yacc.grm"
    {
      appendAList((yyvsp[(1) - (5)].list), (void*)(yyvsp[(3) - (5)].exp));
      appendAList((yyvsp[(1) - (5)].list), (void*)(yyvsp[(5) - (5)].exp));
      (yyval.list) = (yyvsp[(1) - (5)].list);
    }
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1643 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(1) - (1)]).first_line, A_FIELD);
      A_field field = A_Field((yylsp[(1) - (1)]).first_line,
			      S_Symbol("field_0"), S_Symbol((yyvsp[(1) - (1)].sval))); 
      appendAList(list, (void*)field);

      (yyval.ty) = A_RecordTy((yylsp[(1) - (1)]).first_line, list);
    }
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1652 "adl_yacc.grm"
    {
      (yyval.ty) = A_RecordTy((yylsp[(1) - (3)]).first_line, (yyvsp[(2) - (3)].list));
    }
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1658 "adl_yacc.grm"
    {
    A_ty ty = A_RecordTy((yylsp[(3) - (6)]).first_line, (yyvsp[(3) - (6)].list));
    /*A_list keys = A_List(@1.first_line, A_SYMBOL);
    appendAList(keys, (void*)S_Symbol($7));*/
    //int index= 0; /* B_TREE */

    //if ($5) {
    //  popElementList($5); //we only support btree on memory tabs, as for now, Hetal
    //}

    (yyval.dec) = A_TabVarDec((yylsp[(2) - (6)]).first_line, 
        	     S_Symbol("inwindow"),	// name of the table        
		       ty,		// tuple type               
		       //index,		// btree, rtree
                       (yyvsp[(5) - (6)].index),
                       1,
                       (A_exp)0
		       ); 
  }
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1678 "adl_yacc.grm"
    {
    (yyval.dec)=(A_dec)0;
  }
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1685 "adl_yacc.grm"
    {
      (yyval.list) = (A_list)(0);
    }
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1689 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(1) - (1)]).first_line, A_FLOW);

      appendElementList(list, (nt_obj_t*)(yyvsp[(1) - (1)].flow));

      (yyval.list) = list;
    }
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1697 "adl_yacc.grm"
    {
      appendElementList((yyvsp[(1) - (3)].list), (nt_obj_t*)(yyvsp[(3) - (3)].flow));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1705 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(1) - (1)]).first_line, A_MODELITEM);

      appendElementList(list, (nt_obj_t*)(yyvsp[(1) - (1)].modelitem));

      (yyval.list) = list;
    }
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1713 "adl_yacc.grm"
    {
      appendElementList((yyvsp[(1) - (3)].list), (nt_obj_t*)(yyvsp[(3) - (3)].modelitem));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1721 "adl_yacc.grm"
    {
      appendElementList((yyvsp[(1) - (3)].list), (nt_obj_t*)S_Symbol((yyvsp[(3) - (3)].sval)));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1726 "adl_yacc.grm"
    {
      A_list list = A_List((yylsp[(1) - (1)]).first_line, A_SYMBOL);
      appendElementList(list, (nt_obj_t*)S_Symbol((yyvsp[(1) - (1)].sval)));
      (yyval.list) = list;
    }
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1735 "adl_yacc.grm"
    {
      (yyval.ival) = 1;
    }
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1739 "adl_yacc.grm"
    {
      (yyval.ival) = 0;
    }
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1746 "adl_yacc.grm"
    {
      (yyval.flow) = A_Flow((yylsp[(1) - (5)]).first_line, S_Symbol(strdup((yyvsp[(2) - (5)].sval))), (yyvsp[(4) - (5)].list));
    }
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1757 "adl_yacc.grm"
    {
      S_symbol s = NULL;
      char* iName = NULL;
      mkInternalName((yyvsp[(4) - (18)].sval), iName, 1);
      s = S_Symbol(strdup(iName));
      free(iName);

      (yyval.modelitem) = A_ModelItem((yylsp[(1) - (18)]).first_line, S_Symbol(strdup((yyvsp[(1) - (18)].sval))), 
                       s, (yyvsp[(7) - (18)].ival), (yyvsp[(11) - (18)].list), (yyvsp[(16) - (18)].list));
    }
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1773 "adl_yacc.grm"
    {
      (yyval.list) = (yyvsp[(3) - (4)].list);
    }
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1788 "adl_yacc.grm"
    {in_modeltype = 1; setModelName((yyvsp[(2) - (7)].sval));}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1791 "adl_yacc.grm"
    {
      in_modeltype = 0;
      setModelName("");
      S_symbol s = NULL;
      char* iName = NULL;
      mkInternalName((yyvsp[(2) - (10)].sval), iName, 1);
      s = S_Symbol(strdup(iName));
      free(iName);

      (yyval.dec) = A_ModelTypeDec((yylsp[(1) - (10)]).first_line, s, (yyvsp[(4) - (10)].list), (yyvsp[(6) - (10)].list), (yyvsp[(9) - (10)].list));
    }
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1803 "adl_yacc.grm"
    {
      S_symbol s = NULL;
      char* iName = NULL;
      mkInternalName((yyvsp[(2) - (4)].sval), iName, 1);
      s = S_Symbol(strdup(iName));
      free(iName);

      S_symbol s1 = NULL;
      char* iName1 = NULL;
      mkInternalName((yyvsp[(4) - (4)].sval), iName1, 1);
      s1 = S_Symbol(strdup(iName1));
      free(iName1);

      (yyval.dec) = A_ModelTypeDec((yylsp[(1) - (4)]).first_line, s, s1);
    }
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1830 "adl_yacc.grm"
    {
        S_symbol s = NULL;
        char* iName = NULL;
        char fixAggrName[1024];
        
        sprintf(fixAggrName, "%s_window", (yyvsp[(3) - (16)].sval));
        mkInternalName(fixAggrName, iName, 0);  //Don't notify since the aggr may not
                                         //actually be used
        s = S_Symbol(strdup(iName));
        free(iName);
        
	(yyval.dec) = A_Aggrdec((yylsp[(1) - (16)]).first_line,
                       A_window,
		       s, 
		       (yyvsp[(5) - (16)].list), 
		       (yyvsp[(8) - (16)].ty), /* return type */
		       (yyvsp[(11) - (16)].list),
		       (yyvsp[(12) - (16)].exp),
		       (yyvsp[(13) - (16)].exp),
		       (yyvsp[(14) - (16)].exp),
		       (yyvsp[(15) - (16)].exp),
                       (yyvsp[(10) - (16)].dec));

    }
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1861 "adl_yacc.grm"
    {
        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(2) - (13)].sval), iName, 0);  //Don't notify since the aggr may not
                                         //actually be used
        s = S_Symbol(strdup(iName));
        free(iName);
        
	(yyval.dec) = A_Aggrdec((yylsp[(1) - (13)]).first_line, 
                       A_simple,
		       s, 
		       (yyvsp[(4) - (13)].list), 
		       (yyvsp[(7) - (13)].ty), /* return type */
		       (yyvsp[(9) - (13)].list),
		       (yyvsp[(10) - (13)].exp),
		       (yyvsp[(11) - (13)].exp),
		       (A_exp)0,
		       (yyvsp[(12) - (13)].exp),
                       (A_dec)0);
    }
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1883 "adl_yacc.grm"
    {
        S_symbol s = NULL;
	char* iName = NULL;
        mkInternalName((yyvsp[(2) - (11)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
        A_exp exp = A_SeqExp((yylsp[(10) - (11)]).first_line, (yyvsp[(10) - (11)].list));

	(yyval.dec) = A_Aggrdec((yylsp[(1) - (11)]).first_line, 
		       A_simple,
		       s, 
		       (yyvsp[(4) - (11)].list), 
		       (yyvsp[(7) - (11)].ty), 
		       (yyvsp[(9) - (11)].list),
		       exp,
		       (A_exp)0,
		       (A_exp)0,
		       (A_exp)0,
                       (A_dec)0
		       );
    }
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1915 "adl_yacc.grm"
    {
        S_symbol s = NULL;
        char* iName = NULL;
        char fixAggrName[1024];

        sprintf(fixAggrName, "%s_window", (yyvsp[(3) - (18)].sval));
        mkInternalName(fixAggrName, iName, 0);  //Don't notify since the aggr may not
                                         //actually be used
        s = S_Symbol(strdup(iName));
        free(iName);

        (yyval.dec) = A_Aggrdec((yylsp[(1) - (18)]).first_line,
                       A_window,
                       s,
                       (yyvsp[(5) - (18)].list),
                       (yyvsp[(8) - (18)].ty), /* return type */
                       (yyvsp[(10) - (18)].sval), /* glocal c code */
		       (yyvsp[(12) - (18)].list),
                       (yyvsp[(13) - (18)].sval), /* c decs */
                       (yyvsp[(14) - (18)].sval), /* init */
                       (yyvsp[(15) - (18)].sval), /* iterate */
                       (yyvsp[(16) - (18)].sval), /* expire */
                       (yyvsp[(17) - (18)].sval), /* terminate */
                       (yyvsp[(11) - (18)].dec));
    }
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1949 "adl_yacc.grm"
    {
        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(2) - (15)].sval), iName, 0);  //Don't notify since the aggr may not
                                         //actually be used
        s = S_Symbol(strdup(iName));
        free(iName);

        (yyval.dec) = A_Aggrdec((yylsp[(1) - (15)]).first_line,
                       A_simple,
                       s,
                       (yyvsp[(4) - (15)].list),
                       (yyvsp[(7) - (15)].ty), /* return type */
		       (yyvsp[(9) - (15)].sval), /* global c code */ 
                       (yyvsp[(10) - (15)].list),
                       (yyvsp[(11) - (15)].sval), /* c decs */
                       (yyvsp[(12) - (15)].sval), /* init */
                       (yyvsp[(13) - (15)].sval), /* iterate */
                       (char*)0, /* expire */
                       (yyvsp[(14) - (15)].sval), /* termiante */
                       (A_dec)0);
    }
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1974 "adl_yacc.grm"
    {
      (yyval.sval) = NULL;
    }
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1978 "adl_yacc.grm"
    {
      (yyval.sval) = strdup((yyvsp[(4) - (5)].sval));
    }
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1982 "adl_yacc.grm"
    {
      (yyval.sval) = (char*)1;            // share with ITERATE
    }
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1988 "adl_yacc.grm"
    {
      (yyval.sval) = NULL;
    }
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1992 "adl_yacc.grm"
    {
      (yyval.sval) = strdup((yyvsp[(4) - (5)].sval));
    }
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1999 "adl_yacc.grm"
    {
      (yyval.sval) = NULL;
    }
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 2003 "adl_yacc.grm"
    {
      (yyval.sval) = strdup((yyvsp[(4) - (5)].sval));
    }
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 2010 "adl_yacc.grm"
    {
      (yyval.sval) = NULL;
    }
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 2014 "adl_yacc.grm"
    {
      (yyval.sval) = strdup((yyvsp[(4) - (5)].sval));
    }
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 2020 "adl_yacc.grm"
    {
      (yyval.exp) = (A_exp)0;
    }
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 2024 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(3) - (3)].exp);
    }
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 2035 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(3) - (3)].exp);
    }
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 2039 "adl_yacc.grm"
    {
      (yyval.exp) = (A_exp)1;		// share with ITERATE
    }
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 2045 "adl_yacc.grm"
    {
      (yyval.exp) = (A_exp)0;
    }
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 2049 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(3) - (3)].exp);
    }
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 2053 "adl_yacc.grm"
    {
      (yyval.exp) = (A_exp)1;		// share with TERMINATE
    }
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 2059 "adl_yacc.grm"
    {
      (yyval.exp) = (A_exp)0;
    }
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 2063 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(3) - (3)].exp);
    }
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 2070 "adl_yacc.grm"
    { 
      in_sql = 0;
      (yyval.exp) = (yyvsp[(1) - (1)].exp); 
    }
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 2075 "adl_yacc.grm"
    { 
      in_sql = 0;
      (yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 2080 "adl_yacc.grm"
    { 
      in_sql = 0;
      (yyval.exp) = (yyvsp[(1) - (1)].exp); 
    }
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 2085 "adl_yacc.grm"
    { 
      in_sql = 0;
      (yyval.exp) = (yyvsp[(1) - (1)].exp); 
    }
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 2090 "adl_yacc.grm"
    { 
      in_sql = 0;
      (yyval.exp) = (yyvsp[(1) - (1)].exp); 
    }
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 2095 "adl_yacc.grm"
    { 
      in_sql = 0;
      (yyval.exp) = (yyvsp[(1) - (1)].exp); 
    }
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 2100 "adl_yacc.grm"
    {
	in_sql = 0;
	(yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 2105 "adl_yacc.grm"
    {
	in_sql = 0;
	(yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 2110 "adl_yacc.grm"
    {
	in_sql = 0;
	(yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 2115 "adl_yacc.grm"
    {
	in_sql = 0;
	(yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 2128 "adl_yacc.grm"
    {
      if (isValuesSqlOpr((yyvsp[(2) - (2)].exp))) {
	 (yyval.exp) = (yyvsp[(2) - (2)].exp);
      } else {
	/* recast it to INSERT INTO STDOUT query */

	A_list jtl = A_List((yylsp[(2) - (2)]).first_line, A_QUN);
	S_symbol std = S_Symbol("stdout");
	appendAList(jtl, (void*)A_NameQun((yylsp[(2) - (2)]).first_line, std, std));
	appendAList(jtl, (void*)A_QueryQun((yylsp[(2) - (2)]).first_line, 0, (yyvsp[(2) - (2)].exp)));

	(yyval.exp) = A_SqlOprExp((yylsp[(1) - (2)]).first_line,
			 A_SQL_INSERT, 
			 0,
			 (A_list)0,
			 jtl,
			 (A_list)0);
      }
    }
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 2151 "adl_yacc.grm"
    { (yyval.list) = (A_list)0; }
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 2152 "adl_yacc.grm"
    { in_sql = 1; }
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 2153 "adl_yacc.grm"
    { (yyval.list) = (yyvsp[(3) - (3)].list); }
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 2158 "adl_yacc.grm"
    {
      A_list jtl = A_List((yylsp[(1) - (4)]).first_line, A_QUN);
      appendAList(jtl, (void*)A_QueryQun((yylsp[(1) - (4)]).first_line, 0, (yyvsp[(1) - (4)].exp)));
      appendAList(jtl, (void*)A_QueryQun((yylsp[(4) - (4)]).first_line, 0, (yyvsp[(4) - (4)].exp)));

      (yyval.exp) = A_SqlOprExp((yylsp[(1) - (4)]).first_line, 
                       (sqlopr_t)(yyvsp[(2) - (4)].ival),		/* set oper */
                       (yyvsp[(3) - (4)].ival),			/* distinct */
		       (A_list)0,		/* hxp_list */
                       jtl,			/* join table list */
		       (A_list)0		/* wr_prd_list */
		       ); 	
    }
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 2172 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 2179 "adl_yacc.grm"
    { (yyval.list) = (A_list)0; }
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 2181 "adl_yacc.grm"
    { (yyval.list) = (yyvsp[(3) - (3)].list); 
    }
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 2187 "adl_yacc.grm"
    {
      A_orderitem oi = A_OrderItem((yylsp[(3) - (4)]).first_line, (yyvsp[(3) - (4)].exp), (yyvsp[(4) - (4)].ival));
      appendAList((yyvsp[(1) - (4)].list), (void*)oi);
      (yyval.list) = (yyvsp[(1) - (4)].list);
    }
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 2193 "adl_yacc.grm"
    {
      A_orderitem oi = A_OrderItem((yylsp[(1) - (2)]).first_line, (yyvsp[(1) - (2)].exp), (yyvsp[(2) - (2)].ival));
      (yyval.list) = A_List((yylsp[(1) - (2)]).first_line, A_ORDERITEM);
      appendAList((yyval.list), (void*)oi);
    }
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 2201 "adl_yacc.grm"
    { (yyval.ival) =  SQL_ASC; }
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 2202 "adl_yacc.grm"
    { (yyval.ival) =  SQL_ASC; }
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 2203 "adl_yacc.grm"
    { (yyval.ival) =  SQL_DESC; }
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 2208 "adl_yacc.grm"
    {	
      appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].dec));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 2213 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_DEC);
      appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].dec));
    }
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 2221 "adl_yacc.grm"
    {
      A_ty ty = A_RecordTy((yylsp[(2) - (8)]).first_line, (yyvsp[(3) - (8)].list));
      (yyval.dec) = A_TabVarDec((yylsp[(1) - (8)]).first_line,            
		       S_Symbol((yyvsp[(1) - (8)].sval)),           // name of the table        
		       ty, 		       // tuple type               
		       (A_index)0,	       // key declarations	    
		       0, 		       // global, local, memory    
		       (yyvsp[(7) - (8)].exp)		       // init
		       );
    }
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 2234 "adl_yacc.grm"
    {
      (yyval.list) = (A_list)0; 
    }
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 2237 "adl_yacc.grm"
    {in_sql = 1;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 2238 "adl_yacc.grm"
    {
      (yyval.list)= (yyvsp[(4) - (5)].list);
    }
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 2244 "adl_yacc.grm"
    {
      (yyval.win) = (A_win)0;
    }
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 2248 "adl_yacc.grm"
    {
      (yyval.win) = A_Win((yylsp[(1) - (7)]).first_line, (yyvsp[(3) - (7)].list), (yyvsp[(4) - (7)].list), (yyvsp[(5) - (7)].range), (yyvsp[(6) - (7)].slide));
    }
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 2253 "adl_yacc.grm"
    {in_sql = 1;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 2254 "adl_yacc.grm"
    {
      S_symbol s = NULL;
      if((yyvsp[(4) - (8)].sval) != NULL) {
        char* iName;
        mkInternalName((yyvsp[(3) - (8)].sval), (iName), 0); //Don't notify since this
                                        // dec may not even be used
        s = S_Symbol(strdup(iName));
        free(iName);
      }

      S_symbol s1 = NULL;
      char* iName1;
      mkInternalName((yyvsp[(6) - (8)].sval), (iName1), 0); //Don't notify since this
                                        // dec may not even be used
      s1 = S_Symbol(strdup(iName1));
      free(iName1);

      if((yyvsp[(4) - (8)].sval) == NULL)
        (yyval.exp) = A_Runtask((yylsp[(1) - (8)]).first_line, s, S_Symbol((yyvsp[(3) - (8)].sval)), s1, (yyvsp[(7) - (8)].win), (yyvsp[(8) - (8)].list));
      else
        (yyval.exp) = A_Runtask((yylsp[(1) - (8)]).first_line, s, S_Symbol((yyvsp[(4) - (8)].sval)), s1, (yyvsp[(7) - (8)].win), (yyvsp[(8) - (8)].list));
    }
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 2279 "adl_yacc.grm"
    { in_sql = 1; }
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 2285 "adl_yacc.grm"
    {
      (yyval.exp) = A_Select((yylsp[(1) - (9)]).first_line,
		    (yyvsp[(3) - (9)].ival),		/* distinct */
		    (yyvsp[(4) - (9)].list),		/* select_list */
		    (yyvsp[(5) - (9)].list),		/* qun_list */
		    (yyvsp[(6) - (9)].exp),		/* where cond */
		    (yyvsp[(7) - (9)].list),		/* group by */
		    (yyvsp[(8) - (9)].exp),		/* having cond */
		    (yyvsp[(9) - (9)].list)		/* order by */
		    );
    }
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 2296 "adl_yacc.grm"
    { in_sql = 1; }
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 2297 "adl_yacc.grm"
    {
      if ((yyvsp[(4) - (4)].exp)) {
	A_list wr_prd_list = A_List((yylsp[(4) - (4)]).first_line, A_EXP);
	decomposeBoolExpr((yyvsp[(4) - (4)].exp), wr_prd_list);
	(yyvsp[(3) - (4)].exp)->u.sqlopr->prd_list = wr_prd_list;
      }
      (yyval.exp) = (yyvsp[(3) - (4)].exp);
    }
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 2306 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 2312 "adl_yacc.grm"
    {
      (yyval.exp) = (yyvsp[(1) - (1)].exp);
    }
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 2316 "adl_yacc.grm"
    {
      if ((yyvsp[(1) - (3)].exp)->u.sqlopr->kind == A_SQL_SEL) {
	A_list jtl = A_List((yylsp[(1) - (3)]).first_line, A_QUN);
	appendAList(jtl, (void*)A_QueryQun((yylsp[(1) - (3)]).first_line, 0, (yyvsp[(1) - (3)].exp)));
	appendAList(jtl, (void*)A_QueryQun((yylsp[(3) - (3)]).first_line, 0, (yyvsp[(3) - (3)].exp)));
	(yyval.exp) = A_SqlOprExp((yylsp[(1) - (3)]).first_line, 
			 A_SQL_UNION,		/* set oper */
			 0,			/* distinct */
			 (A_list)0,		/* hxp_list */
			 jtl,			/* join table list */
			 (A_list)0		/* wr_prd_list */
			 ); 	
      } else {
	// $1 is a UNION opertor
	appendAList((yyvsp[(1) - (3)].exp)->u.sqlopr->jtl_list, 
		    (void*)A_QueryQun((yylsp[(3) - (3)]).first_line, 0, (yyvsp[(3) - (3)].exp)));
	(yyval.exp) = (yyvsp[(1) - (3)].exp);
      }
    }
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 2338 "adl_yacc.grm"
    {
      (yyval.exp) = A_SqlOprExp((yylsp[(1) - (3)]).first_line,
		       A_SQL_SEL,	/* opr */
		       0,		/* distinct */
		       (yyvsp[(2) - (3)].list),		/* hxp_list */
		       (A_list)0,	/* qun_list */
		       (A_list)0	/* prd */
		       );
    }
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 2351 "adl_yacc.grm"
    { (yyval.ival) = (int)A_SQL_UNION; }
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 2353 "adl_yacc.grm"
    { (yyval.ival) = (int)A_SQL_INTERSECT; }
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 2355 "adl_yacc.grm"
    { (yyval.ival) = (int)A_SQL_EXCEPT; }
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 2360 "adl_yacc.grm"
    { (yyval.ival) = 1; }
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 2362 "adl_yacc.grm"
    { (yyval.ival) = 0; }
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 2367 "adl_yacc.grm"
    { (yyval.ival) = 0; }
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 2369 "adl_yacc.grm"
    { (yyval.ival) = 0; }
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 2371 "adl_yacc.grm"
    { (yyval.ival) = 1; }
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 2376 "adl_yacc.grm"
    { 
      appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].selectitem));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 2381 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_SELECT_ITEM);
      appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].selectitem));
    }
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 2389 "adl_yacc.grm"
    {
      A_exp v = A_VarExp((yylsp[(1) - (7)]).first_line, A_FieldVar((yylsp[(1) - (7)]).first_line, 
			   A_SimpleVar((yylsp[(3) - (7)]).first_line, S_Symbol((yyvsp[(3) - (7)].sval))),
			   S_Symbol((yyvsp[(5) - (7)].sval))));

      A_list list = A_List((yylsp[(3) - (7)]).first_line, A_EXP);
      appendAList(list, (void*)v);

      A_exp e = A_CallExp((yylsp[(2) - (7)]).first_line, 
		     S_Symbol("oldest"), 
		     list, 
		     (S_symbol)0);

      if ((yyvsp[(7) - (7)].sval) == (char *)0) 
	(yyval.selectitem) = A_SelectItem((yylsp[(1) - (7)]).first_line, e, (S_symbol)0);
      else 
	(yyval.selectitem) = A_SelectItem((yylsp[(1) - (7)]).first_line, e, S_Symbol((yyvsp[(7) - (7)].sval)));
    }
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 2408 "adl_yacc.grm"
    {
      (yyval.selectitem) = A_SelectItemComplex((yylsp[(1) - (5)]).first_line, (yyvsp[(1) - (5)].exp), (yyvsp[(4) - (5)].list));
    }
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 2412 "adl_yacc.grm"
    {
      if ((yyvsp[(2) - (2)].sval) == (char *)0) 
	(yyval.selectitem) = A_SelectItem((yylsp[(1) - (2)]).first_line, (yyvsp[(1) - (2)].exp), (S_symbol)0);
      else 
	(yyval.selectitem) = A_SelectItem((yylsp[(1) - (2)]).first_line, (yyvsp[(1) - (2)].exp), S_Symbol((yyvsp[(2) - (2)].sval)));
    }
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 2419 "adl_yacc.grm"
    { 
      if ((yyvsp[(2) - (2)].sval) == (char *)0) 
	(yyval.selectitem) = A_SelectItem((yylsp[(1) - (2)]).first_line, (yyvsp[(1) - (2)].exp), (S_symbol)0);
      else 
	(yyval.selectitem) = A_SelectItem((yylsp[(1) - (2)]).first_line, (yyvsp[(1) - (2)].exp), S_Symbol((yyvsp[(2) - (2)].sval)));
    }
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 2426 "adl_yacc.grm"
    { 
      (yyval.selectitem) = A_SelectItemStar((yylsp[(1) - (3)]).first_line, S_Symbol((yyvsp[(1) - (3)].sval)));
    }
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 2430 "adl_yacc.grm"
    {
      (yyval.selectitem) = A_SelectItemStar((yylsp[(1) - (1)]).first_line, (S_symbol)0);
    }
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 2437 "adl_yacc.grm"
    { 
      appendAList((yyvsp[(1) - (3)].list), (void*)S_Symbol((yyvsp[(3) - (3)].sval)));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 2442 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_SYMBOL);
      appendAList((yyval.list), (void*)S_Symbol((yyvsp[(1) - (1)].sval)));
    }
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 2449 "adl_yacc.grm"
    { (yyval.sval)=(char*)0;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 2450 "adl_yacc.grm"
    { (yyval.sval)=(yyvsp[(1) - (1)].sval); }
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 2451 "adl_yacc.grm"
    { (yyval.sval)=(yyvsp[(2) - (2)].sval);}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 2456 "adl_yacc.grm"
    { 
      appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].qun));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 2461 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_QUN);
      appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].qun));
    }
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 2468 "adl_yacc.grm"
    { 
        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(1) - (1)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
       (yyval.sval)=S_name(s); 
    }
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 2477 "adl_yacc.grm"
    { 
        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(2) - (2)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
       (yyval.sval)=S_name(s);
    }
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 2506 "adl_yacc.grm"
    { (yyval.list) = (A_list)0;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 2508 "adl_yacc.grm"
    { (yyval.list) = (yyvsp[(3) - (3)].list);}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 2512 "adl_yacc.grm"
    {
      (yyval.range) = (yyvsp[(1) - (1)].range);
    }
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 2516 "adl_yacc.grm"
    {
      (yyval.range) = (yyvsp[(1) - (1)].range);
    }
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 2522 "adl_yacc.grm"
    {
	(yyval.range) = A_Range((yylsp[(1) - (3)]).first_line, (yyvsp[(2) - (3)].ival)+1, COUNT_RANGE);
    }
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 2526 "adl_yacc.grm"
    {
	(yyval.range) = A_Range((yylsp[(1) - (3)]).first_line, (yyvsp[(2) - (3)].ival), TIME_RANGE);
    }
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 2530 "adl_yacc.grm"
    {
	(yyval.range) = A_Range((yylsp[(1) - (4)]).first_line, (yyvsp[(2) - (4)].ival)*(yyvsp[(3) - (4)].ival)*1000, TIME_RANGE);
    }
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 2536 "adl_yacc.grm"
    {
        (yyval.range) = A_Range((yylsp[(1) - (3)]).first_line, -1, TIME_RANGE);
    }
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 2542 "adl_yacc.grm"
    {
	  (yyval.slide) = A_Slide((yylsp[(1) - (2)]).first_line, (yyvsp[(2) - (2)].ival), COUNT_SLIDE);
	}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 2546 "adl_yacc.grm"
    {
	  (yyval.slide) = A_Slide((yylsp[(1) - (3)]).first_line, (yyvsp[(2) - (3)].ival)*(yyvsp[(3) - (3)].ival)*1000, TIME_SLIDE);
	}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 2550 "adl_yacc.grm"
    {
	  (yyval.slide) = A_Slide((yylsp[(1) - (4)]).first_line, (yyvsp[(2) - (4)].ival)*(yyvsp[(3) - (4)].ival)*1000, HEARTBEAT_TIME_SLIDE);
	}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 2554 "adl_yacc.grm"
    {
		(yyval.slide) = A_Slide((yyloc).first_line, 1, COUNT_SLIDE);
	}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 2561 "adl_yacc.grm"
    { (yyval.ival) = 1;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 2563 "adl_yacc.grm"
    { (yyval.ival) = 60;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 2565 "adl_yacc.grm"
    { (yyval.ival) = 3600;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 2567 "adl_yacc.grm"
    { (yyval.ival) = 86400;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 2573 "adl_yacc.grm"
    {
	(yyval.ival) = (int)S_Symbol("inwindow");
    }
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 2577 "adl_yacc.grm"
    {
        S_symbol s = NULL;
	char* iName = NULL;
        mkInternalName((yyvsp[(1) - (1)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
        (yyval.ival) = (int)s;
    }
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 2589 "adl_yacc.grm"
    {
      (yyval.qun) = A_NameQun((yylsp[(1) - (1)]).first_line, (S_symbol)((yyvsp[(1) - (1)].ival)), (S_symbol)((yyvsp[(1) - (1)].ival)));
    }
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 2598 "adl_yacc.grm"
    { 
      (yyval.qun) = (yyvsp[(1) - (1)].qun);
    }
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 2602 "adl_yacc.grm"
    {
      (yyval.qun) = A_NameQun((yylsp[(1) - (2)]).first_line, (S_symbol)((yyvsp[(1) - (2)].ival)), S_Symbol((yyvsp[(2) - (2)].sval)));
    }
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 2606 "adl_yacc.grm"
    {
        S_symbol s1 = NULL;
        char* iName1 = NULL;
        mkInternalName((yyvsp[(1) - (6)].sval), iName1, 1);
        s1 = S_Symbol(strdup(iName1));
        free(iName1);
        S_symbol s5 = NULL;
	if(strcasecmp((yyvsp[(5) - (6)].sval), "current") == 0) {
	  s5 = s1;
        }
        else {
          char* iName5 = NULL;
          mkInternalName((yyvsp[(5) - (6)].sval), iName5, 1);
          s5 = S_Symbol(strdup(iName5));
          free(iName5);
        }
        (yyval.qun) = A_WindowQun((yylsp[(1) - (6)]).first_line, s1, (yyvsp[(4) - (6)].range), s5, s1);
    }
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 2625 "adl_yacc.grm"
    {
        S_symbol s3 = NULL;
        char* iName3 = NULL;
        mkInternalName((yyvsp[(3) - (10)].sval), iName3, 1);
        s3 = S_Symbol(strdup(iName3));
        free(iName3);
        S_symbol s7 = NULL;
	if(strcasecmp((yyvsp[(7) - (10)].sval), "current") == 0) {
	  s7 = s3;
        }
        else {
          char* iName7 = NULL;
          mkInternalName((yyvsp[(7) - (10)].sval), iName7, 1);
          s7 = S_Symbol(strdup(iName7));
          free(iName7);
        }
        S_symbol s10 = NULL;
        char* iName10 = NULL;
        mkInternalName((yyvsp[(10) - (10)].sval), iName10, 1);
        s10 = S_Symbol(strdup(iName10));
        free(iName10);
        (yyval.qun) = A_WindowQun((yylsp[(1) - (10)]).first_line, s3, (yyvsp[(6) - (10)].range), s7, s10);
    }
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 2649 "adl_yacc.grm"
    {
        S_symbol s1 = NULL;
        if(S_Symbol("fetchtbl") != S_Symbol((yyvsp[(3) - (8)].sval))) {
          char* iName1 = NULL;
          mkInternalName((yyvsp[(3) - (8)].sval), iName1, 1);
          s1 = S_Symbol(strdup(iName1));
          free(iName1);
        }
        else {
          s1 = S_Symbol(strdup((yyvsp[(3) - (8)].sval)));
        }
        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(8) - (8)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
        (yyval.qun) = A_FunctionQun((yylsp[(1) - (8)]).first_line, s1, (yyvsp[(5) - (8)].list), s);
    }
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 2668 "adl_yacc.grm"
    { 
        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(4) - (4)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
        (yyval.qun) = A_QueryQun((yylsp[(1) - (4)]).first_line, s, (yyvsp[(2) - (4)].exp));
	/* A_SetQunWindow($$, $4); */
    }
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 2681 "adl_yacc.grm"
    { (yyval.list) = (A_list)0; }
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 2683 "adl_yacc.grm"
    { 
      (yyval.list) = (yyvsp[(2) - (2)].list);
    }
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 2689 "adl_yacc.grm"
    { (yyval.exp) = (A_exp)0; }
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 2691 "adl_yacc.grm"
    { 
      (yyval.exp) = (yyvsp[(2) - (2)].exp);
    }
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 2697 "adl_yacc.grm"
    { (yyval.list) = (A_list)0; }
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 2699 "adl_yacc.grm"
    { (yyval.list) = (yyvsp[(3) - (3)].list); }
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 2703 "adl_yacc.grm"
    { (yyval.exp) = (A_exp)0; }
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 2705 "adl_yacc.grm"
    { 
      (yyval.exp) = (yyvsp[(2) - (2)].exp);
    }
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 2716 "adl_yacc.grm"
    { in_sql = 1; }
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 2717 "adl_yacc.grm"
    {
      (yyval.exp) = (A_exp)0;
    }
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 2721 "adl_yacc.grm"
    {
    }
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 2726 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_TABLE_COLUMN);
      appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].tablecolumn));
    }
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 2731 "adl_yacc.grm"
    { 
      appendAList((yyvsp[(1) - (3)].list), (void *)(yyvsp[(3) - (3)].tablecolumn)); 
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 2738 "adl_yacc.grm"
    {
      (yyval.tablecolumn) = A_TableColumn((yylsp[(1) - (3)]).first_line, (yyvsp[(1) - (3)].field), (yyvsp[(2) - (3)].ival), (yyvsp[(3) - (3)].ival));
    }
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 2744 "adl_yacc.grm"
    { (yyval.ival) = 0; }
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 2746 "adl_yacc.grm"
    { (yyval.ival) = COLUMN_NOT_NULL; }
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 2750 "adl_yacc.grm"
    { (yyval.ival) = 0; }
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 2762 "adl_yacc.grm"
    {in_sql = 0;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 2763 "adl_yacc.grm"
    {	
	S_symbol s = NULL;
        char* iName = NULL;
	mkInternalName((yyvsp[(3) - (9)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
	if((yyvsp[(9) - (9)].sval) != (char*)NULL)
          (yyval.exp) = A_CreatestreamExp((yylsp[(1) - (9)]).first_line, s, (yyvsp[(6) - (9)].exp), S_Symbol((yyvsp[(9) - (9)].sval)));
	else
          (yyval.exp) = A_CreatestreamExp((yylsp[(1) - (9)]).first_line, s, (yyvsp[(6) - (9)].exp), (S_symbol)NULL);
    }
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 2784 "adl_yacc.grm"
    {
	S_symbol s = NULL;
        char* iName = NULL;
	mkInternalName((yyvsp[(3) - (8)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
	//if($9) {
	//  popElementList($9);
        //}
        (yyval.exp) = A_CreateviewExp((yylsp[(1) - (8)]).first_line, s, (yyvsp[(6) - (8)].exp), (A_list)0, (viewmode_t)(yyvsp[(8) - (8)].ival));
    }
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 2799 "adl_yacc.grm"
    {
	(yyval.ival) = (int)A_immidiate;
    }
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 2803 "adl_yacc.grm"
    {
	(yyval.ival) = (int)A_defferred;
    }
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 2815 "adl_yacc.grm"
    { in_sql = 1; }
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 2816 "adl_yacc.grm"
    { 
    }
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 2818 "adl_yacc.grm"
    { in_sql = 1; }
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 2819 "adl_yacc.grm"
    { 
    }
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 2831 "adl_yacc.grm"
    {	
                  (yyval.qun) = (yyvsp[(1) - (1)].qun);
		}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 2835 "adl_yacc.grm"
    {	
                  (yyval.qun) = A_NameQun((yylsp[(1) - (1)]).first_line, S_Symbol("stdout"), 
                                                      S_Symbol("stdout"));
		}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 2840 "adl_yacc.grm"
    {
		  (yyval.qun) = A_NameQun((yylsp[(1) - (1)]).first_line, S_Symbol("return"), 
                                                      S_Symbol("return"));
		}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 2847 "adl_yacc.grm"
    { in_sql = 1; }
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 2848 "adl_yacc.grm"
    {
      A_list jtl = A_List((yylsp[(4) - (6)]).first_line, A_QUN);

      if ((yyvsp[(6) - (6)].list) && (yyvsp[(5) - (6)].exp)->kind == A_selectExp) {
	(yyvsp[(5) - (6)].exp)->u.select.order_by_list = (yyvsp[(6) - (6)].list);
      }
      
      appendAList(jtl, (yyvsp[(4) - (6)].qun));
      appendAList(jtl, (void*)A_QueryQun((yylsp[(5) - (6)]).first_line, 0, (yyvsp[(5) - (6)].exp)));

      (yyval.exp) = A_SqlOprExp((yylsp[(1) - (6)]).first_line,
		       A_SQL_INSERT, 
		       0,
		       (A_list)0,
		       jtl,
		       (A_list)0);

    }
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 2873 "adl_yacc.grm"
    { in_sql=1; }
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 2874 "adl_yacc.grm"
    {
      A_list jtl = A_List((yylsp[(3) - (6)]).first_line, A_QUN);

        S_symbol s = NULL;
        char* iName = NULL;
        mkInternalName((yyvsp[(6) - (6)].sval), iName, 1);
        s = S_Symbol(strdup(iName));
        free(iName);
      appendAList(jtl, (void*)A_NameQun((yylsp[(3) - (6)]).first_line, S_Symbol((yyvsp[(3) - (6)].sval)), 0));
      appendAList(jtl, (void*)A_NameQun((yylsp[(6) - (6)]).first_line, s, 0));
      (yyval.exp) = A_SqlOprExp((yylsp[(1) - (6)]).first_line,
		       A_SQL_LOAD, 
		       0,
		       (A_list)0,
		       jtl,
		       (A_list)0);
    }
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 2898 "adl_yacc.grm"
    { in_sql = 1; }
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 2899 "adl_yacc.grm"
    {
      A_list wr_prd_list = (A_list)0;
      A_list jtl = A_List((yylsp[(4) - (6)]).first_line, A_QUN);
      appendAList(jtl, (yyvsp[(3) - (6)].qun));

      if ((yyvsp[(6) - (6)].exp)) {
	wr_prd_list = A_List((yylsp[(6) - (6)]).first_line, A_EXP);
	decomposeBoolExpr((yyvsp[(6) - (6)].exp), wr_prd_list);
      }

      (yyval.exp) = A_SqlOprExp((yylsp[(1) - (6)]).first_line,
		       A_SQL_UPDATE, 
		       0,	// distinct
		       (yyvsp[(5) - (6)].list),	// hxp
		       jtl,	// join table list
		       wr_prd_list // where
		       );
    }
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 2919 "adl_yacc.grm"
    {
      (yyval.list) = A_List((yylsp[(1) - (1)]).first_line, A_EXP);
      appendAList((yyval.list), (void*)(yyvsp[(1) - (1)].exp));
    }
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 2924 "adl_yacc.grm"
    {
      appendAList((yyvsp[(1) - (3)].list), (void*)(yyvsp[(3) - (3)].exp));
      (yyval.list) = (yyvsp[(1) - (3)].list);
    }
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 2930 "adl_yacc.grm"
    {
      A_var var = A_SimpleVar((yylsp[(1) - (3)]).first_line, S_Symbol((yyvsp[(1) - (3)].sval))); 

      (yyval.exp) = A_AssignExp((yylsp[(1) - (3)]).first_line, var, (yyvsp[(3) - (3)].exp));
    }
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 2936 "adl_yacc.grm"
    {
      A_var var = A_RefVar((yylsp[(1) - (3)]).first_line, (yyvsp[(1) - (3)].ref)); 

      (yyval.exp) = A_AssignExp((yylsp[(1) - (3)]).first_line, var, (yyvsp[(3) - (3)].exp));
    }
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 2947 "adl_yacc.grm"
    { in_sql = 1; }
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 2948 "adl_yacc.grm"
    {
      A_list jtl = A_List((yylsp[(4) - (5)]).first_line, A_QUN);
      A_list wr_prd_list = (A_list)0;
      appendAList(jtl, (yyvsp[(4) - (5)].qun));

      if ((yyvsp[(5) - (5)].exp)) {
	wr_prd_list = A_List((yylsp[(5) - (5)]).first_line, A_EXP);
	decomposeBoolExpr((yyvsp[(5) - (5)].exp), wr_prd_list);
      }

      (yyval.exp) = A_SqlOprExp((yylsp[(1) - (5)]).first_line,
		       A_SQL_DELETE, 
		       0,	// distinct
		       (A_list)0, // hxp
		       jtl,	// join table list
		       wr_prd_list // where
		       );
    }
    break;



/* Line 1455 of yacc.c  */
#line 6561 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }

  yyerror_range[0] = yylloc;

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, &yylloc);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  yyerror_range[0] = yylsp[1-yylen];
  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      yyerror_range[0] = *yylsp;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;

  yyerror_range[1] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
  *++yylsp = yyloc;

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, &yylloc);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



