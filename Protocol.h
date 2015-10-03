/**
  * @file
  * @author Alexander Panasyuk
  * @brief Implements only function identical between ProtocolWithIDs and ProtocolWithMnemonics
  *        DO NOT INSTANTIATE THING CLASS BY ITSELF
  * @verbatim
    Protocol description.
    - all GUI->FW messages are commands:
      - Either command ID or mnemonics.
      - parameter bytes
      - 1 byte of checksum of the bytes above
    - FW-GUI messages are differentiated  based on the first int8_t CODE
        - command return:
          - If CODE is 0 the following is successful latest command return:
            - uint16_t Size is size of data being transmitted.
            - data
            - 1 uint8_t data checksum
          - If CODE is < 0, then it is the last command failure return message:
            - CODE >= -MaxSpecCode, @see enum SpecErrorCodes. Special errro codes start
              predefined actions in GUI
            - checksum byte which is equal to CODE
           - if CODE < -MaxSpecCode it is the error message size, followed by
              - error message text without trailing 0
              - 1 uint8_t error message text checksum
              .
        Every command has to be responded with either successful return or error message, and only
        one of them.
        - If CODE is > 0, then it is an info message size, followed by
          - info message text without trailing 0
          - 1 uint8_t info message text checksum
          .
        Info messages may come at any time
      If error or info message do not fit into 127 bytes remaining text is formatted into consecutive  info message(s) is
  * @endverbatim
  * @date 9/21/15 instead of dynamically creating commands let's create a static table
  * avp::Protocol::Command_ avp::Protocol::CommandTable = {{Func,NumParamBytes},{Func,NumParamBytes},...}
  * should be defined elsewhere
  */

#ifndef COMMAND_PROTOCOL_HPP_INCLUDED
#define COMMAND_PROTOCOL_HPP_INCLUDED

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "Math.h"
#include "Error.h"
#include "Time.h"
#include "Port.h"
#include "IO.h"
#include "CommandParser.h"

namespace avp {
  enum ProtocolSpecCodes {UART_OVERRUN=CommandParser::NUM_ERRORS, NUM_ERR_CODES};

/// @tparam Port static class defined by template in AVP_LIBS/General/Port.h. We should call
///   proper Port::Init??? function
/// @tparam InputParser - class which provides ParseByte and Flush commands. Former parses input byte stream,
///   finding commands and parameters and executing them and latter flushes it if something goes wrong.
///   Subclass of CommandParser.
/// @tparam MaxSpecCode - return codes < 0 > -MaxSpecCode are considered special error
///   codes. The InputParser is using only one code - CS_ERROR and
///   Protocol is using UART_OVERRUN
  template<class Port, class InputParser, uint8_t MaxSpecCode=NUM_ERR_CODES>
  class Protocol: public Port {
  protected:

    static_assert(MaxSpecCode >= NUM_ERR_CODES,"Not less then number of codes already used");
    static bool PortConnected;
    static const char *BeaconStr;

    /// base private message which writes both info and error messages
    static inline bool info_message_(const uint8_t *Src, int8_t Size) {
      return Port::write_char(Size) &&
      Port::write(Src,Size) &&
      Port::write_byte(sum<uint8_t>(Src,Size));
    } // info_message_

    /// sends error message, checking whether we need padding
    /// @param Size - positive
    static inline void error_message_(const uint8_t *Src, int8_t Size) {
      AVP_ASSERT(Size > 0);
      uint8_t PadSize = 0;
      if(Size <= MaxSpecCode) Size += (PadSize = MaxSpecCode-Size+1);

      AVP_ASSERT(Port::write_char(-Size));
      AVP_ASSERT(Port::write(Src,Size));
      AVP_ASSERT(Port::write_byte(sum<uint8_t>(Src,Size)));

      static uint8_t Pad[MaxSpecCode];
      if(PadSize) AVP_ASSERT(write_buffered<Port::write>::array(Pad,PadSize));
    } // error_message

    static void FlushRX() {
      // read the rest if something is transmitted
      Millisec::Pause(10); // for uart to finish
      uint8_t b;
      do {
        Port::FlushRX();
        // wait if something appears
        uint32_t WaitUntil = millis() + 3;
        while(WaitUntil - millis() > UINT32_MAX/2); // 10 millis delay
      } while(Port::read(&b));
      InputParser::Flush();
    } // FlushRX

