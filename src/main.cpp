#include <Arduino.h>
#include <IWatchdog.h>
#include <SPI.h>
#include <Buzzer.hpp>
#include <GC9A01.hpp>
#include <MAX6675.hpp>
#include <SpiDmaTx.hpp>
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

// SPI1 + GC9A01 — debug bring-up of the 240x240 round LCD via the HAL-backed
// SpiDmaTx bus. Bypasses LVGL on purpose; rip this out once lv_port_disp is
// wired into app composition. SpiDmaTx owns SPI1 and DMA1_Channel1.
static driver::SpiDmaTx spi_lcd(SPI1, PIN_LCD_MOSI, PIN_LCD_SCK,
                                DMA1_Channel1, DMA_REQUEST_SPI1_TX,
                                DMA1_Channel1_IRQn);
static driver::GC9A01 lcd(spi_lcd, PIN_LCD_CS, PIN_LCD_DC,
                          PIN_LCD_RST, PIN_LCD_BL);

// Two half-frame RGB565 buffers for ping-pong: while DMA pumps one to the
// panel, CPU renders into the other. Same total RAM as a single full-frame
// buffer (115,200 bytes) but lets render and DMA overlap, so per-frame time
// drops from (render + DMA) to max(render, DMA).
static constexpr uint16_t kHalfRows = 120;
static uint16_t half_top[kHalfRows * 240];   // rows 0..119
static uint16_t half_bot[kHalfRows * 240];   // rows 120..239

