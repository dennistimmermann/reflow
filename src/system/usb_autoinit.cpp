// USB bring-up and per-loop tick, hidden from the application layer.
//
// - usb_autoinit() runs via `__attribute__((constructor(102)))`, right after
//   STM32duino's own premain (priority 101 — runs HAL_Init +
//   SystemClock_Config). At that point HSI48 is on and the HAL is ready,
//   but main() / setup() have not yet been entered and static C++
//   constructors have not yet run. Perfect slot to stand the USB stack up.
//
// - serialEventRun() is a WEAK symbol in the Arduino core
//   (framework-arduinoststm32/cores/arduino/WSerial.cpp) that the core's
//   main.cpp calls once per loop() iteration. Overriding it here keeps
//   TinyUSB ticking without the application having to put tud_task() at
//   the top of its loop().

#include <Arduino.h>
#include <tusb.h>

extern "C" void usb_bsp_init(void);

extern "C" __attribute__((constructor(102))) void usb_autoinit(void) {
    usb_bsp_init();
    tud_init(BOARD_TUD_RHPORT);
}

// Core-declared C++ linkage (WSerial.h). Not extern "C".
void serialEventRun(void) {
    tud_task();
}
