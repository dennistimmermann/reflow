#include "door_control.hpp"
#include "../sensors/ntc.hpp"
#include <DRV8251.hpp>

namespace control {

static driver::DRV8251 motor(PIN_MOTOR_IN1, PIN_MOTOR_IN2);
static DoorState state = DoorState::CLOSED;

// Hard limit for the door motor body temperature — refuse to drive above this.
static constexpr float kMotorMaxC = 80.0f;

void door_init() { motor.begin(); motor.coast(); }

void door_tick() {
  if (sensors::ntc_celsius() > kMotorMaxC && state != DoorState::CLOSED) {
    motor.coast();
    state = DoorState::FAULT;
  }
  // TODO: full state machine with stall detection via IPROPI.
}

void door_request_open()  { if (state == DoorState::CLOSED) state = DoorState::OPENING; }
void door_request_close() { if (state == DoorState::OPEN)   state = DoorState::CLOSING; }
DoorState door_state()    { return state; }

}  // namespace control
