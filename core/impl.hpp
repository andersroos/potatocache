
#ifndef POTATOCACHE_IMPL_HPP
#define POTATOCACHE_IMPL_HPP

#include <string>

#include "config.hpp"
#include "os.hpp"

// The actual cache implementation. The public methods are duplicated form potatocache::api.
namespace potatocache {

   struct impl {

      impl(const std::string& name, const config& config);

      std::string get(const std::string& key, bool& missing_out);
      
      void put(const std::string& key, const std::string& value);

      virtual ~impl();
      
   private:

      shm _shm;
      config _config;
   };
   
}

#endif
