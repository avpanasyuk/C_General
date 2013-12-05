/*
 * BitVar.h
 *
 * Created: 11/9/2013 10:42:57 AM
 *  Author: panasyuk
 * integer value with random number of bits. Useful in places with controlled wrap
 */


#ifndef BITVAR_H_
#define BITVAR_H_

#include "General.h"


template<uint8_t NumBits, typename tSize=uint8_t> class BitVar {
  tSize I:NumBits;
public:
  BitVar() {}
  BitVar(tSize Value): I(Value) {}
  BitVar(BitVar const &V): I(V.I)  {}
  BitVar(BitVar volatile const &V): I(V.I) {}
    
// important thing here is not to let it to convert to tSize too early, becuase it compromises bit limit
// That's why we make this conversion explicit
  explicit operator tSize() volatile const { return I; } 

  BitVar volatile &operator=(BitVar V)  volatile { I= V.I; return (*this); }
  BitVar volatile &operator+=(BitVar V)  volatile { I+= V.I; return (*this); }
  BitVar operator+(BitVar V)  volatile const { return BitVar(*this) += V; }
  BitVar volatile &operator-=(BitVar V) volatile { I-= V.I; return(*this); }
  BitVar operator-(BitVar V) volatile const { return BitVar(*this) -= V; }
  bool operator==( BitVar V) volatile { return I == V.I; }
}; // BitVar
#endif /* BITVAR_H_ */