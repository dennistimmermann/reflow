#pragma once
#include <stdint.h>

namespace control {

enum class Fault : uint8_t {
  NONE = 0,
  OPEN_THERMOCOUPLE,
  OVER_TEMP,
  RUNAWAY,
  SENSOR_DISAGREE,
  MOTOR_OVERHEAT,
};

void  safety_init();
void  safety_tick();        // 100 ms — runs before heater_tick in the scheduler
Fault safety_fault();
bool  safety_ok();
void  safety_ack();         // user acknowledges fault via UI

}  // namespace control
