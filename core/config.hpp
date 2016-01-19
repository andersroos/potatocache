#pragma once

#include <stdint.h>

// This is the configuration object for potatocache the default config is supposed to be adequate for most situations.
namespace potatocache {
   
   // Configration object for the cache.
   struct config {
      
      // Size in number of entries.
      uint64_t size = 1024;
      
      // Size of the shared mem segment to use for the chache.
      uint64_t memory_segment_size = 2 * 1024 * 1024;

   };

}
