#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdexcept>
#include <iostream>
#include <algorithm>

#include "os.hpp"
#include "utils.hpp"
#include "shared.hpp"

using namespace std;

namespace potatocache {

   char* map(const string& name, uint64_t size, int fd) {
      char* mem = static_cast<char*>(mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
      if (mem == MAP_FAILED) {
         throw os_exception() << fmt("failed to mmap shared memory section %s, errno %d", name.c_str(), errno);
      }
      return mem;
   }

   
   shm::shm(const std::string& name) : _name('/' + name), _fd(-1), _mem(NULL), _size(0)
   {
      size_t len = name.size();
      if (len < 1 or 30 < len) {
         throw invalid_argument(fmt("name should be from 1 to 30 chars, \"%s\" is %u", name.c_str(), len));
      }
      
      if (find_if(++name.begin(), name.end(), [](char c) { return !(isalnum(c) || (c == '_')); }) != name.end()) {
         throw invalid_argument(fmt("name should contain only alphanum and _, \"%s\" does not", name.c_str()));
      }
      
      if (sizeof(char) != 1) {
         throw os_exception() << "this code does not work on systems where sizeof char != 1";
      }
   }

   bool shm::create(uint64_t size)
   {
      int fd = shm_open(_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0700);
      if (fd < 0) {
         if (errno == EEXIST) {
            return false;
         }
         
         throw os_exception() << fmt("failed to create shared memory section %s, errno %d", _name.c_str(), errno);
      }
      _fd = fd;

      if (ftruncate(_fd, size) < 0) {
         shm_unlink(_name.c_str());
         throw base_exception() << fmt("failed to set size of shared object, errno %d", _name.c_str(), errno);
      }

      _mem = map(_name, size, _fd);

      return true;
   }

   bool shm::open()
   {
      int fd = shm_open(_name.c_str(), O_RDWR, 0700);
      if (fd < 0) {
         if (errno == ENOENT) {
            return false;
         }
         throw os_exception() << fmt("failed to open shared memory section %s, errno %d", _name.c_str(), errno);
      }
      _fd = fd;

      // TODO Is there a timing issue here if other thread is between create and ftruncate? Fail if stat is below
      // threshold? Time to start c++ unittests I think. Use mutex to fix this?
      
      _mem = map(_name, size(), _fd);
      
      return true;
   }

   void shm::remove()
   {
      if (shm_unlink(_name.c_str()) < 0) {
         if (errno != ENOENT) {
            throw os_exception() << fmt("failed unlink shared memory section %s, errno %d", _name.c_str(), errno);
         }
      }
   }

   uint64_t shm::size()
   {
      struct stat s;
      if (fstat(_fd, &s) < 0) {
         throw os_exception() << fmt("failed to stat shared memory section %s, errno %d", _name.c_str(), errno);
      }
      return s.st_size;
   }
   
   shm::~shm()
   {
      if (_mem) {
         try {
            munmap(_mem, size());
         }
         catch (const os_exception& e) {
         }
      }
      if (_fd != -1) {
         close(_fd);
      }
   }
}




















