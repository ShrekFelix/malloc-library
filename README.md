# implementation

There are 2 versions of thread-safe malloc implementation: one uses mutex lock and the other doesn't except when calling `sbrk()`, which is not thread safe itself. I built up these 2 versions upon my previous non-thread-safe implementation.

- original non-thread-safe implementation

  - Request space from kernel by calling `sbrk()` to increase the break of the process. 
  - Store the meta data of the allocated space in the beginning of the allocated space. 
  - Track all blocks of meta data using a double linked list structure. 
    - There are 2 sets of pointers for each block - one tracking the next/previous free block, the other tracking the next/previous physically adjacent block. 
    - Freeing and mallocing space would be maintaining the blocks in the linked list, unless there are no free blocks left - then request new space and add that to the linked list.

- locking version adaptation

  Using mutex lock would be the simplist solution. Just aquire a lock when entering the malloc/free functions and release it when exiting. Usually the performance of mutex lock parallelism could be improved by using read/write lock pattern. However, in this particular implementation it makes things awfully complicated. I didn't use that pattern.

- non-locking version adaptation

  This implementation requires the use of thread-local storage. In other words, threads maintain their own list of blocks. Note this list should only track free blocks ordered by physical address. This is because allocated memory space is all shared among (read and written by) threads and can not be tracked by any thread locally. 

  The work for each thread would be:

  - free

    Recycle the block represented by this address from the public allocated space and updating its local linked list of free blocks

  - malloc

    Find the best block from its local linked list of free blocks, split it if possible, and return to the caller. If no such block is found, call `sbrk()`

  Since I tracked physically adjacent blocks in my original implementation, for this non-locking malloc function I re-built the `bf_malloc()` that only tracks free blocks.

# test performance

The performance of both versions are:

|          Version          | Locking  | Non-locking |
| :-----------------------: | :------: | :---------: |
| execution time (seconds)  | 3.155424 |  0.319438   |
| data segment size (bytes) | 44801040 |  45029296   |

The locking version is significantly slower than the non-locking version. As it serializes the entire process, the threads in it spend most time waiting to acquire the lock, which is inefficient. The non-locking version doesn't have this problem, but it takes more space. This is because each thread only access part of the whole free blocks and will have to request space from kernel more often. However, as I'm using best allocation strategy and split/merge blocks, the space efficiency difference between these 2 versions is not huge.
