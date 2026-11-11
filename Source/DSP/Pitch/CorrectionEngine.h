#pragma once
#include "TargetSelector.h"
#include "YinDetector.h"

namespace metatune
{
// Turns pitch estimates into a per-sample pitch ratio: target selection -> requested correction
// (semitones, scaled by Amount) -> one-pole smoothing with the Speed time constant.
//
// Smoothing the correction offset (not the output pitch) is what makes slow Speed natural: fast input
// movement such as vibrato passes through, because the offset cannot follow it.
//
// Speed mapping (ms):
//   > 0  : one-pole time constant T = speed; alpha = 1 - exp(-1 / (T * fs)).
//   == 0 : instant (alpha = 1): correction jumps at each detector hop (2 ms).
//   < 0  : "aggressive" (-3..0): instant, and target hysteresis shrinks linearly to 0 at -3, so
//          the output snaps between notes like a hard-tuned effect. Negative values never reach exp().
// Unvoiced / no allowed notes: correction releases to 0 with max(T, 20 ms).
// Sustain: once a note has been held for kSustainOnsetS, the time constant shrinks toward 0 over
// kSustainRampS, by the Sustain fraction: attacks and slides stay natural, long notes lock in.
class CorrectionEngine
{
public:
    struct Params
    {
        double speedMs = 20.0;
        double amount = 1.0;       // 0..1
        double referenceHz = 440.0;
        PitchClassMask mask = kAllNotes;
        double hysteresisCents = 15.0;
        float minConfidence = 0.70f;
        double stabilizerMs = 0.0; // Note Stabilizer: how long a new note must hold before retargeting
        double sustain = 0.0;      // 0..1: held notes tighten from Speed toward instant correction
        double estimateSeconds = 0.002; // time between onEstimate() calls (detector hop)
    };

    void prepare (double sampleRate);
    void reset() noexcept;
    void setParams (const Params&) noexcept; // per block; cheap when unchanged
    void onEstimate (const PitchEstimate&) noexcept;
    float nextRatio() noexcept;

    // Diagnostics (audio-thread values; copy out through a queue for the UI).
    double inputMidi() const noexcept { return inMidi; }    // 0 when unvoiced
    int targetNote() const noexcept { return target; }      // TargetSelector::kNoTarget when none
    double correctionSemitones() const noexcept { return c; }
    double sustainTimeRatio() const noexcept { return sustainRatio; }

private:
    void updateCoefficients() noexcept;

    static constexpr double kSustainOnsetS = 0.12, kSustainRampS = 0.25;

    Params p;
    TargetSelector selector;
    double sr = 48000, alpha = 1, baseAlpha = 1, releaseAlpha = 1, noteAge = 0;
    double desired = 0, c = 0, inMidi = 0;
    double sustainRatio = 1.0;
    bool active = false;
    int target = TargetSelector::kNoTarget;
};
} // namespace metatune
