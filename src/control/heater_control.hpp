#pragma once
#include "../system/task.hpp"

namespace control {

// PID + slow-PWM update loop. Reads setpoint_{top,bottom} and tc_{top,bottom}
// from Store; writes duty_{top,bottom}. Goes to KILLED on safety::all_off().
class HeaterTask : public sys::Task {
 public:
  enum class State : uint32_t { OFF = 0, RUNNING = 1, KILLED = 2 };
  HeaterTask() : sys::Task("heater", 100) {}
  void on_init() override;
  void on_tick() override;

  void kill();         // hard cutoff — called by safety
  void clear_kill();   // user ack via UI
};

// Singleton accessor for cross-module callers (safety, UI ack handler).
// Replaced by EventBus subscriptions in step 4.
HeaterTask& heater_task();

}  // namespace control
