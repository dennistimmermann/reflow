#pragma once

// Force-included from platformio.ini via -include, so every translation
// unit sees `Serial` without needing an explicit #include. Payload is
// guarded because the C-only TUs inside TinyUSB and LVGL would choke on
// Print.h's `class` keyword.
//
// Deliberately does NOT pull in <tusb.h>: the Arduino variant TU lives in
// its own build group that doesn't get the TinyUSB include path, so every
// TinyUSB interaction stays out-of-line in tud_serial.cpp.

#ifdef __cplusplus

#include <Arduino.h>
#include <Print.h>

class TudSerial : public Print {
  public:
    void    begin(uint32_t = 0) {}     // baud ignored: CDC has no wire rate
    void    end() {}

    int     available();
    int     read();
    int     peek();
    void    flush() override;

    size_t  write(uint8_t c) override;
    size_t  write(const uint8_t* buf, size_t n) override;
    using   Print::write;

    explicit operator bool();
};

extern TudSerial Serial;

#endif  // __cplusplus
