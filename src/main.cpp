#include <Arduino.h>
#include <tusb.h>
#include "system/tud_serial.hpp"
#include "system/dfu.hpp"
#include <Buzzer.hpp>
#include "sensors/mcu_temp.hpp"

extern "C" void usb_bsp_init(void);

static driver::Buzzer buzzer(PIN_BUZZER);

static const driver::Note* const kMelodies[] = {
    driver::melodies::tick,
    driver::melodies::confirm,
    driver::melodies::back,
    driver::melodies::chirp_up,
    driver::melodies::startup,
    driver::melodies::done,
    driver::melodies::error,
};
static constexpr uint8_t kNumMelodies = sizeof(kMelodies) / sizeof(kMelodies[0]);
static uint8_t melody_idx = 0;

void setup() {
    usb_bsp_init();
    tud_init(BOARD_TUD_RHPORT);   // tusb_init() is a no-op without the legacy
                                  // CFG_TUSB_RHPORTx_MODE macros — call the
                                  // device-stack init directly.

    analogReadResolution(12);
    pinMode(PIN_USER_LED, OUTPUT);
    pinMode(PIN_FET_0, OUTPUT);
    digitalWrite(PIN_FET_0, LOW);
    pinMode(PIN_BOOT_BTN, INPUT);  // active-HIGH, external pull-down
    buzzer.begin();
}

void loop() {
    tud_task();        // service USB every iteration — CDC + DFU runtime
    buzzer.update();

    // Rising-edge detection with 30 ms debounce
    static bool     prev_btn  = false;
    static uint32_t debounce  = 0;
    bool btn = digitalRead(PIN_BOOT_BTN);
    if (btn != prev_btn && millis() - debounce >= 30) {
        prev_btn = btn;
        debounce = millis();
        if (btn) {
            buzzer.play(kMelodies[melody_idx]);
            melody_idx = (melody_idx + 1) % kNumMelodies;
        }
    }

    // Heartbeat LED + temperature sample — 100 ms tick
    static uint32_t tick_ms   = 0;
    static bool     led_state = false;
    if (millis() - tick_ms >= 100) {
        tick_ms   = millis();
        led_state = !led_state;
        digitalWrite(PIN_USER_LED, led_state);
        sensors::mcu_temp_update();
    }

    // MCU die temperature report — 1000 ms
    static uint32_t temp_ms = 0;
    if (millis() - temp_ms >= 1000) {
        temp_ms = millis();
        Serial.printf("MCU temp: %.1f C\r\n", sensors::mcu_temp_celsius());
    }
}
