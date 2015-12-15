
#ifndef POTATOCACHE_OS_HPP
#define POTATOCACHE_OS_HPP

#include <stdint.h>
#include <pthread.h>

#include "exceptions.hpp"

// Abstraction of os dependent stuff for easier conversion to other os with boost or whatever later.

namespace potatocache {

   // TODO Change to system_error?
   struct os_exception : public base_exception {};

   // Class representing a raw shared memory section including locking of it.
   struct shm
   {
      // Offset is the start offset for shmem usable outside this class. Offset 0 - offset is used for internal
      // bookkeping and should never be used outside it.
      static const uint64_t offset = sizeof(pthread_mutex_t);
      
      // Create a shm object.
      //
      // name: the name of the shared memory section alphanum and _ is allowed, length <= 30
      //
      // throws: std::invalid_argument on bad name, or os_exception if os not compatible
      shm(const std::string& name);
      
      // Create shared memory. If created the lock will also be initialized and locked on return.
      //
      // size: size in bytes
      //
      // returns: true if created (and opened), false if already existed
      //
      // throws: os_exception if failed to create
      bool create(uint64_t size);
   
      // Open existing shared memory.
      //
      // returns: true if opened, false if it did not exist
      //
      // throws: os_exception if it failed to open
      bool open();

      // Destroy shared memory section.
      //
      // throws: os_exception if failed to remove (will not throw if already removed)
      void remove();

      // Get the current size of the shared memory section.
      //
      // throws: os_exception if failed to get size
      uint64_t size();
      
      // Get a object pointer based on a byte.
      template<class T>
      T* ptr(uint64_t byte_offset) { return reinterpret_cast<T*>(_mem + byte_offset); }

      // Get a object reference based on a byte.
      template<class T>
      T& ref(uint64_t byte_offset) { return *reinterpret_cast<T*>(_mem + byte_offset); }

      // Lock shared memory section.
      void lock();
   
      // Unlock shared memory section.
      void unlock();

      virtual ~shm();

   private:

      std::string _name;
      int _fd;
      char* _mem;
      uint64_t _size;
   };

   // Used to lock shm with raii.
   struct shm_lock
   {
      shm_lock(shm& shm) : _shm(shm)
      {
         _shm.lock();
      }

      virtual ~shm_lock()
      {
         _shm.unlock();
      }

   private:
      shm& _shm;
   };

}

#endif
