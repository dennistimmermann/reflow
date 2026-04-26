#pragma once
#include "../system/task.hpp"

namespace control {

// Fan controller. Reads oven_state + tc_top from Store; writes duty_fan.
// AUTO is automatic during cool-down; MANUAL is reserved for the UI's
// manual-test screen (drives a manual-override duty in step 5).
class FanTask : public sys::Task {
 public:
  enum class State : uint32_t { OFF = 0, AUTO = 1, MANUAL = 2 };
  FanTask() : sys::Task("fan", 100) {}
  void on_init() override;
  void on_tick() override;
};

FanTask& fan_task();

}  // namespace control
