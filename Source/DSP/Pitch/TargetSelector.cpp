#include "TargetSelector.h"
#include "PitchMath.h"
#include <cmath>

namespace metatune
{
int TargetSelector::nearestAllowed (double midi, PitchClassMask m) noexcept
{
    if ((m & kAllNotes) == 0 || ! std::isfinite (midi))
        return kNoTarget;

    // Allowed notes repeat every octave, so +-11 semitones around the rounded pitch contains the nearest.
    const int centre = int (std::lround (midi));
    int best = kNoTarget;
    double bestDist = 1e9;
    for (int n = centre - 11; n <= centre + 11; ++n)
    {
        if (! isAllowed (m, pitchClassOf (n)))
            continue;
        const double d = std::abs (midi - n);
        if (d < bestDist - 1e-9) { bestDist = d; best = n; } // strict: lower note wins ties
    }
    return best;
}

int TargetSelector::select (double midi) noexcept
{
    const int candidate = nearestAllowed (midi, mask);
    if (candidate == kNoTarget)
    {
        pending = kNoTarget;
        pendingCount = 0;
        return current = kNoTarget;
    }

    const bool currentValid = current != kNoTarget && isAllowed (mask, pitchClassOf (current));
    if (currentValid && current != candidate && std::abs (midi - current) <= std::abs (midi - candidate) + 2.0 * hysteresis)
        return current;

    // First note, a removed note or no stabilizer: switch at once.
    if (! currentValid || candidate == current || stabilize == 0)
    {
        pending = kNoTarget;
        pendingCount = 0;
        return current = candidate;
    }

    // Stabilizer: the new note must persist before it becomes the target.
    if (candidate == pending)
        ++pendingCount;
    else
    {
        pending = candidate;
        pendingCount = 1;
    }
    if (pendingCount >= stabilize)
    {
        pending = kNoTarget;
        pendingCount = 0;
        current = candidate;
    }
    return current;
}
} // namespace metatune
