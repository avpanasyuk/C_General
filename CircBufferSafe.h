/*
* CircBufferSafe.h
*
* Created: 7/27/2013 8:03:10 PM
*  Author: panasyuk
* The difference from CircBuffer is that when we giving a slot we mark it dirty until "Finished..." routine is called
* to mark it clean. This way we can not read slot still being written and vice versa. 
*/


#ifndef CIRCBUFFERSAFE_H_
#define CIRCBUFFERSAFE_H_

#include <stddef.h>
#include <stdint.h>
//the size of CircBuffer is powers of 2 to simplify rollovers

template <typename T, uint8_t sizePower2, typename tSize=uint8_t> class CircBufferSafe {
protected:
  T Data[1 << sizePower2];
  volatile struct _Pointer { // I am using structure with bitfields here to make pointers to wrap without any additional code
    tSize I:sizePower2;
  } BeingRead, BeingWritten;
  volatile uint8_t NotYetWritten, NotYetRead; // 0 if the slot pointed to by Being????? is read/written, 1 if not yet
  static constexpr tSize BitMask = (1LLU << sizePower2) - 1;
public:
  CircBufferSafe() { Emptify(); };
  T *GetSlotToWrite() { // does not discard reading slots
    return (((BeingWritten.I+NotYetRead+1) & BitMask) == BeingRead.I)?NULL:
      (NotYetWritten = 1,&Data[++BeingWritten.I]);
  } // GetSlotToStore
  T *ForceSlotToWrite() { // if next slot is DoneRead, can shift reading pointer thus discarding slots to read
    // if next slot is still being read return NULL;
    if(((BeingWritten.I+NotYetRead+1) & BitMask) == BeingRead.I) { 
      if(!NotYetRead) ++BeingRead.I; else return NULL; }
    NotYetWritten = 1;
    return &Data[++BeingWritten.I];
  } // ForceSlotToStore
  void FinishedWriting() { NotYetWritten = 0; }
  void FinishedReading() { NotYetRead = 0; }
  const T *GetSlotToRead() {
    return (((BeingRead.I+NotYetWritten) & BitMask) == BeingWritten.I)?NULL:
      (NotYetRead=1,&Data[++BeingRead.I]);
  } // GetSlotToRead
  void Emptify() { BeingRead.I = BeingWritten.I; NotYetWritten = NotYetRead = 0;}
  uint16_t GetSlotSize() { return sizeof(T); /* *(1 << sizePower2);*/ }
  tSize LeftToWrite() { return (BeingRead.I - BeingWritten.I - NotYetRead - 1) & BitMask; }
  tSize LeftToRead() { return (BeingWritten.I - BeingRead.I - NotYetWritten) & BitMask; }
}; // CircBuffer

#endif /* CIRCBUFFERSAFE_H_ */