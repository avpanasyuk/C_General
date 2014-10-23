/*
 * Complex.h
 *
 * Created: 11/12/2013 1:29:20 PM
 *  Author: panasyuk
 */


#ifndef AVP_COMPLEX_H_
#define AVP_COMPLEX_H_

namespace avp {
  template<typename T> struct Complex {
      T Real; T Imag;

      Complex() {}
      Complex(T const &Real_, T const &Imag_ = 0): Real(Real_), Imag(Imag_) {}
      Complex(const Complex &a): Real(a.Real), Imag(a.Imag) {}

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

      Complex operator+ (Complex a) const {
        return a += *this;
      }

      const Complex &operator-= (const Complex &a2) {
        Real -= a2.Real;
        Imag -= a2.Imag;
        return *this;
      }

      Complex operator- (Complex a) const {
        return a -= *this;
      }

      const Complex &operator*= (const Complex &a2) {
        const T NewReal = Real*a2.Real - Imag*a2.Imag;
        Imag = Real*a2.Imag + Imag*a2.Real;
        Real = NewReal;
        return *this;
      } // *=

      Complex operator* (Complex a) const {
        return a *= *this;
      }

      const Complex &operator/= (const Complex &a) {
        T as = a.abs_sqr();
        *this *= conj(a);
        Real /= as;
        Imag /= as;
        return *this;
      } // /=

      Complex operator/ (const Complex &a) const {
        Complex Temp(*this);
        return Temp /= a;
      }

      static Complex conj(Complex a) {
        return a.conj();
      }
  };
} // namespace avp

#endif /* AVP_COMPLEX_H_ */
