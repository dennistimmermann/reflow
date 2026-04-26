#include "store.hpp"

namespace sys {

// Meyers singleton — initialised on first call, regardless of cross-TU
// static init order. Necessary because Slot pointer tables in other
// modules (e.g. ThermocoupleTask) reference store() from their own
// namespace-scope statics.
Store& store() {
  static Store g_store;
  return g_store;
}

}  // namespace sys
