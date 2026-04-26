#include "app.hpp"
#include "system/scheduler.hpp"
#include "system/task.hpp"
#include "system/logger.hpp"
#include "system/storage.hpp"
#include "system/persistence_task.hpp"
#include "sensors/thermocouples.hpp"
#include "sensors/ntc.hpp"
#include "sensors/mcu_temp.hpp"
#include "control/heater_control.hpp"
#include "control/fan_control.hpp"
#include "control/door_control.hpp"
#include "control/safety.hpp"
#include "control/program_controller.hpp"
#include "tasks/status_led.hpp"
#include "tasks/buzzer_task.hpp"
#include "tasks/encoder_task.hpp"
#include "ui/ui.hpp"

namespace app {

static sys::Scheduler scheduler;

// Periodic tasks — periods in ms. See CLAUDE.md §8 for the table.
// Modules that own a Meyers-singleton Task (heater/fan/program/...) live
// inside their own TUs; we just take a reference here. Tasks that don't
// have cross-module callers live as namespace-scope statics.
static sensors::ThermocoupleTask t_thermocouples;
static sensors::NtcTask          t_ntc;
static sensors::McuTempTask      t_mcu_temp;
static sys::LoggerDrainTask      t_logger;

void init() {
  sys::logger_init();
  sys::storage_init();

  t_thermocouples.on_init();
  t_ntc.on_init();
  t_mcu_temp.on_init();
  control::heater_task() .on_init();
  control::fan_task()    .on_init();
  control::program_task().on_init();
  control::safety_task() .on_init();
  control::door_task()   .on_init();
  tasks::status_led_task().on_init();
  tasks::buzzer_task()    .on_init();
  tasks::encoder_task()   .on_init();
  sys::persistence_task() .on_init();
  ui::ui_task()           .on_init();

  scheduler.add(control::safety_task());   // highest priority — first
  scheduler.add(t_thermocouples);
  scheduler.add(t_ntc);
  scheduler.add(t_mcu_temp);
  scheduler.add(control::heater_task());
  scheduler.add(control::fan_task());
  scheduler.add(control::door_task());
  scheduler.add(control::program_task());
  scheduler.add(tasks::encoder_task());
  scheduler.add(tasks::buzzer_task());
  scheduler.add(tasks::status_led_task());
  scheduler.add(ui::ui_task());
  scheduler.add(sys::persistence_task());
  scheduler.add(t_logger);
}

void tick() {
  scheduler.run();
}

}  // namespace app
