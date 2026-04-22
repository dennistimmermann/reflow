#include "safety.hpp"
#include "heater_control.hpp"
#include "../sensors/thermocouples.hpp"
#include "../sensors/ntc.hpp"
#include <initializer_list>

namespace control {

// Hard ceiling; trips kill heaters independent of any active profile.
static constexpr float kHardMaxC = 280.0f;

static Fault fault_ = Fault::NONE;

void safety_init() { fault_ = Fault::NONE; }

void safety_tick() {
  if (fault_ != Fault::NONE) return;   // latched until ack

  for (auto role : { sensors::TcRole::TOP, sensors::TcRole::BOTTOM, sensors::TcRole::TARGET }) {
    const auto r = sensors::read(role);
    if (r.open_tc)            { fault_ = Fault::OPEN_THERMOCOUPLE; break; }
    if (r.celsius > kHardMaxC){ fault_ = Fault::OVER_TEMP;         break; }
  }

  if (fault_ != Fault::NONE) {
    all_off();
  }
}

Fault safety_fault() { return fault_; }
bool  safety_ok()    { return fault_ == Fault::NONE; }
void  safety_ack()   { fault_ = Fault::NONE; }

}  // namespace control
