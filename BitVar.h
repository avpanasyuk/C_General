/*
 * BitVar.h
 *
 * Created: 11/9/2013 10:42:57 AM
 *  Author: panasyuk
 */


#ifndef BITVAR_H_
#define BITVAR_H_

#include "General.h"

template<uint8_t NumBits, typename tSize=uint8_t> class BitVar {
  tSize I:NumBits;
public:
  BitVar() {}
  BitVar(tSize Value): I(Value) {}
  explicit operator tSize()  const { return I; } 
  BitVar &operator+=(BitVar V)  { I+= V.I; return (*this); }
  BitVar operator+(BitVar V)  const { return BitVar(*this) += V; }
  BitVar &operator-=(BitVar V)  { I-= V.I; return(*this); }
  BitVar operator-(BitVar V)  const { return BitVar(*this) -= V; }
  bool operator==( BitVar v)  { return I == v.I; }
}; // BitVar
#endif /* BITVAR_H_ */