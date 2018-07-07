#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>
#include "General.h"

namespace avp {
  class Exception: public std::exception {
      std::string err_text;
    public:
      Exception(const char *file, unsigned int line, const char *func_name, const char *what = "") {
        err_text = string_printf("%s in %s of file %s at line %d!\n",
                                 what, func_name, file, line));

      } // constructor
      virtual const char* what() const noexcept {
          return err_text.c_str();
      } //
  }; // Exception
}

#define THROW_EXC(why) throw avp::Exception(__FILE__,__LINE__,__PRETTY_FUNCTION__,(why));

#endif // EXCEPTION_H
