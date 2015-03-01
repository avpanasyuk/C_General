/**
  * @file
  * @author Alexander Panasyuk
  * @verbatim
    Protocol description.
    GUI->FW command: 4 bytes ASCII command name, 1 byte its checksum, serial_protocol::Command::NumParamBytes of
    ParamBytes, 1 byte of total checksum,
    FW-GUI command return: 1) uint8_t status. If it is 0 then command is successful and next uint16_t Size is size
    of data being transmitted. If it is not 0, then it is error code and next uint8_t is the size of error message.
    FW-GUI messages: 8-bit size and then text
  * @endverbatim
  */

#ifndef COMMAND_PROTOCOL_HPP_INCLUDED
#define COMMAND_PROTOCOL_HPP_INCLUDED

#include <string.h>
#include <stdint.h>
#include "Time.h"
#include "BufferedPort.h"

namespace avp {
  template<class Port> class CommandProtocol {
   protected:
    typedef void (* tReleaseFunc)(void *);

    static bool PortConnected = false;
    static union { uint32_t Number; char Str[]; } BeaconID;

    static struct {
      union { uint32_t ID; uint8_t Start[]; };
      uint8_t ID_CSum;
      uint8_t Params[MaxParamBytes+1]; //!< Total checksum is in Params[pCur->NumParamBytes]
    } __attribute__((packed)) InputBytes;
    static uint8_t *pInputByte; //!< this pointer traces position of the input stream in InputBytes

    static struct Command {
      static constexpr uint8_t NameLength = sizeof(uint32_t);
      static constexpr uint8_t MaxParamBytes = 100;
      const tFunc pFunc;
      const uint32_t ID; //!< command ID is just its ASCII name converted to uint32_t

      const uint8_t NumParamBytes;
      const Command *pPrev;

      typedef void (* tFunc)(const uint8_t Params[]);  //<! callback function type

      // member functions
      Command(uint32_t ID_, tFunc pFunc_, uint8_t NumParamBytes_):
        ID(ID_), pFunc(pFunc_), NumParamBytes(NumParamBytes_) {
        static_assert(NumParamBytes_ < MaxParamBytes);
      } // Command

      bool IsIt(uint32_t ID_) const { return ID_ == ID; }
    } *pLast, *pCurrent; // class Command

    // ************** service commands
   protected:
    // max message length is 256, if message is longer 255 chars are output and last char is '>'
    static bool vprintf(char const *format, va_list ap) {
      int Size = vsnprintf(NULL,0,format,ap);
      bool TooLong = false;
      if(Size < 0) return false;
      if(Size > UINT8_MAX) { Size = UINT8_MAX; TooLong = true; }

      uint8_t Buffer[1+Size+1]; // Size does not include tailing 0, but vsprintf prints it anyway
      Buffer[0] = Size;
      vsnprintf((char *)Buffer+1,Size,format,ap);
      if(TooLong) Buffer[Size] = '>'; // indicator that message is truncated
      Buffer[Size+1] = avp::sum<uint8_t>(Buffer+1,Size); // replace trailing 0 by checksum
      Port::write(Buffer,1+Size+1); // we do not write tailing 0 byte
      return true;
    } // vprintf

   public:
    static bool ReturnBad(uint8_t code, char const *format = nullptr, ...) {
      static_assert(code != 0);
      if(format != nullptr) {
        va_list ap;
        va_start(ap,format);
        bool Out =  vprintf(format,ap);
        va_end(ap);
        return Out;
      } else return true;
    } // ReturnBad

    /** each time we find a command we put it in the beginning of the chain
    * so if a single command gets issued over and over we do not spend time searching for it
    */
    const Command *FindCommandByID(uint32_t ID) {
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

    bool AddCommand(const char *Name, tFunc pFunc, uint8_t NumParamBytes) {
      union { uint32_t Num; char Str[4]; } ID; //  just to convert from str to uint32_t
      strncpy(ID.Str, Name, Command::NameLength);
      AVP_ASSERT(pLast == nullptr || FindCommandByID(ID) == nullptr); // check whether we have this command name already
      Command *p = new Command(ID.Num, pFunc, NumParamBytes);
      p->pPrev = pLast;
      pLast = p;
    } // AddCommand;

    //! unidirectional chain, and pLast is where it starts
    static void ParseByte(uint8_t b) {
      *(pInputByte++) = b;

      if(pInputByte == InputBytes.Params) { // just got a new command name and its checksum
        if(avp::sum<uint8_t>(InputBytes.Start,NameLength) == InputBytes.ID_CSum) {
          if((pCurrent = FindCommandByID(InputBytes.ID)) == nullptr) {
            AVP_ASSERT(ReturnBad(8,"Command not found!")); goto Fail;
          }
        } else {
          AVP_ASSERT(ReturnBad(6,"Command name checksum is wrong!")); goto Fail;
        }
      } else if(pInputByte == &InputBytes.Params[pCurrent->NumParamBytes + 1])  { // got everything: command,parameters and checksum
        if(avp::sum<uint8_t>(InputBytes.Start,NameLength + pCurrent->NumParamBytes + 1) == InputBytes.Params[pCurrent->NumParamBytes]) {
          pCurrent->pFunc(InputBytes.Params); // executing command
          pInputByte = InputBytes.Start;
        } else {
          AVP_ASSERT(ReturnBad(7,"Command checksum is wrong!")); goto Fail;
        }
      }
      return; // success
Fail:
      while(Port::read(&b));  // flashing input buffer, b is used just not to introduce new variable
      pInputByte = InputBytes.Start;
    } // Command::Parse;

   public:

    void Init(uint32_t baud, const char* ID): pInputByte(InputBytes.Start), pLast(nullptr) {
      strncpy(BeaconID.Str, ID, sizeof(BeaconID.Str));
      Port::Init(baud);
    } // Init

    void cycle() {
      static Millisec BeaconTicker(BeaconPeriod);

      // if port is disconnected run beacon, which allows GUI to find our serial port
      if(Port::GotSomething()) {
        PortConnected = true;
        Command::Parse(Port::GetByte());
      } else if(BeaconPeriod != 0 && !PortConnected && BeaconTicker.Passed())
        Port::write(BeaconID.Str);
    } //  cycle

    /** @brief we have to redefine from avp::vprintf those because VPRINTF now has to send size byte first and csum byte last
    */
    void ReturnBytesBuffered(const uint8_t *src, uint16_t size) {
      Port::write(uint8_t(0)); // status
      Port::write(size);
      Port::write(src, size);
      Port::write(avp::sum<uint8_t>((const uint8_t *)src,size));
    } // CommandProtocol::ReturnBytesBuffered
    void ReturnBytesUnbuffered(const uint8_t *src, uint16_t size, BufferPort::tReleaseFunc pFunc = nullptr)  {
      Port::write(uint8_t(0)); // status
      Port::write(size);
      Port::write_pointed(src, size, pFunc);
      Port::write(avp::sum<uint8_t>((const uint8_t *)src,size));
    } // CommandProtocol::ReturnBytesUnbuffered;

    template<typename T> void ReturnUnbuffered(const T *pT, uint16_t size = 1, BufferPort::tReleaseFunc pFunc = nullptr) {
      ReturnBytesUnbuffered((const uint8_t *)pT,sizeof(T)*size,pFunc);
    } // ReturnUnbuffered
    template<typename T> static void Return(T x) {
      ReturnBytesBuffered((const uint8_t *)&x,sizeof(T));
    } // Return
    static void ReturnOK() { Port::write(uint32_t(0)); } // 1 byte status, 2 - size and 1 - checksum
  }; //class CommandProtocol
} // namespace avp

#endif /* SERIAL_PROTOCOL_HPP_INCLUDED */
