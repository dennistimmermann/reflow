#include <Arduino.h>
#include "app.hpp"

// Ensure heaters are OFF before anything else. Called from startup so a hang
// in any later init path can never leave a FET energised.
static void force_fets_off() {
  pinMode(board::PIN_FET_0, OUTPUT); digitalWrite(board::PIN_FET_0, LOW);
  pinMode(board::PIN_FET_1, OUTPUT); digitalWrite(board::PIN_FET_1, LOW);
  pinMode(board::PIN_FET_2, OUTPUT); digitalWrite(board::PIN_FET_2, LOW);
}

void setup() {
  force_fets_off();
  app::init();
}

void loop() {
  app::tick();
}
