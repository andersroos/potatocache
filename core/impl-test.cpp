
#include <boost/test/unit_test.hpp>
#include <sys/wait.h>

#include <iostream>
#include <system_error>

#include "test.hpp"
#include "impl.hpp"


using namespace potatocache;
using namespace std;

BOOST_AUTO_TEST_CASE(test_basic_create_and_destroy_of_impl)
{
   string name(unique_shm_name());
   config config;
   impl impl(name, config);
}

BOOST_AUTO_TEST_CASE(test_put_get)
{
   string name(unique_shm_name());
   config config;
   config.size = 2;
   config.memory_segment_size = 5 * 1024;
   bool missing;
   impl impl(name, config);
   impl.put("key", "value");
   BOOST_CHECK_EQUAL("value", impl.get("key", missing));
   BOOST_CHECK(not missing);
}
