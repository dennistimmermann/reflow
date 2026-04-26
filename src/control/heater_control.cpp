#include "heater_control.hpp"
#include "pid.hpp"
#include "../system/store.hpp"

namespace control {

// TODO: bind these two PIDs to the actual FET pins via a LoadMap.
static Pid pid_top   ({8.0f, 0.15f, 2.0f}, 0.0f, 1.0f);
static Pid pid_bottom({8.0f, 0.15f, 2.0f}, 0.0f, 1.0f);

void HeaterTask::on_init() {
  pid_top.reset(25.0f);
  pid_bottom.reset(25.0f);
  set_state(static_cast<uint32_t>(State::OFF));
  sys::store().duty_top.set(0.0f);
  sys::store().duty_bottom.set(0.0f);
}

void HeaterTask::on_tick() {
  if (state() == static_cast<uint32_t>(State::KILLED)) {
    sys::store().duty_top.set(0.0f);
    sys::store().duty_bottom.set(0.0f);
    return;
  }

  const auto& sp_t = sys::store().setpoint_top;
  const auto& sp_b = sys::store().setpoint_bottom;

  // No fresh setpoint published → idle.
  const bool any_fresh =
      sp_t.fresh_within(sys::freshness::kSetpointBudgetMs) ||
      sp_b.fresh_within(sys::freshness::kSetpointBudgetMs);
  if (!any_fresh) {
    set_state(static_cast<uint32_t>(State::OFF));
    sys::store().duty_top.set(0.0f);
    sys::store().duty_bottom.set(0.0f);
    return;
  }

  set_state(static_cast<uint32_t>(State::RUNNING));

  const float dt = 0.1f;   // matches our 100 ms period
  const float duty_top    = pid_top.step   (sp_t.get(), sys::store().tc_top.get(),    dt);
  const float duty_bottom = pid_bottom.step(sp_b.get(), sys::store().tc_bottom.get(), dt);
  sys::store().duty_top.set(duty_top);
  sys::store().duty_bottom.set(duty_bottom);
  // TODO: feed duty → slow-PWM windowed driver on the assigned FETs.
}

void HeaterTask::kill() {
  set_state(static_cast<uint32_t>(State::KILLED));
  sys::store().duty_top.set(0.0f);
  sys::store().duty_bottom.set(0.0f);
}

void HeaterTask::clear_kill() {
  if (state() == static_cast<uint32_t>(State::KILLED)) {
    set_state(static_cast<uint32_t>(State::OFF));
  }
}

HeaterTask& heater_task() {
  static HeaterTask instance;
  return instance;
}

}  // namespace control
