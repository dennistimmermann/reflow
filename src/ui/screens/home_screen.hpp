#pragma once
#include <lvgl.h>

namespace ui::screens {
void home_build(lv_group_t* g);    // one-time construction
void home_show();                  // called by ui::go_to(Screen::HOME)
}  // namespace ui::screens
