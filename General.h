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

namespace avp {
// to suppress unused-variable or unused-value
// volatile auto x = (unused-value-expression);
// avp::unused(x)
  template<typename T> void unused(T const &) {}
}#ifndef NO_STL#include <string>
namespace avp {
  std::string string_vprintf(const char *format, va_list a) __attribute__ ((format (printf, 1, 0)));
  std::string string_printf(char const *format, ...) __attribute__ ((format (printf, 1, 2)));
} // avp

#endif
#endif /* GENERAL_H_ */
