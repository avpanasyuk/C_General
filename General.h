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
  template<typename T> inline constexpr T CeilRatio(T const& num, T const& denom)
  { return (num + denom - 1)/denom; }
  template<typename T> inline constexpr T RoundRatio(T const& num, T const& denom)
  { return (num + num + denom)/denom/2; }
  template<typename Tin, typename Tout> inline constexpr Tout sqr(Tin const& a)
  { return Tout(a)*Tout(a); }

  // following functions use  function of type to do formatted output bool write(void *Ptr, uint16_t Size);
  // NOTE both functions do not write string-ending 0 !!!!!
  extern bool vprintf(bool (*pwrite)(const void *Ptr, size_t Size),char const *format, va_list ap);
  extern bool printf(bool (*pwrite)(const void *Ptr, size_t Size), char const *format, ...);
  static inline constexpr uint16_t Word(const uint8_t *Bytes) {
    return (uint16_t(Bytes[1]) << 8)+Bytes[0];
  } // Word
  template<class ElType, class SzType> ElType checkXOR(const ElType *p, SzType size) {
    ElType XORvalue = 0;
    while(size--) XORvalue ^= *(p++);
    return XORvalue;
  } // checksum
  template<class OutType, class ElType, class SzType>
  OutType sum(const ElType *p, SzType size) {
    OutType out = 0;
    while(size--) out += *(p++);
    return out;
  } // checksum
  // template<typename type> uint8_t log2(type x) { uint8_t out=0; while(x>>=1) out++; return out; }
  template<typename type> constexpr int8_t log2(type x) { return x?log2<type>(x>>1)+1:-1; }
  // 1<<CurValue/x ? x/(1<<(Curvalue-1)), (1<<(2*CurValue -1) ? x^2)
  template<typename type> constexpr uint8_t RoundLog2(type x, uint8_t CurValue=0) {
    return (1<<CurValue) > x?
           (1UL << (2*CurValue-1) < avp::sqr<type,uint32_t>(x)?CurValue:CurValue-1
           ):RoundLog2<type>(x,CurValue+1);
  }
  
  // 1<<CurValue/x ? x/(1<<(Curvalue-1)), (1<<(2*CurValue -1) ? x^2)
  // numer*2/denom ? denom/numer, numer^2*2	? denom^2
  constexpr int8_t RoundLog2Ratio(uint32_t numer, uint32_t denom, bool Sorted=false) {
    return Sorted?
           (numer > denom?
            (numer > 0x4000?
             RoundLog2Ratio(numer>>1,denom,true):RoundLog2Ratio(numer,denom<<1,true)
            )+1:(numer*numer*2 < denom*denom?-1:0)
           ):(numer > denom?RoundLog2Ratio(numer,denom,true):-RoundLog2Ratio(denom,numer,true));
  }
  constexpr int8_t CeilLog2Ratio(uint32_t numer, uint32_t denom) {
    return avp::log2((avp::CeilRatio(numer,denom) << 1) - 1); 
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

  // ***** BIT HANDLING FUNCTIONS
  template<typename type> inline constexpr type make_mask(uint8_t lowest_bit, uint8_t numbits) {
    return ((type(1) << numbits) - 1) << lowest_bit;
  }
  template<typename type> inline void set_high(type &var, uint8_t bitI) { var |= 1 << bitI; }
  template<typename type> inline void set_low(type &var, uint8_t bitI) { var &= ~(1 << bitI); }
  template<typename type> inline void setbit(type &var, uint8_t bitI, bool value) {
    value?set_high(var,bitI):set_low(var,bitI);
  } // setbit
  template<typename type> inline constexpr bool getbit(type const &var,uint8_t bitI) {
    return (var >> bitI) & 1;
  }
  template<typename type> inline void setbits(type &var, uint8_t lowest_bit, uint8_t numbits, type value) {
    var = (var & ~make_mask<type>(lowest_bit,numbits)) | (value << lowest_bit);
  }
}// avp

#define LOG10(x) ((x)>999?3:(x)>99?2:(x)>9?1:0)
// #define LOG2(x) ((x)>32767?15:(x)>16384?14:(x)>8191?13:(x)>4095?12:(x)>2047?11: \
// (x)>1023?10:(x)>511?9:(x)>255?8:(x)>127?7:(x)>63?6:(x)>31?5:(x)>15?4:(x)>7?3:(x)>3?2:(x)>1?1:0)
#define LOG2(x) avp::log2(x)

// following are operators which can be universaly derived from others
template<typename T> T &operator++(T &v) { return v += 1; }
template<typename T> T operator++(T volatile &v) { return v += 1; }
template<typename T> T operator++(T &v, int) { T old(v); v += 1; return old; }
template<typename T> bool operator!=(T const &v1, T const &v2) { return !(v1 == v2); }

// preprocessor tricks. __VA_ARGS__ is used so the last parameter may be empty
#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define __COMB(a,b,...) a##b##__VA_ARGS__
#define _COMB(a,b,...) __COMB(a,b,__VA_ARGS__)
#define __COMB2(a,...) a##__VA_ARGS__
#define _COMB2(a,...) __COMB2(a,__VA_ARGS__) // second parameter may be absent

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