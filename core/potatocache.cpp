#include "potatocache.hpp"
#include "shared.hpp"

using namespace std;

namespace potatocache {
   
   api::api(const std::string& name,
            const config& config) :
      _name(name),
      _config(config)
   {
      
   }
}
