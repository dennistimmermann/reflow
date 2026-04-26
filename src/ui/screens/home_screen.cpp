#include "home_screen.hpp"
#include "../../system/store.hpp"

namespace ui::screens {

static lv_obj_t* screen_ = nullptr;
static lv_obj_t* lbl_top_ = nullptr;
static lv_obj_t* lbl_bot_ = nullptr;
static lv_obj_t* lbl_tgt_ = nullptr;
static lv_timer_t* refresh_ = nullptr;
static lv_group_t* group_ = nullptr;

static void refresh_cb(lv_timer_t*) {
  if (!screen_) return;
  lv_label_set_text_fmt(lbl_top_, "Top %.0f°C",    sys::store().tc_top.get());
  lv_label_set_text_fmt(lbl_bot_, "Bot %.0f°C",    sys::store().tc_bottom.get());
  lv_label_set_text_fmt(lbl_tgt_, "Target %.0f°C", sys::store().tc_target.get());
}

void home_build(lv_group_t* g) {
  group_ = g;
  screen_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(screen_, lv_color_black(), 0);

  // Round-screen layout: content in a centred 170×170 inscribed square.
  lbl_tgt_ = lv_label_create(screen_); lv_obj_align(lbl_tgt_, LV_ALIGN_CENTER,  0, -30);
  lbl_top_ = lv_label_create(screen_); lv_obj_align(lbl_top_, LV_ALIGN_CENTER,  0,   0);
  lbl_bot_ = lv_label_create(screen_); lv_obj_align(lbl_bot_, LV_ALIGN_CENTER,  0,  30);

  refresh_ = lv_timer_create(refresh_cb, 250, nullptr);
}

void home_show() {
  lv_screen_load(screen_);
}

}  // namespace ui::screens
