#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>

#include "os.hpp"
#include "utils.hpp"
#include "shared.hpp"

using namespace std;

namespace potatocache {
   
   void shm::create(uint64_t size)
   {
      _fd = shm_open(_name.c_str(), O_RDWR | O_CREAT, 0700);
      if (_fd < 0) {
         // TODO Better exceptions.
         throw os_exception() << fmt("failed to open/create shared memory section %s, errno %d", _name.c_str(), errno);
      }

      if (ftruncate(_fd, size) < 0) {
         throw base_exception() << fmt("failed to set size of shared object, errno %d", _name.c_str(), errno);
      }
      cerr << "size is " << size << endl;
      
      // TODO Check for min size.
      _mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);

      cerr << "mem is " << _mem << " data is " << *static_cast<uint64_t*>(_mem) << endl;
      
      // TODO Created or connected? How to know if structures should be initied. C++ init possible?

      // TODO Use malloc? Possible?

      // TODO Lock is needed.

      // TODO Is same mem mapped to different addresses? Then we need to work with offsets.
      mem_header* header = static_cast<mem_header*>(_mem);
      
      header->mem_size = size;
      header->hash_offset = sizeof(mem_header);
      header->hash_size = 0;
      header->blocks_offset = 0;
      header->blocks_size = 0;
      header->blocks_free = 0;
      header->free_block_offset = header->blocks_offset;
   }
}




















