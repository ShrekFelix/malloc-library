#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct Block2 {
  struct Block2* next;
  struct Block2* prev;
  size_t size;
  int free;
};

__thread struct Block2* head2 = NULL;
__thread struct Block2* tail2 = NULL;

void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

void remove_block2(struct Block2* b);
void merge_blocks2(struct Block2* a, struct Block2* b);
struct Block2* next_seg2(struct Block2* b);
struct Block2* create_block2(size_t size);