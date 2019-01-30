#include <stdlib.h>

typedef struct _Block{
  struct _Block* next;
  struct _Block* prev;
  struct _Block* phys_next;
  struct _Block* phys_prev;
  size_t size;
  int free;
} Block;

Block* head = NULL;
Block* tail = NULL;
Block* phys_head = NULL;
Block* phys_tail = NULL;

//Best Fit malloc/free
void* bf_malloc(size_t size);
void bf_free(void *ptr);

void insert_block(Block* p, Block* b);
void extend_freeLL(Block* b);
void insert_physLL(Block* p, Block* b);
//void extend_physLL(Block* b);
void remove_block_freeLL(Block* b);
void remove_block_physLL(Block* b);
void merge_blocks(Block* a, Block* b);
Block* initialize_block(Block*b, size_t size);

unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes

// debug
void physLL_summary();
void freeLL_summary();
