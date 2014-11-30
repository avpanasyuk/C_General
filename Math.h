#ifndef MATH_H_INCLUDED
#define MATH_H_INCLUDED

/**
  * @file AVP_LIBS/General/Math.h
  * @author Alexander Panasyuk
  */

namespace avp {
  template<typename T> inline constexpr T max(T const& a, T const& b) { return a>b?a:b; }
  template<typename T> inline constexpr T min(T const& a, T const& b) { return a<b?a:b; }
  template<typename T> inline constexpr T Abs(T const& a) { return a<0?-a:a; }
  template<typename T> inline constexpr T CeilRatio(T const& num, T const& denom)
  { return (num + denom - 1)/denom; }
  template<typename T, typename T1> inline constexpr T RoundRatio(T const& num, T1 const& denom)
  { return (num + num + denom)/denom/2; }
  template<typename Tin, typename Tout = Tin> inline constexpr Tout sqr(Tin const& a)
  { return Tout(a)*Tout(a); }

    // template<typename type> uint8_t log2(type x) { uint8_t out=0; while(x>>=1) out++; return out; }
  template<typename type> constexpr int8_t log2(type x) { return x?log2<type>(x>>1)+1:-1; }
  // 1<<CurValue/x ? x/(1<<(Curvalue-1)), (1<<(2*CurValue -1) ? x^2)
  template<typename type> constexpr int8_t ceil_log2(type x) { return log2<type>(x-1)+1; }

  template<typename type> constexpr uint8_t RoundLog2(type x, uint8_t CurValue=0) {
    return (1<<CurValue) > x?
           (1UL << (2*CurValue-1) < avp::sqr<type,uint32_t>(x)?CurValue:CurValue-1
           ):RoundLog2<type>(x,CurValue+1);
  }

  // 1<<CurValue/x ? x/(1<<(Curvalue-1)), (1<<(2*CurValue -1) ? x^2)
  // numer*2/denom ? denom/numer, numer^2*2 ? denom^2
  constexpr int8_t RoundLog2Ratio(uint32_t numer, uint32_t denom, bool Sorted=false) {
    return Sorted?
           (numer > denom?
            (numer > 0x4000?
             RoundLog2Ratio(numer>>1,denom,true):RoundLog2Ratio(numer,denom<<1,true)
        )+1:(numer*numer*2 < denom*denom?-1:0)
           ):(numer > denom?RoundLog2Ratio(numer,denom,true):-RoundLog2Ratio(denom,numer,true));
  }
  constexpr int8_t CeilLog2Ratio(uint32_t numer, uint32_t denom) {
    return ceil_log2(CeilRatio(numer,denom));
  }

  template<typename out_type, typename in_type> out_type Sqrt(in_type y) {
    in_type x = 1, old_x, y_=y;
    while(y_>>=2) x <<= 1; // rough estimate
    do {
      old_x = x;
      x = (old_x+y/old_x)>>1;
    } while (x != old_x && x + 1 != old_x);
    return x;
  } //Sqrt
} // namespace avp

#endif /* MATH_H_INCLUDED */
