/*
 * common_cpp.cpp
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
// Vector.h
#include <stdlib.h>
#include <stdio.h>

namespace avp {
  void (*bad_pointer_func)() = abort;
  void (*bad_index_func)() = abort;
  bool vprintf(bool (*pwrite)(const void *Ptr, size_t Size),char const *format, va_list ap) {
    size_t Size = vsnprintf(NULL,0,format,ap);
    if(Size < 0) return false;
    char Buffer[Size+1];
    vsprintf(Buffer,format,ap);
    return (*pwrite)((void *)Buffer,Size);
  } // vprintf
  bool printf(bool (*pwrite)(const void *Ptr, size_t Size), char const *format, ...) {
    va_list ap;
    va_start(ap,format);
    bool Out =  vprintf(pwrite,format,ap);
    va_end(ap);
    return Out;
  } // printf
}
