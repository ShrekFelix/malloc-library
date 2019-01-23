#include "my_malloc.h"
#include <assert.h>
#define BLK_SZ (sizeof(struct Block))

struct Block* create_block(size_t size){
  // initialize the block
  struct Block* b = sbrk(size + BLK_SZ); // request space from kernel
  if ((int)b == -1) { // sbrk failed
    return NULL;
  }
  b->size = size;
  b->next = NULL;
  b->prev = NULL;
  b->free = 0;
  // insert the block into LL
  if(!head){ // empty LL
    head = b;
    tail = b;
  }else{
    insert_block(tail, b);
  }
  return b;
}

void merge_blocks(struct Block* a, struct Block* b){
  if( a && b && (void*)a + a->size + BLK_SZ == (void*)b){
    a->size += b->size + BLK_SZ;
    remove_block(b);
  }
}

// insert b into LL right after p
void insert_block(struct Block* p, struct Block* b){
  assert(head && tail);
  b->next = p->next;
  b->prev = p;
  p->next = b;
  if(b->next){
    b->next->prev = b;
  }else{
    tail = b;
  }
}

// remove b from LL
void remove_block(struct Block* b){
  b->free = 0;
  if(b != head){
    b->prev->next = b->next;
  }else{ // b is head and then head is removed
    head = head->next; // replace head
  }
  if(b != tail){
    b->next->prev = b->prev;
  }else{ // b is tail and then tail is removed
    tail = tail->prev; // replace tail
  }
}

//First Fit malloc/free
void *ff_malloc(size_t size){
  if (size <= 0) {
    return NULL;
  }
  struct Block* b = head;
  while(b){
    assert(b->free); // LL only tracks free blocks
    if( b->size >= size ){ // found a free block big enough
      // TODO: can split this block to save space
      remove_block(b);
      return b+1;
    }
    b = b->next;
  }
  // no available block, get a new one
  b = create_block(size);
  return b+1;
}

void ff_free(void* ptr){
  if(!ptr){
    return;
  }
  struct Block* b = (struct Block*)ptr - 1; // locate block
  assert(!b->free);
  b->free = 1;
  // insert b to LL
  if(!head){ // empty LL
    assert(!tail);
    head = b;
    tail = b;
  }else{
    // LL tracks free blocks by their physical address order
    struct Block* p = tail;
    while(p){
      if(p < b){ // stop until the node's physical address is smaller than b
        break;
      }
      p = p->prev;
    }
    if(p){
      insert_block(p, b);
    }else{ // scanned to the end
      // insert p into head
      head->prev = b;
      b->next = head;
      b->prev = NULL;
      head = b;
    }
  }
  merge_blocks(b, b->next);
  merge_blocks(b->prev, b);
}

//Best Fit malloc/free
void *bf_malloc(size_t size){
  if (size <= 0) {
    return NULL;
  }
  struct Block* b = head;
  struct Block* best = NULL;
  while(b){
    assert(b->free); // LL only tracks free blocks
    if( b->size >= size ){ // found a free block big enough
      // TODO: can split this block to save space
      if(!best || b->size < best->size){
        best = b;
      }
      if(best->size == size){
        break;
      }
    }
    b = b->next;
  }
  if(best){
    remove_block(best);
    return best+1;
  }
  // no available block, get a new one
  b = create_block(size);
  return b+1; // user's space starts from (void*)b + BLK_SZ
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
