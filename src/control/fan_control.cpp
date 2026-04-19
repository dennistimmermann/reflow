#include "fan_control.hpp"

namespace control {

static float duty_ = 0.0f;

void fan_init() {}
void fan_tick() {
  // TODO: drive the fan FET (LoadMap-assigned) with slow-PWM or on/off.
  (void)duty_;
}
void fan_set(float d) { duty_ = d < 0 ? 0 : d > 1 ? 1 : d; }

}  // namespace control
