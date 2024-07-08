/*
 * common_c.c
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
/// @cond
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/// @endcond
#include "../C_General/General_C.h"
#include "../C_General/Error.h"

#ifdef EB_MONITOR
extern int _write(int file, char *ptr, int len);

__weak int debug_puts(const char *s) {
  return _write(0, (char *)s, strlen(s)+1);
}

__weak int debug_vprintf(const char *format, va_list a) {
  return debug_puts(svprintf_static(format, a));
} // debug_vprintf

__weak int debug_puts_free(const char *s, free_func_t free_func) {
  int out = debug_puts(s);
  if(free_func != NULL) free_func((void *)s);
  return out;
} // debug_puts

#else // EB_MONITOR

__weak int debug_vprintf(const char *format, va_list a) {
  return vprintf(format, a);
} // debug_vprintf

#endif // EB_MONITOR

__weak void debug_action() { };

__weak int debug_printf(char const *format, ...) {
  va_list ap;
  va_start(ap, format);
  int Out = debug_vprintf(format, ap) >= 0;
  va_end(ap);
  return Out;
}
__weak void hang_cpu() {
  while(1);
}

__weak void new_handler() {
  hang_cpu();
}

char *svprintf_alloc(const char *format, va_list ap) {
  va_list ap_;
  va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
  int Size = vsnprintf(NULL, 0, format, ap_);
  if(Size < 0) return "svprintf_alloc: format is wrong!";
  char *out = (char *)malloc(Size + 1); // +1 to include ending zero byte
  if(out == NULL) return "svprintf_alloc: failed to allocate memory!";
  vsprintf(out, format, ap);
  return out;
} // svprintf_alloc

PRINTF_WRAPPER(char *, sprintf_alloc, svprintf_alloc)

char *svprintf_realloc(const char *format, va_list ap) {
  va_list ap_;
  va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
  int Size = vsnprintf(NULL, 0, format, ap_);
  if(Size < 0) return "string_vprintf: format is wrong!";
  static char *out = NULL;
  static size_t Reserved = 0;
  if(Size + 1 > Reserved) out = (char *)realloc(out, Reserved = 2*(Size + 1));
  if(out == NULL) return "svprintf_static: failed to reallocate memory!";
  vsprintf(out, format, ap);
  return out;
} // svprintf_realloc

#define MAX_SVPRINTF_BUFFER_SIZE  200
char *svprintf_static(const char *format, va_list ap) {
  va_list ap_;
  va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
  int Size = vsnprintf(NULL, 0, format, ap_);
  if(Size < 0) return "string_vprintf: format is wrong!";
  static char Buffer[MAX_SVPRINTF_BUFFER_SIZE] = {0};
  vsnprintf(Buffer, MAX_SVPRINTF_BUFFER_SIZE-1, format, ap);
  return Buffer;
} // string_vprintf

PRINTF_WRAPPER(char *, sprintf_static, svprintf_static)

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

