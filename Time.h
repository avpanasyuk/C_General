/**
  * @file AVP_LIBS\General\Time.h
  * @author Alexander Panasyuk
  *
  */

#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

/// @cond
  // millis() and micros() should defined beforehand and return microseconds and milliseconds

#include <stdint.h>#include <limits.h>
/// @endcond
#include "General.h"
namespace avp {  /// !!!! Periods and Delays should not be longer than half of value which fits into typename!
  typedef decltype(micros()) Time_t;
  
  //! @tparam T should be unsigned!
  template<Time_t (*pTickFunction)(), typename T=Time_t>
  class TimePeriod {
      T NextTime;
      T Period;
    public:
      void Reset() { NextTime = T((*pTickFunction)()) + Period; }
      /**
      * @param Timeout in whatever units TickFunction works
      */
      TimePeriod(T Timeout):Period(Timeout) {
        static_assert(T(0) < T(-1),"T should be unsigned!");        // AVP_ASSERT(Timeout < std::numeric_limits<T>::max()/2);
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
  template<Time_t (*TickFunction)(), typename T=Time_t>
  class TimeOut: public TimePeriod<TickFunction,T> {
    public:
      TimeOut(T Timeout):TimePeriod<TickFunction,T>(Timeout)  {}
      operator bool() { return TimePeriod<TickFunction,T>::Expired();  }
  }; // TimeOut

  /// RunPeriodically may be static class because Func and Period make all of them different
  template<Time_t (*TickFunction)(), void (*Func)(), Time_t Period>
  class RunPeriodically {
      static TimePeriod<TickFunction, Time_t> TP;
    public:
      static void cycle() { if(TP.Expired()) (*Func)(); } // cycle
      static void Reset() { TP.Reset(); }
  }; // RunPeriodically

  template<Time_t (*TickFunction)(), void (*Func)(), Time_t Period>
  TimePeriod<TickFunction, Time_t> RunPeriodically<TickFunction,Func,Period>::TP(Period);

}; // namespace avp


typedef class avp::TimePeriod<millis> Millisec;
typedef class avp::TimePeriod<micros> Microsec;
// typedef class avp::TimePeriod<CPU_Ticks> CPU_Tick;

#endif /* TIME_H_INCLUDED */
