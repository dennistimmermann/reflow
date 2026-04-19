#pragma once

namespace control {

void fan_init();
void fan_tick();
void fan_set(float duty);   // 0..1

}  // namespace control
