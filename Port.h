/*!
* @file Port.h
* @brief this class is one level of abstraction up from HW_UART. MCU-independent. 
* sending stuff by bytes and blocks, receiving by bytes
* 
* Created: 7/29/2013 2:37:48 PM
*  Author: panasyuk
*  @note To use this class first we have to define HW_UART class using UART_CLASS_DECL(I,PRRi,VectI) macro
*/


#ifndef AVP_PORT_H_
#define AVP_PORT_H_

#include <stdint.h>
#include <stdlib.h>
#include "CircBuffer.h"

namespace avp {
  //! @tparam tSize - type of CircBuffer counter, should be big enough to fit all buffer sizes.
  template<class HW_UART_, uint8_t Log2_TX_Buf_size=7, uint8_t Log2_TX_BlockBufSize=4, uint8_t Log2_RX_Buf_Size=7,
           typename tSize=uint8_t>
  struct  Port {
    typedef void (* tReleaseFunc)(void *p);
    protected:
    // there are two transmit buffers - one for bytes which buffers data  and one for blocks which buffers only pointyers, 
    // data are unbuffered!
    // blocks are not buffered, they provide ReleaseFunc function and are supposed to be intact all the time until
    // this function is called
    // we are transmitting normally from byte buffer
    // when write_pointed unbuffered block writing function is called we put descriptive block into block buffer
    // and  a byte == ESC_code into byte buffer to let transmitter callback know that there is a block to send here
    // if byte == ESC_code needs to be send 0-sized descriptive block put into block buffer
    // All this is done so a sequence of write and write_pointer produce the right sequence of bytes sent
    static CircBuffer<uint8_t, tSize, Log2_TX_Buf_size> BufferTX;	// byte transmit buffer

    // ***************  data for unbuffered block transmit buffer
    struct BlockInfo;
    struct BlockInfo {
      const uint8_t *Ptr;
      size_t Size;
      tReleaseFunc pReleaseFunc; // data pointed by Ptr should not get corrupted until this function is called
    }; // BlockInfo
    static CircBuffer<BlockInfo, tSize, Log2_TX_BlockBufSize> BlockInfoBufTX;	// Block transmit buffer
    static constexpr uint8_t ESC_code = 224; // this is special byte code placed in BufferTX to indicate that we
    // should transmit block here. If next byte is ESC_code we have to transmit block. If it is 0 we transmit just ECS_code
    // the value is random, selected so it is not ASCII and there is nothing special about it
    // *************** things for receive buffer
    static CircBuffer<uint8_t, tSize, Log2_RX_Buf_Size> BufferRX; //  receive buffer ( we receive by byte only )
    static const uint8_t *pCurByteInBlock;

    //! this function is called from HW_UART interrupt handler to store received byte in the RX Circular buffer
    static bool StoreReceivedByte(uint8_t b) {
      if(!BufferRX.LeftToWrite()) return false;
      BufferRX.Write(b);
      return true;
    } // StoreReceivedByte

    //! this function is called from HW_UART interrupt handler to get byte from Circular buffer to send
    //! WRITING TO *p immediately send byte, so do it ONLY ONCE !
    static bool GetByteToSend(volatile uint8_t *p) {
      // check whether we are reading from block currently
      if(pCurByteInBlock != nullptr) { 
        const BlockInfo *pCurBlock = BlockInfoBufTX.GetSlotToRead();

        if(++pCurByteInBlock == pCurBlock->Ptr + pCurBlock->Size) { // we are done with this block
          if(pCurBlock->pReleaseFunc != nullptr) (*pCurBlock->pReleaseFunc)((void *)pCurBlock->Ptr);
          BlockInfoBufTX.FinishedReading();
          pCurByteInBlock = nullptr;
        } else {
          *p = *pCurByteInBlock; // send next byte from the block
          return true;
        }
      }

      if(!BufferTX.LeftToRead()) return false;
      else {
        uint8_t b = BufferTX.Read_(); // can not write to *p here, it will be transmitted at once
        if(b != ESC_code) *p = b;
        else {
          // dealing with a special case here, either block or byte with ESC_Code value
          const BlockInfo *pCurBlock = BlockInfoBufTX.GetSlotToRead();
          if(pCurBlock->Size == 0) { // it is not really a block send, just a byte with ESC_code value
            BlockInfoBufTX.FinishedReading();
            *p = ESC_code;
          } else *p = *(pCurByteInBlock = BlockInfoBufTX.GetSlotToRead()->Ptr); // start sending block
        }
      }
      return true;
    } //  GetByteToSend

    //! If HW_UART is capable to send data by blocks (say via DMA) we can use this function
    //! this function is called from HW_UART interrupt handler to get data block to send
    //! @param[out] p - pointer to pointer from where to send data.
    //! @param[in,out] pSz - to return block size. ON INPUT IT SHOULD HAVE PRESERVED VALUE FROM THE PREVIOUS CALL!. 
    //! *pSz == 0 means current or previous byte operation, so size is really 1, but *pSz == 1 is
    //! reserved for 1-byte sized block operation
    static bool GetBlockToSend(uint8_t **p, size_t *pSz) {
      if (*pSz != 0) { BlockInfoBufTX.FinishedReading(); *pSz = 0; } // finish with previous block 
      if(!BufferTX.LeftToRead()) return false;
      else {
        static uint8_t b = BufferTX.Read_(); 
        *p = &b; // by default it is just a byte to send
        if(b == ESC_code) {
          // dealing with a special case here, either block or byte with ESC_Code value
          // in either case there should be BlockInfo allocated in the BlockInfoBufTX
          AVP_ASSERT(BlockInfoBufTX.LeftToRead());
          const BlockInfo *pCurBlock = BlockInfoBufTX.GetSlotToRead();
          if(pCurBlock->Size == 0) { // it is not really a block send, just a byte with ESC_code value,
            // so we are done with this BlockInfo
            BlockInfoBufTX.FinishedReading();
          } else { //  and here is really a block
            *p = BlockInfoBufTX.GetSlotToRead()->Ptr; // start sending block
            *pSz = BlockInfoBufTX.GetSlotToRead()->Size;
          }            
        }
      }
      return true;
     } //  GetBlockToSend

