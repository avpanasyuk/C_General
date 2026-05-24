/*!
 * @file ../C_General/General.hpp
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */

#pragma once

/// @cond
#include <type_traits>
#include <limits>
#include <functional>
#include <stdint.h>
#include <stdarg.h>
#ifndef NO_STL
#include <string>
#endif
/// @endcond

#include "General.h"
#include "Error.h"

#ifdef PRINTF_WRAPPER
#undef PRINTF_WRAPPER
#endif

/**
 * USAGE
 static PRINTF_WRAPPER(return type, info_printf, vprintf)
 __attribute__((format (printf, 1, 2)))
 */
#define PRINTF_WRAPPER(return_type, func_name, vprintf_func)                                \
  /* __attribute__((format(printf, 1, 2))) */ return_type func_name(const char *fmt, ...) { \
    va_list ap;                                                                             \
    va_start(ap, fmt);                                                                      \
    return_type Out = vprintf_func(fmt, ap);                                                \
    va_end(ap);                                                                             \
    return Out;                                                                             \
  }

#if 0 // cause reallu weird errors in c+11 and I think already built-in
 // following are operators which can be universaly derived from others
 template<typename T> inline T operator++(T &v) { return v += 1; }
 template<typename T> inline T operator++(T &v, int) { T old(v); v += 1; return old; }
 template<typename T> inline T operator--(T &v) { return v -= 1; }
 template<typename T> inline T operator--(T &v, int) { T old(v); v -= 1; return old; }
 template<typename T> inline T operator-(const T &x) { return 0 - x; } // unitary minus
#endif

#if 0
 // DO NOT REDEFINE BUILT-IN OPERATORS!!!!! INFINITE RECURSION HELL
 // DO NOT DEFINE BINARTY OPERATORS FROM UNITARY USING TEMPLATE FUNCTIONS. COMPLILER DOES NOT
 // TRY USING IMPLICIT CONVERSION TO FIT PARAMETER TYPES
 // USE TEMPLATE CLASS FRIEND FUNCTIONS - THEY ARE SYMMETRIC AND COMPILER TRIES
 // IMPLICIT CONVERSIONS
 // LIKE THIS: for template<typename type> class T:
 // beginning of examble
