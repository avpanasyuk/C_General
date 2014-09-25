/*
 * common_cpp.cpp
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
#include <stdlib.h>
#include "General.h"

void Fail::default_function() { abort(); }

namespace avp {
  Fail::function bad_index_func = Fail::default_function;
  Fail::function bad_pointer_func = Fail::default_function;

  int AssertError = 0;

  __weak bool error_output(volatile void *Ptr, size_t Size) { return fwrite((const void *)Ptr, 1, Size, stderr) == Size; }
  __weak void hang_cpu() { while(1); }
IGNORE(-Wunused-variable)
  __weak void major_fail(uint8_t reason) { volatile uint8_t reason_copy = reason; hang_cpu(); }
STOP_IGNORING
} // namespace avp
