INCLUDES=-I../../../ -I../../../../dbug -I../../../../im_db -I../../ -I../../../../
esl_LDADD=../../../libatlas.a ../../../libadl.a ../../../../im_db/libimdb.a ../../../../dbug/libdbug.a -ldl -ldb  /usr/local/lib/libmm.a 

all: buf winbuf_test

buf: buf.cc $(esl_LDADD)
	g++ -g -o $@ $< $(INCLUDES) $(esl_LDADD)
winbuf_test: winbuf_test.o $(esl_LDADD)
	g++ -g -o $@ $< $(INCLUDES) $(esl_LDADD)

.cc.o:
	g++ -c -g -o $@ $< $(INCLUDES)
