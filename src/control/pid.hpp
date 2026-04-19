#pragma once
#include <algorithm>

namespace control {

// Classic PID with anti-windup clamping and derivative-on-measurement.
// Tuned per-axis in heater_control.
class Pid {
 public:
  struct Gains { float kp, ki, kd; };

  Pid(Gains g, float out_min, float out_max)
    : g_(g), out_min_(out_min), out_max_(out_max) {}

  void reset(float measurement) {
    integ_ = 0.0f;
    last_meas_ = measurement;
    have_last_ = true;
  }

  float step(float setpoint, float measurement, float dt_s) {
    const float err = setpoint - measurement;
    integ_ += err * dt_s;

    float deriv = 0.0f;
    if (have_last_) deriv = -(measurement - last_meas_) / dt_s;   // on-measurement
    last_meas_ = measurement;
    have_last_ = true;

    float out = g_.kp * err + g_.ki * integ_ + g_.kd * deriv;

    // Back-calc anti-windup: if clamped, unwind the integrator.
    if (out > out_max_) { integ_ -= (out - out_max_) / (g_.ki + 1e-6f); out = out_max_; }
    if (out < out_min_) { integ_ += (out_min_ - out) / (g_.ki + 1e-6f); out = out_min_; }
    return out;
  }

  void set_gains(Gains g) { g_ = g; }

 private:
  Gains g_;
  float out_min_, out_max_;
  float integ_ = 0.0f;
  float last_meas_ = 0.0f;
  bool  have_last_ = false;
};

}  // namespace control
