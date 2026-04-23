#pragma once
#include <Arduino.h>

// Detection uses the two signals STM32duino's CDC stack already exports.
// `linecoding` is a USBD_CDC_LineCodingTypeDef whose first field is the
// 32-bit bitrate, so aliasing it as uint32_t reads the baud rate portably.
extern "C" {
    extern uint32_t     linecoding;  // first field of USBD_CDC_LineCodingTypeDef: bitrate
    extern volatile bool dtrState;   // host DTR state, updated in the USB IRQ
}

namespace sys {

// Magic written to TAMP->BKP0R to request DFU entry on the next boot. TAMP
// backup registers survive a software reset (NVIC_SystemReset) but are
// cleared by power-on reset — exactly the semantics we need. Consumed and
// cleared in initVariant(); see variants/reflow_oven/variant.cpp.
constexpr uint32_t kDfuMagic = 0xB007DF00u;

// True when the host has opened the CDC port at 1200 baud and then dropped
// DTR — the canonical Arduino "reset into bootloader" signal. We gate on
// the DTR falling edge rather than the raw line-coding value so stale state
// from a previous session can't trigger DFU on its own.
inline bool dfu_requested() {
#ifdef USBCON
    static bool prev_dtr = false;
    const bool now        = dtrState;
    const bool edge_down  = prev_dtr && !now;
    prev_dtr = now;
    return edge_down && linecoding == 1200;
#else
    return false;
#endif
}

// Drive outputs safe, stash the magic, and reset. initVariant() jumps to
// the STM32G0 ROM DFU bootloader on the next boot — before USB is brought
// back up. Going through a full system reset (rather than jumping directly
// from the running USB stack) guarantees D+ goes low long enough for the
// host to see a disconnect and re-enumerate us cleanly as the DFU device.
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
