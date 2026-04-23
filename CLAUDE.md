# Reflow Oven Firmware — Plan & Architecture

This file is the single source of truth for the firmware design. Update it when architecture changes. Hardware details come from `../hw/reflow.kicad_sch` — always cross-check the pin map here against that file before acting.

---

## 1. Hardware summary

| Item | Part | Notes |
|---|---|---|
| MCU | STM32G0B1CET (LQFP-48, 512 KB flash, 144 KB RAM, 64 MHz) | Single-core Cortex-M0+, full-speed USB device, UCPD |
| Thermocouples | 3× MAX6675 (K-type, 12-bit, 0.25 °C LSB) | SPI daisy with per-chip CS |
| Motor driver | DRV8251A (H-bridge, ~4.1 A peak) | Drives door-open motor; current sense via IPROPI |
| Load switches | 3× AO3400A logic-level N-FETs | Gate drive from MCU → SSRs / fan |
| Rotary input | Encoder w/ push button | Primary UI |
| Buzzer | Piezo | PWM tone generation |
| Status LED | 1× WS2812B | RGB status feedback |
| Door motor temp | NTC on J5 (divider → PA0) | Safety input |
| Display | 240×240 round LCD (15-pin FPC, very likely GC9A01) | SPI + DC/RST/BLK; optional I²C touch on same FPC |
| Debug | SWD via Tag-Connect J1; USB-C on J18 (USB FS device) | |

Power rails: `+5V` (TPS563201 from VIN), `+3V3` (TPS563201 from +5V), `VUSB`.

## 2. Pin map (authoritative — mirror in `src/board.hpp`)

| Pin | Port | Net | Role | Peripheral hint |
|---|---|---|---|---|
| 1 | PC13 | ROT_B | Encoder B | EXTI / TIM input |
| 2 | PC14 | ROT_A | Encoder A | EXTI / TIM input |
| 3 | PC15 | — | unused | — |
| 8 | PF0 | SPEAKER | Buzzer | TIM14_CH1 (PWM) |
| 9 | PF1 | RGB | WS2812B data | TIM / SPI / bit-bang |
| 10 | PF2 | ~RST | NRST (pin strap) | — |
| 11 | PA0 | TH_1 | NTC ADC | ADC1_IN0 |
| 13 | PA2 | SCREEN_BLK | LCD backlight (via Q6) | TIM15_CH1 (PWM, optional) |
| 14 | PA3 | SCREEN_DC | LCD data/cmd | GPIO out |
| 15 | PA4 | SCREEN_CS | LCD CS | GPIO out |
| 16 | PA5 | SCREEN_CLK | LCD SCK | SPI1_SCK |
| 18 | PA7 | SCREEN_MOSI | LCD MOSI | SPI1_MOSI |
| 19 | PB0 | SCREEN_RST | LCD reset | GPIO out |
| 20 | PB1 | TOUCH_RST | Touch reset | GPIO out |
| 21 | PB2 | TOUCH_INT | Touch IRQ | EXTI |
| 22 | PB10 | SCL | I²C (touch / expansion) | I2C2_SCL |
| 23 | PB11 | SDA | I²C | I2C2_SDA |
| 25 | PB13 | LED | Discrete status LED | GPIO out |
| 33 | PA11 | USB_UP_D- | USB D− | USB_DM |
| 34 | PA12 | USB_UP_D+ | USB D+ | USB_DP |
| 35 | PA13 | SWDIO | SWD | — |
| 36 | PA14 | SWCLK | SWD | — |
| 37 | PA15 | CS_T_2 | MAX6675 #2 CS | GPIO out |
| 38 | PD0 | CS_T_1 | MAX6675 #1 CS | GPIO out |
| 39 | PD1 | SCK | MAX6675 shared SCK | SPI2_SCK |
| 40 | PD2 | CS_T_0 | MAX6675 #0 CS | GPIO out |
| 41 | PD3 | MISO | MAX6675 shared MISO | SPI2_MISO |
| 42 | PB3 | SERVO | Spare servo output (J3) | TIM2 |
| 43 | PB4 | BTN_ROT | Encoder push button | EXTI (pull-up) |
| 44 | PB5 | IN1 | DRV8251 IN1 | TIM3_CH2 (PWM capable) |
| 45 | PB6 | IN2 | DRV8251 IN2 | TIM1_CH1 / TIM16 |
| 46 | PB7 | FET_0 | Load 0 gate (J12 LOAD0) | GPIO out or slow PWM |
| 47 | PB8 | FET_1 | Load 1 gate (J13 LOAD1) | GPIO out or slow PWM |
| 48 | PB9 | FET_2 | Load 2 gate (J14 LOAD2) | GPIO out or slow PWM |

