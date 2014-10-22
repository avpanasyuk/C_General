/**
  * @file
  * @author Alexander Panasyuk
  */

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include "General.h"

namespace avp {
  extern int AssertError;

  bool error_output(volatile void *Ptr, size_t Size);
  void major_fail(uint8_t reason = 0);
  void hang_cpu();
}; //avp

#ifdef DEBUG
#define AVP_ERROR(format, ...) do{ \
 avp::printf<avp::vprintf<avp::error_output> >("Error in file %s, line %d: " format, \
             __FILE__, __LINE__, ##__VA_ARGS__); avp::major_fail(); }while(0)
// #else
// #define AVP_ERROR(format,...) major_fail();
#endif

// "exp" should return 0 if success and error code if not
#ifdef DEBUG

// POLICY: AVP_ASSERT is used just before a deadly operation, like assigning zero
// pointer, and not preliminary

#define AVP_ASSERT_(exp,err_form,...) do{ if((avp::AssertError = !(exp))) \
{ AVP_ERROR("Expression (%s) is false in %s on line %d" err_form "!\n", \
            #exp, __FILE__, __LINE__,##__VA_ARGS__); }}while(0)
#define AVP_ASSERT(exp) AVP_ASSERT_(exp,"")
#define ASSERT_BEING_0(exp) do{ if((avp::AssertError = (exp))) \
  { AVP_ERROR("Expression (%s) equals %d != 0 in %s on line %d!\n", \
            #exp, avp::AssertError, __FILE__, __LINE__); }}while(0)
#define AVP_ASSERT_ERRNUM(exp, ERRNUM) AVP_ASSERT_(exp,", error #%d", ERRNUM)
#else
#define AVP_ASSERT(exp,...) { if((avp::AssertError = !(exp))) avp::major_fail(##__VA_ARGS__); }
#define AVP_ASSERT_ERRNUM(exp,...) AVP_ASSERT(exp,##_VA_ARGS)
#define ASSERT_BEING_0(exp,...) { if((avp::AssertError = (exp))) avp::major_fail(##__VA_ARGS__); }
#endif

#endif /* ERROR_H_INCLUDED */
