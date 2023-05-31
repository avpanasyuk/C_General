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
/// usage: PRINTF_WRAPPER_BOOL(printf,vprintf)
#define PRINTF_WRAPPER(func_name, vprintf_func) \
    inline  __attribute__((format (printf, 1, 2))) bool func_name(char const *format, ...) \
    { va_list ap; va_start(ap,format); \
    bool Out =  vprintf_func(format,ap); va_end(ap); \
    return Out; }

/// @cond
#include <string.h>
#include <limits.h>
/// @endcond

char *svprintf_alloc(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
char *sprintf_alloc(char const *format, ...) __attribute__ ((format (printf, 1, 2)));

typedef void (*free_func_t)(void *);

#endif /* GENERAL_C_H_ */
