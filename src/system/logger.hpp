#pragma once
#include <Arduino.h>
#include "task.hpp"

namespace sys {

void logger_init();
void log_info(const char* fmt, ...);
void log_warn(const char* fmt, ...);
void log_err (const char* fmt, ...);

// Drains pending log output to USB CDC. Currently a no-op (Serial.write
// pushes into TinyUSB's CDC ring without blocking) but kept as an explicit
// task so we have a place to add backpressure handling without touching
// every call site.
class LoggerDrainTask : public Task {
 public:
  LoggerDrainTask() : Task("logger", 20) {}
  void on_tick() override;
};

}  // namespace sys
