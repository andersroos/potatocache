#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
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
         throw base_exception() << fmt("failed to open/create shared memory section %s, errno %d", name.c_str(), errno);
      }

      if (ftruncate(fd, config.memory_segment_size) < 0) {
         throw base_exception() << fmt("failed to set size of shared object, errno %d", name.c_str(), errno);
      }
      cerr << "size is " << config.memory_segment_size << endl;
      
      // TODO Check for min size.
      _mem = mmap(NULL, config.memory_segment_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

      cerr << "mem is " << _mem << " data is " << *static_cast<uint64_t*>(_mem) << endl;
      
      // TODO Created or connected? How to know if structures should be initied. C++ init possible?

      // TODO Use malloc? Possible?

      // TODO Lock is needed.

      // TODO Is same mem mapped to different addresses? Then we need to work with offsets.
      mem_header* header = static_cast<mem_header*>(_mem);
      header->mem_size = config.memory_segment_size;
      header->hash_offset = sizeof(mem_header);
      header->hash_size = 0;
      header->blocks_offset = 0;
      header->blocks_size = 0;
      header->blocks_free = 0;
      header->free_block_offset = header->blocks_offset;
   }
}
