/*! @file General\CommandTable.h
 *! @author Alexander Panasyuk
 */
#ifndef COMMANDTABLE_H_INCLUDED
#define COMMANDTABLE_H_INCLUDED

#include "Error.h"
#include "Math.h"
#include "CommandParser.h"

namespace avp {
  struct Command_ {
    CommandFunc_ Func;
    int8_t NumParamBytes; ///< if == -1 the command has variable number of arguments, and the first
    /// parameter byte is the number of following parameters
  }; // struct Command_

/// Table and NumCommands should be implemented in the user code
  class CommandTable: public CommandParser {
    protected:
      static int8_t CurNumOfParamBytes;
      static uint8_t InputI;
      static const Command_ Table[];
      static const uint8_t NumCommands;
    public:
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
        AVP_ASSERT_WITH_EXPL(InputI < sizeof(Input.Cmd),1,
                             "Modify MaxNumParamBytes to fit %hhu param bytes.",
                             InputI);
        Input.Bytes[InputI++] = b;

        if(InputI == 1) { // b is a CurCommand.ID
          if(b != 0) { // it is not a NOOP command
            if(b > NumCommands || Table[b-1].Func == nullptr ) return WRONG_ID;
            CurNumOfParamBytes = Table[b-1].NumParamBytes; // may be -1
          } else { // 0 is NOOP command, no parameters or checksum
            InputI = 0;
            return NOOP;
          }
        } else {
          if(InputI == 2 && CurNumOfParamBytes == -1) CurNumOfParamBytes = b; // variable number of parameters
          if(InputI == CurNumOfParamBytes + 2) { // we've got all parameter bytes
            // and a checksum
            if(sum<uint8_t>(Input.Bytes,InputI - 1) != b) return BAD_CHECKSUM;
            else {
              Table[Input.Cmd.ID-1].Func(Input.Cmd.Params); // callback function should do return itself
              InputI = 0;
            }
          }
        }
        return NO_ERROR;
      } // ParseByte

      static void Flush() {InputI = 0;}
  }; // class CommandTable
} // namespace avp


#endif /* COMMANDTABLE_H_INCLUDED */

