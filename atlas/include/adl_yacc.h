#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

#ifndef YYSTYPE
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
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

#ifndef YYLTYPE
typedef struct yyltype
{
  int first_line;
  int first_column;

  int last_line;
  int last_column;
} yyltype;

# define YYLTYPE yyltype
# define YYLTYPE_IS_TRIVIAL 1
#endif

# define	IDENT	257
# define	SQL_ID	258
# define	STRING	259
# define	SQL_STRING	260
# define	CCODE	261
# define	NUM	262
# define	SQL_NUM	263
# define	DAY	264
# define	HOUR	265
# define	MINUTE	266
# define	SECOND	267
# define	TIMESTAMP_SYM	268
# define	REAL_NUM	269
# define	SQL_REAL_NUM	270
# define	END_OF_INPUT	271
# define	LIST_SYM	272
# define	QUIT_SYM	273
# define	HELP_SYM	274
# define	DB2_SYM	275
# define	COMPILE_SYM	276
# define	NIL	277
# define	OF	278
# define	IF	279
# define	THEN	280
# define	ELSE	281
# define	WHILE	282
# define	DO	283
# define	FOR	284
# define	TO	285
# define	BREAK	286
# define	LET	287
# define	IN	288
# define	END	289
# define	TYPE	290
# define	ARRAY	291
# define	VAR	292
# define	FUNCTION	293
# define	AGGREGATE	294
# define	CAGGREGATE	295
# define	INT_SYM	296
# define	CHAR_SYM	297
# define	REAL_SYM	298
# define	IEXT_SYM	299
# define	TRUE	300
# define	FALSE	301
# define	SQL_CREATE	302
# define	SQL_SELECT	303
# define	SQL_DROP	304
# define	SQL_INSERT	305
# define	SQL_DELETE	306
# define	SQL_UPDATE	307
# define	SQL_LOAD	308
# define	MODELTYPE	309
# define	FLOW	310
# define	UDA	311
# define	SHAREDTABLES	312
# define	PARAMETERS	313
# define	PARTABLES	314
# define	WITH	315
# define	AS	316
# define	UNION	317
# define	INTERSECT	318
# define	EXCEPT	319
# define	ALL	320
# define	DISTINCT	321
# define	REFRESH	322
# define	IMMEDIATE	323
# define	DEFFERRED	324
# define	FROM	325
# define	WHERE	326
# define	GROUP	327
# define	PARTITION	328
# define	OVER	329
# define	ROWS	330
# define	RANGE	331
# define	SLIDE	332
# define	PRECEDING	333
# define	BY	334
# define	UNLIMITED	335
# define	HAVING	336
# define	ORDER	337
# define	UNIQUE	338
# define	RUN	339
# define	USING	340
# define	ON	341
# define	ORDERED	342
# define	SORTED	343
# define	LOCAL	344
# define	SOURCE	345
# define	MEMORY	346
# define	BTREE	347
# define	HASH	348
# define	RTREE	349
# define	TARGET	350
# define	OID	351
# define	IS	352
# define	SYSTEM	353
# define	DEFINED	354
# define	REF	355
# define	VALUES	356
# define	EXISTS	357
# define	CASE	358
# define	WHEN	359
# define	TABLE	360
# define	STREAM	361
# define	INDEX	362
# define	CLIKE	363
# define	RLIKE	364
# define	SLIKE	365
# define	INTO	366
# define	DYNAMIC	367
# define	ASC	368
# define	DESC	369
# define	SET	370
# define	LIST	371
# define	BAG	372
# define	COMPLEX_FUNC	373
# define	SCALAR_FUNC	374
# define	BUILTINFUNC	375
# define	PORT	376
# define	NULLSYM	377
# define	EXTERNAL	378
# define	NAME	379
# define	INWINDOW	380
# define	WINDOW	381
# define	RETURN	382
# define	STDOUT	383
# define	STATE	384
# define	INITIALIZE	385
# define	ITERATE	386
# define	EXPIRE	387
# define	PRODUCE	388
# define	TERMINATE	389
# define	NOP	390
# define	PRIMARY	391
# define	KEY	392
# define	AGGR_AVG	393
# define	AGGR_SUM	394
# define	AGGR_MIN	395
# define	AGGR_MAX	396
# define	AGGR_COUNT	397
# define	AGGR_XA	398
# define	BF_ABS	399
# define	BF_CASE	400
# define	BF_LENGTH	401
# define	BF_LTRIM	402
# define	BF_MAX	403
# define	BF_MIN	404
# define	BF_RTRIM	405
# define	BF_SQRT	406
# define	BF_SUBSTR	407
# define	BF_TRIM	408
# define	XML_XE	409
# define	XML_ATTR	410
# define	BF_OLDEST	411
# define	INTERNAL	412
# define	OR	413
# define	AND	414
# define	BETWEEN	415
# define	GE	416
# define	LE	417
# define	NE	418
# define	LIKE	419
# define	SQL_IN	420
# define	NEG	421
# define	NOT	422
# define	POINTER	423
# define	MINUSMINUS	424
# define	PLUSPLUS	425
# define	PLUSEQL	426
# define	MINUSEQL	427


extern YYSTYPE yylval;

#endif /* not BISON_Y_TAB_H */
