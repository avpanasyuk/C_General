/**
 * @file class to send messages from interrupt handlers which uses circular buffer. 
 * "putchar" is very light, "call_in_loop" calls log_func with whatever is accumulated in the
 * buffer.
 * @author your name (you@domain.com)
 * @brief this class is used to provide lightweight messaging system useable from interrupts
 * @version 0.1
 * @date 2026-01-27
 *
 * @copyright Copyright (c) 2026
 * @note the FORCE_INLINE stuff is here so the functions an be called from ESP ISRs with IRAM_ATTR
 *
 */
#pragma once

#include <stddef.h>
#include "CircBufferWithCont.hpp"

namespace avp {
  template<uint8_t sizeLog2>
  struct ISR_Message {
    static inline CircBufferWithCont<char, sizeLog2, uint32_t> Buf;
    static constexpr char Overrun = '$';

    /**
     * Should be called in loop. If there is something to log calls log_func
     */
    static void call_in_loop(void (*log_func)(const char *s, int sz)) {
        const char *s = Buf.GetContinousBlockToRead();
      if(s != nullptr) {
        log_func(s, Buf.GetSizeToRead());
        Buf.FinishedReading();
      }
    } // debug_in_loop

    static int FORCE_INLINE putchar(char c) { // redefining one in common_c.c
      switch(Buf.LeftToWrite()) {
      case 0:
        return -1;
      case 1:
        Buf.Write_(Overrun);
        return -1;
      default:
        Buf.Write_(c);
        return c;
      }
    } // putchar
  }; // class ISR_Message
} // namespace avp


