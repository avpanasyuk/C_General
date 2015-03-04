/**
  * @file
  * @author Alexander Panasyuk
  * @verbatim
    Protocol description.
    GUI->FW command: 4 bytes ASCII command name, 1 byte its checksum, serial_protocol::Command::NumParamBytes of
    ParamBytes, 1 byte of total checksum,
    FW-GUI command return: 
    1) uint8_t status. 
    2) If status is 0 then command is successful and next uint16_t Size is size of data being transmitted and uint8_t data csum. 
    3) If status is not 0, then status is an error code and next uint8_t is the size of error message, then error message and 
       then its uint8_t csum
    FW-GUI messages: 8-bit size and then text and them text's csum
    
    Initial handshaking looks like following:
    Before connection is established FW sends out BeaconStr (specified in Init) every 0.5 sec. Receiver checking all ports one by one
    until it finds the port sending BeaconStr. Receiver sends NOOP command in response. When FW gets the first byte in port it stops 
    transmitting BeaconStr and consider connection established.
  * @endverbatim
  */

#ifndef COMMAND_PROTOCOL_HPP_INCLUDED
#define COMMAND_PROTOCOL_HPP_INCLUDED

TODO("Restart transmitting BeaconStr when connection breaks. It is tough to determine when in happens, though.");

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "Math.h"
#include "Error.h"
#include "Time.h"
#include "Port.h"

namespace avp {
  template<class Port> class Protocol {
   protected:
    // typedef void (* tReleaseFunc)(void *);

    static bool PortConnected;
    static union FLW { uint32_t Number; char Str[]; } BeaconID;

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

      bool IsIt(uint32_t ID_) const { return ID_ == ID.Number; }
      void Execute(const uint8_t *Params) { (*pFunc)(Params); } 
    } *pLast, *pCurrent; // class Command

    static struct InputBytes_ {
      union { uint32_t ID; uint8_t Start[]; };
      uint8_t ID_CSum;
      uint8_t Params[Command::MaxParamBytes+1]; //!< Total checksum is in Params[pCur->NumParamBytes]
    } __attribute__((packed)) InputBytes;
    static uint8_t *pInputByte; //!< this pointer traces position of the input stream in InputBytes

    // ************** service commands
   protected:
   //! @brief form a whole message, including first size byte and ending csum
    // max message length is 256, if message is longer 255 chars are output and last char is '>'
    static bool vprintf(char const *format, va_list ap) {
      int Size = vsnprintf(NULL,0,format,ap);
      if(Size < 0) return vprintf("vprintf can not convert!\n",ap); // ap here is just for right number of parameters

      bool TooLong = false;
      if(Size > UINT8_MAX) { Size = UINT8_MAX; TooLong = true; }

      uint8_t Buffer[1+Size+1]; // Size does not include tailing 0, but vsprintf prints it anyway
      // we transmit size as a first byte and csum as the last
      Buffer[0] = Size;
      vsnprintf((char *)Buffer+1,Size,format,ap);
      if(TooLong) Buffer[Size] = '>'; // indicator that message is truncated
      Buffer[Size+1] = avp::sum<uint8_t>(Buffer+1,Size); // replace trailing 0 by checksum
      Port::write(Buffer,1+Size+1);
      return true;
    } // vprintf

   public:
    static bool ReturnBad(uint8_t code, char const *format = nullptr, ...) {
      AVP_ASSERT(code != 0);
      if(format != nullptr) {
        va_list ap;
        va_start(ap,format);
        bool Out =  vprintf(format,ap);
        va_end(ap);
        return Out;
      } else {
        Port::write(code);
        Port::write(uint16_t(0)); // message length and csum
        return true;
      }        
    } // ReturnBad

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
        if(avp::sum<uint8_t>(InputBytes.Start,Command::NameLength) == InputBytes.ID_CSum) {
          if((pCurrent = FindCommandByID(InputBytes.ID)) == nullptr) {
            AVP_ASSERT(ReturnBad(8,"Command not found!")); goto Fail;
          }
        } else {
          AVP_ASSERT(ReturnBad(6,"Command name checksum is wrong!")); goto Fail;
        }
      } else if(pInputByte == &InputBytes.Params[pCurrent->NumParamBytes + 1])  { // got everything: command,parameters and checksum
        if(avp::sum<uint8_t>(InputBytes.Start,Command::NameLength + pCurrent->NumParamBytes + 1) == InputBytes.Params[pCurrent->NumParamBytes]) {
          pCurrent->Execute(InputBytes.Params); // executing command
          // command is supposed to do all the Returning, because only it knows what receiver is expecting
          pInputByte = InputBytes.Start; // start again
        } else {
          AVP_ASSERT(ReturnBad(7,"Command checksum is wrong!")); goto Fail;
        }
      }
      return; // success
