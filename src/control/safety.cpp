#include "safety.hpp"
#include "heater_control.hpp"
#include "../system/store.hpp"

namespace control {

// Hard ceiling; trips kill heaters independent of any active profile.
static constexpr float kHardMaxC = 280.0f;

static Fault fault_ = Fault::NONE;

void safety_init() { fault_ = Fault::NONE; }

void safety_tick() {
  if (fault_ != Fault::NONE) return;   // latched until ack

  const sys::Slot<float>* tcs[3]   = {
    &sys::store().tc_top, &sys::store().tc_bottom, &sys::store().tc_target,
  };
  const sys::Slot<bool>* opens[3]  = {
    &sys::store().tc_top_open, &sys::store().tc_bottom_open, &sys::store().tc_target_open,
  };

  for (int i = 0; i < 3; ++i) {
    if (opens[i]->valid() && opens[i]->get())                  { fault_ = Fault::OPEN_THERMOCOUPLE; break; }
    if (tcs[i]->valid()  && tcs[i]->get() > kHardMaxC)         { fault_ = Fault::OVER_TEMP;         break; }
  }

  if (fault_ != Fault::NONE) {
    heater_task().kill();
  }
}

Fault safety_fault() { return fault_; }
bool  safety_ok()    { return fault_ == Fault::NONE; }
void  safety_ack()   {
  fault_ = Fault::NONE;
  heater_task().clear_kill();
}

}  // namespace control
