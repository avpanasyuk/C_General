/**
  * @file
  * @author Alexander Panasyuk
  * @brief Implements only function identical between ProtocolWithIDs and ProtocolWithMnemonics
  *        DO NOT INSTANTIATE THING CLASS BY ITSELF
  * [PROTOCOL]
    Protocol description.
    - all GUI->FW messages are commands. Command is sequence of bytes which contains:
      - Either 1 byte command ID (if InputParser is CommandTable) or mnemonics ( if InputParser is CommandChain)
      - if NumParamBytes == -1 in correspong Command_ structure:
        - a byte giving the number of following parameter bytes parameter bytes.
      - Parameter bytes. Their number is given either by NumParamBytes field in Command_ structure (if it is not -1) or previous byte
        if NumParamBytes == -1
      - 1 byte of checksum of the bytes above
    - FW-GUI messages are formatted in blocks, each one starts with int8_t CODE and ends with checksum. Block content may be:
        - command return:
          - CODE byte
          - If CODE is 0 the following is successful latest command return:
            - typical command
              - uint16_t Size is size of data being transmitted.
              - data
              - 1 uint8_t data checksum
            - SOME COMMANDS WHICH RETURN LARGE DATA VOLUMES MAY USE DIFFERENT RETURN SEQUENCE
          - If CODE is < 0, then
            - if
                CODE == -1, command block is received with checksum error, command was never executed
                and should be resent
                or
                CODE == -2, port RX is overrun, communication should be flushed and command was never executed
                and should be resent
              - checksum byte which is equal to CODE
            - if CODE < -2, command was properly received but failed. CODE it is the error message size
              - error message of size -CODE (no trailing 0)
              - 1 uint8_t error message text checksum
        - info message
        - If CODE is > 0, then it is an info message size, followed by
          - info message text without trailing 0
          - 1 uint8_t info message text checksum
          .
        Info message block may come at any time, but not inside another block
      If error or info message do not fit into 127 bytes remaining text is formatted into consecutive  info message(s) is
  * [PROTOCOL]
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
/// @tparam Port static class defined by template in AVP_LIBS/General/Port.h. We should call
///   proper Port::Init??? function
/// @tparam InputParser - class which provides ParseByte and Flush commands. Former parses input byte stream,
///   finding commands and parameters and executing them and latter flushes it if something goes wrong.
///   Subclass of CommandParser, currently either CommandChain or CommandTable
  template<class Port, class InputParser>
  class Protocol: public Port {
    protected:
      enum ErrorCodes_ { CS_ERROR = 1, PORT_OVERRUN };
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
        if(Size <= PORT_OVERRUN) Size += (PadSize = PORT_OVERRUN-Size+1);

        AVP_ASSERT(Port::write_char(-Size));
        AVP_ASSERT(Port::write(Src,Size));
        AVP_ASSERT(Port::write_byte(sum<uint8_t>(Src,Size)));

        static uint8_t Pad[PORT_OVERRUN];
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
          debug_printf("Port overrun!");
          return_error_code(PORT_OVERRUN);
          FlushRX();
          InputParser::Flush();
        } else if(SomethingToRX()) {
          int8_t ErrCode;
          switch(ErrCode = InputParser::ParseByte(Port::GetByte())) {
            case InputParser::WRONG_ID:
              return_error_printf("Command is not defined!\n");
              break;
            case InputParser::BAD_CHECKSUM:
              return_error_code(CS_ERROR);
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

      /// returns code which indicated that command was not received and has to be resent
      static bool return_error_code(int8_t Code) {
        AVP_ASSERT(Code <= PORT_OVERRUN);
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
      static void Return(const type &X) {
        AVP_ASSERT(ReturnBytesBuffered((const uint8_t *)&X,sizeof(X)));
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
#define _TEMPLATE_DECL_ template<class Port, class InputParser>
#define _TEMPLATE_SPEC_ Protocol<Port, InputParser>

  _TEMPLATE_DECL_ bool _TEMPLATE_SPEC_::PortConnected = false;
  _TEMPLATE_DECL_ const char *_TEMPLATE_SPEC_::BeaconStr;

#undef _TEMPLATE_DECL_
#undef _TEMPLATE_SPEC_
} // namespace avp

#endif /* SERIAL_PROTOCOL_HPP_INCLUDED */
