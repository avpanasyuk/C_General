/**
  @file
  @author Alexander Panasyuk

  @page ProtocolDescription GUI<->FW Communication Protocol Description
    - all GUI->FW messages are commands. Command is a sequence of bytes which contains:
      + either a single byte command ID (if InputParser is avp::CommandTable) or mnemonics ( if InputParser is avp::CommandChain)
      + a byte giving the number of following parameter bytes. It is present only for commands that take a variable number of Parameter bytes
        which is indicated by parameter NumParamBytes being equal to VAR_PARAM_NUM  in corresponding AddCommand call.
      + Parameter bytes. Their number is given either by corresponding NumParamBytes parameter in AddCommand call (if it is >=0) or
        by the previous byte if NumParamBytes == VAR_PARAM_NUM
      + one byte of checksum of the bytes above
    - FW<-GUI messages are formatted in blocks. Block starts with a single byte CODE, and then either
      -# successful command return, indicated by CODE equal 0. In this case following are:
        + uint16_t Size is the size of data being transmitted.
        + data
        + one uint8_t data checksum
      -# unsuccessful command error message, indicated by CODE < 0, then
        + if CODE == -1, the command block was received with a checksum error, so the command was never executed
            and should be resent. A checksum byte which is equal to CODE follows.
        + if CODE == -2, port RX is overrun, the command was never executed, communication should be flushed and
            the command should be resent. A checksum byte which is equal to CODE follows.
        + if CODE < -2, command was properly received but failed. -CODE value is the error message size. In this case following are:
          - error message text of size -CODE (no trailing 0)
          - one uint8_t error message text checksum
      -# an info message, indicated by CODE is > 0 which represents info message size, followed by
        + info message text without trailing 0
        + one info message text checksum
        .
        Info message block may come at any time, but not inside another return block
      .
      If error or info message do not fit into 127 bytes remaining text is formatted into consecutive info message(s).
    .

  @section StartHandshake Initial Handshake.
  There may be several physical and virtual COM ports in the system, and we got to determine which
  one device is connected to. This class supports automatic port finding. It sends out BeaconStr
  every time avp::Protocol::SendBeacon is called (which should be done continuously before connection). The program on the
  opposite end of connection should
    -# (optional) if port number is unknown, checks every COM port until it finds one continuously transmitting BeaconStr.
    -# leave the transmitting port open (or reopen it)
    -# sends NOOP command (which a single 0 byte) to the port
    -# immediately start monitoring incoming stream on the presence of four 0 byte sequence
    -# on reception of NOOP command this FW stops sending BeaconStr to port and
      responds to it in a standard way according to \ref ProtocolDescription "Protocol Description",
      sending out four 0 bytes - one for status, two for size and the last one as checksum.
    -# when the communicating program receives four 0 byte sequence it should continue communication
      using \ref ProtocolDescription "Protocol Description".

  @section Resynchronization Resynchronization
  If a byte or two got lost communication may hang, because e.g. the FW may wait for missing parameter bytes, so
  it would not process the command and send out return.
  To resync the protocol:
    - communicating program should start sending NOOPs (single 0 byte) one by one.
    - eventually FW will receive all bytes it was waiting for (though they would not be correct bytes to properly execute previous command),
    - FW will return "bad checksum" error status (because parameter bytes would be wrong)
    - FW will starts responding with four 0 byte on every NOOP received.
    - as soon communicating program receives the first four 0 bytes return it should stop sending NOOPs
    - communicating program should read out all 0 bytes until there are no more
    - when it happens the protocol is resynchronized.

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
  /**
  @tparam Port static class defined by template in AVP_LIBS/General/Port.h. We should call
  proper avp::Port::Init() function
  @tparam InputParser - class which provides ParseByte and Flush commands. Former parses input byte stream,
  finding commands and parameters and executing them and latter flushes it if something goes wrong.
  Subclass of CommandParser, currently either CommandChain or CommandTable
  */
  template<class Port, class InputParser, uint16_t BeaconPeriod, void (*ConnectFunc)() = nullptr, void (*DropFunc)() = nullptr>
  class Protocol: public Port {
    protected:
      enum ErrorCodes_ {CS_ERROR = 1, PORT_OVERRUN};
      static bool PortConnected;
      static const char *BeaconStr;

      /// base private message which writes both info and error messages
      /// @param Src - string to output
      /// @param Size - int8_t size of string
      /// @param NonVolat - bool, true if string is static until sent, false by default
      static bool info_message_(const uint8_t *Src, int8_t Size, bool NonVolat = false) {
        return Port::write_char(Size) &&
               (NonVolat?Port::write_unbuffered(Src,Size):Port::write(Src,Size)) &&
               Port::write_byte(sum<uint8_t>(Src,Size));
      } // info_message_

      /// sends error message, checking whether we need padding
      /// @param Src - string to output
      /// @param Size - int8_t size of string
      /// @param NonVolat - bool, true if string is static until sent, false by default
      static void error_message_(const uint8_t *Src, int8_t Size, bool NonVolat = false) {
        AVP_ASSERT(Size >= 0);
        // Because Sizes <= PORT_OVERRUN are special error codes we can not have a
        // text error message with this Size. So if it does occur we pad it with some empty
        // bytes
        uint8_t PadSize = 0;
        if(Size <= PORT_OVERRUN) Size += (PadSize = PORT_OVERRUN-Size+1);

        AVP_ASSERT(Port::write_char(-Size));
        if(NonVolat) AVP_ASSERT(Port::write_unbuffered(Src,Size));
        else AVP_ASSERT(Port::write(Src,Size));

        static uint8_t Pad[] = {' ',' '};
        static_assert(sizeof(Pad) == PORT_OVERRUN, "Adjust Pad initialization if PORT_OVERRUN changes!");

        if(PadSize) AVP_ASSERT(Port::write_unbuffered(Pad,PadSize));

        AVP_ASSERT(Port::write_byte(sum<uint8_t>(Src,Size) + sum<uint8_t>(Pad,PadSize)));
      } // error_message

      /// flashing serial port input
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

      static inline bool IsConnected() { return PortConnected; }

      /// returns code which indicated that command was not received and has to be resent
      static bool return_error_code(int8_t Code) {
        AVP_ASSERT(Code <= PORT_OVERRUN);
        return Port::write_char(-Code) && Port::write_char(-Code); // checksum which is equal to error code
      } //  return_error_code

      /**
      @brief this function should be called repeatedly as a part of main program loop to maintain communication
      @callgraph
      */
      static void cycle() {
        RunPeriodically<millis,SendBeacon,BeaconPeriod>::cycle();

        if(Port::IsOverrun()) {
          debug_printf("Port overrun!");
          return_error_code(PORT_OVERRUN);
          FlushRX();
          InputParser::Flush();
        } else if(SomethingToRX()) {
          switch(InputParser::ParseByte(Port::GetByte())) {
            case InputParser::WRONG_ID:
              return_error_str("Command is not defined!\n");
              FlushRX();  // Oops
              break;
            case InputParser::WRONG_PARAM_SIZE:
              return_error_str("Too many parameter bytes!\n");
              FlushRX();  // Oops
              break;
            case InputParser::BAD_CHECKSUM:
              return_error_code(CS_ERROR);
              FlushRX();  // Oops
              break;
            case InputParser::NO_ERROR: break;
            case InputParser::NOOP: ReturnOK(); break;
            default: AVP_ERROR(0,"Unrecognized error code.");
          }
        }
        Port::TryToSend();
      } //  cycle

      /// split info message into proper chunks
      /// @param Src - byte array to output
      /// @param Size - size_t size of array
      /// @param NonVolat - bool, true if the array would not disappear until sent in background
      static bool info_message(const uint8_t *Src, size_t Size, bool NonVolat) {
        while(Size > INT8_MAX)  {
          if(!info_message_(Src,INT8_MAX,NonVolat)) return false;
          Src += INT8_MAX;
          Size -= INT8_MAX;
        }
        return info_message_(Src,Size,NonVolat);
      } // info_message

      /// @note - I do not use default parameters in the previous function because it screws templates
      /// @param Src - byte array to output
      /// @param Size - size_t size of array
      static inline bool info_message(const uint8_t *Src, size_t Size) { return info_message(Src,Size,false); }

      static inline bool info_str(const char *s, bool NonVolat = true) {
        return info_message((const uint8_t *)s,strlen(s),NonVolat);
      } // info_str

      static PRINTF_WRAPPER(info_printf, vprintf<info_message>)

      /// sends error message of any size, splits if necessary into chunks
      /// @param Src - byte array to output
      /// @param Size - size_t size of array
      /// @param NonVolat - bool, true if the array would not disappear until sent in background
      static bool return_error_message(const uint8_t *Src, size_t Size, bool NonVolat) {
        if(Size > INT8_MAX) {
          error_message_(Src,INT8_MAX,NonVolat);
          info_message(Src+INT8_MAX,Size-INT8_MAX,NonVolat);
        } else error_message_(Src,Size,NonVolat);
        return true;
      } // return_error_message

      /// @note - I do not use default parameters because it screws templates
      /// @param Src - byte array to output
      /// @param Size - size_t size of array
      static inline bool return_error_message(const uint8_t *Src, size_t Size) {
        return return_error_message(Src,Size,false);
      } // return_error_message

      /// sends error message of any size, splits if necessary into chunks
      /// @param ErrMsg - string to output
      /// @param NonVolat - bool, true if string would not disappear until sent in background, true by default
      /// @note NonVolat is different here than in other similar functions, because
      /// this function is used on static strings most of the time
      static bool return_error_str(const char* ErrMsg, bool NonVolat = true) {
        return return_error_message((const uint8_t *)ErrMsg, strlen(ErrMsg), NonVolat);
      } // return_error_str

      static PRINTF_WRAPPER(return_error_printf, vprintf<return_error_message>)

      static bool ReturnBytesBuffered(const uint8_t *src, size_t size) {
        return Port::write_byte(0) &&
               write_buffered<Port::write>::object((uint16_t)size) &&
               Port::write(src, size) &&
               Port::write_byte(sum<uint8_t>(src,size));
      } // Protocol::ReturnBytesBuffered
      static bool ReturnBytesUnbuffered(const uint8_t *src, size_t size, typename Port::tReleaseFunc pFunc = nullptr)  {
        return Port::write_byte(0) &&
               write_buffered<Port::write>::object((uint16_t)size) &&
               Port::write_unbuffered(src, size, pFunc) &&
               Port::write_byte(sum<uint8_t>(src,size));
      } // Protocol::ReturnBytesUnbuffered;

#define RET_IF_FALSE(exp) do{if(!(exp)) return false;}while(0)

      /** this function allows us to return several blocks with one call
      it takes variable number of arguments in triplets or quadruplets
      all integer values get promoted to "int" in variable list, so that's the only type
      we can use
      (const uint8_t *src, int numbytes, int Buffered[, typename Port::tReleaseFunc pFunc])
      pFunc is present only when Buffered is false
      the end of the list is marked by src == nullptr. numbytes and Buffered are absent
      */
      static bool ReturnMultiByPtrs(const void *src, int numbytes, int Buffered, ...) {
        va_list ap, ap_size;
        uint16_t total_bytes = numbytes;
        bool IsBuf = Buffered;

        va_start(ap, Buffered);
        // first pass through the arguments to calculate full size
        va_copy(ap_size, ap);

        while(1) {
          if(!IsBuf) va_arg(ap_size, typename Port::tReleaseFunc);
          // lets pull the next tri/quadro/plet
          if(va_arg(ap_size, const uint8_t *) == nullptr) break;
          total_bytes += va_arg(ap_size, int);
          IsBuf = va_arg(ap_size, int);
        }
        va_end(ap_size);

        // sending
        RET_IF_FALSE(Port::write_byte(0)); // status OK
        RET_IF_FALSE(write_buffered<Port::write>::object(total_bytes));

        uint8_t cs = 0;

        // now send data
        while(1) {
          if(!Buffered)
            RET_IF_FALSE(Port::write_unbuffered((const uint8_t *)src, numbytes, va_arg(ap, typename Port::tReleaseFunc)));
          else
            RET_IF_FALSE(Port::write((const uint8_t *)src,numbytes));
          cs += sum<uint8_t>(src,numbytes);

          // lets pull the next tri/quadro/plet
          if((src = va_arg(ap, const uint8_t *)) == nullptr) break;
          numbytes = va_arg(ap, int);
          Buffered = va_arg(ap, int);
        }
        va_end(ap);

        RET_IF_FALSE(Port::write_byte(cs));

        return true;
      } // ReturnMultiByPtrs

      static void ReturnOK() {
        AVP_ASSERT(write_buffered<Port::write>::object(uint32_t(0)));  // 1 byte status, 2 - size and 1 - checksum
        // debug_printf("Four zeros\n");
      }

      IGNORE(-Waddress)

      static bool SomethingToRX() {
        if(Port::SomethingToRX()) {
          if(!PortConnected) {
            PortConnected = true;
            if(ConnectFunc != nullptr) (*ConnectFunc)();
          }
          return true;
        } else return false;
      } // SomethingToRX

      static bool SomethingToTX() { return Port::SomethingToTX(); }

      // some useful templates
      template<typename type>
      static void Return(const type &X) {
        AVP_ASSERT(ReturnBytesBuffered((const uint8_t *)&X,sizeof(X)));
      }
      // some useful templates
      template<typename type>
      static void ReturnUnbuffered(const type &X, typename Port::tReleaseFunc pFunc = nullptr) {
        AVP_ASSERT(ReturnBytesUnbuffered((const uint8_t *)&X,sizeof(X),pFunc));
      }
      template<typename type>
      static void ReturnByPtr(const type *p, size_t size=1) {
        AVP_ASSERT(write_buffered<ReturnBytesBuffered>::array(p,size));
      }

      // We do not have to do ReturnUnbuffered, there always should be pointer
      template<typename type>
      static void ReturnUnbufferedByPtr(const type *p, size_t size=1, typename Port::tReleaseFunc pFunc = nullptr) {
        AVP_ASSERT(write_unbuffered<ReturnBytesUnbuffered>::array(p,size,pFunc));
      }
  }; //class Protocol

// following defines are just for code clearness, do not use elsewhere
#define _TEMPLATE_DECL_ template<class Port, class InputParser, uint16_t BeaconPeriod, void (*ConnectFunc)(), void (*DropFunc)()>
#define _TEMPLATE_SPEC_ Protocol<Port, InputParser, BeaconPeriod, ConnectFunc, DropFunc>

  _TEMPLATE_DECL_ bool _TEMPLATE_SPEC_::PortConnected = false;
  _TEMPLATE_DECL_ const char *_TEMPLATE_SPEC_::BeaconStr;

#undef _TEMPLATE_DECL_
#undef _TEMPLATE_SPEC_
} // namespace avp

#endif /* SERIAL_PROTOCOL_HPP_INCLUDED */
