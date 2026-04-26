#include "encoder_task.hpp"
#include "../system/event_bus.hpp"
#include "../system/store.hpp"
#include <RotaryEncoder.hpp>
#include <Arduino.h>

namespace tasks {

static driver::RotaryEncoder encoder(PIN_ROT_A, PIN_ROT_B, PIN_ROT_BTN);

// Long-press threshold; matches "press, then back" UX.
static constexpr uint32_t kLongPressMs = 500;

static int32_t  cumulative_pos = 0;
static uint32_t press_started_ms = 0;
static bool     long_press_fired = false;
static bool     button_held      = false;

void EncoderTask::on_init() {
  encoder.begin();
  set_state(static_cast<uint32_t>(State::IDLE));
  sys::store().encoder_position.set(0);
}

void EncoderTask::on_tick() {
  // Rotation
  const int8_t delta = encoder.poll_delta();
  if (delta != 0) {
    cumulative_pos += delta;
    sys::store().encoder_position.set(cumulative_pos);
    sys::bus().encoder_rotate.publish(sys::EncoderRotate{delta});
  }

  // Button: driver::RotaryEncoder::pressed() returns true on the falling
  // edge with debounce. We additionally track hold duration here so a
  // long-press fires LongPress without a trailing Press.
  if (encoder.pressed()) {
    button_held      = true;
    press_started_ms = millis();
    long_press_fired = false;
    set_state(static_cast<uint32_t>(State::PRESSED));
  }

  if (button_held) {
    const uint32_t held_ms = millis() - press_started_ms;
    if (!long_press_fired && held_ms >= kLongPressMs) {
      long_press_fired = true;
      set_state(static_cast<uint32_t>(State::LONG_HELD));
      sys::bus().encoder_long_press.publish(sys::EncoderLongPress{
        static_cast<uint16_t>(held_ms),
      });
    }

    // Detect release. The driver returns falling-edge presses; for
    // release we just sample the pin level.
    if (digitalRead(PIN_ROT_BTN) == HIGH) {
      const bool was_long = long_press_fired;
      button_held = false;
      set_state(static_cast<uint32_t>(State::IDLE));
      if (!was_long) {
        sys::bus().encoder_press.publish(sys::EncoderPress{});
      }
    }
  }
}

EncoderTask& encoder_task() {
  static EncoderTask instance;
  return instance;
}

}  // namespace tasks
