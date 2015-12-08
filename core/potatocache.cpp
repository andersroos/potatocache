#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>

#include "potatocache.hpp"
#include "shared.hpp"
#include "exceptions.hpp"
#include "utils.hpp"
#include "os.hpp"

using namespace std;

namespace potatocache {
   
   api::api(const std::string& name,
            const config& config) :
      _shm(name),
      _config(config)
   {
      // TODO Name should start with '/', contain no other '/' and be max 255 chars, check it.
      
      _shm.create(config.memory_segment_size);
   }
}
