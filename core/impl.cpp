#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <system_error>

#include "shared.hpp"
#include "utils.hpp"
#include "os.hpp"
#include "impl.hpp"
#include "config.hpp"

using namespace std;


namespace potatocache {

   // Raii class for setting operation. Bware, if code inside block can throw this should not be used.
   struct op_set
   {
      op_set(shm& shm, operation op) : _shm(shm)
      {
         _shm.ref<mem_header>(_shm.offset).op = op;
      }

      virtual ~op_set()
      {
         _shm.ref<mem_header>(_shm.offset).op = op_noop;
      }

   private:
      shm& _shm;
   };

   // Public methods.
   
   impl::impl(const std::string& name,
            const config& config) :
      _shm(name),
      _config(config)
   {
      uint64_t required_size = _shm.offset + sizeof(mem_header);
      required_size = align(required_size + sizeof(hash_entry) * _config.size);
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

               if (not recover_p()) {
                  _shm.remove();
                  continue;
               }
               open();
               return;
            }
            catch (const system_error& e) {
               // Lock (or remove) failed (probably not initializsed yet), creator dead?
               _shm.remove();
               continue;
            }
         }
         
         // Try to create if could not open.
         if (_shm.create(_config.memory_segment_size)) {
            create();
            _shm.unlock();
            return;
         }
      }

      throw logic_error("tried to open/create 10 times, but no exceptions, this is probably a bug");
   }

   impl::~impl()
   {
   }

   // Private methods.
   
   // TODO Test this.
   uint64_t impl::align(uint64_t offset)
   {
      uint64_t rest = offset % _shm.page_size;
      if (rest) {
         return offset + _shm.page_size - rest;
      }
      return offset;
   }
   
   bool impl::recover_p()
   {
      switch (head().op) {

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
            throw logic_error(fmt("recover could not handle state %d, this is a bug", head().op));
      }
   }

   void impl::open()
   {
      op_set set(_shm, op_open);
      // TODO Check other pids, add own pid.
   }

   void impl::create()
   {
      op_set set(_shm, op_init);

      // Init head.
      auto head = impl::head();
      head.process_count = 1;
      head.pids[0] = getpid();
      head.mem_size = _shm.size();
      head.hash_offset = _shm.offset + sizeof(hash_entry);
      head.hash_size = _config.size;
      head.blocks_offset = align(head.hash_offset);
      head.blocks_size = (head.mem_size - head.blocks_offset) / sizeof(block);
      head.blocks_free = head.blocks_size;
      head.free_block_index = 0;

      // Init hash entries.
      for (uint64_t i = 0; i < head.hash_size; ++i) {
         hash_entry& h = _shm.ref<hash_entry>(head.hash_offset + sizeof(hash_entry) * i);
         h.value_index = -1;
      }

      // Init blocks.
      int64_t last = -1;
      for (int64_t i = head.blocks_size - 1; i >= 0; i--) {
         _shm.ref<block>(head.blocks_offset + sizeof(block) * i).next_block_index = last;
         last = i;
      }
   }

}
