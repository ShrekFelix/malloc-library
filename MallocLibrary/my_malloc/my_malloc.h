#include <stdlib.h>

struct Block {
  struct Block* next;
  struct Block* prev;
  size_t size;
  int free;
};

struct Block* head = NULL;
struct Block* tail = NULL;

//First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);
//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes