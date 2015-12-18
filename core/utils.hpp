#ifndef POTATOCACHE_UTILS_HPP
#define POTATOCACHE_UTILS_HPP

#include <string.h>
#include <stdarg.h>
#include <string>

// Format a classic c format with va_args to a string.
std::string fmt(const char* format, ...);

// Return the current time in microseconds.
uint64_t now_us();

#endif
