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
      op_set(shm& shm, operation_t op) : _shm(shm)
      {
         _shm.ref<mem_header_t>(_shm.offset).op = op;
      }

      virtual ~op_set()
      {
         _shm.ref<mem_header_t>(_shm.offset).op = op_noop;
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
      uint64_t required_size = _shm.offset + sizeof(mem_header_t);
      required_size = align(required_size + sizeof(hash_entry_t) * _config.size);
      required_size += sizeof(block_t) * _config.size;

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
      {
         auto& head = impl::head();
         auto pid = shm::pid();
         
         shm_lock lock(_shm);
         
         if (recover_p() and head.process_count > 1) {
            
            // Remove myself from pid list if I am there.
            for (uint32_t i = 0; i < PIDS_SIZE; ++i) {
               if (head.pids[i] == pid) {
                  head.process_count--;
                  head.pids[i] = 0;
                  break;
               }
            }
            return;
         }
      }
      _shm.remove();
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

      auto& head = impl::head();

      // Update current list with still running pids.
      head.process_count = 0;
      for (uint64_t i = 0; i < PIDS_SIZE; ++i) {
         auto pid = head.pids[i];
         if (pid and shm::process_exists(pid)) {
            head.process_count += 1;
         }
         else {
            head.pids[i] = 0;
         }
      }

      // Add self.
      if (head.process_count < PIDS_SIZE) {
         head.process_count += 1;
         for (uint64_t i = 0; i < PIDS_SIZE; ++i) {
            if (not head.pids[i]) {
               head.pids[i] = shm::pid();
               break;
            }
         }
      }
   }

   void impl::create()
   {
      op_set set(_shm, op_init);

      // Init head.
      auto& head = impl::head();
      head.process_count = 1;
      head.pids[0] = _shm.pid();
      head.mem_size = _shm.size();
      head.hash_offset = _shm.offset + sizeof(mem_header_t);
      head.hash_size = _config.size;
      head.blocks_offset = align(head.hash_offset + sizeof(hash_entry_t) * head.hash_size);
      head.blocks_size = (head.mem_size - head.blocks_offset) / sizeof(block_t);
      head.blocks_free = head.blocks_size;
      head.free_block_index = 0;

      // Init hash entries.
      for (uint64_t i = 0; i < head.hash_size; ++i) {
         entry(i).value_index = -1;
      }

      // Init blocks.
      int64_t last = -1;
      for (int64_t i = head.blocks_size - 1; i >= 0; i--) {
         block(i).next_block_index = last;
         last = i;
      }
   }

}
