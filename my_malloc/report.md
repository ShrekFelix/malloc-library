# implementation

The main idea is to call `sbrk()` to allocate heap space via incrementing the break of the process. In order to reuse freed space, I have to track each segment I requested from `sbrk()`. I stored the meta data each time I request space and used a linked list to track all free blocks of segment meta. Then each time I recycle used segment, I add that block meta to my linked list.

```c
struct Block {
  struct Block* next;
  struct Block* prev;
  size_t size;
  int free;
};
```

There are 2 ways of allocation strategies: First Fit and Best Fit. For each allocation strategy,  I iterate through the linked list to find a block that is big enough as the caller asked, if not, I request space with `sbrk()`. The only difference is, for First Fit, once I found a satisfying block, I return it immediately. But for Best Fit, I'll iterate through all free blocks (unless I encounter a block with the same size as the requested size, in that case there cannot be a smaller satisfying block) and return the smallest one.

As I recycle a used block, I check the block's adjacent free block to see if they are also physically adjacent. If so, I merge these blocks by manipulating pointers in the linked list.

# test result

|                        | First Fit | Best Fit |
| :--------------------: | :-------: | :------: |
|   equal size allocs    |           |          |
| small range rand alloc |           |          |
| large range rand alloc |           |          |



# thoughts

