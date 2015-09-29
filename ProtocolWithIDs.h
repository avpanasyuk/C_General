/**
  * @file
  * @author Alexander Panasyuk
  * @ see Protocol.h for protocol descrition. The speciality of this calss is that Commands are identified by
  * their index in CommandTable
  * avp::Protocol::Command_ avp::Protocol::CommandTable = {{Func,NumParamBytes},{Func,NumParamBytes},...}
  * should be defined elsewhere
  */

#ifndef PROTOCO_WITH_IDS_HPP_INCLUDED
#define PROTOCO_WITH_IDS_HPP_INCLUDED

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "Math.h"
#include "Error.h"
#include "Time.h"
#include "Port.h"
#include "IO.h"
#include "Protocol.h"

/// special error codes
// SpecErrorCodes field AVP_LIB/+AVP/serial_protocol.m class
// TODO("Restart transmitting BeaconStr when connection breaks. It is tough to determine when in happens, though.");

namespace avp {
  struct ProtocolCommand_ {
    CommandFunc_ CallBackFunc;
    uint8_t NumParamBytes;
  };

/// @tparam Port static class defined by template in AVP_LIBS/General/Port.h. We should call
///   proper Port::Init??? function
/// @tparam MaxSpecCode - return codes < 0 > -MaxSpecCode are considered special error
///   codes. The class itself is using only two codes - CS_ERROR and UART_OVERRUN
  template<class Port, uint8_t MaxSpecCode=2>
  class ProtocolWithIDs: public Protocol<Port,MaxSpecCode> {
  protected:
    static ProtocolCommand_ *CommandTable;
    static uint8_t NumCommands;

    static void ParseByte(uint8_t b) {
      static union Input_ {
        struct {
          uint8_t ID;
          uint8_t Params[MaxNumParamBytes];
        } Cmd;
        uint8_t Bytes[]; // command byte + param bytes
      } Input;
      /// @note COMMAND BYTE is index in CommandTable + 1.
      /// COMMAND BYTE == 0 is NOOP command

      Input.Bytes[InputI++] = b;

      if(InputI == 1) { // b == CurCommand.ID
        if(b) { // it is not a NOOP command
          if(b > NumCommands ||
              CommandTable[b-1].CallBackFunc == nullptr ) {
            return_error_printf("No such command!");
            FlushRX();
          }
        } else { // 0 is NOOP command, no checksum
          ReturnOK();
          InputI = 0;
        }
      } else if(InputI == CommandTable[Input.Cmd.ID-1].NumParamBytes + 2) { // we've got all parameter bytes
        // and a checksum
        if(sum<uint8_t>(Input.Bytes,InputI - 1) != b) {
          return_error_code(CS_ERROR);
          FlushRX();
        } else {
          CommandTable[Input.Cmd.ID-1].CallBackFunc(Input.Cmd.Params); // callback function should do return itself
          InputI = 0;
        }
      }
    } // ParseByte

  public:
    static void Init(const char *BeaconStr_, ProtocolCommand_ *pTable_, uint8_t NumCommands_) {
      Protocol<Port,MaxSpecCode>::Init(BeaconStr_);
      CommandTable = pTable_; NumCommands = NumCommands_;
    } // Init

    static void cycle() {
      if(Port::SomethingToRX()) {
        PortConnected = true;
        ParseByte(Port::GetByte());
      }

      if(Port::IsOverrun()) {
        FlushRX();
        return_error_code(UART_OVERRUN);
      }

      Protocol<Port,MaxSpecCode>::cycle();
    } //  cycle
  }; //class ProtocolWithIDs

// following defines are just for code clearness, do not use elsewhere
#define _TEMPLATE_DECL_ template<class Port, uint8_t MaxSpecCode>
#define _TEMPLATE_SPEC_ ProtocolWithIDs<Port, MaxSpecCode>
  _TEMPLATE_DECL_ uint8_t _TEMPLATE_SPEC_::InputI = 0;
  _TEMPLATE_DECL_ ProtocolCommand_ *_TEMPLATE_SPEC_::CommandTable;
  _TEMPLATE_DECL_ uint8_t _TEMPLATE_SPEC_::NumCommands;

#undef _TEMPLATE_DECL_
#undef _TEMPLATE_SPEC_
} // namespace avp

#endif /* SERIAL_PROTOCOL_HPP_INCLUDED */
