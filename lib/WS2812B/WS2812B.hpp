#pragma once
// Single-pixel WS2812B status LED driver.
//
// NOTE: WS2812 bit timing is 1.25 µs per bit ±150 ns. On STM32G0 the reliable
// approach is either a SPI-MOSI-based encoder (one SPI byte per WS2812 bit) or
// a TIM+DMA PWM chain. This single-file stub uses a pin-bitbang placeholder
// while interrupts are disabled — fine for a single pixel infrequent update,
// but replace with the SPI/DMA path before production.
//
// Single-file inline-header library per CLAUDE.md §5.

#include <Arduino.h>

namespace driver {

class WS2812B {
 public:
  struct Rgb { uint8_t r, g, b; };

  explicit WS2812B(uint32_t pin) : pin_(pin) {}

  void begin() {
    pinMode(pin_, OUTPUT);
    digitalWrite(pin_, LOW);
  }

  // Quick and dirty: works for a one-off write on a ≥64 MHz MCU with
  // interrupts off. Do not call often; replace with DMA path ASAP.
  inline void set(Rgb c) {
    // GRB order.
    const uint8_t bytes[3] = { c.g, c.r, c.b };
    noInterrupts();
    for (uint8_t B : bytes) {
      for (uint8_t i = 0; i < 8; ++i) {
        if (B & 0x80) {
          digitalWrite(pin_, HIGH);
          __asm__ volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
          digitalWrite(pin_, LOW);
          __asm__ volatile ("nop\n nop\n nop\n");
        } else {
          digitalWrite(pin_, HIGH);
          __asm__ volatile ("nop\n nop\n");
          digitalWrite(pin_, LOW);
          __asm__ volatile ("nop\n nop\n nop\n nop\n nop\n nop\n");
        }
        B <<= 1;
      }
    }
    interrupts();
    // Latch: ≥50 µs low.
    delayMicroseconds(60);
  }

 private:
  uint32_t pin_;
};

}  // namespace driver
