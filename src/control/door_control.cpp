#include "door_control.hpp"
#include "../system/store.hpp"
#include "../system/event_bus.hpp"
#include <DRV8251.hpp>

namespace control {

static driver::DRV8251 motor(PIN_MOTOR_IN1, PIN_MOTOR_IN2);
static DoorState current_ = DoorState::CLOSED;

// Hard limit for the door motor body temperature — refuse to drive above this.
static constexpr float kMotorMaxC = 80.0f;

static bool motor_hot() {
  const auto& ntc = sys::store().ntc_door_motor;
  return ntc.fresh_within(sys::freshness::kNtcBudgetMs) && ntc.get() > kMotorMaxC;
}

void DoorTask::on_init() {
  motor.begin();
  motor.coast();

  // On a fault: open the door so the load cools, unless the motor is
  // already too hot to drive — in that case stay put and let the alarm
  // do the talking.
  sys::bus().fault_tripped.subscribe([](const sys::FaultTripped&) {
    if (motor_hot()) return;
    if (current_ == DoorState::CLOSED || current_ == DoorState::CLOSING) {
      current_ = DoorState::OPENING;
    }
  });
}

void DoorTask::on_tick() {
  if (motor_hot() && current_ != DoorState::CLOSED) {
    motor.coast();
    current_ = DoorState::FAULT;
  }
  // TODO: full state machine with stall detection via IPROPI.
}

void door_request_open()  { if (current_ == DoorState::CLOSED) current_ = DoorState::OPENING; }
void door_request_close() { if (current_ == DoorState::OPEN)   current_ = DoorState::CLOSING; }
DoorState door_state()    { return current_; }

DoorTask& door_task() {
  static DoorTask instance;
  return instance;
}

}  // namespace control
