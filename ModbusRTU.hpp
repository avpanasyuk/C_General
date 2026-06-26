/**
 * @file ModbusRTU.hpp
 * @brief Minimal, transport-agnostic Modbus-RTU **master** (header-only).
 *
 * Speaks just enough Modbus-RTU to poll typical RS485 sensors: build a request
 * frame, append the Modbus CRC16 (via avp::Crc16 with the reflected 0xA001 poly),
 * send it, read the reply, and validate addr/function/length/CRC.
 *
 * It is deliberately free of any platform/UART dependency: the caller injects
 * three plain function pointers (write bytes, read one byte, read a millisecond
 * clock), so the same code runs on Arduino/SoftwareSerial, STM32 HAL UART, or a
 * host serial port. Half-duplex direction control is assumed to be automatic
 * (e.g. an XY-017 auto-flow RS485 converter); there is no DE/RE handling here.
 *
 * Only function code 0x03 (read holding registers) is implemented, which covers
 * the read-only "report your measurements" sensors this is meant for.
 *
 * @author panasyuk
 */

#ifndef AVP_MODBUS_RTU_HPP_
#define AVP_MODBUS_RTU_HPP_

/// @cond
#include <stdint.h>
#include <stddef.h>
/// @endcond
#include "General.hpp" // avp::Crc16, avp::CRC16_MODBUS_POLY

namespace avp {

  /**
   * @brief Modbus-RTU master over a caller-supplied byte transport.
   *
   * All I/O goes through three function pointers passed at construction, so this
   * class pulls in no UART/Arduino headers. Typical wiring on Arduino:
   * @code
   *   void wr(const uint8_t* b, size_t n) { mySerial.write(b, n); }
   *   int  rd()                           { return mySerial.read(); } // -1 if none
   *   avp::ModbusMaster mb(wr, rd, millis);
   *   uint16_t reg[7];
   *   if(mb.ReadHoldingRegisters(1, 0x0000, 7, reg)) { ... } else { ...mb.LastError()... }
   * @endcode
   */
  class ModbusMaster {
  public:
    /// Send @p len bytes onto the bus. Must transmit all of them.
    using WriteFn = void (*)(const uint8_t *buf, size_t len);
    /// Return the next received byte, or a negative value if none is available yet.
    using ReadFn = int (*)();
    /// Return a free-running millisecond counter (e.g. Arduino millis).
    using MillisFn = uint32_t (*)();

    /**
     * @brief Construct a master bound to a byte transport.
     * @param wr            byte-block transmit function (sends the whole buffer).
     * @param rd            single-byte receive function; returns <0 when no byte
     *                      is currently available (non-blocking poll).
     * @param ms            millisecond clock used for receive timeouts.
     * @param timeout_ms    max time to wait for a complete reply (default 300 ms,
     *                      ample for a ~20-byte frame even at 4800 baud).
     */
    ModbusMaster(WriteFn wr, ReadFn rd, MillisFn ms, uint32_t timeout_ms = 300)
      : Write(wr), Read(rd), Millis(ms), TimeoutMs(timeout_ms), Error(nullptr) {}

