#pragma once
#include <stdint.h>
#include <stddef.h>

// Persistent settings/profiles via STM32 flash EEPROM emulation.
// CRC-tagged, ping-pong writes — see CLAUDE.md §10.
namespace sys {

void storage_init();

// Returns false if the slot is empty or CRC-invalid.
bool storage_read (uint16_t key, void* out, size_t len);
bool storage_write(uint16_t key, const void* in, size_t len);
void storage_erase_all();

}  // namespace sys
