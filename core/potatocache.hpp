#pragma once

#include "config.hpp"
#include "impl.hpp"
#include "log.hpp"

// See 9a0617dd8a259b6eda06c3fa8949f1c86231fe9a for full api sketch.

// The contents of this file and config.hpp is considered public api, the rest of the code is not.
namespace potatocache {

   // Main api class for communicating with potatocache. All methods are atomic if not othervise stated and may safely
   // be called in a multithreaded environment.
   struct api {

      // Create an api instance for communication with a potatocache in shared memory. If there is no cache present with
      // that name created one will be created.
      //
      // name: the key for the cache, used to share cache among processes, max 254 chars alphanum and _
      //
      // config: the cache config, see config class above, config is used by the process that creates the cache
      // processes that opens a created cache can't configure it
      //
      // throws: various std::exception on irrecoverable errors
      api(const std::string& name, const config& config) : _impl(name, config) {}
      
      // Get value from cache.
      //
      // key: the key for the value
      //
      // missing_out: set to true if key does not exist in cache, otherwise set to false
      //
      // returns: the value, will be empty string if missing_out == true
      std::string get(const std::string& key, bool& missing_out) { return _impl.get(key, missing_out); }

      // Put value into cache.
      //
      // key: the key for the value, max length 31
      //
      // value: the value to put into the cache
      //
      // throws: invalid_argument if key is too large and length_error if value is too large or if cache is full
      void put(const std::string& key, const std::string& value) { _impl.put(key, value); }

      // Set ostream for logging. This is set globally for all instances in this process, hence static. By default the
      // logging is turned off.
      //
      // stream: out stream to log to, set to NULL to turn off
      static void set_log_output(std::ostream* stream) { potatocache::set_log_output(stream); }

      // Set log level. This is set globally for all instances in this process, hence static. Default is INFO.
      static void set_log_level(const log_level& level) { potatocache::set_log_level(level); }

      // Removes the cache if last process connected.
      virtual ~api() {}
      
   private:
      impl _impl;
   };
   
}
