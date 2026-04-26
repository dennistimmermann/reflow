#pragma once
#include "../system/task.hpp"

namespace control {

enum class DoorState { CLOSED, OPENING, OPEN, CLOSING, FAULT };

class DoorTask : public sys::Task {
 public:
  DoorTask() : sys::Task("door", 50) {}
  void on_init() override;
  void on_tick() override;
};

DoorTask& door_task();

void door_request_open();
void door_request_close();
DoorState door_state();

}  // namespace control