Fail:
      while(Port::read(&b));  // flashing input buffer, b is used just not to introduce new variable
      pInputByte = InputBytes.Start;
    } // Command::Parse;

    //! if port is disconnected run beacon, which allows GUI to find our serial port
    static void SendBeacon() { if(!PortConnected) Port::write(BeaconID); }
    static void NOOP(const uint8_t[]) { ReturnOK(); }

   public:

    static void Init(uint32_t baud, const char* BeaconStr) {
      strncpy(BeaconID.Str, BeaconStr, sizeof(BeaconID));
      Port::Init(baud);
      AddCommand("NOOP",NOOP,0); // we need this command for initial handshaking
    } // Init

    static void cycle() {
      static RunPeriodically<millis,SendBeacon,100> BeaconTicker;

      BeaconTicker.cycle();

      if(Port::GotSomething()) {
        PortConnected = true;
        ParseByte(Port::GetByte());
      }
    } //  cycle

    /** @brief we have to redefine from avp::vprintf those because VPRINTF now has to send size byte first and csum byte last
    */
    static void ReturnBytesBuffered(const uint8_t *src, uint16_t size) {
      AVP_ASSERT(Port::write(uint8_t(0))); // status
      AVP_ASSERT(Port::write(size));
      AVP_ASSERT(Port::write(src, size));
      AVP_ASSERT(Port::write(avp::sum<uint8_t>((const uint8_t *)src,size)));
    } // Protocol::ReturnBytesBuffered
    static void ReturnBytesUnbuffered(const uint8_t *src, uint16_t size, typename Port::tReleaseFunc pFunc = nullptr)  {
      AVP_ASSERT(Port::write(uint8_t(0))); // status
      AVP_ASSERT(Port::write(size));
      AVP_ASSERT(Port::write_pointed(src, size, pFunc));
      AVP_ASSERT(Port::write(avp::sum<uint8_t>((const uint8_t *)src,size)));
    } // Protocol::ReturnBytesUnbuffered;

    template<typename T> static void ReturnUnbuffered(const T *pT, uint16_t size = 1, typename Port::tReleaseFunc pFunc = nullptr) {
      ReturnBytesUnbuffered((const uint8_t *)pT,sizeof(T)*size,pFunc);
    } // ReturnUnbuffered
    template<typename T> static void Return(T x) {
      ReturnBytesBuffered((const uint8_t *)&x,sizeof(T));
    } // Return
    static void ReturnOK() { AVP_ASSERT(Port::write(uint32_t(0))); } // 1 byte status, 2 - size and 1 - checksum
  }; //class Protocol

  template<class Port> bool Protocol<Port>::PortConnected = false;
  template<class Port> typename Protocol<Port>::FLW Protocol<Port>::BeaconID;
  template<class Port> typename Protocol<Port>::InputBytes_ Protocol<Port>::InputBytes;
  template<class Port> uint8_t *Protocol<Port>::pInputByte = Protocol<Port>::InputBytes.Start; //!< this pointer traces position of the input stream in InputBytes
  template<class Port> typename Protocol<Port>::Command *Protocol<Port>::pLast = nullptr;
  template<class Port> typename Protocol<Port>::Command *Protocol<Port>::pCurrent;
} // namespace avp



#endif /* SERIAL_PROTOCOL_HPP_INCLUDED */
