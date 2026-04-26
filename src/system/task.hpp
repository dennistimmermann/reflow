#pragma once
#include <Arduino.h>
#include <stdint.h>

namespace sys {

// Base class every periodic worker derives from. The Scheduler invokes
// on_tick() at fixed period_ms; everything else (state, transitions,
// logging hooks) is bookkeeping the subclass can use or ignore.
//
// State is held as a plain uint32_t so the base class can log it without
// knowing the subclass's enum type. Subclasses define their own
// `enum class State : uint32_t { ... }` and call set_state(State::X);
// on_state_enter/exit fire on every actual transition.
class Task {
 public:
  Task(const char* name, uint32_t period_ms)
      : name_(name), period_ms_(period_ms) {}
  virtual ~Task() = default;

  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;

  // Called once after construction, before the scheduler starts ticking.
  virtual void on_init() {}

  // Called every period_ms by the scheduler.
  virtual void on_tick() = 0;

  const char* name()      const { return name_; }
  uint32_t    period_ms() const { return period_ms_; }
  uint32_t    state()     const { return state_; }

 protected:
  void set_state(uint32_t next) {
    if (next == state_) return;
    on_state_exit(state_);
    const uint32_t prev = state_;
    state_ = next;
    on_state_enter(prev);
  }

  virtual void on_state_enter(uint32_t /*prev*/) {}
  virtual void on_state_exit (uint32_t /*next*/) {}

 private:
  const char* name_;
  uint32_t    period_ms_;
  uint32_t    state_ = 0;
};

// Adapter that lets us register a free function as a Task during the
// migration to the Task-based architecture. Delete this class once every
// scheduler client is a real Task subclass.
class LegacyTask : public Task {
 public:
  LegacyTask(const char* name, uint32_t period_ms, void (*fn)())
      : Task(name, period_ms), fn_(fn) {}

  void on_tick() override { if (fn_) fn_(); }

 private:
  void (*fn_)();
};

}  // namespace sys
