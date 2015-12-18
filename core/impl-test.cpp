
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
   impl(name, config);
}
