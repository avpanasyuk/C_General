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

#ifndef AVP_ERROR_MSG_BUFFER_SZ
#define AVP_ERROR_MSG_BUFFER_SZ 255
#endif

#ifdef __cplusplus
extern "C"
#endif
char AVP_ErrorMsgBuffer[AVP_ERROR_MSG_BUFFER_SZ + 1];

/**
@note AVP_ERROR_STR is using global variable AVP_ErrorMsgBuffer, so it is not thread safe
or reenterable! Use std::string(AVP_ERROR_STR(...)) to make it better or better avp::string_[v]printf
@note "format" can be a literal string only !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
#if defined(_MSC_VER) && defined(_DEBUG) || defined(__GNUC__) && defined(DEBUG)
#define AVP_ERROR_STR(format,...) (snprintf(AVP_ErrorMsgBuffer, AVP_ERROR_MSG_BUFFER_SZ, \
  " in '%s', file '" __FILE__ "', line %u: " format, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__) < 0? \
  "Failed to snprintf error message in '" __FILE__ "'!":AVP_ErrorMsgBuffer)
#else
#define AVP_ERROR_STR(format,...) (snprintf(AVP_ErrorMsgBuffer, AVP_ERROR_MSG_BUFFER_SZ, \
  format, ##__VA_ARGS__) < 0? \
  "Failed to snprintf error message, format = " format " !":AVP_ErrorMsgBuffer)
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

#define DEBUG_OUT do{ debug_printf("%s: %d\n", __PRETTY_FUNCTION__, __LINE__); }while(0)

extern volatile uint8_t FailReason;
typedef void (*failfunc_type)(uint8_t reason); //  __attribute__((noreturn));

enum MAJOR_FAIL_REASONS_0 {MEMALLOC = 1,NUM_FAIL_REASONS_0};

void major_fail(uint8_t reason) __attribute__((noreturn));
// we can redefine this function (it is defined in common_cpp as a __weak  empty function)
// and call it from everywhere.
void new_handler(); //  __attribute__((noreturn)); // NOTE! got to be installed on startup with std::set_new_handler(avp::new_handler);
#ifdef __cplusplus
}
#endif
// #ifndef DEBUG_PRINTF
//  #define DEBUG_PRINTF debug_printf

// #ifndef RELEASE
#ifdef DEBUG
# define AVP_ERROR_PRETTY(format, ...) do{ \
    debug_printf("Error in %s in file " __FILE__ ", line %d: " format, \
                 __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); hang_cpu();\
  }while(0)

# define AVP_ERROR(format, ...) do{ \
    debug_printf("Error in file " __FILE__ ", line %d: " format, \
                 __LINE__, ##__VA_ARGS__); hang_cpu();\
  }while(0)

/// AVP_ASSERT_WITH_EXPL = AVP_ASSERT_WITH_CODE with additional explanation
/// @code - numeric code, optional
/// @param ext_format - additional format string, followed by parameters
#else // DEBUG
# define AVP_ERROR(format,...) do{ hang_cpu(); }while(0)
// # define AVP_ASSERT(exp,format,...) do{ void(exp); void(__VA_ARGS__ + 0); }while(0)
#endif // DEBUG

#define AVP_ASSERT(exp) do{ if(!(exp)) \
    { AVP_ERROR(#exp " is false!\n"); }}while(0)

#define AVP_ASSERT_WITH_EXPL(exp, format, ...) do{ if(!(exp)) \
    { AVP_ERROR(#exp " is false: " format "\n", ##__VA_ARGS__); }}while(0)

#define ASSERT_BEING_0(exp,...) AVP_ASSERT((exp) == 0, ##__VA_ARGS__)

#define AVP_ASSERT_RETURN_STR(exp) do{ if(!(exp)) \
    { return AVP_ERROR_STR(#exp " is false"); }}while(0)

// #endif
IGNORE_WARNING(-Wunused-value)
#endif // __GNUC__










