#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <stdexcept>

#include "potatocache.hpp"
#include "shared.hpp"
#include "exceptions.hpp"
#include "utils.hpp"
#include "os.hpp"
#include "shared.hpp"

using namespace std;

#define HEAD _shm.ref<mem_header>(_shm.offset)

namespace potatocache {

   // Raii class for setting operation. Bware, if code inside block can throw this should not be used.
   struct op_set
   {
      op_set(shm& shm, operation op) : _shm(shm)
      {
         HEAD.op = op;
      }

      virtual ~op_set()
      {
         HEAD.op = op_noop;
      }

   private:
      shm& _shm;
   };

   // Return page aligned offset.
   // TODO Test this.
   uint64_t align(uint64_t page_size, uint64_t offset)
   {
      uint64_t rest = offset % page_size;
      if (rest) {
         return offset + page_size - rest;
      }
      return offset;
   }
   
   // Check if recover is needed, try to fix broken cache.
   //
   // returns: true if no recover needed to if successful recover, false if it is not possible
   bool recover_p(shm& _shm)
   {
      switch (HEAD.op) {

         case op_noop:
            // The normal case.
            return true;
            
         case op_create:
            // Create here means we have hit a super small create timing window or a dead creato, not recoverable.
            return false;
            
         case op_init:
            // We have a dead creator, not recoverable.
            return false;

         default:
            throw logic_error(fmt("recover could not handle state %d, this is a bug", HEAD.op));
      }
   }

   // Open an existing, we have lock and should just check pids.
   void open(shm& _shm)
   {
      op_set set(_shm, op_open);
      // TODO Check other pids, add own pid.
   }

   // Shared memory just created, need to set up all data structures.
   void create(shm& _shm)
   {
      op_set set(_shm, op_init);
      
      
   }
   
   api::api(const std::string& name,
            const config& config) :
      _shm(name),
      _config(config)
   {
      uint64_t required_size = _shm.offset + sizeof(mem_header);
      required_size = align(_shm.page_size, required_size + sizeof(hash_entry) * _config.size);
      required_size += sizeof(block) * _config.size;

      if (required_size > _config.memory_segment_size) {
         throw std::invalid_argument(fmt("too litte memory configured, absolute minimum size needed is %lu,"
                                         " configured is %lu, lower size or raise memory",
                                         required_size, _config.memory_segment_size));
      }

      for (uint32_t create_open_try_count = 0; create_open_try_count < 10; ++create_open_try_count) {

         // Try to open first.
         if (_shm.open()) {

            try {
               shm_lock lock(_shm);

               if (not recover_p(_shm)) {
                  _shm.remove();
                  continue;
               }
               open(_shm);
               return;
            }
            catch (const os_exception& e) {
               // Lock (or remove) failed (probably not initializsed yet), creator dead?
               _shm.remove();
               continue;
            }
         }
         
         // Try to create if could not open.
         if (_shm.create(config.memory_segment_size)) {
            create(_shm);
            _shm.unlock();
            return;
         }
      }

      throw logic_error("tried to open/create 10 times, but no exceptions, this is probably a bug");
   }

   api::~api()
   {
   }
}
