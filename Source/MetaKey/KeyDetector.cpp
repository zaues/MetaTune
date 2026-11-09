#include "KeyDetector.h"
#include <cmath>

void KeyDetector::prepare (double sampleRate)
{
    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    for (int i = 0; i < kSize; ++i) // Hann window
        window[(size_t) i] = 0.5f - 0.5f * std::cos (2.0f * juce::MathConstants<float>::pi * float (i) / float (kSize - 1));
    acc.reserve ((size_t) kSize * 2);
    reset();
}

void KeyDetector::reset()
{
    acc.clear();
    chromaAcc.fill (0.0);
    frames = 0.0;
}

void KeyDetector::accumulate (const float* mono, int numSamples)
{
    if (numSamples <= 0) return;
    acc.insert (acc.end(), mono, mono + numSamples);
    std::array<float, kSize> framed {};
    while ((int) acc.size() >= kSize)
    {
        for (int i = 0; i < kSize; ++i)
            framed[(size_t) i] = acc[(size_t) i] * window[(size_t) i];
        processFrame (framed.data());
        acc.erase (acc.begin(), acc.begin() + kHop);
    }
    // Don't let the buffer grow without bound if frames aren't consuming it.
    if (acc.size() > (size_t) kSize * 4)
        acc.erase (acc.begin(), acc.end() - kSize);
}

void KeyDetector::processFrame (const float* windowed)
{
    std::fill (fftData.begin(), fftData.end(), 0.0f);
    std::copy (windowed, windowed + kSize, fftData.begin());
    fft.performFrequencyOnlyForwardTransform (fftData.data()); // magnitudes in bins [0 .. kSize/2]

    const int nBins = kSize / 2;
    float maxMag = 0.0f;
    for (int b = 1; b < nBins; ++b) maxMag = juce::jmax (maxMag, fftData[(size_t) b]);
    if (maxMag < 1.0e-6f) return; // silence

    // Only spectral PEAKS (local maxima above a floor) contribute: tonal content has sharp peaks, while
    // drums / broadband noise do not, so this keeps percussion from flattening the chromagram.
    const double binHz = sr / double (kSize);
    const float floorMag = maxMag * 0.04f;
    double frameEnergy = 0.0;
    std::array<double, kBins> frameChroma {};
    for (int b = 2; b < nBins - 1; ++b)
    {
        const float m = fftData[(size_t) b];
        if (m < floorMag) continue;
        const float lo = fftData[(size_t) (b - 1)], hi = fftData[(size_t) (b + 1)];
        if (! (m > lo && m >= hi)) continue; // local peak

        // Parabolic interpolation for a more accurate peak frequency.
        const float denom = lo - 2.0f * m + hi;
        const float delta = std::abs (denom) > 1.0e-9f ? 0.5f * (lo - hi) / denom : 0.0f;
        const double f = binHz * (double (b) + (double) juce::jlimit (-0.5f, 0.5f, delta));
        if (f < 60.0 || f > 2000.0) continue;

        // Emphasise the mid band where harmony lives; taper sub-bass (kick) and highs (cymbals).
        double w = 1.0;
        if (f < 130.0)       w = juce::jmax (0.25, f / 130.0);
        else if (f > 1000.0) w = juce::jmax (0.2, 1.0 - (f - 1000.0) / 2500.0);

        const double midi = 69.0 + 12.0 * std::log2 (f / 440.0);
        double pcf = std::fmod (midi, 12.0);
        if (pcf < 0.0) pcf += 12.0;
        const int bin = ((int) std::llround (pcf * (kBins / 12.0))) % kBins; // 3 sub-bins per semitone
        const double mag = std::sqrt ((double) m) * w; // compress so one loud peak can't dominate
        frameChroma[(size_t) bin] += mag;
        frameEnergy += mag;
    }
    if (frameEnergy < 1.0e-9) return; // no tonal peaks this frame
    for (int i = 0; i < kBins; ++i)
        chromaAcc[(size_t) i] += frameChroma[(size_t) i] / frameEnergy; // each frame weighted equally
    frames += 1.0;
}

