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
 *
 */
#pragma once

#ifndef AVP_RAM_ATTR
#define AVP_RAM_ATTR // set to IRAM_ATTR for ESP
#else
#if defined(ESP32) || defined(ESP8266)
#include <esp_attr.h>
#endif 
#endif

#include <stddef.h>
#include "CircBufferWithCont.hpp"

namespace avp {
  template<uint8_t sizeLog2>
  struct DebugMessage {
    static inline CircBufferWithCont<char, sizeLog2, uint32_t> Buf;
    static constexpr char Overrun = '$';

    /**
     * Should be called in loop. If there is something to log calls log_func
     */
    static void call_in_loop(void (*log_func)(const char *s, int sz)) {
      if(Buf.LeftToRead()) {
        const char *s = Buf.GetContinousBlockToRead();
        log_func(s, Buf.GetSizeToRead());
        Buf.FinishedReading();
      }
    } // debug_in_loop

    static inline int AVP_RAM_ATTR putchar(char c) { // redefining one in common_c.c
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
  }; // class DebugMessage
} // namespace avp


