/*!
 * @file ../C_General/General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */

 #pragma once
 

 /// @cond
#include <stdint.h>
#include <stdarg.h>
/// @endcond

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak,noinline))
#endif /* __weak */
#else
#define __attribute__(...)
#endif /* __GNUC__ */

/// creates printf-type function named "func_name" out of vprintf-type function named "vprinf_func"
/// usage: return_type PRINTF_WRAPPER_C(int,printf,vprintf)
#define PRINTF_WRAPPER_C(return_type,func_name,vprintf_func) \
   __attribute__((format (printf, 1, 2))) return_type func_name(const char *fmt, ...) \
    { va_list ap; va_start(ap, fmt); \
    return_type Out =  vprintf_func(fmt,ap); va_end(ap); \
    return Out; }

/// @cond
#include <string.h>
#include <limits.h>
/// @endcond

#ifdef __cplusplus
extern "C" {
#endif

/**
 * following two function allocate space for string every times using malloc, it needs eventually to be free"d"
 */
const char *svprintf_alloc(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
const char *sprintf_alloc(char const *format, ...) __attribute__ ((format (printf, 1, 2)));

/**
 * following two function return a pointer to the same memory every time, and reallocated the space for this memory as necessary
 * this memory does not need to be free"d"
 */
const char *svprintf_static(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
const char *sprintf_static(char const *format, ...) __attribute__ ((format (printf, 1, 2)));

const char *svprintf_realloc(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
const char *sprintf_realloc(char const *format, ...) __attribute__ ((format (printf, 1, 2)));

typedef void (*free_func_t)(void *);

uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t crc, uint16_t poly);
uint32_t Crc32(const uint8_t *pcBlock, long long len, uint32_t crc, uint32_t poly);

#ifdef __cplusplus
}
#endif
