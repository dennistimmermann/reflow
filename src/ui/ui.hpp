#pragma once
#include "../system/task.hpp"

namespace ui {

// Drives LVGL: feeds lv_tick_inc and pumps lv_task_handler at the
// scheduler's UI rate. UI screens own their own widget state; this
// task is just the drain.
class UiTask : public sys::Task {
 public:
  UiTask() : sys::Task("ui", 10) {}
  void on_init() override;
  void on_tick() override;
};

UiTask& ui_task();

// Navigation between top-level screens.
enum class Screen { HOME, PROFILES, PROFILE_EDIT, RUN, MANUAL, SETTINGS, FAULT };
void go_to(Screen s);

}  // namespace ui
