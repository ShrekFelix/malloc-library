CC=gcc
CFLAGS=-ggdb3 -O3 -fPIC

all: lib lib_nolock

lib: my_malloc.o
	$(CC) $(CFLAGS) -shared -o libmymalloc.so my_malloc.o

lib_nolock: my_malloc_nolock.o
	$(CC) $(CFLAGS) -shared -o libmymalloc_nolock.so my_malloc_nolock.o

my_malloc.o: my_malloc.c my_malloc.h
	$(CC) $(CFLAGS) -c -o $@ $< 

my_malloc_nolock.o: my_malloc_nolock.c my_malloc_nolock.h
	$(CC) $(CFLAGS) -c -o $@ $< 

lib_osx: my_malloc.c
	$(CC) -ggdb3 -O3 -dynamiclib -std=gnu99 -o libmymalloc.dylib $<

clean:
	rm -f *~ *.o *.so *.dylib

clobber:
	rm -f *~ *.o
