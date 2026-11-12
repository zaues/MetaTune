#include "Orb.h"
#include <cstring>


// Angle on the circle for a pitch class (C at 12 o'clock, clockwise).
static float pcAngle (float midi) { return juce::MathConstants<float>::twoPi * std::fmod (midi, 12.0f) / 12.0f; }

// Particle colour by angle around the core (0 = top, clockwise): violet -> pink -> blush -> cyan -> blue.
static juce::Colour coreColourAt (float a)
{
    static const std::pair<float, juce::Colour> stops[] {
        { 0.0f, juce::Colour (0xffffb3de) }, { 0.15f, juce::Colour (0xffffe3e0) },
        { 0.32f, juce::Colour (0xff67d2f0) }, { 0.48f, juce::Colour (0xff94bfff) },
        { 0.64f, juce::Colour (0xffb086ff) }, { 0.81f, juce::Colour (0xfff574e7) },
        { 1.0f, juce::Colour (0xffffb3de) } };
    const float u = a / juce::MathConstants<float>::twoPi;
    for (size_t i = 1; i < std::size (stops); ++i)
        if (u <= stops[i].first)
            return theme::orbTint (stops[i - 1].second.interpolatedWith (stops[i].second, (u - stops[i - 1].first) / (stops[i].first - stops[i - 1].first)));
    return theme::orbTint (stops[0].second);
}

Orb::Orb (juce::RangedAudioParameter& x, juce::RangedAudioParameter& y, juce::String xn, juce::String yn, juce::UndoManager* um)
    : xParam (x), yParam (y), xName (std::move (xn)), yName (std::move (yn)), undoManager (um),
      xAttach (x, [this] (float v) { xNorm = xParam.convertTo0to1 (v); repaint(); }, um),
      // Only X opens an undo transaction: one XY drag = one undo step (Y joins it).
      yAttach (y, [this] (float v) { yNorm = yParam.convertTo0to1 (v); repaint(); }, nullptr)
{
    xAttach.sendInitialUpdate();
    yAttach.sendInitialUpdate();
    setRepaintsOnMouseActivity (false);
}

void Orb::setDisplay (const DisplayState& s, float retuneSpeed, float bypassBlend, float seconds)
{
    const float frames = juce::jlimit (0.0f, 0.1f, seconds) * 60.0f;
    const auto rate = [frames] (float step) { return 1.0f - std::pow (1.0f - step, frames); };
    negativeSpeed = retuneSpeed < 0.0f;
    bypassAmount = juce::jlimit (0.0f, 1.0f, bypassBlend);
    splitAmount += ((negativeSpeed ? 1.0f : 0.0f) - splitAmount) * rate (0.26f);
    if (splitAmount < 0.004f) splitAmount = 0.0f;
    if (splitAmount > 0.996f) splitAmount = 1.0f;

    // Bypass changes the lighting, while pitch and the envelope keep moving.
    const bool voiced = s.voiced;
    voicedSeconds = juce::jlimit (0.0f, 2.5f, voicedSeconds + (voiced ? 1.0f : -4.0f) * frames / 60.0f);
    const float level = voiced ? juce::jlimit (0.0f, 1.0f, s.level) : 0.0f;
    const float target = voiced ? s.presence * (0.25f + 0.75f * level) : 0.0f;
    act += (target - act) * rate (target > act ? 0.20f : 0.10f);

    haloLight += (act - haloLight) * rate (act > haloLight ? 0.18f : 0.031f);
    const float attack = juce::jlimit (0.0f, 1.0f, (level - voiceLevel) * 3.4f);
    voiceLevel += (level - voiceLevel) * rate (voiced ? 0.09f : 0.18f);
    syllable += (attack - syllable) * rate (attack > syllable ? 0.55f : 0.12f);
    // A small negative Speed barely lights the bolt. Voice attacks lift the steady glow.
    const float depth = juce::jlimit (0.0f, 1.0f, -retuneSpeed / 3.0f);
    const float visibility = depth * (0.16f + 0.84f * depth);
    const float vocalFlash = juce::jmin (1.0f, 0.42f * level + 0.66f * syllable);
    const float boltTarget = visibility * (0.74f + 0.26f * vocalFlash);
    if (negativeSpeed && bypassAmount < 0.999f)
        boltAmt += (boltTarget - boltAmt) * rate (boltTarget > boltAmt ? 0.55f : 0.22f);
    else
        boltAmt = 0.0f;
    if (boltAmt < 0.01f) boltAmt = 0.0f;

    const double elapsed = frames / 60.0;
    t += elapsed;
    colourTime += elapsed * (0.72 + 0.28 * act);
    const float secondsElapsed = float (elapsed);
    const float turn = juce::MathConstants<float>::twoPi;
    // Native video textures move counterclockwise about 3.5–3.6 degrees per 0.30 seconds.
    haloPhase = std::remainder (haloPhase - secondsElapsed * juce::degreesToRadians (12.0f), turn);
    fieldPhase = std::remainder (fieldPhase + secondsElapsed * (0.58f + 0.23f * haloLight), turn);
    contourPhase = std::remainder (contourPhase + secondsElapsed * std::lerp (0.76f, 0.31f, splitAmount)
                                                      * (0.33f + 0.67f * act), turn);
    colourPhase = std::remainder (colourPhase + secondsElapsed * std::lerp (0.86f, 0.60f, splitAmount)
                                                    * (0.29f + 0.71f * act), turn);
    spin += frames * (0.006f + 0.018f * act + 0.010f * syllable);
    if (spin > turn) spin -= turn;
    if (voiced)
    {
        // Follow pitch changes by the shortest route while the material keeps flowing.
        const float delta = std::remainder (pcAngle (s.outputMidi) - coreAngle, turn);
        coreAngle = std::remainder (coreAngle + delta * rate (0.075f), turn);
    }
    repaint();
}

Orb::GuideGeom Orb::guideGeom() const
{
    const auto bounds = getLocalBounds().toFloat();
    const float u = unit();
    return { 21.0f * u, bounds.getBottom() - 31.0f * u, 70.0f * u,
             bounds.getRight() - 113.0f * u, 30.0f * u };
}

juce::Rectangle<float> Orb::trackArea() const
{
    const auto guide = guideGeom();
    const float inset = 12.0f * unit();
    const float left = guide.cornerX + guide.radius + 3.1f * inset;
    const float top = guide.topY + inset;
    const float right = guide.rightX - inset;
    const float bottom = guide.baseY - guide.radius - inset;
    return { left, top, right - left, bottom - top };
}

juce::Point<float> Orb::handlePos() const
{
    const auto t = trackArea();
    return { t.getX() + xNorm * t.getWidth(), t.getBottom() - yNorm * t.getHeight() };
}

Orb::DragAxis Orb::projectionAt (juce::Point<float> p) const
{
    const auto gg = guideGeom();
    const auto hp = handlePos();
    const float radius = 14.0f * unit();
    if (p.getDistanceFrom ({ gg.cornerX, hp.y }) < radius)
        return DragAxis::wet;
    if (p.getDistanceFrom ({ hp.x, gg.baseY }) < radius)
        return DragAxis::width;
    return DragAxis::none;
}

bool Orb::hitTest (int x, int y)
{
    // The sphere, the handle and the two Doubler guide lines take the mouse; the rest of the stage lets clicks through.
    const juce::Point<float> p { float (x), float (y) };
    const float U = unit();
    if (p.getDistanceFrom (centre()) < (kSphereR + 4.0f) * U || p.getDistanceFrom (handlePos()) < 20.0f * U
        || projectionAt (p) != DragAxis::none)
        return true;
    const auto gg = guideGeom();
    const float tol = 7.0f * U;
    const bool onV = std::abs (p.x - gg.cornerX) < tol && p.y > gg.topY && p.y < gg.baseY + tol;
    const bool onH = std::abs (p.y - gg.baseY) < tol && p.x > gg.cornerX - tol && p.x < gg.rightX;
    return onV || onH;
}

Orb::Radii Orb::coreRadii (float radius) const
{
    Radii radii {};
    const float life = 0.54f + 0.53f * act;
    const float clock = float (t);
    const float tilt = 0.18f * std::sin (0.38f + contourPhase) - 0.08f;
    const float leftCentre = 3.79f + 0.20f * std::sin (clock * 1.55f);
    const float rightCentre = 2.68f + 0.13f * std::sin (0.7f - clock * 1.25f);
    const auto lobe = [] (float angle)
    {
        const float shoulder = juce::jmax (0.0f, std::cos (angle));
        const float squared = shoulder * shoulder;
        return squared * squared * squared;
    };
    float mean = 0.0f;
    for (int i = 0; i < contourSamples; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * float (i) / contourSamples;
        const float x = std::sin (angle);
        const float y = -std::cos (angle);
        const float down = juce::jmax (0.0f, -std::cos (angle - tilt));
        const float lowerArc = 0.087f * lobe (angle - leftCentre) - 0.047f * lobe (angle - rightCentre)
                             - 0.083f * down * down * down * down;
        const float warp = theme::surfaceNoise (21.0f * x + 32.0f + clock * 3.7f,
                                                 28.0f * y - 19.0f - clock * 2.8f);
        const float detail = theme::surfaceNoise (93.0f * x - 16.0f - clock * 3.2f,
                                                   93.0f * y + 47.0f + clock * 2.5f);
        const float lean = 0.047f * std::cos (angle - 5.04f - tilt);
        // Low-order profile correction measured across the aligned reference sequence.
        const float profile = -0.002f * y + 0.008f * x
                            + 0.036f * std::cos (2.0f * angle) - 0.034f * std::sin (2.0f * angle)
                            - 0.021f * std::cos (3.0f * angle) + 0.009f * std::sin (3.0f * angle)
                            + 0.014f * std::cos (4.0f * angle) + 0.005f * std::sin (4.0f * angle)
                            - 0.013f * lobe (angle - 0.94f) - 0.007f * lobe (angle - 4.92f);
        const float fluidBend = 0.012f * std::sin (3.0f * angle - 1.23f * clock + 0.41f)
                              + 0.008f * std::cos (2.0f * angle + 0.79f * clock + 0.26f);
        const float deform = lean + profile + life * (lowerArc + fluidBend + 0.088f * warp + 0.014f * detail);
        radii[size_t (i)] = deform;
        mean += deform;
    }
    mean /= float (contourSamples);
    for (int i = 0; i < contourSamples; ++i)
        radii[size_t (i)] = radius * (0.951f + radii[size_t (i)] - mean);
    radii.back() = radii.front();
    return radii;
}