// Tiny 8x8 bitmap font — only the glyphs we need for "FPS:NN". Each row is a
// byte, MSB on the left. Hand-rolled, debug-only; promote to a real font lib
// if reused elsewhere.
namespace font8 {
struct Glyph { uint8_t rows[8]; };
static constexpr Glyph kSpace = {{0,0,0,0,0,0,0,0}};
static constexpr Glyph kF     = {{0xF8,0x80,0x80,0xF0,0x80,0x80,0x80,0x00}};
static constexpr Glyph kP     = {{0xF0,0x88,0x88,0xF0,0x80,0x80,0x80,0x00}};
static constexpr Glyph kS     = {{0x78,0x80,0x80,0x70,0x08,0x08,0xF0,0x00}};
static constexpr Glyph kColon = {{0x00,0x00,0x60,0x60,0x00,0x60,0x60,0x00}};
static constexpr Glyph kDigits[10] = {
    {{0x70,0x88,0x98,0xA8,0xC8,0x88,0x70,0x00}}, // 0
    {{0x20,0x60,0x20,0x20,0x20,0x20,0x70,0x00}}, // 1
    {{0x70,0x88,0x08,0x10,0x20,0x40,0xF8,0x00}}, // 2
    {{0x70,0x88,0x08,0x30,0x08,0x88,0x70,0x00}}, // 3
    {{0x10,0x30,0x50,0x90,0xF8,0x10,0x10,0x00}}, // 4
    {{0xF8,0x80,0xF0,0x08,0x08,0x88,0x70,0x00}}, // 5
    {{0x30,0x40,0x80,0xF0,0x88,0x88,0x70,0x00}}, // 6
    {{0xF8,0x08,0x10,0x20,0x40,0x80,0x80,0x00}}, // 7
    {{0x70,0x88,0x88,0x70,0x88,0x88,0x70,0x00}}, // 8
    {{0x70,0x88,0x88,0x78,0x08,0x10,0x60,0x00}}, // 9
};
static const Glyph& lookup(char c) {
    if (c >= '0' && c <= '9') return kDigits[c - '0'];
    switch (c) {
        case 'F': return kF;
        case 'P': return kP;
        case 'S': return kS;
        case ':': return kColon;
        default:  return kSpace;
    }
}
}  // namespace font8

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return uint16_t(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

// Smooth animated diagonal-band gradient: each channel cycles independently
// with x, y, and (x+y) so colours wash across the panel as `t` advances.
// Renders 120 rows starting at screen-y `y_start` into `buf`.
static void render_gradient_half(uint16_t* buf, uint8_t t, uint16_t y_start) {
    for (uint16_t row = 0; row < kHalfRows; ++row) {
        const uint16_t y = y_start + row;
        for (uint16_t x = 0; x < 240; ++x) {
            const uint8_t r = uint8_t(x + t);
            const uint8_t g = uint8_t(y + t);
            const uint8_t b = uint8_t(x + y + t);
            buf[row * 240 + x] = rgb565(r, g, b);
        }
    }
}

// Draws text at screen-coordinates (x, y) into a half buffer that represents
// 120 rows starting at screen-y `y_offset`. Pixels falling outside this half
// are skipped automatically — caller doesn't need to know which half holds
// the text.
static void draw_text_half(uint16_t* buf, uint16_t y_offset,
                           uint16_t x, uint16_t y, uint8_t scale,
                           const char* str, uint16_t color) {
    while (*str) {
        const auto& gl = font8::lookup(*str);
        for (uint8_t row = 0; row < 8; ++row) {
            const uint8_t bits = gl.rows[row];
            for (uint8_t col = 0; col < 8; ++col) {
                if (!(bits & (0x80 >> col))) continue;
                for (uint8_t sy = 0; sy < scale; ++sy) {
                    for (uint8_t sx = 0; sx < scale; ++sx) {
                        const uint16_t screen_y = y + row * scale + sy;
                        if (screen_y < y_offset || screen_y >= y_offset + kHalfRows) continue;
                        const uint16_t buf_y = screen_y - y_offset;
                        const uint16_t buf_x = x + col * scale + sx;
                        buf[buf_y * 240 + buf_x] = color;
                    }
                }
            }
        }
        x += 8 * scale;
        ++str;
    }
}

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

// Render one animated frame using the ping-pong half-buffer pipeline.
// Window + CS/DC are configured once on the first call and never reset:
// the GC9A01's RAMWR auto-wraps within the window, so each half just streams
// to its proper rows automatically.
//
// Steady-state per call (entering with prev frame's bottom DMA still in flight):
//   1. render top(t)          — concurrent with DMA bot(t-1)
//   2. wait_idle, kick DMA top(t)
//   3. render bot(t)          — concurrent with DMA top(t)
//   4. wait_idle, kick DMA bot(t)
//   5. return — DMA bot(t) keeps streaming into next frame's top render
static void render_frame() {
    static uint32_t fps_window_start_ms = 0;
    static uint32_t fps_frames           = 0;
    static uint32_t fps                  = 0;
    static uint8_t  t                    = 0;
    static bool     first                = true;

    char fps_str[12];
    snprintf(fps_str, sizeof(fps_str), "FPS:%lu", (unsigned long)fps);
    const uint16_t text_w = uint16_t(strlen(fps_str) * 8 * 2);
    const uint16_t text_x = (240 - text_w) / 2;
    const uint16_t text_y = 56;   // inside the top half — keeps text in one buf

    // Phase 1: render top of frame t into half_top.
    render_gradient_half(half_top, t, /*y_start=*/0);
    draw_text_half(half_top, /*y_offset=*/0,
                   text_x, text_y, /*scale=*/2, fps_str, 0xFFFF);

    if (first) {
        // One-time setup: prime the window and lower CS for the rest of life.
        lcd.set_window(0, 0, 239, 239);
        digitalWrite(PIN_LCD_DC, HIGH);
        digitalWrite(PIN_LCD_CS, LOW);
        first = false;
    } else {
        // Wait for previous frame's bottom DMA before kicking this top.
        spi_lcd.wait_idle();
    }
    spi_lcd.transmit_dma(reinterpret_cast<const uint8_t*>(half_top),
                         sizeof(half_top));

    // Phase 2: render bottom while DMA-top runs in the background.
    render_gradient_half(half_bot, t, /*y_start=*/kHalfRows);
    draw_text_half(half_bot, /*y_offset=*/kHalfRows,
                   text_x, text_y, /*scale=*/2, fps_str, 0xFFFF);

    spi_lcd.wait_idle();
    spi_lcd.transmit_dma(reinterpret_cast<const uint8_t*>(half_bot),
                         sizeof(half_bot));
    // Bottom DMA keeps running; next call's top render starts concurrently.

    ++fps_frames;
    ++t;
    const uint32_t now = millis();
    if (now - fps_window_start_ms >= 1000) {
        fps                 = fps_frames * 1000 / (now - fps_window_start_ms);
        fps_frames          = 0;
        fps_window_start_ms = now;
    }
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

    lcd.begin();   // configures spi_lcd (SPI1 + DMA1_Channel1) internally

    sched.add({.interval = 100,  .fn = heartbeat_100ms});
    sched.add({.interval = 1000, .fn = mcu_temp_report_1s});
    sched.add({.interval = 1000, .fn = tc_report_1s});

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
    render_frame();   // flat-out display test; gated by SPI/DMA throughput
    IWatchdog.reload();
}
