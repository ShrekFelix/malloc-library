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

void insert_freeLL(Block* p, Block* b){
  b->free = 1;
  if(!p){
    if(!head_loc){
      assert(!tail_loc);
      head_loc = b;
      tail_loc = b;
      b->next = NULL;
      b->prev = NULL;
    }else{
      head_loc->prev = b;
      b->next = head_loc;
      b->prev = NULL;
      head_loc = b;
    }
  }else{
    assert(p<b);
    if(tail_loc == p){
      tail_loc = b;
    }else{
      p->next->prev = b;
    }
    b->next = p->next;
    b->prev = p;
    p->next = b;
  }
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
  Block* p = head_loc;
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


Block* next_seg(Block* b){
  return (void*)b + b->size + BLK_SZ;
}

void merge_blocks_loc(Block* a, Block* b){
  if( a && b && b==next_seg(a)){
    a->size += b->size + BLK_SZ;
    remove_block_loc(b);
  }
}

void remove_block_loc(Block* b){
  b->free = 0;
  if(b != head_loc){
    b->prev->next = b->next;
  }else{ // b is head_loc and then head_loc is removed
    head_loc = head_loc->next; // replace head_loc
  }
  if(b != tail_loc){
    b->next->prev = b->prev;
  }else{ // b is tail_loc and then tail_loc is removed
    tail_loc = tail_loc->prev; // replace tail_loc
  }
  b->prev = NULL;
  b->next = NULL;
}

void *ts_malloc_nolock(size_t size){
  if (size <= 0) {
    return NULL;
  }
  Block* b = head_loc;
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
      best->size -= size + BLK_SZ;
      return nb+1;
    }else{
      remove_block_loc(best);
      return best+1;
    }
  }else{
    pthread_mutex_lock(&lock);
    Block* addr = sbrk(size + BLK_SZ);
    pthread_mutex_unlock(&lock);
    Block* nb = initialize_block(addr, size);
    return nb+1;
  }
}

void ts_free_nolock(void* ptr){
  if(!ptr){
    return;
  }
  Block* b = (Block*)ptr - 1; // locate block
  assert(!b->free);
  Block* p = tail_loc;
  while(p>b){
    p = p->prev;
  }
  insert_freeLL(p, b);
  merge_blocks_loc(b, b->next);
  merge_blocks_loc(b->prev, b);
}
