/*
 * General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */


#ifndef GENERAL_H_
#define GENERAL_H_

#include <stdint.h>

#define N_ELEMENTS(array) (sizeof(array)/sizeof(array[0]))

namespace avp {
  template<typename T> inline constexpr T max(T const& a, T const& b) { return a>b?a:b; }
  template<typename T> inline constexpr T min(T const& a, T const& b) { return a<b?a:b; }
  template<typename T> inline constexpr T Abs(T const& a) { return a<0?-a:a; }
  template<typename T> inline constexpr T round(T const& num, T const& denom) { return (num + num + denom)/denom/2; }
  template<typename Tin, typename Tout> inline constexpr Tout sqr(Tin const& a) { return a*a; } 
} // avp

#define LOG10(x) ((x)>999?3:(x)>99?2:(x)>9?1:0)
#define LOG2(x) ((x)>32767?15:(x)>16384?14:(x)>8191?13:(x)>4095?12:(x)>2047?11: \
(x)>1023?10:(x)>511?9:(x)>255?8:(x)>127?7:(x)>63?6:(x)>31?5:(x)>15?4:(x)>7?3:(x)>3?2:(x)>1?1:0)

inline uint16_t Word(const uint8_t *Params) {
  return (uint16_t(Params[0]) << 8)+Params[1];
} // Word

// following are operators which can be universaly derived from others
template<typename T> T &operator++(T &v) { return v += 1; }
template<typename T> T operator++(T &v, int) { T old(v); v += 1; return old; }
  
#endif /* GENERAL_H_ */