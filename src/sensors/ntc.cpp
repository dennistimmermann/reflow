#include "ntc.hpp"
#include <Arduino.h>
#include <math.h>

namespace sensors {

// Steinhart-Hart constants for a 10k NTC β=3950 @25°C. Refine once the
// populated NTC part is confirmed.
static constexpr float kBeta       = 3950.0f;
static constexpr float kR0         = 10000.0f;    // NTC resistance at T0
static constexpr float kT0_K       = 298.15f;     // 25 °C
static constexpr float kR_PULLUP   = 10000.0f;    // R7 on the schematic
static constexpr float kVref       = 3.3f;

static float last_c = 25.0f;

void ntc_init() {
  pinMode(PIN_NTC, INPUT_ANALOG);
  analogReadResolution(12);
}

void ntc_tick() {
  // 16× oversample → ~14-bit effective.
  uint32_t acc = 0;
  for (int i = 0; i < 16; ++i) acc += analogRead(PIN_NTC);
  const float v = (acc / 16.0f) * (kVref / 4095.0f);
  if (v <= 0.01f || v >= kVref - 0.01f) return;     // open / short
  const float r_ntc = kR_PULLUP * (v / (kVref - v));
  const float inv_T = (1.0f / kT0_K) + (1.0f / kBeta) * logf(r_ntc / kR0);
  last_c = (1.0f / inv_T) - 273.15f;
}

float ntc_celsius() { return last_c; }

}  // namespace sensors
