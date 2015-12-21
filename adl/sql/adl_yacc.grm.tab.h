typedef union {
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
//    A_fundec fundec;
//    A_aggrdec aggrdec;
  A_selectitem selectitem;
  A_qun qun;
  A_tablecolumn tablecolumn;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#define	IDENT	258
#define	SQL_ID	259
#define	STRING	260
#define	SQL_STRING	261
#define	NUM	262
#define	SQL_NUM	263
#define	REAL_NUM	264
#define	SQL_REAL_NUM	265
#define	END_OF_INPUT	266
#define	LIST_SYM	267
#define	QUIT_SYM	268
#define	HELP_SYM	269
#define	DB2_SYM	270
#define	COMPILE_SYM	271
#define	NIL	272
#define	OF	273
#define	IF	274
#define	THEN	275
#define	ELSE	276
#define	WHILE	277
#define	DO	278
#define	FOR	279
#define	TO	280
#define	BREAK	281
#define	LET	282
#define	IN	283
#define	END	284
#define	TYPE	285
#define	ARRAY	286
#define	VAR	287
#define	FUNCTION	288
#define	AGGREGATE	289
#define	INT_SYM	290
#define	CHAR_SYM	291
#define	REAL_SYM	292
#define	SQL_CREATE	293
#define	SQL_SELECT	294
#define	SQL_DROP	295
#define	SQL_INSERT	296
#define	SQL_DELETE	297
#define	SQL_UPDATE	298
#define	SQL_LOAD	299
#define	WITH	300
#define	AS	301
#define	UNION	302
#define	INTERSECT	303
#define	EXCEPT	304
#define	ALL	305
#define	DISTINCT	306
#define	FROM	307
#define	WHERE	308
#define	GROUP	309
#define	BY	310
#define	HAVING	311
#define	ORDER	312
#define	UNIQUE	313
#define	ORDERED	314
#define	SORTED	315
#define	LOCAL	316
#define	GLOBAL	317
#define	MEMORY	318
#define	OID	319
#define	IS	320
#define	SYSTEM	321
#define	DEFINED	322
#define	REF	323
#define	VALUES	324
#define	EXISTS	325
#define	CASE	326
#define	WHEN	327
#define	TABLE	328
#define	INDEX	329
#define	CLIKE	330
#define	RLIKE	331
#define	SLIKE	332
#define	INTO	333
#define	ASC	334
#define	DESC	335
#define	SET	336
#define	LIST	337
#define	BAG	338
#define	COMPLEX_FUNC	339
#define	SCALAR_FUNC	340
#define	BUILTINFUNC	341
#define	NULLSYM	342
#define	EXTERNAL	343
#define	NAME	344
#define	RETURN	345
#define	STATE	346
#define	INITIALIZE	347
#define	ITERATE	348
#define	PRODUCE	349
#define	TERMINATE	350
#define	NOP	351
#define	PRIMARY	352
#define	KEY	353
#define	AGGR_AVG	354
#define	AGGR_SUM	355
#define	AGGR_MIN	356
#define	AGGR_MAX	357
#define	AGGR_COUNT	358
#define	BF_ABS	359
#define	BF_CASE	360
#define	BF_LENGTH	361
#define	BF_LTRIM	362
#define	BF_MAX	363
#define	BF_MIN	364
#define	BF_RTRIM	365
#define	BF_SQRT	366
#define	BF_SUBSTR	367
#define	BF_TRIM	368
#define	OR	369
#define	AND	370
#define	BETWEEN	371
#define	GE	372
#define	LE	373
#define	NE	374
#define	LIKE	375
#define	SQL_IN	376
#define	NEG	377
#define	NOT	378
#define	POINTER	379


extern YYSTYPE yylval;
