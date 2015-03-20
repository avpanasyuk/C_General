/**
  * @file AVP_LIBS/General/Error.h
  * @author Alexander Panasyuk
  */

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include "IO.h"

namespace avp {
  extern volatile uint8_t FailReason;
  /// defined in General library as weak, sending stuff to ::vprinf
  /// may be redefined
  bool debug_printf(const char *format, ...);
  void major_fail(uint8_t reason = 0);
  void hang_cpu();
}; //avp

#ifdef DEBUG
# define AVP_ERROR_WITH_CODE(code,format, ...) do{ \
    avp::debug_printf("Error in file %s, line %d: " format, \
    __FILE__, __LINE__, ##__VA_ARGS__); avp::major_fail(code); }while(0)
/// assert with additional explanation
/// @code - numeric code, optional
/// @param ext_format - additional format string, followed by parameters
# define AVP_ASSERT_WITH_EXPL(exp,code,ext_format,...) do{ if(!(exp)) \
    { AVP_ERROR_WITH_CODE(code+0,"Expression (%s) is false: " ext_format "!\n", \
    #exp, ##__VA_ARGS__); }}while(0)

#else // RELEASE
# define AVP_ERROR_WITH_CODE(code,...) avp::major_fail(code)
# define AVP_ASSERT_WITH_EXPL(exp,...) do{ [] (...) { ((exp),##_VA_ARGS__); } }while(0) // we gotta execute exp and args
#endif // DEBUG

#define AVP_ASSERT_WITH_CODE(exp,code) AVP_ASSERT_WITH_EXPL(exp,code,)
#define AVP_ASSERT(exp) AVP_ASSERT_WITH_CODE(exp,)
#define AVP_ERROR(...) AVP_ERROR_WITH_CODE(0,##__VA_ARGS__)
#define ASSERT_BEING_0(exp) AVP_ASSERT((exp) == 0)

#endif /* ERROR_H_INCLUDED */
