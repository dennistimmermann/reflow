// LVGL 9 configuration for the reflow oven (240×240 round).
// Trimmed to what we actually use — revisit as features land.
#pragma once

#define LV_COLOR_DEPTH            16
#define LV_COLOR_16_SWAP          1

#define LV_MEM_SIZE               (48U * 1024U)
#define LV_USE_STDLIB_MALLOC      LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING      LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF     LV_STDLIB_BUILTIN

#define LV_DEF_REFR_PERIOD        16      // ~60 FPS target
#define LV_DPI_DEF                130

#define LV_USE_LOG                1
#define LV_LOG_PRINTF             1
#define LV_LOG_LEVEL              LV_LOG_LEVEL_WARN

#define LV_USE_PERF_MONITOR       0
#define LV_USE_MEM_MONITOR        0

// Widgets we use.
#define LV_USE_LABEL              1
#define LV_USE_ARC                1
#define LV_USE_BAR                1
#define LV_USE_BUTTON             1
#define LV_USE_CHART              1
#define LV_USE_ROLLER             1
#define LV_USE_SLIDER             1
#define LV_USE_SWITCH             1
#define LV_USE_MSGBOX             1

// Inputs.
#define LV_USE_INDEV_ENCODER      1

// Fonts — keep compact.
#define LV_FONT_MONTSERRAT_14     1
#define LV_FONT_MONTSERRAT_20     1
#define LV_FONT_MONTSERRAT_28     1
#define LV_FONT_DEFAULT           &lv_font_montserrat_14
