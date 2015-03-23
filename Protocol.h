/**
  * @file
  * @author Alexander Panasyuk
  * @verbatim
    Protocol description.
    - all GUI->FW messages are commands:
      - 4 bytes ASCII command name
      - 1 byte its checksum,
      - serial_protocol::Command::NumParamBytes bytes of parameters
      - 1 byte of total checksum,
    - FW-GUI messages are differentiated  based on the first int8_t CODE
        - command return:
          - If CODE is 0 the following is successful latest command return:
            - uint16_t Size is size of data being transmitted.
            - data
            - 1 uint8_t data checksum
          - If CODE is < 0, then it is the last command failure return message:
            - CODE > -NUM_SPEC_CODES, @see enum SpecErrorCodes
           - if CODE <= -NUM_SPEC_CODES it is the error message size, followed by
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
      If error or info message do not fit into 127 bytes remaining text is formatted into consequtive  info message(s) is
  * @endverbatim
  */

#ifndef COMMAND_PROTOCOL_HPP_INCLUDED
#define COMMAND_PROTOCOL_HPP_INCLUDED

/// special error codes
enum SpecErrorCodes { CMD_NAME_CS=1, CMD_MESSAGE_CS, UART_OVERRUN, NUM_SPEC_CODES };
// TODO("Restart transmitting BeaconStr when connection breaks. It is tough to determine when in happens, though.");

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "Math.h"
#include "Error.h"
#include "Time.h"
#include "Port.h"

namespace avp {
/// @tparam Port static class defined by template in AVP_LIBS/General/Port.h
/// @tparam millis global system function returning milliseconds
  template<class Port, uint32_t (*millis)()>
  class Protocol {
  protected:
    static bool PortConnected;
    static union FLW {
      uint32_t Number;
      char Str[];
    } BeaconID;

    static struct Command {
      typedef void (* tCallBackFunc)(const uint8_t Params[]);
    protected:
      union FLW ID; //!< command ID is just its ASCII name converted to uint32_t
      const tCallBackFunc pFunc;

    public:
      static constexpr uint8_t NameLength = sizeof(uint32_t);
      static constexpr uint8_t MaxParamBytes = 100;
      const uint8_t NumParamBytes;
      Command *pPrev;

      // member functions
      Command(uint32_t ID_, tCallBackFunc pFunc_, uint8_t NumParamBytes_):
        pFunc(pFunc_), NumParamBytes(NumParamBytes_) {
        ID.Number = ID_;
        AVP_ASSERT(NumParamBytes_ < MaxParamBytes);
      } // Command

      bool IsIt(uint32_t ID_) const {
        return ID_ == ID.Number;
      }
      void Execute(const uint8_t *Params) {
        (*pFunc)(Params);
      }
    } *pLast, *pCurrent; // class Command

    static struct InputBytes_ {
      union {
        uint32_t ID;
        uint8_t Start[];
      };
      uint8_t ID_CSum;
      uint8_t Params[Command::MaxParamBytes+1]; //!< Total checksum is in Params[pCur->NumParamBytes]
    } __attribute__((packed)) InputBytes;
    static uint8_t *pInputByte; //!< this pointer traces position of the input stream in InputBytes

    // ************** service commands
  protected:
    /// base private message which writes bot info and error messages
    /// There are no Size == 0 messages, it would interfere with command data return
    static inline void message_(const uint8_t *Src, int8_t Size) {
      if(Size) {
        AVP_ASSERT(Port::write(Size));
        AVP_ASSERT(Port::write(Src,Abs(Size)));
        AVP_ASSERT(Port::write(sum<uint8_t>(Src,Abs(Size))));
      }
    } // message_

    static void FlushRX() {
      // read the rest is something is transmitted
      uint8_t b;
      do {
        Port::FlushRX();
        // wait if something appears
        uint32_t WaitUntil = millis() + 3;
        while(WaitUntil - millis() > UINT32_MAX/2); // 10 millis delay
      } while(Port::read(&b));
    } // FlushRX

  public:
    static void info_message(const uint8_t *Src, size_t Size) {
      while(Size > INT8_MAX)  {
        message_(Src,INT8_MAX);
        Src += INT8_MAX;
        Size -= INT8_MAX;
      }
      message_(Src,Size);
    } // info_message

    static bool info_message  PRINTF_WRAPPER(vprintf<make_true<info_message>>)

    static void return_failed(const uint8_t *Src, size_t Size) {
      AVP_ASSERT(Size >= NUM_SPEC_CODES);
      int8_t FirstSize = min<size_t>(Size,INT8_MAX);
      message_(Src,-FirstSize); // message with negative Size indicates error return
      info_message(Src + FirstSize, Size - FirstSize);
    } // return_failed

    static bool return_failed  PRINTF_WRAPPER(vprintf<make_true<return_failed>>)

    /** each time we find a command we put it in the beginning of the chain
    * so if a single command gets issued over and over we do not spend time searching for it
    */
    static Command *FindCommandByID(uint32_t ID) {
      if(pLast->IsIt(ID)) return pLast; // if it is already the first command in chain
      // we have to do absolutely nothing
      // Ok, it is not the first command, things are more complex
      for(Command *pNext = pLast; pNext->pPrev != nullptr; pNext = pNext->pPrev)
        if(pNext->pPrev->IsIt(ID)) {
          // removing current command from its place in chain and inserting as a first link
          Command *p = pNext->pPrev;
          pNext->pPrev = p->pPrev;
          p->pPrev = pLast;
          return pLast = p;
        }
      return nullptr;
    } // FindCommandByID

