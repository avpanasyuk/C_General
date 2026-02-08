/**
 *  @file
 *
 *  @author: panasyuk
 *  @brief:
 *
 *  @details
 */

#pragma once

#include "Error.h"
#include "BitBang.hpp"
#include "MyMath.hpp"

#ifndef AVP_RAM_ATTR
#define AVP_RAM_ATTR // set to IRAM_ATTR for ESP
#else
#if defined(ESP32)
#include <esp_attr.h>
#endif
#if defined(ESP8266)
#include <Arduino.h>
#endif 
#endif


template<typename T>
int inline AVP_RAM_ATTR debug_put_binary(T x, bool LeadingZeroes = true) {
  for(unsigned i = 0; i < sizeof(T)*8; ++i) { 
    if((x = avp::rotate_left(x)) & 1) {
      if(debug_putchar('1') == -1) return -1;
      LeadingZeroes = true;
    } else if(LeadingZeroes) if(debug_putchar('0') == -1) return -1;
  }
  return 0;
} // debug_put_binary<>

template<typename T>
int inline AVP_RAM_ATTR debug_put_hex(T x, bool LeadingZeroes = true) {
  static const char Chars[] = "0123456789ABCDEF";

  for(unsigned i = 0; i < sizeof(T)*2; ++i) {
    auto v = (x = avp::rotate_left(x,4)) & 0xF; 
    if(v) {
      if(debug_putchar(Chars[v]) == -1) return -1;
      LeadingZeroes = true;
    } else if(LeadingZeroes) if(debug_putchar('0') == -1) return -1;
  }
} // debug_put_hex<>

template<typename T>
int inline AVP_RAM_ATTR debug_put_decimal(T x) {
  if(x < 0) {
    debug_putchar('-');
    return debug_put_decimal<T>(-x);
  }
  char B[avp::CeilRatio(sizeof(x)*8,3U)];

  uint_fast8_t i = 0;
  do {
    T x1 = x; x /= 10; B[i++] = '0' + x1 - x*10;
  } while(x);
  do {
    if(debug_putchar(B[i-1]) == -1) return -1; 
  } while(--i);
  return 1; 
} // debug_put_decimal

#undef DEBUG_PUT_PLACE // comes from Error.h
#define DEBUG_PUT_PLACE do { \
  debug_puts(__PRETTY_FUNCTION__); \
  debug_puts(" in " __FILE__ " on line "); \
  debug_put_decimal(__LINE__); \
} while(0);


#ifndef NO_STL
#include "General.hpp"

#define RETURN_ERROR_STRING(s) do{\
  return avp::string_printf("\"%s\" in %s, file " __FILE__ ", line %d\n",\
        s, __PRETTY_FUNCTION__, __LINE__);\
}while(0)


#define ASSERT_ELSE_RETURN(exp) do{ \
    if (!(exp)) RETURN_ERROR_STRING("\"" #exp "\" is false"); \
}while(0)

/**
 ERRNO_ASSERT_ELSE_RETURN works with functions returning -1 when failed and filling errno
 */
#include <cstring>
#include <cerrno>
#define ERRNO_ASSERT_ELSE_RETURN(exp) do{ \
    if((exp) == -1) RETURN_ERROR_STRING(strerror(errno)); \
}while(0)

/**
 * "exp" return error string, success is when the string is empty
 */
#define ASSERT_EMPTY_STRING(exp) do{\
    auto ErrStr = (exp); \
    if(!ErrStr.empty()) AVP_ERROR_PRINTF("%s",ErrStr.c_str()); \
}while(0)

/**
 * "exp" return error string, success is when the string is empty
 */
#define ASSERT_EMPTY_STRING_ELSE_RETURN(exp) do{\
    auto ErrStr = (exp); \
    if (!ErrStr.empty()) RETURN_ERROR_STRING(ErrStr.c_str()); \
}while(0)

#endif // NO_STL

#if defined(USE_EXCEPTIONS) && USE_EXCEPTIONS != 0

#define AVP_THROW(exception,format,...) do{ throw exception(AVP_DEBUG_PRINTF(format,  ##__VA_ARGS__)); } while(0)

#endif
