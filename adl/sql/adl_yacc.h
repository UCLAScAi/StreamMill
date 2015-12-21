/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

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




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 132 "adl_yacc.grm"
typedef union YYSTYPE {
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
} YYSTYPE;
/* Line 1403 of yacc.c.  */
#line 413 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

#if ! defined (YYLTYPE) && ! defined (YYLTYPE_IS_DECLARED)
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

extern YYLTYPE yylloc;


