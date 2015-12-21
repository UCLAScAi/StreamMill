GCC=gcc -g -I. -I./common -Wall -DDEBUG -DSWIM -O3 
#-DDEVELOP
GPP=g++ -I.

BASIC_OBJ = fptree.o memmgr.o timeval.o histmgr.o swimlib.o
TARGETS = libswimfis.a
all: $(TARGETS)

fptree.o: ./common/fptree.c ./common/fptree.h
	$(GCC) -c ./common/fptree.c

histmgr.o: ./common/histmgr.c ./common/histmgr.h
	$(GCC) -c ./common/histmgr.c

timeval.o: ./common/timeval.c ./common/timeval.h
	$(GCC) -c ./common/timeval.c
	
memmgr.o: ./common/memmgr.c ./common/memmgr.h
	$(GCC) -c ./common/memmgr.c

swimlib.o: swimlib.c swimlib.h
	$(GCC) -c swimlib.c

	
#	$(GCC) -lm swimlib.c $(BASIC_OBJ) -c swimlib.o

libswimfis.a: ${BASIC_OBJ}
	ar cru libswimfis.a ${BASIC_OBJ}	



	
clean:
	rm -f $(TARGETS) *.o *~

