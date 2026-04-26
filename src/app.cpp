#include "app.hpp"
#include "system/scheduler.hpp"
#include "system/task.hpp"
#include "system/logger.hpp"
#include "system/storage.hpp"
#include "sensors/thermocouples.hpp"
#include "sensors/ntc.hpp"
#include "control/heater_control.hpp"
#include "control/fan_control.hpp"
#include "control/door_control.hpp"
#include "control/safety.hpp"
#include "control/program_controller.hpp"
#include "ui/ui.hpp"

namespace app {

static sys::Scheduler scheduler;

// Periodic tasks — periods in ms. See CLAUDE.md §8 for the table.
// Modules that own a Meyers-singleton Task (heater/fan/program) live
// inside their own TUs; we just take a reference here. Modules still
// using free-function ticks are wrapped in LegacyTask until they migrate.
static sensors::ThermocoupleTask t_thermocouples;
static sensors::NtcTask          t_ntc;
static sys::LegacyTask           t_door("door",  50, control::door_tick);
static sys::LegacyTask           t_ui  ("ui",    10, ui::tick);
static sys::LoggerDrainTask      t_logger;

void init() {
  sys::logger_init();
  sys::storage_init();

  t_thermocouples.on_init();
  t_ntc.on_init();
  control::heater_task() .on_init();
  control::fan_task()    .on_init();
  control::program_task().on_init();
  control::safety_task() .on_init();

  control::door_init();

  ui::init();

  scheduler.add(control::safety_task());   // highest priority — first
  scheduler.add(t_thermocouples);
  scheduler.add(t_ntc);
  scheduler.add(control::heater_task());
  scheduler.add(control::fan_task());
  scheduler.add(t_door);
  scheduler.add(control::program_task());
  scheduler.add(t_ui);
  scheduler.add(t_logger);
}

void tick() {
  scheduler.run();
}

}  // namespace app
