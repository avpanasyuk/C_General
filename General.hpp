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
#define PRINTF_WRAPPER(return_type, func_name, vprintf_func)                                       \
  /* __attribute__((format(printf, 1, 2))) */ return_type func_name(const char *fmt, ...) {        \
    va_list ap;                                                                                    \
    va_start(ap, fmt);                                                                             \
    return_type Out = vprintf_func(fmt, ap);                                                       \
    va_end(ap);                                                                                    \
    return Out;                                                                                    \
  }

#define PRINTF_WRAPPER_VOID(func_name, vprintf_func)                                       \
  /* __attribute__((format(printf, 1, 2))) */ void func_name(const char *fmt, ...) {               \
    va_list ap;                                                                                    \
    va_start(ap, fmt);                                                                             \
    vprintf_func(fmt, ap);                                                       \
    va_end(ap);                                                                                    \
  }

#if defined(__GNUC__)
#ifndef __weak
#define __weak __attribute__((weak, noinline))
#endif /* __weak */
#endif /* __GNUC__ */

namespace avp {
  // Template alternative to the PRINTF_WRAPPER / PRINTF_WRAPPER_VOID macros: turns
  // a vprintf-style function  R f(const char*, va_list)  (e.g. HTML_Log::vprintf)
  // into a printf-style variadic one. Handles a void R with no separate _VOID
  // form: `return f(...)` of a void expression makes the deduced `auto` void.
  // VaGuard runs va_end on every path. Bind it to a name with an alias, e.g.:
  //   constexpr auto log_error = avp::printf_wrapper<HTML_Log::vprintf>;
  template<auto VprintfFunc>
  auto printf_wrapper(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    struct VaGuard { va_list &ap; ~VaGuard() { va_end(ap); } } guard{ap};
    return VprintfFunc(fmt, ap);
  } // printf_wrapper

  // to suppress unused-variable or unused-value
  // volatile auto x = (unused-value-expression);
  // avp::unused(x)
  template<typename T>
  void unused(T const &) {}

  template<typename T, typename U>
  inline constexpr bool is_same_type_v = std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<U>>;

  template<typename T1, typename T2>
  inline bool unsigned_is_smaller(
    const T1 &x, const T2 &y, T1 WrapValue = std::numeric_limits<T1>::max()) {
    static_assert(is_same_type_v<T1, T2>, "Types should be identical!");
    static_assert(std::is_unsigned<T1>::value, "Type should be unsigned!");
    return (x - y) > (WrapValue >> 1);
  } // unsigned_is_smaller

#ifndef NO_STL
  std::string string_vprintf(const char *format, va_list a) __attribute__((format(printf, 1, 0)));
  std::string string_printf(char const *format, ...) __attribute__((format(printf, 1, 2)));
#endif

  /// this function is for comparison two relatively close unsigned values of the same type in case
  /// larger of them  wraps and we want to consider wrapped value to be still "larger" than the
  /// other one. Literal comparison does not work in this case.
  /// @note we use T1 and T2 instead of a single T to detect cases when parameter types are
  /// different
  /// @return true if y > x even if y is wrapped
  template<typename T1, typename T2>
  inline bool unsigned_is_smaller_or_equal(
    const T1 &x, const T2 &y, T1 WrapValue = std::numeric_limits<T1>::max()) {
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
    explicit RestoreOnReturn(T &Var) : SavedValue(Var), p(&Var) {}
    ~RestoreOnReturn() { *p = SavedValue; }
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
  uint16_t Crc16(
    const uint8_t *pcBlock, long long len, uint16_t crc = 0xFFFF, uint16_t poly = CRC16_CCITT_POLY);

  /// IEEE 802.3 (Ethernet) reflected polynomial. Pair with default crc init
  /// 0xFFFFFFFF; the function returns the raw accumulator (no final XOR).
  constexpr uint32_t CRC32_IEEE_POLY = 0xEDB88320;
  uint32_t Crc32(const uint8_t *pcBlock, long long len, uint32_t crc = 0xFFFFFFFFu,
    uint32_t poly = CRC32_IEEE_POLY);

  template<typename T>
  class ReleaseWhenOutOfScope {
    const T p;
    void (*ReleaseFunc)(T);

  public:
    ReleaseWhenOutOfScope(T p_, void (*ReleaseFunc_)(T)) : p(p_), ReleaseFunc(ReleaseFunc_) {}

    ~ReleaseWhenOutOfScope() { ReleaseFunc(p); }

    operator T() { return p; }
  }; // ReleaseWhenOutOfScope

  class CallWhenOutOfScope {
    const std::function<void()> fun;

  public:
    CallWhenOutOfScope(std::function<void()> fun_) : fun(fun_) {}
    ~CallWhenOutOfScope() { fun(); }
  }; // CallWhenOutOfScope

// some libraries use std::cout and std::cerr to report errors, lets have a way to redirect them is
// necessary
#if !defined(NO_STL) && defined(REDIRECT_COUT)

/// @cond
#include <istream>
#include <ostream>
#include <streambuf>
#include <iostream>
  /// @endcond

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

  template<typename T>
  /**
   * @brief 
   * 
   * @param func - should copy the string and not just store the pointer.
   * @param format 
   * @param ap 
   * @return T 
   */
  T svprintf_puts(T (*func)(const char *), const char *format, va_list ap) {
    va_list ap_;
    va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
    int Size = vsnprintf(NULL, 0, format, ap_);
    va_end(ap_);
    if(Size < 0) return func("svprintf_alloc: format is wrong!");

    char out[Size + 1];
    vsnprintf(out, Size + 1, format, ap);

    return func(out); // we do not write ending 0 byte
  } // svprintf_alloc
} // namespace avp
