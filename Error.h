/**
  * @file AVP_LIBS/General/Error.h
  * @author Alexander Panasyuk
  * @note - to use MACROS you have to properly implement "avp::debug_vprintf".
  * it is weakly linked in service.c as an output to stderr which often does
  * not work
  */

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "Macros.h"

#ifdef __cplusplus
extern "C" int debug_printf(const char *format, ...);

namespace avp {
  extern volatile uint8_t FailReason;
  typedef void (*failfunc_type)(uint8_t reason) __attribute__((noreturn));

  /// defined in General library as weak, sending stuff to ::vprintf
  /// may be redefined
  bool debug_vprintf(const char *format, va_list a);
  /// @param reason - 1 _MALLOC fail, other are user defined
  void major_fail(uint8_t reason = 0) __attribute__((noreturn));
  void hang_cpu() __attribute__((noreturn));
}; //avp

// ************************* ASSERT/ERROR macros **********************
// ERROR halts program even in RELEASE
// ASSERT works only in DEBUG

#ifdef DEBUG
# define AVP_ERROR_WITH_CODE(code,format, ...) do{ \
    debug_printf("Error in file %s, line %d: " format, \
                 __FILE__, __LINE__, ##__VA_ARGS__); avp::major_fail(code); }while(0)

/// AVP_ASSERT_WITH_EXPL = AVP_ASSERT_WITH_CODE with additional explanation
/// @code - numeric code, optional
/// @param ext_format - additional format string, followed by parameters
# define AVP_ASSERT_WITH_EXPL(exp,code,ext_format,...) do{ if(!(exp)) \
    { AVP_ERROR_WITH_CODE(code+0,"Expression (%s) is false: " ext_format "!\n", \
                          #exp, ##__VA_ARGS__); }}while(0)

#else // RELEASE
# define AVP_ERROR_WITH_CODE(code,format,...) avp::major_fail(code)
# define AVP_ASSERT_WITH_EXPL(exp,code,ext_format,...) do{ (void)((void)(exp),##__VA_ARGS__); }while(0)
// we gotta execute exp and args but do nothing else
#endif // DEBUG

#define AVP_ASSERT_WITH_CODE(exp,code) AVP_ASSERT_WITH_EXPL(exp,code,"")
#define AVP_ASSERT(exp) AVP_ASSERT_WITH_CODE(exp,0)
#define AVP_ERROR(...) AVP_ASSERT_WITH_EXPL(0,1,##__VA_ARGS__)
#define ASSERT_BEING_0(exp) AVP_ASSERT((exp) == 0)

#else
extern int debug_printf(const char *format, ...);
#endif


#endif /* ERROR_H_INCLUDED */
