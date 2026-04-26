#include "buzzer_task.hpp"
#include "../system/event_bus.hpp"
#include <Buzzer.hpp>

namespace tasks {

static driver::Buzzer buzzer(PIN_BUZZER);

// One-shot single-note holder for synthesised BuzzerPlay events. The
// Buzzer driver consumes a Note* sequence terminated by {0,0}; we
// keep two slots so the active sequence stays valid while update()
// walks it, and the next event can stomp the inactive one.
static driver::Note seq_a[2];
static driver::Note seq_b[2];
static bool active_a = true;

void BuzzerTask::on_init() {
  buzzer.begin();
  set_state(static_cast<uint32_t>(State::IDLE));

  sys::bus().buzzer_play.subscribe([](const sys::BuzzerPlay& e) {
    auto& seq = active_a ? seq_b : seq_a;
    seq[0].hz = e.hz;
    seq[0].ms = e.ms;
    seq[1].hz = 0;
    seq[1].ms = 0;
    buzzer.play(seq);
    active_a = !active_a;
  });
}

void BuzzerTask::on_tick() {
  buzzer.update();
  // The driver doesn't expose its internal state, so we infer from
  // whether play has been issued. Refining this is a step-5 nicety
  // worth picking up if the BETWEEN_NOTES distinction ever matters.
}

BuzzerTask& buzzer_task() {
  static BuzzerTask instance;
  return instance;
}

}  // namespace tasks
