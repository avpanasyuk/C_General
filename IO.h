#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

/**
  * @file AVP_LIBS/General/IO.h
  * @author Alexander Panasyuk
  * Generic functions for output.
  * @brief templates of write_byte->write->vprintf->printf hierarhy
  */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "Error.h"
#include "Time.h"


/// creates printf-type function named "func_name" out of vprintf-type function named "vprinf_func"
/// usage: PRINTF_WRAPPER_BOOL(printf,vprintf)
#define PRINTF_WRAPPER(func_name, vprintf_func) \
  bool func_name(char const *format, ...) \
  { va_list ap; va_start(ap,format); \
    bool Out =  vprintf_func(format,ap); va_end(ap); \
    return Out; }

namespace avp {
//! @note ALL IO FUNCTIONS HAVE atomic output - either they write or not,
//! return corresponding value
  typedef bool (*write_byte_func)(uint8_t); ///< writes byte
  typedef bool (*write_type_func)(const uint8_t *Ptr, size_t Size); ///< writes byte array
  typedef bool (*vprintf_type_func)(char const *format, va_list ap);
  typedef bool (*printf_type_func)(char const *format, ...);

  enum IO_ERROR_CODES {FORMAT=1,TIMEOUT};

  /** @note about following  functions - they should be used with BLOCKED write which
  * returns only after data pointed by Ptr are used. Or it should make a copy.
  * @note functions do not write string-ending 0 !!!!!
  */

  /// generates write_type_func from write_byte_func
  template<write_byte_func write_byte, size_t (*space_left)() = nullptr>
  bool write(const uint8_t *p, size_t sz) {
    if(space_left != nullptr && sz < space_left()) return false;
    for(; sz--;) if(!write_byte(*(p++))) return false; // it makes for non-atomic situation, but it should not happen
    return true;
  } // write

  /// function which allows to write something with timeout and call loop function while write is pending
  template<write_type_func write_func, uint32_t Timeout = 1000, uint32_t (*TickFunction)() = millis,
           void (*loop_func)() = nullptr, failfunc_type fail_func = nullptr>
  bool write_with_timeout(const uint8_t *Ptr, size_t Size)  {
    TimeOut<TickFunction> T(Timeout); \
    while(!write_func(Ptr,Size)) {
      if(loop_func != nullptr) loop_func();
      if(T) {
        if(fail_func != nullptr) fail_func(TIMEOUT);
        return false;
      }
    };
    return true;
  } // write_with_timeout

  /*! object to create misc output functions - very convenient to use with typedef which we can not do
   *! with function templates
   */
  template<write_type_func write_>
  struct write_buffered {
    template<typename T> static bool object(const T &obj) { return write_((const uint8_t *)&obj,sizeof(obj)); }
    template<typename T> static bool array(T *p, size_t Size=1) { return write_((const uint8_t *)p,sizeof(p[0])*Size); }
    static bool string(const char *str) { return  write_((const uint8_t *)str,strlen(str)); }

    static bool vprintf(char const *format, va_list ap) {
      int Size = vsnprintf(NULL,0,format,ap);
      AVP_ASSERT_WITH_EXPL(Size >= 0,FORMAT,"vprintf: Format %s is bad!",format);
      uint8_t Buffer[Size+1]; // +1 to include ending zero byte
      vsprintf((char *)Buffer,format,ap);
      return write_((const uint8_t *)Buffer,Size); // we do not write ending 0 byte
    } // vprintf

    static PRINTF_WRAPPER(printf,vprintf)
  }; // class write_buffered

  template<bool (*write_)(const uint8_t *Ptr, size_t Size, void (*ReleaseFunc)())>
  struct write_unbuffered {
    template<typename T> static bool object(const T &obj, void (*ReleaseFunc)() = nullptr)
      { return write_((const uint8_t *)&obj,sizeof(obj),ReleaseFunc); }
    template<typename T> static bool array(T *p, size_t Size=1, void (*ReleaseFunc)() = nullptr)
      { return write_((const uint8_t *)p,sizeof(p[0])*Size,ReleaseFunc); }
    static bool string(const char *str, void (*ReleaseFunc)() = nullptr)
      { return  write_((const uint8_t *)str,strlen(str),ReleaseFunc); }
  }; // class write_unbuffered



  // ******************** FUNCTION TEMPLATES, I DO NOT KNOW HOW USEFUL THEY ARE
  template<write_type_func write>
  inline bool vprintf(char const *format, va_list ap) { return write_buffered<write>::vprintf(format,ap); }
  template<write_byte_func write_byte, size_t (*space_left)() = nullptr>
  bool vprintf(char const *format, va_list ap) { return vprintf<write<write_byte,space_left>>(format, ap); }
  template<vprintf_type_func vprintf> PRINTF_WRAPPER(printf,vprintf)
  template<write_type_func write> PRINTF_WRAPPER(printf,vprintf<write>)
  template<write_byte_func write_byte, size_t (*space_left)() = nullptr>
  PRINTF_WRAPPER(printf,SINGLE_ARG(vprintf<write_byte,space_left>))
} // namespace avp

#endif /* IO_H_INCLUDED */
