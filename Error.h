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

// "exp" should return 0 if success and error code if not
#ifdef DEBUG
#define AVP_ASSERT(exp) do{ if((avp::AssertError = !exp)) \
{ avp::printf(avp::error_output,"Expression (" #exp ") is false in " \
              __FILE__ " on line %d!\n", __LINE__); }}while(0)
#define ASSERT_BEING_0(exp) do{ if((avp::AssertError = exp)) \
  { avp::printf(avp::error_output,"Expression (" #exp ") equals %d != 0 in " \
                __FILE__ " on line %d!\n", avp::AssertError, __LINE__); }}while(0)
#else
#define AVP_ASSERT(exp) { if((avp::AssertError = !exp)) major_fail(); }
#define ASSERT_BEING_0(exp) { if((avp::AssertError = exp)) major_fail(); }
#endif

#endif /* ERROR_H_INCLUDED */
