/*
 * Complex.h
 *
 * Created: 11/12/2013 1:29:20 PM
 *  Author: panasyuk
 */


#ifndef AVP_COMPLEX_H_
#define AVP_COMPLEX_H_

#include <math.h>

namespace avp {
  template<typename T = float>
  struct Complex {
    T Real; T Imag;

    Complex() {}
    Complex(T const &Real_, T const &Imag_ = 0): Real(Real_), Imag(Imag_) {}
    Complex(const Complex &a): Real(a.Real), Imag(a.Imag) {}

    template<typename T1>
    Complex(const Complex<T1> &a): Real(a.Real), Imag(a.Imag) {}

    const Complex &operator= (const Complex &a) {
      Real = a.Real;
      Imag = a.Imag;
      return *this;
    }

    const Complex &conj() { Imag = - Imag; return *this; }
    T abs_sqr() const { return Real*Real + Imag*Imag; }

    const Complex &operator+= (const Complex &a2) {
      Real += a2.Real;
      Imag += a2.Imag;
      return *this;
    }

    const Complex &operator-= (const Complex &a2) {
      Real -= a2.Real;
      Imag -= a2.Imag;
      return *this;
    }

    const Complex &operator*= (const Complex &a) {
      return *this = Complex(Real*a.Real - Imag*a.Imag, Real*a.Imag + Imag*a.Real);
    } // *=

    const Complex &operator/= (const Complex &a) { // we can not use binary /, or we get into infinite recursion
      return *this *= Complex(a.Real/a.abs_sqr(),-a.Imag/a.abs_sqr());
    } // /=

    static Complex conj(Complex a) {
      return a.conj();
    }  // conj

// FOllowing are binary operators as friends
#define BINARY_OP_FROM_SELF(op) \
inline friend Complex operator op (const Complex &x1, const Complex &x2) \
{ return Complex(x1) op##= x2; }

    BINARY_OP_FROM_SELF(-)
    BINARY_OP_FROM_SELF(+)
    BINARY_OP_FROM_SELF(*)
    BINARY_OP_FROM_SELF(/)
#undef BINARY_OP_FROM_SELF

    inline friend bool operator==(Complex const &v1, Complex const &v2) { return v1.Real == v2.Real && v1.Imag == v2.Imag; }
    inline friend bool operator!=(Complex const &v1, Complex const &v2) { return !(v1 == v2); }

    bool IsNormal() { return isnormal(Real) && isnormal(Imag); }
    // isnormal is defined as macro, so no overloading
  }; // Complex
} // namespace avp

#endif /* AVP_COMPLEX_H_ */
