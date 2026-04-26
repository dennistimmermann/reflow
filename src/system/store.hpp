#pragma once
#include <Arduino.h>
#include <stdint.h>

namespace sys {

// Typed slot for a single piece of shared state. Producers call set() on
// every update; consumers can read the value, check valid, and check
// age_ms() against a freshness budget. A slot starts invalid; a consumer
// that ignores valid will see a default-constructed T, not garbage.
template <typename T>
class Slot {
 public:
  void set(const T& v) {
    value_         = v;
    timestamp_ms_  = millis();
    valid_         = true;
  }

  void invalidate() { valid_ = false; }

  T        get()           const { return value_; }
  bool     valid()         const { return valid_; }
  uint32_t timestamp_ms()  const { return timestamp_ms_; }

  // Wrap-safe age. Returns UINT32_MAX if the slot has never been set.
  uint32_t age_ms() const {
    if (!valid_) return UINT32_MAX;
    return millis() - timestamp_ms_;
  }

  // Convenience: valid AND younger than budget_ms.
  bool fresh_within(uint32_t budget_ms) const {
    return valid_ && age_ms() <= budget_ms;
  }

 private:
  T        value_         = T{};
  uint32_t timestamp_ms_  = 0;
  bool     valid_         = false;
};

// Strong door-position type — the Door FSM publishes here, the UI and
// safety supervisor read.
enum class DoorPos : uint8_t { CLOSED, OPENING, OPEN, CLOSING, FAULT };

// High-level oven mode. Applies to reflow / anneal / bake equally — the
// shape of the active profile is what differentiates them, not this enum.
enum class OvenState : uint8_t { IDLE, RUNNING, COOLING, DONE, FAULT };

// Per-slot freshness budgets. Living next to the slot list so producer
// period and consumer deadline are visible side-by-side; a typo here
// is reviewable, not a runtime computation.
namespace freshness {
constexpr uint32_t kTcBudgetMs        = 750;   // produced every ~250 ms × 3-way RR
constexpr uint32_t kNtcBudgetMs       = 3000;  // produced every 1000 ms
constexpr uint32_t kMcuTempBudgetMs   = 3000;
constexpr uint32_t kSetpointBudgetMs  = 1000;
constexpr uint32_t kDutyBudgetMs      = 500;
}  // namespace freshness

// Single global store. Lives in app.cpp; modules grab a reference via
// store(). Slots are public on purpose — this is a data-only container,
// not an encapsulating class. Wrapping every slot in a getter would be
// noise.
struct Store {
  Slot<float> tc_top;
  Slot<float> tc_bottom;
  Slot<float> tc_target;
  Slot<bool>  tc_top_open;
  Slot<bool>  tc_bottom_open;
  Slot<bool>  tc_target_open;

  Slot<float> ntc_door_motor;
  Slot<float> mcu_temp;

  Slot<float> setpoint_top;
  Slot<float> setpoint_bottom;

  Slot<float> duty_top;
  Slot<float> duty_bottom;
  Slot<float> duty_fan;

  Slot<DoorPos>   door_position;
  Slot<OvenState> oven_state;

  Slot<uint32_t> fault_flags;
  Slot<int32_t>  encoder_position;
};

Store& store();

}  // namespace sys
