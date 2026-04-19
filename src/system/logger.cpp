#include "logger.hpp"
#include <cstdarg>
#include <cstdio>

namespace sys {

// Minimal stub: forwards to Serial (USB CDC). A ring-buffered non-blocking
// drain will replace this once USB enumeration is verified on hardware.
void logger_init() {
  Serial.begin(115200);
}

void logger_drain() {
  // no-op for now; Serial.print is already non-blocking with the CDC core.
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
