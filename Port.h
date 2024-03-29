/*!
* @file Port.h
* @brief this class is one level of abstraction up from HW communication protocols, like
* Serial, SPI or I2C. MCU-independent.
* sending stuff by bytes and blocks, receiving by bytes. It is buffered for both
* transmission and reception
*
* There are two types of HW_IO class - one can transmit only by bytes, another by block (via DMA, e.g)
* template Port should not be instantiated - use either PortByteTX or PortBlockTX, depending on capability
*
* -# should be static
* -# member functions:
*    - static void SetCallBacks(tStoreReceivedByte pStoreReceivedByte_,  tGetBlockToSend pGetBlockToSend_), or
*    - static void SetCallBacks(tStoreReceivedByte pStoreReceivedByte_,  tGetByteToSend pGetByteToSend_), depending on class capability
*    (whether it can send by blocks, via, e.g. DMA), where
*      - bool StoreReceivedByte(uint8_t b)
*      - bool GetByteToSend(uint8_t *p)
*      - bool GetBlockToSend(uint8_t **p, size_t *pSz)
* -# static void TryToSend(); - this class calls it to let HW_IO_ know that there are
*    new data in buffer to transmit. This function may be called at the end of all primary "write"
*    functions, from the interrupt indicating end of previous transfer, or from the "cycle" loop
*    just in case. It should maintain locks when necessary.
* -# HW_IO_ should call StoreReceivedByte supplied to it by SetCallBacks call when it received a byte
* -# HW_IO_ should call GetBlockToSend or GetByteToSend when it is ready to send new data
* -# should provide static void FlushRX();
*
* To avoid needless data copying  there are two transmit buffers - one for bytes which buffers data
* and one for blocks which buffers only pointers,
* data themselves are unbuffered!
* blocks are not buffered, ReleaseFunc function should be provided with a blcok
* and data in block are supposed to be intact all the time until
* this function is called
* we are transmitting normally from byte buffer
* when write_unbuffered unbuffered block writing function is called we put descriptive block into block pointer buffer
* and  a byte == ESC_code into byte buffer to let transmitter callback know that there is a block to send here
* if byte == ESC_code needs to be send 0-sized descriptive block put into block buffer
* All this is done so a sequence of write and write_pointer produce the right sequence of bytes sent
*
* Created: 7/29/2013 2:37:48 PM
*  Author: panasyuk
*/


#ifndef AVP_PORT_H_
#define AVP_PORT_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <AVP_LIBS/General/Math.h>
#include <AVP_LIBS/General/Error.h>
#include "CircBuffer.h"

#define AVP_PORT_DEF_RX_BUF_SIZE 7
#define AVP_PORT_DEF_TX_BUF_SIZE 7
#define AVP_PORT_DEF_BLK_BUF_SIZE 4

namespace avp {
  //! @note this template should not be instatiated - use either PortByteTX or PortBlockTX
  //! @tparam HW_IO_ - hardware communication class which provides this as in the header description above
  //! @tparam tSize - type of CircBuffer counter, should be big enough to fit all buffer sizes.
  //! @tparam ESC_code - this is special byte code placed in BufferTX to indicate that we
  //! should transmit block here. If next byte is ESC_code we have to transmit block. If it is 0 we transmit just ECS_code
  //! the value is random, selected so it is not ASCII and there is nothing special about it
# define __PORT_TEMPLATE__ template<class HW_IO_, uint8_t Log2_TX_Buf_size=AVP_PORT_DEF_TX_BUF_SIZE, \
                                    uint8_t Log2_TX_BlockBufSize=AVP_PORT_DEF_BLK_BUF_SIZE, \
                                    uint8_t Log2_RX_Buf_Size=AVP_PORT_DEF_RX_BUF_SIZE, \
                                    typename tSize=uint8_t, uint8_t ESC_code = 224 /* random value */>

