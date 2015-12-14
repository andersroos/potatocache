#include <pthread.h>
#include <unistd.h>

#include "utils.hpp"
#include "os.hpp"

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

int main()
{
   string name("arne");
   
   shm shm1(name);
   if (shm1.create(256)) {
      cerr << "created shm" << endl;
      pthread_mutexattr_t attr;
      pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
      pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
      
      auto mutex = shm1.ptr<pthread_mutex_t>(0);
      cerr << "init " << errstr(pthread_mutex_init(mutex, &attr)) << endl;
      
      cerr << "locking " << errstr(pthread_mutex_lock(mutex)) << endl;
      countdown(20);
   }
   else if (shm1.open()) {
      cerr << "opened shm" << endl;
      auto mutex = shm1.ptr<pthread_mutex_t>(0);
      int res = pthread_mutex_lock(mutex);
      cerr << "locking " << errstr(res) << endl;
      if (res == EOWNERDEAD) {
         cerr << "setting consistent " << errstr(pthread_mutex_consistent(mutex)) << endl;
      }
   }
   else {
      cerr << "failed to create or open" << endl;
      return 1;
   }
   
   return 0;
}
