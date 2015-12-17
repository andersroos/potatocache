#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>

#include "potatocache.hpp"
#include "shared.hpp"
#include "exceptions.hpp"
#include "utils.hpp"
#include "os.hpp"
#include "shared.hpp"

using namespace std;

namespace potatocache {
   
   api::api(const std::string& name,
            const config& config) :
      _shm(name),
      _config(config)
   {

      while (true) {
         if (_shm.open()) {
            // TODO Check size.
            cerr << "opened" << endl;
            break;
         }
         if (_shm.create(config.memory_segment_size)) {
            cerr << "created" << endl;
            break;
         }
      }
      
   }
}
