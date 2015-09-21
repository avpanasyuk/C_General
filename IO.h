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

/// creates printf-type function out of vprintf
/// usage: bool printf PRINTF_WRAPPER(vprintf)
#define PRINTF_WRAPPER(vprintf_func) (char const *format, ...) \
     { va_list ap; va_start(ap,format); \
    bool Out =  vprintf_func(format,ap); va_end(ap); \
    return Out; }


namespace avp {
  // ************************** FORMATTED OUTPUT ***********************************
  // Note about following two functions - they should be used with BLOCKED pwrite which
  // returns only after data pointed by Ptr are used. Or it should make a copy.
  // following functions use general "write" function of type bool write(const void *Ptr, uint16_t Size) to do formatted output;
  // NOTE both functions do not write string-ending 0 !!!!!
  template<bool (*pwrite)(const uint8_t *Ptr, size_t Size)>
  bool vprintf(char const *format, va_list ap) {
    int Size = vsnprintf(NULL,0,format,ap);
    if(Size < 0) return false;
    uint8_t Buffer[Size+1]; // +1 to include ending zero byte
    vsprintf((char *)Buffer,format,ap);
    return (*pwrite)((const uint8_t *)Buffer,Size); // we do not write ending 0 byte
  } // vprintf

  template<bool (*vprintf)(char const *format, va_list ap)>
  bool printf PRINTF_WRAPPER(vprintf)

  template<bool (*pwrite)(const uint8_t *Ptr, size_t Size)>
  bool printf PRINTF_WRAPPER(vprintf<pwrite>)

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
        const uint8_t *p = Buffer.GetContinousBlockToRead();
        const char *msg = "Error in bg_message::foreground_sendout!\n";
        if(p == nullptr) write((const uint8_t *)msg,strlen(msg));
        else write(p,Buffer.GetSizeToRead());
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

  /// in some of my classes "write" function handles errors itself and returns void, but "printf" and "vprintf"
  /// need function returning bool, so there is a wrapper
  template<void (*write)(const uint8_t *Src, size_t Size)>
  bool make_true(const uint8_t *Src, size_t Size) { write(Src,Size); return true; }

  /// following is a terrible kludge to use "printf" for "write".
  /// Useful sometimes...
  template<int (*printf)( const char * format, ... )>
  bool write(const uint8_t *Src, size_t Size) { return printf("%.*s",Size,Src) >= 0; }
} // namespace avp

#endif /* IO_H_INCLUDED */
