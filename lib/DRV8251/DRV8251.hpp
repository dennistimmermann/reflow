#pragma once
// TI DRV8251A — H-bridge motor driver, IN/IN control mode.
// We wire IN1 → PB5 (TIM3_CH2) and IN2 → PB6. In IN/IN mode:
//   IN1=PWM, IN2=0 → forward at duty
//   IN1=0,  IN2=PWM → reverse at duty
//   IN1=1,  IN2=1  → brake (short OUT1/OUT2 to GND)
//   IN1=0,  IN2=0  → coast (high-Z)
//
// Datasheet: https://www.ti.com/product/DRV8251A
//
// Single-file inline-header library per CLAUDE.md §5.

#include <Arduino.h>

namespace driver {

class DRV8251 {
 public:
  enum class Direction { FORWARD, REVERSE };

  DRV8251(uint32_t in1, uint32_t in2) : in1_(in1), in2_(in2) {}

  void begin() {
    pinMode(in1_, OUTPUT); pinMode(in2_, OUTPUT);
    coast();
  }

  inline void drive(float duty, Direction dir) {
    if (duty < 0) duty = 0; else if (duty > 1) duty = 1;
    const uint32_t pwm = static_cast<uint32_t>(duty * 255.0f);
    if (dir == Direction::FORWARD) {
      analogWrite(in1_, pwm);
      digitalWrite(in2_, LOW);
    } else {
      digitalWrite(in1_, LOW);
      analogWrite(in2_, pwm);
    }
  }

  inline void brake() {
    digitalWrite(in1_, HIGH);
    digitalWrite(in2_, HIGH);
  }

  inline void coast() {
    digitalWrite(in1_, LOW);
    digitalWrite(in2_, LOW);
  }

 private:
  uint32_t in1_, in2_;
};

}  // namespace driver
