#pragma once
#include "task.hpp"

namespace sys {

// Debounced flash-EEPROM writer. Other modules call mark_dirty() when
// settings change; the task waits for the debounce window to elapse,
// then flushes via storage_write(). Storage itself is still a stub
// (see storage.cpp) — this task is the structure waiting for it.
class PersistenceTask : public Task {
 public:
  enum class State : uint32_t {
    CLEAN          = 0,
    DIRTY_DEBOUNCE = 1,
    WRITING        = 2,
  };
  PersistenceTask() : Task("persistence", 1000) {}
  void on_init() override;
  void on_tick() override;

  void mark_dirty();
};

PersistenceTask& persistence_task();

}  // namespace sys