> **LOAD0/1/2 ↔ function mapping**: the PCB labels are generic. In firmware a `LoadMap` struct binds `top_heater`, `bottom_heater`, `fan` to `FET_0/1/2`. Set it to match the wiring in the enclosure.
> **TH0/TH1/TH2 ↔ role**: same idea for thermocouples — a `SensorMap` binds `top`, `bottom`, `target` to `CS_T_0/1/2`.

Unused pins worth knowing about: PC15, PA1, PA6, PA8, PA9/PA10 (alt), PC6, PC7, PB12, PB14, PB15. Keep free for v2 features.

## 3. Software stack

- **Platform**: PlatformIO, framework = `arduino` (STM32Duino). Gives us Arduino conveniences plus direct HAL access when needed (SPI DMA for the LCD, timer captures for the encoder).
- **Language**: C++17.
- **GUI**: LVGL 9.x pulled via `lib_deps`, `lv_conf.h` lives in `src/` so PIO picks it up via `LV_CONF_PATH`.
- **Upload/debug**: ST-Link over Tag-Connect. `upload_protocol = stlink`, `debug_tool = stlink`.
- **USB**: enabled as CDC for a serial console and log forwarding.
- **Scheduling**: cooperative super-loop with a small `Scheduler` that runs tasks at fixed periods. No RTOS to start; re-evaluate if LVGL + control loops start fighting for CPU.
- **Coding style**: prefer readability and simplicity over performance. Optimise only when there is a measured problem or a clear hard-real-time constraint.

## 4. Directory layout

```
sw/
├── CLAUDE.md              ← this file
├── platformio.ini
├── src/
│   ├── main.cpp           entry point
│   ├── app.{hpp,cpp}      application composition root
│   ├── board.hpp          pin map (single source of truth)
│   ├── lv_conf.h          LVGL config for 240×240 round
│   ├── system/            scheduler, event bus, storage, logger
│   ├── sensors/           thermocouples, ntc (uses lib/MAX6675)
│   ├── control/           pid, heater, fan, door, safety, profile
│   └── ui/                lv_port_*, screens/, widgets/
├── lib/                   inline-header libraries (see §5)
│   ├── MAX6675/
│   ├── DRV8251/
│   ├── WS2812B/
│   ├── Buzzer/
│   ├── RotaryEncoder/
│   └── GC9A01/
├── include/               (reserved, currently empty)
└── test/                  native unit tests for control/profile
```

## 5. Library layout preference

Each `lib/<Name>/` contains **one header file** with the class fully defined inline (methods defined inside the class body or as `inline` below). Implementation is co-located with the declaration — no separate `.cpp`, no `include/` / `src/` split inside the lib. Rationale: reflow-oven drivers are small enough that a single file is easier to read, PlatformIO's LDF picks them up without extra config, and the whole driver is visible at a glance. If a driver outgrows a single screenful of code, promote it to `.hpp`+`.cpp` — not a rule to enforce dogmatically.

## 6. Module plans

### 6.1 `lib/MAX6675`

- Single class `MAX6675` holding `SPIClass&`, CS pin, last reading, last error.
- 16-bit SPI transaction on shared SPI2; per-instance CS. Bit 2 = open-thermocouple flag, bits 15..3 = temperature × 4.
- Read cadence: one chip at a time, round-robin every 250 ms (datasheet conversion time 170–220 ms).
- Returns `Reading { float celsius; bool open_tc; bool fresh; }`.
- Pluggable `SPIClass*` so we can swap to software SPI if needed.

### 6.2 `lib/DRV8251`

