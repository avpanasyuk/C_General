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
  template<typename T> inline T max(T a, T b) { return a>b?a:b; }
  template<typename T> inline T min(T a, T b) { return a<b?a:b; }
  template<typename T> inline T Abs(T a) { return a<0?-a:a; }
  template<typename T> inline T round(T num, T denom) { return (num + denom/2)/denom; }
  template<typename Tin, typename Tout> inline Tout sqr(Tin a) { return a*a; } 
  template<typename T> struct Complex {
    T Real; T Imag;

    Complex() {}
    Complex(const T &Real_, const T& Imag_): Real(Real_), Imag(Imag_) {}
    Complex(const Complex<T> &a2): Real(a2.Real), Imag(a2.Imag) {}
    
    const Complex<T> &operator+= (const Complex<T> &a2) { Real += a2.Real; Imag += a2.Imag; return *this;}
    Complex<T> operator+ (const Complex<T> &a2) const { Complex Temp(*this); return Temp += a2;}
    const Complex<T> &operator-= (const Complex<T> &a2) { Real -= a2.Real; Imag -= a2.Imag; return *this;}
    Complex<T> operator- (const Complex<T> &a2) const { Complex Temp(*this); return Temp -= a2;}    
    const Complex<T> &operator= (const Complex<T> &a2) { Real = a2.Real; Imag = a2.Imag; return *this; }
    // template<typename T2> operator T2() const { return T2(Real, Imag);  }  
  };
} // avp

#define LOG10(x) ((x)>999?3:(x)>99?2:(x)>9?1:0)
#define LOG2(x) ((x)>32767?15:(x)>16384?14:(x)>8191?13:(x)>4095?12:(x)>2047?11: \
(x)>1023?10:(x)>511?9:(x)>255?8:(x)>127?7:(x)>63?6:(x)>31?5:(x)>15?4:(x)>7?3:(x)>3?2:(x)>1?1:0)

inline uint16_t Word(const uint8_t *Params) {
  return (uint16_t(Params[0]) << 8)+Params[1];
} // Word

template<typename T> T &operator++(T &v) { return v += 1; }
template<typename T> T operator++(T &v, int) { T old(v); v += 1; return old; }  
#endif /* GENERAL_H_ */