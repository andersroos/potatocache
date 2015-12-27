#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <system_error>

#include "shared.hpp"
#include "utils.hpp"
#include "log.hpp"
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
      _name(name),
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
               auto process_count = open();
               LOG_INFO("Opened cache '%s', size is %d bytes, %d proceses attached.", name.c_str(), _shm.size(),
                        process_count);
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
            LOG_INFO("Created cache '%s', size is %d bytes.", name.c_str(), _shm.size());
            return;
         }
      }

      throw logic_error("tried to open/create 10 times, but no exceptions, this is probably a bug");
   }

   void impl::put(const std::string& key, const std::string& value)
   {
      // TODO Think about recoverablility is everything done in the correct order?
      
      if (key.size() > 31) {
         throw length_error(fmt("key is too long, was %u", key.size()));
      }

      uint32_t hash = impl::hash(key);
      
      shm_lock lock(_shm);
      if (not recover_p()) {
         // TODO What todo if irrecoverrable here? Reinit? Or does recover do that?
      }
      
      op_set set(_shm, op_put);

      // Find hash entry.

      auto& head = impl::head();
      auto size = head.hash_size;
      
      auto hash_i = hash;
      hash_entry_t* entry;
      while (true) {
         entry = &impl::entry(hash_i % size);
         if (entry->value_index == -1
             or (hash == entry->hash and not strncmp(key.c_str(), entry->key, sizeof(hash_entry_t::key)))) {
            break;  // We found the entry to use.
         }

         hash_i++;

         if (hash_i % size == hash % size) {
            throw length_error(fmt("out of hash entries, capacity is %d entries", size));
         }
      }

      // Free old value.
      
      if (entry->value_index != -1) {
         // TODO Is putting same value as existing a normal case? maybe check for it?

         // TODO Clear data here.
         
         int64_t data_size = entry->value_size;
         int64_t value_index = entry->value_index;
         auto end_index = value_index;
         while (data_size > 0) {
            end_index = block(end_index).next_block_index;
            data_size -= sizeof(block_t::data);
            head.blocks_free++;
         }
         block(end_index).next_block_index = head.free_block_index;
         head.free_block_index = value_index;
      }

      // Insert new value.

      int64_t data_size = value.size();
      if (data_size > int64_t(head.blocks_free * sizeof(block_t::data))) {
         throw length_error(fmt("out of value storage, capacity is %u bytes",
                                head.blocks_size * sizeof(block_t::data)));
      }

      const char* data = value.c_str();
      
      entry->value_index = head.free_block_index;
      entry->value_size = data_size;
      entry->hash = hash;
      strncpy(entry->key, key.c_str(), sizeof(hash_entry_t::key));
            
      while (data_size > 0) {
         auto& free = block(head.free_block_index);
         memcpy(free.data, data, min(data_size, int64_t(sizeof(block_t::data))));
         head.free_block_index = free.next_block_index;
         head.blocks_free--;
         data_size -= sizeof(block_t::data);
         data += sizeof(block_t::data);
      }
   }

   std::string impl::get(const std::string& key, bool& missing_out)
   {
      // TODO Think about recoverablility is everything done in the correct order?

      missing_out = true;
      
      if (key.size() > 31) {
         return "";
      }

      uint32_t hash = impl::hash(key);
      
      shm_lock lock(_shm);
      if (not recover_p()) {
         // TODO What todo if irrecoverrable here? Reinit? Or does recover do that?
      }
      
      op_set set(_shm, op_get);

      // Find hash entry.

      auto& head = impl::head();
      auto size = head.hash_size;
      
      auto hash_i = hash;
      hash_entry_t* entry;
      while (true) {
         entry = &impl::entry(hash_i % size);
         if (entry->value_index == -1) {
            return ""; 
         }

         if (hash == entry->hash and not strncmp(key.c_str(), entry->key, sizeof(hash_entry_t::key))) {
            break;
         }

         hash_i++;

         if (hash_i % size == hash % size) {
            return "";
         }
      }
      
      // We found the entry build the response.

      missing_out = false;

      string res(entry->value_size, '_');

      uint64_t target_index = 0;
      int64_t data_size = entry->value_size;
      int64_t data_index = entry->value_index;

      while (data_size > 0) {
         auto& block = impl::block(data_index);
         auto len = min(data_size, int64_t(sizeof(block_t::data)));
         res.replace(target_index, len, block.data, len); // At least gcc stdlib does this in place.
         data_index = block.next_block_index;
         data_size -= sizeof(block_t::data);
         target_index += sizeof(block_t::data);
      }

      return res;
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
            LOG_INFO("Closed cache '%s'.", _name.c_str());
            return;
         }
      }
      _shm.remove();
      LOG_INFO("Removed cache '%s' (was last process attached).", _name.c_str());
   }

   // Private methods.
   
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

            // TODO Complete all revocery.
            
         default:
            throw logic_error(fmt("recover could not handle state %d, this is a bug", head().op));
      }
   }

   uint32_t impl::open()
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
      return head.process_count;
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

   uint64_t impl::hash(const string& key)
   {
      uint64_t hash = 0;
      const char* str = key.c_str();
      while (*str) {
         hash = hash * 101 + *str++;
      }
      return hash;
   }

   void impl::dump()
   {
      shm_lock lock(_shm);
      
      auto& out = cout;

      out << endl;

      // Header.

      uint64_t offset;
      uint64_t bytes;

      auto& head = impl::head();
      offset = _shm.offset;
      bytes = sizeof(mem_header_t);
      out << fmt("head, size %d, offset [%d-%d], mem [%p-%p]\n", sizeof(mem_header_t), offset,
                 offset + bytes, _shm.ptr<char>(offset), _shm.ptr<char>(offset + bytes));
      out << fmt("    op: %d\n", head.op); 
      out << fmt("    process_count: %d\n", head.op);
      stringstream ss;
      string delim("[");
      for (uint32_t i = 0; i < PIDS_SIZE; ++i) {
         ss << delim << head.pids[i];
         delim = ", ";
      }
      ss << "]";
      out << fmt("    pids: %s\n", ss.str().c_str()); 
      out << fmt("    mem_size: %d\n", head.mem_size); 
      out << fmt("    hash_offset: %d\n", head.hash_offset); 
      out << fmt("    hash_size: %d\n", head.hash_size); 
      out << fmt("    blocks_offset: %d\n", head.blocks_offset); 
      out << fmt("    blocks_size: %d\n", head.blocks_size); 
      out << fmt("    blocks_free: %d\n", head.blocks_free); 
      out << fmt("    free_block_index: %d\n", head.free_block_index);

      // Hash entries.

      offset = head.hash_offset;
      bytes = sizeof(hash_entry_t) * head.hash_size;
      out << fmt("entries, size %d, entry size %d, count %d, offset [%d-%d], mem [%p-%p]\n",
                 bytes, sizeof(hash_entry_t), head.hash_size,
                 offset, offset + bytes, _shm.ptr<char>(offset), _shm.ptr<char>(offset + bytes));
      for (uint64_t i = 0; i < head.hash_size; ++i) {
         auto& entry = impl::entry(i);
         out << fmt("    entry %d, offset %d, mem %p\n", i, offset + sizeof(hash_entry_t) * i, &entry);
         out << fmt("        hash: 0x%0*X\n", sizeof(hash_entry_t::hash) * 2, entry.hash);
         out << fmt("        key: '%s'\n", entry.key);
         out << fmt("        value_index: %d\n", entry.value_index);
         out << fmt("        value_size: %d\n", entry.value_size);
      }

      // Blocks.

      offset = head.blocks_offset;
      bytes = sizeof(block_t) * head.blocks_size;
      out << fmt("blocks, size %d, block size %d, count %d, free count %d, offset [%d-%d], mem [%p-%p]\n",
                 bytes, sizeof(block_t), head.blocks_size, head.blocks_free,
                 offset, offset + bytes, _shm.ptr<char>(offset), _shm.ptr<char>(offset + bytes));
      for (uint64_t i = 0; i < head.blocks_size; ++i) {
         auto& block = impl::block(i);
         out << fmt("    block %d, offset %d, mem %p\n", i, offset + sizeof(block_t) * i, &block);
         out << fmt("        next_block_index: %d\n", block.next_block_index);
         out << fmt("        data: '%s'\n", string(block.data, sizeof(block.data)).c_str());
      }
      
      out << endl;
   }
   
}
