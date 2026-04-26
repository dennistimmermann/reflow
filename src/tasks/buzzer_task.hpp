#pragma once
#include "../system/task.hpp"

namespace tasks {

// Owns a driver::Buzzer instance. Subscribes to BuzzerPlay events and
// drives the piezo non-blocking. The 10 ms tick keeps the driver's
// internal note advancer running between events.
class BuzzerTask : public sys::Task {
 public:
  enum class State : uint32_t { IDLE = 0, PLAYING = 1, BETWEEN_NOTES = 2 };
  BuzzerTask() : sys::Task("buzzer", 10) {}
  void on_init() override;
  void on_tick() override;
};

BuzzerTask& buzzer_task();

}  // namespace tasks
