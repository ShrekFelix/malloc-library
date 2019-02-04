#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct Block {
  struct Block* next;
  struct Block* prev;
  size_t size;
  int free;
};

__thread struct Block* head = NULL;
__thread struct Block* tail = NULL;

void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

//First Fit malloc/free
void* ff_malloc(size_t size);
void ff_free(void *ptr);
//Best Fit malloc/free
void* bf_malloc(size_t size);
void bf_free(void *ptr);

void insert_block(struct Block* p, struct Block* b);
void remove_block(struct Block* b);
void merge_blocks(struct Block* a, struct Block* b);
struct Block* next_seg(struct Block* b);
struct Block* create_block(size_t size);