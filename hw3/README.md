## This program is an implemention of dynamic memory allocator.

### how it works:

* Blocks are assigned following a best fit protocol.
* Each block maintains a header and footer that stores information about the block. This adds internal fragmentation, however we get a positive tradeoff when coalescing free blocks.
* The program implements bidirectional coalescing to reduce external fragmentation.
* It maintains 4 explicit free lists, each list maintains pointers to a certain range of free block sizes.
