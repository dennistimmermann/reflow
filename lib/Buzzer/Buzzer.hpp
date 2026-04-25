#pragma once
// Non-blocking piezo buzzer driver. PWM via TIM14_CH1 on PF0.
// ::tone() uses TIM6 + GPIO bit-bang — we own TIM14 via HardwareTimer
// for real PWM so duty cycle (and thus volume) can be controlled.

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
    // Octave 7 (above ~4 kHz the piezo thins out, but useful for chirps)
    constexpr uint16_t C7 = 2093, D7 = 2349, E7 = 2637, F7 = 2794;
    constexpr uint16_t G7 = 3136, A7 = 3520, B7 = 3951;
    // Accidentals (`s` = sharp; `#` is illegal in identifiers)
    constexpr uint16_t Cs3 = 139, Ds3 = 156, Fs3 = 185, Gs3 = 208, As3 = 233;
    constexpr uint16_t Cs4 = 277, Ds4 = 311, Fs4 = 370, Gs4 = 415, As4 = 466;
    constexpr uint16_t Cs5 = 554, Ds5 = 622, Fs5 = 740, Gs5 = 831, As5 = 932;
    constexpr uint16_t Cs6 = 1109, Ds6 = 1245, Fs6 = 1480, Gs6 = 1661, As6 = 1865;
    constexpr uint16_t Cs7 = 2217, Ds7 = 2489, Fs7 = 2960, Gs7 = 3322, As7 = 3729;
    // Flat aliases (Db=Cs, Eb=Ds, Gb=Fs, Ab=Gs, Bb=As) for readability in pop tunes
    constexpr uint16_t Db3 = Cs3, Eb3 = Ds3, Gb3 = Fs3, Ab3 = Gs3, Bb3 = As3;
    constexpr uint16_t Db4 = Cs4, Eb4 = Ds4, Gb4 = Fs4, Ab4 = Gs4, Bb4 = As4;
    constexpr uint16_t Db5 = Cs5, Eb5 = Ds5, Gb5 = Fs5, Ab5 = Gs5, Bb5 = As5;
    constexpr uint16_t Db6 = Cs6, Eb6 = Ds6, Gb6 = Fs6, Ab6 = Gs6, Bb6 = As6;
    constexpr uint16_t Db7 = Cs7, Eb7 = Ds7, Gb7 = Fs7, Ab7 = Gs7, Bb7 = As7;
}  // namespace notes

namespace melodies {
    using N = Note;
    using namespace notes;

    // ─── tick — subtle rotary encoder click ─────────────────────────────────
    inline constexpr N tick[]     = { {2800, 12}, {0, 0} };
    inline constexpr N tick_a[]   = { {3200,  8}, {0, 0} };
    inline constexpr N tick_b[]   = { {1800, 14}, {0, 0} };
    inline constexpr N tick_c[]   = { {2600,  8}, {REST, 12}, {2600, 8}, {0, 0} };
    inline constexpr N tick_d[]   = { {2400, 10}, {1800,  8}, {0, 0} };

    // ─── confirm — positive ack ─────────────────────────────────────────────
    inline constexpr N confirm[]    = { {C5, 55}, {E5, 75}, {0, 0} };
    inline constexpr N confirm_a[]  = { {C5, 50}, {G5, 90}, {0, 0} };           // perfect-fifth jump
    inline constexpr N confirm_b[]  = { {C5, 45}, {E5, 45}, {G5, 90}, {0, 0} }; // major triad
    inline constexpr N confirm_c[]  = { {C5, 40}, {C6, 110}, {0, 0} };          // octave snap
    inline constexpr N confirm_d[]  = { {D5, 60}, {A5, 110}, {0, 0} };          // bing-bong

    // ─── back — cancel / dismiss ────────────────────────────────────────────
    inline constexpr N back[]    = { {E5, 60}, {C5, 85}, {0, 0} };
    inline constexpr N back_a[]  = { {G5, 60}, {C5, 95}, {0, 0} };               // descending fifth
    inline constexpr N back_b[]  = { {E5, 55}, {Cs5, 95}, {0, 0} };              // minor descend
    inline constexpr N back_c[]  = { {G5, 50}, {E5, 50}, {C5, 100}, {0, 0} };    // three-note tumble
    inline constexpr N back_d[]  = { {A4, 60}, {F4, 110}, {0, 0} };              // soft thunk

    // ─── chirp_up — generic chirp ───────────────────────────────────────────
    inline constexpr N chirp_up[]    = { {C5, 40}, {G5, 45}, {0, 0} };
    inline constexpr N chirp_up_a[]  = { {E5, 30}, {A5, 30}, {D6, 50}, {0, 0} };           // 3-note slide
    inline constexpr N chirp_up_b[]  = { {G5, 45}, {Cs6, 80}, {0, 0} };                    // question rise
    inline constexpr N chirp_up_c[]  = { {E6, 25}, {REST, 30}, {E6, 25}, {G6, 45}, {0, 0} }; // birdy
    inline constexpr N chirp_up_d[]  = { {C5, 25}, {G5, 25}, {C6, 25}, {E6, 70}, {0, 0} }; // pluck

    // ─── startup — boot complete (long ones welcome) ────────────────────────
    inline constexpr N startup[]    = { {C5, 90}, {E5, 90}, {G5, 90}, {C6, 220}, {0, 0} };

