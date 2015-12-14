#include "os.hpp"

// Used for various experimenting and testing.

using namespace potatocache;
using namespace std;

int main()
{
   string name("arne");
   
   shm shm1(name);
   if (not shm1.create(256)) {
      shm1.open();
   }
   
   shm1.ref<uint32_t>(0) = 1234;
   cerr << shm1.ref<uint32_t>(0) << endl;
   shm1.remove();

   shm shm2(name);
   shm2.create(256);
   cerr << shm2.ref<uint32_t>(0) << endl;
   
   return 0;
}
