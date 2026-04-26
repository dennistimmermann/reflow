#pragma once
#include "../system/task.hpp"

namespace control {

// Watchdog-style supervisor. Reads sensor slots through the Store with
// strict freshness budgets — a stale slot is itself a fault. On any
// trip, latches into TRIPPED, writes fault_flags into the Store, and
// publishes FaultTripped on the event bus. Stays latched until the UI
// posts FaultAcknowledged.
class SafetyTask : public sys::Task {
 public:
  enum class State : uint32_t { NORMAL = 0, TRIPPED = 1 };
  SafetyTask() : sys::Task("safety", 100) {}
  void on_init() override;
  void on_tick() override;

  // Public so the FaultAcknowledged subscriber (registered in on_init)
  // can reset us. Calling this when not TRIPPED is a no-op.
  void set_state_normal();
};

SafetyTask& safety_task();

// Convenience for callers that just want a yes/no.
bool  safety_ok();

}  // namespace control
