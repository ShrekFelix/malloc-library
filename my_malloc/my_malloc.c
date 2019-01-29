#include "my_malloc.h"
#include <stdio.h>
#include <assert.h>
#define BLK_SZ (sizeof(Block))

Block* create_block(size_t size){
  // initialize the block
  Block* b = sbrk(size + BLK_SZ); // request space from kernel
  if ((int)b == -1) { // sbrk failed
    perror("sbrk failed\n");
    return NULL;
  }
  b->size = size;
  b->next = NULL;
  b->prev = NULL;
  b->free = 0;
  extend_physLL(b);
  return b;
}

void extend_freeLL(Block* b){
  assert(!b->free);
  assert(!b->prev);
  assert(!b->next);
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
void extend_physLL(Block* b){
  if(phys_tail){
    phys_tail->phys_next = b;
  }else{
    phys_head = b;
  }
  b->phys_prev = phys_tail;
  b->phys_next = NULL;
  phys_tail = b;
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
void *bf_malloc(size_t size){
  if (size <= 0) {
    return NULL;
  }
  Block* b = head;
  Block* best = NULL;
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
    if(best->size > BLK_SZ + size){
      // split the block
      Block* nb = (void*) best + BLK_SZ + size;
      nb->size = best->size - size;
      extend_freeLL(nb);
      nb->phys_next = best->phys_next;
      nb->phys_prev = best;
      best->phys_next = nb;
      if(nb->phys_next){
        nb->phys_next->phys_prev = nb;
      }
    }
    remove_block_freeLL(best);
    return best+1;
  }
  // no available block, get a new one
  b = create_block(size);
  if(!b){ // sbrk failed
    return NULL;
  }
  return b+1; // jump off header
}

void bf_free(void* ptr){
  if(!ptr){
    return;
  }
  Block* b = (Block*)ptr - 1; // locate block
  extend_freeLL(b);
  merge_blocks(b, b->phys_next);
  merge_blocks(b->phys_prev, b);
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
