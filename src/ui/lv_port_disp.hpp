#pragma once

namespace ui {

// Wires the GC9A01 driver into LVGL's display interface. Allocates two
// partial-refresh buffers (1/10 screen each) in RAM.
void lv_port_disp_init();

}  // namespace ui
