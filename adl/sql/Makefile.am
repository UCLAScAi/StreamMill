CPP_FLAGS = -Wno-deprecated
CPPFLAGS = -DSWIM
SUFFIXES = .grm .c .o

bin_PROGRAMS = eslc adlc atlaswin_test esl_start eslt testq
bin_SCRIPTS = adlcc eslcc
lib_LIBRARIES = libatlas.a libswimfis.a

eslc_SOURCES = esl/eslc.cc
eslc_LDADD =  libatlas.a ../im_db/libimdb.a /usr/local/lib/libmm.a ../dbug/libdbug.a -ldl ../BerkeleyDB/lib/libdb.a

esl_start_SOURCES = esl/esl.cc 
esl_start_LDADD =  libatlas.a ../im_db/libimdb.a /usr/local/lib/libmm.a ../dbug/libdbug.a -ldl ../BerkeleyDB/lib/libdb.a

eslt_SOURCES = esl/eslt.cc
eslt_LDADD =  libatlas.a ../im_db/libimdb.a /usr/local/lib/libmm.a ../dbug/libdbug.a -ldl ../BerkeleyDB/lib/libdb.a

testq_SOURCES = esl/testq.cc
testq_LDADD = libatlas.a ../BerkeleyDB/lib/libdb.a ../im_db/libimdb.a ../rtree/librtree.a /usr/local/lib/libmm.a ../dbug/libdbug.a -ldl

libswimfis_a_SOURCES = uda-swim/common/fptree.h uda-swim/common/memmgr.h uda-swim/common/histmgr.h uda-swim/common/timeval.h uda-swim/swimlib.c uda-swim/common/fptree.c uda-swim/common/histmgr.c uda-swim/common/memmgr.c uda-swim/common/timeval.c

libatlas_a_SOURCES = adllib.cc atlaswin.cc adllib.h atlaswin.h adl_yacc.cc lex.cc absyn.cc sql_rewrite.cc oid_rewrite.cc semant.cc bind.cc sqlsem.cc builtin.cc udf.cc semant_aggr.cc aggr_info.cc table.cc trans2C.cc symbol.cc SMLog.cc types.cc env.cc adl_sys.cc io.cc adl_obj.cc util.cc list.cc err.cc getopt.c getopt1.c err.h  getopt.h io.h lex.h list.h adl_obj.h adl_sys.h system.h absyn.h trans2C.h const.h irtree.h semant.h aggr_info.h table.h types.h env.h temp.h util.h symbol.h SMLog.h adlhash.h atlaswin.h basic.h esl/GUIClient.cc esl/buffer.cc winbuf.cc esl/dbt.cc esl/querySchdl.cc esl/driver.cc esl/monitor.cc esl/strategy.cc esl/stmt.cc esl/compiler.cc esl/windowBuf.cc esl/ios/ios.cc

libatlas_a_LIBADD = /usr/local/lib/libmm.a ../im_db/hash-arbitrary.o

adlc_SOURCES = adlc.cc
adlc_LDADD =  libatlas.a ../im_db/libimdb.a ../dbug/libdbug.a /usr/local/lib/libmm.a ../BerkeleyDB/lib/libdb.a


atlaswin_test_SOURCES = atlaswin_test.cc 
atlaswin_test_LDADD = ./libatlas.a

include_HEADERS = ../BerkeleyDB/include/db.h adlhash.h atlaswin.h adllib.h esl/buffer.h winbuf.h esl/stmt.h types.h const.h abstypes.h absyn.h adlhash.h adllib.h adl_obj.h adl_sys.h adl_yacc.h basic.h env.h err.h getopt.h heap.h io.h irtree.h lex.h list.h semant.h sql_rewrite.h symbol.h SMLog.h system.h table.h trans2C.h translate.h util.h win.h esl/compiler.h esl/dbt.h esl/driver.h esl/monitor.h esl/strategy.h esl/GUIClient.h esl/querySchdl.h esl/ios/ios.h esl/windowBuf.h uda-swim/swimlib.h uda-swim/common/fptree.h  uda-swim/common/memmgr.h uda-swim/common/histmgr.h  uda-swim/common/timeval.h


EXTRA_DIST = adl_yacc.grm Makefile.win

adlcc:
	chmod +x adlcc

.grm.cc:
	$(YACC) $(AM_YFLAGS) $(YFLAGS) $< && \
	sed -e 's/yyn += yytoken;/yyn += yytoken;\
	{\
	int dodefault = (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yytoken);\
	int newlookahead; \
	if (((dodefault \&\& yydefact[yystate]==0) ||\
	(!dodefault \&\& (yytable[yyn] == YYPACT_NINF || yytable[yyn] == 0))) \&\&\
	(newlookahead = yylex2())) {\
	yychar = newlookahead;	goto yybackup;	\
	}	\
	}/' y.tab.c > $*.cc
	if test -f y.tab.h; then \
	if cmp -s y.tab.h $*.h; then rm -f y.tab.h; else mv y.tab.h $*.h; fi; \
	else :; fi


#adl_yacc.h: adl_yacc.cc

datadir = $(prefix)/@DATADIRNAME@
# AM_CFLAGS = -g
# AM_CXXFLAGS = -g
YFLAGS = -d
INCLUDES = -I../rtree -I../im_db -I../BerkeleyDB/include -Iuda-swim -Iuda-swim/common -Iesl -I../dbug -I.

