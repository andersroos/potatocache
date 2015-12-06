
#ifndef POTATOCACHE_POTATOCACHE_HPP
#define POTATOCACHE_POTATOCACHE_HPP

#include <stdint.h>
#include <string>

// See 9a0617dd8a259b6eda06c3fa8949f1c86231fe9a for full api sketch.

// The contents of this file is considered public api, the rest of the code is not.
namespace potatocache {
   
   // Configration object for the cache (0 means unlimited unless othervise stated).
   struct config {
      
      // Size in number of entries.
      uint64_t size = 1024;
      
      // Size of the shared mem segment to use for the chache.
      uint64_t memory_segment_size = 2 * 1024 * 1024;
   };

   // Main api class for communicating with potatocache. All methods are atomic if not othervise stated and may safely
   // be called in a multithreaded environment.
   struct api {

      // Create an api instance for communication with a potatocache in shared memory. If there is no cache present with
      // that name created one will be created.
      //
      // name: the key for the cache, used to share cache among processes
      //
      // config: the cache config, see cache class above
      api(const std::string& name,
          const config& config);
      
      // Get value from cache.
      //
      // key: the key for the value
      //
      // missing_out: set to true if key does not exist in cache, otherwise set to false
      //
      // returns: the value, will be empty string if missing_out == true
      std::string get(const std::string& key,
                      bool& missing_out);

      // Put value into cache.
      //
      // key: the key for the value
      //
      // value: the value to put into the cache
      //
      // throws: potatocache::exception if key or value are too large or if cache is full
      void put(const std::string& key,
               const std::string& value);

   private:
      
      std::string _name;
      config _config;
   };
   
}

#endif
