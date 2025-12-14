/**
    @file MyTime.h
    @author Alexander Panasyuk

*/

#pragma once

/// @cond
// millis() and micros() should defined beforehand and return microseconds and milliseconds

#include <stdint.h>
#include <limits.h>
#include <type_traits>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

/// @endcond
#include "General.hpp"

namespace avp {
  /// !!!! Periods and Delays should not be longer than half of value which fits into typename!
  typedef decltype(millis()) Time_t;
  static_assert(std::is_unsigned<Time_t>::value, "Time_t should be unsigned!");

  /**
     Using: avp::Timeout<> Time_t; while(something) { if(Time_t) { error('Timed out!); break; }}
     @tparam TickFunction, millis() by default
  */
  template<Time_t (*TickFunction)() = millis>
  class TimeOut {
    const Time_t When;
   public:
    TimeOut(Time_t Timeout) : When(TickFunction() + Timeout) { }
    operator bool() const {
      return unsigned_is_smaller(When, TickFunction());  // true when expired
    }
  }; // TimeOut

  template<Time_t (*TickFunction)() = millis>
  class TimeInterval {
    bool IsSet;
    Time_t End;
   public:
    TimeInterval(Time_t Interval = 0) : IsSet(false) {
       if(Interval) Start(Interval);
    }

    void Start(Time_t Interval) {
      End = TickFunction() + Interval;
      IsSet = true;
    } // Start

    bool Expired() {
      if(IsSet && unsigned_is_smaller(End, TickFunction())) {
        IsSet = false;
        return true;   // true when expired
      }
      return false;
    } // Expired
  }; // TimeInterval

  //! @tparam Time_t should be unsigned!
  template<Time_t (*TickFunction)() = millis>
  class TimePeriod {
    Time_t NextTime, Period;
   public:
    void Reset() {
      NextTime = TickFunction() + Period;
    }
    /**
      @param Timeout in whatever units TickFunction works
    */
    TimePeriod(Time_t Timeout, bool Expired = false) :Period(Timeout) { 
      if(Expired) NextTime = TickFunction(); else Reset(); 
    }
    /// just checks whether TimePeriod had passed, does not do reset
    /// if there is no Reset for too long the counter may wrap over
    bool JustCheck() const {
      auto t = TickFunction();
      return (NextTime == t) || unsigned_is_smaller(NextTime, t);
    }

    /// if TimePeriod had passed does reset to start a new TimePeriod
    bool Expired() {
      bool Out = JustCheck();
      if(Out) Reset();
      return Out;
    } // Passed

    explicit operator Time_t&() { return Period; }

    /// Pause with an internal loop
    static void Pause(Time_t Delay, void (*LoopFunc)()) {
      TimePeriod Timer(Delay);
      while(!Timer.Expired()) (*LoopFunc)();
    } // Pause

    static void Pause(Time_t Delay) {
      Pause(Delay, []() { });
    }
  }; // TimePeriod

  //! @tparam Time_t should be unsigned!
  template<Time_t Period, Time_t (*TickFunction)() = millis>
  class TimePeriod1 {
    Time_t NextTime;
   public:
    void Reset() {
      NextTime = TickFunction() + Period;
    }
    /**
      @param Timeout in whatever units TickFunction works
    */
    TimePeriod1() { Reset(); }
    /// just checks whether TimePeriod had passed, does not do reset
    /// if there is no Reset for too long the counter may wrap over
    bool JustCheck() const {
      return unsigned_is_smaller(NextTime, TickFunction());
    }

    /// if TimePeriod had passed does reset to start a new TimePeriod
    bool Expired() {
      bool Out = JustCheck();
      if(Out) Reset();
      return Out;
    } // Passed

    operator Time_t() { return Period; }

    /// Pause with an internal loop
    static void Pause(Time_t Delay, void (*LoopFunc)()) {
      TimePeriod Timer(Delay);
      while(!Timer.Expired()) (*LoopFunc)();
    } // Pause

    static void Pause(Time_t Delay) {
      Pause(Delay, []() { });
    }
  }; // TimePeriod1

  /**
    @brief we Run called in loop executes function with a given period
     Example:
    void loop() {
      avp::Periodically<SendData>::Run(1000);
    }
      When period is set to 0 the function does not run

     @tparam (*Func)()
  */
  template<void (*Func)(), Time_t(*TickFunction)() = millis>
  class Periodically {
    static Time_t NextTime;
   public:
   /**
    * @brief when called in the loop runs the function with a given period
    * @param Period - period in whatever units TickFunction works, Function is
    *                 not called if Period is 0
    */
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
    static void cycle() {
      if(TP.Expired()) (*Func)();  // cycle
    }
    static void Reset() { TP.Reset(); }
  }; // RunPeriodically

/// RunPeriodically may be static class because Func and Period make all of them different
  template<Time_t(*TickFunction)(), Time_t Period>
  class RunPeriodically1 {
    static TimePeriod1<Period, TickFunction> TP;
   public:
    static void cycle(void (*Func)()) {
      if(TP.Expired()) (*Func)();  // cycle
    }
    static void Reset() { TP.Reset(); }
  }; // RunPeriodically

  template<Time_t (*TickFunction)(), void (*Func)(), Time_t Period>
  TimePeriod<TickFunction> RunPeriodically<TickFunction, Func, Period>::TP(Period);

  template<Time_t (*TickFunction)(), Time_t Period>
  TimePeriod1<Period,TickFunction> RunPeriodically1<TickFunction, Period>::TP;

  typedef class TimePeriod<millis> Millisec;
  // typedef class TimePeriod<micros> Microsec;
  // typedef class avp::TimePeriod<CPU_Ticks> CPU_Tick;
  
  std::string getCurrentTimeFormatted(const char *Format);

}; // namespace avp

