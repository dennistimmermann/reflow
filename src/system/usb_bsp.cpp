// Board-support glue for TinyUSB on the STM32G0B1. TinyUSB's
// `dcd_stm32_fsdev` driver needs HSI48 + USB peripheral clock + NVIC hook.
// HSI48 itself is already turned on in variants/reflow_oven/variant.cpp
// inside SystemClock_Config, so we only handle the USB-specific bits here.

#include <Arduino.h>
#include <tusb.h>

extern "C" void usb_bsp_init(void) {
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    // STM32G0B1 gates the USB transceiver behind PWR->CR2.USV — without this
    // the peripheral is clocked but D+ never pulls up and the host sees
    // nothing. (STM32duino's USBDevice stack used to do this for us.)
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWREx_EnableVddUSB();

    __HAL_RCC_USB_CLK_ENABLE();

    // Lock HSI48 to the USB host SOF. Without this the stock HSI48 is good
    // to ~1%, which is just inside USB FS spec under perfect conditions —
    // CRS sync makes it sit at ~0.1% regardless of temperature.
    __HAL_RCC_CRS_CLK_ENABLE();
    RCC_CRSInitTypeDef crs = {};
    crs.Prescaler             = RCC_CRS_SYNC_DIV1;
    crs.Source                = RCC_CRS_SYNC_SOURCE_USB;
    crs.Polarity              = RCC_CRS_SYNC_POLARITY_RISING;
    crs.ReloadValue           = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000, 1000);
    crs.ErrorLimitValue       = 34;
    crs.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;
    HAL_RCCEx_CRSConfig(&crs);

    NVIC_SetPriority(USB_UCPD1_2_IRQn, 2);
    NVIC_EnableIRQ(USB_UCPD1_2_IRQn);
}

// USB + UCPD1/2 share one vector on G0B1. Forward to TinyUSB; we don't use
// UCPD, so no other handler competes for the IRQ.
extern "C" void USB_UCPD1_2_IRQHandler(void) {
    tud_int_handler(0);
}
