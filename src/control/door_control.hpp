#pragma once

namespace control {

enum class DoorState { CLOSED, OPENING, OPEN, CLOSING, FAULT };

void door_init();
void door_tick();

void door_request_open();
void door_request_close();
DoorState door_state();

}  // namespace control
