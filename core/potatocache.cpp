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
      
      // TODO Created or connected? How to know if structures should be initied. C++ init possible?

      // TODO Use malloc? Possible?

      // TODO Lock is needed.

      // TODO Is same mem mapped to different addresses? Then we need to work with offsets.
      mem_header* header = _shm.ref<mem_header>(0);
      
      header->mem_size = config.memory_segment_size;
      header->hash_offset = sizeof(mem_header);
      header->hash_size = 0;
      header->blocks_offset = 0;
      header->blocks_size = 0;
      header->blocks_free = 0;
      header->free_block_offset = header->blocks_offset;
      
   }
}
