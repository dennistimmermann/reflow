#include "safety.hpp"
#include "heater_control.hpp"
#include "door_control.hpp"
#include "../system/store.hpp"
#include "../system/event_bus.hpp"

namespace control {

// Hard ceiling; trips kill heaters independent of any active profile.
static constexpr float kHardMaxC          = 280.0f;
static constexpr float kMotorOverheatC    = 80.0f;

void SafetyTask::on_init() {
  set_state(static_cast<uint32_t>(State::NORMAL));
  sys::store().fault_flags.set(0u);

  sys::bus().fault_acknowledged.subscribe([](const sys::FaultAcknowledged&) {
    safety_task().set_state_normal();
  });
}

void SafetyTask::on_tick() {
  if (state() == static_cast<uint32_t>(State::TRIPPED)) return;   // latched

  uint32_t flags = 0;

  const sys::Slot<float>* tcs[3]   = {
    &sys::store().tc_top, &sys::store().tc_bottom, &sys::store().tc_target,
  };
  const sys::Slot<bool>* opens[3]  = {
    &sys::store().tc_top_open, &sys::store().tc_bottom_open, &sys::store().tc_target_open,
  };

  // Freshness check: every TC slot must have been updated within the
  // budget. A stale slot is itself a fault — the producer hung or never
  // started.
  for (int i = 0; i < 3; ++i) {
    if (!tcs[i]->fresh_within(sys::freshness::kTcBudgetMs)) {
      flags |= sys::fault::kStaleSensor;
    } else if (tcs[i]->get() > kHardMaxC) {
      flags |= sys::fault::kOverTemp;
    }
    if (opens[i]->valid() && opens[i]->get()) {
      flags |= sys::fault::kOpenThermocouple;
    }
  }

  // Door-motor temperature is allowed to be stale (NTC is 1 Hz) but if
  // present, enforce the limit.
  const auto& ntc = sys::store().ntc_door_motor;
  if (ntc.fresh_within(sys::freshness::kNtcBudgetMs) &&
      ntc.get() > kMotorOverheatC) {
    flags |= sys::fault::kMotorOverheat;
  }

  if (flags != 0) {
    set_state(static_cast<uint32_t>(State::TRIPPED));
    sys::store().fault_flags.set(flags);
    sys::bus().fault_tripped.publish(sys::FaultTripped{flags});
  }
}

void SafetyTask::set_state_normal() {
  set_state(static_cast<uint32_t>(State::NORMAL));
  sys::store().fault_flags.set(0u);
}

bool safety_ok() {
  return safety_task().state() == static_cast<uint32_t>(SafetyTask::State::NORMAL);
}

SafetyTask& safety_task() {
  static SafetyTask instance;
  return instance;
}

}  // namespace control
