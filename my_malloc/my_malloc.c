#include "my_malloc.h"
#include <stdio.h>
#include <assert.h>
#define BLK_SZ (sizeof(Block))

Block* initialize_block(Block* b, size_t size){
  b->size = size;
  b->next = NULL;
  b->prev = NULL;
  b->free = 0;
  return b;
}

void extend_freeLL(Block* b){
  b->free = 1;
  if(tail){
    tail->next = b;
  }else{
    head = b;
  }
  b->prev = tail;
  b->next = NULL;
  tail = b;
}

void insert_physLL(Block* p, Block* b){
  if(!p){ // called insert_physLL(phys_tail, b) when phys_tall==NULL
    assert(!phys_head && !phys_tail);
    phys_head = b;
    phys_tail = b;
    b->phys_next = NULL;
    b->phys_prev = NULL;
  }else{
    if(phys_tail == p){
      phys_tail = b;
    }else{
      p->phys_next->phys_prev = b;
    }
    b->phys_next = p->phys_next;
    b->phys_prev = p;
    p->phys_next = b;
  }
}

void remove_block_freeLL(Block* b){
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
  b->prev = NULL;
  b->next = NULL;
}
void remove_block_physLL(Block* b){
  b->free = 0;
  if(b != phys_head){
    b->phys_prev->phys_next = b->phys_next;
  }else{ // b is head and then head is removed
    phys_head = phys_head->phys_next; // replace head
  }
  if(b != phys_tail){
    b->phys_next->phys_prev = b->phys_prev;
  }else{ // b is tail and then tail is removed
    phys_tail = phys_tail->phys_prev; // replace tail
  }
  b->phys_prev = NULL;
  b->phys_next = NULL;
}

void merge_blocks(Block* a, Block* b){
  if( a && b && a->free && b->free){
    a->size += b->size + BLK_SZ;
    remove_block_freeLL(b);
    remove_block_physLL(b);
  }
}

//Best Fit malloc/free
void *ts_malloc_lock(size_t size){
  if (size <= 0) {
    return NULL;
  }
  pthread_mutex_lock(&lock);
  Block* b = head;
  Block* best = NULL;
  while(b){
    assert(b->free);
    if( b->size >= size ){ // found a free block big enough
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
    if(best->size > BLK_SZ + size){
      // split the block
      Block* addr = (void*) best + best->size - size;
      Block* nb = initialize_block(addr, size);
      insert_physLL(best, nb);
      best->size -= size + BLK_SZ;
      pthread_mutex_unlock(&lock);
      return nb+1;
    }else{
      remove_block_freeLL(best);
      pthread_mutex_unlock(&lock);
      return best+1;
    }
  }else{
    Block* addr = sbrk(size + BLK_SZ);
    Block* nb = initialize_block(addr, size);
    insert_physLL(phys_tail, nb);
    pthread_mutex_unlock(&lock);
    return nb+1;
  }
}

void ts_free_lock(void* ptr){
  pthread_mutex_lock(&lock);
  if(!ptr){
    return;
  }
  Block* b = (Block*)ptr - 1; // locate block
  extend_freeLL(b);
  merge_blocks(b, b->phys_next);
  merge_blocks(b->phys_prev, b);
  pthread_mutex_unlock(&lock);
}

void freeLL_summary(){
  printf("freeLL\n");
  Block* p = head;
  while(p){
    printf("%p : %lu\n",p,p->size);
    p = p->next;
  }
  printf("\n");
}
void physLL_summary(){
  printf("physLL\n");
  Block* p = phys_head;
  while(p){
    printf("%p : %d %lu\n",p,p->free,p->size);
    p = p->phys_next;
  }
  printf("\n");
}

unsigned long get_data_segment_free_space_size(){
  Block* p = head;
  unsigned long sum = 0;
  while(p){
    sum += p->size;
    p = p->next;
  }
  return sum;
}
unsigned long get_data_segment_size(){
  Block* p = phys_head;
  unsigned long sum = 0;
  while(p){
    sum += p->size;
    p = p->phys_next;
  }
  return sum;;
}



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