    // Mario 1-up
    inline constexpr N startup_a[]  = {
        {E5, 125}, {G5, 125}, {E6, 125}, {C6, 125}, {D6, 125}, {G6, 300}, {0, 0}
    };

    // Tetris (Korobeiniki) opening
    inline constexpr N startup_b[]  = {
        {E5, 180}, {B4,  90}, {C5,  90}, {D5, 180}, {C5,  90}, {B4,  90},
        {A4, 180}, {A4,  90}, {C5,  90}, {E5, 180}, {D5,  90}, {C5,  90},
        {B4, 270}, {C5,  90}, {D5, 180}, {E5, 180}, {C5, 180}, {A4, 180}, {A4, 360},
        {0, 0}
    };

    // Final Fantasy victory fanfare
    inline constexpr N startup_c[]  = {
        {C5, 150}, {REST, 30}, {C5, 150}, {REST, 30}, {C5, 150}, {REST, 30}, {C5, 450},
        {Ab4, 450}, {Bb4, 450}, {C5, 150}, {REST, 30}, {Bb4, 150}, {C5, 900},
        {0, 0}
    };

    // Zelda "item get" treasure jingle
    inline constexpr N startup_d[]  = {
        {G5, 160}, {Fs5, 160}, {Ds5, 160}, {A4, 160},
        {Gs4, 160}, {E5, 160}, {Gs5, 160}, {C6, 650},
        {0, 0}
    };

    // ─── done — reflow profile complete ─────────────────────────────────────
    inline constexpr N done[]    = { {E5, 130}, {G5, 130}, {C6, 320}, {0, 0} };
    inline constexpr N done_a[]  = { {G5, 200}, {REST, 80}, {C6, 200}, {REST, 80}, {E6, 500}, {0, 0} };  // bell
    inline constexpr N done_b[]  = { {C5, 180}, {E5, 180}, {G5, 180}, {B5, 180}, {C6, 500}, {0, 0} };    // gentle resolve
    inline constexpr N done_c[]  = { {D6, 180}, {F6, 180}, {A6, 180}, {G6, 180}, {E6, 180}, {C6, 500}, {0, 0} }; // Zelda puzzle solve
    inline constexpr N done_d[]  = { {B5, 90}, {E6, 500}, {0, 0} };                                      // Mario coin

    // ─── error — fault / alarm ──────────────────────────────────────────────
    inline constexpr N error[]    = { {A4, 110}, {REST, 40}, {A4, 110}, {REST, 40}, {E4, 220}, {0, 0} };
    inline constexpr N error_a[]  = { {D5, 200}, {A4, 200}, {D5, 200}, {A4, 400}, {0, 0} };              // NEE-naw
    inline constexpr N error_b[]  = { {C5, 160}, {B4, 160}, {Bb4, 160}, {A4, 500}, {0, 0} };             // sad trombone
    inline constexpr N error_c[]  = { {F4, 90}, {D4, 90}, {F4, 90}, {D4, 90}, {F4, 90}, {D4, 400}, {0, 0} }; // low triple warble
    inline constexpr N error_d[]  = { {Cs5, 250}, {C5, 250}, {Cs5, 500}, {0, 0} };                       // dissonant buzz
}  // namespace melodies

class Buzzer {
 public:
    explicit Buzzer(uint32_t pin) : pin_(pin), ht_(TIM14) {}

    void begin() {
        ht_.setMode(1, TIMER_OUTPUT_COMPARE_PWM1, pin_);
        set_volume(volume_);  // initialise cached pct_
    }

    void play(const Note* melody) {
        melody_     = melody;
        note_idx_   = 0;
        prev_idx_   = UINT8_MAX;  // force apply on first tick
        note_start_ = millis();
    }

    void tone_ms(uint16_t hz, uint16_t ms) {
        single_[0] = {hz, ms};
        single_[1] = {0, 0};
        play(single_);
    }

    // 0–100. Quadratic curve → 0–50% PWM duty (50% = max square wave).
    // Below ~14 is barely audible; 0 is silent.
    void set_volume(uint8_t v) {
        volume_ = v;
        uint32_t p = (uint32_t)v * v / 200;
        pct_ = (v && !p) ? 1 : static_cast<uint8_t>(p);
    }

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

        uint32_t now = millis();
        while (melody_[note_idx_].ms != 0 && now - note_start_ >= melody_[note_idx_].ms) {
            note_start_ += melody_[note_idx_].ms;
            ++note_idx_;
        }

        const Note& n = melody_[note_idx_];
        if (n.ms == 0)             { mute(); return; }
        if (note_idx_ == prev_idx_)  return;
        prev_idx_ = note_idx_;

        if (n.hz) {
            ht_.setOverflow(n.hz, HERTZ_FORMAT);
            ht_.setCaptureCompare(1, pct_, PERCENT_COMPARE_FORMAT);
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
    uint8_t       pct_        = 1;     // cached from set_volume(); synced in begin()
    const Note*   melody_     = nullptr;
    uint8_t       note_idx_   = 0;
    uint8_t       prev_idx_   = UINT8_MAX;
    uint32_t      note_start_ = 0;
    Note          single_[2];  // scratch buffer for tone_ms()
};

}  // namespace driver
