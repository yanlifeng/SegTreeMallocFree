CC:= mpicc
AR:= ar rcs

CFLAGS:= -I./include
CFLAGS+= -w -g -O2 

all: lib/libylfMallocFree.a

lib/libylfMallocFree.a: build/ylfMallocFree.o
	$(AR) $@ $^

build/ylfMallocFree.o: src/ylfMallocFree.c
	$(CC) $(CFLAGS) -c $^ -o $@
