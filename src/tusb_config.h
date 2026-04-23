#pragma once
// TinyUSB configuration for the reflow oven controller.
// CFG_TUSB_MCU, CFG_TUSB_OS, BOARD_TUD_RHPORT, BOARD_TUD_MAX_SPEED come from
// build_flags in platformio.ini.

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CFG_TUSB_MCU
#error "CFG_TUSB_MCU must be defined (set in platformio.ini)"
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS            OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG         0
#endif

#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN     __attribute__((aligned(4)))
#endif

// Device stack
#define CFG_TUD_ENABLED        1
#define CFG_TUD_MAX_SPEED      BOARD_TUD_MAX_SPEED
#define CFG_TUD_ENDPOINT0_SIZE 64

// Classes: one CDC + one DFU runtime interface.
#define CFG_TUD_CDC            1
#define CFG_TUD_DFU_RUNTIME    1

// CDC buffer sizes. EP size 64 is max for USB FS; ring buffers larger so the
// host can drain bursts without losing bytes while we're busy in loop().
#define CFG_TUD_CDC_RX_BUFSIZE 256
#define CFG_TUD_CDC_TX_BUFSIZE 256
#define CFG_TUD_CDC_EP_BUFSIZE 64

#ifdef __cplusplus
}
#endif
