#include "SpiDmaTx.hpp"
#include <pinmap.h>

namespace driver {

namespace {
// One static pointer per supported DMA channel. Trampoline below dispatches.
// Add more entries (and matching trampolines) as more channels are wired up.
SpiDmaTx* dma1_ch1_owner = nullptr;
}  // namespace

SpiDmaTx::SpiDmaTx(SPI_TypeDef* spi,
                   uint32_t mosi_pin, uint32_t sck_pin,
                   DMA_Channel_TypeDef* dma_ch,
                   uint32_t dma_request,
                   IRQn_Type dma_irq)
  : spi_(spi),
    mosi_pin_(mosi_pin), sck_pin_(sck_pin),
    dma_ch_(dma_ch), dma_request_(dma_request), dma_irq_(dma_irq) {}

void SpiDmaTx::begin(uint32_t baudrate_prescaler) {
  // GPIO AF for MOSI / SCK using STM32duino's own pinmap helper. MISO stays
  // unconfigured; this is a TX-only bus.
  pinmap_pinout(digitalPinToPinName(mosi_pin_), PinMap_SPI_MOSI);
  pinmap_pinout(digitalPinToPinName(sck_pin_),  PinMap_SPI_SCLK);

  // Peripheral clocks. Only the SPI instances that exist on STM32G0 are
  // dispatched here; expand as we grow.
  if (spi_ == SPI1) {
    __HAL_RCC_SPI1_CLK_ENABLE();
  } else if (spi_ == SPI2) {
    __HAL_RCC_SPI2_CLK_ENABLE();
  }
  __HAL_RCC_DMA1_CLK_ENABLE();

  hspi_.Instance               = spi_;
  hspi_.Init.Mode              = SPI_MODE_MASTER;
  hspi_.Init.Direction         = SPI_DIRECTION_2LINES;
  hspi_.Init.DataSize          = SPI_DATASIZE_8BIT;
  hspi_.Init.CLKPolarity       = SPI_POLARITY_LOW;
  hspi_.Init.CLKPhase          = SPI_PHASE_1EDGE;
  hspi_.Init.NSS               = SPI_NSS_SOFT;
  hspi_.Init.BaudRatePrescaler = baudrate_prescaler;
  hspi_.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  hspi_.Init.TIMode            = SPI_TIMODE_DISABLE;
  hspi_.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  hspi_.Init.CRCPolynomial     = 7;
  HAL_SPI_Init(&hspi_);

  hdma_.Instance                 = dma_ch_;
  hdma_.Init.Request             = dma_request_;
  hdma_.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_.Init.Mode                = DMA_NORMAL;
  hdma_.Init.Priority            = DMA_PRIORITY_HIGH;
  HAL_DMA_Init(&hdma_);
  __HAL_LINKDMA(&hspi_, hdmatx, hdma_);

  // Register self for the DMA TC IRQ. HAL_SPI_Transmit_DMA tracks completion
  // via this IRQ; without it routed, HAL_SPI_GetState never returns READY.
  if (dma_irq_ == DMA1_Channel1_IRQn) {
    dma1_ch1_owner = this;
  }
  HAL_NVIC_SetPriority(dma_irq_, 1, 0);
  HAL_NVIC_EnableIRQ(dma_irq_);
}

void SpiDmaTx::transmit(const uint8_t* buf, size_t len) {
  HAL_SPI_Transmit(&hspi_, const_cast<uint8_t*>(buf), len, HAL_MAX_DELAY);
}

void SpiDmaTx::transmit_dma(const uint8_t* buf, size_t len) {
  HAL_SPI_Transmit_DMA(&hspi_, const_cast<uint8_t*>(buf), len);
}

void SpiDmaTx::wait_idle() {
  while (HAL_SPI_GetState(&hspi_) != HAL_SPI_STATE_READY) { /* spin */ }
}

bool SpiDmaTx::is_idle() {
  return HAL_SPI_GetState(&hspi_) == HAL_SPI_STATE_READY;
}

}  // namespace driver

extern "C" void DMA1_Channel1_IRQHandler() {
  if (driver::dma1_ch1_owner) {
    HAL_DMA_IRQHandler(&driver::dma1_ch1_owner->dma_handle());
  }
}
