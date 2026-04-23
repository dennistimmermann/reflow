#include "mcu_temp.hpp"
#include <Arduino.h>
#include <stm32g0xx_ll_adc.h>

namespace sensors {

static constexpr uint8_t kSamples = 16;

static float    last_c   = 25.0f;
static uint32_t vref_acc = 0;
static uint32_t ts_acc   = 0;
static uint8_t  count    = 0;

void mcu_temp_update() {
    vref_acc += analogRead(AVREF);
    ts_acc   += analogRead(ATEMP);
    if (++count < kSamples) return;

    const float vref = static_cast<float>(vref_acc) / kSamples;
    const float ts   = static_cast<float>(ts_acc)   / kSamples;
    vref_acc = ts_acc = count = 0;

    // Normalise ts to calibration reference voltage. Both cal tables use the
    // same reference so VDDA cancels: ts_cal = ts * vrefint_cal / vref.
    const float ts_cal = ts * static_cast<float>(*VREFINT_CAL_ADDR) / vref;

    last_c = (ts_cal - static_cast<float>(*TEMPSENSOR_CAL1_ADDR))
             * static_cast<float>(TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP)
             / static_cast<float>(*TEMPSENSOR_CAL2_ADDR - *TEMPSENSOR_CAL1_ADDR)
             + static_cast<float>(TEMPSENSOR_CAL1_TEMP);
}

float mcu_temp_celsius() { return last_c; }

}  // namespace sensors
