#ifndef POTATOCACHE_EXCEPTIONS_HPP
#define POTATOCACHE_EXCEPTIONS_HPP

#include <string>
#include <exception>

class base_exception : public std::exception
{
public:

   base_exception operator<<(const std::string& what)
   {
      _what = what;
      return *this;
   }

   base_exception operator<<(const char* what)
   {
      _what = std::string(what);
      return *this;
   }

   virtual const char* what() const throw()
   {
      return _what.c_str();
   }

   virtual ~base_exception() throw() {};
   
private:
   
   std::string _what;
};

#endif
