#include <Arduino.h>
#include <IWatchdog.h>
#include <SPI.h>
#include <lvgl.h>
#include <Buzzer.hpp>
#include <MAX6675.hpp>
#include "sensors/mcu_temp.hpp"
#include "system/scheduler.hpp"
#include "system/task.hpp"
#include "ui/lv_port_disp.hpp"

static driver::Buzzer buzzer(PIN_BUZZER);
static sys::Scheduler sched;

// SPI2 for the MAX6675 bus. MAX6675 is receive-only but STM32duino's spi_init
// silently bails if MOSI doesn't map to the target SPI peripheral, which leaves
// the SPI registers uninitialised and any transfer() call hangs forever. As a
// workaround we point MOSI at PB15 — an unused pin that *is* a valid SPI2_MOSI
// AF. The peripheral will toggle PB15 harmlessly while we clock data in on PD3.
static constexpr uint32_t PIN_TC_MOSI_DUMMY = PB15;
static SPIClass spi_tc(PIN_TC_MOSI_DUMMY, PIN_TC_MISO, PIN_TC_SCK);
static driver::MAX6675 tc0(spi_tc, PIN_TC_CS_0);

// LVGL screen with two live-updated labels. SPI1 + DMA1_Channel1 + GC9A01 are
// owned by ui::lv_port_disp — main.cpp doesn't touch the LCD bus directly.
static lv_obj_t* lbl_tc_  = nullptr;
static lv_obj_t* lbl_mcu_ = nullptr;

namespace m = driver::melodies;

struct NamedMelody {
    const char*         name;
    const driver::Note* notes;
};

// Audition list: original first in each row, then the four variants.
// Press the boot button to step through; the serial log prints which one.
static const NamedMelody kMelodies[] = {
    {"tick",        m::tick},      {"tick_a",      m::tick_a},      {"tick_b",      m::tick_b},      {"tick_c",      m::tick_c},      {"tick_d",      m::tick_d},
    {"confirm",     m::confirm},   {"confirm_a",   m::confirm_a},   {"confirm_b",   m::confirm_b},   {"confirm_c",   m::confirm_c},   {"confirm_d",   m::confirm_d},
    {"back",        m::back},      {"back_a",      m::back_a},      {"back_b",      m::back_b},      {"back_c",      m::back_c},      {"back_d",      m::back_d},
    {"chirp_up",    m::chirp_up},  {"chirp_up_a",  m::chirp_up_a},  {"chirp_up_b",  m::chirp_up_b},  {"chirp_up_c",  m::chirp_up_c},  {"chirp_up_d",  m::chirp_up_d},
    {"startup",     m::startup},   {"startup_a",   m::startup_a},   {"startup_b",   m::startup_b},   {"startup_c",   m::startup_c},   {"startup_d",   m::startup_d},
    {"done",        m::done},      {"done_a",      m::done_a},      {"done_b",      m::done_b},      {"done_c",      m::done_c},      {"done_d",      m::done_d},
    {"error",       m::error},     {"error_a",     m::error_a},     {"error_b",     m::error_b},     {"error_c",     m::error_c},     {"error_d",     m::error_d},
};
static constexpr uint8_t kNumMelodies = sizeof(kMelodies) / sizeof(kMelodies[0]);
static uint8_t melody_idx = 0;

static void heartbeat_100ms() {
    static bool led = false;
    led = !led;
    digitalWrite(PIN_USER_LED, led);
    sensors::mcu_temp_update();
}

static void mcu_temp_report_1s() {
    const float c = sensors::mcu_temp_celsius();
    Serial.printf("MCU temp: %.1f C\r\n", c);
    if (lbl_mcu_) lv_label_set_text_fmt(lbl_mcu_, "MCU: %.1f C", c);
}

static void tc_report_1s() {
    const auto r0 = tc0.read();
    Serial.printf("TC0: %.2f C [open=%d]\r\n", r0.celsius, r0.open_tc);
    if (lbl_tc_) lv_label_set_text_fmt(lbl_tc_, "TC0: %.1f C", r0.celsius);
}

static void lvgl_tick_10ms() {
    lv_tick_inc(10);
    lv_task_handler();
}

static void build_screen() {
    auto* screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);

    lbl_tc_ = lv_label_create(screen);
    lv_obj_set_style_text_color(lbl_tc_, lv_color_white(), 0);
    lv_label_set_text(lbl_tc_, "TC0: --.- C");
    lv_obj_align(lbl_tc_, LV_ALIGN_CENTER, 0, -20);

    lbl_mcu_ = lv_label_create(screen);
    lv_obj_set_style_text_color(lbl_mcu_, lv_color_white(), 0);
    lv_label_set_text(lbl_mcu_, "MCU: --.- C");
    lv_obj_align(lbl_mcu_, LV_ALIGN_CENTER, 0, 20);

    lv_screen_load(screen);
}

void setup() {
    analogReadResolution(12);
    pinMode(PIN_USER_LED, OUTPUT);
    pinMode(PIN_FET_0, OUTPUT);
    digitalWrite(PIN_FET_0, LOW);
    pinMode(PIN_BOOT_BTN, INPUT);  // active-HIGH, external pull-down
    buzzer.begin();

    spi_tc.begin();
    tc0.begin();

    lv_init();
    ui::lv_port_disp_init();   // configures SPI1 + DMA1_Channel1 + GC9A01
    build_screen();

    static sys::LegacyTask t_lvgl    ("lvgl",      10,   lvgl_tick_10ms);
    static sys::LegacyTask t_heartbeat("heartbeat", 100,  heartbeat_100ms);
    static sys::LegacyTask t_mcu      ("mcu_temp",  1000, mcu_temp_report_1s);
    static sys::LegacyTask t_tc       ("tc_report", 1000, tc_report_1s);
    sched.add(t_lvgl);
    sched.add(t_heartbeat);
    sched.add(t_mcu);
    sched.add(t_tc);

    // Defense-in-depth §9: start AFTER FETs are driven low, so a mid-init
    // reset still leaves heaters off on the way down and back up.
    bool was_wdt_reset = IWatchdog.isReset(true);
    IWatchdog.begin(1'000'000);  // 1 s
    if (was_wdt_reset) {
        Serial.println("WARN: previous reset was watchdog-induced");
    }
}

void loop() {
    buzzer.update();

    // Rising-edge detection with 30 ms debounce
    static bool     prev_btn  = false;
    static uint32_t debounce  = 0;
    bool btn = digitalRead(PIN_BOOT_BTN);
    if (btn != prev_btn && millis() - debounce >= 30) {
        prev_btn = btn;
        debounce = millis();
        if (btn) {
            const NamedMelody& nm = kMelodies[melody_idx];
            Serial.printf("[%u/%u] %s\r\n", melody_idx + 1, kNumMelodies, nm.name);
            buzzer.play(nm.notes);
            melody_idx = (melody_idx + 1) % kNumMelodies;
        }
    }

    sched.run();
    IWatchdog.reload();
}
