#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#include <algorithm>
#include <stdexcept>

#include "utils.hpp"

std::string fmt(const char* format, ...)
{
   ::va_list args;
   ::va_start(args, format);

   size_t len = ::strlen(format);
   
   char message[len + (1<<16)];

   int res = ::vsnprintf(message, sizeof(message), format, args);
   message[sizeof(message) - 1] = 0;

   if (res < 0 or uint32_t(res) >= sizeof(message) - 1) {
      throw std::invalid_argument("fmt failed, total mesage too large or other error");
   }

   ::va_end(args);

   return std::string(message);
}

uint64_t now_us() {
   timeval now;
   ::gettimeofday(&now, 0);
   return now.tv_sec * 1000000 + now.tv_usec;
}

std::string errstr(int errnum) {
   std::string res(strerror(errnum));
   std::transform(res.begin(), res.end(), res.begin(), ::tolower);
   return res;
}
