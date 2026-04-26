#include "ui.hpp"
#include "lv_port_disp.hpp"
#include "lv_port_indev.hpp"
#include "screens/home_screen.hpp"
#include "screens/run_screen.hpp"
#include "screens/profile_screen.hpp"
#include "screens/settings_screen.hpp"
#include "screens/fault_screen.hpp"
#include <lvgl.h>
#include <Arduino.h>

namespace ui {

static lv_group_t* group_ = nullptr;

void UiTask::on_init() {
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();

  group_ = lv_group_create();
  lv_indev_set_group(lv_port_indev_get(), group_);

  screens::home_build(group_);
  go_to(Screen::HOME);
}

void UiTask::on_tick() {
  lv_tick_inc(period_ms());
  lv_task_handler();
}

void go_to(Screen s) {
  switch (s) {
    case Screen::HOME:         screens::home_show();         break;
    case Screen::PROFILES:     screens::profile_show_list(); break;
    case Screen::PROFILE_EDIT: screens::profile_show_edit(); break;
    case Screen::RUN:          screens::run_show();          break;
    case Screen::MANUAL:       /* TODO */                    break;
    case Screen::SETTINGS:     screens::settings_show();     break;
    case Screen::FAULT:        screens::fault_show();        break;
  }
}

UiTask& ui_task() {
  static UiTask instance;
  return instance;
}

}  // namespace ui
