#include "system/serial.hpp"
#include <tusb.h>

TudSerial Serial;

int TudSerial::available() { return tud_cdc_available(); }

int TudSerial::read() {
    uint8_t b;
    return tud_cdc_read(&b, 1) ? (int)b : -1;
}

int TudSerial::peek() {
    uint8_t b;
    return tud_cdc_peek(&b) ? (int)b : -1;
}

void TudSerial::flush() { tud_cdc_write_flush(); }

TudSerial::operator bool() { return tud_cdc_connected(); }

size_t TudSerial::write(uint8_t c) {
    if (!tud_cdc_connected()) return 0;  // fire-and-forget when host isn't listening
    const size_t n = tud_cdc_write_char(c);
    tud_cdc_write_flush();
    return n;
}

size_t TudSerial::write(const uint8_t* buf, size_t n) {
    if (!tud_cdc_connected()) return 0;

    size_t sent = 0;
    while (sent < n) {
        const uint32_t avail = tud_cdc_write_available();
        if (avail == 0) {
            // Ring full: give the USB stack a chance to drain. Bail if the
            // host disconnects mid-write rather than blocking forever.
            tud_cdc_write_flush();
            tud_task();
            if (!tud_cdc_connected()) break;
            continue;
        }
        const size_t chunk = (n - sent < avail) ? (n - sent) : avail;
        sent += tud_cdc_write(buf + sent, chunk);
    }
    tud_cdc_write_flush();
    return sent;
}
