
#define BOOST_TEST_MODULE os test
#include <boost/test/unit_test.hpp>

#include "test.hpp"
#include "os.hpp"

using namespace potatocache;
using namespace std;


BOOST_AUTO_TEST_CASE(create_when_exists_returns_false)
{
   // Test that the message queue is working as a queue (FIFO).

   string name('/' + uniqid());
   
   shm shm(name);
   BOOST_CHECK(shm.create(256));
   BOOST_CHECK(!shm.create(256));
   shm.remove();
}
