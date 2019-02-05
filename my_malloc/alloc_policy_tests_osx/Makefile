CC=gcc
CFLAGS=-ggdb3 -O3 -fPIC
DEPS=my_malloc.h
MALLOC_VERSION=FF
WDIR=..

small_range_rand_allocs: small_range_rand_allocs.c ../libmymalloc.dylib
	gcc -std=gnu99 -ggdb3 -O3 -I$(WDIR) ../libmymalloc.dylib -o $@ $<

clean:
	rm -f *~ *.o equal_size_allocs small_range_rand_allocs large_range_rand_allocs

clobber:
	rm -f *~ *.o
