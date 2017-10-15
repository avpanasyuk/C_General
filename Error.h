/**
  * @file AVP_LIBS/General/Error.h
  * @author Alexander Panasyuk
  * @note - to use MACROS you have to properly implement "avp::debug_vprintf".
  * it is weakly linked in service.c as an output to stderr which often does
  * not work
  * @note AVP_ASSERT in Release built DOES NOT CHECK EXPRESSION!
  */

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "Macros.h"

#ifdef __cplusplus
extern "C" {
#endif
  int debug_printf(const char *format, ...);

  extern volatile uint8_t FailReason;
  typedef void (*failfunc_type)(uint8_t reason); //  __attribute__((noreturn));

  /// defined in General library as weak, sending stuff to ::vprintf
  /// may be redefined
  int debug_vprintf(const char *format, va_list a);

  enum MAJOR_FAIL_REASONS_0 {MEMALLOC = 1,NUM_FAIL_REASONS_0};
  /// @param reason - 1 _MALLOC fail, other are user defined
  void major_fail(uint8_t reason); // __attribute__((noreturn));
  void hang_cpu(); //  __attribute__((noreturn));
  void debug_action(); // if we want to debug something in General lib in primitive way
  // we can redefine this function (it is defined in common_cpp as a __weak  empty function)
  // and call it from everywhere.
  void new_handler(); // NOTE! got to be installed on startup with std::set_new_handler(avp::new_handler);
#ifdef __cplusplus
}
#endif

IGNORE(-Wunused-value)

// ************************* ASSERT/ERROR macros **********************
#ifdef DEBUG
# define AVP_ERROR(code,format, ...) do{ \
    debug_printf("Error in file " __FILE__ ", line %d: " format, \
                 __LINE__, ##__VA_ARGS__); major_fail(code); }while(0)
#else // RELEASE
# define AVP_ERROR(code,format,...) do{ major_fail(code); }while(0)
// we gotta execute exp and args but do nothing else
#endif // DEBUG
/// AVP_ASSERT_WITH_EXPL = AVP_ASSERT_WITH_CODE with additional explanation
/// @code - numeric code, optional
/// @param ext_format - additional format string, followed by parameters
#define AVP_ASSERT_WITH_EXPL(exp,code,ext_format,...) do{ if(!(exp)) \
    { AVP_ERROR(code+0,"Expression \"" #exp "\" is false: " ext_format "!\n", \
                          ##__VA_ARGS__); }}while(0)
#define AVP_ASSERT_WITH_CODE(exp,code) AVP_ASSERT_WITH_EXPL(exp,code,"")
#define AVP_ASSERT(exp,...) AVP_ASSERT_WITH_CODE(exp,__VA_ARGS__ + 0)
#define ASSERT_BEING_0(exp) AVP_ASSERT((exp) == 0)


#endif /* ERROR_H_INCLUDED */
