#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

/**
  * @file AVP_LIBS/General/IO.h
  * @author Alexander Panasyuk
  */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

namespace avp {
  // following functions use general "write" function of type bool write(const void *Ptr, uint16_t Size) to do formatted output;
  // NOTE both functions do not write string-ending 0 !!!!!
  // extern bool vprintf(bool (*pwrite)(volatile void *Ptr, size_t Size),char const *format, va_list ap);
  // extern bool printf(bool (*pwrite)(volatile void *Ptr, size_t Size), char const *format, ...);
// Note about following two functions - they should be used with BLOCKED pwrite which
  // returns only after data pointed by Ptr are used. Or it makes a copy.

  // following functions use general "write" function of type bool write(const void *Ptr, uint16_t Size) to do formatted output;
  // NOTE both functions do not write string-ending 0 !!!!!
  template<bool (*pwrite)(const uint8_t *Ptr, size_t Size)> bool vprintf(char const *format, va_list ap) {
    int Size = vsnprintf(NULL,0,format,ap);
    if(Size < 0) return false;
    uint8_t Buffer[Size+1]; // +1 to include ending zero byte
    vsprintf((char *)Buffer,format,ap);
    return (*pwrite)((const uint8_t *)Buffer,Size); // we do not write ending 0 byte
  } // vprintf

  template<bool (*vprintf)(char const *format, va_list ap)> bool printf(char const *format, ...) {
    va_list ap;
    va_start(ap,format);
    bool Out =  vprintf(format,ap);
    va_end(ap);
    return Out;
  } // printf

  template<bool (*pwrite)(const uint8_t *Ptr, size_t Size)> bool printf(char const *format, ...) {
    va_list ap;
    va_start(ap,format);
    bool Out =  vprintf<pwrite>(format,ap);
    va_end(ap);
    return Out;
  } // printf

  /**
  * given a function to output a single byte this template class creates function to output arbitrary things
  */
  template<bool (*put_byte)(uint8_t)>
  struct Out {
    static bool write(const uint8_t *p, size_t sz) { for(; sz--;) if(!put_byte(*(p++))) return false; return true; }
    static bool write(const char *str) { return  write((const uint8_t *)str,strlen(str)); }
    template<typename T> static bool write(T &obj) { return write((const uint8_t *)&obj,sizeof(obj)); }
    static bool printf(char const *format, ...) {
      va_list ap;
      va_start(ap,format);
      bool Out =  vprintf<write>(format,ap);
      va_end(ap);
      return Out;
    } // printf
  }; // class Out
} // namespace avp


#endif /* IO_H_INCLUDED */
