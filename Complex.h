/*
 * Complex.h
 *
 * Created: 11/12/2013 1:29:20 PM
 *  Author: panasyuk
 */


#ifndef AVP_COMPLEX_H_
#define AVP_COMPLEX_H_

namespace avp {
  template<typename T = float> struct Complex {
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

    const Complex &operator/= (const Complex &a) {
      return *this *= conj(a)/a.abs_sqr();
    } // /=

    static Complex conj(Complex a) {
      return a.conj();
    }  // conj
  }; // Complex

  template <typename T>
  inline Complex<T> operator- (Complex<T> a1, const Complex<T> &a2)  {
    return a1 -= a2;
  }

  template <typename T>
  inline Complex<T> operator+ (Complex<T> a1, const Complex<T> &a2)  {
    return a1 += a2;
  }

  template <typename T>
  inline Complex<T> operator* (Complex<T> a1, const Complex<T> &a2)  {
    return a1 *= a2;
  }

  template <typename T, typename T1>
  inline Complex<T> operator* (Complex<T> a1, const T1 &a2)  {
    a1.Real *= a2;
    a1.Imag *= a2;
    return a1;
  }

  template <typename T, typename T1>
  inline Complex<T> operator* (const T1 &a1, Complex<T> a2)  {
    a2.Real *= a1;
    a2.Imag *= a1;
    return a2;
  }

  template <typename T, typename T1>
  inline Complex<T> operator/ (Complex<T> a1, const T1 &a2) {
    a1.Real /= a2;
    a1.Imag /= a2;
    return a1;
  }

  template <typename T>
  inline Complex<T> operator/ (Complex<T> a1, const Complex<T> &a2) {
    return a1 /= a2;
  }
} // namespace avp

#endif /* AVP_COMPLEX_H_ */
