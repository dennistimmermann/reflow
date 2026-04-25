#pragma once
#include <Arduino.h>
#include <array>

namespace sys {

// Cooperative fixed-period task runner. Not a real RTOS — tasks must return
// quickly. Jitter depends on the slowest task in the loop.
class Scheduler {
 public:
  struct Task {
    uint32_t interval;
    void (*fn)();
    uint32_t next_due = 0;
  };

  static constexpr size_t kMaxTasks = 16;

  void add(Task t) {
    if (count_ >= kMaxTasks) return;
    t.next_due = millis() + t.interval;
    tasks_[count_++] = t;
  }

  void run() {
    const uint32_t now = millis();
    for (size_t i = 0; i < count_; ++i) {
      auto& t = tasks_[i];
      // Signed compare handles millis() wrap cleanly.
      if (static_cast<int32_t>(now - t.next_due) >= 0) {
        t.fn();
        t.next_due += t.interval;
        // If we fell badly behind (e.g. LVGL flush), resync rather than
        // burst-fire the task to catch up.
        if (static_cast<int32_t>(now - t.next_due) > static_cast<int32_t>(t.interval)) {
          t.next_due = now + t.interval;
        }
      }
    }
  }

 private:
  std::array<Task, kMaxTasks> tasks_{};
  size_t count_ = 0;
};

}  // namespace sys
