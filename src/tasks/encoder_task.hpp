#pragma once
#include "../system/task.hpp"

namespace tasks {

// Polls the rotary encoder + button at 5 ms cadence. Quadrature decoding
// and short button debounce live in driver::RotaryEncoder; this task
// just translates "delta since last poll" into bus events and
// long-press detection.
class EncoderTask : public sys::Task {
 public:
  enum class State : uint32_t { IDLE = 0, PRESSED = 1, LONG_HELD = 2 };
  EncoderTask() : sys::Task("encoder", 5) {}
  void on_init() override;
  void on_tick() override;
};

EncoderTask& encoder_task();

}  // namespace tasks
