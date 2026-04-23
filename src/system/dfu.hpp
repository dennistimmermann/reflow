#pragma once
#include <Arduino.h>

// USBD_CDC_LineCodingTypeDef::bitrate is at offset 0; exported non-static from usbd_cdc_if.c
extern "C" { extern uint32_t linecoding; }

namespace sys {

// True when the host has opened the CDC port at 1200 baud (Arduino auto-reset signal).
inline bool dfu_requested() {
#ifdef USBCON
    return linecoding == 1200;
#else
    return false;
#endif
}

// Deinit peripherals and jump to the STM32G0B1 ROM DFU bootloader (0x1FFF0000).
// GPIO is preserved across the jump (no reset), so driving FETs LOW here keeps
// loads off for the entire DFU session. Does not return.
inline void enter_dfu() {
    digitalWrite(PIN_FET_0, LOW);
    digitalWrite(PIN_FET_1, LOW);
    digitalWrite(PIN_FET_2, LOW);
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    __disable_irq();
    HAL_RCC_DeInit();
    HAL_DeInit();
    SysTick->CTRL = 0;
    __set_MSP(*reinterpret_cast<const uint32_t*>(0x1FFF0000));
    reinterpret_cast<void(*)()>(*reinterpret_cast<const uint32_t*>(0x1FFF0004))();
}

} // namespace sys
