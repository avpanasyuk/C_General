/*
 * General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */


#ifndef GENERAL_H_
#define GENERAL_H_

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define N_ELEMENTS(array) (sizeof(array)/sizeof(array[0]))

namespace avp {
  template<typename T> inline constexpr T max(T const& a, T const& b) { return a>b?a:b; }
  template<typename T> inline constexpr T min(T const& a, T const& b) { return a<b?a:b; }
  template<typename T> inline constexpr T Abs(T const& a) { return a<0?-a:a; }
  template<typename T> inline constexpr T round(T const& num, T const& denom) { return (num + num + denom)/denom/2; }
  template<typename Tin, typename Tout> inline constexpr Tout sqr(Tin const& a) { return Tout(a)*Tout(a); }

  // following functions use  function of type to do formatted output bool write(void *Ptr, uint16_t Size);
  // NOTE both functions do not write string-ending 0 !!!!!
  extern bool vprintf(bool (*pwrite)(const void *Ptr, size_t Size),char const *format, va_list ap);
  extern bool printf(bool (*pwrite)(const void *Ptr, size_t Size), char const *format, ...);
  static inline uint16_t Word(const uint8_t *Params) {
    return (uint16_t(Params[1]) << 8)+Params[0];
  } // Word
  template<class T=uint32_t> uint8_t checkXOR(uint8_t const *p, T size) {
    uint8_t XORvalue = 0;
    while(size--) XORvalue ^= *(p++);
    return XORvalue;
  } // checksum
}// avp

#define LOG10(x) ((x)>999?3:(x)>99?2:(x)>9?1:0)
#define LOG2(x) ((x)>32767?15:(x)>16384?14:(x)>8191?13:(x)>4095?12:(x)>2047?11: \
(x)>1023?10:(x)>511?9:(x)>255?8:(x)>127?7:(x)>63?6:(x)>31?5:(x)>15?4:(x)>7?3:(x)>3?2:(x)>1?1:0)


// following are operators which can be universaly derived from others
template<typename T> T &operator++(T &v) { return v += 1; }
template<typename T> T operator++(T &v, int) { T old(v); v += 1; return old; }
template<typename T> bool operator!=(T const &v1, T const &v2) { return !(v1 == v2); }

// preprocessor tricks
#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(x) DO_PRAGMA(message ("TODO - " #x))

// things to supress warning for a bit
#define IGNORE(x) _Pragma ("GCC diagnostic push") \
DO_PRAGMA(GCC diagnostic ignored #x)
#define STOP_IGNORING _Pragma ("GCC diagnostic pop")

namespace Fail {
  typedef void (*function)();
  extern void default_function();
} // Fail


#endif /* GENERAL_H_ */