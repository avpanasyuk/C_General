#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

/**
  * @file AVP_LIBS/General/IO.h
  * @author Alexander Panasyuk
  */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "CircBuffer.h"

namespace avp {
  // following functions use general "write" function of type bool write(const void *Ptr, uint16_t Size) to do formatted output;
  // NOTE both functions do not write string-ending 0 !!!!!
  // extern bool vprintf(bool (*pwrite)(volatile void *Ptr, size_t Size),char const *format, va_list ap);
  // extern bool printf(bool (*pwrite)(volatile void *Ptr, size_t Size), char const *format, ...);
// Note about following two functions - they should be used with BLOCKED pwrite which
  // returns only after data pointed by Ptr are used. Or it makes a copy.

  // following functions use general "write" function of type bool write(const void *Ptr, uint16_t Size) to do formatted output;
  // NOTE both functions do not write string-ending 0 !!!!!
  template<bool (*pwrite)(const uint8_t *Ptr, size_t Size)> bool vprintf(char const *format, va_list ap) {
    int Size = vsnprintf(NULL,0,format,ap);
    if(Size < 0) return false;
    uint8_t Buffer[Size+1]; // +1 to include ending zero byte
    vsprintf((char *)Buffer,format,ap);
    return (*pwrite)((const uint8_t *)Buffer,Size); // we do not write ending 0 byte
  } // vprintf

  template<bool (*vprintf)(char const *format, va_list ap)> bool printf(char const *format, ...) {
    va_list ap;
    va_start(ap,format);
    bool Out =  vprintf(format,ap);
    va_end(ap);
    return Out;
  } // printf

  template<bool (*pwrite)(const uint8_t *Ptr, size_t Size)> bool printf(char const *format, ...) {
    va_list ap;
    va_start(ap,format);
    bool Out =  vprintf<pwrite>(format,ap);
    va_end(ap);
    return Out;
  } // printf

  /**
  * given a function to output a single byte this template class creates function to output arbitrary things
  */
  template<bool (*put_byte)(uint8_t)>
  struct Out {
    static bool write(const uint8_t *p, size_t sz) { for(; sz--;) if(!put_byte(*(p++))) return false; return true; }
    static bool write(const char *str) { return  write((const uint8_t *)str,strlen(str)); }
    template<typename T> static bool write(T &obj) { return write((const uint8_t *)&obj,sizeof(obj)); }
    static bool printf(char const *format, ...) {
      va_list ap;
      va_start(ap,format);
      bool Out =  vprintf<write>(format,ap);
      va_end(ap);
      return Out;
    } // printf
  }; // class Out

  bool debug_write(const uint8_t *Src, size_t Size); // defined week, should be replaced if stderr is not good enough
  // template<uint8_t BufferSizeLog2, typename TypeOfSize> bool bg_message<BufferSizeLog2,TypeOfSize>::put(uint8_t b);

  template<uint8_t BufferSizeLog2 = 8, typename TypeOfSize=uint8_t>
  struct bg_message_mgr {
    static_assert(sizeof(TypeOfSize)*8 >= BufferSizeLog2,"bg_message:Size variable is too small to hold size!");

    static CircBuffer<uint8_t,TypeOfSize,BufferSizeLog2> Buffer;

    static bool put(uint8_t b) { if(!Buffer.LeftToWrite()) return false; Buffer.Write(b); return true; }
    static void send() {
      if(Buffer.LeftToRead()) {
        uint8_t Sz;
        const uint8_t *p = Buffer.GetContinousBlockToRead(&Sz);
        const char *msg = "Error in bg_message::Buffer!\n";
        if(p == nullptr) debug_write((const uint8_t *)msg,strlen(msg));
        else debug_write(p,Sz);
        Buffer.FinishedReading();
      }
    } // send
  }; // bg_message_mgr

  template<uint8_t BufferSizeLog2 = 8, typename TypeOfSize=uint8_t>
  struct bg_message: public Out<bg_message_mgr<BufferSizeLog2,TypeOfSize>::put> {
    static void send() { bg_message_mgr<BufferSizeLog2,TypeOfSize>::send(); }
  }; // bg_message

  template<uint8_t BufferSizeLog2, typename TypeOfSize>
  CircBuffer<uint8_t,TypeOfSize,BufferSizeLog2> bg_message_mgr<BufferSizeLog2,TypeOfSize>::Buffer;


} // namespace avp

#endif /* IO_H_INCLUDED */
