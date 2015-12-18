#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <system_error>
#include <stdexcept>
#include <iostream>
#include <algorithm>

#include "os.hpp"
#include "utils.hpp"

#define MUTEX_PTR (&ref<internal_header>(0).mutex)

using namespace std;

namespace potatocache {
   
   //
   // Local functions.
   //

   char* map(const string& name, uint64_t size, int fd)
   {
      auto mem = static_cast<char*>(::mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
      if (mem == MAP_FAILED) {
         throw system_error(errno, system_category(), fmt("failed to mmap shared memory section %s", name.c_str()));
      }
      return mem;
   }

   void init_mutex(pthread_mutex_t* mutex)
   {
      pthread_mutexattr_t attr;

      int res;

      res = ::pthread_mutexattr_init(&attr);
      if (res) {
         throw system_error(res, system_category(), "failed to init attr for mutex");
      }

      res = ::pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
      if (res) {
         throw system_error(res, system_category(), "failed to set share attr for mutex");
      }
      
      res = ::pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
      if (res) {
         throw system_error(res, system_category(), "failed to set robust attr for mutex");
      }

      res = ::pthread_mutex_init(mutex, &attr);
      if (res) {
         throw system_error(res, system_category(), "failed to init mutex");
      }
   }

   struct internal_header
   {
      pthread_mutex_t mutex;
   };

   //
   // Shm class.
   //
   
   shm::shm(const std::string& name) :
      offset(sizeof(internal_header)),
      page_size(::sysconf(_SC_PAGESIZE)),
      _name('/' + name),
      _fd(-1),
      _mem(NULL)
   {
      auto len = name.size();
      if (len < 1 or 254 < len) {
         throw invalid_argument(fmt("name should be from 1 to 254 chars, \"%s\" is %u", name.c_str(), len));
      }
      
      if (::find_if(name.begin(), name.end(), [](char c) { return !(::isalnum(c) || (c == '_')); }) != name.end()) {
         throw invalid_argument(fmt("name should contain only alphanum and _, \"%s\" does not", name.c_str()));
      }
      
      if (sizeof(char) != 1) {
         throw runtime_error("this code does not work on systems where sizeof char != 1");
      }
   }

   bool shm::create(uint64_t size)
   {
      try {
         
         auto fd = ::shm_open(_name.c_str(), O_RDWR | O_CREAT | O_EXCL, 0700);
         if (fd < 0) {
            if (errno == EEXIST) {
               return false;
            }
            
            throw system_error(errno, system_category(),
                               fmt("failed to create shared memory section %s", _name.c_str()));
         }
         _fd = fd;

         if (::ftruncate(_fd, size) < 0) {
            throw system_error(errno, system_category(), fmt("failed to set size of shared object", _name.c_str()));
         }

         _mem = map(_name, size, _fd);

         ::memset(_mem, 0, size);

         init_mutex(MUTEX_PTR);
         // There is a timing issue here when the mutex is created but not locked, will be handled by status flag
         // outside this class.
         lock();
      }
      catch (const system_error& e) {
         remove();
         throw;
      }

      return true;
   }

   bool shm::open()
   {
      try {
      
         auto fd = ::shm_open(_name.c_str(), O_RDWR, 0700);
         if (fd < 0) {
            if (errno == ENOENT) {
               return false;
            }
            throw system_error(errno, system_category(), fmt("failed to open shared memory section %s"));
         }
         _fd = fd;

         auto size = shm::size();
         if (size < offset) {
            // Creating process in the middle of resizing or memory section in broken state. Clean up and return false
            // for later retry.
            close();
            return false;
         }
         _mem = map(_name, size , _fd);
         
      }
      catch (const system_error& e) {
         close();
         throw;
      }

      return true;
   }

   void shm::remove()
   {
      if (_mem) {
         auto mutex = MUTEX_PTR;
         ::pthread_mutex_unlock(mutex);
         ::pthread_mutex_destroy(mutex);
      }
      close();
      if (::shm_unlink(_name.c_str()) < 0) {
         if (errno != ENOENT) {
            throw system_error(errno, system_category(), fmt("failed unlink shared memory section %s", _name.c_str()));
         }
      }
   }

   uint64_t shm::size()
   {
      struct stat s;
      if (::fstat(_fd, &s) < 0) {
         throw system_error(errno, system_category(), fmt("failed to stat shared memory section %s", _name.c_str()));
      }
      return s.st_size;
   }

   void shm::lock()
   {
      if (not _mem) {
         return;
      }
      
      auto mutex = MUTEX_PTR;
      int res;

      res = ::pthread_mutex_lock(mutex);
      if (not res) {
         return;
      }
      
      if (res == EOWNERDEAD) {
         res = ::pthread_mutex_consistent(mutex);
         if (res) {
            throw system_error(res, system_category(), "failed to make mutex consistent");
         }
         return;
      }

      throw system_error(res, system_category(), "failed to lock mutex");
   }

   void shm::unlock()
   {
      if (not _mem) {
         return;
      }
      
      auto res = ::pthread_mutex_unlock(MUTEX_PTR);
      if (res) {
         throw system_error(res, system_category(), "failed to unlock mutex");
      }
   }

   pid_t shm::pid()
   {
      return getpid();
   }

   bool shm::process_exists(pid_t pid)
   {
      if (::kill(pid, 0)) {
         return false;
      }
      return true;
   }
   
   shm::~shm()
   {
      close();
   }

   void shm::close()
   {
      if (_mem) {
         ::munmap(_mem, size());
         _mem = NULL;
      }
      
      if (_fd != -1) {
         ::close(_fd);
         _fd = -1;
      }
   }
}
