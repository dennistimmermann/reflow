/*
 * Variant header for the Reflow Oven Controller board.
 * MCU: STM32G0B1CET6, LQFP-48, 512 KB flash, 144 KB RAM, 64 MHz HSI.
 *
 * Pin numbering is identical to the generic G0B1C variant so that
 * PeripheralPins.c (copied verbatim) remains correct.
 */
#pragma once

/*----------------------------------------------------------------------------
 *        STM32 pin numbers  (chip-determined, do not reorder)
 *----------------------------------------------------------------------------*/
#define PA0                     PIN_A0
#define PA1                     PIN_A1
#define PA2                     PIN_A2
#define PA3                     PIN_A3
#define PA4                     PIN_A4
#define PA5                     PIN_A5
#define PA6                     PIN_A6
#define PA7                     PIN_A7
#define PA8                     8
#define PA9                     9
#define PA10                    10
#define PA11                    11
#define PA12                    12
#define PA13                    13
#define PA14                    14
#define PA15                    15
#define PB0                     PIN_A8
#define PB1                     PIN_A9
#define PB2                     PIN_A10
#define PB3                     19
#define PB4                     20
#define PB5                     21
#define PB6                     22
#define PB7                     23
#define PB8                     24
#define PB9                     25
#define PB10                    PIN_A11
#define PB11                    PIN_A12
#define PB12                    PIN_A13
#define PB13                    29
#define PB14                    30
#define PB15                    31
#define PC6                     32
#define PC7                     33
#define PC13                    34
#define PC14                    35
#define PC15                    36
#define PD0                     37
#define PD1                     38
#define PD2                     39
#define PD3                     40
#define PF0                     41
#define PF1                     42
#define PF2                     43
#define PA9_R                   44
#define PA10_R                  45

// Alternate pin numbers
#define PA1_ALT1                (PA1    | ALT1)
#define PA2_ALT1                (PA2    | ALT1)
#define PA3_ALT1                (PA3    | ALT1)
#define PA4_ALT1                (PA4    | ALT1)
#define PA6_ALT1                (PA6    | ALT1)
#define PA6_ALT2                (PA6    | ALT2)
#define PA7_ALT1                (PA7    | ALT1)
#define PA7_ALT2                (PA7    | ALT2)
#define PA7_ALT3                (PA7    | ALT3)
#define PA9_ALT1                (PA9    | ALT1)
#define PA9_R_ALT1              (PA9_R  | ALT1)
#define PA10_ALT1               (PA10   | ALT1)
#define PA10_R_ALT1             (PA10_R | ALT1)
#define PA14_ALT1               (PA14   | ALT1)
#define PA15_ALT1               (PA15   | ALT1)
#define PB0_ALT1                (PB0    | ALT1)
#define PB1_ALT1                (PB1    | ALT1)
#define PB1_ALT2                (PB1    | ALT2)
#define PB3_ALT1                (PB3    | ALT1)
#define PB4_ALT1                (PB4    | ALT1)
#define PB5_ALT1                (PB5    | ALT1)
#define PB6_ALT1                (PB6    | ALT1)
#define PB6_ALT2                (PB6    | ALT2)
#define PB7_ALT1                (PB7    | ALT1)
#define PB8_ALT1                (PB8    | ALT1)
#define PB9_ALT1                (PB9    | ALT1)
#define PB13_ALT1               (PB13   | ALT1)
#define PB14_ALT1               (PB14   | ALT1)
#define PB15_ALT1               (PB15   | ALT1)
#define PB15_ALT2               (PB15   | ALT2)
#define PC6_ALT1                (PC6    | ALT1)
#define PC7_ALT1                (PC7    | ALT1)

/*----------------------------------------------------------------------------
 *        Pin count
 *----------------------------------------------------------------------------*/
#define NUM_DIGITAL_PINS        46
#define NUM_REMAP_PINS          2
#define NUM_ANALOG_INPUTS       14

/*----------------------------------------------------------------------------
 *        Board identity
 *----------------------------------------------------------------------------*/
#define LED_BUILTIN             PB13    // Discrete status LED
#define USER_BTN                PB4     // Encoder push button, active-low
#define USER_BTN_ACTIVE         LOW

