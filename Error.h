/**
  * @file
  * @author Alexander Panasyuk
  */

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include "General.h"

namespace avp {
  extern int AssertError;

  bool error_output(const void *Ptr, size_t Size);
  void major_fail(uint8_t reason = 0);
  void hang_cpu();
}; //avp

#ifdef DEBUG
#define AVP_ERROR(format, ...) \
 avp::printf(avp::error_output, "Error in file %s, line %d: " format, \
             __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define AVP_ERROR(format,...) major_fail();
#endif

// "exp" should return 0 if success and error code if not
#ifdef DEBUG
#define AVP_ASSERT(exp) do{ if((avp::AssertError = !exp)) \
{ AVP_ERROR("Expression (%s) is false in %s on line %d!\n", \
            #exp, __FILE__, __LINE__); }}while(0)
#define ASSERT_BEING_0(exp) do{ if((avp::AssertError = exp)) \
  { AVP_ERROR("Expression (%s) equals %d != 0 in %s on line %d!\n", \
            #exp, avp::AssertError, __FILE__, __LINE__); }}while(0)
#else
#define AVP_ASSERT(exp) { if((avp::AssertError = !exp)) major_fail(); }
#define ASSERT_BEING_0(exp) { if((avp::AssertError = exp)) major_fail(); }
#endif

#endif /* ERROR_H_INCLUDED */