  __PORT_TEMPLATE__ struct  Port: public HW_IO_ {
    struct BlockInfo;
    typedef void (* tReleaseFunc)();
    struct BlockInfo {
      const uint8_t *Ptr;
      size_t Size;
      tReleaseFunc pReleaseFunc; //!< data pointed by Ptr should not get corrupted until this function is called
    }; // BlockInfo
    static uint8_t RunningCS;
    static uint16_t BytesTransmitted;
  protected:
    static CircBuffer<uint8_t, tSize, Log2_TX_Buf_size> BufferTX; // byte transmit buffer

    // ***************  data for unbuffered block transmit buffer
    static CircBuffer<BlockInfo, tSize, Log2_TX_BlockBufSize> BlockInfoBufTX; // Block transmit buffer
    // *************** things for receive buffer
    static CircBuffer<uint8_t, tSize, Log2_RX_Buf_Size> BufferRX; //  receive buffer ( we receive by byte only )
    static const uint8_t *pCurByteInBlock; //!< when we are currently reading from block it is read pointer

   /// @{
   /// @brief these are a callback functions which Port supplies to HW_IO class.

   //! this function is called from HW_IO interrupt handler to store received byte in the RX Circular buffer
    static bool StoreReceivedByte(uint8_t b) {
      if(!BufferRX.LeftToWrite()) return false;
      BufferRX.Write(b);
      return true;
    } // StoreReceivedByte


     /// This function is called from HW_IO interrupt handler to get byte from Circular buffer to send
    /// @note WRITING TO *p may immediately send byte, so do it ONLY ONCE !
    /// because this function may read both from byte buffer and block buffer it is
    /// complicated
    /// @param[out] p - pointer supplied by HW_IO to write the byte to
    static bool GetByteToSend(volatile uint8_t *p) {
      // debug_action();

      if(pCurByteInBlock != nullptr) { //  we are reading from block currently
        const BlockInfo *pCurBlock = BlockInfoBufTX.GetSlotToRead();

        if(++pCurByteInBlock == pCurBlock->Ptr + pCurBlock->Size) { // we are done with this block
          if(pCurBlock->pReleaseFunc != nullptr) (*pCurBlock->pReleaseFunc)();
          BlockInfoBufTX.FinishedReading();
          pCurByteInBlock = nullptr;
        } else {
          *p = *pCurByteInBlock; // send next byte from the block
          return true;
        }
      }
      if(!BufferTX.LeftToRead()) return false;
      else {
        uint8_t b = BufferTX.Read_(); // can not write to *p here, it will be transmitted at once, but
        // we gotta check for escape code first
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

    /** @brief HW_IO class uses this function to request data to write
        @note If HW_IO is capable to send data by blocks (say via DMA) we can use this function
        @note This function uses static variables - care should be taken to avoid this function being reentered!!!!!
        @note This is the only function reading BlockInfoBufTX and BufferTX
     *
     * @param[out] pp - pointer to pointer from where to send data. Pointer returned  by this parameter should be
        persistant enough to keep oin existing while HW_IO sends it out
     * @param[out] pSz - to return block size.
     * @return bool - whether there is anything to send
     *
     */
    static bool GetBlockToSend(const uint8_t **pp, size_t *pSz) {
      static bool LastSentIsBlock = false;  // if we got a block to send last time we've got to
      // do FinishedReading

      if(LastSentIsBlock) { // finalize after previous block
        const BlockInfo *pCurBlock = BlockInfoBufTX.GetSlotToRead(); // returns the same slot as last time
        if(pCurBlock->pReleaseFunc != nullptr) (*pCurBlock->pReleaseFunc)();
        BlockInfoBufTX.FinishedReading();
        LastSentIsBlock = false;
      }

      if(!BufferTX.LeftToRead()) return false;
      else {
        static uint8_t b; // pointer to this variable is sent outside of this function, make sure it does not disappear
        b = BufferTX.Read_();
        *pp = &b; *pSz = 1; // by default it is just a byte to send
        if(b == ESC_code) { // except when it is a special code
          // dealing with a special case here, either block or byte with ESC_Code value
          // in either case there should be BlockInfo allocated in the BlockInfoBufTX
          AVP_ASSERT(BlockInfoBufTX.LeftToRead());
          const BlockInfo *pCurBlock = BlockInfoBufTX.GetSlotToRead();
          if(pCurBlock->Size == 0) { // there is no really a block to send, just a byte with ESC_code value,
            // so we are done with this BlockInfo
            BlockInfoBufTX.FinishedReading();
          } else { //  and here is really a block
            *pp = BlockInfoBufTX.GetSlotToRead()->Ptr; // start sending block
            *pSz = BlockInfoBufTX.GetSlotToRead()->Size;
            LastSentIsBlock = true;
          }
        }
      }
      return true;
    } //  GetBlockToSend

    ///@}

    ///@{
    /// Functions for writing
    /// @{
    /// Low level functions all output goes through

    /**
     unbuffered unsafe write - no BufferTX check, no interrupt reenable
     @param Ptr - pointer to data block. data should keep on existing until function finishes sending them out and call pReleaseFunc
     @param Size - block size. Size == 0 is a special case, we create a fake BlockInfo entry,
      it just mean that we have to transmit ESC_code-valued byte
     @param pReleaseFunc - function to call when data are sent and may be released
    */
    static bool write_unbuffered_(const uint8_t *Ptr, size_t Size, tReleaseFunc pReleaseFunc = nullptr) {
      if(!BlockInfoBufTX.LeftToWrite()) return false;
      BlockInfo *pCurBlock = BlockInfoBufTX.GetSlotToWrite();
      if((pCurBlock->Size = Size) != 0) { // BlockInfo is real, not fake
        pCurBlock->Ptr = Ptr;
        pCurBlock->pReleaseFunc = pReleaseFunc;
      }
      BlockInfoBufTX.FinishedWriting();
      BufferTX.Write_(ESC_code); // do this afterwards, so transmit callback does not find ESC_code without block being in place
      RunningCS += avp::sum<uint8_t>(Ptr,Size);
      BytesTransmitted += Size;
      return true;
    } // write_unbuffered_

    //! unsafe write - no BufferTX check, no interrupt reenable
    /// @param d - byte to send
    static bool write_byte_(uint8_t d) {
      if(d == ESC_code) return write_unbuffered_(nullptr,0);
      BufferTX.Write_(d);
      RunningCS += d;
      BytesTransmitted++;
      return true;
    } // write_byte_
    /// @}

  public:
    // *************** TRANSMISSION FUNCTIONS ********************
    // ALL write function return false if buffer is overrun and true if OK
    //! safe writeBufferRX.
    static bool write_byte(uint8_t d) {
      if(!BufferTX.LeftToWrite()) return false;
      bool Res = write_byte_(d);
      HW_IO_::TryToSend(); // got something to transmit, reenable interrupt
      return Res;
    } // write_byte

    static bool write_char(int8_t d) { return write_byte((uint8_t)d); }

    //! buffered safe write
    static bool write(const uint8_t *Ptr, size_t Size) {
      if(Size == 0) return true; // that was easy
      if(Size > BufferTX.LeftToWrite()) return false;
      while(Size--) if(!write_byte_(*(Ptr++))) return false;
      HW_IO_::TryToSend(); // got something to transmit, reenable interrupt
      return true;
    }  // write_buffered

    // unbuffered safe write. Content of Ptr should be preserved until pReleaseFunc is called
    static bool write_unbuffered(const uint8_t *Ptr, size_t Size, tReleaseFunc pReleaseFunc = nullptr) {
      if(Size == 0) return true;
      if(!BufferTX.LeftToWrite()) return false;
      bool Res = write_unbuffered_(Ptr,Size,pReleaseFunc);
      HW_IO_::TryToSend();
      return Res;
    } // write_unbuffered
    //! @}

    static uint8_t GetRCS() { return RunningCS; } ///< get current rinning checksum
    static uint16_t GetNtransmitted() { return BytesTransmitted; } ///< get number of transmitted bytes since beginning of session

    // ********************************** RECEPTION *********************
    //! stores character by pointer pd, returns true if there is really a character to read
    static void FlushRX() { HW_IO_::FlushRX();  BufferRX.Clear(); }
    static bool SomethingToTX() { return BufferTX.LeftToRead() != 0; }

// *************** RECEPTION FUNCTIONS **************************
    static bool read(uint8_t *pd) { return BufferRX.Read(pd); }
    static bool SomethingToRX() { return BufferRX.LeftToRead() != 0; }
    static uint8_t GetByte() { uint8_t out; read(&out); return out; }
  }; // Port

// following defines are just to make static variables initiation code readable, no point in using them elsewhere

#define _TEMPLATE_DECL_ template<class HW_IO_, uint8_t Log2_TX_Buf_size, uint8_t Log2_TX_BlockBufSize, uint8_t Log2_RX_Buf_Size, \
                                 typename tSize, uint8_t ESC_code>

#define _TEMPLATE_SPEC_ Port<HW_IO_, Log2_TX_Buf_size,Log2_TX_BlockBufSize,Log2_RX_Buf_Size, tSize, ESC_code>

  _TEMPLATE_DECL_ const uint8_t *_TEMPLATE_SPEC_::pCurByteInBlock = nullptr;
  _TEMPLATE_DECL_ CircBuffer<uint8_t, tSize, Log2_TX_Buf_size> _TEMPLATE_SPEC_::BufferTX;
  _TEMPLATE_DECL_ CircBuffer<struct _TEMPLATE_SPEC_::BlockInfo, tSize, Log2_TX_BlockBufSize> _TEMPLATE_SPEC_::BlockInfoBufTX;
  _TEMPLATE_DECL_ CircBuffer<uint8_t, tSize, Log2_RX_Buf_Size> _TEMPLATE_SPEC_::BufferRX;
  _TEMPLATE_DECL_ uint8_t _TEMPLATE_SPEC_::RunningCS;
  _TEMPLATE_DECL_ uint16_t _TEMPLATE_SPEC_::BytesTransmitted = 0;

  /*! Port template for HW_IO which transmits by byte
   *!
   */
  __PORT_TEMPLATE__ struct  PortByteTX: public _TEMPLATE_SPEC_ {
    static void Init() { HW_IO_::SetCallBacks(_TEMPLATE_SPEC_::StoreReceivedByte,_TEMPLATE_SPEC_::GetByteToSend); }
  }; //  PortByteTX

  /*! Port template for HW_IO which transmits by block
   *!
   */
  __PORT_TEMPLATE__ struct  PortBlockTX: public _TEMPLATE_SPEC_ {
    static void Init() { HW_IO_::SetCallBacks(_TEMPLATE_SPEC_::StoreReceivedByte,_TEMPLATE_SPEC_::GetBlockToSend); }
  }; //  PortBlockTX

#undef _TEMPLATE_DECL_
#undef _TEMPLATE_SPEC_

} // avp

#endif /* AVP_PORT_H_ */

