#pragma once
// GC9A01 240×240 round LCD driver — minimal init + windowed blit for LVGL.
// Assumed controller; confirm against the module datasheet (the 15-pin FPC on
// J11 is consistent with GC9A01 modules on AliExpress/LCSC). If it's an
// ST7789-round instead, swap the init table below — the rest is identical.
//
// SPI1 @ up to 40 MHz. CS/DC/RST/BL are GPIOs. Blitting is synchronous here;
// LVGL's lv_port_disp is responsible for DMA + flush_cb signalling.
//
// Single-file inline-header library per CLAUDE.md §5.

#include <Arduino.h>
#include <SPI.h>

namespace driver {

class GC9A01 {
 public:
  GC9A01(SPIClass& spi, uint32_t cs, uint32_t dc, uint32_t rst, uint32_t bl)
    : spi_(spi), cs_(cs), dc_(dc), rst_(rst), bl_(bl) {}

  void begin() {
    pinMode(cs_, OUTPUT);  digitalWrite(cs_,  HIGH);
    pinMode(dc_, OUTPUT);  digitalWrite(dc_,  HIGH);
    pinMode(rst_, OUTPUT); digitalWrite(rst_, HIGH);
    pinMode(bl_, OUTPUT);  digitalWrite(bl_,  LOW);

    hard_reset();
    run_init_sequence();
    digitalWrite(bl_, HIGH);
  }

  inline void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    write_cmd(0x2A);
    write_data16(x0); write_data16(x1);
    write_cmd(0x2B);
    write_data16(y0); write_data16(y1);
    write_cmd(0x2C);
  }

  // RGB565, MSB-first. Length is in pixels.
  inline void blit(const uint16_t* pixels, uint32_t count) {
    digitalWrite(dc_, HIGH);
    digitalWrite(cs_, LOW);
    spi_.beginTransaction(SPISettings(kSpiHz, MSBFIRST, SPI_MODE0));
    spi_.transfer((const uint8_t*)pixels, count * 2);
    spi_.endTransaction();
    digitalWrite(cs_, HIGH);
  }

 private:
  static constexpr uint32_t kSpiHz = 40'000'000;

  void hard_reset() {
    digitalWrite(rst_, LOW);  delay(10);
    digitalWrite(rst_, HIGH); delay(120);
  }

  inline void write_cmd(uint8_t c) {
    digitalWrite(dc_, LOW);
    digitalWrite(cs_, LOW);
    spi_.beginTransaction(SPISettings(kSpiHz, MSBFIRST, SPI_MODE0));
    spi_.transfer(c);
    spi_.endTransaction();
    digitalWrite(cs_, HIGH);
  }

  inline void write_data8(uint8_t d) {
    digitalWrite(dc_, HIGH);
    digitalWrite(cs_, LOW);
    spi_.beginTransaction(SPISettings(kSpiHz, MSBFIRST, SPI_MODE0));
    spi_.transfer(d);
    spi_.endTransaction();
    digitalWrite(cs_, HIGH);
  }

  inline void write_data16(uint16_t d) {
    digitalWrite(dc_, HIGH);
    digitalWrite(cs_, LOW);
    spi_.beginTransaction(SPISettings(kSpiHz, MSBFIRST, SPI_MODE0));
    spi_.transfer(d >> 8);
    spi_.transfer(d & 0xFF);
    spi_.endTransaction();
    digitalWrite(cs_, HIGH);
  }

  void run_init_sequence() {
    // Abbreviated GC9A01 init — exit sleep, MADCTL, COLMOD (RGB565), display on.
    // Full magic-register sequence goes here; omitted in the scaffold to keep
    // the file short. Fill in from the GC9A01 datasheet / AliExpress demo
    // before first bring-up.
    write_cmd(0x11); delay(120);
    write_cmd(0x36); write_data8(0x48);        // MADCTL: MX, BGR
    write_cmd(0x3A); write_data8(0x05);        // COLMOD: 16-bit/pixel
    write_cmd(0x29); delay(20);                // Display ON
  }

  SPIClass& spi_;
  uint32_t cs_, dc_, rst_, bl_;
};

}  // namespace driver
