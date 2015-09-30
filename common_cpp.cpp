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

namespace avp {
  volatile uint8_t FailReason = 0;

  /// crap, ::vprintf does not work with semihosting
  ///__weak bool debug_vprintf(const char *format, va_list a) { return vprintf<write_buffered< ::printf>>(format,a); }
  __weak bool debug_vprintf(const char *format, va_list a) { return ::vprintf(format,a) >= 0; }
  __weak void hang_cpu() { while(1); }
  __weak void major_fail(uint8_t reason) {
    FailReason = reason;
    hang_cpu();
  }
} // namespace avp

int debug_printf(char const *format, ...) {
  va_list ap;
  va_start(ap,format);
  bool Out = avp::debug_vprintf(format,ap);
  va_end(ap);
  return Out?1:-1;
}
