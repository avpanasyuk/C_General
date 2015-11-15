#ifndef COMMANDCHAIN_H_INCLUDED
#define COMMANDCHAIN_H_INCLUDED

#include <stdint.h>
#include "BitBang.h"
#include "Error.h"
#include "CommandParser.h"

namespace avp {
/// unidirectional command chain
  template<typename IDtype, uint8_t MaxNumParamBytes = 255>
  class CommandChain: public CommandParser {

    /// One Command makes a Link ------------------------------------------------
    static struct Link {
      union {
        const IDtype ID; ///< command ID is just its ASCII name converted to IDtype
        const char Name[];
      };
      const CommandFunc_ pFunc;
      const uint8_t NumParamBytes;
      Link *pNext;
    // public:
      Link(const char *Name_, CommandFunc_ pFunc_, uint8_t NumParamBytes_, Link *pFirst):
        ID(Chars2type<IDtype>(Name_)), pFunc(pFunc_), NumParamBytes(NumParamBytes_), pNext(pFirst) {
        AVP_ASSERT_WITH_EXPL((ID & 0xFF) != 0,2,"Byte  0 is reserved for NOOP pseudo-command.");
        AVP_ASSERT_WITH_EXPL(NumParamBytes <= MaxNumParamBytes,3,"Modify MaxNumParamBytes to accommodate %hhu bytes.",NumParamBytes);
        AVP_ASSERT_WITH_EXPL(pFunc != nullptr,4,"We do not do useless commands.");
      } //  constructor
      bool IsIt(IDtype ID_) const { return ID_ == ID; }
    } *pFirst;  ///< pointer to the first command in chain -----------------------

    // check for a new command
    static struct InputBytes_ {
      union {
        IDtype ID;
        uint8_t Name[];
      };
      uint8_t Params[MaxNumParamBytes+1]; ///< really checksum is in Params[pCur->NumParamBytes]
    } InputBytes;

    static uint8_t *pInputByte; ///< tracks byte number of current command packet

    /// this find function is a bit tricky - it moves found command to the start of the chain,
    /// so if the same command is called over and over we will be fincing it very fast
    static const class Link *FindByID(IDtype ID) {
      if(pFirst == nullptr) return nullptr;
      if(pFirst->IsIt(ID)) return pFirst; // if it is already the first command in chain
      // we have to do absolutely nothing
      // Ok, it is not the first command, things are more complex
      for(Link *pPrev = pFirst; pPrev->pNext != nullptr; pPrev = pPrev->pNext)
        if(pPrev->pNext->IsIt(ID)) {
          // removing current command from its place in chain and inserting as a first link
          Link *p = pPrev->pNext;
          pPrev->pNext = p->pNext;
          p->pNext = pFirst;
          return pFirst = p;
        }
      return nullptr;
    } // FindByID
  public:
    static void AddCommand(const char Name[sizeof(IDtype)], CommandFunc_ pFunc, uint8_t NumParamBytes) {
      AVP_ASSERT_WITH_EXPL(FindByID(Chars2type<IDtype>(Name)) == nullptr,1,
                           "A command with this ID already exists."); // check whether we have this command name already
	  AVP_ASSERT(NumParamBytes <= MaxNumParamBytes);
      pFirst = new Link(Name,pFunc,NumParamBytes,pFirst);
    } // AddCommand

    static void Flush() { pInputByte = (uint8_t *)InputBytes.Name; }

    static ParseError_ ParseByte(uint8_t NewByte) { // this is static member function
      static const Link *pCur;
      if(pInputByte == InputBytes.Name && NewByte == 0) return NOOP;

      *pInputByte = NewByte;

      if(++pInputByte == InputBytes.Params) { // just got a new command ID
        if((pCur = FindByID(InputBytes.ID)) == nullptr) return WRONG_ID;
      } else if(pInputByte == &InputBytes.Params[pCur->NumParamBytes + 1])  { // got everything: command,parameters and checksum
        if(sum<uint8_t>(InputBytes.Name,sizeof(IDtype) + pCur->NumParamBytes) == InputBytes.Params[pCur->NumParamBytes]) {
          pCur->pFunc(InputBytes.Params); // executing command
          pInputByte = InputBytes.Name;
        } else {
          debug_printf("CS received = %hhu, calculated = %hhu", 0,0);
          return BAD_CHECKSUM;
        }
      }
      return NO_ERROR; // everything's OK
    } // ParseByte
  }; //class CommandChain

#define _TEMPLATE_DECL_ template<typename IDtype, uint8_t MaxNumParamBytes>
#define _TEMPLATE_SPEC_ CommandChain<IDtype, MaxNumParamBytes>

  _TEMPLATE_DECL_ typename _TEMPLATE_SPEC_::Link *_TEMPLATE_SPEC_::pFirst = nullptr;
  _TEMPLATE_DECL_ typename _TEMPLATE_SPEC_::InputBytes_ _TEMPLATE_SPEC_::InputBytes;
  _TEMPLATE_DECL_  uint8_t *_TEMPLATE_SPEC_::pInputByte = (uint8_t *)_TEMPLATE_SPEC_::InputBytes.Name;

#undef _TEMPLATE_DECL_
#undef _TEMPLATE_SPEC_


} // namespace avp



#endif /* COMMANDCHAIN_H_INCLUDED */