#define CLASS xxxxx
#define BINARY_OP_FROM_SELF(op) \
  inline friend CLASS operator op(const CLASS &x1, const CLASS &x2) { return CLASS(x1) op## = x2; }

 BINARY_OP_FROM_SELF(-)
 BINARY_OP_FROM_SELF(+)
 BINARY_OP_FROM_SELF(*)
 BINARY_OP_FROM_SELF(/ )
 BINARY_OP_FROM_SELF(&)
 BINARY_OP_FROM_SELF(| )
#undef BINARY_OP_FROM_SELF

 inline friend bool operator==(CLASS const &v1, CLASS const &v2) { return equal(v1, v2); }
 inline friend bool operator!=(CLASS const &v1, CLASS const &v2) { return !(v1 == v2); }

#endif // end of example

#if defined(__GNUC__)
#ifndef __weak
#define __weak __attribute__((weak, noinline))
#endif /* __weak */
#endif /* __GNUC__ */

namespace avp {
  // to suppress unused-variable or unused-value
  // volatile auto x = (unused-value-expression);
  // avp::unused(x)
  template<typename T>
  void unused(T const &) {}

  template<typename T, typename U>
  inline constexpr bool is_same_type_v = std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>>;

  template<typename T1, typename T2>
  inline bool unsigned_is_smaller(const T1 &x, const T2 &y, T1 WrapValue = std::numeric_limits<T1>::max()) {
    static_assert(is_same_type_v<T1, T2>, "Types should be identical!");
    static_assert(std::is_unsigned<T1>::value, "Type should be unsigned!");
    return (x - y) > (WrapValue >> 1);
  } // unsigned_is_smaller

#ifndef NO_STL
  std::string string_vprintf(const char *format, va_list a) __attribute__((format(printf, 1, 0)));
  std::string string_printf(char const *format, ...) __attribute__((format(printf, 1, 2)));
#endif

  /// this function is for comparison two relatively close unsigned values of the same type in case larger of them  wraps
  /// and we want to consider wrapped value to be still "larger" than the other one. Literal comparison does not work
  /// in this case.
  /// @note we use T1 and T2 instead of a single T to detect cases when parameter types are different
  /// @return true if y > x even if y is wrapped
  template<typename T1, typename T2>
  inline bool unsigned_is_smaller_or_equal(const T1 &x, const T2 &y, T1 WrapValue = std::numeric_limits<T1>::max()) {
    static_assert(std::is_same<T1, T2>::value, "Types should be identical!");
    static_assert(std::is_unsigned<T1>::value, "Type should be unsigned!");
    return (y - x) < (WrapValue >> 1);
  } // unsigned_is_smaller

  /**
   *@brief restores value of a variable upon getting out of scope
   *
   * @tparam T - variable type
   */
  template<typename T>
  class RestoreOnReturn {
    const T SavedValue;
    T *p;

  public:
    explicit RestoreOnReturn(T &Var) : SavedValue(Var), p(&Var) {
    }
    ~RestoreOnReturn() {
      *p = SavedValue;
    }
  };
  // RestoreOnReturn

#define RESTORE_ON_RETURN(x) avp::RestoreOnReturn<decltype(x)> _##__LINE__(x);

  /**
   *
   */
  template<typename T>
  inline void shift_array_left(T *To, std::size_t N, std::size_t By = 1) {
    while(N--) {
      *To = *(To + By);
      ++To;
    }
  } // shift_array_left

  constexpr uint16_t CRC16_CCITT_POLY = 0x1021;
  uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t crc = 0xFFFF, uint16_t poly = CRC16_CCITT_POLY);

  template<typename T>
  class ReleaseWhenOutOfScope {
    const T p;
    void (*ReleaseFunc)(T);

  public:
    ReleaseWhenOutOfScope(T p_, void (*ReleaseFunc_)(T)) : p(p_), ReleaseFunc(ReleaseFunc_) {
    }

    ~ReleaseWhenOutOfScope() {
      ReleaseFunc(p);
    }

    operator T() {
      return p;
    }
  }; // ReleaseWhenOutOfScope

  class CallWhenOutOfScope {
    const std::function<void()> fun;

  public:
    CallWhenOutOfScope(std::function<void()> fun_) : fun(fun_) {}
    ~CallWhenOutOfScope() { fun(); }
  }; // CallWhenOutOfScope

// some libraries use std::cout and std::cerr to report errors, lets have a way to redirect them is necessary
#if !defined(NO_STL) && defined(REDIRECT_COUT)

#include <istream>
#include <ostream>
#include <streambuf>
#include <iostream>

  class DebugStreamBuf : public std::streambuf {
  public:
    DebugStreamBuf() {
      // Initialize the put area (optional, but good practice)
      setp(buffer_, buffer_ + sizeof(buffer_));
      std::cout.rdbuf(this);
      std::cerr.rdbuf(this);
    } // constructor

  protected:
    // Called when a character needs to be written
    int overflow(int c) override {
      if(c != EOF) {
        // Write the character to your byte-writing function
        debug_putchar(static_cast<uint8_t>(c));
      }
      return c; // Return the character written (or EOF on error)
    }

    // Called when the stream is flushed (e.g., std::endl or std::flush)
    int sync() override {
      // Optional: Add flushing logic if your destination requires it
      // For example, flush a serial buffer or ensure data is sent
      return 0; // Return 0 on success, -1 on failure
    }

  private:
    // Optional: Small internal buffer to reduce calls to write_byte
    char buffer_[256]; // Adjust size based on your needs
  };
#endif

} // namespace avp
