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
  int AssertError = 0;
  volatile uint8_t FailReason = 0;


  __weak bool debug_write(const uint8_t *Ptr, size_t Size) {
    return fwrite((const void *)Ptr, 1, Size, stdout) == Size;
  }
  __weak void hang_cpu() { while(1); }
  __weak void major_fail(uint8_t reason) {
    FailReason = reason;
    hang_cpu();
  }
} // namespace avp

