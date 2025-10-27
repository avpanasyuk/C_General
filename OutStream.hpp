#pragma once

#include <ostream>
#include <streambuf>

namespace avp {
  class OutStream : public std::ostream {
    friend class Buffer;
    virtual std::size_t Write(uint8_t *p, std::size_t s) = 0;

    class Buffer : public std::streambuf {
      OutStream &owner;

    protected:
      /// single-character write
      virtual int_type overflow(int_type c) override {
        if (c != EOF) {
          uint8_t ch = c;
          if (owner.Write(&ch, 1) == 0) return EOF;
        }
        return c;
      } // overflow
      /// bulk write
      virtual std::streamsize xsputn(const char *s, std::streamsize n) override {
        return n > 0 ? owner.Write((uint8_t *)s, static_cast<std::size_t>(n)) : 0;
      }

    public:
      Buffer(OutStream &owner_) : owner(owner_) {}
    } Buffer; // class Buffer
  public:
    OutStream() : std::ostream(&Buffer), Buffer(*this) {}
    virtual bool is_open() { return true; }
  }; // class OutStream
} // namespace avp
