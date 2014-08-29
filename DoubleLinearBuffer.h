/**
  * @file
  * @author Alexander Panasyuk
  * @brief User case - we prepare blocks to transfer something by adding single elements.
  * Like DMA transfer in UART - we may be adding char by char, but we want to transfer
  * big blocks when DMA becomes available
  * NOTE this thing does not work with interrupts because we have to consistently change two variables
  */

#include <AVP_LIBS/General/Error.h>

#ifndef DOUBLELINEARBUFFER_H_INCLUDED
#define DOUBLELINEARBUFFER_H_INCLUDED

template<typename T, size_t Size> class DoubleLinearBuffer {
protected:
  T *Buffer[2];
  volatile T *pToRead, *pToStore;
  volatile uint8_t SideToReadI, SideToStoreI;
public:
  DoubleLinearBuffer() {
      Buffer[0] = new T[Size];
      Buffer[1] = new T[Size];
      pToStore = pToRead = Buffer[SideToStoreI = SideToReadI = 0];
    } // constructor

  ~DoubleLinearBuffer() {
      delete[] Buffer[0];
      delete[] Buffer[1];
   }  // destructor

  bool Add(const T *pNew) {
    if(pToStore == Buffer[SideToStoreI] + Size) { // ran out of this buffer side
      if(SideToReadI != SideToStoreI) return false; // we are still reading from another side
      pToStore = Buffer[SideToStoreI = 1 - SideToStoreI];
      return Add(pNew);
    } else {
        *(pToStore++) = *pNew;
        return true;
    }
  } // Add

  T *GetOneAndMark() {
    if(pToRead == pToStore) return nullptr;
    if(pToRead == Buffer[SideToReadI] + Size) { // read this whole Side
      pToRead = Buffer[SideToReadI = 1 - SideToReadI];
      return GetOneAndMark();
    } else return pToRead++;
  } // GetOne

  const T *GetBlock(size_t *Sz = nullptr) {
    if(pToRead == pToStore) return nullptr;
    if(Sz != 0)
      *Sz = (SideToReadI == SideToStoreI?pToStore:Buffer[SideToReadI] + Size) - pToRead;
    // removing volatiliness because pointed values are not going to change until we are done
    return (T const *)pToRead;
  } // GetBlock

  void MarkAsDone() {
    pToRead = SideToReadI == SideToStoreI?pToStore:Buffer[SideToReadI = 1 - SideToReadI];
  } // MarkAsDone
}; // DoubleLinearBuffer
#endif /* DOUBLELINEARBUFFER_H_INCLUDED */
