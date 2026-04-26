#include "thermal_profile.hpp"

namespace control {

// SAC305 lead-free typical profile — ramp / soak / reflow / cool.
const ThermalProfile kProfileSAC305 = {
  "SAC305",
  {
    { 150.0f, 90,  2.0f },   // ramp to soak
    { 180.0f, 90,  0.7f },   // soak
    { 245.0f, 60,  2.0f },   // reflow peak
    {  60.0f, 120, 4.0f },   // cool
  },
  4
};

// 63/37 leaded Sn-Pb profile.
const ThermalProfile kProfileSnPb = {
  "Sn63Pb37",
  {
    { 130.0f, 75,  2.0f },
    { 165.0f, 90,  0.6f },
    { 215.0f, 45,  2.0f },
    {  60.0f, 120, 4.0f },
  },
  4
};

float profile_target_at(const ThermalProfile& p, float elapsed_s, bool* done) {
  *done = false;
  float t_start = 0.0f;
  float prev_target = 25.0f;
  for (uint8_t i = 0; i < p.segment_count; ++i) {
    const float t_end = t_start + p.segments[i].duration_s;
    if (elapsed_s <= t_end) {
      const float k = (elapsed_s - t_start) / p.segments[i].duration_s;
      return prev_target + k * (p.segments[i].target_c - prev_target);
    }
    t_start = t_end;
    prev_target = p.segments[i].target_c;
  }
  *done = true;
  return prev_target;
}

}  // namespace control
