#include "app.hpp"
#include "system/scheduler.hpp"
#include "system/logger.hpp"
#include "system/storage.hpp"
#include "sensors/thermocouples.hpp"
#include "sensors/ntc.hpp"
#include "control/heater_control.hpp"
#include "control/fan_control.hpp"
#include "control/door_control.hpp"
#include "control/safety.hpp"
#include "control/reflow_controller.hpp"
#include "ui/ui.hpp"

namespace app {

static sys::Scheduler scheduler;

void init() {
  sys::logger_init();
  sys::storage_init();

  sensors::thermocouples_init();
  sensors::ntc_init();

  control::heater_init();
  control::fan_init();
  control::door_init();
  control::safety_init();
  control::reflow_init();

  ui::init();

  // Periodic tasks — periods in ms. See CLAUDE.md §8 for the table.
  scheduler.add({100, control::safety_tick});       // highest priority — first
  scheduler.add({250, sensors::thermocouples_tick});
  scheduler.add({1000, sensors::ntc_tick});
  scheduler.add({100, control::heater_tick});
  scheduler.add({100, control::fan_tick});
  scheduler.add({50,  control::door_tick});
  scheduler.add({250, control::reflow_tick});
  scheduler.add({10,  ui::tick});
  scheduler.add({20,  sys::logger_drain});
}

void tick() {
  scheduler.run_once();
}

}  // namespace app
