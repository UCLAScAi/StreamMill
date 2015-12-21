INCLUDES=-I../../../ -I../../../../dbug -I../../../../im_db -I../../ -I../../../../ -I../../../../../atlas/include/
esl_LDADD=../../../libatlas.a ../../../../im_db/libimdb.a ../../../../dbug/libdbug.a -ldl /usr/lib/libmm.a ../../../../../atlas/lib/libdb.a

all: buf winbuf_test

buf: buf.cc $(esl_LDADD)
	g++ -w -g -o $@ $< $(INCLUDES) $(esl_LDADD)
winbuf_test: winbuf_test.o $(esl_LDADD)
	g++ -w -g -o $@ $< $(INCLUDES) $(esl_LDADD)

.cc.o:
	g++ -w -c -g -o $@ $< $(INCLUDES)
