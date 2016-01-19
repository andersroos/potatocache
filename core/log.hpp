#pragma once

#include <stdint.h>

#include <ostream>

#include "config.hpp"

#define LOG_ERROR(FORMAT, ...)   log(potatocache::log_level::ERROR, __FILE__, __LINE__, FORMAT, ##__VA_ARGS__)
#define LOG_WARNING(FORMAT, ...) log(potatocache::log_level::WARNING, __FILE__, __LINE__, FORMAT, ##__VA_ARGS__)
#define LOG_INFO(FORMAT, ...)    log(potatocache::log_level::INFO, __FILE__, __LINE__, FORMAT, ##__VA_ARGS__)
#define LOG_DEBUG(FORMAT, ...)   log(potatocache::log_level::DEBUG, __FILE__, __LINE__, FORMAT, ##__VA_ARGS__)
// TODO Remove LOG level debug or, have macro with if statement?

namespace potatocache {

   // Log levels in potatocache.
   enum log_level {
      
      // You need to take care of those.
      ERROR = 0,

      // Bad usage or configuration, but no immediate action is needed.
      WARNING = 1,

      // Just information, no excessive logging, safe to use in prod.
      INFO = 2,

      // For developers.
      DEBUG = 3,
      
   };
   
   extern std::ostream* log_stream;
   
   extern log_level log_level_filter;
   
   // Log, but use the macro instead.
   void log(const uint32_t& level, const char* file, int line, const char* format, ...);
   
    // Set the output stream for the log.
   void set_log_output(std::ostream* stream);

    // Set the log level for the log.
   void set_log_level(const log_level& level);
   
}
