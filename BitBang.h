#ifndef BITBANG_H_INCLUDED
#define BITBANG_H_INCLUDED

/**
  * @file AVP_LIBS/General/BitBang.h
  * @author Alexander Panasyuk
  */

#include <stdint.h>
#include <stdlib.h>

namespace avp {
  // byte-combining functions
  template<typename T, uint8_t NumChars = sizeof(T)>
  static inline constexpr T Bytes2type(const uint8_t bytes[NumChars]) {
    return T(bytes[0]) + (NumChars == 1?0:(Bytes2type<T,NumChars-1>(bytes+1) << 8));
  } // NumChars

  template<typename T, uint8_t NumChars = sizeof(T)>
  static inline constexpr T Chars2type(const char str[NumChars]) {
    return Bytes2type<T,NumChars>((const uint8_t *)str);
  } // Chars2type

  static inline constexpr uint16_t Word(const uint8_t *Bytes) {
    return Bytes2type<uint16_t>(Bytes);
  } // Word

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
  template<typename type> inline void set_high(type &var, uint8_t bitI) { var |= 1 << bitI; }
  template<typename type> inline void set_low(type &var, uint8_t bitI) { var &= ~(1 << bitI); }
  template<typename type> inline void toggle(type &var, uint8_t bitI) { var ^= 1 << bitI; }
  template<typename type> inline void setbit(type &var, uint8_t bitI, bool value) {
    value?set_high(var,bitI):set_low(var,bitI);
  } // setbit
  template<typename type, typename type1> inline void setbits(type &var, uint8_t lowest_bit, uint8_t numbits, type1 value) {
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
  template<typename type> inline constexpr type getbits(type x, uint8_t numbits, uint8_t first_bit) {
    return (x >> first_bit) & ((type(1U) << numbits) - 1);
  } // bits
} // namespace avp



#endif /* BITBANG_H_INCLUDED */
