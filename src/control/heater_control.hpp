#pragma once

namespace control {

void heater_init();
void heater_tick();       // 100 ms: PID + slow-PWM update

void set_setpoints(float top_c, float bottom_c);
void all_off();           // called by safety — hard cutoff

}  // namespace control