juce::Path Orb::coreContour (juce::Point<float> centre, const Radii& radii) const
{
    juce::Path contour;
    for (int i = 0; i < contourSamples; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * float (i) / contourSamples;
        const float radius = radii[size_t (i)];
        const auto point = centre.translated (radius * std::sin (angle), -radius * std::cos (angle));
        if (i == 0) contour.startNewSubPath (point); else contour.lineTo (point);
    }
    contour.closeSubPath();
    return contour;
}

void Orb::drawParticles (juce::Graphics& g, juce::Point<float> centre, const Radii& radii, float flow, float colourFlow, float light)
{
    struct Particle { float angle, x, y, offset, size, alpha, phaseX, phaseY; };
    static const auto particles = []
    {
        std::array<Particle, 10400> result {};
        juce::Random random (0x6f7262);
        for (auto& p : result)
        {
            const float reach = std::pow (random.nextFloat(), 1.85f);
            constexpr float turn = juce::MathConstants<float>::twoPi;
            const float angle = random.nextFloat() * turn;
            const float offset = reach * (random.nextBool() ? 13.0f : -6.5f);
            const float size = 0.34f + 0.94f * random.nextFloat();
            const float alpha = (0.19f + 0.65f * random.nextFloat()) * (1.0f - 0.73f * reach);
            const float phase = random.nextFloat() * turn;
            p = { angle * contourSamples / turn, std::sin (angle), -std::cos (angle),
                  offset, size, alpha, std::sin (phase), std::cos (phase) };
        }
        return result;
    }();

    constexpr float turn = juce::MathConstants<float>::twoPi;
    std::array<juce::Colour, contourSamples> palette;
    std::array<float, contourSamples> density, emission, outwardSpread, exteriorShoulder;
    std::array<float, contourSamples> smokeLift, smokeLight, smokeCurl, smokeExchange, crestBreak;
    struct CrestProfile { float inward, outward, crestGain, smokeGain; };
    // Width and light fit sampled clockwise from the top. Separate crest and smoke gains
    // move light into the reference's softer shoulders only where they are visible.
    static constexpr std::array<CrestProfile, 12> crestProfile {{
        { 0.684f, 1.015f, 0.970f, 1.21f }, { 0.574f, 1.100f, 1.033f, 1.11f }, { 1.034f, 1.200f, 0.800f, 0.97f },
        { 0.850f, 1.210f, 1.020f, 1.18f }, { 0.747f, 1.145f, 1.074f, 1.26f }, { 0.616f, 0.895f, 1.124f, 1.20f },
        { 0.766f, 1.300f, 0.879f, 1.27f }, { 1.112f, 0.765f, 1.135f, 0.84f }, { 1.210f, 1.145f, 1.101f, 0.92f },
        { 0.650f, 0.865f, 1.050f, 1.11f }, { 0.672f, 0.765f, 1.056f, 0.85f }, { 0.539f, 0.955f, 1.008f, 1.26f }
    }};
    const auto shoulder = [] (float angle, float peak)
    {
        const float arc = juce::jmax (0.0f, std::cos (angle - juce::degreesToRadians (peak)));
        const float squared = arc * arc;
        const float fourth = squared * squared;
        return fourth * fourth;
    };
    // RGB residuals for the upper continuous crest, in the same angular order.
    static constexpr std::array<std::array<float, 3>, 12> crestTint {{
        { -13.0f, -27.0f, 6.0f }, { 4.0f, -42.0f, 4.0f }, { 16.0f, -41.0f, -1.0f },
        { 23.0f, -5.0f, -5.0f }, { 27.0f, -2.0f, -12.0f }, { 15.0f, 2.0f, -5.0f },
        { -3.0f, 1.0f, 1.0f }, { 2.0f, 0.0f, 0.0f }, { -1.0f, -3.0f, -4.0f },
        { -6.0f, -3.0f, -4.0f }, { -19.0f, -8.0f, 8.0f }, { 2.0f, -33.0f, 16.0f }
    }};
    static constexpr std::array<float, 12> trailBalance {
        0.78f, 0.89f, 0.65f, 1.04f, 1.29f, 1.12f, 1.17f, 0.95f, 1.36f, 0.95f, 1.0f, 1.08f
    };
    const float crestTintStrength = splitAmount * (1.0f - bypassAmount);
    const float phase = std::remainder (flow, turn);
    const float clock = float (t);
    const float tintPhase = std::remainder (colourFlow, turn);
    const auto lilac = theme::orbTint (juce::Colour (0xffc29aff));
    const auto pink = theme::orbTint (juce::Colour (0xffe7addd));
    const float smokeX = 46.0f * std::sin (clock * 1.37f);
    const float smokeY = 38.0f * std::cos (clock * 1.13f);
    float meanLift = 0.0f, meanLight = 0.0f;
    for (int i = 0; i < contourSamples; ++i)
    {
        const float angle = turn * float (i) / contourSamples;
        const float local = std::fmod (angle - phase + turn, turn);
        const float smokeAngle = angle - clock * 0.87f;
        const float u = 98.0f * std::sin (smokeAngle) + smokeX;
        const float v = 98.0f * std::cos (smokeAngle) + smokeY;
        const auto index = size_t (i);
        const float billow = theme::surfaceNoise (u, v);
        const float filament = std::sin (5.0f * angle - 5.3f * clock + 2.1f * billow);
        const float counterflow = std::sin (9.0f * angle + 7.1f * clock - 1.7f * billow);
        smokeLift[index] = 7.4f * (billow + 0.30f * filament + 0.15f * counterflow);
        smokeLight[index] = 1.05f * theme::surfaceNoise (u + 73.0f, v - 41.0f)
                           + 0.23f * filament + 0.11f * counterflow;
        const float trailAngle = angle + 0.53f * clock;
        smokeCurl[index] = theme::surfaceNoise (83.0f * std::sin (trailAngle) - smokeY,
                                                83.0f * std::cos (trailAngle) + smokeX)
                         + 0.22f * std::sin (4.0f * angle + 3.2f * clock + billow);
        smokeExchange[index] = 0.62f * std::sin (3.0f * angle - 6.4f * clock)
                             + 0.58f * std::sin (5.0f * angle + 8.1f * clock + 0.8f)
                             + 0.42f * std::sin (2.0f * angle + 4.2f * clock + 1.5f);
        const float gaps = juce::jlimit (0.0f, 1.0f, 0.64f - 0.72f * filament - 0.80f * billow);
        crestBreak[index] = shoulder (angle, 52.0f) * gaps * gaps * crestTintStrength;
        meanLift += smokeLift[index];
        meanLight += smokeLight[index];
        const auto plum = lilac.interpolatedWith (pink, 0.5f + 0.5f * std::sin (2.0f * local + clock));
        const float colourAngle = std::fmod (angle - tintPhase + turn, turn);
        const float cloud = theme::surfaceNoise (43.0f * std::sin (local) + clock * 4.8f,
                                                  43.0f * std::cos (local) - clock * 3.0f);
        const float knots = theme::surfaceNoise (101.0f * std::sin (local) - clock * 3.7f,
                                                  101.0f * std::cos (local) + clock * 5.3f);
        const float x = std::sin (colourAngle);
        const float y = -std::cos (colourAngle);
        const float upper = juce::jlimit (0.0f, 1.0f, 0.26f - 0.78f * y + 0.12f * x);
        const float lower = juce::jlimit (0.0f, 1.0f, 0.23f + 0.78f * y - 0.34f * x);
        const float left = juce::jmax (0.0f, -x);
        const float lowerRight = juce::jlimit (0.0f, 1.0f, 0.62f + 0.72f * x) * juce::jmax (0.0f, y);
        const float rightPink = juce::jmax (0.0f, x) * (1.0f - juce::jmax (0.0f, y));
        const float bottom = juce::jmax (0.0f, y);
        const float highlight = juce::jlimit (0.0f, 1.0f, 0.28f + 0.50f * upper + 0.55f * lower
                                                         + 0.40f * cloud + 0.62f * knots
                                                         + 0.41f * left - 0.38f * lowerRight
                                                         + 0.24f * rightPink - 0.19f * bottom * bottom);
        const float whiteMix = 0.13f + (0.67f - 0.14f * bottom) * highlight * highlight
                             + 0.11f * juce::jmax (0.0f, knots);
        const auto chroma = coreColourAt (colourAngle)
            .interpolatedWith (pink, 0.56f * upper + 0.37f * rightPink)
            .interpolatedWith (theme::orbTint (juce::Colour (0xffa2dbef)), 0.88f * lower);
        palette[size_t (i)] = plum.interpolatedWith (chroma, 0.16f + 0.84f * splitAmount)
            .interpolatedWith (juce::Colour (0xffffa398), bypassAmount)
            .interpolatedWith (juce::Colours::white, whiteMix);
        const float lowerRightSide = 2.0f * juce::jmax (0.0f, x) * bottom;
        const float bottomCentre = bottom * bottom;
        emission[size_t (i)] = 0.18f + 1.21f * highlight + 0.17f * rightPink
                               + 0.23f * lowerRightSide - 0.14f * bottomCentre * bottomCentre;
        outwardSpread[size_t (i)] = 0.75f + 0.40f * left + 0.25f * upper
                                   + 0.24f * lowerRightSide - 0.22f * bottomCentre;
        exteriorShoulder[size_t (i)] = 0.24f * shoulder (angle, 150.0f) + 0.38f * shoulder (angle, 225.0f)
                                      + 0.29f * shoulder (angle, 300.0f) + 0.30f * shoulder (angle, 390.0f);
        density[size_t (i)] = juce::jlimit (0.16f, 1.38f, 0.56f + 0.94f * cloud + 0.74f * knots
                                                        + 0.24f * upper + 0.25f * lower
                                                        + 0.28f * left - 0.27f * lowerRight);
    }

    // Remove the angular mean so smoke can billow locally without pulsing the whole rim.
    meanLift /= float (contourSamples);
    meanLight /= float (contourSamples);
    const auto ramp = [] (float value)
    {
        const float x = juce::jlimit (0.0f, 1.0f, value);
        return x * x * (3.0f - 2.0f * x);
    };
    const float u = unit();
    const int size = particleBuffer.getWidth();
    const float renderScale = float (size) / (2.65f * kCoreR * u);
    const float half = 0.5f * size;
    juce::Image::BitmapData pixels (particleBuffer, juce::Image::BitmapData::readWrite);
    for (int y = 0; y < size; ++y)
        std::memset (pixels.getLinePointer (y), 0, size_t (size) * sizeof (juce::PixelARGB));

    // Light the deforming boundary in the same raster as its scattered grains.
    for (const auto& p : edgePixels)
    {
        const auto i = size_t (p.angle);
        const auto next = size_t ((p.angle + 1) % contourSamples);
        const float cluster = std::lerp (density[i], density[next], p.fraction);
        const float signedDistance = (p.radius - std::lerp (radii[i], radii[i + 1], p.fraction)) / u;
        const float spread = std::lerp (outwardSpread[i], outwardSpread[next], p.fraction);
        const float brightness = std::lerp (emission[i], emission[next], p.fraction);
        // Keep the inner wall compact and let the outward scatter follow the local rim.
        const float inward = juce::jmax (0.0f, -signedDistance);
        const float outward = juce::jmax (0.0f, signedDistance);
        const float sector = (float (p.angle) + p.fraction) * float (crestProfile.size()) / contourSamples;
        const auto sectorIndex = size_t (sector);
        const auto& a = crestProfile[sectorIndex];
        const auto& b = crestProfile[(sectorIndex + 1) % crestProfile.size()];
        const float fraction = sector - float (sectorIndex);
        const float innerWidth = (2.32f + 1.50f * cluster) * std::lerp (a.inward, b.inward, fraction);
        const float outerWidth = (3.5f + 2.55f * spread) * std::lerp (a.outward, b.outward, fraction);
        const float crest = juce::jmax (0.0f, 1.0f - inward / innerWidth - outward / outerWidth);
        const float crestGain = std::lerp (a.crestGain, b.crestGain, fraction);
        const float smokeGain = std::lerp (a.smokeGain, b.smokeGain, fraction);
        const float smokeWeight = ramp ((outward - 2.0f) / 4.0f);
        const float lift = juce::jlimit (-5.2f, 5.2f, std::lerp (smokeLift[i], smokeLift[next], p.fraction) - meanLift);
        const float billow = juce::jlimit (-0.62f, 0.76f, std::lerp (smokeLight[i], smokeLight[next], p.fraction) - meanLight);
        const float smokeRadius = juce::jmax (0.0f, outward - smokeWeight * lift);
        const float glow = 1.0f + smokeWeight * billow;
        const float scatter = juce::jlimit (0.0f, 1.0f, smokeRadius * 0.36f)
                            * juce::jmax (0.0f, 1.0f - smokeRadius / (8.5f + 9.0f * spread));
        // Only the exterior scatter follows the smoke field. The crest keeps its fitted widths.
        const float shoulderBand = ramp ((smokeRadius - 2.0f) / 3.0f) * ramp ((14.0f - smokeRadius) / 6.0f);
        const float shoulderStrength = std::lerp (exteriorShoulder[i], exteriorShoulder[next], p.fraction);
        const float outerBalance = std::lerp (1.0f, smokeGain, ramp ((outward - 1.5f) / 3.5f));
        // A broad envelope carries two separate, tapering translucent strands.
        const float broadLight = ((0.28f + 0.23f * cluster) * spread * scatter
                                + 0.90f * shoulderStrength * shoulderBand) * glow * outerBalance;
        float strandAlpha = 0.0f;
        float transfer = 0.0f;
        if (outward > 0.0f && outward < 17.0f)
        {
            const float curl = std::lerp (smokeCurl[i], smokeCurl[next], p.fraction);
            const float nearPosition = (outward - (3.5f + 0.48f * lift)) / 4.9f;
            const float farPosition = (outward - (7.0f + 2.6f * curl + 0.19f * lift)) / 5.7f;
            const float nearDepth = (0.38f + 0.59f * shoulderStrength) * ramp (1.0f - nearPosition * nearPosition)
                                  * ramp (outward / 2.6f) * (1.0f + 0.37f * billow);
            const float farDepth = (0.10f + 0.48f * shoulderStrength) * ramp (1.0f - farPosition * farPosition)
                                 * ramp (outward / 4.0f) * (0.85f + 0.58f * curl);
            strandAlpha = 1.0f - std::exp (-(nearDepth + farDepth) * light * outerBalance);
            // Exchange opacity outside the luminous edge. The taper keeps shoulder
            // transport from shifting the crest even when the two layers overlap.
            const float nearBand = (outward - (5.0f + 0.18f * lift)) / 1.65f;
            const float farBand = (outward - (9.1f + 0.28f * curl)) / 2.0f;
            const float exchange = std::lerp (smokeExchange[i], smokeExchange[next], p.fraction);
            transfer = 0.207f * exchange * ramp ((outward - 2.7f) / 2.5f)
                     * (0.76f * std::exp (-0.5f * farBand * farBand) - std::exp (-0.5f * nearBand * nearBand));
            transfer *= light / 0.83f;
        }
        // Preserve the fitted filament. The moving depth layers replace only its shoulder.
        const float crestShimmer = 1.0f + 0.23f * billow - 0.14f * ramp ((-billow - 0.07f) / 0.34f);
        const float gap = std::lerp (crestBreak[i], crestBreak[next], p.fraction);
        const float crestAlpha = 1.46f * brightness * ramp (crest) * crestGain * crestShimmer
                               * (1.0f - 0.23f * gap) * (0.88f + 0.22f * p.grain) * light;
        const float hazeAlpha = broadLight * (0.73f + 0.37f * p.grain) * light;
        const float shoulderAlpha = hazeAlpha + (1.0f - hazeAlpha) * strandAlpha;
        const float trailGain = std::lerp (trailBalance[sectorIndex],
                                           trailBalance[(sectorIndex + 1) % trailBalance.size()], fraction);
        const float exteriorAlpha = std::lerp (hazeAlpha, shoulderAlpha, 0.52f)
                                  * std::lerp (1.0f, trailGain, ramp ((outward - 1.8f) / 3.4f));
        const float opacity = crestAlpha + juce::jmax (0.0f, exteriorAlpha + transfer);
        const float tintAmount = crestTintStrength * juce::jmax (0.0f, 1.0f - std::abs (signedDistance) * 0.25f) / 255.0f;
        const auto& tintA = crestTint[sectorIndex];
        const auto& tintB = crestTint[(sectorIndex + 1) % crestTint.size()];
        const auto colour = palette[i];
        const auto channel = [&] (float value, size_t index)
        {
            return juce::jlimit (0.0f, 1.0f, value + tintAmount * std::lerp (tintA[index], tintB[index], fraction));
        };
        const auto crestColour = juce::Colour::fromFloatRGBA (channel (colour.getFloatRed(), 0),
                                                             channel (colour.getFloatGreen(), 1),
                                                             channel (colour.getFloatBlue(), 2),
                                                             juce::jmin (1.0f, opacity));
        auto* row = reinterpret_cast<juce::PixelARGB*> (pixels.getLinePointer (p.y));
        row[p.x] = crestColour.getPixelARGB();
    }

    const float drift = phase * contourSamples / turn;
    const float dx = std::sin (phase), dy = std::cos (phase);
    const float shimmerX = std::sin (clock * 2.2f), shimmerY = std::cos (clock * 2.2f);
    for (const auto& p : particles)
    {
        float index = p.angle + drift;
        if (index < 0.0f) index += contourSamples;
        if (index >= contourSamples) index -= contourSamples;
        const int i = int (index);
        const float flutter = p.phaseX * shimmerY + p.phaseY * shimmerX;
        const float fraction = index - float (i);
        const auto currentIndex = size_t (i);
        const auto nextIndex = size_t ((i + 1) % contourSamples);
        const float cluster = std::lerp (density[currentIndex], density[nextIndex], fraction);
        const float offset = p.offset * (0.50f + 0.50f * cluster);
        const float smokeWeight = ramp ((offset - 2.0f) / 5.0f);
        const float lift = juce::jlimit (-5.2f, 5.2f, std::lerp (smokeLift[currentIndex], smokeLift[nextIndex], fraction) - meanLift);
        const float glow = juce::jlimit (-0.62f, 0.76f, std::lerp (smokeLight[currentIndex], smokeLight[nextIndex], fraction) - meanLight);
        const float exchange = std::lerp (smokeExchange[currentIndex], smokeExchange[nextIndex], fraction);
        const float gap = std::lerp (crestBreak[currentIndex], crestBreak[nextIndex], fraction);
        const float flutterAmount = std::lerp (0.78f, 0.34f, smokeWeight);
        const float flutterLight = std::lerp (0.12f, 0.055f, smokeWeight);
        const float radius = std::lerp (radii[currentIndex], radii[currentIndex + 1], fraction)
                           + u * (offset + smokeWeight * (lift + 0.44f * exchange) + flutterAmount * flutter);
        const float grainCoverage = 0.59f + 0.41f * ramp ((std::abs (offset) - 1.5f) / 4.0f);
        const float opacity = p.alpha * cluster * light * grainCoverage
                            * (0.88f + flutterLight * flutter)
                            * (1.0f + smokeWeight * glow + 0.11f * gap);
        const float x = half + radius * renderScale * (p.x * dy - p.y * dx);
        const float y = half + radius * renderScale * (p.x * dx + p.y * dy);
        if (x < 0.0f || y < 0.0f || x >= float (size - 1) || y >= float (size - 1))
            continue;
        const int left = int (x), top = int (y);
        const float sx = x - float (left), sy = y - float (top);
        const auto colour = palette[size_t (i)].withAlpha (juce::jmin (1.0f, opacity * p.size * renderScale));
        const auto pixel = colour.getPixelARGB();
        auto* row = reinterpret_cast<juce::PixelARGB*> (pixels.getLinePointer (top));
        auto* next = reinterpret_cast<juce::PixelARGB*> (pixels.getLinePointer (top + 1));
        row[left].blend (pixel, juce::uint32 (256.0f * (1.0f - sx) * (1.0f - sy)));
        row[left + 1].blend (pixel, juce::uint32 (256.0f * sx * (1.0f - sy)));
        next[left].blend (pixel, juce::uint32 (256.0f * (1.0f - sx) * sy));
        next[left + 1].blend (pixel, juce::uint32 (256.0f * sx * sy));
    }

    const float side = 2.65f * kCoreR * u;
    // The raster already contains the light and particle opacity.
    g.setOpacity (1.0f);
    g.drawImage (particleBuffer, juce::Rectangle<float> (side, side).withCentre (centre));
}

