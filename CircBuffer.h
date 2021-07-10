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
  * @note: The class does not use virtual functions, so do not monkey with pointers to base classes
  * @tparam CounterType - type of size variable
  * @tparam size - size in bytes.
  */
template <typename T, size_t size, typename CounterType=size_t>
struct CircBuffer {
    static_assert(std::is_unsigned<CounterType>::value,"'CounterType' has to be unsigned type!");

    // *********** General functions
    CircBuffer():BeingWritten(0) { Clear(); }
    void Clear()  {  BeingRead = BeingWritten; }
    static constexpr CounterType GetCapacity() { return size - 1; };

    //************* Reader functions ***************************************
    CounterType LeftToRead() const  { return BeingWritten >= BeingRead?BeingWritten - BeingRead:BeingWritten + size - BeingRead; }

    //! @brief returns the same slot if called several times in a row. Only FinishReading moves pointer
    const T *GetSlotToRead() const { return &Buffer[BeingRead]; }

    void FinishedReading() { if(++BeingRead == size) BeingRead = 0; }

    T Read_() { T temp = *GetSlotToRead(); FinishedReading(); return temp; }

    /** safe read. Returns false if the buffer is  empty.
     * @param Dst - pointer to store read data via
     * @return success of operation
     */
    bool Read(T* Dst) {
      if(LeftToRead() == 0) return false;
      else { *Dst = Read_(); return true; }
    } // safer Read

    //************* Writer functions *************************************************
    uint8_t LeftToWrite() const  { return GetCapacity() - LeftToRead(); }

    T *GetSlotToWrite() { return &Buffer[BeingWritten]; }

    void FinishedWriting() { if(++BeingWritten == size) BeingWritten = 0; }

    void Write_(T const &d) { *GetSlotToWrite() = d; FinishedWriting(); }

    /** safe write. Returns false is buffer is full.
     * @param Dst - pointer to store read data via
     * @return success of operation
     */
    bool Write(T const &d) {
      if(LeftToWrite() == 0) return false;
      else { Write_(d); return true; }
    } // safe Write

    // It is risky function because if there is no place to write it modifies BeingRead index, so interferes with reading function. E.g
    // if read is in progress and this function is called from the interrupt (or vise versa) things may get screwed up.
    // The function never fails
    void ForceFinishedWriting()  {
      if(LeftToWrite() == 0) FinishedReading();
      FinishedWriting();
    } // ForceFinishedWriting

    /// returns a preciding entry counting from Write pointer
    /// @param BackIndex - how many entries before, 0 corresponds to current Write pointer
    const T &operator[] (CounterType BackIndex) const {
      return Buffer[BeingWritten >= BackIndex?BeingWritten - BackIndex:BeingWritten + size - BackIndex];
    }
  protected:
    CounterType BeingRead, BeingWritten; //!< indexes of buffer currently being ....
    avp::Vector<T,size> Buffer;
}; // CircBuffer

/** CircBufferPWR2 is a CircBuffer with size being a power of 2, so for rollovers I can use binary AND operation
 * instead of condition statements, I think it is faster
 * @tparam BitsInCounter determines Buffer size = 1<<BitsInCounter
 */
template <typename T, uint8_t BitsInCounter, typename CounterType=size_t>
struct CircBufferPWR2: public CircBuffer<T, size_t(1) << BitsInCounter, CounterType> {
  static_assert(std::numeric_limits<CounterType>::digits >= BitsInCounter,
      "'CounterType' is too small to hold size!");

  using Base = CircBuffer<T, size_t(1) << BitsInCounter, CounterType>;

  //************* Writer functions *************************************************
  CounterType LeftToWrite() const  { return (Base::BeingRead - 1 - Base::BeingWritten) & Mask; }

  void FinishedWriting() { Base::BeingWritten = (Base::BeingWritten + 1) & Mask; }

  void Write_(T const &d) { *Base::GetSlotToWrite() = d; FinishedWriting(); }

  bool Write(T const &d) {
    if(LeftToWrite() == 0) return false;
    else { Write_(d); return true; }
  } // safe Write

  // It is risky function because if there is no place to write it modifies BeingRead index, so interferes with reading function. E.g
  // if read is in progress and this function is called from the interrupt (or vise versa) things may get screwed up.
  // The function never fails
  void ForceFinishedWriting()  {
    if(LeftToWrite() == 0) FinishedReading();
    FinishedWriting();
  } // ForceFinishedWriting

  //************* Reader functions ***************************************
  CounterType LeftToRead() const  { return (Base::BeingWritten - Base::BeingRead) & Mask; }

  void FinishedReading() { Base::BeingRead = (Base::BeingRead + 1) & Mask; }

  T Read_() { T temp = *Base::GetSlotToRead(); FinishedReading(); return temp; }

  bool Read(T* Dst) {
    if(LeftToRead() == 0) return false;
    else { *Dst = Read_(); return true; }
  } // safer Read

protected:
  static constexpr CounterType Mask = Base::GetCapacity(); //!< marks used bits in index variables
}; // CircBufferPWR2

/** CircBufferAutoWrap is a CircBuffer with size precisely fitting into one of the numeric types, so for rollovers
 * I do not have to so a things, they are automatic
 * @tparam BitsInCounter determines Buffer size = 1<<BitsInCounter
 */
template <typename T, typename CounterType>
struct CircBufferAutoWrap: public CircBuffer<T, size_t(std::numeric_limits<CounterType>::max()) + 1, CounterType> {
  static_assert(std::is_unsigned<CounterType>::value,"'CounterType' has to be unsigned type!");

  using Base = CircBuffer<T, size_t(std::numeric_limits<CounterType>::max()) + 1, CounterType>;

  //************* Writer functions *************************************************
  CounterType LeftToWrite() const  { return Base::BeingRead - 1 - Base::BeingWritten; }

  void FinishedWriting() { ++Base::BeingWritten; }

  void Write_(T const &d) { *Base::GetSlotToWrite() = d; FinishedWriting(); }

  bool Write(T const &d) {
    if(LeftToWrite() == 0) return false;
    else { Write_(d); return true; }
  } // safe Write

  // It is risky function because if there is no place to write it modifies BeingRead index, so interferes with reading function. E.g
  // if read is in progress and this function is called from the interrupt (or vise versa) things may get screwed up.
  // The function never fails
  T *ForceSlotToWrite()  {
    if(LeftToWrite() == 0) FinishedReading();
    return Base::GetSlotToWrite();
  } // ForceSlotToWrite

  //************* Reader functions ***************************************
  CounterType LeftToRead() const  { return Base::BeingWritten - Base::BeingRead; }

  void FinishedReading() { ++Base::BeingRead; }

  T Read_() { T temp = *Base::GetSlotToRead(); FinishedReading(); return temp; }

  bool Read(T* Dst) {
    if(LeftToRead() == 0) return false;
    else { *Dst = Read_(); return true; }
  } // safer Read
}; // CircBufferPWR2

#endif /* CIRCBUFFER_H_ */
