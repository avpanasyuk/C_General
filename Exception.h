#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>
#include <cstdarg>
#include "General.h"

namespace avp {
  class Exception: public std::exception {
      std::string err_text;
    public:
      Exception(const char *file, unsigned int line, const char *func_name)  {
        err_text = string_printf("Error in %s of file %s at line %d!\n",
                                 func_name, file, line);

      } // constructor

      Exception(const char *file, unsigned int line, const char *func_name,
                const char *format, ...) {
        va_list ap;
        va_start(ap,format);
        err_text = string_printf("Error in %s of file %s at line %d: ",
                                 func_name, file, line) + string_vprintf(format,ap);
        va_end(ap);
      } // constructor

      virtual const char* what() const noexcept {
          return err_text.c_str();
      } //
  }; // Exception
}

#define THROW_EXC  throw avp::Exception(__FILE__,__LINE__,__PRETTY_FUNCTION__)
#define THROW_EXC_PRINTF(format, ...) \
   throw avp::Exception(__FILE__,__LINE__,__PRETTY_FUNCTION__,format,##__VA_ARGS__)

#define AVP_ASSERT(exp) do{ if(!(exp)) \
{ THROW_EXC_PRINTF("Expression (" #exp ") is false!\n"); }}while(0)
#endif // EXCEPTION_H
