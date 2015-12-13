#include <sys/mman.h>
#include <sys/stat.h>
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

   bool shm::create(uint64_t size)
   {
      int fd = shm_open(_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0700);
      if (fd < 0) {
         if (errno == EEXIST) {
            return false;
         }
         
         throw os_exception() << fmt("failed to create shared memory section %s, errno %d", _name.c_str(), errno);
      }

      if (ftruncate(fd, size) < 0) {
         shm_unlink(_name.c_str());
         throw base_exception() << fmt("failed to set size of shared object, errno %d", _name.c_str(), errno);
      }

      _mem = static_cast<char*>(mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
      _fd = fd;

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

      // TODO Is there a timing issue here if other thread is between create and ftruncate? Fail if stat is below
      // threshold? Time to start c++ unittests I think. Use mutex to fix this?
      
      struct stat s;
      if (fstat(fd, &s) < 0) {
         throw os_exception() << fmt("failed to stat shared memory section %s, errno %d", _name.c_str(), errno);
      }
      
      _mem = static_cast<char*>(mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
      _fd = fd;
      
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
   
}




















