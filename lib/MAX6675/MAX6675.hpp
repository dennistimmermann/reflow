#pragma once
// MAX6675 cold-junction-compensated K-type thermocouple reader.
// 12-bit resolution, 0.25 °C LSB, 16-bit SPI frame (mode 0, clock ≤4.3 MHz).
//
// Datasheet: https://www.analog.com/en/products/max6675.html
//
// Single-file inline-header library per the project convention (see CLAUDE.md §5).
// Shared-SPI usage: construct one instance per chip, all pointing at the same
// SPIClass; CS multiplexing is handled in-class.

#include <Arduino.h>
#include <SPI.h>

namespace driver {

class MAX6675 {
 public:
  struct Reading {
    float celsius;
    bool  open_tc;    // true when T- is not connected
  };

  MAX6675(SPIClass& spi, uint32_t cs_pin)
    : spi_(spi), cs_(cs_pin) {}

  void begin() {
    pinMode(cs_, OUTPUT);
    digitalWrite(cs_, HIGH);
  }

  // Blocking read; takes ~1 µs once the MAX6675's 170–220 ms conversion has
  // completed. Call at most ~4 Hz per chip.
  inline Reading read() {
    spi_.beginTransaction(SPISettings(kSpiHz, MSBFIRST, SPI_MODE0));
    digitalWrite(cs_, LOW);
    const uint8_t hi = spi_.transfer(0x00);
    const uint8_t lo = spi_.transfer(0x00);
    digitalWrite(cs_, HIGH);
    spi_.endTransaction();

    const uint16_t raw = (static_cast<uint16_t>(hi) << 8) | lo;
    const bool open_tc = (raw & 0x0004) != 0;
    const uint16_t data = (raw >> 3) & 0x0FFF;      // 12-bit temperature
    return Reading{ data * 0.25f, open_tc };
  }

 private:
  static constexpr uint32_t kSpiHz = 4'000'000;    // datasheet max 4.3 MHz
  SPIClass& spi_;
  uint32_t cs_;
};

}  // namespace driver
