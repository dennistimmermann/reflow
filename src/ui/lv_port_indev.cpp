#include "lv_port_indev.hpp"
#include "../board.hpp"
#include <RotaryEncoder.hpp>

namespace ui {

static driver::RotaryEncoder enc(board::PIN_ROT_A, board::PIN_ROT_B, board::PIN_ROT_BTN);
static lv_indev_t* indev_ = nullptr;
static int32_t enc_accum_ = 0;

static void read_cb(lv_indev_t*, lv_indev_data_t* data) {
  enc_accum_ += enc.poll_delta();
  data->enc_diff = static_cast<int16_t>(enc_accum_);
  enc_accum_ = 0;
  data->state = enc.pressed() ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

void lv_port_indev_init() {
  enc.begin();
  indev_ = lv_indev_create();
  lv_indev_set_type(indev_, LV_INDEV_TYPE_ENCODER);
  lv_indev_set_read_cb(indev_, read_cb);
}

lv_indev_t* lv_port_indev_get() { return indev_; }

}  // namespace ui
