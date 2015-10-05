#ifndef BITBANG_H_INCLUDED
#define BITBANG_H_INCLUDED

/**
  * @file AVP_LIBS/General/BitBang.h
  * @author Alexander Panasyuk
  */

#include <stdint.h>
#include <stdlib.h>
#include "Math.h"

namespace avp {
  // bit-banging functions
  template<typename type>
  inline constexpr uint8_t num_bits_in() { return avp::log2(uint32_t(type(-1))+1); }

  static inline constexpr uint16_t Word(const uint8_t *Bytes) {
    return (uint16_t(Bytes[1]) << 8)+Bytes[0];
  } // Word
  template<typename ElType, typename SzType> ElType checkXOR(const ElType *p, SzType size) {
    ElType XORvalue = 0;
    while(size--) XORvalue ^= *(p++);
    return XORvalue;
  } // checksum

  // ***** BIT HANDLING FUNCTIONS
  template<typename type>
  inline constexpr type make_mask(uint8_t numbits = num_bits_in<type>(), uint8_t lowest_bit = 0) {
    return ((type(1U) << numbits) - 1) << lowest_bit;
  }
  // SETTING BITS
  template<typename type> inline void set_high(type &var, uint8_t bitI) { var |= 1 << bitI; }
  template<typename type> inline void set_low(type &var, uint8_t bitI) { var &= ~(1 << bitI); }
  template<typename type> inline void toggle(type &var, uint8_t bitI) { var ^= 1 << bitI; }
  template<typename type> inline void setbit(type &var, uint8_t bitI, bool value) {
    value?set_high(var,bitI):set_low(var,bitI);
  } // setbit
  template<typename type, typename type1>
  inline void setbits(type &var, type value, type mask = make_mask<type>()) {
    var = (var & ~mask) | (value & mask);
  }
  template<typename type, typename type1 = type>
  inline void setbits(type &var, type1 value, uint8_t numbits = num_bits_in<type>(), uint8_t lowest_bit = 0) {
    setbits(var,value << lowest_bit,make_mask<type>(numbits,lowest_bit));
  }
  // GETTING BITS
  //! single bit
  template<typename type> inline constexpr bool getbit(type const &var,uint8_t bitI) {
    return (var >> bitI) & 1;
  }
  //! get value from a range of bits
  template<typename type> inline constexpr type bits(type x, uint8_t numbits, uint8_t lowest_bit = 0) {
    return (x >> lowest_bit) & make_mask<type>(numbits, lowest_bit);
  } // bits

  // converting a pattern in memory to type
  template<typename T, uint8_t NumChars = sizeof(T)>
  static inline constexpr T Bytes2type(const uint8_t bytes[NumChars]) {
    return T(bytes[0]) + (NumChars == 1?0:(Bytes2type<T,NumChars-1>(bytes+1) << 8));
  } // Chars2type

  template<typename T, uint8_t NumChars = sizeof(T)>
  static inline constexpr T Chars2type(const char str[NumChars]) {
    return Bytes2type<T,NumChars>((const uint8_t *)str);
  } // Chars2type
} // namespace avp



#endif /* BITBANG_H_INCLUDED */
