
#ifndef POTATOCACHE_POTATOCACHE_HPP
#define POTATOCACHE_POTATOCACHE_HPP

namespace potatocache {

   uint32_t UNLIMITED = 0;
   uint32_t USE_DEFAULT = 0xDEFA;
   
   // Configration object for the cache (0 means unlimited unless othervise stated).
   struct config {
      config()
         : default_soft_timeout_us(UNLIMITED),
           default_hard_timeout_us(UNLIMITED),
           default_stale_update_timeout_us(2e6),  // Time before first stale response until next client get stale flag.
           initial_count(1024),
           max_count(UNLIMITED),
           memory_segment_size(2 * 1024 * 1024),
           initial_memory_segment_count(1),
           max_memory_segment_count(UNLIMITED)
      {}
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
      // stale_out: set to true if the getter is supposed to calculate a new value, when stale the first getter of the
      //            key will get stale_out=true, the subsequent getters will get stale_out=false unless
      //            stale_update_timout_us has passed
      //
      // stale_us_out: number of micro seconds the entry have been stale for (soft timeouted), 0 if not stale, this
      //               value is not affected by stale_out flag, so when stale stale_us_out will be > 0 but stale_out may
      //               be false if someone did a call and received stale_out=true
      //
      // missing_out: set to true if the key was not present in the cache
      //
      // returns: the value, will be empty if missing_out == true
      std::string get(const std::string& key);

      // Put value into cache.
      //
      // key: the key for the value TODO key max size
      //
      // value: the value to put into the cache TODO size configration
      //
      // soft_timeout_us: time after put when entry becomes stale (see get)
      //
      // hard_timeout_us: time after when entru is removed from cache
      //
      // stale_update_timout_us: when stale, time after the get caller gets stale_out=true until the next caller will
      //                         get stale_out=true, i.e. the time the first caller got to renew the entry
      void put(const std::string& key,
               const std::string& value,
               uint64_t soft_timeout_us=USE_DEFAULT,
               uint64_t hard_timeout_us=USE_DEFAULT,
               uint64_t stale_update_timeout_us=USE_DEFAULT);
      
   private:
   };
   
}

#endif
