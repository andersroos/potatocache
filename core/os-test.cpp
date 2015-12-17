
#include <iostream>
#include <system_error>

#include "test.hpp"
#include "os.hpp"

#define BOOST_TEST_MODULE os test
#include <boost/test/unit_test.hpp>

using namespace potatocache;
using namespace std;

BOOST_AUTO_TEST_CASE(test_bad_name_raises_exception)
{
   string s(255, 'x');
   BOOST_CHECK_THROW(shm x(s), invalid_argument);
   
   BOOST_CHECK_THROW(shm x("0/1"), invalid_argument);
   
   BOOST_CHECK_THROW(shm x("0 1"), invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_create_when_exists_returns_false)
{
   string name("shm" + uniqueid());
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
   string name("shm" + uniqueid());

   shm shm1(name);
   BOOST_CHECK(shm1.create(256));
   shm1.ref<uint32_t>(shm1.offset) = 1234;

   shm shm2(name);
   shm2.open();
   BOOST_CHECK_EQUAL(1234, shm2.ref<uint32_t>(shm2.offset));
   shm1.remove();
}

BOOST_AUTO_TEST_CASE(test_lock_can_be_taken_on_process_death)
{
   string name("shm" + uniqueid());

   if (fork()) {
      // parent
      shm shm(name);
      while (not shm.open()) {
         usleep(100);
      }
      shm_lock lock(shm);
      BOOST_CHECK_EQUAL(1234, shm.ref<uint32_t>(shm.offset));
      shm.remove();
   }
   else {
      // child
      shm shm(name);
      shm.create(256);
      shm.ref<uint32_t>(shm.offset) = 1234;
      // no unlock
      exit(0);
   }
}

BOOST_AUTO_TEST_CASE(test_name_is_reusable_after_remove)
{
   string name("shm" + uniqueid());

   shm shm1(name);
   shm1.create(256);
   shm1.ref<uint32_t>(shm1.offset) = 1234;
   shm1.unlock();
   shm1.remove();

   shm shm2(name);
   BOOST_CHECK(not shm2.open());
   BOOST_CHECK(shm2.create(256));
   BOOST_CHECK(1234 != shm2.ref<uint32_t>(shm2.offset));
   shm2.remove();
}

BOOST_AUTO_TEST_CASE(test_create_fails_on_too_big_size)
{
   string name("shm" + uniqueid());

   shm shm(name);
   BOOST_CHECK_THROW(shm.create(1e19), system_error);
}

BOOST_AUTO_TEST_CASE(test_size_on_non_existent_shared_memory_section_throws)
{
   string name("shm" + uniqueid());

   shm shm(name);
   BOOST_CHECK_THROW(shm.size(), system_error);
}
