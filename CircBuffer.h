/*
* CircBuffer.h
*
* Created: 7/27/2013 8:03:10 PM
*  Author: panasyuk
*/


#ifndef CIRCBUFFER_H_
#define CIRCBUFFER_H_

#include <stddef.h>
#include <stdint.h>
//the size of CircBuffer is powers of 2 to simplify rollovers

template <typename T, uint8_t sizePower2, typename tSize=uint8_t> class CircBuffer {
protected:
  T Data[1 << sizePower2];
  volatile struct _Pointer { // I am using structure with bitfields here to make pointers to wrap without any additional code
      tSize I:sizePower2;
  } BeingRead, BeingWritten;
	static constexpr tSize BitMask = (1LLU << sizePower2) - 1;
public:
  CircBuffer() { Emptify(); };
  T *ForceSlotToWrite() { // never returns NULL, if we ran down sending pointer move it (and discard data)
    if(++BeingWritten.I == BeingRead.I) ++BeingRead.I; // storing pointer ran down sending one, we discard a slot
    return &Data[BeingWritten.I];
  } // ForceSlotToStore
  T *GetSlotToWrite() {
    if(((BeingWritten.I+1) & BitMask) == BeingRead.I) return NULL; // storing pointer ran down sending one
    return &Data[++BeingWritten.I];
  } // GetSlotToStore
  const T *GetSlotToRead() { return (BeingRead.I == BeingWritten.I)?NULL:&Data[++BeingRead.I]; }
  void Emptify() { BeingRead.I = BeingWritten.I; }
  uint16_t GetSlotSize() { return sizeof(T); /* *(1 << sizePower2);*/ }
  tSize LeftToWrite() { return (BeingRead.I - BeingWritten.I - 1) & BitMask; }
  tSize LeftToRead() { return (BeingWritten.I - BeingRead.I) & BitMask; }
  void FinishedReading() {}; // implemented in CircBufferSafe
  void FinishedWriting() {};
}; // CircBuffer



#endif /* CIRCBUFFER_H_ */