/*
 * common_cpp.cpp
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
#include <stdlib.h>
#include <stdio.h>
#include "General.h"
#include "Macros.h"
#include "IO.h"
#include "CircBuffer.h"
#include "Error.h"

void Fail::default_function() { abort(); }

namespace avp {
  Fail::function bad_index_func = Fail::default_function;
  Fail::function bad_pointer_func = Fail::default_function;

  int AssertError = 0;

  __weak bool error_output(const uint8_t *Ptr, size_t Size) { return fwrite((const void *)Ptr, 1, Size, stderr) == Size; }
  __weak void hang_cpu() { while(1); }
  IGNORE(-Wunused-variable)
  __weak void major_fail(uint8_t reason) { volatile uint8_t reason_copy = reason; hang_cpu(); }
  STOP_IGNORING

  namespace bg_error {
    static CircBuffer<char> Buffer;

    bool put_byte(uint8_t b) { if(!Buffer.LeftToWrite()) return false; Buffer.Write(b); return true; }
    void process() {
      if(Buffer.LeftToRead()) {
        uint8_t Sz;
        const char *p = Buffer.GetContinousBlockToRead(&Sz);

        avp::error_output((const uint8_t *)p,Sz);
        Buffer.FinishedReading();
      }
    } // cycle
  } // namespace bg_error

} // namespace avp
