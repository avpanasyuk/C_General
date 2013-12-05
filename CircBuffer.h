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
  T Data[1U << sizePower2];
  volatile struct BitVar<sizePower2, tSize>  BeingRead, BeingWritten;
  public:
  CircBuffer() { Clear(); }

  void Clear() {  BeingWritten = BeingRead + 1; }
  tSize LeftToWrite() { return tSize(BeingRead - BeingWritten); }
  tSize LeftToRead() { return tSize(BeingWritten - BeingRead - 1); }
  T *GetSlotToWrite() { return &Data[tSize(BeingWritten)]; }
  T const *GetSlotToRead() { return &Data[tSize(++BeingRead)]; }
  void FinishedWriting() { ++BeingWritten; }
  T *ForceSlotToWrite() {  if(!LeftToWrite()) ++BeingRead; return GetSlotToWrite(); }
}; // CircBuffer

template <class T> class SimpleCircBuffer { // I think none of the operations have to be atomic. 
  // the algorithm is modified a bit to make it happen.
protected:
  T Buffer[1<<8];
  uint8_t ReadI, WrittenI;
public:
  SimpleCircBuffer() { Clear(); }
  uint8_t LeftToWrite() { return ReadI - WrittenI; }
  uint8_t LeftToRead() { return WrittenI - ReadI - 1; }
  void Write(T d) { Buffer[WrittenI++] = d; } 
  T Read() { return Buffer[++ReadI]; } 
  void ForceWrite(T d) { if(!LeftToWrite()) ++ReadI; Write(d); } 
  void Clear() { WrittenI = ReadI + 1; }
}; //  SimpleCircBuffer

#endif /* CIRCBUFFER_H_ */
