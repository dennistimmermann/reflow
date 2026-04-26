#include "persistence_task.hpp"
#include "storage.hpp"
#include <Arduino.h>

namespace sys {

// Wait this long after the last mark_dirty() before flushing — collapses
// rapid edits (e.g. PID-tune sliders) into one write.
static constexpr uint32_t kDebounceMs = 5000;

static uint32_t last_dirty_ms = 0;

void PersistenceTask::on_init() {
  set_state(static_cast<uint32_t>(State::CLEAN));
}

void PersistenceTask::on_tick() {
  const auto cur = static_cast<State>(state());
  if (cur == State::CLEAN) return;

  if (cur == State::DIRTY_DEBOUNCE) {
    if (millis() - last_dirty_ms >= kDebounceMs) {
      set_state(static_cast<uint32_t>(State::WRITING));
      // TODO: iterate dirty keys and call storage_write() for each.
      // The stub returns false; real impl is in src/system/storage.cpp.
      set_state(static_cast<uint32_t>(State::CLEAN));
    }
  }
}

void PersistenceTask::mark_dirty() {
  last_dirty_ms = millis();
  set_state(static_cast<uint32_t>(State::DIRTY_DEBOUNCE));
}

PersistenceTask& persistence_task() {
  static PersistenceTask instance;
  return instance;
}

}  // namespace sys
