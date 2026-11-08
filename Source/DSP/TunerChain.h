#pragma once
#include "Common/FixedDelay.h"
#include "Common/SpscQueue.h"
#include "Pitch/CorrectionEngine.h"
#include "Pitch/PitchShifter.h"
#include "Pitch/YinDetector.h"

namespace metatune
{
// The complete real-time tuner signal path, independent of any plugin framework so the plugin and
// the offline render/test tools run identical code:
//   mono sum -> YinDetector -> CorrectionEngine -> PitchShifter (stereo-linked)
//   dry path delayed by the same latency; bypass = 10 ms linear crossfade between the two.
//
// One display sample per detector hop, sent to the UI through a lock-free queue.
struct Telemetry
{
    float inputMidi = 0;  // fractional MIDI of the detected pitch; 0 = unvoiced
    float outputMidi = 0; // inputMidi + applied correction
    float confidence = 0;
    float rms = 0;
    int target = -1;      // target MIDI note, -1 = none
};
using TelemetryQueue = SpscQueue<Telemetry, 1024>; // ~2 s at the 2 ms hop

// Threading: prepare() allocates; everything else is RT-safe and audio-thread owned.
class TunerChain
{
public:
    static constexpr int kSubBlock = 32; // control-rate granularity (0.67 ms @48 kHz)
    static constexpr int kMaxChannels = 2;

    struct Settings
    {
        CorrectionEngine::Params correction;
        double minFreq = 70.0; // range floor: sets detector frame and shifter latency
        bool bypass = false;
        double doublerMix = 0.0;   // 0..1: level of the two doubled voices
        double doublerWidth = 0.0; // 0..1: centre -> hard left/right
        int toneNote = -1;         // MIDI note of the reference tone, -1 = off
        int toneChord = 0;         // 0 = single note, 1 = major triad, 2 = minor triad
        double toneGain = 0.0;     // linear
    };

    void prepare (double sampleRate, double lowestMinFreq);
    void reset() noexcept;
    void setSettings (const Settings&) noexcept; // call once per block
    void process (float* const* channels, int numChannels, int numSamples) noexcept;

    int latencySamples() const noexcept { return shifter.latencySamples(); }
    static int latencyFor (double sampleRate, double minFreq) noexcept { return PitchShifter::latencyFor (sampleRate, minFreq); }

    // Optional; the queue must outlive the chain. Set before processing starts.
    void setTelemetryQueue (TelemetryQueue* q) noexcept { telemetry = q; }

    const CorrectionEngine& correction() const noexcept { return engine; }
    const YinDetector& detector() const noexcept { return yin; }

private:
    YinDetector yin;
    CorrectionEngine engine;
    PitchShifter shifter;
    FixedDelay dryDelay;
    // Doubler: two detuned, slightly late copies of the tuned voice, panned apart.
    static constexpr double kDoubleCents[2] { -9.0, 8.0 }, kDoubleDelayMs[2] { 13.0, 21.0 };
    PitchShifter voiceShift[2];
    FixedDelay voiceDelay[2];
    float voice[2][kSubBlock] {}, voiceRatio[2][kSubBlock] {};
    float dMix = 0, dMixTarget = 0, dWidth = 0, dWidthTarget = 0;
    double voicePeriod = 0;
    // Reference tone: sine at toneNote (plus a triad's 3rd and 5th when toneChord > 0), short A/R.
    double tonePhase = 0, toneHz = 0, refHz = 440;
    double tonePhase3rd = 0, tonePhase5th = 0, toneHz3rd = 0, toneHz5th = 0;
    float toneEnv = 0, toneGain = 0, toneStep = 0.001f;
    int toneNote = -1, toneChord = 0;
    TelemetryQueue* telemetry = nullptr;
    double sr = 48000, minFreq = 70;
    float wet = 1, wetTarget = 1, wetStep = 0.01f, smoothStep = 0.001f;
    float mono[kSubBlock] {}, ratio[kSubBlock] {}, dry[kMaxChannels][kSubBlock] {};
};
} // namespace metatune