void Orb::renderImages (float sc)
{
    renderedScale = sc;
    const float U = unit();
    // A cloudy inner falloff and a narrow, uneven crest share a seeded texture.
    const auto shellRadius = kSphereR * U;
    const int shellSide = juce::jmax (1, juce::roundToInt (2.12f * shellRadius * sc));
    sphere = juce::Image (juce::Image::ARGB, shellSide, shellSide, true, juce::SoftwareImageType {});
    {
        juce::Image::BitmapData pixels (sphere, juce::Image::BitmapData::writeOnly);
        juce::Random random (0x5348454c);
        const float half = 0.5f * shellSide;
        for (int y = 0; y < shellSide; ++y)
            for (int x = 0; x < shellSide; ++x)
            {
                const float nx = (float (x) + 0.5f - half) * 1.06f / half;
                const float ny = (float (y) + 0.5f - half) * 1.06f / half;
                const float r = std::sqrt (nx * nx + ny * ny);
                if (r < 0.63f || r > 1.052f) continue;

                const float grain = random.nextFloat() - 0.5f;
                const float cloud = theme::surfaceNoise (nx * 180.0f + 38.0f, ny * 180.0f + 64.0f);
                const float fibres = theme::surfaceNoise (nx * 640.0f - 22.0f, ny * 640.0f + 37.0f);
                const float inverse = 1.0f / r;
                const float axis = nx * inverse;
                const float roughness = theme::surfaceNoise (axis * 108.0f, ny * inverse * 108.0f);
                const float edge = 0.996f + 0.010f * roughness + 0.005f * (4.0f * axis * axis * axis - 3.0f * axis);
                const float d = r - edge;
                const float ridge = std::exp (-std::pow (d / (d > 0.0f ? 0.0105f : 0.027f), 2.0f));
                const float air = std::exp (juce::jmin (d, 0.0f) / 0.140f) * std::exp (-juce::jmax (d, 0.0f) / 0.0095f);
                const float innerMask = juce::jlimit (0.0f, 1.0f, (r - 0.63f) / 0.17f);
                const float light = (0.51f - 0.126f * ny - 0.043f * nx + 0.16f * roughness) * ridge;
                const float mist = (0.41f + 0.29f * cloud + 0.103f * fibres + 0.128f * grain) * air;
                const float opacity = juce::jlimit (0.0f, 1.0f, (light + mist) * innerMask);
                const auto colour = juce::Colour (0xffe3dee9).interpolatedWith (juce::Colours::white, ridge * 0.60f);
                pixels.setPixelColour (x, y, colour.withAlpha (opacity));
            }
    }

    // Keep the violet ring circular while varying its crest and falloff around the angle.
    const float fieldRadius = kFieldR * U;
    const int fieldSide = juce::jmax (1, juce::roundToInt (2.0f * (fieldRadius + 14.0f * U) * sc));
    fieldMask = juce::Image (juce::Image::ARGB, fieldSide, fieldSide, true, juce::SoftwareImageType {});
    {
        juce::Image::BitmapData pixels (fieldMask, juce::Image::BitmapData::writeOnly);
        const float half = 0.5f * fieldSide;
        for (int y = 0; y < fieldSide; ++y)
            for (int x = 0; x < fieldSide; ++x)
            {
                const float nx = (float (x) + 0.5f - half) / (sc * U);
                const float ny = (float (y) + 0.5f - half) / (sc * U);
                const float r = std::sqrt (nx * nx + ny * ny);
                const float d = r - kFieldR;
                if (std::abs (d) > 14.0f) continue;
                const float shade = 0.94f + 0.25f * theme::surfaceNoise (nx * 0.59f + 72.0f, ny * 0.59f);
                const float profile = 0.68f * std::exp (-0.5f * (d / 4.05f) * (d / 4.05f))
                                    + 0.18f * std::exp (-0.5f * (d / 7.5f) * (d / 7.5f));
                pixels.setPixelColour (x, y, juce::Colours::white.withAlpha (profile * shade));
            }
    }

    // Cache only the neutral interior grain. The edge particles follow the live contour.
    const int side = juce::jmax (1, int (kCoreR * U * 2.45f * sc));
    grain = juce::Image (juce::Image::ARGB, side, side, true);
    const int dustSide = juce::jmax (1, int (kCoreR * U * 2.65f * juce::jmin (2.0f, sc)));
    particleBuffer = juce::Image (juce::Image::ARGB, dustSide, dustSide, true, juce::SoftwareImageType {});
    edgePixels.clear();
    const float unitPerPixel = 2.65f * kCoreR * U / float (dustSide);
    const float half = 0.5f * dustSide;
    juce::Random edgeSeed (0x45444745);
    for (int y = 0; y < dustSide; ++y)
        for (int x = 0; x < dustSide; ++x)
        {
            const float dx = (float (x) + 0.5f - half) * unitPerPixel;
            const float dy = (float (y) + 0.5f - half) * unitPerPixel;
            const float radius = std::sqrt (dx * dx + dy * dy);
            if (std::abs (radius / U - kCoreR) > 24.0f) continue;
            constexpr float turn = juce::MathConstants<float>::twoPi;
            const float index = std::fmod (std::atan2 (dx, -dy) + turn, turn) * contourSamples / turn;
            const int i = juce::jmin (int (index), contourSamples - 1);
            edgePixels.push_back ({ x, y, i, radius, index - float (i), edgeSeed.nextFloat() });
        }
    juce::Image::BitmapData dots (grain, juce::Image::BitmapData::writeOnly);
    juce::Random random (0x47524149);
    for (int y = 0; y < side; ++y)
        for (int x = 0; x < side; ++x)
        {
            const float cloud = theme::surfaceNoise (float (x) / sc * 0.38f + 74.0f, float (y) / sc * 0.38f);
            const float fleck = theme::surfaceNoise (float (x) / sc * 1.6f, float (y) / sc * 1.6f);
            const float value = random.nextFloat() - 0.5f;
            const auto colour = value > 0.0f ? juce::Colours::white : juce::Colours::black;
            const float contrast = juce::jlimit (0.06f, 0.30f, 0.19f + 0.18f * cloud + 0.09f * fleck);
            dots.setPixelColour (x, y, colour.withAlpha (std::abs (value) * contrast));
        }
}

