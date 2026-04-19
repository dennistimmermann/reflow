#pragma once
#include <stdint.h>

namespace control {

struct ProfileSegment {
  float target_c;
  uint16_t duration_s;
  float max_slope_c_per_s;
};

struct ReflowProfile {
  char name[16];
  ProfileSegment segments[8];
  uint8_t segment_count;
};

// Built-in factory presets — loaded at first boot.
extern const ReflowProfile kProfileSAC305;
extern const ReflowProfile kProfileSnPb;

// Interpolated target at a point in the profile. Returns 0 and sets done=true
// when the profile has elapsed.
float profile_target_at(const ReflowProfile& p, float elapsed_s, bool* done);

}  // namespace control
