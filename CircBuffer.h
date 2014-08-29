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

/** Circular Buffer of elements of class T. One reader and one writer may work in parallel. Reader is using
  * only BeingRead index, and writer only BeingWritten, so index are volatile ONLY when cross-used,
  * like in Clear()
  * NOTE: the size of CircBuffer is whateven fit into variable of tSize, probably 255 or 65535.
  *  to avoid conditional rollovers (when we compare index with an end of buffer all the time)
  * @tSize - type of size variable, has to be unsigned!
  * NOTE: When both BeingRead and BeingWritten refer the same block the buffer is empty, as this block is
  * not written yet, so there is nothing to read
  * NOTE: Writing and reading function DO NOT CHECK WHETHER THERE IS SPACE! Call LeftTo... function beforehand
  */
template <typename T, typename tSize=uint8_t> struct CircBufferFullType {
  // *********** General functions
  CircBufferFullType() { Clear(); }
  void Clear() volatile {  BeingWritten = BeingRead; }
  static constexpr tSize GetCapacity() { return tSize(-1); }; // how may elements fit

  //************* Writer functions, "BeingRead" is volatile here
  tSize LeftToWrite() const volatile { return BeingRead - 1 - BeingWritten; }
  T *GetSlotToWrite() { return &Buffer[BeingWritten]; }
  void FinishedWriting() { ++BeingWritten; }
  void Write(T d) { *GetSlotToWrite() = d; FinishedWriting(); }
  // It is risky function because BeingRead may change in between. We have to disable interrupts accross it
  T *ForceSlotToWrite() volatile {  if(!LeftToWrite()) ++BeingRead; return GetSlotToWrite(); }

  //************* Reader functions, "BeingWritten" is volatile here
  tSize LeftToRead() const volatile { return BeingWritten - BeingRead; }
  T const *GetSlotToRead() { return &Buffer[BeingRead]; }
  T const *GetContinousBlockToRead(tSize *Sz) volatile {
    if(Sz != nullptr) {
        long tempSz = (long)BeingWritten - BeingRead; // it is here, so we address BeingWritten only once
        *Sz = tempSz < 0?(unsigned long)GetCapacity()+1-BeingRead:tempSz;
    }
    return (T const *)&Buffer[BeingRead];
  } // GetContinousBlockToRead
  // if FinishedReading is used after GetContinousBlockToRead Sz should be specified
  void FinishedReading(tSize Sz = 1) { BeingRead += Sz; }
  T Read() { T temp = *GetSlotToRead(); FinishedReading(); return temp; }
protected:
  T Buffer[(unsigned long)GetCapacity()+1];
  tSize BeingRead, BeingWritten; //!< indexes of buffer currently being ....
}; // CircBufferFullType

/** Circular Buffer of size less then a full type
  * NOTE: the size of CircBuffer is powers of 2 to avoid conditional rollovers (when we compare
  * index with an end of buffer all the time
  * @tparam sizeLog2 -  Log2(Size)
  * @tSize - type of size variable
  * NOTE: we do not use inheritance because Buffer is wrong size,
  * and we do not want to make virtual functions
  * NOTE: see CircBufferFullType comments
  */
template <typename T, uint8_t sizeLog2, typename tSize=uint8_t> struct CircBuffer {
  // *********** General functions
  CircBuffer() { Clear(); }
  void Clear() volatile {  BeingWritten = BeingRead; }
  static constexpr tSize GetCapacity() { return (1UL << sizeLog2) - 1; };

  //************* Writer functions, "BeingRead" is volatile here
  tSize LeftToWrite() const volatile { return (BeingRead - 1 - BeingWritten) & Mask; }
  T *GetSlotToWrite() { return &Buffer[BeingWritten]; }
  void FinishedWriting() { BeingWritten = (BeingWritten + 1) & Mask; }
  void Write(T d) { *GetSlotToWrite() = d; FinishedWriting(); }
  // It is risky function because BeingRead may change in between. We have to disable interrupts across it
  T *ForceSlotToWrite() volatile {
    if(!LeftToWrite()) BeingRead = (BeingRead + 1) & Mask;
    return GetSlotToWrite();
  } // ForceSlotToWrite

  //************* Reader functions, "BeingWritten" is volatile here
   tSize LeftToRead() const volatile { return (BeingWritten - BeingRead) & Mask; }
  T const *GetSlotToRead() { return &Buffer[BeingRead]; }
  T const *GetContinousBlockToRead(tSize *Sz) volatile {
    if(Sz != nullptr) {
        long tempSz = (long)BeingWritten - BeingRead; // it is here, so we address BeingWritten only once
        *Sz = tempSz < 0?(unsigned long)GetCapacity()+1-BeingRead:tempSz;
    }
    return (T const *)&Buffer[BeingRead];
  } // GetContinousBlockToRead
  void FinishedReading(tSize Sz = 1) { BeingRead = (BeingRead + Sz) & Mask; }
  T Read() { T temp = *GetSlotToRead(); FinishedReading(); return temp; }
protected:
  T Buffer[(unsigned long)GetCapacity()+1];
  static constexpr tSize Mask = GetCapacity(); // marks used bits in index variables
  // we do not care what happens in upper bits
  tSize BeingRead, BeingWritten; //!< indexes of buffer currently being ....
}; // CircBuffer

// template<typename T, uint8_t sizeLog2, typename tSize=uint8_t>
// constexpr tSize CircBuffer<T,sizeLog2,tSize>::Mask;

#endif /* CIRCBUFFER_H_ */
