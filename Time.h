/**
  * @file
  * @author Alexander Panasyuk
  */

#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

#include <stdint.h>

namespace avp {
  //! @tparam T should be unsigned!
  template<uint32_t (*pTickFunction)(), typename T=uint32_t> class TimePeriod {
    T NextTime;
    const T Period;
   public:
    void Reset() { NextTime = (*pTickFunction)() + Period; }
    /**
    * @param Timeout in whatever units TickFunction works
    */
    TimePeriod(T Timeout):Period(Timeout) { Reset(); }
    bool Passed() {
      bool Out = (T((*pTickFunction)()) - NextTime) < ((~T(0))/2);
      if(Out) Reset();
      return Out;
    }
  }; // TimePeriod

  //! @tparam T should be unsigned!
  template<uint32_t (*TickFunction)(), typename T=uint32_t>
  class TimeOut: public TimePeriod<TickFunction,T> {
   public:
    TimeOut(T Timeout):TimePeriod<TickFunction,T>(Timeout)  {}
    operator bool() { return TimePeriod<TickFunction,T>::Passed();  }
  }; // TimeOut


}; // namespace avp

#endif /* TIME_H_INCLUDED */
