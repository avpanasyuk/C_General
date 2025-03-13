/*!
 * @file ../C_General/General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */

#ifndef GENERAL_H_
#define GENERAL_H_

/// @cond
#include <stdint.h>
#include <stdarg.h>
#include <tuple>
#include "General_C.h"
/// @endcond

#ifdef PRINTF_WRAPPER
#undef PRINTF_WRAPPER
#endif

/**
* USAGE
static PRINTF_WRAPPER(return type, info_printf, vprintf)
 __attribute__((format (printf, 1, 2)))
*/
#define PRINTF_WRAPPER(return_type,func_name,vprintf_func) \
  /* __attribute__((format(printf, 1, 2))) */ return_type func_name(const char *fmt, ...) \
  { va_list ap; va_start(ap,fmt); \
    return_type Out =  vprintf_func(fmt,ap); va_end(ap); \
    return Out; }

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
  inline friend CLASS operator op (const CLASS &x1, const CLASS &x2) { return CLASS(x1) op##= x2; }

BINARY_OP_FROM_SELF(-)
BINARY_OP_FROM_SELF(+)
BINARY_OP_FROM_SELF(*)
BINARY_OP_FROM_SELF(/ )
BINARY_OP_FROM_SELF(&)
BINARY_OP_FROM_SELF(| )
#undef BINARY_OP_FROM_SELF

inline friend bool operator==(CLASS const &v1, CLASS const &v2) { return equal(v1, v2); }
inline friend bool operator!=(CLASS const &v1, CLASS const &v2) { return !(v1 == v2); }

#endif  // end of example

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak,noinline))
#endif /* __weak */
#endif /* __GNUC__ */

namespace avp {
  // to suppress unused-variable or unused-value
  // volatile auto x = (unused-value-expression);
  // avp::unused(x)
  template<typename T> void unused(T const &) { }
}
#ifndef NO_STL
/// @cond
#include <string>
#include <type_traits>
#include <limits>
/// @endcond

namespace avp {
  std::string string_vprintf(const char *format, va_list a) __attribute__((format(printf, 1, 0)));
  std::string string_printf(char const *format, ...) __attribute__((format(printf, 1, 2)));

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

  template<typename T1, typename T2>
  inline bool unsigned_is_smaller(const T1 &x, const T2 &y, T1 WrapValue = std::numeric_limits<T1>::max()) {
    static_assert(std::is_same<T1, T2>::value, "Types should be identical!");
    static_assert(std::is_unsigned<T1>::value, "Type should be unsigned!");
    return (x - y) > (WrapValue >> 1);
  } // unsigned_is_smaller

  /**
   *@brief restores value of a variable upon getting out of scope
   *
   * @tparam T - variable type
   */
  template<typename T>
  class RestoreOnReturn {
    const T SavedValue; T *p;
   public:
    RestoreOnReturn(T &Var) : SavedValue(Var), p(&Var) { }
    ~RestoreOnReturn() { *p = SavedValue; }
  }; // RestoreOnReturn

#define RESTORE_ON_RETURN(x) avp::RestoreOnReturn<decltype(x)> _##__LINE__(x);

  template<int Length>
  class Log {
    char Text[Length + 1];
    int L;
    const char * const Br;
    const int BrL;
   public:
    Log(const char *Break = "<br>") : Text { 0 }, L(0), Br(Break), BrL(strlen(Br)) { }

    const char *Get() const { return Text; }

    void Add(const char *s, bool NoBreak = false) {
      int N = strlen(s);
      int SpaceForBreak = NoBreak ? 0 : BrL;

      if(N + SpaceForBreak > Length) Add("New entry is too big!");
      else {
        int Shift = L + N + SpaceForBreak - Length;

        if(Shift > 0) { // overran ReservedSz, got to shift
          const char *pBr = strstr(Text + Shift, Br); // find next break after Shift

          if(pBr != nullptr) {
            pBr += BrL; // step over the last break, we do not need to copy it
            L = Text + L - pBr;
            for(char *p = Text; p <= Text + L; ++p, ++pBr) *p = *pBr;
          } else L = 0;
        }
        strcpy(Text + L, s); L += N;
        if(!NoBreak) { strcpy(Text + L, Br); L += BrL; }
      }
    } // Add

  }; // class Log
  /**
  *
  */
  template<typename T>
  inline void shift_array_left(T *To, std::size_t N, std::size_t By = 1) {
    while(N--) { *To = *(To + By); ++To; }
  } // shift_array_left

  template<typename T>
  auto MinMax(const T &v) {
    using C = typename T::value_type;
    C Min = std::numeric_limits<C>::max();
    C Max = std::numeric_limits<C>::min();

    for(const auto &d:v) {
      if(d < Min) Min = d;
      if(d > Max) Max = d;
    }
    return std::tuple<C, C>{Min,Max};
  } // MinMax

  template<typename C>
  auto MinMax(const C *v, size_t N) {
    C Min = std::numeric_limits<C>::max();
    C Max = std::numeric_limits<C>::min();

    for(size_t i=0; i<N; ++i) {
      if(v[i] < Min) Min = v[i];
      if(v[i] > Max) Max = v[i];
    }
    return std::tuple<C, C>{Min,Max};
  } // MinMax
} // avp
#endif
#endif /* GENERAL_H_ */
