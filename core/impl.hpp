#pragma once

#include <string>
#include <iostream>

#include "config.hpp"
#include "os.hpp"
#include "shared.hpp"

// The actual cache implementation. The public methods are duplicated form potatocache::api.
namespace potatocache {

   struct impl {

      //
      // Public api.
      //
      
      impl(const std::string& name, const config& config);

      std::string get(const std::string& key, bool& missing_out);
      
      void put(const std::string& key, const std::string& value);

      virtual ~impl();

      //
      // Internal API.
      //

      // Acces mem header.
      inline mem_header_t& head() const { return _shm.ref<mem_header_t>(_shm.offset); }

      // Access hash entry (head needs to be initialized first).
      inline hash_entry_t& entry(uint64_t index)
      {
         return _shm.ref<hash_entry_t>(head().hash_offset + sizeof(hash_entry_t) * index);
      }

      // Return the byte offset for the block.
      inline uint64_t block_o(uint64_t index) const
      {
         return head().blocks_offset + sizeof(block_t) * index;
      }
      
      // Access block (head needs to be initialized first).
      inline block_t& block(uint64_t index) const
      {
         return _shm.ref<block_t>(block_o(index));
      }
      
      // Return page aligned offset.
      uint64_t align(uint64_t offset);
      
      // Check if recover is needed, try to fix broken cache. Should be locked when calling.
      //
      // returns: true if no recover needed to if successful recover, false if it is not possible
      bool recover_p();
      
      // Open an existing, we have lock and should just check pids.
      //
      // returns: process count after open
      uint32_t open();
      
      // Shared memory just created, need to set up all data structures.
      void create();

      // Hash key to an int.
      uint64_t hash(const std::string& key);

      // Dump shared memory content on stdout.
      void dump();
 
      // Return the size in bytes of the mapped section (not the same as the shared memory size).
      uint64_t size() { return _shm.size(); }
     
      std::string _name;
      shm _shm;
      config _config;
   };
   
}
