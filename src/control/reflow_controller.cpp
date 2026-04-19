#include "reflow_controller.hpp"
#include "heater_control.hpp"
#include "fan_control.hpp"
#include "door_control.hpp"
#include "safety.hpp"
#include <Arduino.h>

namespace control {

static ReflowState state_ = ReflowState::IDLE;
static const ReflowProfile* profile_ = nullptr;
static uint32_t start_ms_ = 0;

void reflow_init() { state_ = ReflowState::IDLE; }

void reflow_tick() {
  if (!safety_ok()) { state_ = ReflowState::FAULT; return; }
  if (state_ == ReflowState::IDLE || state_ == ReflowState::DONE) return;
  if (!profile_) return;

  bool done = false;
  const float t = (millis() - start_ms_) / 1000.0f;
  const float target = profile_target_at(*profile_, t, &done);

  set_setpoints(target, target);

  // Cool phase: once target drops below setpoint and we've entered cool-down,
  // open the door and run the fan. Phase classification is a TODO — for now
  // infer from a dropping target.
  if (done) {
    set_setpoints(0, 0);
    fan_set(1.0f);
    door_request_open();
    state_ = ReflowState::DONE;
  }
}

void reflow_start(const ReflowProfile& p) {
  profile_ = &p;
  start_ms_ = millis();
  state_ = ReflowState::PREHEAT;
}

void reflow_abort() {
  set_setpoints(0, 0);
  state_ = ReflowState::IDLE;
}

ReflowState reflow_state() { return state_; }
float reflow_elapsed_s()   { return (millis() - start_ms_) / 1000.0f; }

}  // namespace control
