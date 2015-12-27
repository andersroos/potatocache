
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <cstdarg>
#include <ostream>

#include "log.hpp"

using namespace std;

namespace potatocache {

   std::ostream* log_stream = NULL;
   
   log_level log_level_filter = log_level::INFO;
   
   void set_log_output(ostream* stream)
   {
      log_stream = stream;
   }

   void set_log_level(const log_level& level)
   {
      log_level_filter = level;
   }
  
   void log(const uint32_t& level, const char* file, int line, const char* format, ...)
   {
      if (log_stream == NULL or level > log_level_filter) {
         return;
      }
      
      va_list va_args;
      va_start(va_args, format);
   
      //
      // va_args
      //
   
      // message
      
      char message[2048];
      const char* lvl;
      vsnprintf(message, sizeof(message), format, va_args);
      
      // timestamp
      
      time_t t;
      time(&t);
      struct tm ti;
      struct tm* timeinfo = gmtime_r(&t, &ti);
      char timestamp[20];
      if (timeinfo && !strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo)) {
         // Don't throw an exception here, because it is good if the log line is be printed anyway.
         timestamp[0] = '\0'; 
      }
   
      // level
      
      switch (level) {
         case log_level::DEBUG:   lvl = "DEBUG   "; break;
         case log_level::INFO:    lvl = "INFO    "; break;
         case log_level::WARNING: lvl = "WARNING "; break;
         case log_level::ERROR:   lvl = "ERROR   "; break;
      }
   
      // file
   
      if (strlen(file) > 24) {
         file += strlen(file) - 24;
      }
      
      // write the message in one go to make it thread safe
      
      char buf[4096];
      snprintf(buf, sizeof(buf),
               "%s %s [%d/0x%016lx %24s:%-3d]: %s\n",
               timestamp, lvl, getpid(), pthread_self(), file, line, message);   
      
      *log_stream << buf;
      log_stream->flush();
      
      //
      // va_args
      //
      
      va_end(va_args);
   }
}
