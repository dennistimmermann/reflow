#include "fan_control.hpp"
#include "../system/store.hpp"

namespace control {

// Cool-phase ceiling: keep the fan on until the chamber is below this.
// Same value applies to reflow / anneal / bake.
static constexpr float kFanOffBelowC = 60.0f;

void FanTask::on_init() {
  set_state(static_cast<uint32_t>(State::OFF));
  sys::store().duty_fan.set(0.0f);
}

void FanTask::on_tick() {
  if (state() == static_cast<uint32_t>(State::MANUAL)) {
    // Manual duty is written elsewhere (UI manual-test screen). Don't clobber.
    return;
  }

  const auto& oven = sys::store().oven_state;
  const auto& tc_t = sys::store().tc_top;

  float duty = 0.0f;
  if (oven.valid()) {
    const auto s = oven.get();
    if (s == sys::OvenState::COOLING ||
        s == sys::OvenState::DONE) {
      // Run while still warm; otherwise drop to 0.
      const bool warm = tc_t.fresh_within(sys::freshness::kTcBudgetMs) &&
                        tc_t.get() > kFanOffBelowC;
      duty = warm ? 1.0f : 0.0f;
    }
  }

  set_state(static_cast<uint32_t>(duty > 0 ? State::AUTO : State::OFF));
  sys::store().duty_fan.set(duty);
  // TODO: feed duty → fan FET (LoadMap-assigned) with slow-PWM or on/off.
}

FanTask& fan_task() {
  static FanTask instance;
  return instance;
}

}  // namespace control
