#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"

#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p)    ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p)    bf_free(p)
#endif


int main(int argc, char *argv[])
{
  int i;
  int *array[10];

  array[0] = (int *)MALLOC(1 * sizeof(int));
  array[1] = (int *)MALLOC(2 * sizeof(int));
  array[2] = (int *)MALLOC(3 * sizeof(int));
  array[3] = (int *)MALLOC(4 * sizeof(int));
  FREE(array[0]);
  FREE(array[1]);
  FREE(array[2]);
  array[4] = (int *)MALLOC(4 * sizeof(int));
  array[5] = (int *)MALLOC(6 * sizeof(int));
  array[6] = (int *)MALLOC(7 * sizeof(int));
  array[7] = (int *)MALLOC(8 * sizeof(int));
  array[8] = (int *)MALLOC(9 * sizeof(int));
  array[9] = (int *)MALLOC(10 * sizeof(int));

  return 0;
}
