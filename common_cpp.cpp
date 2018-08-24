/*
 * common_cpp.cpp
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
#include <stdlib.h>
#include <stdio.h>
#include "General.h"
#include "Error.h"

volatile uint8_t FailReason = 0;
void new_handler() { major_fail(MEMALLOC); }

__weak int debug_vprintf(const char *format, va_list a) { return ::vprintf(format,a); }
__weak void hang_cpu() { while(1); }

__weak void major_fail(uint8_t reason) {
  FailReason = reason;
  hang_cpu();
}
__weak void debug_action() {};

__weak int debug_printf(char const *format, ...) {
  va_list ap;
  va_start(ap,format);
  bool Out = debug_vprintf(format,ap) >= 0;
  va_end(ap);
  return Out?1:-1;
}
#ifndef NO_STL
namespace avp {
  std::string string_vprintf(const char *format, va_list ap) {
    va_list ap_;
    va_copy(ap_,ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
    int Size = vsnprintf(nullptr,0,format,ap_);
    if(Size < 0) return "string_vprintf: format is wrong!";
    char Buffer[Size+1]; // +1 to include ending zero byte
    vsprintf(Buffer,format,ap);
    return std::string(Buffer,Size); // we do not write ending 0 byte
  } // string_vprintf

 std::string string_printf(char const *format, ...) {
    va_list ap;
    va_start(ap,format);
    std::string Out =  string_vprintf(format,ap);
    va_end(ap);
    return Out;
  } // string_printf
} // namnespace avp#endif


