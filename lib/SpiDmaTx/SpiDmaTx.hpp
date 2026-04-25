#pragma once
// SpiDmaTx — TX-only SPI bus driven by STM32 HAL with optional DMA.
//
// Wraps a single SPI peripheral + a single DMA channel. Provides:
//   - transmit(buf, len)      polled, blocking. Use for small payloads.
//   - transmit_dma(buf, len)  kicks DMA, returns immediately.
//   - wait_idle()             blocks until any in-flight DMA completes.
//
// MISO is intentionally not configured — this is a TX-only bus. The display,
// MAX6675-style sensors that have their own bus, and write-only peripherals
// all fit. Add an RX path here if/when needed.
//
// IRQ trampolines for the DMA channels we support live in SpiDmaTx.cpp.
// Currently DMA1_Channel1 only; DMA1_Channel2_3 / DMA1_Channel4_5_6_7 can be
// added with one extra static pointer + one trampoline each.

#include <Arduino.h>

namespace driver {

class SpiDmaTx {
 public:
  SpiDmaTx(SPI_TypeDef* spi,
           uint32_t mosi_pin, uint32_t sck_pin,
           DMA_Channel_TypeDef* dma_ch,
           uint32_t dma_request,           // DMA_REQUEST_SPIx_TX
           IRQn_Type dma_irq);

  // baudrate_prescaler is one of SPI_BAUDRATEPRESCALER_2..256.
  void begin(uint32_t baudrate_prescaler);

  // Polled, blocking. Use for small payloads (commands, init headers).
  void transmit(const uint8_t* buf, size_t len);

  // Kicks DMA, returns once started. Caller must keep `buf` alive until
  // wait_idle() returns. Buffer must be in DMA-accessible RAM (any SRAM on G0).
  void transmit_dma(const uint8_t* buf, size_t len);

  // Block until any in-flight DMA finishes and the SPI shift register drains.
  void wait_idle();

  // Non-blocking check — true once the bus is ready for a new transfer.
  // Lets callers run other work (e.g. USB tud_task()) while waiting.
  bool is_idle();

  // Exposed so the IRQ trampoline can call HAL_DMA_IRQHandler.
  DMA_HandleTypeDef& dma_handle() { return hdma_; }

 private:
  SPI_HandleTypeDef    hspi_{};
  DMA_HandleTypeDef    hdma_{};
  SPI_TypeDef*         spi_;
  uint32_t             mosi_pin_;
  uint32_t             sck_pin_;
  DMA_Channel_TypeDef* dma_ch_;
  uint32_t             dma_request_;
  IRQn_Type            dma_irq_;
};

}  // namespace driver
