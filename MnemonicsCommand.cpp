#include "MnemonicsCommand.h"

namespace avp {
  struct CommandChain::InputBytes_ CommandChain::InputBytes;
  uint8_t *CommandChain::pInputByte = CommandChain::InputBytes.Start;
  struct CommandChain::Command *CommandChain::pLast = nullptr;

  /// this find function is a bit tricky - it moves found command to the start of the chain,
  /// so if the same command is called over and over we will be fincing it very fast
  const Command *CommandChain::FindByID(uint32_t ID) {
    if(pLast == nullptr) return nullptr;
    if(pLast->IsIt(ID)) return pLast; // if it is already the first command in chain
    // we have to do absolutely nothing
    // Ok, it is not the first command, things are more complex
    for(Command *pPrev = pLast; pPrev->pNext != nullptr; pPrev = pPrev->pNext)
      if(pPrev->pNext->IsIt(ID)) {
        // removing current command from its place in chain and inserting as a first link
        Command *p = pPrev->pNext;
        pPrev->pNext = p->pNext;
        p->pNext = pLast;
        return pLast = p;
      }
    return nullptr;
  } // FindByID

  bool CommandChain::ParseByte(uint8_t NewByte) { // this is static member function
    static const Command *pCur;
    *(pInputByte++) = NewByte;

    if(pInputByte == InputBytes.Params) { // just got a new command ID
        if((pCur = FindByID(InputBytes.ID)) == nullptr) {
          return_error_printf("Command with ID %.4s not found!", (const char *)InputBytes.Start);
          return false;
        } // not doing anything otherwise
    } else if(pInputByte == &InputBytes.Params[pCur->NumParamBytes + 1])  { // got everything: command,parameters and checksum
      if(avp::sum<uint8_t>(InputBytes.Start,NameLength + pCur->NumParamBytes + 1) == InputBytes.Params[pCur->NumParamBytes]) {
        pCur->pFunc(InputBytes.Params); // executing command
        pInputByte = InputBytes.Start;
      } else {
        return_error_printf("Command %.4s checksum %hh is wrong!", (const char *)InputBytes.Start,
                            InputBytes.Params[pCur->NumParamBytes]);
        return false;
      }
    }
    return true; // everything's OK
  } // Command::Parse
} // namespace avp
