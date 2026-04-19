#pragma once
// Quadrature rotary encoder with pushbutton. EXTI-driven on both channels plus
// a simple 4-state Gray-code decoder. Button uses a short software debounce.
//
// Single-file inline-header library per CLAUDE.md §5.

#include <Arduino.h>

namespace driver {

class RotaryEncoder {
 public:
  RotaryEncoder(uint32_t pin_a, uint32_t pin_b, uint32_t pin_btn)
    : pin_a_(pin_a), pin_b_(pin_b), pin_btn_(pin_btn) {}

  void begin() {
    pinMode(pin_a_,   INPUT_PULLUP);
    pinMode(pin_b_,   INPUT_PULLUP);
    pinMode(pin_btn_, INPUT_PULLUP);
    last_ab_ = read_ab();
    btn_prev_ = digitalRead(pin_btn_);
  }

  // Call often (e.g. every 1–2 ms, or from EXTI ISRs). Returns net rotation
  // since the last call (-N..+N detents).
  inline int8_t poll_delta() {
    const uint8_t ab = read_ab();
    int8_t d = 0;
    // Gray-code transition table. Each valid step is ±1 quarter, accumulate
    // four to yield one detent.
    static const int8_t lut[16] = { 0,-1, 1, 0, 1, 0, 0,-1,-1, 0, 0, 1, 0, 1,-1, 0 };
    d = lut[(last_ab_ << 2) | ab];
    last_ab_ = ab;
    quarter_ += d;
    int8_t detents = 0;
    while (quarter_ >=  4) { quarter_ -= 4; ++detents; }
    while (quarter_ <= -4) { quarter_ += 4; --detents; }
    return detents;
  }

  // Edge-triggered; returns true once per press.
  inline bool pressed() {
    const uint8_t now = digitalRead(pin_btn_);
    const uint32_t t = millis();
    if (now != btn_prev_ && (t - btn_edge_ms_) > 5) {
      btn_edge_ms_ = t;
      btn_prev_ = now;
      return now == LOW;
    }
    return false;
  }

 private:
  inline uint8_t read_ab() const {
    return (digitalRead(pin_a_) << 1) | digitalRead(pin_b_);
  }
  uint32_t pin_a_, pin_b_, pin_btn_;
  uint8_t  last_ab_ = 0;
  int8_t   quarter_ = 0;
  uint8_t  btn_prev_ = HIGH;
  uint32_t btn_edge_ms_ = 0;
};

}  // namespace driver
