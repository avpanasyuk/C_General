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
#include "BitVar.h"
// the size of CircBuffer is powers of 2 to simplify rollovers

// operation "read" moves pointer first and returns slot to read after.
// "write" returns slot to write and moves pointer on "FinishedWriting"
// this class seems to be reenterable and safe for interrupts without additional protection

template <typename T, uint8_t sizePower2, typename tSize=uint8_t> class CircBuffer {
  protected:
  T Buffer[1U << sizePower2];
  struct BitVar<sizePower2, tSize>  BeingRead, BeingWritten;
  public:
  CircBuffer() { Clear(); }

#define MEMBERS_CIRCBUFFER1(...) \
  void Clear() __VA_ARGS__ {  BeingWritten = BeingRead + 1; } \
  tSize LeftToWrite() const __VA_ARGS__ { return tSize(BeingRead - BeingWritten); } \
  tSize LeftToRead() const __VA_ARGS__ { return tSize(BeingWritten - BeingRead - 1); } \
  T __VA_ARGS__ *GetSlotToWrite() __VA_ARGS__ { return &Buffer[tSize(BeingWritten)]; } \
  void Write(T d) __VA_ARGS__ { *GetSlotToWrite() = d; FinishedWriting(); } \
  T const __VA_ARGS__ *GetSlotToRead() __VA_ARGS__ { return &Buffer[tSize(++BeingRead)]; } \
  T Read() __VA_ARGS__ { return *GetSlotToRead(); } \
  void FinishedWriting() __VA_ARGS__ { ++BeingWritten; } \
  T *ForceSlotToWrite() __VA_ARGS__ {  if(!LeftToWrite()) ++BeingRead; return GetSlotToWrite(); }

// this class often used with interrupts, so it is often volatile, so we need two sets of members - volatile and not
MEMBERS_CIRCBUFFER1(volatile)
MEMBERS_CIRCBUFFER1()

  constexpr tSize GetFullSize() const { return (1U << sizePower2) - 1; };
}; // CircBuffer

//! simplest form is 256 entries
template<typename T> class CircBuffer<T,8> {
protected:
  T Buffer[1U<<8];
  uint8_t ReadI, WrittenI;
public:
  CircBuffer() { Clear(); }

#define MEMBERS_CIRCBUFFER2(...) \
  uint8_t LeftToWrite() const __VA_ARGS__ { return ReadI - WrittenI; } \
  uint8_t LeftToRead() const __VA_ARGS__ { return WrittenI - ReadI - 1; } \
  void Write(T d) __VA_ARGS__ { Buffer[WrittenI++] = d; } \
  T Read() __VA_ARGS__ { return Buffer[++ReadI]; } \
  void ForceWrite(T d) __VA_ARGS__ { if(!LeftToWrite()) ++ReadI; Write(d); } \
  void Clear() __VA_ARGS__ { WrittenI = ReadI + 1; }

// this class often used with interrupts, so it is often volatile, so we need two sets of members - volatile and not
MEMBERS_CIRCBUFFER2(volatile)
MEMBERS_CIRCBUFFER2()

  constexpr uint8_t GetFullSize() const { return (1U<<8) - 1; }
}; //  CircBuffer

#endif /* CIRCBUFFER_H_ */
