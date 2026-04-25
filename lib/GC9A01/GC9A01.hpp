#pragma once
// GC9A01 240×240 round LCD driver — minimal init + windowed blit for LVGL.
// Assumed controller; confirm against the module datasheet (the 15-pin FPC on
// J11 is consistent with GC9A01 modules on AliExpress/LCSC). If it's an
// ST7789-round instead, swap the init table below — the rest is identical.
//
// SPI access goes through driver::SpiDmaTx — bulk pixel writes are DMA, small
// command/header writes are polled. CS/DC/RST/BL are GPIOs.
//
// Single-file inline-header library per CLAUDE.md §5.

#include <Arduino.h>
#include <SpiDmaTx.hpp>

namespace driver {

class GC9A01 {
 public:
  GC9A01(SpiDmaTx& bus, uint32_t cs, uint32_t dc, uint32_t rst, uint32_t bl)
    : bus_(bus), cs_(cs), dc_(dc), rst_(rst), bl_(bl) {}

  void begin() {
    pinMode(cs_, OUTPUT);  digitalWrite(cs_,  HIGH);
    pinMode(dc_, OUTPUT);  digitalWrite(dc_,  HIGH);
    pinMode(rst_, OUTPUT); digitalWrite(rst_, HIGH);
    pinMode(bl_, OUTPUT);  digitalWrite(bl_,  LOW);

    // SPI1 on STM32G0B1: APB=64 MHz, /2 prescaler = 32 MHz (the ceiling).
    bus_.begin(SPI_BAUDRATEPRESCALER_2);

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

  // RGB565, MSB-first. Length is in pixels. DMA transfer.
  inline void blit(const uint16_t* pixels, uint32_t count) {
    digitalWrite(dc_, HIGH);
    digitalWrite(cs_, LOW);
    bus_.transmit_dma(reinterpret_cast<const uint8_t*>(pixels), count * 2);
    bus_.wait_idle();
    digitalWrite(cs_, HIGH);
  }

  // Solid-colour fill of the full 240×240 frame via DMA. A half-frame buffer
  // (120 rows = 57,600 bytes) is pre-filled and DMA'd out twice — only two
  // CPU round-trips per fill instead of 240. A full-frame single DMA isn't
  // possible: DMA NDTR is 16-bit (max 65,535 bytes) and the frame is 115,200,
  // so two transfers is the minimum for any buffer choice.
  inline void fill_screen(uint16_t color565) {
    static constexpr uint32_t kHalfFrameBytes = 240 * 120 * 2;  // 57,600
    static uint8_t buf[kHalfFrameBytes];
    const uint8_t hi = color565 >> 8;
    const uint8_t lo = color565 & 0xFF;
    for (uint32_t i = 0; i < sizeof(buf); i += 2) {
      buf[i]     = hi;
      buf[i + 1] = lo;
    }
    set_window(0, 0, 239, 239);
    digitalWrite(dc_, HIGH);
    digitalWrite(cs_, LOW);
    bus_.transmit_dma(buf, sizeof(buf));
    bus_.wait_idle();
    bus_.transmit_dma(buf, sizeof(buf));
    bus_.wait_idle();
    digitalWrite(cs_, HIGH);
  }

 private:
  void hard_reset() {
    digitalWrite(rst_, LOW);  delay(10);
    digitalWrite(rst_, HIGH); delay(120);
  }

  inline void write_cmd(uint8_t c) {
    digitalWrite(dc_, LOW);
    digitalWrite(cs_, LOW);
    bus_.transmit(&c, 1);
    digitalWrite(cs_, HIGH);
  }

  inline void write_data8(uint8_t d) {
    digitalWrite(dc_, HIGH);
    digitalWrite(cs_, LOW);
    bus_.transmit(&d, 1);
    digitalWrite(cs_, HIGH);
  }

  inline void write_data16(uint16_t d) {
    const uint8_t buf[2] = { uint8_t(d >> 8), uint8_t(d & 0xFF) };
    digitalWrite(dc_, HIGH);
    digitalWrite(cs_, LOW);
    bus_.transmit(buf, 2);
    digitalWrite(cs_, HIGH);
  }

  // Canonical GC9A01 init from the controller datasheet / reference modules.
  // The bulk are undocumented vendor "inner registers" (0x80..0x9F, 0xE8, 0xEB,
  // 0xED, 0xEF, 0xF0..0xF3 etc.) that configure power, gamma and VCOM. Without
  // them the panel powers on with the backlight lit but stays black — exactly
  // why the abbreviated init didn't work. MADCTL=0x48 selects MX+BGR (so the
  // datasheet's RGB565 bytes show as the expected colour); flip bit 3 to 0x40
  // if red and blue come out swapped on the panel module you have.
  void run_init_sequence() {
    // Format: [cmd, n_args, args...]. End-of-table marker is cmd=0x00 with n=0.
    static const uint8_t kInit[] = {
      0xEF, 0,
      0xEB, 1, 0x14,
      0xFE, 0,                                 // inner register enable 1
      0xEF, 0,                                 // inner register enable 2
      0xEB, 1, 0x14,
      0x84, 1, 0x40,
      0x85, 1, 0xFF,
      0x86, 1, 0xFF,
      0x87, 1, 0xFF,
      0x88, 1, 0x0A,
      0x89, 1, 0x21,
      0x8A, 1, 0x00,
      0x8B, 1, 0x80,
      0x8C, 1, 0x01,
      0x8D, 1, 0x01,
      0x8E, 1, 0xFF,
      0x8F, 1, 0xFF,
      0xB6, 2, 0x00, 0x00,                     // display function control
      0x36, 1, 0x48,                           // MADCTL: MX, BGR
      0x3A, 1, 0x05,                           // COLMOD: 16 bpp / RGB565
      0x90, 4, 0x08, 0x08, 0x08, 0x08,
      0xBD, 1, 0x06,
      0xBC, 1, 0x00,
      0xFF, 3, 0x60, 0x01, 0x04,
      0xC3, 1, 0x13,                           // power control 2
      0xC4, 1, 0x13,                           // power control 3
      0xC9, 1, 0x22,                           // power control 4
      0xBE, 1, 0x11,
      0xE1, 2, 0x10, 0x0E,
      0xDF, 3, 0x21, 0x0C, 0x02,
      0xF0, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,   // gamma 1
      0xF1, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,   // gamma 2
      0xF2, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,   // gamma 3
      0xF3, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,   // gamma 4
      0xED, 2, 0x1B, 0x0B,
      0xAE, 1, 0x77,
      0xCD, 1, 0x63,
      0x70, 9, 0x07, 0x07, 0x04, 0x0E, 0x0F, 0x09, 0x07, 0x08, 0x03,
      0xE8, 1, 0x34,                           // frame rate
      0x62, 12, 0x18, 0x0D, 0x71, 0xED, 0x70, 0x70, 0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70,
      0x63, 12, 0x18, 0x11, 0x71, 0xF1, 0x70, 0x70, 0x18, 0x13, 0x71, 0xF3, 0x70, 0x70,
      0x64, 7,  0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07,
      0x66, 10, 0x3C, 0x00, 0xCD, 0x67, 0x45, 0x45, 0x10, 0x00, 0x00, 0x00,
      0x67, 10, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x01, 0x54, 0x10, 0x32, 0x98,
      0x74, 7,  0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00,
      0x98, 2,  0x3E, 0x07,
      0x35, 0,                                 // tearing effect line ON
      0x21, 0,                                 // display inversion ON
      0x00, 0,                                 // end marker
    };

    for (size_t i = 0; i < sizeof(kInit); ) {
      const uint8_t cmd = kInit[i++];
      const uint8_t n   = kInit[i++];
      if (cmd == 0x00 && n == 0) break;
      write_cmd(cmd);
      for (uint8_t j = 0; j < n; ++j) write_data8(kInit[i++]);
    }

    write_cmd(0x11); delay(120);               // sleep out
    write_cmd(0x29); delay(20);                // display on
  }

  SpiDmaTx& bus_;
  uint32_t  cs_, dc_, rst_, bl_;
};

}  // namespace driver
