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
    TimePeriod(T Timeout):Period(Timeout) {
      static_assert(T(0) < T(-1),"T should be unsigned!");
      Reset();
    }
    bool Passed() {
      bool Out = (T((*pTickFunction)()) - NextTime) < ((~T(0))/2);
      if(Out) Reset();
      return Out;
    }
    static void Pause(T Delay) {
      TimePeriod Timer(Delay);
      while(!Timer.Passed());
    } // Pause
  }; // TimePeriod

  //! @tparam T should be unsigned!
  template<uint32_t (*TickFunction)(), typename T=uint32_t>
  class TimeOut: public TimePeriod<TickFunction,T> {
  public:
    TimeOut(T Timeout):TimePeriod<TickFunction,T>(Timeout)  {}
    operator bool() { return TimePeriod<TickFunction,T>::Passed();  }
  }; // TimeOut

  template<typename TickType, TickType (*TickFunction)(), void (*Func)(), TickType Period>
  class RunPeriodically {
    static TimePeriod<TickFunction, uint32_t> TP;
  public:
    static void cycle() { if(TP.Passed()) (*Func)(); } // cycle
    static void Reset() { TP.Reset(); }
  }; // RunPeriodically

  template<typename TickType, TickType (*TickFunction)(), void (*Func)(), TickType Period>
  TimePeriod<TickFunction, uint32_t> RunPeriodically<TickType,TickFunction,Func,Period>::TP(Period);
}; // namespace avp

#endif /* TIME_H_INCLUDED */
