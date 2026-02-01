/*
* CircBufferWithCont.h
*
* Created: 7/27/2013 8:03:10 PM
*  Author: panasyuk
*
*  @brief Circular buffer with continuos read. Writes element by element, but can read a
*  continous sequence of the elements at once.
*/

#pragma once
#include "Macros.h"

#ifndef AVP_RAM_ATTR
#define AVP_RAM_ATTR // set to IRAM_ATTR for ESP
#else
#if defined(ESP32) || defined(ESP8266)
#include <esp_attr.h>
#endif 
#endif

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
  * @tparam sizeLog2 - Log2 of desired size in bytes. Capacity of this buffer is 2^sizeLog2 - 1
  */
template <typename T, uint8_t sizeLog2, typename tSize=unsigned int>
struct CircBufferWithCont {
  static_assert(sizeof(tSize)*8 >= sizeLog2,"CircBuffer:Size variable is too small to hold size!");

  // *********** General functions
  CircBufferWithCont() { BeingWritten = 0; Clear(); }
  void AVP_RAM_ATTR Clear()  {  BeingRead = BeingWritten; LastBlockSize = 0;}
  static constexpr size_t GetCapacity() { return (1UL << sizeLog2) - 1; };

  //************* Writer functions *************************************************
  tSize AVP_RAM_ATTR LeftToWrite() const  { return (BeingRead - 1 - BeingWritten) & Mask; }

  T * AVP_RAM_ATTR GetSlotToWrite() { return &Buffer[BeingWritten]; }

  void AVP_RAM_ATTR FinishedWriting() { BeingWritten = (BeingWritten + 1) & Mask; }

  void AVP_RAM_ATTR Write_(T const &d) { *GetSlotToWrite() = d; FinishedWriting(); }

  bool AVP_RAM_ATTR Write(T const &d) {
    if(LeftToWrite() == 0) return false;
    else { Write_(d); return true; }
  } // safe Write

  // It is risky function because if there is no place to write it modifies BeingRead index, so interferes with reading function. E.g
  // if read is in progress and this function is called from the interrupt (or vise versa) things may get screwed up.
  // The function never fails
  T * AVP_RAM_ATTR ForceSlotToWrite()  {
    if(LeftToWrite() == 0) { // we will free some space
      if(LastBlockSize == 0) LastBlockSize = 1; // when no read is on progress we still have to move pointer
      FinishedReading();
    }
    return GetSlotToWrite();
  } // ForceSlotToWrite

  // It is marginally safer function because it moves BeingRead index only when no read is in progress. Interrupts may still screw things up
  // if racing condition occurs. Fails and returns NULL is reading is in progress
  T * AVP_RAM_ATTR SaferForceSlotToWrite()  {
    if(LeftToWrite() == 0) { // we will try to free some space
      if(LastBlockSize == 0) BeingRead = (BeingRead + 1) & Mask; // move read pointer if no read is in progress
      else return nullptr; // will fail but not overwrite data being read
    }
    // debug_printf("%hu/%hu ",BeingRead, BeingWritten);
    return GetSlotToWrite();
  } // ForceSlotToWrite


  //************* Reader functions ***************************************
  tSize AVP_RAM_ATTR LeftToRead() const  { return (BeingWritten - BeingRead) & Mask; }

  //! @brief returns the same slot if called several times in a row. Only FinishReading moves pointer
  T const * AVP_RAM_ATTR GetSlotToRead() { LastBlockSize = 1; return &Buffer[BeingRead]; }

  void AVP_RAM_ATTR FinishedReading() { 
    BeingRead = (BeingRead + LastBlockSize) & Mask; 
    LastBlockSize = 0; 
  }

  T AVP_RAM_ATTR Read_() { T temp = *GetSlotToRead(); FinishedReading(); return temp; }

  bool AVP_RAM_ATTR Read(T* Dst) {
    if(LeftToRead() == 0) return false;
    else { *Dst = Read_(); return true; }
  } // safer Read

  // ************* Continous block reading functions
  /** instead of a single entry marks for reading a continuous block.
  * Use GetSizeToRead to determine size of the block to read
  * BeingWritten can change behind our back
  */
  T const * AVP_RAM_ATTR GetContinousBlockToRead() {
    auto FrozenBeingWritten = BeingWritten; // BeingWritten can change behind our back
    // which is OK, but we need to be consistent here
    if(BeingRead > FrozenBeingWritten) { // writing wrapped, continuous blocks goes just to the end of the buffer
      LastBlockSize = GetCapacity() + 1 - BeingRead; // BeingRead is at least 1 here
    } else  LastBlockSize = (FrozenBeingWritten - BeingRead) & Mask;
    return &Buffer[BeingRead];
  } // GetContinousBlockToRead

  tSize AVP_RAM_ATTR GetSizeToRead() { return LastBlockSize; }

  // ************* service functions
  // for debugging purposes
  void AVP_RAM_ATTR GetInternals(tSize *WriteI, tSize *ReadI, tSize *ReadSize) {
    *WriteI = BeingWritten; *ReadI = BeingRead; *ReadSize = LastBlockSize;
  } // GetInternals
protected:
  T Buffer[size_t(GetCapacity())+1]; //!< buffer size is 2^sizeLog2
  static constexpr tSize Mask = GetCapacity(); //!< marks used bits in index variables
  // we do not care what happens in upper bits
  volatile tSize BeingRead, BeingWritten; //!< indexes of buffer currently being ....
  tSize LastBlockSize; //! 0 if no read is in progress, size of the read being in progress otherwise
}; // CircBufferWithCont






