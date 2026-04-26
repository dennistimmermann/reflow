#include "program_controller.hpp"
#include "door_control.hpp"
#include "safety.hpp"
#include "../system/store.hpp"
#include <Arduino.h>

namespace control {

static const ThermalProfile* profile_ = nullptr;
static uint32_t              start_ms_ = 0;

void ProgramTask::on_init() {
  set_state(static_cast<uint32_t>(sys::OvenState::IDLE));
  sys::store().oven_state.set(sys::OvenState::IDLE);
}

void ProgramTask::on_tick() {
  // Pre-step 4: still polling safety_ok() directly. Step 4 swaps this for
  // a FaultTripped subscription.
  if (!safety_ok()) {
    set_state(static_cast<uint32_t>(sys::OvenState::FAULT));
    sys::store().oven_state.set(sys::OvenState::FAULT);
    return;
  }

  const auto cur = static_cast<sys::OvenState>(state());
  if (cur == sys::OvenState::IDLE ||
      cur == sys::OvenState::DONE ||
      cur == sys::OvenState::FAULT) return;
  if (!profile_) return;

  bool done = false;
  const float t      = (millis() - start_ms_) / 1000.0f;
  const float target = profile_target_at(*profile_, t, &done);

  sys::store().setpoint_top.set(target);
  sys::store().setpoint_bottom.set(target);

  if (done) {
    sys::store().setpoint_top.set(0.0f);
    sys::store().setpoint_bottom.set(0.0f);
    door_request_open();
    set_state(static_cast<uint32_t>(sys::OvenState::DONE));
    sys::store().oven_state.set(sys::OvenState::DONE);
  }
}

void ProgramTask::start(const ThermalProfile& p) {
  profile_  = &p;
  start_ms_ = millis();
  set_state(static_cast<uint32_t>(sys::OvenState::RUNNING));
  sys::store().oven_state.set(sys::OvenState::RUNNING);
}

void ProgramTask::abort() {
  sys::store().setpoint_top.set(0.0f);
  sys::store().setpoint_bottom.set(0.0f);
  set_state(static_cast<uint32_t>(sys::OvenState::IDLE));
  sys::store().oven_state.set(sys::OvenState::IDLE);
}

float ProgramTask::elapsed_s() const {
  return (millis() - start_ms_) / 1000.0f;
}

ProgramTask& program_task() {
  static ProgramTask instance;
  return instance;
}

}  // namespace control
