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

void init() {
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();

  group_ = lv_group_create();
  lv_indev_set_group(lv_port_indev_get(), group_);

  screens::home_build(group_);
  go_to(Screen::HOME);
}

void tick() {
  lv_tick_inc(10);          // matches our scheduler period
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

}  // namespace ui