- PH/EN or IN/IN mode selection (we're wired IN1/IN2, so phase/enable via dual PWM).
- API: `drive(float duty, Direction dir)`, `brake()`, `coast()`, `set_current_limit(uint16_t mA)` (via IPROPI ADC if wired).
- Safe defaults: coast on boot, ramp on direction change.

### 6.3 `lib/WS2812B`

- Single pixel on PF1. We don't need a DMA chain driver — a TIM-based PWM + DMA or a tightly timed bit-bang is enough.
- API: `set_rgb(uint8_t r, uint8_t g, uint8_t b)`, `breathe(Color, period_ms)`, `pulse(Color, ms)`.
- Status codes live in `ui/status_led.hpp`, not here (driver stays dumb).

### 6.4 `lib/Buzzer`

- PWM on PF0 (TIM14_CH1). API: `tone(hz, ms)`, `beep()`, `chirp_up()`, `mute()`.
- Non-blocking: each call schedules a stop time via the `Scheduler`.

### 6.5 `lib/RotaryEncoder`

- Two GPIOs on PC13/PC14, button on PB4. Use STM32 timer encoder mode if available on those pins; otherwise EXTI + quadrature state machine.
- Debounce: 5 ms for button, Gray-code decoding for the wheel.
- Emits events via a callback: `OnRotate(int8_t delta)`, `OnPress()`, `OnLongPress(ms)`.

### 6.6 `lib/GC9A01`

- Minimal display driver: init sequence, `set_window`, `blit(buf, len)`. DMA transfer when available.
- Chosen because 240×240 round + 15-pin FPC strongly implies GC9A01. If the module is ST7789-round, swap the init table — the rest is identical.
- Used by `src/ui/lv_port_disp.cpp` as a backend.

### 6.7 `src/sensors/thermocouples`

- Owns three `MAX6675` instances and a `SensorMap` for logical names (`top`, `bottom`, `target`).
- Kalman or simple IIR filter per channel (start with IIR α=0.3, revisit after first thermal test).
- Detects stuck readings, open-TC, unrealistic gradients; forwards to `Safety`.

### 6.8 `src/sensors/ntc`

- Steinhart-Hart for NTC on PA0 (divider with R7). Measures door-motor body temp.
- Oversampled ADC (16× accumulate → 14-bit effective), 1 Hz update.

### 6.9 `src/control/pid`

- Templated `Pid<float>` with anti-windup, output clamp, derivative-on-measurement.
- Unit-tested in `test/native/` on host (no hardware dependencies).

### 6.10 `src/control/heater_control`

- Two independent PIDs (top, bottom) with slow-PWM outputs (window = 1–2 s, suited to SSRs).
- Input: filtered temperatures from `thermocouples`. Setpoint: from `reflow_controller`.
- Feed-forward term from profile slope so ramps don't saturate the PID.
- Publishes: current duty, PID state, error, for the UI.

### 6.11 `src/control/fan_control`

- Simple on/off or slow-PWM on the FET assigned to fan. Automatic during cool phase; manual-override via UI.

### 6.12 `src/control/door_control`

- State machine: `CLOSED → OPENING → OPEN → CLOSING → CLOSED`. Drives DRV8251 with a ramp.
- Timeout per transition; stall detection via IPROPI current sense and travel time.
- Gated by NTC motor-temp: refuse to drive if motor is hot.

### 6.13 `src/control/safety`

- Watchdog-style supervisor, ticks at 10 Hz.
- Checks: open-TC count, max temperature (per zone), rate-of-rise vs. expected (runaway), sensor disagreement between top/bottom/target, power-stage over-current (if IPROPI feed is wired to an ADC channel).
- On trip: kills all FETs, drives door open, buzzer alarm, UI fault screen. Requires explicit reset.
- Independent MCU watchdog (IWDG) fed from the main loop; if we hang, heaters are already guaranteed OFF by Safety turning them off before reset anyway.

### 6.14 `src/control/reflow_profile`

- Struct representing a profile as a list of segments `{ target_C, duration_s, slope_C_per_s_max }`.
- Built-in profiles (SnPb, SAC305 leaded/lead-free standard). User-editable via UI; persisted to flash (see 6.17).
- Provides `target_at(elapsed_s) -> float` to the controller.

### 6.15 `src/control/reflow_controller`

- High-level state machine: `IDLE → PREHEAT → SOAK → REFLOW → COOL → DONE | FAULT`.
- Binds profile → heater setpoints, manages fan + door during cool, emits `ReflowState` events to the UI.

### 6.16 `src/system/scheduler`

- Tiny cooperative scheduler: `add(Task{period_ms, callback})`, `run_once()` in the loop.
- Replaces ad-hoc `millis()` checks. Jitter-tolerant; does **not** use for anything safety-critical — Safety runs on its own hard periodic hook.

### 6.17 `src/system/storage`

- Thin wrapper around STM32 flash EEPROM emulation (last two sectors reserved). Keys: profiles, load/sensor map, UI settings.
- CRC-tagged records, ping-pong writes, atomic swap.

### 6.18 `src/system/event_bus`

- Small typed publisher/subscriber so the UI and Safety see the same `Temperature`/`ReflowState`/`Fault` events without tight coupling.

### 6.19 `src/system/logger`

- Writes to USB CDC when enumerated, SWO otherwise. Ring buffer in RAM; background task drains.

### 6.20 `src/ui/*` (LVGL)

See §7.

## 7. UI architecture (LVGL 9, 240×240 round)

### 7.1 Ports

- `lv_port_disp`: double-buffered partial refresh, 1/10 of screen per buffer, SPI1 DMA flush (`GC9A01` driver).
- `lv_port_indev`: encoder indev. Rotary ticks → `LV_KEY_NEXT/PREV`, button → `LV_KEY_ENTER`, long-press → back. We deliberately don't add touch yet (even though the LCD FPC carries it), to keep the UX simple. The I²C lines and IRQ pin are broken out so touch can be added later without firmware rework.

### 7.2 Screen model

```
Home ───► Run (ReflowRunning)
  │
  ├──► Profiles ──► ProfileEdit
  │
  ├──► Manual (jog heaters/fan/door for test)
  │
  └──► Settings ──► LoadMap, SensorMap, PID tune, Theme
```

- All screens share a `lv_group_t*` that the encoder drives.
- Back navigation is long-press on the encoder button everywhere.

### 7.3 Round-screen design rules

- Primary content inside an inscribed square (≈170×170 centered). Corners (outside the visible circle) must hold no critical text or touch targets.
- Radial gauges for temperature, arc progress bars for profile time — they feel native on a circular display.
- Status LED (WS2812) mirrors screen state (idle/warm/cool/fault) so the user gets feedback even at an angle.

### 7.4 Designer tooling question — `viewer.lvgl.io`

`viewer.lvgl.io` is LVGL's web viewer for the new declarative XML UI format (LVGL 9). It's good for mocking layouts, sharing with others, and fast iteration without flashing.

- **Upside**: fast layout iteration, live preview, no toolchain, supports the LVGL 9 declarative format that's moving toward first-class support.
- **Downside**: the XML→C pipeline is still maturing; exporting to production C that drops into our firmware isn't as polished as SquareLine Studio's. Custom widgets and event handlers still need C.
- **Recommendation**: use it for design sketches and screenshots to discuss UX. For production UI code, either hand-write LVGL C (best for small projects like this) or use SquareLine Studio (exports clean C + works with LVGL 9). Don't commit to the XML pipeline for the whole UI yet — revisit when LVGL's declarative tooling is more stable.

## 8. Control loop timing

| Task | Period | Priority |
|---|---|---|
| MAX6675 read (one channel) | 250 ms | high |
| NTC read | 1 s | low |
| PID update (heaters) | 100 ms | high |
| Slow-PWM update | 100 ms | high |
| Safety supervisor | 100 ms | **highest** |
| Door state machine | 50 ms | high |
| Reflow state machine | 250 ms | med |
| Encoder poll | — (IRQ) | — |
| LVGL tick | 5 ms | med |
| LVGL task handler | 10–20 ms | med |
| WS2812 refresh | 50 ms | low |
| Logger drain | 20 ms | low |

The cooperative scheduler slots these by period; LVGL gets the biggest slice. Safety runs before any heater update — if Safety trips, the heater update is skipped.

## 9. Safety design

Defense-in-depth:

1. **Firmware guards** (Safety module): open-TC → kill heaters; temp > hard max (e.g. 280 °C) → kill heaters; rate-of-rise > N °C/s with no duty response → kill heaters.
2. **Independent watchdog**: IWDG configured for ~1 s; only the main loop feeds it.
3. **Default-off FETs**: AO3400 gates pulled to GND at boot; `board_init()` sets them low *before* any other peripheral init.
4. **Door-open on fault**: DRV8251 drives the door open whenever Safety trips (unless NTC says the motor is too hot; then alarm only).
5. **UI fault latch**: trips require explicit user acknowledgement — no auto-clear.

## 10. Persistence & configuration

- `settings.bin` (flash emulated EEPROM): load map, sensor map, selected profile index, PID gains, UI theme.
- `profiles[0..7]`: eight slots, each a `ReflowProfile`.
- First boot: defaults written atomically, then normal flow.

## 11. Build & run

```
pio run              # build
pio run -t upload    # flash via ST-Link
pio device monitor   # USB CDC log
pio test -e native   # host tests (PID, profile, state machine)
```

## 12. Open questions

- [ ] Exact LCD controller (GC9A01 vs ST7789-round) — confirm from module datasheet/FPC pinout before firmware bring-up.
- [ ] LOAD0/1/2 ↔ top/bottom/fan wiring — decided at assembly, stored in `LoadMap`.
- [ ] TH0/1/2 ↔ top/bottom/target — same.
- [ ] PB3 `SERVO` and J3: unused by default. Repurpose for door limit switch or leave as expansion?
- [ ] Is IPROPI (DRV8251 pin 1) routed to a free ADC pin, or is current sense wish-list only? Check schematic net `__unnamed_38` — affects whether current-based stall detection is possible.

## 13. Generated files

- `/tmp/reflow_sch.json` — schematic analyzer dump (regenerable, not committed).
