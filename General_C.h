/*!
 * @file AVP_LIBS/General/General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */

#ifndef GENERAL_C_H_
#define GENERAL_C_H_

/// @cond
#include <stdint.h>
#include <stdarg.h>
/// @endcond

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak,noinline))
#endif /* __weak */
#endif /* __GNUC__ */

/// creates printf-type function named "func_name" out of vprintf-type function named "vprinf_func"
/// usage: return_type PRINTF_WRAPPER_BOOL(printf,vprintf)
#define PRINTF_WRAPPER(return_type, func_name, vprintf_func) \
    __attribute__((format (printf, 1, 2))) return_type func_name(char const *format, ...) \
    { va_list ap; va_start(ap,format); \
    return_type Out =  vprintf_func(format,ap); va_end(ap); \
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
char *svprintf_alloc(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
char *sprintf_alloc(char const *format, ...) __attribute__ ((format (printf, 1, 2)));

/**
 * following two function return a pointer to the same memory every time, and reallocated the space for this memory as necessary
 * this memory does not need to be free"d"
 */
char *svprintf_static(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
char *sprintf_static(char const *format, ...) __attribute__ ((format (printf, 1, 2)));

typedef void (*free_func_t)(void *);

uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t start);

#ifdef __cplusplus
}
#endif


#endif /* GENERAL_C_H_ */
