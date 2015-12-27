#include <pthread.h>
#include <unistd.h>

#include <iostream>

#include "utils.hpp"
#include "potatocache.hpp"
#include "impl.hpp"

// Used for various experimenting and testing.

using namespace potatocache;
using namespace std;

void countdown(uint32_t count)
{
   while (count > 0) {
      sleep(1);
      count--;
      cerr << "sleeping " << count << endl;
   }
}

int main(int argc, char** argv)
{
   api::set_log_output(&std::cerr);
   api::set_log_level(log_level::DEBUG);

   string name("arne");
   
   config conf;
   conf.size = 2;
   conf.memory_segment_size = 5 * 1024;
      
   bool missing;

   potatocache::api cache(name, conf);
   // impl cache(name, conf);

   cerr << "putting value" << endl;
   cache.put("key", "value");

   cerr << "sleeping" << endl;
   sleep(5);
   
   cerr << "getting value: " << cache.get("key", missing) << endl;
   
   
   return 0;
}
