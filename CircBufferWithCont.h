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

/**
 * this is CircBuffer class which allows to read entries by continues blocks. It still writes one by one
 * @note as CurrentBuffer??? classes this class is not virtual, so if you override one function you have to override
 * every function which relies on it, otherwise superclass versions will be called. VirtCircBuffer.h implements
 * CircBuffer class with virtual classes
 * @tparam CircBufferClass - may be any of CircBuffer??? classes
 */


template <class CircBufferClass>
struct CircBufferWithReadBlock: public CircBufferClass {
  CircBufferWithReadBlock() { Clear(); }
  void Clear() {  CircBufferClass::Clear(); LastReadSize = 0;}

  void ForceFinishedWriting()  {
    if(CircBufferClass::LeftToWrite() == 0) FinishedReading();
    CircBufferClass::FinishedWriting();
  } // SaferForceFinishedWriting

  // It is marginally safer function because it moves BeingRead index only when no read is in progress. Interrupts may still screw things up
  // if racing condition occurs. Fails and returns flase is reading is in progress
  bool SaferForceFinishedWriting()  {
    if(CircBufferClass::LeftToWrite() == 0) { // we will try to free some space
      if(LastReadSize == 0) FinishedReading(); // move read pointer if no read is in progress
      else return false; // will fail but not overwrite data being read
    }
    CircBufferClass::FinishedWriting();
    return true;
  } // SaferForceFinishedWriting

  //! @brief returns the same slot if called several times in a row. Only FinishReading moves pointer
  auto GetSlotToRead() { LastReadSize = 1; return CircBufferClass::GetSlotToRead(); }
  void FinishedReading() { CircBufferClass::BeingRead += LastReadSize; CircBufferClass::NormalizeReadCounter(); LastReadSize = 0; }

  // ************* Continous block reading functions
  /** instead of a single entry marks for reading a continuous block.
  * Use GetReadSize to determine size of the block to read
  */
  auto GetContinousBlockToRead() {
    // if true BeingWritten has wrapped
    LastReadSize = CircBufferClass::BeingRead > CircBufferClass::BeingWritten ?
        CircBufferClass::GetCapacity() + 1 - CircBufferClass::BeingRead :
        CircBufferClass::LeftToRead();
    return &CircBufferClass::Buffer[CircBufferClass::BeingRead];
  } // GetContinousBlockToRead

  size_t GetReadSize() const { return LastReadSize; }

protected:
  size_t LastReadSize; //! 0 if no read is in progress, size of the read being in progress otherwise
}; // CircBufferWithReadBlock

#endif /* CIRCBUFFERWITHCONT_H_ */
