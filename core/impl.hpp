
#ifndef POTATOCACHE_IMPL_HPP
#define POTATOCACHE_IMPL_HPP

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
      inline mem_header_t& head() { return _shm.ref<mem_header_t>(_shm.offset); }

      // Access hash entry (head needs to be initialized first).
      inline hash_entry_t& entry(uint64_t index)
      {
         return _shm.ref<hash_entry_t>(head().hash_offset + sizeof(hash_entry_t) * index);
      }

      // Access block (head needs to be initialized first).
      inline block_t& block(uint64_t index)
      {
         return _shm.ref<block_t>(head().blocks_offset + sizeof(block_t) * index);
      }
      
      // Return page aligned offset.
      uint64_t align(uint64_t offset);
      
      // Check if recover is needed, try to fix broken cache. Should be locked when calling.
      //
      // returns: true if no recover needed to if successful recover, false if it is not possible
      bool recover_p();
      
      // Open an existing, we have lock and should just check pids.
      void open();
      
      // Shared memory just created, need to set up all data structures.
      void create();

      shm _shm;
      config _config;
   };
   
}

#endif
