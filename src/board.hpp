#pragma once
// Pin map for the reflow oven PCB (STM32G0B1CBT, LQFP-48).
// This file is the single source of truth — mirrors the table in ../CLAUDE.md §2.
// Any mismatch with ../hw/reflow.kicad_sch is a bug here; fix this file.

#include <Arduino.h>

namespace board {

// ---- User input -----------------------------------------------------------
constexpr uint32_t PIN_ROT_A       = PC14;  // Encoder A
constexpr uint32_t PIN_ROT_B       = PC13;  // Encoder B
constexpr uint32_t PIN_ROT_BTN     = PB4;   // Encoder push, active-low

// ---- Feedback -------------------------------------------------------------
constexpr uint32_t PIN_BUZZER      = PF0;   // TIM14_CH1
constexpr uint32_t PIN_WS2812      = PF1;   // Single-pixel status LED
constexpr uint32_t PIN_USER_LED    = PB13;  // Discrete status LED on the PCB

// ---- LCD (SPI1) -----------------------------------------------------------
constexpr uint32_t PIN_LCD_SCK     = PA5;   // SPI1_SCK
constexpr uint32_t PIN_LCD_MOSI    = PA7;   // SPI1_MOSI
constexpr uint32_t PIN_LCD_CS      = PA4;
constexpr uint32_t PIN_LCD_DC      = PA3;
constexpr uint32_t PIN_LCD_RST     = PB0;
constexpr uint32_t PIN_LCD_BL      = PA2;   // PWM via MOSFET Q6

// ---- Touch (optional, reserved) -------------------------------------------
constexpr uint32_t PIN_TOUCH_RST   = PB1;
constexpr uint32_t PIN_TOUCH_INT   = PB2;
constexpr uint32_t PIN_I2C_SCL     = PB10;  // I2C2
constexpr uint32_t PIN_I2C_SDA     = PB11;

// ---- Thermocouples (MAX6675, shared SPI2) ---------------------------------
constexpr uint32_t PIN_TC_SCK      = PD1;
constexpr uint32_t PIN_TC_MISO     = PD3;
constexpr uint32_t PIN_TC_CS_0     = PD2;   // MAX6675 U1
constexpr uint32_t PIN_TC_CS_1     = PD0;   // MAX6675 U4
constexpr uint32_t PIN_TC_CS_2     = PA15;  // MAX6675 U5

// ---- NTC (door motor body temp) -------------------------------------------
constexpr uint32_t PIN_NTC         = PA0;   // ADC1_IN0

// ---- Door motor (DRV8251A) ------------------------------------------------
constexpr uint32_t PIN_MOTOR_IN1   = PB5;
constexpr uint32_t PIN_MOTOR_IN2   = PB6;
// IPROPI wiring: net __unnamed_38 — confirm ADC routing before enabling current sense.

// ---- Load FETs (SSRs / fan) ----------------------------------------------
// Labels on the PCB are generic LOAD0/1/2. Assignment to top/bottom/fan
// happens in src/control/load_map at runtime.
constexpr uint32_t PIN_FET_0       = PB7;   // J12 LOAD0
constexpr uint32_t PIN_FET_1       = PB8;   // J13 LOAD1
constexpr uint32_t PIN_FET_2       = PB9;   // J14 LOAD2

// ---- Spare / expansion ----------------------------------------------------
constexpr uint32_t PIN_SERVO       = PB3;   // J3 — currently unused

}  // namespace board
