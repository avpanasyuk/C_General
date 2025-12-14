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

#include "General.hpp"
#include "millis_micros.hpp"

#ifndef NO_STL
uint32_t millis() {
  return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
} // millis

uint32_t micros() {
  return (uint32_t)std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
} // millis
#endif
namespace avp {
  uint16_t Crc16(const uint8_t *pcBlock, long long len, uint16_t crc, uint16_t poly) {
    return ::Crc16(pcBlock, len, crc, poly);
  }

#ifndef NO_STL
  std::string string_vprintf(const char *format, va_list ap) {
    va_list ap_;
    va_copy(ap_, ap); // turns out vsnprintf is changing ap, so we have to make a reserve copy
    const int Size = vsnprintf(nullptr, 0, format, ap_);
    if(Size < 0) return "string_vprintf: format is wrong!";
#ifdef _WIN32
    char *Buffer = (char *)_alloca(Size + 1); // +1 to include ending zero byte
#else
    char Buffer[Size + 1]; // +1 to include ending zero byte
#endif
    vsnprintf(Buffer, Size + 1, format, ap);
    return std::string(Buffer, Size); // we do not write ending 0 byte
  } // string_vprintf

  PRINTF_WRAPPER(std::string, string_printf, string_vprintf)
  
    std::string getCurrentTimeFormatted(const char *Format) {
    // Get current time
    std::time_t now = std::time(nullptr);

    // Convert to local time
    std::tm *timeinfo = std::localtime(&now);

    // Format using ostringstream and put_time
    std::ostringstream oss;
    oss << std::put_time(timeinfo, Format);

    return oss.str();
  } // getCurrentTimeFormatted

#endif
} // namespace avp


