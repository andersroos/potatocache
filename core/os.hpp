#pragma once

#include <stdint.h>
#include <pthread.h>

// Abstraction of os dependent stuff for easier conversion to other os with boost or whatever later.

namespace potatocache {

   // Class representing a raw shared memory section including lock handling for it.
   struct shm
   {
      // Offset is the start offset for shmem usable outside this class. Offset 0 - offset is used for internal
      // bookkeping and should never be used outside it.
      const uint64_t offset;

      // Page size of virtual memory pages.
      const uint64_t page_size;
      
      // Create a shm object.
      //
      // name: the name of the shared memory section alphanum and _ is allowed, length <= 254
      //
      // throws: std::invalid_argument on bad name, or std::system_error if os not compatible
      shm(const std::string& name);
      
      // Create shared memory. If created the lock will also be initialized and locked on return.
      //
      // size: size in bytes
      //
      // returns: true if created (and opened), false if already existed
      //
      // throws: std::system_error if failed to create
      bool create(uint64_t size);

      // Open existing shared memory.
      //
      // returns: true if opened, false if it did not exist or if not yet sized (= try again)
      //
      // throws: std::system_error if it failed to open
      bool open();

      // Destroy shared memory section.
      //
      // throws: std::system_error if failed to remove (will not throw if already removed)
      void remove();

      // Resize the shared memory segment to size, can only increase size. Recommended that is locked when doing it.
      // 
      // size: size in bytes to increase to
      //
      // throws: std::invalid_argument if current size is less than size, std::system_error if failed to resize
      void resize(uint64_t size);

      // Remap the memory segment to it's actual size. Can't be locked when calling this method.
      //
      // throws: std::system_error if failed to resize
      void remap_to_size();
      
      // Get the actual current size of the shared memory section by calling stat.
      //
      // throws: std::system_error if failed to get size
      uint64_t stat_size();

      // Get the current mapped size of the shared memory section.
      uint64_t size() const { return _size; }
      
      // Get a object pointer based on a byte.
      template<class T> inline
      T* ptr(uint64_t byte_offset) const { return reinterpret_cast<T*>(_mem + byte_offset); }

      // Get a object reference based on a byte.
      template<class T> inline
      T& ref(uint64_t byte_offset) const { return *reinterpret_cast<T*>(_mem + byte_offset); }

      // Lock shared memory section.
      //
      // throws: std::system_error if failed to lock (due to not initialized)
      void lock();
      
      // Unlock shared memory section.
      //
      // throws: std::system_error if failed to unlock (normally never fails)
      void unlock();

      // Get the pid if the process.
      //
      // returns: pid
      static pid_t pid();

      // Find out if process exists (note that pids are recycled).
      //
      // returns: true if exists, false otherwise (or if unable to find out)
      static bool process_exists(pid_t pid);
      
      // Unmaps and closes shared memory.
      virtual ~shm();
      
   private:

      // Munmap if mapped.
      void munmap();
      
      // Try to close and clean up as much as possible.
      void close();
      
      std::string _name;
      int _fd;
      char* _mem;
      uint64_t _size;
   };
   
   // Used to lock shm with raii. Will always unlock if not explicitly unlocked.
   struct shm_lock
   {
      shm_lock(shm& shm) : _shm(shm), _locked(false)
      {
         _shm.lock();
         _locked = true;
      }

      void unlock()
      {
         if (_locked) {
            _shm.unlock();
            _locked = false;
         }
      }

      virtual ~shm_lock()
      {
         unlock();
      }

   private:
      shm& _shm;
      bool _locked;
   };
}
