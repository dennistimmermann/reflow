#pragma once
#include "../system/task.hpp"

namespace tasks {

// Drives the WS2812B status LED. Reads oven_state + fault_flags from the
// Store and chooses an animation. Subscribes to FaultTripped so faults
// take effect within one tick instead of waiting on Store polling.
class StatusLedTask : public sys::Task {
 public:
  enum class State : uint32_t {
    BOOT_FLASH      = 0,
    IDLE_BREATHE    = 1,
    RUNNING_PULSE   = 2,
    FAULT_BLINK     = 3,
  };
  StatusLedTask() : sys::Task("status_led", 50) {}
  void on_init() override;
  void on_tick() override;
  void set_state_internal(State s);   // for the FaultTripped subscriber
};

StatusLedTask& status_led_task();

}  // namespace tasks