    static void AddCommand(const char *Name, typename Command::tCallBackFunc pFunc, uint8_t NumParamBytes) {
      union FLW ID; //  just to convert from str to uint32_t
      strncpy(ID.Str, Name, Command::NameLength);
      AVP_ASSERT(pLast == nullptr || FindCommandByID(ID.Number) == nullptr); // check whether we have this command name already
      Command *p = new Command(ID.Number, pFunc, NumParamBytes);
      p->pPrev = pLast;
      pLast = p;
    } // AddCommand;

    //! unidirectional chain, and pLast is where it starts
    static void ParseByte(uint8_t b) {
      *(pInputByte++) = b;

      if(pInputByte == InputBytes.Params) { // just got a new command name and its checksum
        if(sum<uint8_t>(InputBytes.Start,Command::NameLength) == InputBytes.ID_CSum) {
          if((pCurrent = FindCommandByID(InputBytes.ID)) == nullptr) {
            AVP_ASSERT(return_failed("Command <%.*s> not found!", Command::NameLength, InputBytes.Start));
            goto Fail;
          }
        } else {
          AVP_ASSERT(Port::write(-int8_t(CMD_NAME_CS))); // "Command name checksum is wrong" code
          goto Fail;
        }
      } else if(pInputByte == &InputBytes.Params[pCurrent->NumParamBytes + 1])  { // got everything: command,parameters and checksum
        if(sum<uint8_t>(InputBytes.Start,Command::NameLength + pCurrent->NumParamBytes + 1) == InputBytes.Params[pCurrent->NumParamBytes]) {
          pCurrent->Execute(InputBytes.Params); // executing command
          // command is supposed to do all the Returning, because only it knows what receiver is expecting
          pInputByte = InputBytes.Start; // start again
        } else {
          AVP_ASSERT(Port::write(-int8_t(CMD_MESSAGE_CS))); // "Whole command message checksum is wrong!" code
          goto Fail;
        }
      }
      return; // success
Fail:
      FlushRX();
      pInputByte = InputBytes.Start;
    } // Command::Parse;

    //! if port is disconnected run beacon, which allows GUI to find our serial port
    static void SendBeacon() {
      if(!PortConnected) Port::write_unbuffered(&BeaconID);
    }
    static void NOOP(const uint8_t[]) { ReturnOK(); }

  public:

    /// @defgroup InitFunc two init functions, only one of them sg=hould be called depending on port capabilities
    /// @{
    static void Init(const char *BeaconStr) {
      strncpy(BeaconID.Str, BeaconStr, sizeof(BeaconID));
      Port::Init();
      AddCommand("NOOP",NOOP,0); // we need this command for initial handshaking
    } // Init

    static void InitBlock(const char *BeaconStr) {
      strncpy(BeaconID.Str, BeaconStr, sizeof(BeaconID));
      Port::InitBlock();
      AddCommand("NOOP",NOOP,0); // we need this command for initial handshaking
    } // InitBlock
    /// @}

    static void cycle() {
      static RunPeriodically<millis,SendBeacon,500> BeaconTicker;

      BeaconTicker.cycle();

      if(Port::GotSomething()) {
        PortConnected = true;
        ParseByte(Port::GetByte());
      }

      if(Port::IsOverrun()) {
        FlushRX();
        AVP_ASSERT(Port::write(-int8_t(UART_OVERRUN)));
        pInputByte = InputBytes.Start;
      }
    } //  cycle

    static void ReturnBytesBuffered(const uint8_t *src, uint16_t size) {
      AVP_ASSERT(Port::write(uint8_t(0))); // status
      AVP_ASSERT(Port::write(size));
      AVP_ASSERT(Port::write(src, size));
      AVP_ASSERT(Port::write(sum<uint8_t>((const uint8_t *)src,size)));
    } // Protocol::ReturnBytesBuffered
    static void ReturnBytesUnbuffered(const uint8_t *src, uint16_t size, typename Port::tReleaseFunc pFunc = nullptr)  {
      AVP_ASSERT(Port::write(uint8_t(0))); // status
      AVP_ASSERT(Port::write(size));
      AVP_ASSERT(Port::write_unbuffered(src, size, pFunc));
      AVP_ASSERT(Port::write(sum<uint8_t>((const uint8_t *)src,size)));
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
      AVP_ASSERT(Port::write(uint32_t(0)));  // 1 byte status, 2 - size and 1 - checksum
    }
  }; //class Protocol

// following defines are just for code clearnest, do not use elsewhere
#define TEMPLATE template<class Port, uint32_t (*millis)()>
#define PROTOCOL Protocol<Port, millis>

  TEMPLATE bool PROTOCOL::PortConnected = false;
  TEMPLATE typename PROTOCOL::FLW PROTOCOL::BeaconID;
  TEMPLATE typename PROTOCOL::InputBytes_ PROTOCOL::InputBytes;
  TEMPLATE uint8_t *PROTOCOL::pInputByte = PROTOCOL::InputBytes.Start;
  TEMPLATE typename PROTOCOL::Command *PROTOCOL::pLast = nullptr;
  TEMPLATE typename PROTOCOL::Command *PROTOCOL::pCurrent;
} // namespace avp



#endif /* SERIAL_PROTOCOL_HPP_INCLUDED */
