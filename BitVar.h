/*
 * BitVar.h
 *
 * Created: 11/9/2013 10:42:57 AM
 *  Author: panasyuk
 * integer value with random number of bits. Useful in places with controlled wrap
 */


#ifndef BITVAR_H_
#define BITVAR_H_

#include <stdint.h>

template<uint8_t NumBits, typename tSize=uint8_t> class BitVar {
  tSize I:NumBits;
public:
  BitVar() {}
  BitVar(tSize Value): I(Value) {}
    
// important thing here is not to let it to convert to tSize too early, because it compromises bit limit
// That's why we make this conversion explicit
#define MEMBERS_BITVAR(...) \
  BitVar(BitVar const __VA_ARGS__ &V): I(V.I)  {} \
  explicit operator tSize() const __VA_ARGS__ { return I; } \
  BitVar operator+(BitVar V)  const __VA_ARGS__ { return BitVar(*this) += V; } \
  BitVar operator-(BitVar V) const __VA_ARGS__ { return BitVar(*this) -= V; } \
  bool operator==( BitVar V) const __VA_ARGS__ { return I == V.I; }
    
// this class often used with interrupts, so it is often volatile, so we need two sets of members - volatile and not  
  MEMBERS_BITVAR(volatile);
  MEMBERS_BITVAR();    

  // we have to define separate functions for volatile case here because "standard" definition of these functions 
  // return reference to volatile object and this reference most of the time is ignored, so compiler is screaming about
  // volatile object is not being read. So we just return non-volatile copy
  BitVar operator=(BitVar V)  volatile { I = V.I; return (*this); } 
  BitVar operator+=(BitVar V)  volatile { I += V.I; return (*this); } 
  BitVar operator-=(BitVar V) volatile { I -= V.I; return(*this); } 

  // FOR NONJ-VOLATILE CASE FUNCTIONS ARE "STANDARD"
  BitVar &operator=(BitVar V) { I = V.I; return (*this); }
  BitVar &operator+=(BitVar V){ I+= V.I; return (*this); }
  BitVar &operator-=(BitVar V){ I-= V.I; return(*this); }
}; // BitVar
#endif /* BITVAR_H_ */