CC = gcc
INCLUDE=-I../../BerkeleyDB/include -I../../sql
LIB=../../sql/libatlas.a $(ATLAS_HOME)/lib/libdbug.a $(ATLAS_HOME)/lib/libdb.a $(ATLAS_HOME)/lib/libimdb.a /usr/lib/libmm.a -ldl -lstdc++
#LIB=
FLAG = -g

all : gini.so tabf.so gini sample.so

sample.so : sample.c
	$(CC) -shared -o $@ -fPIC $(INCLUDE) $< $(LIB)

tabf.so : tabf.c
	$(CC) -shared -o tabf.so -fPIC $(INCLUDE) tabf.c $(LIB)

gini.so : gini.c Makefile
	$(CC) -shared -o gini.so -fPIC $(INCLUDE) gini.c $(LIB)


gini : gini.c
	$(CC) $(FLAG) -c gini.c
	$(CC) $(FLOG) -o gini gini.o $(INCLUDE)  $(LIB)
