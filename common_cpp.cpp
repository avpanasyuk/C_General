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
#include "CommandTable.h"

volatile uint8_t FailReason = 0;
void new_handler() { major_fail(MEMALLOC); }


/// crap, ::vprintf does not work with semihosting
///__weak bool debug_vprintf(const char *format, va_list a) { return vprintf<write_buffered< ::printf>>(format,a); }
__weak int debug_vprintf(const char *format, va_list a) { return ::vprintf(format,a); }
__weak void hang_cpu() { while(1); }

__weak void major_fail(uint8_t reason) {
  FailReason = reason;
  hang_cpu();
}
__weak void debug_action() {};

int debug_printf(char const *format, ...) {
  va_list ap;
  va_start(ap,format);
  bool Out = debug_vprintf(format,ap) >= 0;
  va_end(ap);
  return Out?1:-1;
}



