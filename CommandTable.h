#ifndef COMMANDTABLE_H_INCLUDED
#define COMMANDTABLE_H_INCLUDED

#include "Error.h"
#include "CommandParser.h"

namespace avp {
  template<const Command_ *Ptr, uint8_t NumCommands>
  class CommandTable: public CommandParser {
    static uint8_t InputI;

    struct Command_ {
      CommandFunc_ Func;
      uint8_t NumParamBytes;
    }; // struct Command_

    // static const Command_ *Ptr;
    // static const uint8_t NumCommands;
  public:
    // static void Init(Command_ *pTable, uint8_t NumCommands_):
    //   Ptr(pTable), NumCommands(NumCommands_) {}

    static ParseError_ ParseByte(uint8_t b) {
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

      if(InputI == 1) { // b is CurCommand.ID
        if(b != 0) { // it is not a NOOP command
          if(b > NumCommands || CommandTable[b-1].Func == nullptr ) return WRONG_ID;
          AVP_ASSERT_WITH_EXPL(CommandTable[b-1].NumParamBytes < MaxNumParamBytes,1,
                               "Modify MaxNumParamBytes to fit %hhu param bytes.",
                               CommandTable[b-1].NumParamBytes);
        } else { // 0 is NOOP command, no parameters or checksum
           InputI = 0;
           return NOOP;
        }
      } else if(InputI == CommandTable[Input.Cmd.ID-1].NumParamBytes + 2) { // we've got all parameter bytes
        // and a checksum
        if(sum<uint8_t>(Input.Bytes,InputI - 1) != b) return BAD_CHECKSUM;
        else {
          CommandTable[Input.Cmd.ID-1].Func(Input.Cmd.Params); // callback function should do return itself
          InputI = 0;
        }
      }
      return NO_ERROR;
    } // ParseByte

    static void Flush() {InputI = 0;}
  }; // class CommandTable

  template<const Command_ *Ptr, uint8_t NumCommands> uint8_t CommandTable<Ptr,NumCommands>::InputI = 0;

} // namespace avp


#endif /* COMMANDTABLE_H_INCLUDED */
