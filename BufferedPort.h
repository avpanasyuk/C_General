/*!
* @file BufferedPort.h
*
* Created: 7/29/2013 2:37:48 PM
*  Author: panasyuk
*  @note To use this class first we have to define HW_UART class using UART_CLASS_DECL(I,PRRi,VectI) macro
*/


#ifndef AVP_UART_H_
#define AVP_UART_H_

#include <stdint.h>
#include <string.h>
#include "CircBuffer.h"

namespace avp {
  //! @tparam tSize - type of CircBuffer counter, should be big enough to fit all buffer sizes.
  template<class HW_UART_, uint8_t Log2_TX_Buf_size=7, uint8_t Log2_TX_BlockBufSize=4, uint8_t Log2_RX_Buf_Size=7,
           typename tSize=uint8_t>
  class BufferedPort {
    // there are two transmit buffers - for bytes and for blocks
    // blocks are not buffered, they provide ReleaseFunc function and are supposed to be intact all the time until
    // this function is called
    // we are transmitting normally from byte buffer
    // when write_pointed unbuffered block writing function is called we put descriptive block into block buffer
    // and  a byte == ESC_code into byte buffer to let transmitter callback know that there is a block to send here
    // if byte == ESC_code needs to be send 0-sized descriptive block put into block buffer
    // All this is done so a sequence of write and write_pointer produce the right sent sequence of bytes
    static CircBuffer<uint8_t, tSize, Log2_TX_Buf_size> BufferTX;	// byte transmit buffer

    // ***************  data for block transmit buffer
    struct PointedBlock;
    typedef void (* tReleaseFunc)(void *p); // data pointed by p should not vary while being transmitted. This function is called
    struct PointedBlock {
      const uint8_t *Ptr;
      uint16_t Size;
      tReleaseFunc pReleaseFunc; // data pointed by Ptr should not vary while being transmitted. This function is called
      // when they got transmitted.
    }; // PointedBlock
    static CircBuffer<PointedBlock, tSize, Log2_TX_BlockBufSize> BlockBufTX;	// Block transmit buffer
    static constexpr uint8_t ESC_code = 224; // this is special byte code placed in BufferTX to indicate that we
    // should transmit block here. If next byte is ESC_code we have to transmit block. If it is 0 we transmit just ECS_code
    // the value is random, selected so it is not ASCII and there is nothing special about it
    // *************** things for receive buffer
    static CircBuffer<uint8_t, tSize, Log2_RX_Buf_Size> BufferRX; //  receive buffer ( we receive by byte only )
    static const uint8_t *pCurByteInBlock;

    //! this function is called from RX_vect in HW_UART to store received byte in Circular buffer
    static bool StoreReceivedByte(uint8_t b) {
      if(!BufferRX.LeftToWrite()) return false;
      BufferRX.Write(b);
      return true;
    } // StoreReceivedByte

    //! this function is called from UDRE_vect in HW_UART to get byte from Circular buffer to send
    //! WRITING TO *p immediately send byte, so do it ONLY ONCE !
    static bool GetByteToSend(volatile uint8_t *p) {
      if(pCurByteInBlock != nullptr) { // we are reading from block currently
        const PointedBlock *pCurBlock = BlockBufTX.GetSlotToRead();

        if(++pCurByteInBlock == pCurBlock->Ptr + pCurBlock->Size) { // we are done with this block
          if(pCurBlock->pReleaseFunc != nullptr) (*pCurBlock->pReleaseFunc)((void *)pCurBlock->Ptr);
          BlockBufTX.FinishedReading();
          pCurByteInBlock = nullptr;
        } else {
          *p = *pCurByteInBlock; // continue sending block
          return true;
        }
      }

      if(!BufferTX.LeftToRead()) return false;
      else {
        uint8_t b = BufferTX.Read_(); // can not write to *p here, it will be transmitted at once
        if(b != ESC_code) *p = b;
        else {
          // dealing with a special case here
          const PointedBlock *pCurBlock = BlockBufTX.GetSlotToRead();
          if(pCurBlock->Size == 0) {
            BlockBufTX.FinishedReading();
            *p = ESC_code;
          } else *p = *(pCurByteInBlock = BlockBufTX.GetSlotToRead()->Ptr); // start sending block
        }
      }
      return true;
    } //  GetByteToSend

