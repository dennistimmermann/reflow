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
#include "control/reflow_controller.hpp"
#include "ui/ui.hpp"

namespace app {

static sys::Scheduler scheduler;

// Periodic tasks — periods in ms. See CLAUDE.md §8 for the table.
// LegacyTask is a temporary adapter while modules still expose
// free-function ticks; they will become Task subclasses one by one.
static sys::LegacyTask     t_safety       ("safety",        100, control::safety_tick);
static sys::LegacyTask     t_thermocouples("thermocouples", 250, sensors::thermocouples_tick);
static sys::LegacyTask     t_ntc          ("ntc",          1000, sensors::ntc_tick);
static sys::LegacyTask     t_heater       ("heater",        100, control::heater_tick);
static sys::LegacyTask     t_fan          ("fan",           100, control::fan_tick);
static sys::LegacyTask     t_door         ("door",           50, control::door_tick);
static sys::LegacyTask     t_reflow       ("reflow",        250, control::reflow_tick);
static sys::LegacyTask     t_ui           ("ui",             10, ui::tick);
static sys::LoggerDrainTask t_logger;

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

  scheduler.add(t_safety);          // highest priority — first
  scheduler.add(t_thermocouples);
  scheduler.add(t_ntc);
  scheduler.add(t_heater);
  scheduler.add(t_fan);
  scheduler.add(t_door);
  scheduler.add(t_reflow);
  scheduler.add(t_ui);
  scheduler.add(t_logger);
}

void tick() {
  scheduler.run();
}

}  // namespace app
