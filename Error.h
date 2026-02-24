/**
 * @file ../C_General/Error.h
 * @author Alexander Panasyuk
 * @note - to use MACROS you have to properly implement
 * extern "C" int debug_puts(const char *s)".
 * it is weakly linked in common_c.c as an output to stderr
 * @note AVP_ASSERT in Release built DOES NOT CHECK EXPRESSION!
 * you can define debug output printf style function name be defining
 * DEBUG_PRINTF before including this file
 */

#pragma once

/// @cond
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
/// @endcond
#include "General.h"
#include "Macros.h"

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#if defined(_MSC_VER) && defined(_DEBUG) || \
  defined(__GNUC__) && !defined(NDEBUG) ||  \
  defined(DEBUG_LEVEL) && DEBUG_LEVEL

#define AVP_DEBUG_PRINTF(format, ...)                                  \
  do { debug_printf(" in '%s', file '" __FILE__ "', line %u: " format, \
    __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); } while(0);

#else

#define AVP_DEBUG_PRINTF(format, ...) \
  do { debug_printf(format, ##__VA_ARGS__); } while(0)

#endif

#ifdef __GNUC__
#ifdef __cplusplus
  extern "C" {
#endif
  /// defined in General library as weak, sending stuff to stderr
  /// Each one calls previous ones, may be redefined at any level
  int debug_putchar(char c);
  int debug_puts(const char *s);
  int debug_puts_free(const char *s, free_func_t free_func);
  int debug_vprintf(const char *format, va_list a);
  int debug_printf(const char *format, ...);
  void hang_cpu();     //  __attribute__((noreturn));
  void debug_action(); // if we want to debug something in General lib in primitive way

  typedef void (*free_func_t)(void *);

#define DEBUG_PUT_PLACE \
  do { debug_printf("%s in " __FILE__ ", %d\n", __PRETTY_FUNCTION__, __LINE__); } while(0);

  void major_fail(uint8_t reason) __attribute__((noreturn));
  // we can redefine this function (it is defined in common_cpp as a __weak  empty function)
  // and call it from everywhere.
  void new_handler(); //  __attribute__((noreturn)); // NOTE! got to be installed on startup with std::set_new_handler(avp::new_handler);
#ifdef __cplusplus
}
#endif

#ifdef NDEBUG
#define AVP_ERROR_PUTS(s) \
  do { hang_cpu(); } while(0)
#define AVP_ERROR_PRINTF(format, ...) \
  do { hang_cpu(); } while(0)
#define AVP_ASSERT(exp) \
  do { (exp); } while(0)
#else
#define AVP_ERROR_PUTS(s) \
  do { hang_cpu(); } while(0)

#define AVP_ERROR_PRINTF(format, ...)               \
  do {                                       \
    AVP_DEBUG_PRINTF(format, ##__VA_ARGS__); \
    hang_cpu();                              \
  } while(0)

#define AVP_ASSERT(exp)                            \
  do {                                             \
    if(!(exp)) { AVP_ERROR_PRINTF(#exp " is false!\n"); } \
  } while(0)
#endif // NDEBUG

#define AVP_ERROR AVP_ERROR_PRINTF // backward compatibility

/// AVP_ASSERT_WITH_EXPL = AVP_ASSERT_WITH_CODE with additional explanation
/// @code - numeric code, optional
/// @param ext_format - additional format string, followed by parameters
#define AVP_ASSERT_WITH_EXPL(exp, format, ...)                               \
  do {                                                                       \
    if(!(exp)) { AVP_ERROR_PRINTF(#exp " is false: " format "\n", ##__VA_ARGS__); } \
  } while(0)

#define ASSERT_BEING_0(exp, ...) AVP_ASSERT((exp) == 0, ##__VA_ARGS__)

#define AVP_ASSERT_AND_RETURN_STR(exp)      \
  do {                                      \
    if(!(exp)) { return #exp " is false"; } \
  } while(0)

#define AVP_ASSERT_RETURNED_STR(func_call)                                  \
  do {                                                                      \
    const char *err = (func_call);                                          \
    if(err != nullptr) { AVP_ERROR_PRINTF(#func_call " failed with %s!\n", err); } \
  } while(0)

// #endif
// clang-format off
IGNORE_WARNING(-Wunused-value)
// clang-format on
#endif // __GNUC__
