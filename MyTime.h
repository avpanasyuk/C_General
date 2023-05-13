/**
  * @file AVP_LIBS\General\Time.h
  * @author Alexander Panasyuk
  *
  */

#ifndef TIME_H_INCLUDED
#define TIME_H_INCLUDED

  /// @cond
    // millis() and micros() should defined beforehand and return microseconds and milliseconds

#include <stdint.h>
#include <limits.h>
/// @endcond
#include "General.h"

namespace avp {
  /// !!!! Periods and Delays should not be longer than half of value which fits into typename!
  typedef decltype(millis()) Time_t;

  //! @tparam T should be unsigned!
  template<typename T = Time_t, T(*TickFunction)() = millis>
  class TimeOut {
    const T When;
  public:
    TimeOut(T Timeout) : When(TickFunction() + Timeout) { }
    operator bool() const { return unsigned_is_smaller(When, TickFunction()); }
  }; // TimeOut

  //! @tparam T should be unsigned!
  template<typename T = Time_t, T(*TickFunction)() = millis>
  class TimePeriod {
    T NextTime, Period;
  public:
    void Reset() { NextTime = TickFunction() + Period; }
    /**
    * @param Timeout in whatever units TickFunction works
    */
    TimePeriod(T Timeout) :Period(Timeout) {
      static_assert(T(0) < T(-1), "T should be unsigned!");
      // AVP_ASSERT(Timeout < std::numeric_limits<T>::max()/2);
      Reset();
    }
    /// just checks whether TimePeriod had passed, does not do reset
    /// if there is no Reset for too long the counter may wrap over
    bool JustCheck() const { return unsigned_is_smaller(NextTime, TickFunction()); }

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

    static void Pause(T Delay) { Pause(Delay, []() { }); }
  }; // TimePeriod

  template<void (*Func)(), Time_t(*TickFunction)() = millis>
  class Periodically {
    static Time_t NextTime;
  public:
    static void Run(Time_t Period) {
      Time_t Now = TickFunction();
      if(unsigned_is_smaller(NextTime, Now)) {
        NextTime = Now + Period;
        if(Period != 0) (*Func)();
      }
    } // Run
  }; // Periodically

  template<void (*Func)(), Time_t(*TickFunction)()>
  Time_t Periodically<Func, TickFunction>::NextTime = TickFunction();

  /// RunPeriodically may be static class because Func and Period make all of them different
  template<Time_t(*TickFunction)(), void (*Func)(), Time_t Period>
  class RunPeriodically {
    static TimePeriod<Time_t, TickFunction> TP;
  public:
    static void cycle() { if(TP.Expired()) (*Func)(); } // cycle
    static void Reset() { TP.Reset(); }
  }; // RunPeriodically

  template<Time_t(*TickFunction)(), void (*Func)(), Time_t Period>
  TimePeriod<Time_t, TickFunction> RunPeriodically<TickFunction, Func, Period>::TP(Period);

  typedef class TimePeriod<Time_t, millis> Millisec;
  typedef class TimePeriod<Time_t, micros> Microsec;
  // typedef class avp::TimePeriod<CPU_Ticks> CPU_Tick;
}; // namespace avp

#endif /* TIME_H_INCLUDED */
