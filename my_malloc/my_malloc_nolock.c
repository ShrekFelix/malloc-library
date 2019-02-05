#include "my_malloc_nolock.h"
#include <assert.h>
#define BLK_SZ2 (sizeof(struct Block2))

struct Block2* create_block2(size_t size){
  // initialize the block
  pthread_mutex_lock(&lock);
  struct Block2* b = sbrk(size + BLK_SZ2); // request space from kernel
  pthread_mutex_unlock(&lock);
  if ((int)b == -1) { // sbrk failed
    return NULL;
  }
  b->size = size;
  b->next = NULL;
  b->prev = NULL;
  b->free = 0;
  return b;
}

struct Block2* next_seg2(struct Block2* b){
  return (void*)b + b->size + BLK_SZ2;
}

void merge_blocks2(struct Block2* a, struct Block2* b){
  if( a && b && b==next_seg2(a)){
    a->size += b->size + BLK_SZ2;
    remove_block2(b);
  }
}

// remove b from LL
void remove_block2(struct Block2* b){
  b->free = 0;
  if(b != head2){
    b->prev->next = b->next;
  }else{ // b is head2 and then head2 is removed
    head2 = head2->next; // replace head2
  }
  if(b != tail2){
    b->next->prev = b->prev;
  }else{ // b is tail2 and then tail2 is removed
    tail2 = tail2->prev; // replace tail2
  }
}

void ts_free_nolock(void* ptr){
  if(!ptr){
    return;
  }
  struct Block2* b = (struct Block2*)ptr - 1; // locate block
  //assert(!b->free);
  b->free = 1;
  if(!head2){ // empty LL
    //assert(!tail2);
    head2 = b;
    tail2 = b;
    b->prev = NULL;
    b->next = NULL;
  }else if(b > tail2){
    // b should be physically the last free block in the LL
    // insert b to end of LL
    tail2->next = b;
    b->prev = tail2;
    b->next = NULL;
    tail2 = b;
  }else{
    // look for a free block physically after b
    struct Block2* p = next_seg2(b);
    while(p){
      if(p->free){
        b->prev = p->prev;
        b->next = p;
        p->prev = b;
        if(b->prev){
          b->prev->next = b;
        }else{
          head2 = b;
        }
        break;
      }
      p = next_seg2(p);
    }
  }
  merge_blocks2(b, b->next);
  merge_blocks2(b->prev, b);
}

void *ts_malloc_nolock(size_t size){
  if (size <= 0) {
    return NULL;
  }
  struct Block2* b = head2;
  struct Block2* best = NULL;
  while(b){
    //assert(b->free); // LL only tracks free blocks
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
    remove_block2(best);
    return best+1;
  }
  // no available block, get a new one
  b = create_block2(size);
  return b+1; // user's space starts from (void*)b + BLK_SZ2
}