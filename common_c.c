/*
 * common_c.c
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
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
  return debug_puts(svprintf_static(format, a));
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

__weak int debug_puts(const char *s) {
  while(*(s++))
    if(debug_putchar(*(s - 1)) == -1) return -1;
  return 0;
} // debug_puts

__weak int debug_vprintf(const char *format, va_list a) {
  return debug_puts(svprintf_static(format, a));
} // debug_vprintf

__weak int debug_puts_free(const char *s, free_func_t free_func) {
  int out = debug_puts(s);
  if(free_func != NULL) free_func((void *)s);
  return out;
} // debug_puts

__weak void debug_action() {};

__weak PRINTF_WRAPPER_C(int, debug_printf, debug_vprintf)

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
  if(Size < 0) return "string_vprintf: format is wrong!";
  static char *out = NULL;
  static size_t Reserved = 0;
  if(Size + 1 > Reserved) out = (char *)realloc(out, Reserved = 2 * (Size + 1));
  if(out == NULL) return "svprintf_static: failed to reallocate memory!";
  vsprintf(out, format, ap);
  return out; // we do not write ending 0 byte
} // string_vprintf

PRINTF_WRAPPER_C(const char *, sprintf_realloc, svprintf_realloc)

/*
 * pointer returned by this function should not be freed after use
 */
const char *svprintf_static(const char *format, va_list ap) {
  va_list ap_;
  va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
#define BUFFER_SIZE 200
  static char Buffer[BUFFER_SIZE];
  vsnprintf(Buffer, BUFFER_SIZE, format, ap);
  return Buffer; // we do not write ending 0 byte
} // string_vprintf

PRINTF_WRAPPER_C(const char *, sprintf_static, svprintf_static)

uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t crc, uint16_t poly) {
  while(len--) {
    crc ^= ((uint16_t)*(pcBlock++)) << 8;

    for(uint8_t i = 0; i < 8; ++i)
      if(crc & 0x8000)
        crc = (crc << 1) ^ poly;
      else crc <<= 1;
  }
  return crc;
} // Crc16
