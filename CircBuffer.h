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
  * like in Clear()
  * NOTE: When both BeingRead and BeingWritten refer the same block the buffer is empty, as this block is
  * not written yet, so there is nothing to read
  * NOTE: Writing and reading function DO NOT CHECK WHETHER THERE IS SPACE! THEY NEVER RETURN
  * NULLPTR. Call LeftTo... function beforehand
  * @tparam CounterType - type of size variable
  * @tparam size - size in bytes.
  */
template <typename T, typename CounterType, CounterType size>
struct CircBufferBase {
    static_assert(std::is_unsigned<CounterType>::value,"'CounterType' has to be unsigned type!");
    // *********** General functions
    CircBufferBase() { BeingWritten = 0; Clear(); }
    void Clear()  {  BeingRead = BeingWritten; }
    static constexpr CounterType GetCapacity() { return size - 1; };

    //************* Reader functions ***************************************
    CounterType LeftToRead() const  { return BeingWritten >= BeingRead?BeingWritten - BeingRead:BeingWritten + size - BeingRead; }

    //! @brief returns the same slot if called several times in a row. Only FinishReading moves pointer
    T const *GetSlotToRead() { return &Buffer[BeingRead]; }

    void FinishedReading() { if(++BeingRead == size) BeingRead = 0; }

    T Read_() { T temp = *GetSlotToRead(); FinishedReading(); return temp; }

    bool Read(T* Dst) {
      if(LeftToRead() == 0) return false;
      else { *Dst = Read_(); return true; }
    } // safer Read

    //************* Writer functions *************************************************
    uint8_t LeftToWrite() const  { return GetCapacity() - LeftToRead(); }

    T *GetSlotToWrite() { return &Buffer[BeingWritten]; }

    void FinishedWriting() { if(++BeingWritten == size) BeingWritten = 0; }

    void Write_(T const &d) { *GetSlotToWrite() = d; FinishedWriting(); }

    bool Write(T const &d) {
      if(LeftToWrite() == 0) return false;
      else { Write_(d); return true; }
    } // safe Write

    // It is risky function because if there is no place to write it modifies BeingRead index, so interferes with reading function. E.g
    // if read is in progress and this function is called from the interrupt (or vise versa) things may get screwed up.
    // The function never fails
    T *ForceSlotToWrite()  {
      if(LeftToWrite() == 0) FinishedReading();
      return GetSlotToWrite();
    } // ForceSlotToWrite

    /// returns previous entry from Write pointer
    /// @param BackIndex - how many entries before, 0 corresponds to current Write pointer
    const T &operator[] (CounterType BackIndex) const {
      return Buffer[BeingWritten >= BackIndex?BeingWritten - BackIndex:BeingWritten + size - BackIndex];
    }
  protected:
    CounterType BeingRead, BeingWritten; //!< indexes of buffer currently being ....
    avp::Vector<T,size> Buffer;
}; // CircBufferBase


template <typename T, size_t size>
struct CircBuffer: public CircBufferBase<T,size_t,size> {};

template <uint8_t BitsInCounter>
struct CircBufferCounter {
  static constexpr size_t GetCapacity() { return (size_t(1) << BitsInCounter) - 1; };

  CircBufferCounter():BeingWritten(0) { Clear(); }
  void Clear()  {  BeingRead = BeingWritten; }

  //************* Reader functions ***************************************
  size_t LeftToRead() const  { return BeingWritten - BeingRead; }
  void FinishedReading() { ++BeingRead; }

  //************* Writer functions *************************************************
  size_t LeftToWrite() const  { return GetCapacity() - LeftToRead(); }
  void FinishedWriting() { ++BeingWritten; }
protected:
  size_t BeingRead:BitsInCounter;
  size_t BeingWritten:BitsInCounter;
}; // CircBufferCounter

#define CIRC_BUFFER_SPEC(type) \
   template <> \
   struct CircBufferCounter<std::numeric_limits<type>::digits> { \
   CircBufferCounter():BeingWritten(0) { Clear(); } \
   void Clear()  {  BeingRead = BeingWritten; } \
   size_t LeftToRead() const  { return BeingWritten - BeingRead; } \
   static constexpr type GetCapacity() { return std::numeric_limits<type>::max(); }; \
   void FinishedReading() { ++BeingRead; } \
   size_t LeftToWrite() const  { return GetCapacity() - LeftToRead(); } \
   void FinishedWriting() { ++BeingWritten; } \
   protected: \
     type BeingRead, BeingWritten; \
   } // CircBufferCounter specialization

CIRC_BUFFER_SPEC(uint8_t);
CIRC_BUFFER_SPEC(uint16_t);
CIRC_BUFFER_SPEC(uint32_t);


/** CircBufferPWR2 is mostly similar to \ref CircBuffer
  * @note CircBufferPWR2 is simpler/faster than CircBuffer because for all the counters overflows it relies on
  * automatic type wrap. When BitsInCounter = 8, 16 or 32 it is expecially fast, as it does not have to deal with bit fields
  * @tparam BitsInCounter - size of counters in bits, defines size and maximum size which fits. for 8.16 or 32
  * the class is specially fast
  */
template <typename T, uint8_t BitsInCounter>
struct CircBufferPWR2: public CircBufferCounter<BitsInCounter> {
  // *********** General functions


    //************* Reader functions ***************************************

    //! @brief returns the same slot if called several times in a row. Only FinishReading moves pointer
    T const *GetSlotToRead() { return &Buffer[CircBufferCounter<BitsInCounter>::BeingRead]; }

    T Read_() { T temp = *GetSlotToRead(); CircBufferCounter<BitsInCounter>::FinishedReading(); return temp; }

    bool Read(T* Dst) {
      if(CircBufferCounter<BitsInCounter>::LeftToRead() == 0) return false;
      else { *Dst = Read_(); return true; }
    } // safer Read

    //************* Writer functions *************************************************
    T *GetSlotToWrite() { return &Buffer[CircBufferCounter<BitsInCounter>::BeingWritten]; }

    void Write_(T const &d) { *GetSlotToWrite() = d; CircBufferCounter<BitsInCounter>::FinishedWriting(); }

    bool Write(T const &d) {
      if(CircBufferCounter<BitsInCounter>::LeftToWrite() == 0) return false;
      else { Write_(d); return true; }
    } // safe Write

    // It is risky function because if there is no place to write it modifies BeingRead index, so interferes with reading function. E.g
    // if read is in progress and this function is called from the interrupt (or vise versa) things may get screwed up.
    // The function never fails
    T *ForceSlotToWrite()  {
      if(CircBufferCounter<BitsInCounter>::LeftToWrite() == 0) CircBufferCounter<BitsInCounter>::FinishedReading();
      return GetSlotToWrite();
    } // ForceSlotToWrite

    /// returns previous entry from Write pointer
    /// @param BackIndex - how many entries before, 0 corresponds to current Write pointer
    const T &operator[] (size_t BackIndex) const {
      return Buffer[CircBufferCounter<BitsInCounter>::BeingWritten - BackIndex];
    }
  protected:
    avp::Vector<T,size_t(1) << BitsInCounter> Buffer;
}; // CircBufferBase

#endif /* CIRCBUFFER_H_ */
