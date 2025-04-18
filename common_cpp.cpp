/*
 * common_cpp.cpp
 *
 * Created: 11/11/2013 3:02:38 PM
 *  Author: panasyuk
 */
#ifndef NO_STL
#include <chrono>
#include <string>
#endif // NO_STL

#include "millis_micros.hpp"
#include "General.hpp"

namespace avp {
  uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t crc, uint16_t poly) {
    return ::Crc16(pcBlock, len, crc, poly);
  }

#ifndef NO_STL
  uint32_t millis() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch()
           ).count();
  } // millis

  std::string string_vprintf(const char *format, va_list ap) {
    va_list ap_;
    va_copy(ap_,ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
    const int Size = vsnprintf(nullptr,0,format,ap_);
    if(Size < 0) return "string_vprintf: format is wrong!";
#ifdef _WIN32
    char *Buffer = (char *)_alloca(Size+1); // +1 to include ending zero byte
#else
    char Buffer[Size+1]; // +1 to include ending zero byte
#endif
    vsnprintf(Buffer,Size,format,ap);
    return std::string(Buffer,Size); // we do not write ending 0 byte
  } // string_vprintf

  PRINTF_WRAPPER(std::string,string_printf,string_vprintf)
//  std::string string_printf(char const *format, ...) {
//    va_list ap;
//    va_start(ap,format);
//    std::string Out =  string_vprintf(format,ap);
//    va_end(ap);
//    return Out;
//  } // string_printf
#endif
} // namespace avp


// #include <stdlib.h>
// #include <stdio.h>
// #include "General.h"
// #include "Error.h"
// #include "BitBang.h"

// char AVP_ErrorMsgBuffer[AVP_ERROR_MSG_BUFFER_SZ];

// #ifdef  __GNUC__
// volatile uint8_t FailReason = 0;
// void new_handler() { major_fail(MEMALLOC); }

// __weak int debug_vprintf(const char *format, va_list a) { return ::vprintf(format,a); }
// __weak void hang_cpu() { while(1); }

// __weak void major_fail(uint8_t reason) {
//  FailReason = reason;
//  hang_cpu();
// }
// __weak void debug_action() {};

// __weak int debug_printf(char const *format, ...) {
//  va_list ap;
//  va_start(ap,format);
//  bool Out = debug_vprintf(format,ap) >= 0;
//  va_end(ap);
//  return Out?1:-1;
// }

// #endif


// #ifndef NO_STL
// namespace avp {
//  std::string string_vprintf(const char *format, va_list ap) {
//    va_list ap_;
//    va_copy(ap_,ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
//    const int Size = vsnprintf(nullptr,0,format,ap_);
//    if(Size < 0) return "string_vprintf: format is wrong!";
// #ifdef _WIN32
//    char *Buffer = (char *)_alloca(Size+1); // +1 to include ending zero byte
// #else
//    char *Buffer = (char *)alloca(Size+1); // +1 to include ending zero byte
// #endif
//    vsprintf(Buffer,format,ap);
//    return std::string(Buffer,Size); // we do not write ending 0 byte
//  } // string_vprintf

// std::string string_printf(char const *format, ...) {
//    va_list ap;
//    va_start(ap,format);
//    std::string Out =  string_vprintf(format,ap);
//    va_end(ap);
//    return Out;
//  } // string_printf
// uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t start) {
//   uint16_t crc = start;

//   while(len--) {
//     crc ^= uint16_t(*pcBlock++) << 8;

//     for(uint8_t i = 0; i < 8; i++)
//       if((crc & 0x8000) != 0)
//         crc = ((crc ^ 0x8810) << 1) + 1;
//       else crc <<= 1;
//   }
//   return crc;
// } // Crc16
// } // namnespace avp
// #endif





