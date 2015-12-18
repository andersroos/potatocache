
#ifndef POTATOCACHE_IMPL_HPP
#define POTATOCACHE_IMPL_HPP

#include <string>

#include "config.hpp"
#include "os.hpp"
#include "shared.hpp"

// The actual cache implementation. The public methods are duplicated form potatocache::api.
namespace potatocache {

   struct impl {

      impl(const std::string& name, const config& config);

      std::string get(const std::string& key, bool& missing_out);
      
      void put(const std::string& key, const std::string& value);

      virtual ~impl();
      
   private:

      // Acces mem header.
      inline mem_header& head() { return _shm.ref<mem_header>(_shm.offset); }
      
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
