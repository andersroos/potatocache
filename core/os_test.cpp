#include <boost/test/unit_test.hpp>
#include "os.hpp"

using namespace potatocache;


BOOST_AUTO_TEST_CASE(os_test_create_shm)
{
   // Test that the message queue is working as a queue (FIFO).

   shm shm("test");
}
