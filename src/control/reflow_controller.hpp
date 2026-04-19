#pragma once
#include "reflow_profile.hpp"

namespace control {

enum class ReflowState { IDLE, PREHEAT, SOAK, REFLOW, COOL, DONE, FAULT };

void reflow_init();
void reflow_tick();

void reflow_start(const ReflowProfile& p);
void reflow_abort();
ReflowState reflow_state();
float reflow_elapsed_s();

}  // namespace control