/*----------------------------------------------------------------------------
 *        Default peripheral bus pins
 *        SPI1 = LCD bus; I2C2 = touch / expansion
 *----------------------------------------------------------------------------*/
#define PIN_SPI_SS              PA4
#define PIN_SPI_SS1             PA15
#define PIN_SPI_SS2             PB0
#define PIN_SPI_SS3             PNUM_NOT_DEFINED
#define PIN_SPI_MOSI            PA7
#define PIN_SPI_MISO            PA6
#define PIN_SPI_SCK             PA5

#define PIN_WIRE_SDA            PB11
#define PIN_WIRE_SCL            PB10

// Hardware UART instance for Serial (overridden to CDC by USBCON build flags)
#define SERIAL_UART_INSTANCE    4
#define PIN_SERIAL_RX           PA1
#define PIN_SERIAL_TX           PA0

/*----------------------------------------------------------------------------
 *        Timer defaults
 *----------------------------------------------------------------------------*/
#define TIMER_TONE              TIM6
#define TIMER_SERVO             TIM7

/*----------------------------------------------------------------------------
 *        HAL modules
 *----------------------------------------------------------------------------*/
#if !defined(HAL_DAC_MODULE_DISABLED)
  #define HAL_DAC_MODULE_ENABLED
#endif

/*----------------------------------------------------------------------------
 *        Reflow board pin aliases  (replaces src/board.hpp)
 *----------------------------------------------------------------------------*/
// Rotary encoder
#define PIN_ROT_A               PC14
#define PIN_ROT_B               PC13
#define PIN_ROT_BTN             PB4

// Boot / user button SW2 on PA14 (SWCLK and hardware BOOT0 when nBOOT_SEL=0).
// Active-HIGH: 3V3 through SW2; R44 100 kΩ pull-down to GND.
// Never configure as OUTPUT — PA14 is the STLink SWD clock.
#define PIN_BOOT_BTN            PA14

// Audio & status
#define PIN_BUZZER              PF0    // TIM14_CH1
#define PIN_WS2812              PF1    // single-pixel RGB
#define PIN_USER_LED            PB13   // discrete LED

// LCD (SPI1)
#define PIN_LCD_BL              PA2    // backlight via Q6, TIM15_CH1
#define PIN_LCD_DC              PA3
#define PIN_LCD_CS              PA4
#define PIN_LCD_SCK             PA5
#define PIN_LCD_MOSI            PA7
#define PIN_LCD_RST             PB0

// Touch controller (optional, I2C2)
#define PIN_TOUCH_RST           PB1
#define PIN_TOUCH_INT           PB2
#define PIN_I2C_SCL             PB10   // I2C2_SCL
#define PIN_I2C_SDA             PB11   // I2C2_SDA

// Thermocouples MAX6675 (SPI2)
#define PIN_TC_SCK              PD1    // SPI2_SCK
#define PIN_TC_MISO             PD3    // SPI2_MISO
#define PIN_TC_CS_0             PD2    // MAX6675 U1
#define PIN_TC_CS_1             PD0    // MAX6675 U4
#define PIN_TC_CS_2             PA15   // MAX6675 U5

// NTC door-motor temperature (ADC1_IN0)
#define PIN_NTC                 PA0

// Door motor DRV8251A
#define PIN_MOTOR_IN1           PB5
#define PIN_MOTOR_IN2           PB6

// Load FETs / SSRs (LOAD0/1/2 — role assigned in LoadMap at runtime)
#define PIN_FET_0               PB7    // J12
#define PIN_FET_1               PB8    // J13
#define PIN_FET_2               PB9    // J14

// Spare servo output
#define PIN_SERVO               PB3    // J3, TIM2_CH2

/*----------------------------------------------------------------------------
 *        Arduino serial port aliases
 *----------------------------------------------------------------------------*/
#ifdef __cplusplus
  #ifndef SERIAL_PORT_MONITOR
    #define SERIAL_PORT_MONITOR   Serial
  #endif
  #ifndef SERIAL_PORT_HARDWARE
    #define SERIAL_PORT_HARDWARE  Serial
  #endif

  // True when nBOOT_SEL=0, i.e. BOOT0 is sourced from the PA14 pin.
  bool boot0_pin_enabled();

  // Programs nBOOT_SEL=0 and resets via the option byte loader. Does not return.
  void enable_boot0_pin();
#endif
