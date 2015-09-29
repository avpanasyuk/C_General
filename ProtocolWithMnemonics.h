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

/// unidirectional command chain
class CommandChain {
   // check for a new command
    static struct InputBytes_ {
      union {
        uint8_t Start[];
        uint32_t ID;
      };
      uint8_t Params[MaxParamBytes+1]; ///< really checksum is in Params[pCur->NumParamBytes]
    } InputBytes;

    static uint8_t *pInputByte; ///< tracks byte number of current command packet

// ****************************************************
  static class Command {
//  public:
//    static constexpr uint8_t MaxParamBytes = 100;
  protected:
    static const Command *FindByID(uint32_t ID);

    const CommandFunc_ pFunc;
    union {
      const uint32_t ID; //!< command ID is just its ASCII name converted to uint32_t
      char Name[];
    };

    const uint8_t NumParamBytes;
    const Command *pNext;
  public:
    Command(const char *Name_, CommandFunc_ pFunc_, uint8_t NumParamBytes_, const Command *pLast):
      pFunc(pFunc_), ID(avp::FourCharsToUint32(Name)), NumParamBytes(NumParamBytes_), pNext(pLast) {
      AVP_ASSERT_WITH_EXPL(ID != 0,2,"ID 0 is reserved for NOOP pseudo-command");
    } //  constructor
    bool IsIt(uint32_t ID_) const { return ID_ == ID; }
  } *pLast = nullptr;  //!< all command objects are connected into unidirectional chain
  //! unidirectional chain, and pLast is where it starts
// ********************************************************
public:
  static void AddCommand(const char Name[4], tFunc pFunc, uint8_t NumParamBytes,) {
    AVP_ASSERT_WITH_EXPL(FindByID(avp::FourCharsToUint32(Name)) == nullptr,1,
                         "A command with this ID already exists."); // check whether we have this command name already

    pLast = new Command(Name,pFunc,NumParamBytes,pLast);
  } // AddCommand

  static const Command *FindByID(uint32_t ID);
  static bool ParseByte(uint8_t NewByte);
  static void Flush() { pInputByte = InputBytes.Start; }
};

/// @tparam Port static class defined by template in AVP_LIBS/General/Port.h. We should call
///   proper Port::Init??? function
/// @tparam MaxSpecCode - return codes < 0 > -MaxSpecCode are considered special error
///   codes. The class itself is using only two codes - CS_ERROR and UART_OVERRUN
template<class Port, uint8_t MaxSpecCode=2>
class ProtocolWithMnemonics: public Protocol<Port,MaxSpecCode>, public CommandChain {
protected:
  static void Flush() {
    Millisec::Pause(10); // for uart to finish
    Protocol<Port,MaxSpecCode>::FlushRX();
    CommandChain::Flush();
  } // Flash
public:
  static void Init(const char *BeaconStr) {
    Protocol<Port,MaxSpecCode>::Init(BeaconStr);
  } // Init

  void cycle() {
    Protocol<Port,MaxSpecCode>::cycle();

    // if port is disconnected run beacon, which allows GUI to find our serial port
    if(SomethingToRX() && !CommandChain::ParseByte(GetByte())) Flush();  // Oops
  } //  cycle
}; //class ProtocolWithMnemonics

// following defines are just for code clearness, do not use elsewhere
#define _TEMPLATE_DECL_ template<class Port, uint8_t MaxSpecCode>
#define _TEMPLATE_SPEC_ ProtocolWithMnemonics<Port, MaxSpecCode>
_TEMPLATE_DECL_ uint8_t _TEMPLATE_SPEC_::InputI = 0;
_TEMPLATE_DECL_ ProtocolCommand_ *_TEMPLATE_SPEC_::CommandTable;
_TEMPLATE_DECL_ uint8_t _TEMPLATE_SPEC_::NumCommands;

#undef _TEMPLATE_DECL_
#undef _TEMPLATE_SPEC_
} // namespace avp

#endif /* SERIAL_PROTOCOL_HPP_INCLUDED */
