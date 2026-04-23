#include <Arduino.h>
#include "system/dfu.hpp"
#include <Buzzer.hpp>

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
    pinMode(PIN_USER_LED, OUTPUT);
    pinMode(PIN_FET_0, OUTPUT);
    digitalWrite(PIN_FET_0, LOW);
    pinMode(PIN_BOOT_BTN, INPUT);  // active-HIGH, external pull-down
    buzzer.begin();
}

void loop() {
    if (sys::dfu_requested()) sys::enter_dfu();

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

    // Heartbeat LED — 100 ms toggle
    static uint32_t led_ms    = 0;
    static bool     led_state = false;
    if (millis() - led_ms >= 100) {
        led_ms    = millis();
        led_state = !led_state;
        digitalWrite(PIN_USER_LED, led_state);
    }
}
