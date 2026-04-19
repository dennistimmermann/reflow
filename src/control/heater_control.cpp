#include "heater_control.hpp"
#include "pid.hpp"
#include "../sensors/thermocouples.hpp"

namespace control {

// TODO: bind these two PIDs to the actual FET pins via a LoadMap.
static Pid pid_top   ({8.0f, 0.15f, 2.0f}, 0.0f, 1.0f);
static Pid pid_bottom({8.0f, 0.15f, 2.0f}, 0.0f, 1.0f);

static float sp_top = 0.0f, sp_bottom = 0.0f;
static bool killed = false;

void heater_init() {
  pid_top.reset(25.0f);
  pid_bottom.reset(25.0f);
}

void heater_tick() {
  if (killed) return;
  const auto t = sensors::read(sensors::TcRole::TOP);
  const auto b = sensors::read(sensors::TcRole::BOTTOM);
  // dt=0.1s — tied to the scheduler period.
  (void)pid_top.step(sp_top,    t.celsius, 0.1f);
  (void)pid_bottom.step(sp_bottom, b.celsius, 0.1f);
  // TODO: feed duty → slow-PWM windowed driver on the assigned FETs.
}

void set_setpoints(float top_c, float bottom_c) {
  sp_top = top_c;
  sp_bottom = bottom_c;
}

void all_off() {
  killed = true;
  // TODO: write FET pins LOW directly, bypassing the PWM window.
}

}  // namespace control
