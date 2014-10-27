/*!
 * @file AVP_LIBS/General/General.h
 *
 * Created: 7/28/2013 10:37:52 AM
 *  Author: panasyuk
 */


#ifndef GENERAL_H_
#define GENERAL_H_

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

namespace avp {
  template<typename T> inline constexpr T max(T const& a, T const& b) { return a>b?a:b; }
  template<typename T> inline constexpr T min(T const& a, T const& b) { return a<b?a:b; }
  template<typename T> inline constexpr T Abs(T const& a) { return a<0?-a:a; }
  template<typename T> inline constexpr T CeilRatio(T const& num, T const& denom)
  { return (num + denom - 1)/denom; }
  template<typename T, typename T1> inline constexpr T RoundRatio(T const& num, T1 const& denom)
  { return (num + num + denom)/denom/2; }
  template<typename Tin, typename Tout> inline constexpr Tout sqr(Tin const& a)
  { return Tout(a)*Tout(a); }

  // following functions use general "write" function of type bool write(const void *Ptr, uint16_t Size) to do formatted output;
  // NOTE both functions do not write string-ending 0 !!!!!
  // extern bool vprintf(bool (*pwrite)(volatile void *Ptr, size_t Size),char const *format, va_list ap);
  // extern bool printf(bool (*pwrite)(volatile void *Ptr, size_t Size), char const *format, ...);
// Note about following two functions - they should be used with BLOCKED pwrite which
  // returns only after data pointed by Ptr are used. Or it makes a copy.

  // following functions use general "write" function of type bool write(const void *Ptr, uint16_t Size) to do formatted output;
  // NOTE both functions do not write string-ending 0 !!!!!
  template<bool (*pwrite)(volatile void *Ptr, size_t Size)> bool vprintf(char const *format, va_list ap) {
    int Size = vsnprintf(NULL,0,format,ap);
    if(Size < 0) return false;
    char Buffer[Size+1]; // +1 to include ending zero byte
    vsprintf(Buffer,format,ap);
    return (*pwrite)((volatile void *)Buffer,Size); // we do not write ending 0 byte
  } // vprintf

  template<bool (*vprintf)(char const *format, va_list ap)> bool printf(char const *format, ...) {
    va_list ap;
    va_start(ap,format);
    bool Out =  vprintf(format,ap);
    va_end(ap);
    return Out;
  } // printf

  // bit-banging functions
  static inline constexpr uint16_t Word(const uint8_t *Bytes) {
    return (uint16_t(Bytes[1]) << 8)+Bytes[0];
  } // Word
  template<typename ElType, typename SzType> ElType checkXOR(const ElType *p, SzType size) {
    ElType XORvalue = 0;
    while(size--) XORvalue ^= *(p++);
    return XORvalue;
  } // checksum
  template<typename OutType, typename ElType>
  OutType sum(const ElType *p, size_t size = sizeof(ElType)) {
    OutType out = 0;
    while(size--) out += *(p++);
    return out;
  } // checksum
  // making it macro to avoid ugly template
// #define ROUND_RATIO(a,b) ((2*(a)+(b))/(b)/2)


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

  // ***** BIT HANDLING FUNCTIONS
  template<typename type> inline constexpr type make_mask(uint8_t lowest_bit, uint8_t numbits) {
    return ((type(1) << numbits) - 1) << lowest_bit;
  }
  // SETTING BITS
  template<typename type> inline void set_high(type &var, uint8_t bitI) { var |= 1 << bitI; }
  template<typename type> inline void set_low(type &var, uint8_t bitI) { var &= ~(1 << bitI); }
  template<typename type> inline void toggle(type &var, uint8_t bitI) { var ^= 1 << bitI; }
  template<typename type> inline void setbit(type &var, uint8_t bitI, bool value) {
    value?set_high(var,bitI):set_low(var,bitI);
  } // setbit
  template<typename type> inline void setbits(type &var, uint8_t lowest_bit, uint8_t numbits, type value) {
    var = (var & ~make_mask<type>(lowest_bit,numbits)) | (value << lowest_bit);
  }
  // GETTING BITS
  //! single bit
  template<typename type> inline constexpr bool getbit(type const &var,uint8_t bitI) {
    return (var >> bitI) & 1;
  }
  //! range of bits
  template<typename type> inline constexpr type bits(type x, uint8_t first_bit, uint8_t last_bit) {
    return (x >> first_bit) & ((type(1U) << (last_bit - first_bit + 1)) - 1);
  } // bits
  //! when last_bit is the last bit
  template<typename type> inline constexpr type bits(type x, uint8_t first_bit) {
    return x >> first_bit;
  } // bits
}// avp


// following are operators which can be universaly derived from others
template<typename T> inline T &operator++(T &v) { return v += 1; }
template<typename T> inline T operator++(T volatile &v) { return v += 1; }
template<typename T> inline T operator++(T &v, int) { T old(v); v += 1; return old; }
template<typename T> inline bool operator!=(T const &v1, T const &v2) { return !(v1 == v2); }

namespace Fail {
  typedef void (*function)();
  extern void default_function();
} // Fail

#if  defined ( __GNUC__ )
#ifndef __weak
#define __weak   __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif /* __packed */
#endif /* __GNUC__ */

#endif /* GENERAL_H_ */
