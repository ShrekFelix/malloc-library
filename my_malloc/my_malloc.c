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
  while(1){
    pthread_rwlock_rdlock(&lock);
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
    pthread_rwlock_unlock(&lock);
    pthread_rwlock_wrlock(&lock);
    if(!best || !best->free || best->size < size){
      pthread_rwlock_unlock(&lock);
      continue;
    }
    if(best){
      if(best->size > BLK_SZ + size){
        // split the block
        Block* addr = (void*) best + best->size - size;
        Block* nb = initialize_block(addr, size);
        insert_physLL(best, nb);
        best->size -= size + BLK_SZ;
        pthread_rwlock_unlock(&lock);
        return nb+1;
      }else{
        remove_block_freeLL(best);
        pthread_rwlock_unlock(&lock);
        return best+1;
      }
    }else{
      Block* addr = sbrk(size + BLK_SZ);
      Block* nb = initialize_block(addr, size);
      insert_physLL(phys_tail, nb);
      pthread_rwlock_unlock(&lock);
      return nb+1;
    }
  }
}

void ts_free_lock(void* ptr){
  pthread_rwlock_wrlock(&lock);
  if(!ptr){
    return;
  }
  Block* b = (Block*)ptr - 1; // locate block
  extend_freeLL(b);
  merge_blocks(b, b->phys_next);
  merge_blocks(b->phys_prev, b);
  pthread_rwlock_unlock(&lock);
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
