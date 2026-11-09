#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <vector>

// Estimates the musical key (tonic pitch class + major/minor) of audio by building a chromagram
// (12 pitch-class energies) and matching it against Krumhansl-Schmuckler key profiles. Not thread safe;
// the processor owns one instance and feeds it from a single analysis thread.
class KeyDetector
{
public:
    struct Result
    {
        int root = -1;            // tonic pitch class 0..11 (C..B), -1 = none yet
        bool minor = false;
        float confidence = 0.0f;  // 0..1, how clearly one key wins
        bool valid() const noexcept { return root >= 0; }
    };

    void prepare (double sampleRate);
    void reset();                                   // clear the accumulated chroma
    void accumulate (const float* mono, int numSamples); // feed audio; windowed FFT frames -> chroma
    Result estimate() const;                        // correlate the current chroma against the profiles
    std::array<float, 12> chroma() const;           // normalised 0..1, for display
    double framesAnalysed() const noexcept { return frames; }

private:
    void processFrame (const float* windowed);
    std::array<double, 12> foldTuned() const; // estimate the global tuning, fold the 36-bin chroma to 12

    static constexpr int kOrder = 12;
    static constexpr int kSize  = 1 << kOrder; // 4096
    static constexpr int kHop   = kSize / 2;
    static constexpr int kBins  = 36;          // 3 bins per semitone, so tuning up to +/-1/6 tone is recoverable

    double sr = 44100.0;
    juce::dsp::FFT fft { kOrder };
    std::array<float, kSize> window {};
    std::vector<float> acc;                    // input samples awaiting a full frame
    std::array<float, kSize * 2> fftData {};
    std::array<double, kBins> chromaAcc {};    // high-resolution chroma, folded to 12 with a tuning estimate
    double frames = 0.0;
};
