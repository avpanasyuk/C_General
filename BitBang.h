#ifndef BITBANG_H_INCLUDED
#define BITBANG_H_INCLUDED

/**
  @file AVP_LIBS/General/BitBang.h
  @author Alexander Panasyuk
  */

/// @cond
#include <stdint.h>
#include <stdlib.h>
/// @endcond

namespace avp {
  // byte-combining functions
//  template<typename T, uint8_t NumChars = sizeof(T)>
//  static inline constexpr T Bytes2type(const uint8_t bytes[NumChars]) {
//    return T(bytes[0]) + (NumChars == 1?0:(Bytes2type<T,NumChars-1>(bytes+1) << 8));
//  } // NumChars
//
//  template<typename T, uint8_t NumChars = sizeof(T)>
//  static inline constexpr T Chars2type(const char str[NumChars]) {
//    return Bytes2type<T,NumChars>((const uint8_t *)str);
//  } // Chars2type
//
//  static inline constexpr uint16_t Word(const uint8_t *Bytes) {
//    return Bytes2type<uint16_t>(Bytes);
//  } // Word

  /** reinterprets value pointed by the Ptr as "tOut", direct byte order
   */
  template<typename tOut, typename T1 = uint8_t>
  static inline tOut Ptr2type(const T1 *Ptr) {
    tOut Out;
    const uint8_t *pB = (const uint8_t *)Ptr;
    for(uint8_t *p = (uint8_t *)&Out; p < (uint8_t *)(&Out + 1); ++p) *p = *(pB++);
    return Out;
  } // Ptr2type

  template<typename T>
  static inline T Bytes2type(const uint8_t *Bytes) { return Ptr2type<T>(Bytes); }
  template<typename T>
  static inline T Chars2type(const char *Chars) { return Ptr2type<T,char>(Chars); }


  /** reinterprets value pointed by the Ptr as "tOut", Reverse byte order
   */
  template<typename tOut, typename T1 = uint8_t>
  static inline tOut Ptr2typeRev(const T1 *Ptr) {
    tOut Out;
    const uint8_t *pB = (const uint8_t *)Ptr;
    for(uint8_t *p = (uint8_t *)&Out + sizeof(tOut) - 1; p >= (uint8_t *)&Out; --p) *p = *(pB++);
    return Out;
  } // Ptr2typeRev

  // bit-banging functions
  template<typename ElType, typename SzType> ElType checkXOR(const ElType *p, SzType size) {
    ElType XORvalue = 0;
    while(size--) XORvalue ^= *(p++);
    return XORvalue;
  } // checksum

  // ***** BIT HANDLING FUNCTIONS
  template<typename type> inline constexpr type make_mask(uint8_t lowest_bit, uint8_t numbits) {
    return ((type(1) << numbits) - 1) << lowest_bit;
  }
  // SETTING BITS
  template<typename type> inline void set_high(type &var, uint8_t bitI) { var |= (1 << bitI); }
  template<typename type> inline void set_low(type &var, uint8_t bitI) { var &= ~(1 << bitI); }
  template<typename type> inline void toggle(type &var, uint8_t bitI) { var ^= (1 << bitI); }
  template<typename type> inline void setbit(type &var, uint8_t bitI, bool value) {
    value?set_high(var,bitI):set_low(var,bitI);
  } // setbit
  template<typename type, typename type1> inline void setbits(type &var, uint8_t lowest_bit, uint8_t numbits, type1 value) {
    var = (var & ~make_mask<type>(lowest_bit,numbits)) | (value << lowest_bit);
  }
  // GETTING BITS
  /**
  extracts a single bit from a number
  @param bitI 0-based bit index
  @param var - variable or value to get bit from
  @return bool, bit value
  */
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
  template<typename type> inline constexpr type getbits(type x, uint8_t numbits, uint8_t first_bit) {
    return (x >> first_bit) & ((type(1U) << numbits) - 1);
  } // bits

  // CRC16
  uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t start = 0);

  // byteswap
  inline uint16_t bswap_16(uint16_t w) { return (w << 8) | (w >> 8); }
} // namespace avp



#endif /* BITBANG_H_INCLUDED */
