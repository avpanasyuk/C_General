#ifndef COMMANDPARSER_H_INCLUDED
#define COMMANDPARSER_H_INCLUDED

#include <stdint.h>

namespace avp {
  typedef void (*CommandFunc_)(const uint8_t Params[]);
  static constexpr uint8_t MaxNumParamBytes = 20;

  class CommandParser {
  public:
    enum ParseError_ {NO_ERROR = 0, NOOP, WRONG_ID, BAD_CHECKSUM, NUM_ERRORS};
    /// @retval when NO_ERROR ParseByte send response itself (e.g. from CommandFunc )
    /// when error ParseByte does not send response
//    static ParseError_ ParseByte(uint8_t byte) = 0;
//    static void Flush() = 0;
  }; // class CommandParser
} // namespace avp

#endif /* COMMANDPARSER_H_INCLUDED */
