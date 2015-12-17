#ifndef POTATOCACHE_SHARED_HPP
#define POTATOCACHE_SHARED_HPP

#include <stdint.h>

#define BLOCK_SIZE 64

// Data in the shared section.

namespace potatocache {

   // Enum for storing what operation is in progress.
   enum operation {
      
      // Create in progress but mem has just been created and cleared, mutex not even initialized yet. Recovery should
      // just recreted the shared memory section.
      create = 0,

      // No operation in progress. After getting lock, this should be the operation or recovery is needed.
      noop,
      
      // Initalization after create is in progress.
      init,

      // Getting of a value.
      get,

      // Putting a value.
      put,
      
   };
   
   // Header for shared memory segment, stored at offset in shared memory segment.
   struct mem_header {
      
      // Storing current operation for help with recovery if process is killed during operation.
      operation op;
      
      // Number of processes connected to the cache. This count may not be correct of cache did not shut down
      // gracefully.
      uint32_t process_count;

      // Pids connected to the cache. Used in conjunction with process_count to try too figure out when cahce should be
      // destroyed.
      pid_t pids[32];
      
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
      
      // First free block index, -1 if no free blocks.
      int64_t free_block_index;
   };
   
   // A hash table entry.
   struct hash_entry {
      
      // The hashed key.
      uint64_t hash;

      // The full key (0 terminated string -> max length 31 chars).
      char key[32];

      // Index of value block.
      int64_t value_index;

      // The size of the value in chars.
      uint64_t value_size;
   };

   // Memory block for storing values.
   struct block {

      // Index of next block, -1 if no more blocks in value (size is also usable).
      int64_t next_block_index;

      // The value in the block, no termination char.
      char data[BLOCK_SIZE - sizeof(uint64_t)]; 
   };

}

#endif
