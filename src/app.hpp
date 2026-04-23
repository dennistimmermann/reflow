#pragma once

// Composition root for the reflow oven firmware.
// Wires sensors, control loops, UI and safety together. Called from main.
namespace app {

void init();
void tick();

}  // namespace app
