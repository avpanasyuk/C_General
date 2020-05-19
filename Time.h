/**
  * @file AVP_LIBS\General\Time.h
  * @author Alexander Panasyuk
  *
  */

#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

#include <stdint.h>
#include "General.h"
namespace avp {
  //! @tparam T should be unsigned!
  template<uint32_t (*pTickFunction)(), typename T=uint32_t>
  class TimePeriod {
      T NextTime;
      T Period;
    public:
      void Reset() { NextTime = (*pTickFunction)() + Period; }
      /**
      * @param Timeout in whatever units TickFunction works
      */
      TimePeriod(T Timeout):Period(Timeout) {
        static_assert(T(0) < T(-1),"T should be unsigned!");
        Reset();
      }
      /// just checks whether TimePeriod had passed, does not do reset
      /// if there is no Reset for too long the counter may wrap over
      bool JustCheck() const { return unsigned_is_smaller(NextTime,T((*pTickFunction)())); }

      /// if TimePeriod had passed does reset to start a new TimePeriod
      bool Expired() {
        bool Out = JustCheck();
        if(Out) Reset();
        return Out;
      } // Passed

      operator T&() { return Period; }

      /// Pause with an internal loop
      static void Pause(T Delay, void (*LoopFunc)()) {
        TimePeriod Timer(Delay);
        while(!Timer.Expired()) (*LoopFunc)();
      } // Pause

      static void Pause(T Delay) { Pause(Delay, []() {}); }
  }; // TimePeriod

  //! @tparam T should be unsigned!
  template<uint32_t (*TickFunction)(), typename T=uint32_t>
  class TimeOut: public TimePeriod<TickFunction,T> {
    public:
      TimeOut(T Timeout):TimePeriod<TickFunction,T>(Timeout)  {}
      operator bool() { return TimePeriod<TickFunction,T>::Expired();  }
  }; // TimeOut

  /// RunPeriodically may be static class because Func and Period make all of them different
  template<uint32_t (*TickFunction)(), void (*Func)(), uint32_t Period>
  class RunPeriodically {
      static TimePeriod<TickFunction, uint32_t> TP;
    public:
      static void cycle() { if(TP.Expired()) (*Func)(); } // cycle
      static void Reset() { TP.Reset(); }
  }; // RunPeriodically

  template<uint32_t (*TickFunction)(), void (*Func)(), uint32_t Period>
  TimePeriod<TickFunction, uint32_t> RunPeriodically<TickFunction,Func,Period>::TP(Period);

}; // namespace avp

// following functions should be defined elsewhere and return microseconds and milliseconds
extern uint32_t millis();
extern uint32_t micros();
typedef class avp::TimePeriod<millis> Millisec;
typedef class avp::TimePeriod<micros> Microsec;
// typedef class avp::TimePeriod<CPU_Ticks> CPU_Tick;

#endif /* TIME_H_INCLUDED */
