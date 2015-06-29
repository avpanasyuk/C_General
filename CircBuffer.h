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

/** Circular Buffer of elements of class T. One reader and one writer may work in parallel. Reader is using
  * only BeingRead index, and writer only BeingWritten, so index can be screwed-up ONLY when cross-used,
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
  static_assert(sizeof(tSize)*8 >= sizeLog2,"CircBuffer:Size variable is too small to hold size!");

  // *********** General functions
  CircBuffer() { Clear(); }
  void Clear()  {  BeingWritten = BeingRead; LastReadSize = 0;}
  static constexpr tSize GetCapacity() { return (1UL << sizeLog2) - 1; };

  //************* Writer functions, "BeingRead" is  here
  tSize LeftToWrite() const  {
    return (BeingRead - 1 - BeingWritten) & Mask;
  }
  T *GetSlotToWrite() { return &Buffer[BeingWritten]; }
  void FinishedWriting() { BeingWritten = (BeingWritten + 1) & Mask; }
  void Write_(T const &d) {
    *GetSlotToWrite() = d;
    FinishedWriting();
  }
  bool Write(T const &d) {
    if(LeftToWrite() == 0) return false;
    Write_(d);
    return true;
  }

  // It is risky function because it modifies BeingRead index, so interferes with reading function. E.g
  // if read is in progress and this function is called from the interrupt (rr vise versa) things may get screwed up.
  // The function never fails
  T *ForceSlotToWrite()  {
    if(!LeftToWrite()) {
      if(LastReadSize == 0) LastReadSize = 1; // when no read is on progress we still have to move pointer
      FinishedReading();
    }
    return GetSlotToWrite();
  } // ForceSlotToWrite

  // It is marginally safer function because it moves BeingRead index only when no read is in progress. Interrupts may still screw things up
  // if racing condition occurs. Fails and returns NULL is reading is in progress
  T *SaferForceSlotToWrite()  {
    if(!LeftToWrite()) { // we will try to free some space
      if(LastReadSize == 0) {  // OK, no read is in progress
        LastReadSize = 1; // when no read is on progress we still have to move pointer
        FinishedReading();
      } else return nullptr; // will not free space if read is in progress
    }
    return GetSlotToWrite();
  } // ForceSlotToWrite




  //************* Reader functions, "BeingWritten" is  here
  tSize LeftToRead() const  { return (BeingWritten - BeingRead) & Mask; }
  //! @brief returns the same slot if called several times in a row. Only FinishReading moves pointer
  T const *GetSlotToRead() { LastReadSize = 1; return &Buffer[BeingRead]; }

  void FinishedReading() { BeingRead = (BeingRead + LastReadSize) & Mask; LastReadSize = 0; }

  T Read_() {
    T temp = *GetSlotToRead();
    FinishedReading();
    return temp;
  }

  bool Read(T* Dst) {
    if(LeftToRead() == 0) return false;
    *Dst = Read_();
    return true;
  }

  // ************* Continous block reading functions
  /** instead of a single entry marks for reading a continuous block
  *   @param[out] pSz - pointer to variable to return size in
  */
  T const *GetContinousBlockToRead(tSize *pSz) {
    if(BeingRead > BeingWritten) { // reading is wrapped, continuous blocks goes just to the end of the buffer
      LastReadSize = GetCapacity() + 1 - BeingRead;
    } else  LastReadSize = LeftToRead();

    *pSz = LastReadSize;

    return &Buffer[BeingRead];
  } // GetContinousBlockToRead
protected:
  T Buffer[size_t(GetCapacity())+1];
  static constexpr tSize Mask = GetCapacity(); //!< marks used bits in index variables
  // we do not care what happens in upper bits
  tSize BeingRead, BeingWritten; //!< indexes of buffer currently being ....
  tSize LastReadSize; //! 0 if no read is in progress, size of the last read otherwise
}; // CircBuffer

#endif /* CIRCBUFFER_H_ */
