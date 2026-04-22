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
  pinMode(PB13, OUTPUT);
  pinMode(PB7, OUTPUT);
  // force_fets_off();
}

void loop() {
  if (sys::dfu_requested()) sys::enter_dfu();
  digitalWrite(PB13, LOW);  // user LED off by default
  delay(100);
  digitalWrite(PB7, LOW);  // user LED off by default
  delay(100);
  digitalWrite(PB7, HIGH);  // user LED off by default 
  delay(100);
  digitalWrite(PB13, HIGH); // user LED on by default
  delay(100);
  digitalWrite(PB7, LOW);  // user LED off by default
  delay(100);
  digitalWrite(PB7, HIGH);  // user LED off by default 
  delay(100);
}
