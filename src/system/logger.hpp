#pragma once
#include <Arduino.h>

namespace sys {

void logger_init();
void logger_drain();          // called periodically from the scheduler
void log_info(const char* fmt, ...);
void log_warn(const char* fmt, ...);
void log_err (const char* fmt, ...);

}  // namespace sys
