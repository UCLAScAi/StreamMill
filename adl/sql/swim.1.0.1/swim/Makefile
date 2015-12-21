GCC=gcc -I. -I../common -Wall -DDEBUG -DSWIM -O3 
#-DDEVELOP
GPP=g++ -I.

BASIC_OBJ = fptree.o memmgr.o timeval.o histmgr.o
TARGETS = mem-delayed disk-delayed asc2bin bin2asc
all: $(TARGETS)

#hash_verifier 
 
fptree.o: ../common/fptree.c ../common/fptree.h
	$(GCC) -c ../common/fptree.c

histmgr.o: ../common/histmgr.c ../common/histmgr.h
	$(GCC) -c ../common/histmgr.c

timeval.o: ../common/timeval.c ../common/timeval.h
	$(GCC) -c ../common/timeval.c
	
memmgr.o: ../common/memmgr.c ../common/memmgr.h
	$(GCC) -c ../common/memmgr.c

largeSlidingWind: largeSlidingWind.c $(BASIC_OBJ)
	$(GCC) -lm largeSlidingWind.c  -o largeSlidingWind $(BASIC_OBJ)
	
disk-delayed: disk-delayed.c $(BASIC_OBJ)
	$(GCC) -lm disk-delayed.c  -o disk-delayed $(BASIC_OBJ)
	
mem-delayed: mem-delayed.c $(BASIC_OBJ)
	$(GCC) -lm mem-delayed.c -o mem-delayed $(BASIC_OBJ)

mem-delayed.o: mem-delayed.c
	$(GCC) -lm mem-delayed.c -c -o mem-delayed.o

lib-mem-delayed.a: mem-delayed.o $(BASIC_OBJ)
	ar rcs lib-mem-delayed.a mem-delayed.o $(BASIC_OBJ)

average-analyser: average-analyser.c $(BASIC_OBJ)
	$(GCC) -lm average-analyser.c  -o average-analyser $(BASIC_OBJ)
	
asc2bin: asc2bin.c $(BASIC_OBJ)
	$(GCC) -lm asc2bin.c -o asc2bin $(BASIC_OBJ)
	
bin2asc: bin2asc.c $(BASIC_OBJ)
	$(GCC) -lm bin2asc.c -o bin2asc $(BASIC_OBJ)

debug-bin: debug-bin.c
	$(GCC) debug-bin.c -o debug-bin
	
clean:
	rm -f $(TARGETS) *.o *~ *.a

install: lib-mem-delayed.a
	cp lib-mem-delayed.a $(ATLAS_HOME)/lib/
	
