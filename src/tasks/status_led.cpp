#include "status_led.hpp"
#include "../system/store.hpp"
#include "../system/event_bus.hpp"
#include <WS2812B.hpp>
#include <Arduino.h>
#include <math.h>

namespace tasks {

static driver::WS2812B led(PIN_WS2812);

// Boot animation: two ~80 ms flashes, then go to IDLE_BREATHE.
static constexpr uint32_t kBootFlashTotalMs = 800;

// Maps `valid && fresh` oven state to an animation. State writes to
// the Store from ProgramTask are how the LED knows the oven is active.
static StatusLedTask::State pick_state() {
  const auto& fault = sys::store().fault_flags;
  if (fault.valid() && fault.get() != 0) return StatusLedTask::State::FAULT_BLINK;

  const auto& oven = sys::store().oven_state;
  if (!oven.valid()) return StatusLedTask::State::IDLE_BREATHE;
  switch (oven.get()) {
    case sys::OvenState::RUNNING: return StatusLedTask::State::RUNNING_PULSE;
    case sys::OvenState::FAULT:   return StatusLedTask::State::FAULT_BLINK;
    default:                      return StatusLedTask::State::IDLE_BREATHE;
  }
}

void StatusLedTask::on_init() {
  led.begin();
  led.set({0, 0, 0});
  set_state(static_cast<uint32_t>(State::BOOT_FLASH));

  // Faults skip BOOT_FLASH and pin us into FAULT_BLINK immediately.
  sys::bus().fault_tripped.subscribe([](const sys::FaultTripped&) {
    status_led_task().set_state_internal(State::FAULT_BLINK);
  });
}

void StatusLedTask::on_tick() {
  // Phase counter: 50 ms ticks since the current state was entered.
  // Animation math reads this; transitions reset it via set_state.
  static uint32_t phase_ms = 0;
  phase_ms += period_ms();

  const auto cur = static_cast<State>(state());

  switch (cur) {
    case State::BOOT_FLASH: {
      const bool on = ((phase_ms / 100) % 2) == 0 && (phase_ms < 400);
      led.set(on ? driver::WS2812B::Rgb{20, 20, 20} : driver::WS2812B::Rgb{0, 0, 0});
      if (phase_ms >= kBootFlashTotalMs) {
        set_state(static_cast<uint32_t>(pick_state()));
        phase_ms = 0;
      }
      return;
    }
    case State::IDLE_BREATHE: {
      // Slow sinusoidal breathe in cool blue.
      const float phase = (phase_ms % 4000) / 4000.0f * 6.28318f;
      const uint8_t v = static_cast<uint8_t>((sinf(phase) * 0.5f + 0.5f) * 30);
      led.set({0, 0, v});
      break;
    }
    case State::RUNNING_PULSE: {
      // Faster orange pulse.
      const float phase = (phase_ms % 1500) / 1500.0f * 6.28318f;
      const uint8_t v = static_cast<uint8_t>((sinf(phase) * 0.5f + 0.5f) * 60);
      led.set({v, static_cast<uint8_t>(v / 3), 0});
      break;
    }
    case State::FAULT_BLINK: {
      const bool on = ((phase_ms / 200) % 2) == 0;
      led.set(on ? driver::WS2812B::Rgb{80, 0, 0} : driver::WS2812B::Rgb{0, 0, 0});
      break;
    }
  }

  // Re-evaluate desired state every tick (cheap) — handles oven_state
  // transitions even when nothing publishes an event.
  if (cur != State::BOOT_FLASH) {
    const auto want = pick_state();
    if (want != cur) {
      set_state(static_cast<uint32_t>(want));
      phase_ms = 0;
    }
  }
}

void StatusLedTask::set_state_internal(State s) {
  set_state(static_cast<uint32_t>(s));
}

StatusLedTask& status_led_task() {
  static StatusLedTask instance;
  return instance;
}

}  // namespace tasks
