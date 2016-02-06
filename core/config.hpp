#pragma once

#include <stdint.h>

// This is the configuration object for potatocache the default config is supposed to be adequate for most situations.
namespace potatocache {
   
   // Configration object for the cache.
   struct config {
      
      // Size in number of entries.
      uint64_t size = 1024;
      
      // Initial size of the shared mem segment to use for the chache, this is also the increment used when resizing the
      // cache.
      uint64_t memory_size_initial = 2 * 1024 * 1024;

      // Max size of the shared mem segment to use for the chache. 0 is infinite.
      uint64_t memory_size_max = 0;
      
   };

}
