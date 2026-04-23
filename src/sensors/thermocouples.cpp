#include "thermocouples.hpp"
#include <MAX6675.hpp>
#include <SPI.h>

namespace sensors {

// SPI2 is shared across all three MAX6675 chips; we switch CS per transaction.
static SPIClass spi_tc(PIN_TC_MISO, /*miso*/ PIN_TC_MISO,
                       PIN_TC_SCK);

static driver::MAX6675 tc[3] = {
  driver::MAX6675(spi_tc, PIN_TC_CS_0),
  driver::MAX6675(spi_tc, PIN_TC_CS_1),
  driver::MAX6675(spi_tc, PIN_TC_CS_2),
};

// IIR state per channel. α = 0.3 — re-tune after the first thermal run.
static float filtered[3] = {0, 0, 0};
static bool  open_tc[3]  = {false, false, false};
static bool  fresh[3]    = {false, false, false};
static uint8_t rr = 0;    // round-robin index

void thermocouples_init() {
  spi_tc.begin();
  for (auto& ch : tc) ch.begin();
}

void thermocouples_tick() {
  const auto r = tc[rr].read();
  if (!r.open_tc) {
    filtered[rr] = 0.3f * r.celsius + 0.7f * filtered[rr];
  }
  open_tc[rr] = r.open_tc;
  fresh[rr] = true;
  rr = (rr + 1) % 3;
}

TcReading read(TcRole role) {
  // TODO: honour SensorMap from settings; currently role == physical index.
  const auto i = static_cast<uint8_t>(role);
  TcReading out{ filtered[i], open_tc[i], fresh[i] };
  fresh[i] = false;
  return out;
}

}  // namespace sensors
