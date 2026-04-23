#pragma once
// Non-blocking piezo buzzer driver. PWM via TIM14_CH1 on PF0.
// Melodies are walked by elapsed time in update() — no ISR, no callbacks.
// Single-file inline-header library per CLAUDE.md §5.
//
// Note: ::tone() uses TIM6 + GPIO bit-bang and cannot control duty/volume.
// We own TIM14 directly via HardwareTimer for real PWM and volume control.

#include <Arduino.h>
#include <HardwareTimer.h>

namespace driver {

struct Note {
    uint16_t hz;  // 0 = rest (silence)
    uint16_t ms;  // 0 = end-of-melody sentinel
};

namespace notes {
    constexpr uint16_t REST = 0;
    // Octave 3
    constexpr uint16_t C3 = 131, D3 = 147, E3 = 165, F3 = 175;
    constexpr uint16_t G3 = 196, A3 = 220, B3 = 247;
    // Octave 4
    constexpr uint16_t C4 = 262, D4 = 294, E4 = 330, F4 = 349;
    constexpr uint16_t G4 = 392, A4 = 440, B4 = 494;
    // Octave 5
    constexpr uint16_t C5 = 523, D5 = 587, E5 = 659, F5 = 698;
    constexpr uint16_t G5 = 784, A5 = 880, B5 = 988;
    // Octave 6
    constexpr uint16_t C6 = 1047, D6 = 1175, E6 = 1319, F6 = 1397;
    constexpr uint16_t G6 = 1568, A6 = 1760, B6 = 1976;
    // Octave 7
    constexpr uint16_t C7 = 2093;
}  // namespace notes

namespace melodies {
    using N = Note;
    using namespace notes;

    // Single short blip — subtle rotary encoder click
    inline constexpr N tick[]     = { {2800, 12}, {0, 0} };
    // Quick ascending duo — button confirm
    inline constexpr N confirm[]  = { {C5,  55}, {E5,  75}, {0, 0} };
    // Mirrors confirm, descending — back / cancel
    inline constexpr N back[]     = { {E5,  60}, {C5,  85}, {0, 0} };
    // Snappy up-swoop — generic chirp
    inline constexpr N chirp_up[] = { {C5,  40}, {G5,  45}, {0, 0} };
    // Major arpeggio — boot complete
    inline constexpr N startup[]  = { {C5,  90}, {E5,  90}, {G5,  90}, {C6, 220}, {0, 0} };
    // Ascending resolution — reflow profile done
    inline constexpr N done[]     = { {E5, 130}, {G5, 130}, {C6, 320}, {0, 0} };
    // Double alarm then drop — fault / error
    inline constexpr N error[]    = { {A4, 110}, {REST, 40}, {A4, 110}, {REST, 40}, {E4, 220}, {0, 0} };
}  // namespace melodies

class Buzzer {
 public:
    explicit Buzzer(uint32_t pin) : pin_(pin), ht_(TIM14) {}

    void begin() {
        ht_.setMode(1, TIMER_OUTPUT_COMPARE_PWM1, pin_);
    }

    void play(const Note* melody) {
        melody_     = melody;
        note_idx_   = 0;
        prev_idx_   = 0xFF;  // force apply on first tick
        note_start_ = millis();
    }

    void tone_ms(uint16_t hz, uint16_t ms) {
        single_[0] = {hz, ms};
        single_[1] = {0, 0};
        play(single_);
    }

    // 0–100. Quadratic curve → 0–50% PWM duty (50% = max square wave).
    // Below ~14 is barely audible; 0 is silent.
    void set_volume(uint8_t v) { volume_ = v; }

    void beep()     { tone_ms(2000, 80); }
    void chirp_up() { play(melodies::chirp_up); }

    void mute() {
        ht_.pause();
        digitalWrite(pin_, LOW);
        melody_ = nullptr;
    }

    // Call from the scheduler tick.
    void update() {
        if (!melody_) return;

        // Advance index while the current note has expired.
        while (melody_[note_idx_].ms != 0 &&
               millis() - note_start_ >= melody_[note_idx_].ms) {
            note_start_ += melody_[note_idx_].ms;
            ++note_idx_;
        }

        if (melody_[note_idx_].ms == 0) { mute(); return; }
        if (note_idx_ == prev_idx_) return;
        prev_idx_ = note_idx_;

        if (melody_[note_idx_].hz) {
            ht_.setOverflow(melody_[note_idx_].hz, HERTZ_FORMAT);
            // Quadratic: vol=100→50%, vol=50→12.5%, vol=0→silent.
            // Clamp non-zero to at least 1% so quiet notes stay audible.
            uint32_t pct = (uint32_t)volume_ * volume_ / 200;
            if (volume_ && !pct) pct = 1;
            ht_.setCaptureCompare(1, pct, PERCENT_COMPARE_FORMAT);
            ht_.resume();
        } else {
            ht_.pause();
            digitalWrite(pin_, LOW);
        }
    }

 private:
    uint32_t      pin_;
    HardwareTimer ht_;
    uint8_t       volume_     = 5;
    const Note*   melody_     = nullptr;
    uint8_t       note_idx_   = 0;
    uint8_t       prev_idx_   = 0xFF;
    uint32_t      note_start_ = 0;
    Note          single_[2];  // scratch buffer for tone_ms()
};

}  // namespace driver
