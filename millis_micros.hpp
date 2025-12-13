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
uint32_t micros() {
    return system_get_time();
}

uint32_t millis() {
    return system_get_time() / 1000;
}
#endif

#ifndef NO_STL
#include <chrono>

  uint32_t millis();
  uint32_t micros();

#endif
