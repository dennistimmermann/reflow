#pragma once

// Print-derived replacement for Arduino's `Serial` backed by TinyUSB CDC.
// All existing Serial.print / printf / println call sites resolve through
// this class once the global `Serial` below is linked in.

#include <Arduino.h>
#include <Print.h>
#include <tusb.h>

class TudSerial : public Print {
  public:
    void    begin(uint32_t = 0) {}     // baud ignored: CDC has no wire rate
    void    end() {}

    int     available() { return tud_cdc_available(); }
    int     read()      { uint8_t b; return tud_cdc_read(&b, 1) ? (int)b : -1; }
    int     peek()      { uint8_t b; return tud_cdc_peek(&b)   ? (int)b : -1; }
    void    flush() override { tud_cdc_write_flush(); }

    size_t  write(uint8_t c) override;
    size_t  write(const uint8_t* buf, size_t n) override;
    using   Print::write;

    explicit operator bool() { return tud_cdc_connected(); }
};

extern TudSerial Serial;
