#pragma once
#include "../system/task.hpp"

namespace sensors {

// Reads the STM32 internal temperature sensor (factory-calibrated) once
// per second; publishes Celsius into the Store slot mcu_temp.
class McuTempTask : public sys::Task {
 public:
  McuTempTask() : sys::Task("mcu_temp", 1000) {}
  void on_init() override;
  void on_tick() override;
};

}  // namespace sensors
