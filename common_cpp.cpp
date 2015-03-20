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
#include "IO.h"

namespace avp {
  volatile uint8_t FailReason = 0;


  //! @brief defualt debug print sends things to stdout
  __weak bool debug_vprintf(const char *format, va_list ap) { return ::vprintf(format,ap) >= 0; }
  __weak void hang_cpu() { while(1); }
  __weak void major_fail(uint8_t reason) {
    FailReason = reason;
    hang_cpu();
  }
} // namespace avp

