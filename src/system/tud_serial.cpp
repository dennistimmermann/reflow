#include "system/tud_serial.hpp"

// Global Serial instance. Exposed via `extern TudSerial Serial;` in the
// header. With STM32duino's USB CDC stack removed (HWSERIAL_NONE +
// lib_ignore USBDevice in platformio.ini) there's no symbol conflict.
TudSerial Serial;

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
            // Ring full: give the USB stack a chance to drain. A cold host
            // that stopped reading would block us forever here, so bail if
            // it disconnects mid-write.
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
