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
#include "../C_General/General_C.h"
#include "../C_General/Error.h"

// __weak int debug_puts(const char *s) { return fputs(s,stderr); }
__weak int debug_putchar(char c) { return fputc(c,stderr); }

__weak int debug_puts(const char *s) {
  while(*(s++))
    if(debug_putchar(*(s-1)) == -1)
      return -1;
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

__weak void debug_action() { };

__weak PRINTF_WRAPPER_C(int, debug_printf, debug_vprintf)

__weak void hang_cpu() { while(1); }

__weak void new_handler() { hang_cpu(); }

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

const char *svprintf_static(const char *format, va_list ap) {
  va_list ap_;
  va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
  int Size = vsnprintf(NULL, 0, format, ap_);
  if(Size < 0) return "string_vprintf: format is wrong!";
  static char *out = NULL;
  static size_t Reserved = 0;
  if(Size + 1 > Reserved) out = (char *)realloc(out, Reserved = 2*(Size + 1));
  if(out == NULL) return "svprintf_static: failed to reallocate memory!";
  vsprintf(out, format, ap);
  return out; // we do not write ending 0 byte
} // string_vprintf

PRINTF_WRAPPER_C(const char *,sprintf_alloc,svprintf_alloc)
PRINTF_WRAPPER_C(const char *,sprintf_static,svprintf_static)

uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t start) {
  uint16_t crc = start;

  while(len--) {
    crc ^= ((uint16_t)*(pcBlock++)) << 8;

    for(uint8_t i = 0; i < 8; i++)
      if((crc & 0x8000) != 0)
        crc = ((crc ^ 0x8810) << 1) + 1;
      else crc <<= 1;
  }
  return crc;
} // Crc16

#if __linux__

time_t millis() {
  struct timeval time_now { };
  gettimeofday(&time_now, nullptr);
  return (time_now.tv_sec * time_t(1000)) + (time_now.tv_usec / 1000);
} // millis

time_t micros() {
  struct timeval time_now { };
  gettimeofday(&time_now, nullptr);
  return (time_now.tv_sec * time_t(1000000L)) + time_now.tv_usec;
} // micros
#endif