// Estimate a single global tuning offset (so tracks not at A=440 still map to the right pitch classes),
// then fold the 36-bin chroma down to 12 pitch classes with that offset removed.
std::array<double, 12> KeyDetector::foldTuned() const
{
    // Tuning = circular mean of the energy over the 3 sub-bins that make up each semitone.
    double re = 0.0, im = 0.0;
    for (int b = 0; b < kBins; ++b)
    {
        const double ang = 2.0 * juce::MathConstants<double>::pi * double (b % 3) / 3.0;
        re += chromaAcc[(size_t) b] * std::cos (ang);
        im += chromaAcc[(size_t) b] * std::sin (ang);
    }
    const double shift = (re != 0.0 || im != 0.0)
        ? std::atan2 (im, re) / (2.0 * juce::MathConstants<double>::pi) * 3.0 // sub-bins, -1.5..1.5
        : 0.0;

    std::array<double, 12> c {};
    for (int b = 0; b < kBins; ++b)
    {
        const double adj = double (b) - shift;
        const int pc = (((int) std::llround (adj / 3.0)) % 12 + 12) % 12;
        c[(size_t) pc] += chromaAcc[(size_t) b];
    }
    return c;
}

std::array<float, 12> KeyDetector::chroma() const
{
    auto c = foldTuned();
    std::array<float, 12> out {};
    double mx = 0.0;
    for (double v : c) mx = std::max (mx, v);
    if (mx <= 0.0) return out;
    for (int i = 0; i < 12; ++i) out[(size_t) i] = float (c[(size_t) i] / mx);
    return out;
}

// Albrecht & Shanahan (2013) key profiles, derived from a large corpus: more reliable major/minor
// discrimination than the original Krumhansl-Schmuckler weights.
static const double kMaj[12] = { 0.2380, 0.0060, 0.1110, 0.0060, 0.1370, 0.0940, 0.0160, 0.2140, 0.0090, 0.0800, 0.0080, 0.0810 };
static const double kMin[12] = { 0.2200, 0.0060, 0.1040, 0.1230, 0.0190, 0.1030, 0.0120, 0.2140, 0.0620, 0.0220, 0.0610, 0.0520 };

static double correlate (const std::array<double, 12>& c, const double* prof, int rot)
{
    double mc = 0.0, mp = 0.0;
    for (int i = 0; i < 12; ++i) { mc += c[(size_t) ((i + rot) % 12)]; mp += prof[i]; }
    mc /= 12.0; mp /= 12.0;
    double num = 0.0, dc = 0.0, dp = 0.0;
    for (int i = 0; i < 12; ++i)
    {
        const double a = c[(size_t) ((i + rot) % 12)] - mc, b = prof[i] - mp;
        num += a * b; dc += a * a; dp += b * b;
    }
    return (dc > 0.0 && dp > 0.0) ? num / std::sqrt (dc * dp) : -1.0;
}

KeyDetector::Result KeyDetector::estimate() const
{
    if (frames < 3.0) return {};

    auto c = foldTuned();
    double sum = 0.0; for (double v : c) sum += v;
    if (sum <= 0.0) return {};
    for (auto& v : c) v /= sum;

    Result best;
    double bestScore = -2.0, second = -2.0;
    auto consider = [&] (double score, int rot, bool minor)
    {
        if (score > bestScore) { second = bestScore; bestScore = score; best.root = rot; best.minor = minor; }
        else if (score > second) { second = score; }
    };
    for (int rot = 0; rot < 12; ++rot)
    {
        consider (correlate (c, kMaj, rot), rot, false);
        consider (correlate (c, kMin, rot), rot, true);
    }
    if (best.root < 0) return {};
    best.confidence = (float) juce::jlimit (0.0, 1.0, (bestScore - second) * 4.0);
    return best;
}
