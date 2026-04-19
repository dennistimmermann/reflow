#include "lv_port_disp.hpp"
#include "../board.hpp"
#include <lvgl.h>
#include <SPI.h>
#include <GC9A01.hpp>

namespace ui {

static constexpr uint16_t H = 240;
static constexpr uint16_t W = 240;
static constexpr uint32_t BUF_PX = (W * H) / 10;     // 1/10 of the screen

static SPIClass spi_lcd(board::PIN_LCD_MOSI, /*miso*/ PNUM_NOT_DEFINED, board::PIN_LCD_SCK);
static driver::GC9A01 lcd(spi_lcd, board::PIN_LCD_CS, board::PIN_LCD_DC,
                          board::PIN_LCD_RST, board::PIN_LCD_BL);

static lv_color_t buf_a[BUF_PX];
static lv_color_t buf_b[BUF_PX];
static lv_display_t* disp_ = nullptr;

static void flush_cb(lv_display_t* d, const lv_area_t* area, uint8_t* px) {
  lcd.set_window(area->x1, area->y1, area->x2, area->y2);
  const uint32_t count = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
  lcd.blit(reinterpret_cast<uint16_t*>(px), count);
  lv_display_flush_ready(d);
}

void lv_port_disp_init() {
  spi_lcd.begin();
  lcd.begin();

  disp_ = lv_display_create(W, H);
  lv_display_set_buffers(disp_, buf_a, buf_b, sizeof(buf_a),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp_, flush_cb);
}

}  // namespace ui
