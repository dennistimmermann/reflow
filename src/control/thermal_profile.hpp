#pragma once
#include <stdint.h>

namespace control {

struct ProfileSegment {
  float    target_c;
  uint16_t duration_s;
  float    max_slope_c_per_s;
};

// A thermal program — used for reflow, annealing, or plain bake.
// The kind of program is determined by the segment data, not by a flag.
struct ThermalProfile {
  char           name[16];
  ProfileSegment segments[8];
  uint8_t        segment_count;
};

// Built-in factory presets — loaded at first boot.
extern const ThermalProfile kProfileSAC305;
extern const ThermalProfile kProfileSnPb;

// Interpolated target at a point in the profile. Returns the last
// segment's target and sets done=true when the profile has elapsed.
float profile_target_at(const ThermalProfile& p, float elapsed_s, bool* done);

}  // namespace control
