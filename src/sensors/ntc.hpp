#pragma once

namespace sensors {

void  ntc_init();
void  ntc_tick();
float ntc_celsius();    // door-motor body temperature

}  // namespace sensors
