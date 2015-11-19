
#ifndef POTATOCACHE_POTATOCACHE_HPP
#define POTATOCACHE_POTATOCACHE_HPP

namespace potatocache {
   
   // Configration object for the cache (0 means unlimited unless othervise stated).
   struct config {
      config()
         : default_soft_timeout_us(0),
           default_hard_timeout_us(0),
           default_stale_update_timeout_us(2e6),  // How long time before first stale response until next client get stale flag.
           initial_count(1024),
           max_count(0),
           memory_segment_size(2 * 1024 * 1024),
           initial_memory_segment_count(1),
           max_memory_segment_count(0)
      {}
   };
      
   struct api {

      // Create an api instance for communication with a potatocache in shared memory. If there is no cache present with
      // that name created one will be created.
      //
      // name: the key for the cache
      api(const std::string& name,
          const config& config);
      
      // Get value from cache.
      std::string get(const std::string& key,
                      bool stale);

      // Put value into cache.
      std::string put(const std::string& key,
                      const std::string& value,
                      uint64_t soft_timeout_us=0,
                      uint64_t hard_timeout_us=0);
      
   private:
   };
   
}

#endif
