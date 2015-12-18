
#include <stdlib.h>

#include <iostream>

#include "test.hpp"
#include "utils.hpp"

using namespace std;

// Return a random alphanumeric string.
string uniqueid(uint32_t length)
{
   string s;
   
   static const char alphanum[] =
      "0123456789_"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

   unsigned int seed = now_us();
    for (int i = 0; i < length; ++i) {
        s += alphanum[rand_r(&seed) % (sizeof(alphanum) - 1)];
    }

    return s;
}

std::string unique_shm_name()
{
   string s("potato_test_shm_" + uniqueid());
   return s;
}
