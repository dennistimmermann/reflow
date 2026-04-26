#pragma once
#include "../system/task.hpp"

namespace sensors {

// Reads the door-motor body NTC on PA0 (16× oversampled, ~14-bit
// effective) once per second and publishes Celsius into the Store
// slot ntc_door_motor.
class NtcTask : public sys::Task {
 public:
  NtcTask() : sys::Task("ntc", 1000) {}
  void on_init() override;
  void on_tick() override;
};

}  // namespace sensors
