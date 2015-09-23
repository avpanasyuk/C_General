/**
  * @file AVP_LIBS/General/Error.h
  * @author Alexander Panasyuk
  */

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include "Macros.h"

#ifdef __cplusplus
extern "C" int debug_printf(const char *format, ...);

#include "IO.h"

namespace avp {
  extern volatile uint8_t FailReason;
  /// defined in General library as weak, sending stuff to ::vprintf
  /// may be redefined
  bool debug_vprintf(const char *format, va_list a);
  void major_fail(uint8_t reason = 0);
  void hang_cpu();
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
# define AVP_ASSERT_WITH_EXPL(exp,code,ext_format,...) do{ (void)((exp),##__VA_ARGS__); }while(0)
// we gotta execute exp and args but do nothing else
#endif // DEBUG

#define AVP_ASSERT_WITH_CODE(exp,code) AVP_ASSERT_WITH_EXPL(exp,code,"")
#define AVP_ASSERT(exp) AVP_ASSERT_WITH_CODE(exp,0)
#define AVP_ERROR(...) AVP_ERROR_WITH_CODE(0,##__VA_ARGS__)
#define ASSERT_BEING_0(exp) AVP_ASSERT((exp) == 0)

#else
extern int debug_printf(const char *format, ...);
#endif


#endif /* ERROR_H_INCLUDED */
