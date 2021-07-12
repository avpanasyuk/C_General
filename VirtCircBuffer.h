/*
* CircBuffer.h
*
* Created: 7/27/2013 8:03:10 PM
*  Author: panasyuk
*/

#ifndef VIRTCIRCBUFFER_H_
#define VIRTCIRCBUFFER_H_

#include <stddef.h>
#include <stdint.h>
#include <type_traits>
#include "Vector.h"
// #include "Error.h"

/** Circular Buffer of elements of class T. One reader and one writer may work in parallel. Reader is using
  * only BeingRead index, and writer only BeingWritten, so index can be screwed-up ONLY when cross-used,
  * like in Clear(). I do not need to MUTEXes!
  * @note: When both BeingRead and BeingWritten refer the same block the buffer is empty, as this block is
  * not written yet, so there is nothing to read, but full buffer to write
  * @note: Writing and reading function DO NOT CHECK WHETHER THERE IS SPACE! THEY NEVER RETURN
  * NULLPTR. Call LeftToRead or LeftToWrite functions beforehand
  * @tparam CounterType - type of size variable
  * @tparam size - size of the buffer in elements.
  *
  * FIXME: CircBufferWithReadBlock does not work because we can not figure out type T
  */

template <typename T, size_t size, typename CounterType = size_t>
struct CircBufferBase { // abstract class
  static_assert(std::is_unsigned<CounterType>::value,"'CounterType' has to be unsigned type!");

  // *********** General functions
  explicit CircBufferBase():BeingWritten(0) { Clear(); }

  virtual void Clear()  {  BeingRead = BeingWritten; }
  static constexpr CounterType GetCapacity() { return size - 1; };

  //************* Writer functions *************************************************
  uint8_t LeftToWrite() const  { return GetCapacity() - LeftToRead(); }

  T *GetSlotToWrite() { return &Buffer[BeingWritten]; }

  void Write_(T const &d, bool Forced = false) { *GetSlotToWrite() = d; if(Forced) ForceFinishedWriting(); else FinishedWriting(); }

  /** safe write. Returns false is buffer is full.
   * @param Dst - pointer to store read data via
   * @return success of operation
   */
  bool Write(T const &d) {
    if(LeftToWrite() == 0) return false;
    else { Write_(d); return true; }
  } // safe Write

  virtual void FinishedWriting() = 0;

  // It is risky function because if there is no place to write it modifies BeingRead index, so interferes with reading function. E.g
  // if read is in progress and this function is called from the interrupt (or vise versa) things may get screwed up.
  // The function never fails
  void ForceFinishedWriting()  {
    if(LeftToWrite() == 0) FinishedReading();
    FinishedWriting();
  } // ForceFinishedWriting

  //************* Reader functions ***************************************

  /// returns a preciding entry counting from Write pointer
  /// @param BackIndex - how many entries before, 0 corresponds to current Write pointer
  virtual CounterType LeftToRead() const  = 0;


  //! @brief returns the same slot if called several times in a row. Only FinishReading moves pointer
  virtual const T *GetSlotToRead() { return &Buffer[BeingRead]; }

  virtual void FinishedReading() { ++BeingRead; NormalizeReadCounter(); }

  T Read_() { T temp = *GetSlotToRead(); FinishedReading(); return temp; }

  /** safe read. Returns false if the buffer is  empty.
   * @param Dst - pointer to store read data via
   * @return success of operation
   */
  bool Read(T* Dst) {
    if(LeftToRead() == 0) return false;
    else { *Dst = Read_(); return true; }
  } // safer Read

  /// Returns element with was written BackIndex elements back
  virtual const T &operator[] (CounterType BackIndex) const = 0;
protected:
  CounterType BeingRead, BeingWritten; //!< indexes of buffer currently being read and written correspondingly. From 0 to GetCapacity()
  avp::Vector<T,size> Buffer;

  virtual void NormalizeReadCounter() = 0;
}; // CircBufferBase

template <typename T, size_t size, typename CounterType=size_t>
struct CircBuffer: public CircBufferBase<T,size,CounterType> {
    using Base = CircBufferBase<T,size,CounterType>;

    virtual void FinishedWriting() override { if(++Base::BeingWritten == size) Base::BeingWritten = 0; }

    virtual CounterType LeftToRead() const  override {
      return Base::BeingWritten >= Base::BeingRead?Base::BeingWritten - Base::BeingRead:Base::BeingWritten + size - Base::BeingRead;
    }

