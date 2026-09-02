/*
 * common_c.c
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */

#if defined(ESP32)
/// @cond
#include <esp_attr.h>
/// @endcond
#endif

#if defined(ESP8266)
/// @cond
#include <Arduino.h>
/// @endcond
#endif

#ifndef AVP_RAM_ATTR
#define AVP_RAM_ATTR
#endif

/// @cond
#include <stdlib.h>
#include <stdio.h>
/// @endcond
#include "General.h"
#include "Error.h"

// __weak int debug_puts(const char *s) { return fputs(s,stderr); }
#ifdef _MSC_VER

#pragma comment(linker, "/alternatename:debug_putchar=debug_putchar_default")
int debug_putchar_default(char c) { return fputc(c, stderr); }

#pragma comment(linker, "/alternatename:debug_puts=debug_puts_default")
int debug_puts_default(const char *s) {
  while(*(s++))
    if(debug_putchar(*(s - 1)) == -1)
      return -1;
  return 0;
} // debug_puts

#pragma comment(linker, "/alternatename:debug_vprintf=debug_vprintf_default")
int debug_vprintf_default(const char *format, va_list a) {
  char buf[DEBUG_PRINTF_BUFFER_SIZE]; // private to this call -- see the note by svprintf_static
  vsnprintf(buf, sizeof buf, format, a);
  return debug_puts(buf);
} // debug_vprintf

#pragma comment(linker, "/alternatename:debug_puts_free=debug_puts_free_default")
int debug_puts_free_default(const char *s, free_func_t free_func) {
  int out = debug_puts(s);
  if(free_func != NULL) free_func((void *)s);
  return out;
} // debug_puts

#pragma comment(linker, "/alternatename:debug_action=debug_action_default")
void debug_action_default() {};

#pragma comment(linker, "/alternatename:debug_printf=debug_printf_default")
PRINTF_WRAPPER_C(int, debug_printf_default, debug_vprintf)

#pragma comment(linker, "/alternatename:hang_cpu=hang_cpu_default")
void hang_cpu_default() {
  fflush(stderr);
  while(1);
}

#pragma comment(linker, "/alternatename:new_handler=new_handler_default")
void new_handler_default() { hang_cpu(); }
#else
__weak int debug_putchar(char c) { return fputc(c, stderr); }

__weak int AVP_RAM_ATTR debug_puts(const char *s) {
  while(*s)
    if(debug_putchar(*(s++)) == -1) return -1;
  return 0;
} // debug_puts

__weak int AVP_RAM_ATTR debug_vprintf(const char *format, va_list a) {
  char buf[DEBUG_PRINTF_BUFFER_SIZE]; // private to this call -- see the note by svprintf_static
  vsnprintf(buf, sizeof buf, format, a);
  return debug_puts(buf);
} // debug_vprintf

__weak int AVP_RAM_ATTR debug_puts_free(const char *s, free_func_t free_func) {
  int out = debug_puts(s);
  if(free_func != NULL) free_func((void *)s);
  return out;
} // debug_puts

__weak void debug_action() {};

__weak AVP_RAM_ATTR PRINTF_WRAPPER_C(int, debug_printf, debug_vprintf)

__weak void hang_cpu() {
  fflush(stderr);
  while(1);
}

__weak void new_handler() { hang_cpu(); }
#endif

/*
 * pointer returned by this function has to be freed after use
 */
const char *svprintf_alloc(const char *format, va_list ap) {
  va_list ap_;
  va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
  int Size = vsnprintf(NULL, 0, format, ap_);
  va_end(ap_);
  if(Size < 0) return "svprintf_alloc: format is wrong!";
  char *out = (char *)malloc(Size + 1); // +1 to include ending zero byte
  if(out == NULL) return "svprintf_alloc: failed to allocate memory!";
  vsprintf(out, format, ap);
  return out; // we do not write ending 0 byte
} // svprintf_alloc

PRINTF_WRAPPER_C(const char *, sprintf_alloc, svprintf_alloc)

/*
 * pointer returned by this function should not be freed after use
 */
const char *svprintf_realloc(const char *format, va_list ap) {
  va_list ap_;
  va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
  int Size = vsnprintf(NULL, 0, format, ap_);
  va_end(ap_);
  if(Size < 0) return "string_vprintf: format is wrong!";
  static char *out = NULL;
  static size_t Reserved = 0;
  if(Size + 1 > Reserved) out = (char *)realloc(out, Reserved = 2 * (Size + 1));
  if(out == NULL) return "svprintf_realloc: failed to reallocate memory!";
  vsprintf(out, format, ap);
  return out; // we do not write ending 0 byte
} // string_vprintf

PRINTF_WRAPPER_C(const char *, sprintf_realloc, svprintf_realloc)

/*
 * pointer returned by this function should not be freed after use
 */
/* Buffer is shared by every caller, so a second call overwrites a string the first
 * caller may still be holding -- consume the result before calling again. debug_vprintf
 * stages into its own stack buffer, so at least no debug_*() call can clobber a
 * sprintf_static() result.
 */
const char *svprintf_static(const char *format, va_list ap) {
// Sized for a full HTML status line. 256 was not enough: the SDP810 node's /status page reaches
// 290 bytes once a sensor error string is appended -- so it truncated precisely in the case the
// page exists to report. vsnprintf truncates safely (no overflow), which is what makes the
// failure silent and worth over-sizing against. 512 costs 256 bytes of static RAM per program.
#define BUFFER_SIZE 512
  static char Buffer[BUFFER_SIZE];
  vsnprintf(Buffer, BUFFER_SIZE, format, ap);
  return Buffer; // we do not write ending 0 byte
} // string_vprintf

PRINTF_WRAPPER_C(const char *, sprintf_static, svprintf_static)

uint8_t Crc8(const uint8_t *pcBlock, long long len, uint8_t crc, uint8_t poly, int reflected) {
  while(len--) {
    crc ^= *(pcBlock++);
    for(uint8_t i = 0; i < 8; ++i)
      if(reflected) crc = (crc & 1) ? (crc >> 1) ^ poly : (crc >> 1);   // LSB-first (Dallas/Maxim)
      else crc = (crc & 0x80) ? (crc << 1) ^ poly : (crc << 1);         // MSB-first (Sensirion/SMBus)
  }
  return crc;
} // Crc8

uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t crc, uint16_t poly, int reflected) {
  while(len--) {
    if(reflected) { // LSB-first (e.g. Modbus-RTU with poly 0xA001)
      crc ^= (uint16_t)*(pcBlock++);
      for(uint8_t i = 0; i < 8; ++i)
        crc = (crc & 1) ? (crc >> 1) ^ poly : (crc >> 1);
    } else {        // MSB-first (CRC16-CCITT family)
      crc ^= ((uint16_t)*(pcBlock++)) << 8;
      for(uint8_t i = 0; i < 8; ++i)
        crc = (crc & 0x8000) ? (crc << 1) ^ poly : (crc << 1);
    }
  }
  return crc;
} // Crc16

uint32_t Crc32(const uint8_t *pcBlock, long long len, uint32_t crc, uint32_t poly) {
  while(len--) {
    crc ^= *(pcBlock++);

    for(uint8_t i = 0; i < 8; ++i)
      if(crc & 1u) crc = (crc >> 1) ^ poly;
      else crc >>= 1;
  }
  return crc;
} // Crc32
