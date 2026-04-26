#pragma once
#include <stddef.h>
#include <stdint.h>

namespace sys {

// Event payloads. Add new types as the system grows; keep them small
// and trivially copyable. Empty types (FaultAcknowledged, …) carry only
// the dispatch signal.
struct EncoderRotate          { int8_t   delta; };
struct EncoderPress           {};
struct EncoderLongPress       { uint16_t ms; };
struct ProgramStartRequested  { uint8_t  profile_idx; };
struct ProgramStopRequested   {};
struct ProgramSegmentAdvanced { uint8_t  segment; };
struct FaultTripped           { uint32_t flags; };
struct FaultAcknowledged      {};
struct BuzzerPlay             { uint16_t hz; uint16_t ms; };

// Per-event-type channel: fixed array of handler pointers, synchronous
// dispatch. Subscribers register at init; publishers call publish() and
// every handler runs inline (we're single-threaded). No queueing — if a
// handler is slow it blocks the publisher.
template <typename E, size_t kMaxSubs = 4>
class EventChannel {
 public:
  using Handler = void (*)(const E&);

  bool subscribe(Handler h) {
    if (count_ >= kMaxSubs) return false;
    subs_[count_++] = h;
    return true;
  }

  void publish(const E& e) const {
    for (size_t i = 0; i < count_; ++i) subs_[i](e);
  }

 private:
  Handler subs_[kMaxSubs] = {};
  size_t  count_ = 0;
};

// Single bus instance — one channel per event type. Modules hold no
// references; they just call bus().<channel>.{subscribe,publish}().
struct EventBus {
  EventChannel<EncoderRotate>          encoder_rotate;
  EventChannel<EncoderPress>           encoder_press;
  EventChannel<EncoderLongPress>       encoder_long_press;
  EventChannel<ProgramStartRequested>  program_start;
  EventChannel<ProgramStopRequested>   program_stop;
  EventChannel<ProgramSegmentAdvanced> program_segment;
  EventChannel<FaultTripped>           fault_tripped;
  EventChannel<FaultAcknowledged>      fault_acknowledged;
  EventChannel<BuzzerPlay>             buzzer_play;
};

EventBus& bus();

// Fault flag bits — packed into FaultTripped.flags. Defined here so
// publishers and subscribers (Safety, Heater, Door, UI) share one set.
namespace fault {
constexpr uint32_t kOpenThermocouple = 1u << 0;
constexpr uint32_t kOverTemp         = 1u << 1;
constexpr uint32_t kRunaway          = 1u << 2;
constexpr uint32_t kSensorDisagree   = 1u << 3;
constexpr uint32_t kStaleSensor      = 1u << 4;
constexpr uint32_t kMotorOverheat    = 1u << 5;
}  // namespace fault

}  // namespace sys
