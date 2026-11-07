#include "CorrectionEngine.h"
#include "PitchMath.h"
#include <algorithm>
#include <cmath>

namespace metatune
{
static double onePoleAlpha (double seconds, double sr) noexcept
{
    return seconds > 0 ? 1.0 - std::exp (-1.0 / (seconds * sr)) : 1.0;
}

void CorrectionEngine::prepare (double sampleRate)
{
    sr = sampleRate;
    updateCoefficients();
    reset();
}

void CorrectionEngine::reset() noexcept
{
    selector.reset();
    desired = c = inMidi = noteAge = 0;
    active = false;
    target = TargetSelector::kNoTarget;
}

void CorrectionEngine::setParams (const Params& np) noexcept
{
    Params q = np;
    q.speedMs = std::isfinite (q.speedMs) ? std::clamp (q.speedMs, -3.0, 1000.0) : 20.0;
    q.amount = std::isfinite (q.amount) ? std::clamp (q.amount, 0.0, 1.0) : 1.0;
    q.referenceHz = std::isfinite (q.referenceHz) ? std::clamp (q.referenceHz, 400.0, 480.0) : 440.0;
    q.sustain = std::isfinite (q.sustain) ? std::clamp (q.sustain, 0.0, 1.0) : 0.0;
    q.stabilizerMs = std::isfinite (q.stabilizerMs) ? std::clamp (q.stabilizerMs, 0.0, 1000.0) : 0.0;
    const bool coeffs = q.speedMs != p.speedMs || q.hysteresisCents != p.hysteresisCents;
    p = q;
    selector.setMask (p.mask);
    selector.setStabilizeCalls (int (std::lround (p.stabilizerMs / 1000.0 / std::max (1e-4, p.estimateSeconds))));
    if (coeffs)
        updateCoefficients();
}

void CorrectionEngine::updateCoefficients() noexcept
{
    alpha = baseAlpha = onePoleAlpha (p.speedMs / 1000.0, sr);
    releaseAlpha = onePoleAlpha (std::max (p.speedMs, 20.0) / 1000.0, sr);
    const double h = p.speedMs < 0 ? p.hysteresisCents * (1.0 + p.speedMs / 3.0) : p.hysteresisCents;
    selector.setHysteresisCents (std::max (0.0, h));
}

void CorrectionEngine::onEstimate (const PitchEstimate& e) noexcept
{
    active = false;
    inMidi = 0;
    if (e.voiced && e.confidence >= p.minConfidence && e.frequency > 0)
    {
        inMidi = freqToMidi (e.frequency, p.referenceHz);
        const int previous = target;
        target = selector.select (inMidi);
        active = target != TargetSelector::kNoTarget;
        noteAge = target == previous ? noteAge + p.estimateSeconds : 0.0;
    }
    else
    {
        target = TargetSelector::kNoTarget;
        noteAge = 0;
    }

    // Sustain: shorten the time constant on held notes (only meaningful for positive Speed).
    alpha = baseAlpha;
    if (p.sustain > 0 && p.speedMs > 0 && noteAge > kSustainOnsetS)
    {
        const double ramp = std::min (1.0, (noteAge - kSustainOnsetS) / kSustainRampS);
        alpha = onePoleAlpha (p.speedMs / 1000.0 * (1.0 - 0.97 * p.sustain * ramp), sr);
    }
    desired = active ? (target - inMidi) * p.amount : 0.0;
}

float CorrectionEngine::nextRatio() noexcept
{
    c += (active ? alpha : releaseAlpha) * (desired - c);
    return float (std::exp2 (c / 12.0));
}
} // namespace metatune
