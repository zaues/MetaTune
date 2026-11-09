// Offline accuracy check for KeyDetector: synthesise chord progressions in known keys (optionally with
// drums) and print expected vs detected. usage: keytest.exe
#include "../Source/MetaKey/KeyDetector.h"
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <array>
#include <cstdio>

static constexpr double SR = 44100.0;
static const char* kNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static void addNote (std::vector<float>& buf, int start, int len, double midi, float amp)
{
    const double f = 440.0 * std::pow (2.0, (midi - 69.0) / 12.0);
    for (int i = 0; i < len && start + i < (int) buf.size(); ++i)
    {
        const double t = double (i) / SR;
        const double env = std::min (1.0, std::min (double (i), double (len - i)) / (0.02 * SR)); // short fades
        double s = 0.0;
        for (int h = 1; h <= 4; ++h) s += std::sin (2.0 * juce::MathConstants<double>::pi * f * double (h) * t) / double (h);
        buf[(size_t) (start + i)] += float (amp * env * s);
    }
}

// A chord = up to 4 MIDI notes. A progression is played as equal-length blocks, twice through.
static std::vector<float> render (const std::vector<std::vector<int>>& chords, bool drums)
{
    const int blockLen = int (0.9 * SR);
    const int total = blockLen * (int) chords.size() * 2 + int (0.2 * SR);
    std::vector<float> buf ((size_t) total, 0.0f);
    int pos = 0;
    for (int rep = 0; rep < 2; ++rep)
        for (const auto& ch : chords)
        {
            for (int n : ch) addNote (buf, pos, blockLen, double (n), 0.25f);
            pos += blockLen;
        }
    if (drums)
    {
        juce::Random rng (1);
        for (int i = 0; i < total; ++i)
        {
            const int beat = int (0.5 * SR);
            const int ph = i % beat;
            if (ph < int (0.05 * SR)) // kick: low thud every half second
                buf[(size_t) i] += 0.5f * std::sin (2.0 * juce::MathConstants<double>::pi * 55.0 * double (ph) / SR) * (1.0f - float (ph) / (0.05f * SR));
            if ((i % beat) == int (0.25 * SR)) {} // (snare burst below)
            const int sn = (i - int (0.25 * SR)) % beat;
            if (sn >= 0 && sn < int (0.04 * SR)) // snare: noise burst on the offbeat
                buf[(size_t) i] += 0.35f * (rng.nextFloat() * 2.0f - 1.0f) * (1.0f - float (sn) / (0.04f * SR));
            buf[(size_t) i] += 0.02f * (rng.nextFloat() * 2.0f - 1.0f); // hiss
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

int main()
{
    // C major I-IV-V-I and A minor i-iv-V-i, transposed to make many keys.
    const std::vector<std::vector<int>> Cmaj { { 60, 64, 67 }, { 65, 69, 72 }, { 67, 71, 74 }, { 60, 64, 67 } };
    const std::vector<std::vector<int>> Amin { { 57, 60, 64 }, { 62, 65, 69 }, { 64, 68, 71 }, { 57, 60, 64 } }; // E major dominant

    struct Case { juce::String name; int root; bool minor; std::vector<std::vector<int>> prog; };
    std::vector<Case> cases;
    for (int t : { 0, 2, 5, 7, 9 }) cases.push_back ({ juce::String (kNames[(0 + t) % 12]) + " maj", (0 + t) % 12, false, transpose (Cmaj, t) });
    for (int t : { 0, -3, 3, 5, 7 }) cases.push_back ({ juce::String (kNames[((9 + t) % 12 + 12) % 12]) + " min", ((9 + t) % 12 + 12) % 12, true, transpose (Amin, t) });

    int ok = 0, okRoot = 0, total = 0;
    for (bool drums : { false, true })
    {
        std::printf ("\n=== %s ===\n", drums ? "with drums" : "clean");
        for (const auto& c : cases)
        {
            auto buf = render (c.prog, drums);
            KeyDetector det;
            det.prepare (SR);
            det.accumulate (buf.data(), (int) buf.size());
            const auto r = det.estimate();
            const bool rootOk = r.root == c.root;
            const bool full = rootOk && r.minor == c.minor;
            ok += full; okRoot += rootOk; ++total;
            std::printf ("  %-8s expected %-2s %-5s  got %-2s %-5s  conf %.2f   %s\n",
                         c.name.toRawUTF8(), kNames[c.root], c.minor ? "min" : "maj",
                         r.root >= 0 ? kNames[r.root] : "--", r.minor ? "min" : "maj", r.confidence,
                         full ? "OK" : (rootOk ? "mode?" : "WRONG"));
        }
    }
    std::printf ("\nTOTAL  full %d/%d   root %d/%d\n", ok, total, okRoot, total);
    return 0;
}
