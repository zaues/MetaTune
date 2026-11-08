#include "TunerChain.h"
#include <algorithm>
#include <cmath>

namespace metatune
{
static constexpr double kTwoPi = 6.283185307179586;

void TunerChain::prepare (double sampleRate, double lowestMinFreq)
{
    sr = sampleRate;
    minFreq = lowestMinFreq;
    YinDetector::Config cfg;
    cfg.minFreq = lowestMinFreq;
    yin.prepare (sr, cfg);
    engine.prepare (sr);
    shifter.prepare (sr, kMaxChannels, lowestMinFreq);
    // Sized for the largest latency; the active delay follows the shifter centre (see setSettings).
    dryDelay.prepare (kMaxChannels, shifter.latencySamples(), int (0.005 * sr));
    wetStep = float (1.0 / (0.010 * sr));
    smoothStep = float (1.0 / (0.030 * sr));
    toneStep = float (1.0 / (0.010 * sr));
    for (int v = 0; v < 2; ++v)
    {
        voiceShift[v].prepare (sr, 1, 100.0);
        const int del = int (kDoubleDelayMs[v] * 0.001 * sr);
        voiceDelay[v].prepare (1, del, 0);
        voiceDelay[v].setDelay (del);
        const float r = float (std::exp2 (kDoubleCents[v] / 1200.0));
        std::fill (voiceRatio[v], voiceRatio[v] + kSubBlock, r);
    }
    reset();
}

void TunerChain::reset() noexcept
{
    yin.reset();
    engine.reset();
    shifter.reset();
    dryDelay.reset();
    for (int v = 0; v < 2; ++v) { voiceShift[v].reset(); voiceDelay[v].reset(); }
    wet = wetTarget;
    dMix = dMixTarget;
    dWidth = dWidthTarget;
    toneEnv = 0;
    tonePhase = 0;
}

void TunerChain::setSettings (const Settings& s) noexcept
{
    engine.setParams (s.correction);
    if (s.minFreq != minFreq)
    {
        minFreq = s.minFreq;
        yin.setMinFreq (minFreq);
        shifter.setMinFreq (minFreq);
        dryDelay.setDelay (shifter.latencySamples());
    }
    wetTarget = s.bypass ? 0.0f : 1.0f;
    dMixTarget = float (std::clamp (s.doublerMix, 0.0, 1.0));
    dWidthTarget = float (std::clamp (s.doublerWidth, 0.0, 1.0));
    refHz = s.correction.referenceHz;
    if (s.toneNote >= 0)
    {
        toneNote = s.toneNote;
        toneChord = s.toneChord;
        const auto hz = [this] (int semis) { return refHz * std::exp2 ((toneNote + semis - 69) / 12.0); };
        toneHz = hz (0);
        toneHz3rd = hz (toneChord == 2 ? 3 : 4); // minor or major third
        toneHz5th = hz (7);                       // perfect fifth
    }
    else toneNote = -1;
    toneGain = float (std::clamp (s.toneGain, 0.0, 1.0));
}

void TunerChain::process (float* const* ch, int numCh, int total) noexcept
{
    numCh = std::clamp (numCh, 0, kMaxChannels);
    if (numCh == 0 || total <= 0)
        return;
    const float invCh = 1.0f / float (numCh);

    for (int start = 0; start < total; start += kSubBlock)
    {
        const int n = std::min (kSubBlock, total - start);
        float* io[kMaxChannels] {};
        float* dryPtr[kMaxChannels] { dry[0], dry[1] };
        for (int c = 0; c < numCh; ++c)
        {
            io[c] = ch[c] + start;
            std::copy (io[c], io[c] + n, dry[c]);
        }

        // Stereo-linked detection: one pitch decision for both channels.
        for (int i = 0; i < n; ++i)
        {
            float s = 0;
            for (int c = 0; c < numCh; ++c) s += io[c][i];
            mono[i] = s * invCh;
        }
        yin.process (mono, n, [this] (int, const PitchEstimate& e) {
            engine.onEstimate (e);
            shifter.setPeriod (e.voiced ? sr / e.frequency : 0.0);
            voicePeriod = e.voiced ? sr / e.frequency : 0.0;
            if (telemetry != nullptr)
            {
                Telemetry t;
                t.inputMidi = float (engine.inputMidi());
                // The applied correction (smoothed) is what the listener hears, not the requested one.
                t.outputMidi = t.inputMidi > 0 ? float (engine.inputMidi() + engine.correctionSemitones()) : 0.0f;
                t.confidence = e.confidence;
                t.rms = e.rms;
                t.target = engine.targetNote();
                telemetry->push (t); // drops when the UI is not draining (editor closed)
            }
        });
        for (int i = 0; i < n; ++i)
            ratio[i] = engine.nextRatio();

        shifter.process (io, numCh, n, ratio);
        dryDelay.process (dryPtr, numCh, n);

        // Doubler voices from the tuned mono signal. Skipped entirely while off.
        const bool doubling = dMix > 0.0f || dMixTarget > 0.0f;
        if (doubling)
        {
            for (int v = 0; v < 2; ++v)
            {
                for (int i = 0; i < n; ++i)
                {
                    float sum = 0;
                    for (int c = 0; c < numCh; ++c) sum += io[c][i];
                    voice[v][i] = sum * invCh;
                }
                float* vp[1] { voice[v] };
                voiceDelay[v].process (vp, 1, n);
                voiceShift[v].setPeriod (voicePeriod);
                voiceShift[v].process (vp, 1, n, voiceRatio[v]);
            }
        }

        for (int i = 0; i < n; ++i)
        {
            if (wet != wetTarget)
                wet = wet < wetTarget ? std::min (wetTarget, wet + wetStep) : std::max (wetTarget, wet - wetStep);
            if (doubling)
            {
                if (dMix != dMixTarget) dMix = dMix < dMixTarget ? std::min (dMixTarget, dMix + smoothStep) : std::max (dMixTarget, dMix - smoothStep);
                if (dWidth != dWidthTarget) dWidth = dWidth < dWidthTarget ? std::min (dWidthTarget, dWidth + smoothStep) : std::max (dWidthTarget, dWidth - smoothStep);
                const float a = voice[0][i] * dMix * 0.7f, b = voice[1][i] * dMix * 0.7f;
                if (numCh == 2)
                {
                    // Equal-ish power pan: width 0 = both centred, 1 = voice A left / voice B right.
                    const float near = 0.5f + 0.5f * dWidth, far = 0.5f - 0.5f * dWidth;
                    io[0][i] += a * near + b * far;
                    io[1][i] += a * far + b * near;
                }
                else
                    io[0][i] += 0.5f * (a + b);
            }
            for (int c = 0; c < numCh; ++c)
                io[c][i] = dry[c][i] + wet * (io[c][i] - dry[c][i]);
        }

        // Reference tone, after bypass so it can be heard either way.
        if (toneNote >= 0 || toneEnv > 0.0f)
        {
            const bool chord = toneChord > 0;
            const double inc = kTwoPi * toneHz / sr, inc3 = kTwoPi * toneHz3rd / sr, inc5 = kTwoPi * toneHz5th / sr;
            const float level = toneGain * 0.5f * (chord ? 0.6f : 1.0f); // trim a triad so it doesn't clip
            for (int i = 0; i < n; ++i)
            {
                const float target = toneNote >= 0 ? 1.0f : 0.0f;
                toneEnv = toneEnv < target ? std::min (target, toneEnv + toneStep) : std::max (target, toneEnv - toneStep);
                float s = float (std::sin (tonePhase));
                if (chord) s += float (std::sin (tonePhase3rd)) + float (std::sin (tonePhase5th));
                const float sample = s * toneEnv * level;
                tonePhase += inc;   if (tonePhase > kTwoPi) tonePhase -= kTwoPi;
                tonePhase3rd += inc3; if (tonePhase3rd > kTwoPi) tonePhase3rd -= kTwoPi;
                tonePhase5th += inc5; if (tonePhase5th > kTwoPi) tonePhase5th -= kTwoPi;
                for (int c = 0; c < numCh; ++c) io[c][i] += sample;
            }
        }
    }
}
} // namespace metatune
