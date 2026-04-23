#pragma once

namespace sensors {

void  mcu_temp_update();        // call from scheduler; one ADC pair per tick
float mcu_temp_celsius();       // returns latest averaged die temperature

}  // namespace sensors
