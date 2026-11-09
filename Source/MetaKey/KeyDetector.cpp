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
    std::array<double, 12> frameChroma {};
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
        const int pc = ((int) std::llround (pcf)) % 12;
        const double mag = std::sqrt ((double) m) * w; // compress so one loud peak can't dominate
        frameChroma[(size_t) pc] += mag;
        frameEnergy += mag;
    }
    if (frameEnergy < 1.0e-9) return; // no tonal peaks this frame
    for (int i = 0; i < 12; ++i)
        chromaAcc[(size_t) i] += frameChroma[(size_t) i] / frameEnergy; // each frame weighted equally
    frames += 1.0;
}

std::array<float, 12> KeyDetector::chroma() const
{
    std::array<float, 12> c {};
    double mx = 0.0;
    for (double v : chromaAcc) mx = std::max (mx, v);
    if (mx <= 0.0) return c;
    for (int i = 0; i < 12; ++i) c[(size_t) i] = float (chromaAcc[(size_t) i] / mx);
    return c;
}

KeyDetector::Result KeyDetector::estimate() const
{
    if (frames < 3.0) return {};

    std::array<double, 12> c {};
    double sum = 0.0;
    for (int i = 0; i < 12; ++i) { c[(size_t) i] = chromaAcc[(size_t) i]; sum += c[(size_t) i]; }
    if (sum <= 0.0) return {};
    for (auto& v : c) v /= sum;

    static const double maj[12] = { 6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88 };
    static const double min_[12] = { 6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17 };

    auto corr = [&c] (const double* prof, int rot)
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
    };

    Result best;
    double bestScore = -2.0, second = -2.0;
    auto consider = [&] (double score, int rot, bool minor)
    {
        if (score > bestScore) { second = bestScore; bestScore = score; best.root = rot; best.minor = minor; }
        else if (score > second) { second = score; }
    };
    for (int rot = 0; rot < 12; ++rot)
    {
        consider (corr (maj, rot), rot, false);
        consider (corr (min_, rot), rot, true);
    }
    best.confidence = (float) juce::jlimit (0.0, 1.0, (bestScore - second) * 4.0);
    return best;
}
