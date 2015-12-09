#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>

#include "os.hpp"
#include "utils.hpp"
#include "shared.hpp"

using namespace std;

namespace potatocache {

   shm::shm(const std::string& name) : _name(name), _fd(-1), _mem(NULL), _size(0)
   {
      if (sizeof(char) != 1) {
         throw os_exception() << "this code does not work on systems where sizeof char != 1";
      }
   }

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
      _mem = static_cast<char*>(mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0));

      cerr << "mem is " << _mem << " data is " << *reinterpret_cast<uint64_t*>(_mem) << endl;
   }
}




















