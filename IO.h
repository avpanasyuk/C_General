#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

/**
  * @file AVP_LIBS/General/IO.h
  * @author Alexander Panasyuk
  * Generic functions for output. Templates using user supplied "write" or "put_byte" functions.
  */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "CircBufferWithCont.h"
#include "Time.h"

/// creates printf-type function named "func_name" out of vprintf-type function named "vprinf_func"
/// usage: PRINTF_WRAPPER_BOOL(printf,vprintf)
#define PRINTF_WRAPPER(func_name, vprintf_func) \
  int func_name(char const *format, ...) \
  { va_list ap; va_start(ap,format); \
    int Out =  vprintf_func(format,ap); va_end(ap); \
    return Out; }

namespace avp {
  // ************************** FORMATTED OUTPUT ***********************************
  // ************************* TEMPLATE VPRINTF-TYPE FUNCTION FROM WRITE
  // Note about following two functions - they should be used with BLOCKED pwrite which
  // returns only after data pointed by Ptr are used. Or it should make a copy.
  // following functions use general "write" function of type bool write(const void *Ptr, uint16_t Size) to do formatted output;
  // NOTE both functions do not write string-ending 0 !!!!!
  template<int (*pwrite)(const uint8_t *Ptr, int Size)>
  int vprintf(char const *format, va_list ap) {
    int Size = vsnprintf(NULL,0,format,ap);
    if(Size < 0) return false;
    uint8_t Buffer[Size+1]; // +1 to include ending zero byte
    vsprintf((char *)Buffer,format,ap);
    return (*pwrite)((const uint8_t *)Buffer,Size); // we do not write ending 0 byte
  } // vprintf

  template<void (*pwrite)(const uint8_t *Ptr, size_t Size)>
  void vprintf(char const *format, va_list ap) {
    int Size = vsnprintf(NULL,0,format,ap);
    if(Size < 0) {
      const char *Msg = "Bad message format!";
      (*pwrite)((const uint8_t *)Msg,strlen(Msg));
    } else {
      uint8_t Buffer[Size+1]; // +1 to include ending zero byte
      vsprintf((char *)Buffer,format,ap);
      (*pwrite)((const uint8_t *)Buffer,Size); // we do not write ending 0 byte
    }
  } // vprintf
  // ************************  TEMPLATE PRINTF-TYPE FUNCTION
  template<int (*vprintf)(char const *format, va_list ap)>
  PRINTF_WRAPPER(printf,vprintf)

  template<bool (* vprintf_func)(char const *format, va_list ap)>
  PRINTF_WRAPPER_BOOL(printf_bool,vprintf_func)
  template<void (* vprintf_func)(char const *format, va_list ap)>
  PRINTF_WRAPPER(printf,vprintf_func)
  template<bool (*pwrite)(const uint8_t *Ptr, size_t Size)>
  PRINTF_WRAPPER_BOOL(printf_bool,avp::vprintf_bool<pwrite>)
  template<void (*pwrite)(const uint8_t *Ptr, size_t Size)>
  PRINTF_WRAPPER(printf,avp::vprintf<pwrite>)


  #if 0
  // ****************************** UNFORMATTED OUTPUT ******************************
  /**
  * given a function to output a single byte this template class creates function to output arbitrary things
  * so we can create funny functions: printf<vprintf<Out<put_byte>::write> >
  * can be used as a superclass for thing which need to expand put_byte function
  */
  template<bool (*put_byte)(uint8_t)>
  struct Out {
    static bool write(const uint8_t *p, size_t sz) { for(; sz--;) if(!put_byte(*(p++))) return false; return true; }
    template<typename T> static bool write(T &obj) { return write((const uint8_t *)&obj,sizeof(obj)); }
    static bool write(const char *str) { return  write((const uint8_t *)str,strlen(str)); }
    static bool printf PRINTF_WRAPPER(vprintf<write>)
  }; // class Out

  // ********************************* DEBUG MESSAGES **************************************
  /// @brief default __weak__ version sends data to::printf
  bool debug_vprintf(const char *format, va_list a);

  // ************ BUFFERED BACKGROUND MESSAGES *********************************************
  /// this is a service class, we need it here only to be able to provide "put_byte" function for
  /// bg_messenger superclass "Out". Nothing from this class should be called by the user.
  /// we can not put it inside bg_messager class definition as we need function put in the very
  /// beginning of this definition
  template<bool (*write)(const uint8_t *Src, size_t Size), uint8_t BufferSizeLog2 = 8, typename TypeOfSize=uint8_t>
  struct bg_messager_private {
    static CircBufferWithCont<uint8_t,TypeOfSize,BufferSizeLog2> Buffer;

    static bool put(uint8_t b) { return Buffer.Write(b); }
    static void foreground_sendout() {
      while(Buffer.LeftToRead()) {
        write(Buffer.GetContinousBlockToRead(),Buffer.GetSizeToRead());
        Buffer.FinishedReading();
      }
    } // foreground_sendout
  }; // bg_messager_private

  /// this is user interface class for buffered background messages.
  /// it is using debug_write function for output, which can be redefined.
  /// all functions inherited from Out class ("printf" etc) store messages into a buffer and
  /// they are sent out when foreground_sendout is called.
  /// foreground_sendout should be called periodically, e.g. in main loop.
  template<bool (*write)(const uint8_t *Src, size_t Size), uint8_t BufferSizeLog2 = 8, typename TypeOfSize=uint8_t>
  struct bg_messager:
      public Out<bg_messager_private<write,BufferSizeLog2,TypeOfSize>::put>,
      public bg_messager_private<write,BufferSizeLog2,TypeOfSize>
  {}; // bg_messager

  template<bool (*write)(const uint8_t *Src, size_t Size), uint8_t BufferSizeLog2, typename TypeOfSize>
  CircBufferWithCont<uint8_t,TypeOfSize,BufferSizeLog2> bg_messager_private<write,BufferSizeLog2,TypeOfSize>::Buffer;


  /// following is a terrible kludge to use "printf" for "write".
  /// Useful sometimes...
  template<int (*printf)( const char * format, ... )>
  bool write(const uint8_t *Src, size_t Size) { return printf("%.*s",Size,Src) >= 0; }
  #endif

  /// in some of my classes "write" function handles errors itself and returns void, but "printf" and "vprintf"
  /// need function returning bool, so there is a wrapper (and vise versa)
  template<void (*write)(const uint8_t *Src, size_t Size)>
  bool make_true(const uint8_t *Src, size_t Size) { write(Src,Size); return true; }
  template<bool (*write)(const uint8_t *Src, size_t Size)>
  void ignore_bool(const uint8_t *Src, size_t Size) { (void)write(Src,Size); }

  template<bool (*write_func)(const uint8_t *Ptr, size_t Size),
           void (*loop_func)(), void (*fail_func)(), uint32_t Timeout = 1000, uint32_t (*TickFunction)() = millis>
  void write_with_timeout(const uint8_t *Ptr, size_t Size)  {
    TimeOut<TickFunction> T(Timeout); \
    while(!write_func(Ptr,Size)) { loop_func(); if(T) fail_func(); };
  } // write_with_timeout
} // namespace avp

#endif /* IO_H_INCLUDED */
