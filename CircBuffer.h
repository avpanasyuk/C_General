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
#include "Error.h"

/** Circular Buffer of elements of class T. One reader and one writer may work in parallel. Reader is using
  * only BeingRead index, and writer only BeingWritten, so index are  ONLY when cross-used,
  * like in Clear()
  * NOTE: When both BeingRead and BeingWritten refer the same block the buffer is empty, as this block is
  * not written yet, so there is nothing to read
  * NOTE: Writing and reading function DO NOT CHECK WHETHER THERE IS SPACE! THEY NEVER RETURN
  * NULLPTR. Call LeftTo... function beforehand
  * NOTE: the size of CircBuffer is powers of 2 to avoid conditional rollovers (when we compare
  * index with an end of buffer all the time
  * @tparam tSize - type of size variable, if sizeLog2 is not specified determines the size of buffer, which
  * is maximum value which can fit into this type
  * @tparam sizeLog2 - Log2 of desired size in bytes. Sizes of this buffer are powers of two
   * and we do not want to make virtual functions
  */
template <typename T, typename tSize=uint8_t, uint8_t sizeLog2 = sizeof(tSize)*8>
struct CircBuffer {
  // *********** General functions
  CircBuffer() { Clear(); }
  void Clear()  {  BeingWritten = BeingRead; }
  static constexpr tSize GetCapacity() { return (1UL << sizeLog2) - 1; };

  //************* Writer functions, "BeingRead" is  here
  tSize LeftToWrite() const  {
    return (BeingRead - 1 - BeingWritten) & Mask;
  }
  T *GetSlotToWrite() { return &Buffer[BeingWritten]; }
  void FinishedWriting() { BeingWritten = (BeingWritten + 1) & Mask; }
  void Write(T const &d) {
    AVP_ASSERT(LeftToWrite());
    *GetSlotToWrite() = d;
    FinishedWriting();
  }
  // It is risky function because BeingRead may change in between. We have to disable interrupts across it
  T *ForceSlotToWrite()  {
    if(!LeftToWrite()) FinishedReading();
    return GetSlotToWrite();
  } // ForceSlotToWrite

  //************* Reader functions, "BeingWritten" is  here
  tSize LeftToRead() const  { return (BeingWritten - BeingRead) & Mask; }
  T const *GetSlotToRead() { return &Buffer[BeingRead]; }
  void FinishedReading(tSize Sz = 1) { BeingRead = (BeingRead + Sz) & Mask; }
  T Read() {
    AVP_ASSERT(LeftToRead());
    T temp = *GetSlotToRead();
    FinishedReading();
    return temp;
  }

  // ************* Continous block reading functions
  /** instead of a single entry marks for reading a continous block
  *   release block after reading with FinishedReading(*pSz)
  *   @param[out] pSz - pointer to variable to return size in
  */
  T const *GetContinousBlockToRead(tSize *pSz = &Idle) {
    if(BeingRead > BeingWritten) { // reading is wrapped, continous blocks goes just to the end of the buffer
      *pSz = GetCapacity() + 1 - BeingRead;
    } else  *pSz = LeftToRead();

    return GetSlotToRead();
  } // GetContinousBlockToRead
protected:
  T Buffer[size_t(GetCapacity())+1];
  static constexpr tSize Mask = GetCapacity(); //!< marks used bits in index variables
  // we do not care what happens in upper bits
  tSize BeingRead, BeingWritten; //!< indexes of buffer currently being ....
  static tSize Idle;
}; // CircBuffer

template <typename T, typename tSize, uint8_t sizeLog2>
tSize CircBuffer<T,tSize,sizeLog2>::Idle;

#endif /* CIRCBUFFER_H_ */
