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
      Complex() {}
      Complex(T Real_, T Imag_ = 0): Real(Real_), Imag(Imag_) {}
      Complex(const Complex &a2): Real(a2.Real), Imag(a2.Imag) {}

      const Complex &operator= (const Complex &a2) { Real = a2.Real; Imag = a2.Imag; return *this; }

      const Complex &conj() { Imag = - Imag; return *this; }
      T abs_sqr() const { return Real*Real + Imag*Imag; }

      const Complex &operator+= (const Complex &a2) { Real += a2.Real; Imag += a2.Imag; return *this;}
      Complex operator+ (const Complex &a2) const { Complex Temp(*this); return Temp += a2;}

      const Complex &operator-= (const Complex &a2) { Real -= a2.Real; Imag -= a2.Imag; return *this;}
      Complex operator- (const Complex &a2) const { Complex Temp(*this); return Temp -= a2;}

      const Complex &operator*= (const Complex &a2) {
        const T NewReal = Real*a2.Real - Imag*a2.Imag;
        Imag = Real*a2.Imag + Imag*a2.Real;
        Real = NewReal;
        return *this;
      } // *=
      Complex operator* (const Complex &a2) const { Complex Temp(*this); return Temp *= a2;}

      const Complex &operator/= (const Complex &a) {
        *this *= conj(a)/abs_sqr(a);
        return *this;
      } // /=
      Complex operator/ (const Complex &a2) const { Complex Temp(*this); return Temp /= a2;}

      static Complex conj(const Complex &a) { Complex Temp(a); return Temp.conj(); }
      static T abs_sqr(const Complex &a) { return a.abs_sqr(); }
    protected:
      T Real; T Imag;
  };
} // namespace avp

#endif /* AVP_COMPLEX_H_ */
