#include "mcu_temp.hpp"
#include "../system/store.hpp"
#include <Arduino.h>
#include <stm32g0xx_ll_adc.h>

namespace sensors {

static constexpr uint8_t kSamples = 16;

void McuTempTask::on_init() {
  // ADC channels for VREFINT and the internal temperature sensor are
  // already enabled by Arduino's analogRead path; nothing to set up.
}

void McuTempTask::on_tick() {
  uint32_t vref_acc = 0;
  uint32_t ts_acc   = 0;
  for (uint8_t i = 0; i < kSamples; ++i) {
    vref_acc += analogRead(AVREF);
    ts_acc   += analogRead(ATEMP);
  }
  const float vref = static_cast<float>(vref_acc) / kSamples;
  const float ts   = static_cast<float>(ts_acc)   / kSamples;

  // Normalise ts to calibration reference voltage. Both cal tables use
  // the same reference so VDDA cancels: ts_cal = ts * vrefint_cal / vref.
  const float ts_cal = ts * static_cast<float>(*VREFINT_CAL_ADDR) / vref;

  const float c = (ts_cal - static_cast<float>(*TEMPSENSOR_CAL1_ADDR))
      * static_cast<float>(TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP)
      / static_cast<float>(*TEMPSENSOR_CAL2_ADDR - *TEMPSENSOR_CAL1_ADDR)
      + static_cast<float>(TEMPSENSOR_CAL1_TEMP);

  sys::store().mcu_temp.set(c);
}

}  // namespace sensors
