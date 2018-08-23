/*!
 * @file AVP_LIBS/General/General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */

#ifndef GENERAL_H_
#define GENERAL_H_

#include <stdint.h>
#include <stdarg.h>

#if 0 // cause reallu weird errors in c+11 and I think already built-in
// following are operators which can be universaly derived from others
template<typename T> inline T operator++(T &v) { return v += 1; }
template<typename T> inline T operator++(T &v, int) { T old(v); v += 1; return old; }
template<typename T> inline T operator--(T &v) { return v -= 1; }
template<typename T> inline T operator--(T &v, int) { T old(v); v -= 1; return old; }
template<typename T> inline T operator-(const T &x) { return 0 - x; } // unitary minus
#endif

#if 0
// DO NOT REDEFINE BUILT-IN OPERATORS!!!!! INFINITE RECURSION HELL
// DO NOT DEFINE BINARTY OPERATORS FROM UNITARY USING TEMPLATE FUNCTIONS. COMPLILER DOES NOT
// TRY USING IMPLICIT CONVERSION TO FIT PARAMETER TYPES
// USE TEMPLATE CLASS FRIEND FUNCTIONS - THEY ARE SYMMETRIC AND COMPILER TRIES
// IMPLICIT CONVERSIONS
// LIKE THIS: for template<typename type> class T:
// beginning of examble
#define CLASS xxxxx
#define BINARY_OP_FROM_SELF(op) \
  inline friend CLASS operator op (const CLASS &x1, const CLASS &x2) { return CLASS(x1) op##= x2; }

BINARY_OP_FROM_SELF(-)
BINARY_OP_FROM_SELF(+)
BINARY_OP_FROM_SELF(*)
BINARY_OP_FROM_SELF(/)
BINARY_OP_FROM_SELF(&)
BINARY_OP_FROM_SELF(|)
#undef BINARY_OP_FROM_SELF

inline friend bool operator==(CLASS const &v1, CLASS const &v2) { return equal(v1,v2); }
inline friend bool operator!=(CLASS const &v1, CLASS const &v2) { return !(v1 == v2); }

#endif  // end of example

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak,noinline))
#endif /* __weak */
#endif /* __GNUC__ */

/// creates printf-type function named "func_name" out of vprintf-type function named "vprinf_func"
/// usage: PRINTF_WRAPPER_BOOL(printf,vprintf)
#define PRINTF_WRAPPER(func_name, vprintf_func) \
  inline  __attribute__ ((format (printf, 1, 2))) bool func_name(char const *format, ...) \
  { va_list ap; va_start(ap,format); \
    bool Out =  vprintf_func(format,ap); va_end(ap); \
    return Out; }

namespace avp {
// to suppress unused-variable or unused-value
// volatile auto x = (unused-value-expression);
// avp::unused(x)
  template<typename T> void unused(T const &) {}

  template<bool (*vprintf_ptr)(char const *, va_list)>
  PRINTF_WRAPPER(printf,vprintf_ptr)

  std::string string_vprintf(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
  std::string string_printf(char const *format, ...) __attribute__ ((format (printf, 1, 2)));
} // avp


#endif /* GENERAL_H_ */
