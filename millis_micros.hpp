#pragma once

#include <stdint.h>

#ifdef ESP32
#include <esp32-hal.h>
//#include "esp_timer.h"

// uint32_t micros() {
//     return (uint32_t) esp_timer_get_time();
// }

// uint32_t millis() {
//     return (uint32_t) (esp_timer_get_time() / 1000ULL);
// }
#endif

#ifdef ESP8266
// uint32_t micros() {
//     return system_get_time();
// }

// uint32_t millis() {
//     return system_get_time() / 1000;
// }
#endif

#ifndef NO_STL
#include <chrono>

#if !defined(ESP8266) && !defined(ESP32) && !defined(ARDUINO)
  // On Arduino-flavored platforms, millis()/micros() are provided by the core with
  // signature `unsigned long (*)(void)`. Declaring them again with a different return
  // type produces an ambiguating-declaration error. Only declare on bare platforms.
  uint32_t millis();
  uint32_t micros();
#endif

#endif
