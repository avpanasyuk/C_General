#ifndef COMMANDPARSER_H_INCLUDED
#define COMMANDPARSER_H_INCLUDED

#include <stdint.h>

namespace avp {
  typedef void (*CommandFunc_)(const uint8_t Params[]);

  class CommandParser {
  public:
    enum ParseError_ {NO_ERROR = 0, NOOP, WRONG_ID, BAD_CHECKSUM, WRONG_PARAM_SIZE, NUM_ERRORS};
    /// @retval when NO_ERROR ParseByte send response itself (e.g. from CommandFunc )
    /// when error ParseByte does not send response
//    static ParseError_ ParseByte(uint8_t byte) = 0;
//    static void Flush() = 0;
  }; // class CommandParser
} // namespace avp

#endif /* COMMANDPARSER_H_INCLUDED */
