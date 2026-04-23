#include <Arduino.h>
#include "system/dfu.hpp"
// #include "app.hpp"

// Ensure heaters are OFF before anything else. Called from startup so a hang
// in any later init path can never leave a FET energised.
// static void force_fets_off() {
//   pinMode(board::PIN_FET_0, OUTPUT); digitalWrite(board::PIN_FET_0, LOW);
//   pinMode(board::PIN_FET_1, OUTPUT); digitalWrite(board::PIN_FET_1, LOW);
//   pinMode(board::PIN_FET_2, OUTPUT); digitalWrite(board::PIN_FET_2, LOW);
// }

void setup() {
  pinMode(PIN_USER_LED, OUTPUT);
  pinMode(PIN_FET_0, OUTPUT);
  digitalWrite(PIN_FET_0, LOW);
  // force_fets_off();
}

void loop() {
  if (sys::dfu_requested()) sys::enter_dfu();
  // analogWrite(PIN_FET_0, 0);
  digitalWrite(PIN_FET_0, HIGH);
  digitalWrite(PIN_USER_LED, LOW);
  delay(100);
  digitalWrite(PIN_USER_LED, HIGH);
  delay(100);
}