   public:
    //!!!!! USE ONLY ONE OF THE INIT FUNCTIONS DEPENDING ON HW_UART  CAPABILITIES
    static uint32_t Init(uint32_t baud) {
      return HW_UART_::Init(baud,StoreReceivedByte,GetByteToSend);
    } //  Init

    static uint32_t InitBlock(uint32_t baud) {
      return HW_UART_::Init(baud,StoreReceivedByte,GetBlockToSend);
    } //  InitBlock

    // ******************************** TRANSMISSION ******************
    // ALL write function return false if buffer is overrun and true if OK
    
    //! unsafe write - no BufferTX check, no interrupt reenable
    static bool write_(uint8_t d) {
      if(d == ESC_code) return write_pointed_(nullptr,0);
      BufferTX.Write_(d);
      return true;
    } // write_

    //! safe write
    static bool write(uint8_t d) {
      if(!BufferTX.LeftToWrite()) return false;
      bool Res = write_(d);
      HW_UART_::EnableTX_Interrupt(); // got something to transmit, reenable interrupt
      return Res;
    } // write

    //! buffered safe write
    static bool write(const uint8_t *Ptr, size_t Size) {
      if(Size == 0) return true; // that was easy
      if(Size > BufferTX.LeftToWrite()) return false;
      while(Size--) if(!write_(*(Ptr++))) return false;
      HW_UART_::EnableTX_Interrupt(); // got something to transmit, reenable interrupt
      return true;
    }  // write

    //! unbuffered unsafe write - no BufferTX check, no interrupt reenable 
    //! @param Size - block size. Size == 0 is a special case, we create a fake BlockInfo entry,
    //! it just mean that we have to transmit ESC_code byte
    static bool write_pointed_(const uint8_t *Ptr, size_t Size, tReleaseFunc pReleaseFunc = nullptr) {
      if(!BlockInfoBufTX.LeftToWrite()) return false;
      BlockInfo *pCurBlock = BlockInfoBufTX.GetSlotToWrite();
      if((pCurBlock->Size = Size) != 0) { // BlockInfo is real, not fake
        pCurBlock->Ptr = Ptr;
        pCurBlock->pReleaseFunc = pReleaseFunc;
      }
      BlockInfoBufTX.FinishedWriting();
      BufferTX.Write_(ESC_code); // do this afterwards, so transmit callback does not find ESC_code without block
      return true;
    } // write

    // unbuffered safe write
    static bool write_pointed(const uint8_t *Ptr, size_t Size, tReleaseFunc pReleaseFunc = nullptr) {
      if(Size == 0) return true;
      if(!BufferTX.LeftToWrite()) return false;
      bool Res = write_pointed_(Ptr,Size,pReleaseFunc);
      HW_UART_::EnableTX_Interrupt(); // got something to transmit, reenable interrupt
      return Res;
    } // write

    template<typename T> static bool write(T const *p, size_t Num = 1) { return Num != 0?write((const uint8_t *)p,sizeof(T)*Num):true; }
    static bool write(char const *str) { return write((const uint8_t *)str, ::strlen(str)); } // no ending 0
    template<typename T> static bool write(T d) { return write((const uint8_t *)&d,sizeof(T)); }
    static bool write(int8_t d) { return write((uint8_t)d); }

    // ********************************** RECEPTION *********************
    //! stores character by pointer pd, returns true if there is really a character to read
    static bool read(uint8_t *pd) { return BufferRX.Read(pd); }
    static uint8_t GetStatusRX() { return HW_UART_::GetStatusRX(); }
    static bool GotSomething() { return BufferRX.LeftToRead() != 0; }
    static uint8_t GetByte() { uint8_t out; read(&out); return out; }
      
  }; // BufferedPort

// following defines are just to make static variables initiation code readable, no point in using them
#define BP_ALIAS Port<HW_UART_,Log2_TX_Buf_size,Log2_TX_BlockBufSize,Log2_RX_Buf_Size, tSize>
#define BP_TEMPLATE template<class HW_UART_, uint8_t Log2_TX_Buf_size, uint8_t Log2_TX_BlockBufSize, uint8_t Log2_RX_Buf_Size, typename tSize>

  BP_TEMPLATE const uint8_t *BP_ALIAS::pCurByteInBlock = nullptr;
  BP_TEMPLATE CircBuffer<uint8_t, tSize, Log2_TX_Buf_size> BP_ALIAS::BufferTX;
  BP_TEMPLATE CircBuffer<struct BP_ALIAS::BlockInfo, tSize, Log2_TX_BlockBufSize> BP_ALIAS::BlockInfoBufTX;
  BP_TEMPLATE CircBuffer<uint8_t, tSize, Log2_RX_Buf_Size> BP_ALIAS::BufferRX;
  BP_TEMPLATE constexpr uint8_t BP_ALIAS::ESC_code; // this is special byte code placed in BufferTX to indicate that we
}; // avp


#endif /* AVP_PORT_H_ */