    virtual const T &operator[] (CounterType BackIndex) const override {
      return Base::Buffer[Base::BeingWritten >= BackIndex?Base::BeingWritten - BackIndex:Base::BeingWritten + size - BackIndex];
    }
  protected:
    virtual void NormalizeReadCounter() { if(Base::BeingRead == size) Base::BeingRead = 0; }
}; // CircBuffer

/** CircBufferPWR2 is a CircBuffer with size being a power of 2, so for rollovers I can use binary AND operation
 * instead of condition statements, I think it is faster
 * @tparam BitsInCounter determines Buffer size = 1<<BitsInCounter
 */
template <typename T, uint8_t BitsInCounter, typename CounterType=size_t>
struct CircBufferPWR2: public CircBufferBase<T, size_t(1) << BitsInCounter, CounterType> {
  using Base = CircBufferBase<T, size_t(1) << BitsInCounter, CounterType>;

  virtual void FinishedWriting() override { Base::BeingWritten = (Base::BeingWritten + 1) & Mask; }

  virtual CounterType LeftToRead() const override { return (Base::BeingWritten - Base::BeingRead) & Mask; }

  virtual const T &operator[] (CounterType BackIndex) const override {
    return Base::Buffer[(Base::BeingWritten - BackIndex) & Mask];
  }

protected:
  static constexpr CounterType Mask = Base::GetCapacity(); //!< marks used bits in index variables

  virtual void NormalizeReadCounter() override { Base::BeingRead &= Mask; }
}; // CircBufferPWR2

/** CircBufferAutoWrap is a CircBuffer with size precisely fitting into one of the numeric types, so for rollovers
 * I do not have to so a things, they are automatic
 */
template <typename T, typename CounterType>
struct CircBufferAutoWrap: public CircBuffer<T, size_t(std::numeric_limits<CounterType>::max()) + 1, CounterType> {
  using Base = CircBuffer<T, size_t(std::numeric_limits<CounterType>::max()) + 1, CounterType>;

  virtual void FinishedWriting() override { ++Base::BeingWritten; }

  virtual CounterType LeftToRead() const override { return Base::BeingWritten - Base::BeingRead; }

  virtual const T &operator[] (CounterType BackIndex) const override { return Base::Buffer[Base::BeingWritten - BackIndex]; }

protected:
  virtual void NormalizeReadCounter() override {}
}; // CircBufferAutoWrap


/**
 * this is CircBuffer class which allows to read entries by continues blocks. It still writes one by one
 * @tparam CircBufferClass - may be any of CircBuffer??? classes
 */
template <class CircBufferClass>
struct CircBufferWithReadBlock: public CircBufferClass {

  CircBufferWithReadBlock() { Clear(); }
  virtual void Clear() override { Clear(); LastReadSize = 0; }
//************* Writer functions *************************************************
  // It is marginally safer function because it moves BeingRead index only when no read is in progress. Interrupts may still screw things up
  // if racing condition occurs. Fails and returns flase is reading is in progress
  bool SaferForceFinishedWriting()  {
    if(CircBufferClass::LeftToWrite() == 0) { // we will try to free some space
      if(LastReadSize == 0) FinishedReading(); // move read pointer if there is no read in progress
      else return false; // will fail but not overwrite data being read
    }
    CircBufferClass::FinishedWriting();
    return true;
  } // SaferForceFinishedWriting

  //! @brief returns the same slot if called several times in a row. Only FinishReading moves pointer
  virtual const decltype(CircBufferClass::Buffer[0]) *GetSlotToRead() override { LastReadSize = 1; return CircBufferClass::GetSlotToRead(); }

  virtual void FinishedReading() override { CircBufferClass::BeingRead += LastReadSize; CircBufferClass::NormalizeReadCounter(); LastReadSize = 0; }

  // ************* Continous block reading functions
  /** instead of a single entry marks for reading a continuous block.
  * Use GetReadSize() to determine size of the block to read
  */
  auto GetContinousBlockToRead() {
    // if true BeingWritten has wrapped, so the continues block is only to the end of the buffer
    LastReadSize = CircBufferClass::BeingRead > CircBufferClass::BeingWritten ? CircBufferClass::GetCapacity() + 1 - CircBufferClass::BeingRead : CircBufferClass::LeftToRead();
    return &CircBufferClass::Buffer[CircBufferClass::BeingRead];
  } // GetContinousBlockToRead

  /**
   * returns the size of the continuous block the last GetContinousBlockToRead() points to
   */
  size_t GetReadSize() const { return LastReadSize; }
protected:
  size_t LastReadSize; //! 0 if no read is in progress, size of the read being in progress otherwise
}; // CircBufferWithCont

#endif /* CIRCBUFFER_H_ */