   public:
    static uint32_t Init(uint32_t baud) {
      return HW_UART_::Init(baud,StoreReceivedByte,GetByteToSend);
    } //  Init

    // BufferedPort(uint32_t baud) { Init(baud); }

    static uint8_t LeftToTX() { return BufferTX.LeftToRead(); }

    //! stores character along pd, returns true if there was a character
    static bool read(uint8_t *pd) { return BufferRX.Read(pd); }

    // ALL write function return false is overrun and true if OK
    static bool write_(uint8_t d) {
      if(d == ESC_code) return write_pointed_(nullptr,0);
      BufferTX.Write_(d);
      return true;
    } // write_

    static bool write(uint8_t d) {
      if(!BufferTX.LeftToWrite()) return false;
      bool Res = d == ESC_code?write_pointed(nullptr,0):write_(d);
      HW_UART_::EnableTX_Interrupt(); // got something to transmit, reenable interrupt
      return Res;
    } // write

    // buffered write
    static bool write(const uint8_t *Ptr, size_t Size) {
      if(Size == 0) return true; // that was easy
      if(Size > BufferTX.LeftToWrite()) return false;
      while(Size--) if(!write_(*(Ptr++))) return false;
      HW_UART_::EnableTX_Interrupt(); // got something to transmit, reenable interrupt
      return true;
    }  // write

    // unbuffered write
    // Size == 0 is a special case, it just mean that we have to transmit ESC_code byte

    static bool write_pointed_(const uint8_t *Ptr, size_t Size, tReleaseFunc pReleaseFunc = nullptr) {
      if(!BlockBufTX.LeftToWrite()) return false;
      PointedBlock *pCurBlock = BlockBufTX.GetSlotToWrite();
      if((pCurBlock->Size = Size) != 0) {
        pCurBlock->Ptr = Ptr;
        pCurBlock->pReleaseFunc = pReleaseFunc;
      }
      BlockBufTX.FinishedWriting();
      BufferTX.Write_(ESC_code); // do this afterwards, so transmit callback does not find ESC_code without block
      return true;
    } // write

    static bool write_pointed(const uint8_t *Ptr, size_t Size, tReleaseFunc pReleaseFunc = nullptr) {
      if(!BufferTX.LeftToWrite()) return false;
      bool Res = write_pointed_(Ptr,Size,pReleaseFunc);
      HW_UART_::EnableTX_Interrupt(); // got something to transmit, reenable interrupt
      return Res;
    } // write

    static uint8_t GetStatusRX() { return HW_UART_::GetStatusRX(); }
    static bool GotSomething() { return BufferRX.LeftToRead() != 0; }
    static uint8_t GetByte() { uint8_t out; read(&out); return out; }
      
    template<typename T> static bool write(T const *p, size_t Num = 1) { return write((const uint8_t *)p,sizeof(T)*Num); }
    static bool write(char const *str) { return write((const uint8_t *)str, ::strlen(str)); } // no ending 0
    template<typename T> static bool write(T d) { return write(&d,sizeof(T)); }
    static bool write(int8_t d) { return write((uint8_t)d); }
  }; // BufferedPort

// following defines are just to make code readable, no use
#define BP_ALIAS BufferedPort<HW_UART_,Log2_TX_Buf_size,Log2_TX_BlockBufSize,Log2_RX_Buf_Size, tSize>
#define BP_TEMPLATE template<class HW_UART_, uint8_t Log2_TX_Buf_size, uint8_t Log2_TX_BlockBufSize, uint8_t Log2_RX_Buf_Size, typename tSize>

  BP_TEMPLATE const uint8_t *BP_ALIAS::pCurByteInBlock = nullptr;
  BP_TEMPLATE CircBuffer<uint8_t, tSize, Log2_TX_Buf_size> BP_ALIAS::BufferTX;
  BP_TEMPLATE CircBuffer<struct BP_ALIAS::PointedBlock, tSize, Log2_TX_BlockBufSize> BP_ALIAS::BlockBufTX;
  BP_TEMPLATE CircBuffer<uint8_t, tSize, Log2_RX_Buf_Size> BP_ALIAS::BufferRX;
  BP_TEMPLATE constexpr uint8_t BP_ALIAS::ESC_code; // this is special byte code placed in BufferTX to indicate that we

}; // avp


#endif /* AVP_UART_H_ */

