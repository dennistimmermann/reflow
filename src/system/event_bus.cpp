#include "event_bus.hpp"

namespace sys {

// Meyers singleton — initialised on first use, regardless of cross-TU
// static init order.
EventBus& bus() {
  static EventBus g_bus;
  return g_bus;
}

}  // namespace sys
