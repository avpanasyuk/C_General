/**
  * @file
  * @author Alexander Panasyuk
  * @brief User case - we prepare blocks to transfer something by adding single elements.
  * Like DMA transfer in UART - we may be adding char by char, but we want to transfer
  * big blocks when DMA becomes available.
  * Buffers are not supposed to run to end
  */

#ifndef DOUBLELINEARBUFFER_H_INCLUDED
#define DOUBLELINEARBUFFER_H_INCLUDED

#include <AVP_LIBS/General/Error.h>

template<typename T, size_t Size> class DoubleLinearBuffer {
protected:
  T *Buffer[2]; //!< two buffers - one being read, another written
  T *pWriter; //!< current position in write buffer
  uint8_t WriteBufferI;
  volatile bool ReadBufferDone; //!< Whether Read buffer had been read
public:
  DoubleLinearBuffer() {
    pWriter = Buffer[WriteBufferI = 0] = new T[Size];
    Buffer[1] = new T[Size];
    ReadBufferDone = true;
  } // constructor

  ~DoubleLinearBuffer() {
    delete[] Buffer[0];
    delete[] Buffer[1];
  }  // destructor

  bool Add(T x) {
    if(pWriter == Buffer[WriteBufferI]+Size) return false;
    *(pWriter++) = x;
    return true;
  } // Add

  size_t LeftToRead() { return pWriter - Buffer[WriteBufferI]; }

  /**
  * @param[out] pSz - pointer to store size to read, default null_ptr
  * @return pointer to the buffer currently being written. If another buffer is still being
  * read, so we can not switch to it, returns null_ptr while pSz returns correct size - this
  * is error condition
  */
  const T *GetBlockToRead(size_t *pSz = nullptr) {
    if(!LeftToRead()) { // nothing to get
      return null_ptr;
    } else {
      if(pSz != null_ptr) *pSz = LeftToRead();
      if(!ReadBufferDone) return null_ptr;
      // another buffer is already read, we can switch to it
      ReadBufferDone = false;
      pWriter = Buffer[WriteBufferI = 1 - WriteBufferI];
      return Buffer[1 - WriteBufferI];
    }
  } // GetBlockToRead

  void ReadDone() { ReadBufferDone = true; };
}; // DoubleLinearBuffer
#endif /* DOUBLELINEARBUFFER_H_INCLUDED */
