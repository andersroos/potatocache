
#define BOOST_TEST_MODULE os test
#include <boost/test/unit_test.hpp>

#include "test.hpp"
#include "os.hpp"

using namespace potatocache;
using namespace std;


BOOST_AUTO_TEST_CASE(create_when_exists_returns_false)
{
   string name('/' + uniqid());
   shm shm(name);
   BOOST_CHECK(shm.create(256));
   BOOST_CHECK(!shm.create(256));
   shm.remove();
}

BOOST_AUTO_TEST_CASE(open_returns_false_if_it_does_not_exists)
{
   string name('/' + uniqid());
   shm shm(name);
   BOOST_CHECK(!shm.open());
}

