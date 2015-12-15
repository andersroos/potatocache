
#include <iostream>

#include "test.hpp"
#include "os.hpp"

#define BOOST_TEST_MODULE os test
#include <boost/test/unit_test.hpp>

using namespace potatocache;
using namespace std;

BOOST_AUTO_TEST_CASE(test_bad_name_raises_exception)
{
   BOOST_CHECK_THROW(shm("012345678901234567890123456789XX"), invalid_argument);
   BOOST_CHECK_THROW(shm("0 1"), invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_create_when_exists_returns_false)
{
   string name(uniqueid());
   shm shm(name);
   BOOST_CHECK(shm.create(256));
   shm.unlock();
   BOOST_CHECK(!shm.create(256));
   shm.remove();
}

BOOST_AUTO_TEST_CASE(test_open_returns_false_if_it_does_not_exists)
{
   string name(uniqueid());
   shm shm(name);
   BOOST_CHECK(!shm.open());
   shm.remove();
}

BOOST_AUTO_TEST_CASE(test_writing_and_reading_data_from_same_process_using_different_shm_works)
{
   string name(uniqueid());

   shm shm1(name);
   BOOST_CHECK(shm1.create(256));
   shm1.ref<uint32_t>(shm1.offset) = 1234;

   shm shm2(name);
   shm2.open();
   BOOST_CHECK_EQUAL(1234, shm2.ref<uint32_t>(shm2.offset));
   shm1.remove();
}

