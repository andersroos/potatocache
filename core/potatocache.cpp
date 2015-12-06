#include <sys/mman.h>
#include <fcntl.h>

#include "potatocache.hpp"
#include "shared.hpp"
#include "exceptions.hpp"
#include "utils.hpp"

using namespace std;

namespace potatocache {
   
   api::api(const std::string& name,
            const config& config) :
      _name(name),
      _config(config)
   {
      // TODO Name should start with '/', contain no other '/' and be max 255 chars.
      
      int fd = shm_open(name.c_str(), O_RDWR | O_CREAT, 0700);
      if (fd < 0) {
         // TODO Better exceptions.
         throw base_exception() << fmt("failed to open/create shared memory section %s, errno %d", name.c_str(), -fd);
      }
   }
}
