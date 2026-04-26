#include "thermocouples.hpp"
#include "../system/store.hpp"
#include <MAX6675.hpp>
#include <SPI.h>

namespace sensors {

// SPI2 is shared across all three MAX6675 chips; we switch CS per transaction.
// MAX6675 is receive-only, but STM32duino's spi_init bails silently unless MOSI
// resolves to the target SPI peripheral. Workaround: point MOSI at PB15 — an
// unused pin that happens to be a valid SPI2_MOSI AF. The peripheral wiggles it
// harmlessly while we clock data in on PD3.
static SPIClass spi_tc(PB15, PIN_TC_MISO, PIN_TC_SCK);

static driver::MAX6675 tc[3] = {
  driver::MAX6675(spi_tc, PIN_TC_CS_0),
  driver::MAX6675(spi_tc, PIN_TC_CS_1),
  driver::MAX6675(spi_tc, PIN_TC_CS_2),
};

// IIR state per channel. α = 0.3 — re-tune after the first thermal run.
static float filtered[3] = {0, 0, 0};
static uint8_t rr = 0;    // round-robin index

static sys::Slot<float>* tc_slots[3] = {
  &sys::store().tc_top,
  &sys::store().tc_bottom,
  &sys::store().tc_target,
};
static sys::Slot<bool>* tc_open_slots[3] = {
  &sys::store().tc_top_open,
  &sys::store().tc_bottom_open,
  &sys::store().tc_target_open,
};

void ThermocoupleTask::on_init() {
  spi_tc.begin();
  for (auto& ch : tc) ch.begin();
}

void ThermocoupleTask::on_tick() {
  const auto r = tc[rr].read();
  if (!r.open_tc) {
    filtered[rr] = 0.3f * r.celsius + 0.7f * filtered[rr];
    tc_slots[rr]->set(filtered[rr]);
  }
  tc_open_slots[rr]->set(r.open_tc);
  rr = (rr + 1) % 3;
}

}  // namespace sensors
