#include <tusb.h>
#include "system/dfu.hpp"

// Called by TinyUSB's DFU runtime class when the host sends DFU_DETACH on
// the runtime interface. We mark the boot-time DFU request (TAMP magic)
// and trigger a system reset. The early hook in variant.cpp then lands
// the ROM bootloader — see maybe_enter_rom_dfu().
extern "C" void tud_dfu_runtime_reboot_to_dfu_cb(void) {
    sys::enter_dfu();
}
