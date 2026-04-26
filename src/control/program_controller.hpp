#pragma once
#include "../system/task.hpp"
#include "thermal_profile.hpp"

namespace control {

// High-level oven program runner. Walks a ThermalProfile, writes
// setpoint_top/bottom and oven_state into the Store. Used for reflow,
// annealing, and plain bake — same task, different profile.
class ProgramTask : public sys::Task {
 public:
  ProgramTask() : sys::Task("program", 250) {}
  void on_init() override;
  void on_tick() override;

  void  start(const ThermalProfile& p);
  void  abort();
  float elapsed_s() const;
};

ProgramTask& program_task();

}  // namespace control
