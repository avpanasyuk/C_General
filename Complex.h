/*
 * Complex.h
 *
 * Created: 11/12/2013 1:29:20 PM
 *  Author: panasyuk
 */ 


#ifndef COMPLEX_H_
#define COMPLEX_H_

namespace avp {
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

#endif /* COMPLEX_H_ */