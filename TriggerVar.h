#ifndef TRIGGERINGVAR_H_INCLUDED
#define TRIGGERINGVAR_H_INCLUDED

// #include <AVP_LIBS/General/Macro.h>
#include "Macros.h"

template<typename T, void (* CallWhenAssigned)(const T &NewValue)>
class TriggerVar {
    T Value;
  public:
    explicit TriggerVar(T x) { Value = x; }
    operator const T&() const { return Value; }
    TriggerVar& operator=(T x) {
      CallWhenAssigned(Value = x);
      return *this;
    }
    CLASS_PLUS_MINUS_BLOCK(TriggerVar,+)
    CLASS_PLUS_MINUS_BLOCK(TriggerVar,-)
}; // TriggeringVar

#endif /* TRIGGERINGVAR_H_INCLUDED */
