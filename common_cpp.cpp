/*
 * common_cpp.cpp
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
 /// @cond
#include <stdlib.h>
#include <stdio.h>
#include <string>
/// @endcond
#include "General.h"
#include "Error.h"
#include "BitBang.h"

volatile uint8_t FailReason = 0;

#ifndef NO_STL
namespace avp {
  std::string string_vprintf(const char *format, va_list ap) {
    va_list ap_;
    va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
    int Size = vsnprintf(nullptr, 0, format, ap_);
    if(Size < 0) return "string_vprintf: format is wrong!";
    char Buffer[Size + 1]; // +1 to include ending zero byte
    vsprintf(Buffer, format, ap);
    return std::string(Buffer, Size); // we do not write ending 0 byte
  } // string_vprintf

  PRINTF_WRAPPER(std::string, string_printf, string_vprintf)
} // namnespace avp
#endif

#if __linux__

  time_t millis() {
    struct timeval time_now { };
    gettimeofday(&time_now, nullptr);
    return (time_now.tv_sec * time_t(1000)) + (time_now.tv_usec / 1000);
  } // millis

  time_t micros() {
    struct timeval time_now { };
    gettimeofday(&time_now, nullptr);
    return (time_now.tv_sec * time_t(1000000L)) + time_now.tv_usec;
  } // micros
#endif

