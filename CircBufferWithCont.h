/*
* CircBufferWithCont.h
*
* Created: 7/27/2013 8:03:10 PM
*  Author: panasyuk
*
*  @brief Circular buffer with continuos read. Writes element by element, but can read a
*  continous sequence of the elements at once.
*/

#ifndef CIRCBUFFERWITHCONT_H_
#define CIRCBUFFERWITHCONT_H_

#include <stddef.h>
#include <stdint.h>
#include "CircBuffer.h"

/** Circular Buffer of elements of class T. One reader and one writer may work in parallel. Reader is using
  * only BeingRead index, and writer only BeingWritten, so index can be screwed-up ONLY when cross-used,
  * like in Clear()
  * NOTE: When both BeingRead and BeingWritten refer the same block the buffer is empty, as this block is
  * not written yet, so there is nothing to read
  * NOTE: Writing and reading function DO NOT CHECK WHETHER THERE IS SPACE! THEY NEVER RETURN
  * NULLPTR. Call LeftTo... function beforehand
  * NOTE: the size of CircBuffer is powers of 2 to avoid conditional rollovers (when we compare
  * index with an end of buffer all the time
  * @tparam BitsInCounter - size of counters in bits, defines size and maximum size which fits. for 8.16 or 32
  * the class is specially fast
  */
template <typename T, uint8_t BitsInCounter>
struct CircBufferWithCont: public CircBufferPWR2<T,size_t,BitsInCounter> {
  using Base = CircBufferPWR2<T,size_t,BitsInCounter>;

  CircBufferWithCont() { Clear(); }
  void Clear() {  Base::Clear(); LastReadSize = 0;}

  // It is marginally safer function because it moves BeingRead index only when no read is in progress. Interrupts may still screw things up
  // if racing condition occurs. Fails and returns NULL is reading is in progress
  T *SaferForceSlotToWrite()  {
    if(Base::LeftToWrite() == 0) { // we will try to free some space
      if(LastReadSize == 0) ++Base::BeingRead; // move read pointer if no read is in progress
      else return nullptr; // will fail but not overwrite data being read
    }
    // debug_printf("%hu/%hu ",BeingRead, BeingWritten);
    return Base::GetSlotToWrite();
  } // ForceSlotToWrite

  //! @brief returns the same slot if called several times in a row. Only FinishReading moves pointer
  T const *GetSlotToRead() { LastReadSize = 1; return Base::GetSlotToRead(); }

  void FinishedReading() { Base::BeingRead += LastReadSize; LastReadSize = 0; }

  // ************* Continous block reading functions
  /** instead of a single entry marks for reading a continuous block.
  * Use GetSizeToRead to determine size of the block to read
  */
  T const *GetContinousBlockToRead() {
    if(Base::BeingRead > Base::BeingWritten) { // writing wrapped, continuous blocks goes just to the end of the buffer
      LastReadSize = Base::GetCapacity() + 1 - Base::BeingRead; // BeingRead is at least 1 here
    } else  LastReadSize = Base::LeftToRead();
    return &Base::Buffer[Base::BeingRead];
  } // GetContinousBlockToRead

  size_t GetSizeToRead() const { return LastReadSize; }

protected:
  size_t LastReadSize; //! 0 if no read is in progress, size of the read being in progress otherwise
}; // CircBuffer

#endif /* CIRCBUFFERWITHCONT_H_ */
