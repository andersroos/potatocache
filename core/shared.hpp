#ifndef POTATOCACHE_SHARED_HPP
#define POTATOCACHE_SHARED_HPP

#include <stdint.h>

namespace potatocache {

   // Memory block for storing values.
   struct block {

      // Pointer to next block, null if no more blocks in value.
      uint64_t next_block_offset;

      // The value in the block, no termination char.
      char data[1024 - sizeof(uint64_t)]; 
   };

   // A hash table entry.
   struct hash_entry {
      
      // The hashed key.
      uint64_t hash;

      // The full key (0 terminated string -> max length 31 chars).
      char key[32];

      // Offset of value block.
      uint64_t value_offset;

      // The size of the value in chars.
      uint32_t value_size;
   };

   // Header for shared memory segment, stored first in shared memory segment.
   struct mem_header {
      // Size of shared memory in bytes.
      uint64_t mem_size;
      
      // Start byte of hash table.
      uint64_t hash_offset;
      
      // Number of elementes in hash table.
      uint32_t hash_size;

      // Start byte of blocks.
      uint64_t blocks_offset;

      // Number of bocks.
      uint32_t blocks_size;

      // Number of free blocks.
      uint32_t blocks_free;

      // First free block offset, 0 if no free blocks.
      uint64_t free_block_offset;
   };
}

#endif
