CC:= mpicc
FC:= mpif90
AR:= ar rcs

CFLAGS:= -I./include
#CFLAGS:= -w -g -O0 
CFLAGS+= -w -g -O2 
#FFLAGS:= -I./include

all: lib/libylfMallocFree.a

lib/libylfMallocFree.a: build/ylfMallocFree.o
	$(AR) $@ $^

build/ylfMallocFree.o: src/ylfMallocFree.c
#build/ylfMallocFree.o: src/ok.c
	$(CC) $(CFLAGS) -c $^ -o $@
