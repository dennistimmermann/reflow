#include "storage.hpp"

namespace sys {

// TODO: implement flash EEPROM emulation (last two flash sectors).
// Plan: record = [key:u16][len:u16][data][crc32]. Ping-pong between sectors.

void storage_init() {}
bool storage_read (uint16_t, void*, size_t) { return false; }
bool storage_write(uint16_t, const void*, size_t) { return false; }
void storage_erase_all() {}

}  // namespace sys