void Orb::paint (juce::Graphics& g)
{
    using namespace theme;
    const float scale = juce::Component::getApproximateScaleFactorForComponent (this);
    // The editor scales its canvas, so the orb's logical bounds can stay fixed as it grows on screen.
    if (sphere.isNull() || scale > renderedScale)
        renderImages (std::ceil (scale * 4.0f) * 0.25f);
    const float U = unit();
    const auto c = centre();
    const float a = act;
    const float breath = 0.5f + 0.5f * std::sin (float (t * 0.72));
    const float hit = syllable;
    const float wet = juce::jlimit (0.0f, 1.0f, yNorm);
    const float width = juce::jlimit (0.0f, 1.0f, xNorm);
    const auto ease = [] (float v)
    {
        const float x = juce::jlimit (0.0f, 1.0f, v);
        return x * x * (3.0f - 2.0f * x);
    };
    const float atmosphere = ease (1.55f * a + 0.18f * hit);
    const float stereo = ease ((wet - 0.12f) / 0.35f);
    const auto modeColour = [this] (juce::uint32 plum, juce::uint32 split, juce::uint32 warm)
    {
        return orbTint (juce::Colour (plum).interpolatedWith (juce::Colour (split), splitAmount))
            .interpolatedWith (juce::Colour (warm), bypassAmount);
    };

    // A quiet phrase leaves a thin outline. The smoky shell returns with the voice.
    const float grow = 0.98f + 0.012f * breath + 0.025f * a;
    const float sphereRadius = kSphereR * U * grow;
    const float voiceHue = ease (0.48f + 0.66f * std::sin (float (2.7 * t) + 2.0f * coreAngle));
    const float haloEnvelope = ease (1.56f * haloLight + 0.15f * hit);
    const float sustainedColour = (1.0f - splitAmount) * 0.89f * ease ((voicedSeconds - 0.75f) / 1.15f);
    const float violetShell = haloEnvelope * juce::jmax (sustainedColour * (0.78f + 0.22f * voiceHue),
                                                         stereo * voiceHue);
    const float idlePearl = (0.10f + 0.17f * breath) * (1.0f - stereo);
    const float pearl = juce::jmax (idlePearl, haloEnvelope) * (1.0f - 0.80f * violetShell)
                      * (1.0f - 0.67f * bypassAmount);
    const float shellSide = sphereRadius * 2.12f;
    const auto shellTransform = juce::AffineTransform::translation (-0.5f * sphere.getWidth(), -0.5f * sphere.getHeight())
        .scaled (shellSide / float (sphere.getWidth())).rotated (haloPhase).translated (c.x, c.y);
    if (pearl > 0.004f)
    {
        g.setOpacity (pearl);
        g.drawImageTransformed (sphere, shellTransform);
        g.setOpacity (1.0f);
    }

    // Apply the bypass and Doubler tint through the same atmosphere texture.
    const float normalWash = 0.37f * (1.0f - haloEnvelope) + 0.73f * violetShell;
    const float wash = std::lerp (normalWash, 0.43f + 0.43f * haloEnvelope, bypassAmount);
    const auto tint = juce::Colour (0xffd5d0dd).interpolatedWith (orbTint (juce::Colour (0xff9472ed)), violetShell)
        .interpolatedWith (bypassAccent, bypassAmount);
    g.setColour (tint.withAlpha (juce::jmin (1.0f, wash)));
    g.drawImageTransformed (sphere, shellTransform, true);

    // The field keeps the same radius and fades into the stereo effect as Wet rises.
    const float fieldFade = ease ((wet - 0.58f) / 0.27f);
    const float flicker = theme::surfaceNoise (92.0f * std::cos (fieldPhase), 139.0f + 92.0f * std::sin (fieldPhase));
    const float pulse = (0.55f + 0.27f * atmosphere + 0.10f * hit + 0.07f * flicker) * (1.0f - fieldFade);
    const auto field = modeColour (0xff9075de, 0xffa383ec, 0xffe97c7d);
    const float fieldSide = 2.0f * (kFieldR + 14.0f) * U * grow;
    const auto fieldTransform = juce::AffineTransform::translation (-0.5f * fieldMask.getWidth(), -0.5f * fieldMask.getHeight())
        .scaled (fieldSide / float (fieldMask.getWidth())).rotated (0.18f * std::sin (fieldPhase)).translated (c.x, c.y);
    g.setColour (field.withAlpha (pulse));
    g.drawImageTransformed (fieldMask, fieldTransform, true);

    const float swell = 0.98f + 0.023f * breath + 0.035f * a + 0.031f * hit;
    const float rk = kCoreR * U * swell;
    const auto radii = coreRadii (rk);
    const auto contour = coreContour (c, radii);
    const float light = 0.12f + 0.73f * atmosphere + 0.14f * hit;
    const auto bounds = juce::Rectangle<float> (2.45f * rk, 2.45f * rk).withCentre (c);
    const float colourClock = float (colourTime);
    const float warmDrift = theme::surfaceNoise (colourClock * 27.0f, 82.0f);
    const float coolDrift = theme::surfaceNoise (148.0f, colourClock * 31.0f);
    const float rimTintShift = 0.24f * warmDrift - 0.18f * coolDrift;
    const float fold = std::sin (2.8f * colourClock - 1.6f + 0.12f * std::sin (coreAngle));
    const float pool = std::sin (3.7f * colourClock + 1.2f);
    const float veil = std::sin (2.3f * colourClock + 0.44f);
    const float lateral = std::sin (colourPhase);
    // The warm fold shears across the core while the cool pocket narrows and rebounds.
    const float foldShear = ease ((fold - 0.814f) / 0.177f);
    const float pocketContraction = ease ((pool - 0.34f) / 0.46f) * ease ((1.0f - pool) / 0.23f);
    const float energy = 0.45f + 0.55f * light;
    const auto base = modeColour (0xff272040, 0xff30152d, 0xff291619);
    const auto plum = modeColour (0xff654485, 0xff80387a, 0xffa14f55)
        .interpolatedWith (modeColour (0xff654485, 0xff813681, 0xffa14f55), foldShear);
    const auto violetVeil = modeColour (0xff58438c, 0xff2c2a63, 0xff855264);
    const auto teal = modeColour (0xff3d437d, 0xff185c5f, 0xff592a38)
        .interpolatedWith (modeColour (0xff3d437d, 0xff135b6c, 0xff592a38), foldShear);
    const auto deepTeal = modeColour (0xff1e224b, 0xff0a1728, 0xff341923);
    const auto clay = modeColour (0xff7e6484, 0xffc57d60, 0xffa96759);
    const auto shadow = modeColour (0xff211631, 0xff141d17, 0xff34171a);
    const auto roseVeil = modeColour (0xff8766b0, 0xffc588ab, 0xffc17e73);
    const auto foldLight = modeColour (0xffb091d4, 0xffefbde7, 0xffd49685);
    // Local absorptive and illuminated pigment layers, fitted to the visible color pockets.
    const std::array localTints { juce::Colour (0xff844fac), juce::Colour (0xff171b1b),
                                 juce::Colour (0xff4e2064), juce::Colour (0xff0e3f54),
                                 juce::Colour (0xff12557d) };
    const float localPigmentGate = splitAmount * (1.0f - bypassAmount);
    const auto patch = [&ease] (float x, float y, float width, float height)
    {
        const float u = x / width;
        const float v = y / height;
        return ease (1.0f - u * u - v * v);
    };

    // Sum optical depths before compositing so overlapping pigments have no draw order.
    const std::array pigments { plum, violetVeil, teal, deepTeal, clay, shadow, roseVeil };
    const auto mixPigments = [&] (const std::array<float, pigments.size()>& depths)
    {
        float total = 0.0f, red = 0.0f, green = 0.0f, blue = 0.0f;
        for (size_t i = 0; i < pigments.size(); ++i)
        {
            const float depth = depths[i];
            total += depth;
            red += pigments[i].getFloatRed() * depth;
            green += pigments[i].getFloatGreen() * depth;
            blue += pigments[i].getFloatBlue() * depth;
        }
        const float transmit = std::exp (-total);
        const float scatter = (1.0f - transmit) / juce::jmax (1.0e-5f, total);
        return juce::Colour::fromFloatRGBA (base.getFloatRed() * transmit + red * scatter,
                                           base.getFloatGreen() * transmit + green * scatter,
                                           base.getFloatBlue() * transmit + blue * scatter, 1.0f);
    };
    {
        // Sample the moving pigment clouds on a coarse lattice to retain coherent shapes
        // between pixels and avoid evaluating the noise octaves in the inner paint loop.
        struct PigmentCloud { float warm, cool, dx, dy; };
        constexpr int cloudSide = 17;
        std::array<std::array<PigmentCloud, cloudSide>, cloudSide> clouds;
        const float warmX = 52.0f * std::sin (colourClock * 1.02f);
        const float warmY = 40.0f * std::cos (colourClock * 0.83f);
        const float coolX = 47.0f * std::cos (colourClock * 0.91f);
        const float coolY = 49.0f * std::sin (colourClock * 1.16f);
        const auto flowStrength = [] (float seconds)
        {
            return juce::Point<float> (1.98f * std::sin (7.973f * seconds + 0.044f),
                                       2.97f * std::sin (5.946f * seconds - 1.762f));
        };
        const auto velocity = [&ease] (juce::Point<float> point, juce::Point<float> strength)
        {
            const auto left = point.translated (0.40f, -0.32f);
            const auto right = point.translated (-0.43f, -0.19f);
            const float a = strength.x * std::exp (-left.getDistanceSquaredFromOrigin() / (2.0f * 0.42f * 0.42f));
            const float b = strength.y * std::exp (-right.getDistanceSquaredFromOrigin() / (2.0f * 0.37f * 0.37f));
            const float boundary = ease ((0.87f - point.getDistanceSquaredFromOrigin()) / 0.38f);
            return juce::Point<float> (-left.y * a - right.y * b, left.x * a + right.x * b) * boundary;
        };
        const auto currentFlow = flowStrength (colourClock);
        const auto midpointFlow = flowStrength (colourClock - 0.10f);
        const auto sampleCloud = [&] (juce::Point<float> point)
        {
            return juce::Point<float> (
                theme::surfaceNoise (84.0f * point.x + 35.0f * point.y + warmX, 109.0f * point.y - 19.0f * point.x + warmY),
                theme::surfaceNoise (108.0f * point.x - 26.0f * point.y + coolX - 67.0f, 79.0f * point.y + 31.0f * point.x + coolY));
        };
        for (int j = 0; j < cloudSide; ++j)
            for (int i = 0; i < cloudSide; ++i)
            {
                const float x = 2.45f * float (i) / float (cloudSide - 1) - 1.225f;
                const float y = 2.45f * float (j) / float (cloudSide - 1) - 1.225f;
                const juce::Point<float> point (x, y);
                const auto midpoint = point - velocity (point, currentFlow) * 0.10f;
                const float gate = ease ((point.getDistanceSquaredFromOrigin() - 0.065f) / 0.065f)
                                 * (0.66f + 0.34f * ease ((y + 0.25f) / 0.25f));
                const auto displacement = velocity (midpoint, midpointFlow) * (-0.20f * gate);
                const auto lighting = sampleCloud (point);
                clouds[size_t (j)][size_t (i)] = { lighting.x, lighting.y, displacement.x, displacement.y };
            }
        const auto cloudAt = [&] (float x, float y)
        {
            const auto i = size_t (x), j = size_t (y);
            const auto& a = clouds[j][i];
            const auto& b = clouds[j][i + 1];
            const auto& c0 = clouds[j + 1][i];
            const auto& d = clouds[j + 1][i + 1];
            const float u = x - float (i), v = y - float (j);
            const auto blend = [&] (float PigmentCloud::* field)
            {
                return std::lerp (std::lerp (a.*field, b.*field, u), std::lerp (c0.*field, d.*field, u), v);
            };
            return PigmentCloud { blend (&PigmentCloud::warm), blend (&PigmentCloud::cool),
                                  blend (&PigmentCloud::dx), blend (&PigmentCloud::dy) };
        };
        juce::Image::BitmapData pixels (coreField, juce::Image::BitmapData::writeOnly);
        const float step = 2.45f / float (pixels.width);
        const int width = pixels.width, height = pixels.height;
        const size_t pixelCount = size_t (width) * size_t (height);
        std::vector<float> diffuse (localPigmentGate > 0.0f ? pixelCount : 0);
        const float cloudStep = float (cloudSide - 1) / float (pixels.width);
        // Leave two texels of padding for interpolation around the clipped contour.
        const float paintRadius = *std::max_element (radii.begin(), radii.end()) / rk + 2.0f * step;
        const float paintLimit = paintRadius * paintRadius;
        const auto basePixel = base.getPixelARGB();
        const float earlyClay = 1.0f - ease ((fold - 0.48f) / 0.30f);
        const float basinPhase = ease ((0.99f - pool) / 0.20f) * ease ((pool - 0.34f) / 0.30f);
        const float lateDye = ease ((fold - 0.83f) / 0.14f);
        constexpr std::array pocketLight { 0.465f, 0.102f, 0.282f, 0.080f };
        const float ripplePhase = 9.0f * float (t);
        const float rippleSin = std::sin (ripplePhase), rippleCos = std::cos (ripplePhase);
        // Signed lighting exchange across overlapping pigment folds, in display RGB units.
        const float plumLip = (4.94f * rippleSin - 13.29f * rippleCos) / 255.0f;
        const float plumVeil = (2.48f * rippleSin + 3.33f * rippleCos) / 255.0f;
        const float tealVeil = (2.86f * rippleSin - 17.48f * rippleCos) / 255.0f;
        const float plumShadow = (-2.93f * rippleSin - 7.75f * rippleCos) / 255.0f;
        const auto tealFold = [&] (float x, float y)
        {
            const float u = x + 0.30f;
            const float edge = y - 0.49f + 0.26f * u + 0.72f * u * u;
            return patch (u, y - 0.49f, 0.63f, 0.34f) * ease (1.0f - std::abs (edge) / 0.17f);
        };
        for (int row = 0; row < pixels.height; ++row)
        {
            const float y = (float (row) + 0.5f) * step - 1.225f;
            auto* dest = reinterpret_cast<juce::PixelARGB*> (pixels.getLinePointer (row));
            for (int col = 0; col < pixels.width; ++col)
            {
                const float x = (float (col) + 0.5f) * step - 1.225f;
                const float xSquared = x * x;
                const float distanceSquared = xSquared + y * y;
                if (distanceSquared > paintLimit)
                {
                    dest[col] = basePixel;
                    continue;
                }
                // Bounded traveling ripples deform the pigment samples inside the fixed contour.
                const float rippleEnvelope = ease ((0.96f - xSquared - y * y) / 0.42f);
                const auto cloud = cloudAt ((float (col) + 0.5f) * cloudStep, (float (row) + 0.5f) * cloudStep);
                const float warmRipple = rippleEnvelope * (0.70f * std::sin (4.1f * x + 3.4f * y - 8.1f * colourClock)
                                                           + 1.35f * cloud.warm);
                const float coolRipple = rippleEnvelope * (0.66f * std::sin (3.5f * x - 4.4f * y + 7.3f * colourClock + 0.268f)
                                                           + 1.45f * cloud.cool);
                const float materialX = x + cloud.dx;
                const float materialY = y + cloud.dy;
                const float poolX = x + 0.29f - 0.20f * pool - y * (0.24f + 0.16f * coolDrift)
                                  + 0.023f * pocketContraction;
                const float poolY = y - 0.62f + 0.27f * x + 0.20f * xSquared + 0.13f * pool + 0.003f * pocketContraction;
                const float plumMask = patch (x + 0.166f + 0.10f * lateral + 0.15f * y + 0.465f * foldShear + 0.096f * coolRipple,
                                              y + 0.59f - 0.07f * warmDrift - 0.11f * x + 0.116f * foldShear - 0.126f * warmRipple,
                                              1.47f, 0.99f);
                const float veilMask = patch (x + 0.64f + 0.17f * y - 0.16f * veil,
                                              y + 0.03f - 0.28f * x + 0.22f * warmDrift, 0.85f, 0.94f);
                const float tealMask = patch (poolX + 0.087f * warmRipple, poolY - 0.162f * coolRipple,
                                              1.08f * (1.0f - 0.239f * pocketContraction), 0.99f);
                const float basinMask = patch (x + 0.63f - 0.22f * pool + 0.13f * y - 0.072f * pocketContraction,
                                               y - 0.48f + 0.16f * coolDrift, 0.53f, 0.46f);
                const float clayY = y - 0.093f + 0.10f * fold - 0.11f * veil
                                  + 0.24f * x - 0.16f * xSquared - 0.142f * foldShear;
                const float clayMask = patch (x - 0.733f + 0.23f * y + 0.12f * veil - 0.339f * foldShear - 0.158f * warmRipple,
                                              clayY + 0.108f * coolRipple, 0.96f, 0.46f);
                const float shadowMask = patch (x - 0.93f + 0.40f * fold - 0.20f * y + 0.15f * foldShear,
                                                y + 0.53f - 0.12f * warmDrift + 0.028f * foldShear, 0.69f, 0.64f);
                // The sheen follows a curved surface independently of the color masks.
                const float foldU = (x - 0.182f - 0.218f * fold + 0.09f * y) / 1.23f;
                const float foldV = (y + 0.775f - 0.025f * fold - 0.127f * x
                                     + 0.024f * warmRipple - 0.016f * coolRipple) / 0.554f;
                const float foldDistance = foldU * foldU + foldV * foldV;
                // Contrast travels across the existing fold. Its endpoints fade at different phases.
                const float enter = std::lerp (ease ((x - 0.188f) / 0.472f), 1.0f, ease ((fold - 0.342f) / 0.257f));
                const float leave = std::lerp (1.0f, ease ((0.720f - x) / 0.420f), ease ((fold - 0.885f) / 0.104f));
                const float foldOffset = foldDistance - 1.0f;
                const float ridge = 0.78f * ease (1.0f - 6.5f * std::abs (foldOffset))
                                  + 0.22f * ease (1.0f - 2.8f * std::abs (foldOffset));
                const float sheen = ridge * ease (2.6f * foldV)
                                  * patch (x - 0.42f + 0.34f * fold, y + 0.20f, 1.15f, 0.54f)
                                  * ease (0.34f + 0.77f * fold) * enter * leave;
                // Backtrace the translucent lip below the specular ridge. Keep its
                // envelope and the sheen anchored while the overlap changes depth.
                const float lipFlow = 0.45f * localPigmentGate * ease ((foldOffset - 0.09f) / 0.18f);
                const float lipU = foldU + lipFlow * (cloud.dx + 0.09f * cloud.dy) / 1.23f;
                const float lipV = foldV + lipFlow * (cloud.dy - 0.127f * cloud.dx) / 0.554f;
                const float lipOffset = lipU * lipU + lipV * lipV - 1.0f;
                const float foldLip = ease (4.6f * lipOffset) * ease (3.1f * (0.72f - lipOffset)) * ease (2.6f * foldV)
                                    * patch (x - 0.51f + 0.07f * warmRipple, y + 0.16f, 0.70f, 0.36f)
                                    * ease (0.37f + 0.75f * fold);
                // Local depth gradients shade the existing overlapping pigment regions.
                const float plumDepth = 0.962f + 0.290f * (x + 0.20f) - 0.326f * (y + 0.55f)
                                        + rippleEnvelope * (0.79f * cloud.warm - 0.18f * cloud.cool);
                const float tealDepth = 0.902f + 0.124f * (x + 0.25f) - 0.165f * (y - 0.52f)
                                        + rippleEnvelope * (0.91f * cloud.cool - 0.20f * cloud.warm);
                const float clayDepth = 0.982f - 0.127f * (x - 0.72f) - 0.313f * y
                                        + rippleEnvelope * (0.61f * cloud.warm - 0.60f * cloud.cool);
                const float shadeAmount = splitAmount * (1.0f - bypassAmount);
                const auto pigment = mixPigments ({ 1.48f * energy * plumMask * std::lerp (1.0f, juce::jmax (0.12f, plumDepth), shadeAmount),
                                                     1.37f * energy * veilMask,
                                                     1.04f * energy * tealMask * std::lerp (1.0f, juce::jmax (0.12f, tealDepth), shadeAmount),
                                                     (0.67f + 0.56f * energy) * basinMask,
                                                     (0.86f - 0.22f * fold) * energy * clayMask * std::lerp (1.0f, juce::jmax (0.12f, clayDepth), shadeAmount),
                                                     juce::jmax (0.0f, 0.25f + 0.59f * fold + 0.10f * foldShear) * shadowMask,
                                                     0.86f * energy * foldLip });
                const float localGate = localPigmentGate * ease ((0.87f - distanceSquared) / 0.21f);
                auto layered = pigment;
                if (localGate > 0.0f)
                {
                    const std::array localAlpha {
                        localGate * 0.273f * foldShear * patch (x + 0.51f + 0.07f * foldShear,
                                                                y + 0.51f - 0.24f * (x + 0.51f), 0.49f, 0.36f),
                        localGate * 0.043f * foldShear * patch (x - 0.24f + 0.07f * foldShear,
                                                                y + 0.55f + 0.14f * x, 0.54f, 0.38f),
                        localGate * 0.573f * patch (x + 0.32f, y + 0.58f, 0.67f, 0.41f),
                        localGate * 0.151f * patch (x + 0.04f, y - 0.62f + 0.19f * x, 0.52f, 0.34f),
                        localGate * 0.149f * foldShear * patch (x + 0.16f, y - 0.56f + 0.10f * x, 0.70f, 0.48f)
                    };
                    float alpha = 0.0f, red = 0.0f, green = 0.0f, blue = 0.0f;
                    for (size_t i = 0; i < localAlpha.size(); ++i)
                    {
                        alpha += localAlpha[i];
                        red += localTints[i].getFloatRed() * localAlpha[i];
                        green += localTints[i].getFloatGreen() * localAlpha[i];
                        blue += localTints[i].getFloatBlue() * localAlpha[i];
                    }
                    const float transmitted = 1.0f - alpha;
                    layered = juce::Colour::fromFloatRGBA (transmitted * pigment.getFloatRed() + red,
                                                           transmitted * pigment.getFloatGreen() + green,
                                                           transmitted * pigment.getFloatBlue() + blue, 1.0f);
                }
                const auto tinted = layered;
                // The flow also carries changing optical depth through the observed color pockets.
                const float sheenRegion = patch (x - 0.51f, y + 0.17f, 0.58f, 0.24f);
                const float fluidGate = localGate * ease ((distanceSquared - 0.065f) / 0.065f)
                                      * (1.0f - 0.74f * sheenRegion);
                if (fluidGate > 0.0f)
                {
                    const std::array depths {
                        0.59f * earlyClay * patch (materialX - 0.49f, materialY - 0.22f, 0.36f, 0.31f),
                        1.06f * basinPhase * patch (materialX + 0.24f, materialY - 0.60f, 0.45f, 0.40f),
                        0.51f * lateDye * patch (materialX + 0.22f, materialY - 0.59f, 0.47f, 0.39f),
                        0.80f * lateDye * patch (materialX - 0.37f, materialY + 0.46f, 0.35f, 0.32f)
                    };
                    float total = 0.0f, lightDepth = 0.0f;
                    for (size_t i = 0; i < depths.size(); ++i)
                    {
                        total += depths[i];
                        lightDepth += depths[i] * pocketLight[i];
                    }
                    const float red = layered.getFloatRed(), green = layered.getFloatGreen(), blueChannel = layered.getFloatBlue();
                    const float luminance = 0.299f * red + 0.587f * green + 0.114f * blueChannel;
                    const float scattering = total > 0.0f
                        ? (1.0f - std::exp (-fluidGate * total)) * (lightDepth / total - luminance)
                        : 0.0f;
                    // Balance the broad absorption separately from the traveling fold.
                    // The fold moves an existing band of light through the teal pocket.
                    const float basinShade = 0.008f * patch (x + 0.27f, y - 0.59f, 0.51f, 0.43f);
                    const float plumFill = 0.020f * patch (x - 0.36f, y + 0.46f, 0.43f, 0.36f);
                    const float foldShift = 0.082f * (tealFold (materialX, materialY) - tealFold (x, y));
                    const float u = x + 0.24f * cloud.dx;
                    const float v = y + 0.24f * cloud.dy;
                    const float overlap = plumLip * patch (u + 0.37f, v + 0.42f - 0.48f * (u + 0.37f), 0.45f, 0.135f)
                                        + plumVeil * patch (u + 0.45f, v + 0.48f - 0.47f * (u + 0.45f), 0.43f, 0.29f)
                                        + tealVeil * patch (u - 0.61f, v - 0.37f + 0.76f * (u - 0.61f), 0.28f, 0.52f)
                                        + plumShadow * patch (u - 0.40f, v + 0.58f + 0.38f * (u - 0.40f), 0.36f, 0.32f);
                    const float redistributedLight = fluidGate * (plumFill - basinShade + foldShift + overlap);
                    // Local depth changes light and shade while the underlying chroma fronts keep their drift.
                    const float shade = juce::jlimit (-std::min ({ red, green, blueChannel }),
                                                     1.0f - std::max ({ red, green, blueChannel }),
                                                     scattering + redistributedLight);
                    layered = juce::Colour::fromFloatRGBA (red + shade, green + shade, blueChannel + shade, 1.0f);
                }
                const float sheenWeight = 0.72f * energy * sheen;
                dest[col] = layered.interpolatedWith (foldLight, sheenWeight).getPixelARGB();
                if (! diffuse.empty())
                {
                    // The fluid offset is achromatic. Its green-channel delta records the applied offset.
                    diffuse[size_t (row * width + col)] = (layered.getFloatGreen() - tinted.getFloatGreen())
                                                       * (1.0f - sheenWeight);
                }
            }
        }
        if (! diffuse.empty())
        {
            // The measured ripple varies over a shorter span than the diffuse pocket.
            // Redistribute its existing signal across the overlap while preserving its mean.
            std::vector<float> horizontal (pixelCount), narrow (pixelCount), broad (pixelCount);
            const auto boxFilter = [&] (int radius, std::vector<float>& output)
            {
                const float invWidth = 1.0f / float (2 * radius + 1);
                for (int row = 0; row < height; ++row)
                {
                    const size_t start = size_t (row * width);
                    float sum = 0.0f;
                    for (int k = -radius; k <= radius; ++k)
                        sum += diffuse[start + size_t (juce::jlimit (0, width - 1, k))];
                    for (int col = 0; col < width; ++col)
                    {
                        horizontal[start + size_t (col)] = sum * invWidth;
                        sum += diffuse[start + size_t (juce::jmin (width - 1, col + radius + 1))]
                             - diffuse[start + size_t (juce::jmax (0, col - radius))];
                    }
                }
                for (int col = 0; col < width; ++col)
                {
                    float sum = 0.0f;
                    for (int k = -radius; k <= radius; ++k)
                        sum += horizontal[size_t (juce::jlimit (0, height - 1, k) * width + col)];
                    for (int row = 0; row < height; ++row)
                    {
                        output[size_t (row * width + col)] = sum * invWidth;
                        sum += horizontal[size_t (juce::jmin (height - 1, row + radius + 1) * width + col)]
                             - horizontal[size_t (juce::jmax (0, row - radius) * width + col)];
                    }
                }
            };
            const float pixelScale = 1.0f / (71.0f * step);
            boxFilter (juce::jmax (1, juce::roundToInt (3.0f * pixelScale)), narrow);
            boxFilter (juce::jmax (2, juce::roundToInt (13.0f * pixelScale)), broad);
            float weightedDetail = 0.0f, totalWeight = 0.0f;
            for (int row = 0; row < height; ++row)
            {
                const float y = (float (row) + 0.5f) * step - 1.225f;
                for (int col = 0; col < width; ++col)
                {
                    const float x = (float (col) + 0.5f) * step - 1.225f;
                    const size_t i = size_t (row * width + col);
                    const float weight = ease ((x - 0.08f) / 0.16f) * ease ((y - 0.07f) / 0.16f)
                                       * ease ((0.72f - x * x - y * y) / 0.15f);
                    diffuse[i] = weight;
                    narrow[i] -= broad[i];
                    weightedDetail += weight * narrow[i];
                    totalWeight += weight;
                }
            }
            const float mean = weightedDetail / juce::jmax (1.0f, totalWeight);
            for (int row = 0; row < height; ++row)
            {
                auto* dest = reinterpret_cast<juce::PixelARGB*> (pixels.getLinePointer (row));
                for (int col = 0; col < width; ++col)
                {
                    const size_t i = size_t (row * width + col);
                    if (diffuse[i] == 0.0f)
                        continue;
                    const int red = dest[col].getRed(), green = dest[col].getGreen(), blueChannel = dest[col].getBlue();
                    const int offset = juce::jlimit (-std::min ({ red, green, blueChannel }),
                                                    255 - std::max ({ red, green, blueChannel }),
                                                    juce::roundToInt (255.0f * localPigmentGate * diffuse[i] * (narrow[i] - mean)));
                    dest[col].setARGB (255, juce::uint8 (red + offset), juce::uint8 (green + offset), juce::uint8 (blueChannel + offset));
                }
            }
        }
    }
    {
        juce::Graphics::ScopedSaveState clip (g);
        g.reduceClipRegion (contour);
        g.setOpacity (1.0f);
        g.drawImage (coreField, bounds);
        g.setOpacity (0.16f + 0.67f * light);
        // Transport the cached fine detail without adding texture energy or a second grain layer.
        const float shearX = 0.040f * localPigmentGate * std::sin (7.973f * colourClock + 0.044f);
        const float shearY = 0.035f * localPigmentGate * std::sin (5.946f * colourClock - 1.762f);
        g.drawImageTransformed (grain, juce::AffineTransform::translation (-0.5f * grain.getWidth(), -0.5f * grain.getHeight())
            .sheared (shearX, shearY).scaled (bounds.getWidth() / float (grain.getWidth()))
            .translated (c.x + warmDrift * U, c.y + coolDrift * U));
        g.setOpacity (1.0f);
    }
    drawParticles (g, c, radii, spin, rimTintShift, light);

    // The soft ellipses cross over the folded core at low Width.
    if (stereo > 0.004f)
    {
        const float spread = 1.07f * sphereRadius * width;
        const float sideRadius = 0.655f * sphereRadius;
        const float aspect = 0.496f + 0.53f * (1.0f - width);
        const float feather = (1.0f + 1.35f * (1.0f - stereo)) * U;
        const float alpha = (1.0f + 0.08f * atmosphere) * stereo;
        const auto sideTint = modeColour (0xffa397ed, 0xffaaa0f2, 0xffd7848c);
        const auto frame = juce::Rectangle<float> (2.0f * sideRadius * aspect, 2.0f * sideRadius);
        // Stroke widths stay uniform around the ellipse so its vertical sides remain soft.
        const std::pair<float, float> rimProfile[] { { 31.0f, 0.026f }, { 25.0f, 0.047f }, { 19.0f, 0.090f },
                                                      { 15.0f, 0.135f }, { 11.0f, 0.195f }, { 8.0f, 0.245f },
                                                      { 5.0f, 0.270f } };
        for (const float direction : { -1.0f, 1.0f })
        {
            const auto side = c.translated (direction * spread, 0.0f);
            const auto outline = frame.withCentre (side);
            const float sideX = direction * outline.getWidth() * 0.5f;
            for (const auto [thickness, opacity] : rimProfile)
            {
                const float light = opacity * alpha;
                g.setGradientFill (juce::ColourGradient (sideTint.withAlpha (light), side.translated (sideX, 0.0f),
                                                         sideTint.withAlpha (0.35f * light), side.translated (-sideX, 0.0f), false));
                g.drawEllipse (outline, thickness * feather);
            }

            const float overlap = atmosphere * stereo * (1.0f - ease ((width - 0.47f) / 0.32f));
            if (overlap > 0.004f)
            {
                juce::Graphics::ScopedSaveState clip (g);
                g.reduceClipRegion (contour);
                const auto blue = modeColour (0xff7263f1, 0xff7278ff, 0xffde7779);
                const auto pink = modeColour (0xffcc7fe4, 0xffde82ff, 0xffe4a4a0);
                g.setGradientFill (juce::ColourGradient (blue.withAlpha (0.49f * overlap), c.translated (0.0f, -rk),
                                                         pink.withAlpha (0.46f * overlap), c.translated (0.0f, rk), false));
                g.drawEllipse (outline, 8.0f * U);
            }
        }
    }

    // Palette transitions never leave a bolt visible at zero or positive Speed.
    if (const float ba = boltAmt * (1.0f - bypassAmount); negativeSpeed && ba > 0.0f)
    {
        const auto pivot = c.translated (-0.47f * U, -1.12f * U);
        const auto glyph = juce::Rectangle<float> (23.92f * U, 35.30f * U).withCentre (pivot);
        const auto glyphPoint = [&] (float x, float y)
        {
            return juce::Point<float> (glyph.getX() + x * glyph.getWidth(), glyph.getY() + y * glyph.getHeight());
        };
        juce::Path boltMask;
        boltMask.startNewSubPath (glyphPoint (0.523f, -0.048f));
        boltMask.lineTo (glyphPoint (-0.009f, 0.620f));
        boltMask.lineTo (glyphPoint (0.463f, 0.589f));
        boltMask.lineTo (glyphPoint (0.470f, 1.0f));
        boltMask.lineTo (glyphPoint (0.989f, 0.395f));
        boltMask.lineTo (glyphPoint (0.426f, 0.402f));
        boltMask.closeSubPath();
        g.beginTransparencyLayer (ba);
        juce::DropShadow (violet.withAlpha (0.20f), int (6 * U), {}).drawForPath (g, boltMask);
        juce::ColourGradient boltLight (orbTint (juce::Colour (0xffed6aff)), glyphPoint (0.478f, 0.0f),
                                         orbTint (juce::Colour (0xff718ee0)), glyphPoint (-0.007f, 0.877f), false);
        boltLight.addColour (0.31, orbTint (juce::Colour (0xfff087f8)));
        boltLight.addColour (0.48, orbTint (juce::Colour (0xffe697ff)));
        boltLight.addColour (0.59, orbTint (juce::Colour (0xffad9aff)));
        boltLight.addColour (0.78, orbTint (juce::Colour (0xff7eadeb)));
        auto feather = boltLight;
        feather.multiplyOpacity (0.22f);
        g.setGradientFill (feather);
        g.strokePath (boltMask, juce::PathStrokeType (1.45f * U, juce::PathStrokeType::curved));
        g.setGradientFill (boltLight);
        g.fillPath (boltMask.createPathWithRoundedCorners (0.45f * U));
        g.endTransparencyLayer();
    }
    // Project the handle onto the two straight sides of the rounded guide.
    const auto hp = handlePos();
    const bool dragging = dragAxis != DragAxis::none;
    guide.setTarget (dragging || hoverHandle || hoverLine >= 0 ? 1.0f : 0.0f);
    const float ga = guide.get();
    if (ga > 0.004f)
    {
        const auto gg = guideGeom();
        juce::Path axes;
        axes.startNewSubPath (gg.cornerX, gg.topY);
        axes.lineTo (gg.cornerX, gg.baseY - gg.radius);
        axes.quadraticTo (gg.cornerX, gg.baseY, gg.cornerX + gg.radius, gg.baseY);
        axes.lineTo (gg.rightX, gg.baseY);

        const auto guideColour = lightMode ? text : juce::Colour (0xffc6c7c9);
        g.setColour (guideColour.withAlpha ((dragging || hoverLine >= 0 ? 0.92f : 0.57f) * ga));
        g.strokePath (axes, juce::PathStrokeType (1.8f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        const float dot = 6.0f * U;
        const auto wetDot = juce::Point<float> (gg.cornerX, hp.y);
        const auto widthDot = juce::Point<float> (hp.x, gg.baseY);
        g.setColour (guideColour.withAlpha (0.68f * ga));
        g.fillEllipse (juce::Rectangle<float> (dot, dot).withCentre (wetDot));
        g.fillEllipse (juce::Rectangle<float> (dot, dot).withCentre (widthDot));
        g.setColour ((lightMode ? text : juce::Colours::white).withAlpha (0.97f * ga));
        if (dragAxis == DragAxis::wet || dragAxis == DragAxis::both)
            g.fillRoundedRectangle (juce::Rectangle<float> (8.0f * U, 30.0f * U).withCentre (wetDot), 4.0f * U);
        if (dragAxis == DragAxis::width || dragAxis == DragAxis::both)
            g.fillRoundedRectangle (juce::Rectangle<float> (30.0f * U, 8.0f * U).withCentre (widthDot), 4.0f * U);

        g.setFont (font (12.0f * U, true));
        const auto labelColour = text.withAlpha (0.66f * ga);
        shadowFittedText (g, yName, juce::Rectangle<float> (56.0f * U, 31.0f * U).withCentre ({ gg.cornerX + 9.0f * U, gg.topY - 29.0f * U }),
                          juce::Justification::centred, 2, labelColour);
        shadowFittedText (g, xName, juce::Rectangle<float> (65.0f * U, 32.0f * U).withCentre ({ gg.rightX + 49.0f * U, gg.baseY }),
                          juce::Justification::centred, 2, labelColour);

        const auto wetValue = juce::String (juce::roundToInt (wet * 100.0f)) + "%";
        const auto widthValue = juce::String (juce::roundToInt (width * 100.0f)) + "%";
        g.setFont (font (8.0f * U, true));
        shadowFittedText (g, wetValue, juce::Rectangle<float> (0.0f, hp.y - 6.0f * U, gg.cornerX - 4.0f * U, 12.0f * U),
                          juce::Justification::centredRight, 1, labelColour);
        g.setFont (font (10.0f * U, true));
        shadowFittedText (g, widthValue, juce::Rectangle<float> (42.0f * U, 13.0f * U).withCentre ({ hp.x, gg.baseY + 12.0f * U }),
                          juce::Justification::centred, 1, labelColour);
    }

    // The grey handle turns into the bright double circle while the guides are active.
    const float hr = 14.7f * U;
    const auto handle = juce::Rectangle<float> (2.0f * hr, 2.0f * hr).withCentre (hp);
    juce::Path silhouette;
    silhouette.addEllipse (handle);
    juce::DropShadow (juce::Colours::black.withAlpha (0.58f), juce::roundToInt (4.0f * U),
                      { 0, juce::roundToInt (2.0f * U) }).drawForPath (g, silhouette);
    const auto top = juce::Colour (0xff575857).interpolatedWith (juce::Colour (0xff343435), ga);
    const auto bottom = juce::Colour (0xff5a5a5a).interpolatedWith (juce::Colour (0xff171719), ga);
    g.setGradientFill (juce::ColourGradient (top, hp.x, handle.getY(), bottom, hp.x, handle.getBottom(), false));
    g.fillEllipse (handle);
    g.setColour (juce::Colours::white.withAlpha (0.37f + 0.57f * ga));
    g.drawEllipse (handle, (1.2f + 0.8f * ga) * U);
    g.setColour (juce::Colours::white.withAlpha (0.94f * ga));
    g.fillEllipse (handle.reduced (5.0f * U));
}

void Orb::mouseMove (const juce::MouseEvent& e)
{
    const float U = unit();
    const bool h = e.position.getDistanceFrom (handlePos()) < 20.0f * U;
    const auto gg = guideGeom();
    const float tol = 7.0f * U;
    int hl = -1;
    if (! h)
    {
        if (std::abs (e.position.x - gg.cornerX) < tol && e.position.y > gg.topY && e.position.y < gg.baseY) hl = 0;
        else if (std::abs (e.position.y - gg.baseY) < tol && e.position.x > gg.cornerX && e.position.x < gg.rightX) hl = 1;
    }
    if (h != hoverHandle || hl != hoverLine) { hoverHandle = h; hoverLine = hl; repaint(); }
    const auto projection = projectionAt (e.position);
    setMouseCursor (h || projection != DragAxis::none ? juce::MouseCursor::DraggingHandCursor : juce::MouseCursor::NormalCursor);
}

void Orb::mouseDown (const juce::MouseEvent& e)
{
    dragAxis = e.position.getDistanceFrom (handlePos()) < 20.0f * unit() ? DragAxis::both : projectionAt (e.position);
    if (dragAxis == DragAxis::none)
        return;

    lockFrom = e.position;
    lockX = xNorm;
    lockY = yNorm;
    if (dragAxis != DragAxis::wet)
        xAttach.beginGesture();
    else if (undoManager != nullptr)
        undoManager->beginNewTransaction();
    if (dragAxis != DragAxis::width)
        yAttach.beginGesture();
    if (dragAxis == DragAxis::both)
        mouseDrag (e);
    repaint();
}

void Orb::mouseDrag (const juce::MouseEvent& e)
{
    if (dragAxis == DragAxis::none)
        return;

    const auto track = trackArea();
    const auto delta = e.position - lockFrom;
    const float x = dragAxis == DragAxis::width ? lockX + delta.x / track.getWidth()
                                               : (e.position.x - track.getX()) / track.getWidth();
    const float y = dragAxis == DragAxis::wet ? lockY - delta.y / track.getHeight()
                                             : (track.getBottom() - e.position.y) / track.getHeight();
    float nx = juce::jlimit (0.0f, 1.0f, x);
    float ny = juce::jlimit (0.0f, 1.0f, y);
    if (dragAxis == DragAxis::both && e.mods.isAltDown())
    {
        if (std::abs (delta.x) >= std::abs (delta.y)) ny = lockY;
        else nx = lockX;
    }
    if (dragAxis != DragAxis::wet)
    {
        xNorm = nx;
        xAttach.setValueAsPartOfGesture (xParam.convertFrom0to1 (xNorm));
    }
    if (dragAxis != DragAxis::width)
    {
        yNorm = ny;
        yAttach.setValueAsPartOfGesture (yParam.convertFrom0to1 (yNorm));
    }
    repaint();
}

void Orb::mouseUp (const juce::MouseEvent& e)
{
    if (dragAxis == DragAxis::none)
        return;

    if (dragAxis != DragAxis::wet)
        xAttach.endGesture();
    if (dragAxis != DragAxis::width)
        yAttach.endGesture();
    dragAxis = DragAxis::none;
    mouseMove (e);
    repaint();
}

void Orb::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.position.getDistanceFrom (handlePos()) > 20.0f * unit()) return;
    xAttach.setValueAsCompleteGesture (xParam.convertFrom0to1 (xParam.getDefaultValue()));
    yAttach.setValueAsCompleteGesture (yParam.convertFrom0to1 (yParam.getDefaultValue()));
}
