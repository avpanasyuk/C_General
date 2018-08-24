/*! @file General\CommandTable.h
 *  @author Alexander Panasyuk
 *  To use define
 * template<> const avp::Command_ avp::CommandTable<>::Table[] = {{command,num parameter bytes},...};
 * template<> const uint8_t CommandTable<>::NumCommands = N_ELEMENTS(Table);
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
template<uint8_t MaxNumParamBytes = 255>
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
        IGNORE(-Warray-bounds)
        Input.Bytes[InputI++] = b;
        STOP_IGNORING

        if(InputI == 1) { // b is a CurCommand.ID
          if(b != 0) { // it is not a NOOP command
            if(b > NumCommands || Table[b-1].Func == nullptr ) return WRONG_ID;
            CurNumOfParamBytes = Table[b-1].NumParamBytes; // may be -1
          } else { // 0 is NOOP command, no parameters or checksum
            InputI = 0;
            return NOOP;
          }
        } else {
          if(InputI == 2) {
			  if(CurNumOfParamBytes == -1) CurNumOfParamBytes = b+1; // variable number of parameters
			  AVP_ASSERT(CurNumOfParamBytes <= MaxNumParamBytes);
		  }
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
  
  template<uint8_t MaxNumParamBytes> int8_t CommandTable<MaxNumParamBytes>::CurNumOfParamBytes;
  template<uint8_t MaxNumParamBytes> uint8_t CommandTable<MaxNumParamBytes>::InputI = 0; 
} // namespace avp


#endif /* COMMANDTABLE_H_INCLUDED */

