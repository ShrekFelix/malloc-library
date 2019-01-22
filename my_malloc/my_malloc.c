#include "my_malloc.h"
#include <limits.h>

//First Fit malloc/free
void *ff_malloc(size_t size){
  if (size <= 0) {
    return NULL;
  }
  struct Block* b = head;
  while(b){
    if( b->free && b->size >= size ){ // found a free block big enough
      // TODO: can split this block to save space
      b->free = 0;
      return b+1;
    }
    b = b->next;
  }
  // no available block, get a new one
  // initialize the block
  b = sbrk(size + sizeof(struct Block)); // request space from kernel
  if ((int)b == -1) { // sbrk failed
    return NULL;
  }
  b->size = size;
  b->next = NULL;
  b->prev = NULL;
  b->free = 0;
  if(!head){ // first malloc
    head = b;
    tail = b;
  }
  else{
    tail->next = b;
    b->prev = tail;
    tail = b;
  }
  return b+1;
}

void ff_free(void* p){
    if (!p){
      return;
    }
    struct Block* b = (struct Block*)p - 1; // locate block
    b->free = 1;
    if(b->prev && b->prev->free){ // merge with prev
      b->prev->size += b->size;
      b->prev->next = b->next;
      b = b->prev;
    }
    if(b->next && b->next->free){ // merge with next
      b->size += b->next->size;
      b->next = b->next->next;
    }
}

//Best Fit malloc/free
void *bf_malloc(size_t size){
  if (size <= 0) {
    return NULL;
  }
  struct Block* b = head;
  struct Block* best = b;
  while(b){
    if( b->free && b->size >= size ){ // found a free block big enough
      // TODO: can split this block to save space
      if(b->size < best->size){
        best = b;
      }
    }
    b = b->next;
  }
  if(best){
    return best+1;
  }
  // no available block, get a new one
  // initialize the block
  b = sbrk(size + sizeof(struct Block)); // request space from kernel
  if ((int)b == -1) { // sbrk failed
    return NULL;
  }
  b->size = size;
  b->next = NULL;
  b->prev = NULL;
  b->free = 0;
  if(!head){ // first malloc
    head = b;
    tail = b;
  }
  else{
    tail->next = b;
    b->prev = tail;
    tail = b;
  }
  return b+1;
}
void bf_free(void* p){
  ff_free(p);
}

unsigned long get_data_segment_size(){
  struct Block* b = head;
  int sum = 0;
  while(b){
    sum += b->size;
    b = b->next;
  }
  return sum;
}

unsigned long get_data_segment_free_space_size(){
  struct Block* b = head;
  int sum = 0;
  while(b){
    if(b->free){
      sum += b->size;
    }
    b = b->next;
  }
  return sum;
}