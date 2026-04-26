#include "door_control.hpp"
#include "../system/store.hpp"
#include "../system/event_bus.hpp"
#include <DRV8251.hpp>

namespace control {

static driver::DRV8251 motor(PIN_MOTOR_IN1, PIN_MOTOR_IN2);
static DoorState state = DoorState::CLOSED;

// Hard limit for the door motor body temperature — refuse to drive above this.
static constexpr float kMotorMaxC = 80.0f;

static bool motor_hot() {
  const auto& ntc = sys::store().ntc_door_motor;
  return ntc.fresh_within(sys::freshness::kNtcBudgetMs) && ntc.get() > kMotorMaxC;
}

void door_init() {
  motor.begin();
  motor.coast();

  // On a fault: open the door so the load cools, unless the motor is
  // already too hot to drive — in that case stay put and let the alarm
  // do the talking.
  sys::bus().fault_tripped.subscribe([](const sys::FaultTripped&) {
    if (motor_hot()) return;
    if (state == DoorState::CLOSED || state == DoorState::CLOSING) {
      state = DoorState::OPENING;
    }
  });
}

void door_tick() {
  if (motor_hot() && state != DoorState::CLOSED) {
    motor.coast();
    state = DoorState::FAULT;
  }
  // TODO: full state machine with stall detection via IPROPI.
}

void door_request_open()  { if (state == DoorState::CLOSED) state = DoorState::OPENING; }
void door_request_close() { if (state == DoorState::OPEN)   state = DoorState::CLOSING; }
DoorState door_state()    { return state; }

}  // namespace control
