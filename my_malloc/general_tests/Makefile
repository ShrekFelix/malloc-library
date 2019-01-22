CC=gcc
CFLAGS=-ggdb3 -O3 -fPIC
MALLOC_VERSION=BF
WDIR=..

all: mymalloc_test

test: test.c
	$(CC) $(CFLAGS) -I$(WDIR) -L$(WDIR) -Wl,-rpath=$(WDIR) -o $@ test.c -lmymalloc -lrt

mymalloc_test: mymalloc_test.c
	$(CC) $(CFLAGS) -I$(WDIR) -L$(WDIR) -D$(MALLOC_VERSION) -Wl,-rpath=$(WDIR) -o $@ mymalloc_test.c -lmymalloc -lrt

clean:
	rm -f *~ *.o mymalloc_test test

clobber:
	rm -f *~ *.o
