#pragma once
#include <stdint.h>

namespace sensors {

// Logical roles — bound to physical CS pins via a SensorMap loaded from flash.
enum class TcRole : uint8_t { TOP = 0, BOTTOM = 1, TARGET = 2 };

struct TcReading {
  float celsius;
  bool  open_tc;
  bool  fresh;      // true if the value was updated this tick
};

void thermocouples_init();
void thermocouples_tick();    // round-robin one chip per call (~250 ms period)

TcReading read(TcRole role);

}  // namespace sensors
