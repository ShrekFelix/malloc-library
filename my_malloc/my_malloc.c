#include "my_malloc.h"
#include <assert.h>
#define BLK_SZ (sizeof(struct Block))

struct Block* create_block(size_t size){
  // initialize the block
  seg_sz += size + BLK_SZ;
  struct Block* b = sbrk(size + BLK_SZ); // request space from kernel
  if ((int)b == -1) { // sbrk failed
    return NULL;
  }
  b->size = size;
  b->next = NULL;
  b->prev = NULL;
  b->free = 0;
  return b;
}

struct Block* next_seg(struct Block* b){
  return (void*)b + b->size + BLK_SZ;
}

void merge_blocks(struct Block* a, struct Block* b){
  if( a && b && b==next_seg(a)){
    a->size += b->size + BLK_SZ;
    seg_free_sz += BLK_SZ;
    remove_block(b);
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
      seg_free_sz -= b->size;
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
  seg_free_sz += b->size;
  if(!head){ // empty LL
    assert(!tail);
    head = b;
    tail = b;
    b->prev = NULL;
    b->next = NULL;
  }else if(b > tail){
    // b should be physically the last free block in the LL
    // insert b to end of LL
    tail->next = b;
    b->prev = tail;
    b->next = NULL;
    tail = b;
  }else{
    // look for a free block physically after b
    struct Block* p = next_seg(b);
    while(p){
      if(p->free){
        b->prev = p->prev;
        b->next = p;
        p->prev = b;
        if(b->prev){
          b->prev->next = b;
        }else{
          head = b;
        }
        break;
      }
      p = next_seg(p);
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
    seg_free_sz -= best->size;
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
  return seg_sz;
}

unsigned long get_data_segment_free_space_size(){
  return seg_free_sz;
}
