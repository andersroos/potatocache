
#ifndef POTATOCACHE_TEST_HPP
#define POTATOCACHE_TEST_HPP

#include <stdint.h>
#include <string>

// Return a random alphanumeric string.
std::string uniqueid(uint32_t length=32);

// Return a random unique id name for shm name.
std::string unique_shm_name();

#endif
