// Offline accuracy check for KeyDetector: synthesise chord progressions in every key (clean, with drums,
// detuned, and melody-only) and print expected vs detected. usage: keytest.exe [-v]
#include "../Source/MetaKey/KeyDetector.h"
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <array>
#include <cstdio>
#include <cstring>

static constexpr double SR = 44100.0;
static const char* kNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static void addNote (std::vector<float>& buf, int start, int len, double midi, float amp, double detuneCents)
{
    const double f = 440.0 * std::pow (2.0, (midi - 69.0) / 12.0) * std::pow (2.0, detuneCents / 1200.0);
    for (int i = 0; i < len && start + i < (int) buf.size(); ++i)
    {
        const double t = double (i) / SR;
        const double env = std::min (1.0, std::min (double (i), double (len - i)) / (0.02 * SR));
        double s = 0.0;
        for (int h = 1; h <= 4; ++h) s += std::sin (2.0 * juce::MathConstants<double>::pi * f * double (h) * t) / double (h);
        buf[(size_t) (start + i)] += float (amp * env * s);
    }
}

static std::vector<float> render (const std::vector<std::vector<int>>& chords, bool drums, double detune, bool melody)
{
    const int blockLen = int (0.9 * SR);
    const int total = blockLen * (int) chords.size() * 2 + int (0.2 * SR);
    std::vector<float> buf ((size_t) total, 0.0f);
    int pos = 0;
    for (int rep = 0; rep < 2; ++rep)
        for (const auto& ch : chords)
        {
            if (melody) // arpeggiate: one note at a time
            {
                const int step = blockLen / (int) ch.size();
                for (int k = 0; k < (int) ch.size(); ++k) addNote (buf, pos + k * step, step, double (ch[(size_t) k]) + 12.0, 0.3f, detune);
            }
            else
                for (int n : ch) addNote (buf, pos, blockLen, double (n), 0.25f, detune);
            pos += blockLen;
        }
    if (drums)
    {
        juce::Random rng (1);
        const int beat = int (0.5 * SR);
        for (int i = 0; i < total; ++i)
        {
            const int ph = i % beat;
            if (ph < int (0.05 * SR))
                buf[(size_t) i] += 0.5f * std::sin (2.0 * juce::MathConstants<double>::pi * 55.0 * double (ph) / SR) * (1.0f - float (ph) / (0.05f * SR));
            const int sn = (i - int (0.25 * SR)) % beat;
            if (sn >= 0 && sn < int (0.04 * SR))
                buf[(size_t) i] += 0.35f * (rng.nextFloat() * 2.0f - 1.0f) * (1.0f - float (sn) / (0.04f * SR));
            buf[(size_t) i] += 0.02f * (rng.nextFloat() * 2.0f - 1.0f);
        }
    }
    return buf;
}

static std::vector<std::vector<int>> transpose (const std::vector<std::vector<int>>& prog, int semis)
{
    auto out = prog;
    for (auto& c : out) for (auto& n : c) n += semis;
    return out;
}

int main (int argc, char** argv)
{
    const bool verbose = argc > 1 && std::strcmp (argv[1], "-v") == 0;
    const std::vector<std::vector<int>> Cmaj { { 60, 64, 67 }, { 65, 69, 72 }, { 67, 71, 74 }, { 60, 64, 67 } };
    const std::vector<std::vector<int>> Amin { { 57, 60, 64 }, { 62, 65, 69 }, { 64, 68, 71 }, { 57, 60, 64 } };

    struct Case { juce::String name; int root; bool minor; std::vector<std::vector<int>> prog; };
    std::vector<Case> cases;
    for (int t = 0; t < 12; ++t) cases.push_back ({ juce::String (kNames[t % 12]) + " maj", t % 12, false, transpose (Cmaj, t) });
    for (int t = 0; t < 12; ++t) cases.push_back ({ juce::String (kNames[(9 + t) % 12]) + " min", (9 + t) % 12, true, transpose (Amin, t) });

    struct Variant { const char* name; bool drums; double detune; bool melody; };
    const Variant variants[] = {
        { "clean",        false, 0.0,   false },
        { "drums",        true,  0.0,   false },
        { "detune -30c",  false, -30.0, false },
        { "432Hz +drums", true,  -31.8, false },
        { "melody only",  false, 0.0,   true  },
    };

    int grandOk = 0, grandRoot = 0, grandTot = 0;
    for (const auto& v : variants)
    {
        int ok = 0, okRoot = 0, total = 0;
        for (const auto& c : cases)
        {
            auto buf = render (c.prog, v.drums, v.detune, v.melody);
            KeyDetector det; det.prepare (SR); det.accumulate (buf.data(), (int) buf.size());
            const auto r = det.estimate();
            const bool rootOk = r.root == c.root, full = rootOk && r.minor == c.minor;
            ok += full; okRoot += rootOk; ++total;
            if (verbose || ! full)
                std::printf ("  [%-12s] %-7s exp %-2s %s  got %-2s %s  conf %.2f  %s\n",
                             v.name, c.name.toRawUTF8(), kNames[c.root], c.minor ? "min" : "maj",
                             r.root >= 0 ? kNames[r.root] : "--", r.minor ? "min" : "maj", r.confidence,
                             full ? "OK" : (rootOk ? "MODE?" : "WRONG"));
        }
        std::printf ("%-14s  full %2d/%2d   root %2d/%2d\n", v.name, ok, total, okRoot, total);
        grandOk += ok; grandRoot += okRoot; grandTot += total;
    }
    std::printf ("\nTOTAL  full %d/%d (%.0f%%)   root %d/%d (%.0f%%)\n",
                 grandOk, grandTot, 100.0 * grandOk / grandTot, grandRoot, grandTot, 100.0 * grandRoot / grandTot);
    return 0;
}
