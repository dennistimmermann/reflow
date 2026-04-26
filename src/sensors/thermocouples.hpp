#pragma once
#include <stdint.h>
#include "../system/task.hpp"

namespace sensors {

// Logical roles — bound to physical CS pins via a SensorMap loaded from flash.
enum class TcRole : uint8_t { TOP = 0, BOTTOM = 1, TARGET = 2 };

struct TcReading {
  float celsius;
  bool  open_tc;
  bool  fresh;      // true if the value was updated this tick
};

// Reads the three MAX6675s round-robin (one chip per tick, ~250 ms cadence
// per chip) and publishes filtered values + open-TC flags into the Store
// slots tc_{top,bottom,target} and tc_{top,bottom,target}_open.
class ThermocoupleTask : public sys::Task {
 public:
  ThermocoupleTask() : sys::Task("thermocouples", 250) {}
  void on_init() override;
  void on_tick() override;
};

// Backwards-compat shim — reads from Store. Keeps heater_control / safety /
// UI compiling until they're migrated to read Store directly. Removed in
// step 3.
TcReading read(TcRole role);

}  // namespace sensors
