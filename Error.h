/**
  * @file AVP_LIBS/General/Error.h
  * @author Alexander Panasyuk
  */

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdint.h>
#include "IO.h"

namespace avp {
  extern int AssertError;

  bool error_output(const uint8_t *Ptr, size_t Size);
  void major_fail(uint8_t reason = 0);
  void hang_cpu();
}; //avp

namespace Fail {
  typedef void (*function)();
  extern void default_function();
} // Fail

#ifdef DEBUG
#define AVP_ERROR(format, ...) do{ \
 avp::printf<avp::vprintf<avp::error_output> >("Error in file %s, line %d: " format, \
             __FILE__, __LINE__, ##__VA_ARGS__); avp::major_fail(); }while(0)
// POLICY: AVP_ASSERT is used just before a deadly operation, like assigning zero
// pointer, and not preliminary
#define AVP_ASSERT_(exp,err_form,...) do{ if((avp::AssertError = !(exp))) \
{ AVP_ERROR("Expression (%s) is false in %s on line %d" err_form "!\n", \
            #exp, __FILE__, __LINE__,##__VA_ARGS__); }}while(0)
#define ASSERT_BEING_0(exp) do{ if(avp::AssertError = (exp)) \
  { AVP_ERROR("Expression (%s) equals %d != 0 in %s on line %d!\n", \
            #exp, avp::AssertError, __FILE__, __LINE__); }}while(0)
#else
#define AVP_ERROR(format,...) avp::major_fail()
#define AVP_ASSERT_(exp,...) do{ (exp); }while(0)
#define ASSERT_BEING_0(exp,...) AVP_ASSERT(exp)
#endif

#define AVP_ASSERT(exp) AVP_ASSERT_(exp,"")
#define AVP_ASSERT_ERRNUM(exp, ERRNUM) AVP_ASSERT_(exp,", error #%d", ERRNUM)

#endif /* ERROR_H_INCLUDED */
