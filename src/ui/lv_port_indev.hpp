#pragma once
#include <lvgl.h>

namespace ui {

// Rotary encoder → LVGL encoder indev. The encoder's ticks become
// LV_KEY_NEXT/PREV, the button becomes LV_KEY_ENTER. A long-press is
// mapped to back-navigation by the UI root.
void lv_port_indev_init();
lv_indev_t* lv_port_indev_get();

}  // namespace ui
