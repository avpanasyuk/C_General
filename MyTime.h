/**
  * @file AVP_LIBS\General\Time.h
  * @author Alexander Panasyuk
  *
  */

#pragma once

  /// @cond
    // millis() and micros() should defined beforehand and return microseconds and milliseconds

#include <stdint.h>
#include <limits.h>
/// @endcond
#include "General.h"

extern uint32_t millis();
extern uint32_t micros();

namespace avp {
  /// !!!! Periods and Delays should not be longer than half of value which fits into typename!
  typedef decltype(millis()) Time_t;


  /**
   * Using: avp::Timeout<> Time_t; while(something) { if(Time_t) { error('Timed out!); break; }}
   * @tparam Time_t should be unsigned!
   */
  template<Time_t (*TickFunction)() = millis>
  class TimeOut {
    const Time_t When;
  public:
    TimeOut(Time_t Timeout) : When(TickFunction() + Timeout) { }
    operator bool() const { return unsigned_is_smaller(When, TickFunction()); } // true when expired
  }; // TimeOut

  //! @tparam Time_t should be unsigned!
  template<Time_t (*TickFunction)() = millis>
  class TimePeriod {
    Time_t NextTime, Period;
  public:
    void Reset() { NextTime = TickFunction() + Period; }
    /**
    * @param Timeout in whatever units TickFunction works
    */
    TimePeriod(Time_t Timeout) :Period(Timeout) {
      static_assert(Time_t(0) < Time_t(-1), "Time_t should be unsigned!");
      // AVP_ASSERT(Timeout < std::numeric_limits<Time_t>::max()/2);
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

    operator Time_t&() { return Period; }

    /// Pause with an internal loop
    static void Pause(Time_t Delay, void (*LoopFunc)()) {
      TimePeriod Timer(Delay);
      while(!Timer.Expired()) (*LoopFunc)();
    } // Pause

    static void Pause(Time_t Delay) { Pause(Delay, []() { }); }
  }; // TimePeriod

  /**
   *@brief we Run called in loop executes function with a given period
   * Example:
void loop() {
  avp::Periodically<SendData>::Run(1000);
}
   *
   * @tparam (*Func)()
   */
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
    static TimePeriod<TickFunction> TP;
  public:
    static void cycle() { if(TP.Expired()) (*Func)(); } // cycle
    static void Reset() { TP.Reset(); }
  }; // RunPeriodically

  template<Time_t(*TickFunction)(), void (*Func)(), Time_t Period>
  TimePeriod<TickFunction> RunPeriodically<TickFunction, Func, Period>::TP(Period);

  typedef class TimePeriod<millis> Millisec;
  typedef class TimePeriod<micros> Microsec;
  // typedef class avp::TimePeriod<CPU_Ticks> CPU_Tick;
}; // namespace avp