    /**
     * @brief Read a contiguous block of holding registers (function code 0x03).
     * @param addr      slave address (1..247; 0xFF broadcast-query for some probes).
     * @param startReg  address of the first register to read.
     * @param count     number of 16-bit registers to read (1..125).
     * @param[out] out   caller buffer receiving @p count register values, decoded
     *                   big-endian (Modbus wire order) into host order.
     * @return true on a valid reply (matching address, function 0x03, correct byte
     *         count, CRC ok); false on timeout, framing/length mismatch, CRC error,
     *         or a Modbus exception response. On false, LastError() explains why.
     * @note Blocks (polling @p rd) until the reply arrives or @p timeout_ms elapses.
     * @note Tolerates a half-duplex echo of the request (auto-direction RS485): it
     *       resynchronises to the reply header rather than assuming the first bytes
     *       received are the response.
     */
    bool ReadHoldingRegisters(uint8_t addr, uint16_t startReg, uint16_t count, uint16_t *out) {
      Error = nullptr;
      if(count == 0 || count > 125) { Error = "bad register count"; return false; }

      // Build request: addr, func, startHi, startLo, countHi, countLo, crcLo, crcHi.
      uint8_t req[8];
      req[0] = addr;
      req[1] = 0x03;
      req[2] = uint8_t(startReg >> 8);
      req[3] = uint8_t(startReg);
      req[4] = uint8_t(count >> 8);
      req[5] = uint8_t(count);
      uint16_t crc = ModbusCrc(req, 6);
      req[6] = uint8_t(crc);        // CRC is sent low byte first
      req[7] = uint8_t(crc >> 8);

      DrainRx();
      Write(req, sizeof req);

      // Locate the reply header, skipping any half-duplex echo of the request. On an
      // auto-direction RS485 link (e.g. an XY-017) the transmitted frame is echoed
      // back on RX, also starting addr,0x03,... -- so slide a 3-byte window over the
      // stream until the exact data-reply header [addr, 0x03, 2*count] appears. The
      // echo's third byte is the request's start-reg-hi (0x00) and 2*count is even,
      // so neither the echo nor an exception's func (0x83, odd) can false-match it.
      const uint8_t nData = uint8_t(2 * count);
      uint8_t w1 = 0, w2 = 0, w3 = 0;
      uint32_t start = Millis();
      bool gotHeader = false;
      while(Millis() - start <= TimeoutMs) {
        int c = Read();
        if(c < 0) continue;
        start = Millis(); // reset deadline on progress
        w1 = w2; w2 = w3; w3 = uint8_t(c);
        if(w2 == addr && w3 == (0x03 | 0x80)) { Error = "modbus exception response"; return false; }
        if(w1 == addr && w2 == 0x03 && w3 == nData) { gotHeader = true; break; }
      }
      if(!gotHeader) { Error = "timeout waiting for reply"; return false; }

      // Header found; read the data + 2 CRC bytes and validate CRC over the whole frame.
      uint8_t frame[3 + 2 * 125 + 2]; // header + max data + CRC
      frame[0] = addr; frame[1] = 0x03; frame[2] = nData;
      if(!ReadBytes(frame + 3, nData + 2)) { Error = "timeout reading reply body"; return false; }

      uint16_t calc = ModbusCrc(frame, 3 + nData);
      uint16_t recv = uint16_t(frame[3 + nData]) | (uint16_t(frame[3 + nData + 1]) << 8);
      if(calc != recv) { Error = "CRC mismatch"; return false; }

      for(uint16_t i = 0; i < count; ++i)
        out[i] = (uint16_t(frame[3 + 2 * i]) << 8) | frame[3 + 2 * i + 1];
      return true;
    } // ReadHoldingRegisters

    /**
     * @brief Human-readable reason the last call failed.
     * @return a static string, or nullptr if the last call succeeded.
     */
    const char *LastError() const { return Error; }

    /**
     * @brief Compute the Modbus-RTU CRC16 over @p len bytes.
     * @param buf  bytes to checksum (the frame without its trailing CRC).
     * @param len  number of bytes.
     * @return the CRC16 (poly 0xA001, init 0xFFFF, reflected) in host order; on the
     *         wire it is appended low byte first.
     */
    static uint16_t ModbusCrc(const uint8_t *buf, size_t len) {
      return avp::Crc16(buf, (long long)len, 0xFFFF, avp::CRC16_MODBUS_POLY, true);
    }

  private:
    /**
     * @brief Discard any bytes already sitting in the receive transport.
     *
     * Clears stale/echoed bytes so a reply is not confused with leftovers from a
     * previous transaction.
     */
    void DrainRx() {
      while(Read() >= 0) { /* discard */ }
    }

    /**
     * @brief Block-read exactly @p n bytes into @p dst, honouring TimeoutMs.
     * @param[out] dst  destination buffer (at least @p n bytes).
     * @param n         number of bytes to read.
     * @return true if all @p n bytes arrived before the timeout; false otherwise.
     */
    bool ReadBytes(uint8_t *dst, size_t n) {
      uint32_t start = Millis();
      size_t got = 0;
      while(got < n) {
        int c = Read();
        if(c >= 0) {
          dst[got++] = uint8_t(c);
          start = Millis(); // reset deadline on progress (inter-byte)
        } else if(Millis() - start > TimeoutMs) {
          return false;
        }
      }
      return true;
    } // ReadBytes

    WriteFn Write;
    ReadFn Read;
    MillisFn Millis;
    uint32_t TimeoutMs;
    const char *Error;
  }; // class ModbusMaster

} // namespace avp

#endif // AVP_MODBUS_RTU_HPP_
