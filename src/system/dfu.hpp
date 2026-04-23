#pragma once
#include <Arduino.h>

namespace sys {

// Magic written to TAMP->BKP0R to request DFU entry on the next boot. TAMP
// backup registers survive a software reset (NVIC_SystemReset) but are
// cleared by power-on reset — exactly the semantics we need. Consumed and
// cleared in initVariant(); see variants/reflow_oven/variant.cpp.
constexpr uint32_t kDfuMagic = 0xB007DF00u;

// Drive outputs safe, stash the magic, reset. initVariant() jumps to the
// STM32G0 ROM DFU bootloader on the next boot — before USB comes back up.
// Going through a full system reset guarantees D+ drops long enough for
// the host to see a disconnect and re-enumerate us as the DFU device.
//
// Called from the TinyUSB DFU runtime detach callback (src/system/dfu_rt.cpp)
// and may also be wired to a UI/button later.
[[noreturn]] inline void enter_dfu() {
    digitalWrite(PIN_FET_0, LOW);
    digitalWrite(PIN_FET_1, LOW);
    digitalWrite(PIN_FET_2, LOW);
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    __HAL_RCC_PWR_CLK_ENABLE();      // PWR controller must be clocked to touch CR1
    __HAL_RCC_RTCAPB_CLK_ENABLE();   // TAMP backup registers live on the RTC APB clock
    HAL_PWR_EnableBkUpAccess();      // STM32G0 gates BKPxR writes behind PWR->CR1.DBP
    TAMP->BKP0R = kDfuMagic;

    NVIC_SystemReset();
    while (true) {}
}

} // namespace sys