  public:
    static void Init(const char *BeaconStr_) { BeaconStr = BeaconStr_; }
    //! if port is disconnected run beacon, which allows GUI to find our serial port
    static void SendBeacon() { if(!PortConnected) write_buffered<Port::write>::string(BeaconStr); }

    static void cycle() {
      static RunPeriodically<millis,SendBeacon,100> BeaconTicker;

      BeaconTicker.cycle();

      if(Port::IsOverrun()) {
        debug_printf("Serial port overrun!");
        return_error_code(UART_OVERRUN);
        FlushRX();
      } else if(SomethingToRX()) {
        int8_t ErrCode;
        switch(ErrCode = InputParser::ParseByte(Port::GetByte())) {
          case InputParser::WRONG_ID:
          case InputParser::BAD_CHECKSUM:
            return_error_code(ErrCode);
            FlushRX();  // Oops
          case InputParser::NO_ERROR: break;
          case InputParser::NOOP: ReturnOK(); break;
          default: AVP_ERROR("Unrecognized error code.");
        }
      }
    } //  cycle

    /// split info message into proper chunks
    static bool info_message(const uint8_t *Src, size_t Size) {
      while(Size > INT8_MAX)  {
        info_message_(Src,INT8_MAX);
        Src += INT8_MAX;
        Size -= INT8_MAX;
      }
      info_message_(Src,Size);
      return true;
    } // info_message

    static PRINTF_WRAPPER(info_printf, vprintf<info_message>)

    static bool return_error_message(const uint8_t *Src, size_t Size) {
      if(Size > INT8_MAX) {
        error_message_(Src,INT8_MAX);
        info_message(Src+INT8_MAX,Size-INT8_MAX);
      } else error_message_(Src,Size);
      return true;
    } // return_error_message

    static PRINTF_WRAPPER(return_error_printf, vprintf<return_error_message>)

    /// returns special error code which has predefined action in GUI
    static bool return_error_code(int8_t Code) {
      AVP_ASSERT(Code <= MaxSpecCode);
      return Port::write_char(-Code) && Port::write_char(-Code); // checksum which is equal to error code

    } //  return_error_code

    static bool ReturnBytesBuffered(const uint8_t *src, size_t size) {
      return Port::write_byte(0) &&
      write_buffered<Port::write>::object((uint16_t)size) &&
      Port::write(src, size) &&
      Port::write_byte(sum<uint8_t>((const uint8_t *)src,size));
    } // Protocol::ReturnBytesBuffered
    static bool ReturnBytesUnbuffered(const uint8_t *src, size_t size, typename Port::tReleaseFunc pFunc = nullptr)  {
      return Port::write_byte(0) &&
      write_buffered<Port::write>::object((uint16_t)size) &&
      Port::write_unbuffered(src, size, pFunc) &&
      Port::write_byte(sum<uint8_t>((const uint8_t *)src,size));
    } // Protocol::ReturnBytesUnbuffered;

    static void ReturnOK() {
      AVP_ASSERT(write_buffered<Port::write>::object(uint32_t(0)));  // 1 byte status, 2 - size and 1 - checksum
    }

    static bool SomethingToRX() {
      if(Port::SomethingToRX()) {
        PortConnected = true;
        return true;
      } else return false;
    }
    // some useful templates
    template<typename type>
    static void Return(type X) {
      AVP_ASSERT(avp::write_buffered<ReturnBytesBuffered>::object(X));
    }
    template<typename type>
    static void ReturnByPtr(const type *p, size_t size=1) {
      AVP_ASSERT(avp::write_buffered<ReturnBytesBuffered>::array(p,size));
    }

    // We do not have to do ReturnUnbuffered, there always should be pointer
    template<typename type>
    static void ReturnUnbufferedByPtr(const type *p, size_t size=1) {
      AVP_ASSERT(avp::write_unbuffered<ReturnBytesUnbuffered>::array(p,size));
    }
  }; //class Protocol

// following defines are just for code clearness, do not use elsewhere
#define _TEMPLATE_DECL_ template<class Port, class InputParser, uint8_t MaxSpecCode>
#define _TEMPLATE_SPEC_ Protocol<Port, InputParser, MaxSpecCode>

  _TEMPLATE_DECL_ bool _TEMPLATE_SPEC_::PortConnected = false;
  _TEMPLATE_DECL_ const char *_TEMPLATE_SPEC_::BeaconStr;

#undef _TEMPLATE_DECL_
#undef _TEMPLATE_SPEC_
} // namespace avp

#endif /* SERIAL_PROTOCOL_HPP_INCLUDED */
