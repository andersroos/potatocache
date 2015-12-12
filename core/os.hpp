
#ifndef POTATOCACHE_OS_HPP
#define POTATOCACHE_OS_HPP

#include <stdint.h>

#include "exceptions.hpp"

// Abstraction of shared memory and locks for easier conversion to other os with boost or whatever later.

namespace potatocache {

   struct os_exception : public base_exception {};

   // Class representing a shared memory section.
   struct shm
   {
      // Create a shm object with a name.
      shm(const std::string& name);
      
      // Create shared memory.
      //
      // size: size in bytes
      //
      // returns: true if created (and opened), false if already existed
      //
      // throws: os_exception if already exists or if failed to create
      bool create(uint64_t size);
   
      // Open existing shared memory.
      //
      // returns: true if opened, false if it did not exist
      //
      // throws: os_exception if it failed to open
      bool open();

      // Destroy shared memory section.
      void remove();

      // Lock shared memory section.
      void lock();

      // Unlock shared memory section.
      void unlock();

      // Get a object pointer based on a byte.
      template<class T>
      T* ref(uint64_t byte_offset) { return reinterpret_cast<T*>(_mem + byte_offset); }
      
      virtual ~shm() {};
      
   private:

      std::string _name;
      int _fd;
      char* _mem;
      uint64_t _size;
   };

}

#endif
