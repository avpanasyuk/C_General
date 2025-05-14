#pragma once

#ifdef ARDUINO

#include <Arduino.h> // millis and micros are defined in Arduino.h

#endif

#if defined(__linux__) && __linux__

#include <sys/time.h>

time_t millis();
time_t micros();

#endif

