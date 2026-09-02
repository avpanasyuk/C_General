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
 * @brief printf into a fixed static buffer (BUFFER_SIZE in common_c.c) — the NO-HEAP alternative to
 * Arduino String. Prefer this on memory-constrained firmware: it does no alloc/dealloc, so it cannot
 * fragment RAM (String's churn "muddies" memory). Returns a pointer to a shared static buffer that is
 * valid only until the next call — copy/consume it before calling again; never free it. Output longer
 * than the buffer is truncated (vsnprintf-safe, no overflow), so keep the buffer sized for the longest
 * expected string (e.g. a full HTML status line).
 */
const char *svprintf_static(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
const char *sprintf_static(char const *format, ...) __attribute__ ((format (printf, 1, 2)));

/// Stack staging area debug_vprintf formats into, per call. Deliberately NOT the shared
/// svprintf_static buffer, so a debug_*() call can never overwrite a sprintf_static() result
/// its caller is still holding. Longer output truncates (vsnprintf-safe). Override it on
/// RAM-tight targets -- 256 B of stack is cheap on ESP but not on an AVR.
#ifndef DEBUG_PRINTF_BUFFER_SIZE
#define DEBUG_PRINTF_BUFFER_SIZE 256
#endif

const char *svprintf_realloc(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
const char *sprintf_realloc(char const *format, ...) __attribute__ ((format (printf, 1, 2)));

typedef void (*free_func_t)(void *);

// reflected==0: MSB-first (shift left, test bit15) -- CRC16-CCITT family.
// reflected!=0: LSB-first (shift right, test bit0) -- pass the reflected poly
// (e.g. 0xA001 for Modbus-RTU) and crc init 0xFFFF.
// reflected==0: MSB-first (shift left, test bit7) -- Sensirion/SMBus family (poly 0x31).
// reflected!=0: LSB-first (shift right, test bit0) -- pass the reflected poly
// (e.g. 0x8C for Dallas/Maxim 1-Wire) and crc init 0x00.
uint8_t Crc8(const uint8_t *pcBlock, long long len, uint8_t crc, uint8_t poly, int reflected);
uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t crc, uint16_t poly, int reflected);
uint32_t Crc32(const uint8_t *pcBlock, long long len, uint32_t crc, uint32_t poly);

#ifdef __cplusplus
}
#endif
