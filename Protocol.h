/**
  * @file
  * @author Alexander Panasyuk
  * @verbatim
    Protocol description.
    - all GUI->FW messages are commands:
      - 1 byte command id. It is index in CommandTable + 1.
      - parameter bytes
      - 1 byte of checksum of above bytes
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

/// special error codes
// SpecErrorCodes field AVP_LIB/+AVP/serial_protocol.m class
// TODO("Restart transmitting BeaconStr when connection breaks. It is tough to determine when in happens, though.");

namespace avp {
  typedef void (*CommandFunc_)(const uint8_t Params[]);

  struct ProtocolCommand_ {
    CommandFunc_ CallBackFunc;
    uint8_t NumParamBytes;
  };

/// @tparam Port static class defined by template in AVP_LIBS/General/Port.h. We should call
///   proper Port::Init??? function
/// @tparam millis global system function returning milliseconds
/// @tparam MaxSpecCode - return codes < 0 > -MaxSpecCode are considered special error
///   codes. The class itself is using only two codes - CS_ERROR and UART_OVERRUN
/// @tparam  CommandTable - pointer to command table. COMMAND ID is THE INDEX IN THIS TABLE + 1, COMMAND ID 0 corresponds to NOOP
/// @tparam TableSize - size of command table
  template<class Port, uint8_t MaxSpecCode=2>
  class Protocol: public Port {
    protected:
      enum SpecErrorCodes {CS_ERROR=1,UART_OVERRUN};
      static_assert(MaxSpecCode >= 2,"Class uses 2 codes itself");
      static bool PortConnected;
      static const char *BeaconStr;
      static ProtocolCommand_ *CommandTable;
      static uint8_t NumCommands;

      static constexpr uint8_t MaxNumParamBytes = 100;

      static union Input_ {
        struct {
          uint8_t ID;
          uint8_t Params[MaxNumParamBytes];
        } Cmd;
        uint8_t Bytes[]; // command byte + param bytes
      } Input;
      /// @note COMMAND BYTE is index in CommandTable + 1.
      /// COMMAND BYTE == 0 is NOOP command
      static uint8_t InputI; //!< this pointer traces position of the input stream in InputBytes

      /// base private message which writes both info and error messages
      static inline bool info_message_(const uint8_t *Src, int8_t Size) {
        AVP_ASSERT(Port::write_char(Size));
        AVP_ASSERT(Port::write(Src,Size));
        AVP_ASSERT(Port::write_byte(sum<uint8_t>(Src,Size)));
        return true;
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

        //if(PadSize) {
        //avp::Vector<uint8_t,MaxSpecCode> Pad; Pad = 0;
        //AVP_ASSERT(write_<Port::write>::array(Pad.get_ptr(),PadSize));
        //}
        static uint8_t Pad[MaxSpecCode];
        if(PadSize) AVP_ASSERT(write_<Port::write>::array(Pad,PadSize));
      } // error_message

      static void FlushRX() {
        // read the rest if something is transmitted
        uint8_t b;
        do {
          Port::FlushRX();
          // wait if something appears
          uint32_t WaitUntil = millis() + 3;
          while(WaitUntil - millis() > UINT32_MAX/2); // 10 millis delay
        } while(Port::read(&b));
      } // FlushRX

    public:
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
      static void return_error_code(int8_t Code) {
        AVP_ASSERT(Code <= MaxSpecCode);
        AVP_ASSERT(Port::write_char(-Code));
        AVP_ASSERT(Port::write_char(-Code)); // checksum which is equal to error code
      } //  return_error_code

      static void ParseByte(uint8_t b) {
        Input.Bytes[InputI++] = b;

        if(InputI == 1) { // b == CurCommand.ID
          if(b) { // it is not a NOOP command
            if(b >= NumCommands ||
                CommandTable[b].CallBackFunc == nullptr ) {
              return_error_printf("No such command!");
              FlushRX();
              InputI = 0;
            }
          } else  InputI = 0; // 0 is NOOP command
        } else if(InputI == CommandTable[Input.Cmd.ID].NumParamBytes + 2) { // we've got all parameter bytes
          // and a checksum
          if(sum<uint8_t>(Input.Bytes,InputI - 1) != b) {
            return_error_code(CS_ERROR);
            FlushRX();
          } else CommandTable[Input.Cmd.ID].CallBackFunc(Input.Cmd.Params); // callback function should do return itself
          InputI = 0;
        }
      } // ParseByte

      //! if port is disconnected run beacon, which allows GUI to find our serial port
      static void SendBeacon() { if(!PortConnected) write_<Port::write>::string(BeaconStr); }
    public:
      static void Init(const char *BeaconStr_, ProtocolCommand_ *pTable_, uint8_t NumCommands_) {
        BeaconStr = BeaconStr_; CommandTable = pTable_; NumCommands = NumCommands_;
      } // Init

      static void cycle() {
        static RunPeriodically<millis,SendBeacon,500> BeaconTicker;

        BeaconTicker.cycle();

        if(Port::SomethingToRX()) {
          PortConnected = true;
          ParseByte(Port::GetByte());
        }

        if(Port::IsOverrun()) {
          FlushRX();
          return_error_code(UART_OVERRUN);
          InputI = 0;
        }
      } //  cycle

      static void ReturnBytesBuffered(const uint8_t *src, uint16_t size) {
        AVP_ASSERT(Port::write_byte(0)); // status
        AVP_ASSERT(write_<Port::write>::object(size));
        AVP_ASSERT(Port::write(src, size));
        AVP_ASSERT(Port::write_byte(sum<uint8_t>((const uint8_t *)src,size)));
      } // Protocol::ReturnBytesBuffered
      static void ReturnBytesUnbuffered(const uint8_t *src, uint16_t size, typename Port::tReleaseFunc pFunc = nullptr)  {
        AVP_ASSERT(Port::write_byte(0)); // status
        AVP_ASSERT(write_<Port::write>::object(size));
        AVP_ASSERT(Port::write_unbuffered(src, size, pFunc));
        AVP_ASSERT(Port::write_byte(sum<uint8_t>((const uint8_t *)src,size)));
      } // Protocol::ReturnBytesUnbuffered;

      template<typename T> static void ReturnBuffered(const T *pT, uint16_t size = 1) {
        ReturnBytesBuffered((const uint8_t *)pT,sizeof(T)*size);
      } // ReturnUnbuffered
      template<typename T> static void ReturnUnbuffered(const T *pT, uint16_t size = 1, typename Port::tReleaseFunc pFunc = nullptr) {
        ReturnBytesUnbuffered((const uint8_t *)pT,sizeof(T)*size,pFunc);
      } // ReturnUnbuffered
      template<typename T> static void Return(T x) {
        ReturnBytesBuffered((const uint8_t *)&x,sizeof(T));
      } // Return
      static void ReturnOK() {
        AVP_ASSERT(write_<Port::write>::object(uint32_t(0)));  // 1 byte status, 2 - size and 1 - checksum
      }
  }; //class Protocol

// following defines are just for code clearness, do not use elsewhere
#define _TEMPLATE_DECL_ template<class Port, uint8_t MaxSpecCode>
#define _TEMPLATE_SPEC_ Protocol<Port, MaxSpecCode>

  _TEMPLATE_DECL_ bool _TEMPLATE_SPEC_::PortConnected = false;
  /// @note CommandTable should be specified elsewhere
  _TEMPLATE_DECL_ typename _TEMPLATE_SPEC_::Input_ _TEMPLATE_SPEC_::Input;
  _TEMPLATE_DECL_ uint8_t _TEMPLATE_SPEC_::InputI = 0;
  _TEMPLATE_DECL_ const char *_TEMPLATE_SPEC_::BeaconStr;
  _TEMPLATE_DECL_ ProtocolCommand_ *_TEMPLATE_SPEC_::CommandTable;
  _TEMPLATE_DECL_ uint8_t _TEMPLATE_SPEC_::NumCommands;

#undef _TEMPLATE_DECL_
#undef _TEMPLATE_SPEC_
} // namespace avp

#endif /* SERIAL_PROTOCOL_HPP_INCLUDED */
