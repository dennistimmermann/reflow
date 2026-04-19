#pragma once

namespace ui {

void init();
void tick();      // pumps lv_task_handler at the scheduler's UI rate

// Navigation between top-level screens.
enum class Screen { HOME, PROFILES, PROFILE_EDIT, RUN, MANUAL, SETTINGS, FAULT };
void go_to(Screen s);

}  // namespace ui
