#pragma once
#include <Arduino.h>
#include <array>
#include "task.hpp"

namespace sys {

// Cooperative fixed-period task runner. Not a real RTOS — tasks must return
// quickly. Jitter depends on the slowest task in the loop.
class Scheduler {
 public:
  static constexpr size_t kMaxTasks = 16;

  void add(Task& t) {
    if (count_ >= kMaxTasks) return;
    auto& slot = slots_[count_++];
    slot.task = &t;
    slot.next_due = millis() + t.period_ms();
  }

  void run() {
    const uint32_t now = millis();
    for (size_t i = 0; i < count_; ++i) {
      auto& slot = slots_[i];
      const uint32_t period = slot.task->period_ms();
      // Signed compare handles millis() wrap cleanly.
      if (static_cast<int32_t>(now - slot.next_due) >= 0) {
        slot.task->on_tick();
        slot.next_due += period;
        // If we fell badly behind (e.g. LVGL flush), resync rather than
        // burst-fire the task to catch up.
        if (static_cast<int32_t>(now - slot.next_due) > static_cast<int32_t>(period)) {
          slot.next_due = now + period;
        }
      }
    }
  }

 private:
  struct Slot {
    Task*    task     = nullptr;
    uint32_t next_due = 0;
  };
  std::array<Slot, kMaxTasks> slots_{};
  size_t count_ = 0;
};

}  // namespace sys
