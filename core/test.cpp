
#include <stdlib.h>

#include "test.hpp"

using namespace std;

unsigned int seed = time(NULL);

// Return a random alphanumeric string.
string uniqueid(uint32_t length)
{
   string s;
   
   static const char alphanum[] =
      "0123456789_"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < length; ++i) {
        s += alphanum[rand_r(&seed) % (sizeof(alphanum) - 1)];
    }

    return s;
}
