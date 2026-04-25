#include <Arduino.h>
#include <IWatchdog.h>
#include <SPI.h>
#include <Buzzer.hpp>
#include <GC9A01.hpp>
#include <MAX6675.hpp>
#include "sensors/mcu_temp.hpp"
#include "system/scheduler.hpp"

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

// SPI1 + GC9A01 — debug bring-up of the 240x240 round LCD. Bypasses LVGL on
// purpose; rip this out once lv_port_disp is wired into app composition.
//
// MISO must be a valid SPI1_MISO pin even though the LCD is write-only: same
// trap as the MAX6675 bus above — STM32duino's spi_init() bails if any of
// MOSI/MISO/SCLK is NP, leaving the peripheral uninitialised and any
// transfer() call hanging forever. PA6 is unused on this board (per CLAUDE.md
// §2) and maps to SPI1_MISO, so it's a safe dummy.
static constexpr uint32_t PIN_LCD_MISO_DUMMY = PA6;
static SPIClass spi_lcd(PIN_LCD_MOSI, PIN_LCD_MISO_DUMMY, PIN_LCD_SCK);
static driver::GC9A01 lcd(spi_lcd, PIN_LCD_CS, PIN_LCD_DC,
                          PIN_LCD_RST, PIN_LCD_BL);

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
    Serial.printf("MCU temp: %.1f C\r\n", sensors::mcu_temp_celsius());
}

static void tc_report_1s() {
    const auto r0 = tc0.read();
    Serial.printf("TC0: %.2f C [open=%d]\r\n", r0.celsius, r0.open_tc);
}

static void lcd_cycle_1s() {
    static constexpr uint16_t kColors[] = {
        0xF800,  // red
        0x07E0,  // green
        0x001F,  // blue
        0xFFFF,  // white
    };
    static const char* const kNames[] = {"red", "green", "blue", "white"};
    static uint8_t idx = 0;
    lcd.fill_screen(kColors[idx]);
    Serial.printf("LCD: %s\r\n", kNames[idx]);
    idx = (idx + 1) & 0x3;
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

    spi_lcd.begin();
    lcd.begin();
    lcd.fill_screen(0x0000);

    sched.add({.interval = 100,  .fn = heartbeat_100ms});
    sched.add({.interval = 1000, .fn = mcu_temp_report_1s});
    sched.add({.interval = 1000, .fn = tc_report_1s});
    sched.add({.interval = 1000, .fn = lcd_cycle_1s});

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
