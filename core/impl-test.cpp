
#include <boost/test/unit_test.hpp>
#include <sys/wait.h>

#include <iostream>
#include <system_error>

#include "test.hpp"
#include "impl.hpp"
#include "utils.hpp"

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
   potatocache::config config;
   config.size = 2;
   config.memory_size_initial = 8 * 1024;
   config.memory_size_max = 8 * 1024;
   bool missing;

   {
      // Put get.
      impl impl(unique_shm_name(), config);
      impl.put("key", "value");
      BOOST_CHECK_EQUAL("value", impl.get("key", missing));
      BOOST_CHECK(not missing);
   }

   {
      // Get missing.
      impl impl(unique_shm_name(), config);

      BOOST_CHECK_EQUAL("", impl.get("key", missing));
      BOOST_CHECK(missing);
      
      impl.put("key", "value");
      
      BOOST_CHECK_EQUAL("value", impl.get("key", missing));
      BOOST_CHECK(not missing);

      BOOST_CHECK_EQUAL("", impl.get("another key", missing));
      BOOST_CHECK(missing);
   }
   
   {
      // Replace get.
      impl impl(unique_shm_name(), config);
      impl.put("key", "value");
      impl.put("key", "valu2");
      BOOST_CHECK_EQUAL("valu2", impl.get("key", missing));
      BOOST_CHECK(not missing);
   }

   {
      // Put multi block value, then get.
      impl impl(unique_shm_name(), config);
      string value(2028, 'x');
      impl.put("key", value);
      BOOST_CHECK_EQUAL(value, impl.get("key", missing));
      BOOST_CHECK(not missing);
   }

   {
      // Replace several times.
      impl impl(unique_shm_name(), config);
      for (char c = 'a'; c < 'x'; ++c) {
         string value(1024, c);
         impl.put("key", value);
         BOOST_CHECK_EQUAL(value, impl.get("key", missing));
         BOOST_CHECK(not missing);
      }
   }

   {
      // Free during replace works.
      potatocache::config c;
      c.size = 128;
      c.memory_size_initial = 16 * 1024;
      c.memory_size_max = 16 * 1024;
      impl impl(unique_shm_name(), c);

      string fillmem(128 * 56, 'x');
      impl.put("key", fillmem);
      BOOST_CHECK_EQUAL(fillmem, impl.get("key", missing));

      impl.put("key", "one");
      BOOST_CHECK_EQUAL("one", impl.get("key", missing));

      for (uint32_t i = 0; i < 127; ++i) {
         impl.put(fmt("key%d", i), "value");
      }
   }
   
   {
      // Put 2 get 2.
      impl impl(unique_shm_name(), config);
      
      impl.put("key1", "value1");
      BOOST_CHECK_EQUAL("value1", impl.get("key1", missing));
      BOOST_CHECK(not missing);
      
      impl.put("key2", "value2");
      BOOST_CHECK_EQUAL("value2", impl.get("key2", missing));
      BOOST_CHECK(not missing);

      BOOST_CHECK_EQUAL("value1", impl.get("key1", missing));
      BOOST_CHECK(not missing);      
   }

   {
      // Put empty key.
      impl impl(unique_shm_name(), config);
      impl.put("", "value");
      BOOST_CHECK_EQUAL("value", impl.get("", missing));
      BOOST_CHECK(not missing);
   }
   
   {
      // Put too long key.
      impl impl(unique_shm_name(), config);
      string key(32, 'x');
      BOOST_CHECK_THROW(impl.put(key, "value"), length_error);
   }

   {
      // Fill the hash table.
      impl impl(unique_shm_name(), config);
      
      impl.put("key1", "value1");
      impl.put("key2", "value2");
      BOOST_CHECK_THROW(impl.put("key3", "value3"), length_error);
      BOOST_CHECK_EQUAL("value1", impl.get("key1", missing));
      BOOST_CHECK_EQUAL("value2", impl.get("key2", missing));
   }

   {
      // Fill the memory.
      impl impl(unique_shm_name(), config);
      
      impl.put("key1", "value1");
      BOOST_CHECK_THROW(impl.put("key2", string(10 * 1024, 'x')), length_error);
      BOOST_CHECK_EQUAL("value1", impl.get("key1", missing));
   }
   
   {
      // Put a really big piece of data.
      potatocache::config c;
      c.size = 1;
      c.memory_size_initial = 10 * 1024 * 1024;
      c.memory_size_max = 10 * 1024 * 1024;
      impl impl(unique_shm_name(), c);

      string big(8 * 1024 * 1024, 'x');
      impl.put("key", big);
      BOOST_CHECK_EQUAL(big, impl.get("key", missing));
   }

}

BOOST_AUTO_TEST_CASE(test_resize)
{
   bool missing;

   {
      // Fill the memory and expect a resize.
      potatocache::config c;
      c.size = 2;
      c.memory_size_initial = 8 * 1024;
      c.memory_size_max = 0;
      impl impl(unique_shm_name(), c);

      BOOST_CHECK_EQUAL(c.memory_size_initial, impl.size());
      
      string big(6 * 1024, 'x');
      impl.put("key", big);
      BOOST_CHECK_EQUAL(big, impl.get("key", missing));
      
      BOOST_CHECK_EQUAL(c.memory_size_initial * 2, impl.size());
   }   
 
    {
       // Fill the memory and expect two resize, but then it should be full.
       potatocache::config c;
       c.size = 2;
       c.memory_size_initial = 8 * 1024;
       c.memory_size_max = 32 * 1024;
       impl impl(unique_shm_name(), c);
 
       BOOST_CHECK_EQUAL(c.memory_size_initial, impl.size());
       
       string big(10 * 1024, 'x');
       impl.put("key1", big);
       BOOST_CHECK_THROW(impl.put("key2", big), length_error);
       
       BOOST_CHECK_EQUAL(big, impl.get("key1", missing));
       BOOST_CHECK_EQUAL(c.memory_size_max, impl.size());
    }   
 
    {
       // Make sure the resize propagates to another instance of the shared mem.
       string name(unique_shm_name());
       
       potatocache::config c;
       c.size = 2;
       c.memory_size_initial = 8 * 1024;
       c.memory_size_max = 16 * 1024;
       
       impl impl1(name, c);
 
       impl impl2(name, c);
       
       string big(6 * 1024, 'x');
       
       impl1.put("key", big);
 
       BOOST_CHECK_EQUAL(c.memory_size_max, impl1.size());

       // TODO Need to implement size check at every operation for this to work.
       
       BOOST_CHECK_EQUAL(big, impl2.get("key", missing));
       BOOST_CHECK_EQUAL(c.memory_size_max, impl2.size());
    }   
}
