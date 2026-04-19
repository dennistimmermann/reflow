#pragma once
// Non-blocking piezo buzzer driver. PWM-based tone generation on a timer pin.
// Uses Arduino `tone()` under the hood on STM32Duino, which schedules a
// hardware timer and returns immediately. `stop_at_` is checked by the app's
// periodic tick via update().
//
// Single-file inline-header library per CLAUDE.md §5.

#include <Arduino.h>

namespace driver {

class Buzzer {
 public:
  explicit Buzzer(uint32_t pin) : pin_(pin) {}

  void begin() {
    pinMode(pin_, OUTPUT);
    digitalWrite(pin_, LOW);
  }

  inline void tone_ms(uint16_t hz, uint16_t ms) {
    ::tone(pin_, hz);
    stop_at_ = millis() + ms;
    active_ = true;
  }

  inline void beep()      { tone_ms(2000, 80); }
  inline void chirp_up()  { tone_ms(1500, 60); /* TODO schedule follow-up */ }
  inline void mute()      { ::noTone(pin_); active_ = false; }

  // Call from the scheduler tick.
  inline void update() {
    if (active_ && static_cast<int32_t>(millis() - stop_at_) >= 0) mute();
  }

 private:
  uint32_t pin_;
  uint32_t stop_at_ = 0;
  bool active_ = false;
};

}  // namespace driver
