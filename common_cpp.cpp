/*
 * common_cpp.cpp
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
// Vector.h
#include <stdlib.h>
#include <stdio.h>
#include "General.h"
#include "Vector.h"

void Fail::default_function() { abort(); }

namespace avp {
  Fail::function bad_index_func = Fail::default_function;
  Fail::function bad_pointer_func = Fail::default_function;

  bool vprintf(bool (*pwrite)(const void *Ptr, size_t Size),char const *format, va_list ap) {
    int Size = vsnprintf(NULL,0,format,ap);
    if(Size < 0) return false;
    char Buffer[Size+1]; // +1 to include ending zero byte
    vsprintf(Buffer,format,ap);
    return (*pwrite)((void *)Buffer,Size); // we do not write ending 0 byte
  } // vprintf
  bool printf(bool (*pwrite)(const void *Ptr, size_t Size), char const *format, ...) {
    va_list ap;
    va_start(ap,format);
    bool Out =  vprintf(pwrite,format,ap);
    va_end(ap);
    return Out;
  } // printf
}
