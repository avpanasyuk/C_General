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

void Fail::default_function() { abort(); }

namespace avp {
  Fail::function bad_index_func = Fail::default_function;
  Fail::function bad_pointer_func = Fail::default_function;

  int AssertError = 0;

  // Note about following two functions - they should be used with BLOCKED pwrite which
  // returns only after data pointed by Ptr are used. Or it makes a copy.
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

  __weak bool error_output(const void *Ptr, size_t Size) { return fwrite(Ptr, 1, Size, stderr) == Size; }
  __weak void hang_cpu() { while(1); }
IGNORE(-Wunused-variable)
  __weak void major_fail(uint8_t reason) { volatile uint8_t reason_copy = reason; hang_cpu(); }
STOP_IGNORING
}
