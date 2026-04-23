#include "logger.hpp"
#include "system/tud_serial.hpp"
#include <cstdarg>
#include <cstdio>

namespace sys {

// USB / CDC setup lives in main.cpp (usb_bsp_init + tusb_init). Nothing to
// do here; kept as a no-op so the scheduler hook stays symmetric.
void logger_init() {}

void logger_drain() {
  // no-op for now; Serial.write pushes into TinyUSB's CDC ring without blocking.
}

static void emit(const char* prefix, const char* fmt, va_list ap) {
  char buf[128];
  vsnprintf(buf, sizeof(buf), fmt, ap);
  Serial.print(prefix);
  Serial.println(buf);
}

void log_info(const char* fmt, ...) { va_list ap; va_start(ap, fmt); emit("[I] ", fmt, ap); va_end(ap); }
void log_warn(const char* fmt, ...) { va_list ap; va_start(ap, fmt); emit("[W] ", fmt, ap); va_end(ap); }
void log_err (const char* fmt, ...) { va_list ap; va_start(ap, fmt); emit("[E] ", fmt, ap); va_end(ap); }

}  // namespace sys
