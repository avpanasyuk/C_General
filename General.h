/*!
 * @file AVP_LIBS/General/General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */

#ifndef GENERAL_H_
#define GENERAL_H_

 /// @cond
#include <stdint.h>
#include <stdarg.h>
#include "General_C.h"
/// @endcond
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
  template<typename T1, typename T2> inline bool unsigned_is_smaller(const T1 &x, const T2 &y) {
    static_assert(std::is_same<T1, T2>::value, "Types should be identical!");
    static_assert(std::is_unsigned<T1>::value, "Type should be unsigned!");
    return y - x < std::numeric_limits<T1>::max() / 2;
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

        if(Shift > 0) { // overran Length, got to shift
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
} // avp
#endif
#endif /* GENERAL_H_ */
