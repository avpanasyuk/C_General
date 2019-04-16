/*
 * BgMessage.h
 *
 * Created: 9/21/2015 3:05:28 PM
 *  Author: panasyuk
 *  @brief This template class used to separate writing out text messages in two parts:
 *  background function which stores messages into internal buffer and not calling "write_func" function
 *  and foreground "WriteOut" function which does call "write_func".
 *  Useful e.g to write diagnostics messages from the background which postpone actual writing until it
 *  is safe andprotocol is not getting screwed
 *
 * We will use two buffers, because "write_func" function called by "WriteOut" may call "Append" itself
 * written while other is being sent.
 */


#ifndef BGMESSAGE_H_
#define BGMESSAGE_H_

#include <stdlib.h>
#include <stdarg.h>
#include "IO.h"

namespace avp {
  template<write_type_func write_func, size_t BufferSize, char OverrunIndicator = '~'>
  class BG_message { //
      static uint8_t CurBuffer; //!< toggles between two buffers indexed 0 and 1
      static class Buffer_ {
        protected:
          size_t FilledBytes;
          char Chars[BufferSize];
        public:

          Buffer_(): FilledBytes(0) {}

          IGNORE_WARNING(-Wsign-compare)
          bool vAppend(const char *format, va_list ap) {
            size_t Space = BufferSize-FilledBytes; // space for text and ending 0, we will replace 0 with OverrunIndicator there
            // to indicate overrun
            if(Space != 0) {
              int Size = vsnprintf(Chars+FilledBytes,Space,format,ap);
              // gcc vsnprintf always return full size as if string was not truncated. This size is without 0, but 0 is
              // always written
              if(Size < 0) return false;
              if(Size >= Space) Chars[(FilledBytes = BufferSize)-1] = OverrunIndicator;
              // we store ~ as a last character to indicate that there was more, but it did not fit
              else FilledBytes += Size;
            }
            return true;
          } // vAppend
          STOP_IGNORING_WARNING


          /**
          @brief this function should be called repeatedly as a part of main program loop to maintain background message facility.
          Background messages allow to pass information from e.g. interrupt handlers when using normal messaging would cause race
          conditions and excess delays
          */
          void WriteOut() {
            if(FilledBytes) {
              write_func((uint8_t *)Chars,FilledBytes); // we send size byte and a buffer in one go.
              FilledBytes = 0;
            }
          } // WriteOut
      } Buffer[2];
    public:
      //! does not do any immediate writing to a serial port, so does not call bg_loop
      static bool vAppend(const char *format, va_list ap) { return Buffer[CurBuffer].vAppend(format,ap); }
      static PRINTF_WRAPPER(printf,vAppend);

      static void WriteOut() { Buffer[1 - (CurBuffer = 1 - CurBuffer)].WriteOut(); } // switches buffers as well
  }; // class BG_message

#define _TEMPLATE_DECL_  template<write_type_func write_func, size_t BufferSize, char OverrunIndicator>
#define _TEMPLATE_SPEC_  BG_message<write_func, BufferSize, OverrunIndicator>

  _TEMPLATE_DECL_ uint8_t _TEMPLATE_SPEC_::CurBuffer = 0;
  _TEMPLATE_DECL_ class _TEMPLATE_SPEC_::Buffer_ _TEMPLATE_SPEC_::Buffer[2];

#undef _TEMPLATE_DECL_
#undef _TEMPLATE_SPEC_
} // avp

#endif /* BGMESSAGE_H_ */
