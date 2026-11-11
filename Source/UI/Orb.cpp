#include "Orb.h"


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

void Orb::setDisplay (const DisplayState& s, float retuneSpeed, float bypassBlend)
{
    negativeSpeed = retuneSpeed < 0.0f;
    bypassAmount = juce::jlimit (0.0f, 1.0f, bypassBlend);
    splitAmount += ((negativeSpeed ? 1.0f : 0.0f) - splitAmount) * 0.26f;
    if (splitAmount < 0.004f) splitAmount = 0.0f;
    if (splitAmount > 0.996f) splitAmount = 1.0f;

    // Bypass changes the lighting, while pitch and the envelope keep moving.
    const bool voiced = s.voiced;
    const float level = voiced ? juce::jlimit (0.0f, 1.0f, s.level) : 0.0f;
    const float target = voiced ? s.presence * (0.25f + 0.75f * level) : 0.0f;
    act += (target - act) * (target > act ? 0.20f : 0.10f);

    const float attack = juce::jlimit (0.0f, 1.0f, (level - voiceLevel) * 3.4f);
    voiceLevel += (level - voiceLevel) * (voiced ? 0.09f : 0.18f);
    syllable += (attack - syllable) * (attack > syllable ? 0.55f : 0.12f);
    // A small negative Speed barely lights the bolt. Voice attacks lift the steady glow.
    const float depth = juce::jlimit (0.0f, 1.0f, -retuneSpeed / 3.0f);
    const float visibility = depth * (0.16f + 0.84f * depth);
    const float vocalFlash = juce::jmin (1.0f, 0.42f * level + 0.66f * syllable);
    const float boltTarget = visibility * (0.74f + 0.26f * vocalFlash);
    if (negativeSpeed && bypassAmount < 0.999f)
        boltAmt += (boltTarget - boltAmt) * (boltTarget > boltAmt ? 0.55f : 0.22f);
    else
        boltAmt = 0.0f;
    if (boltAmt < 0.01f) boltAmt = 0.0f;

    t += 1.0f / 60.0f;
    if (t > 3600.0f) t -= 3600.0f;
    spin += 0.006f + 0.056f * act + 0.020f * syllable;
    const float turn = juce::MathConstants<float>::twoPi;
    if (spin > turn) spin -= turn;
    if (voiced)
    {
        // Follow pitch changes by the shortest route while the material keeps flowing.
        const float delta = std::remainder (pcAngle (s.outputMidi) - coreAngle, turn);
        coreAngle = std::remainder (coreAngle + delta * 0.16f, turn);
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

juce::Path Orb::coreContour (juce::Point<float> c, float radius, float flow) const
{
    juce::Path p;
    constexpr int samples = 144;
    for (int i = 0; i < samples; ++i)
    {
        const float th = juce::MathConstants<float>::twoPi * float (i) / float (samples);
        const float materialAngle = th - flow;
        const float ripple = 0.024f * std::sin (3.0f * materialAngle + 0.4f)
                           + 0.017f * std::sin (7.0f * materialAngle)
                           + 0.009f * std::sin (13.0f * materialAngle + 1.1f)
                           + act * (0.031f * std::sin (2.0f * materialAngle + t * 4.0f)
                                  + 0.017f * std::sin (5.0f * materialAngle - t * 5.2f));
        const float r = radius * (1.0f + ripple);
        const juce::Point<float> pt (c.x + r * std::sin (th), c.y - r * std::cos (th));
        if (i == 0) p.startNewSubPath (pt); else p.lineTo (pt);
    }
    p.closeSubPath();
    return p;
}

void Orb::renderImages (float sc)
{
    renderedScale = sc;
    const float U = unit();
    const auto c = centre();

    // Atmosphere: smoky glass. Dark centre, a misty violet-grey body, a soft lit edge, fine smoke grain.
    sphere = juce::Image (juce::Image::ARGB, juce::jmax (1, int (getWidth() * sc)), juce::jmax (1, int (getHeight() * sc)), true);
    {
        juce::Graphics g (sphere);
        g.addTransform (juce::AffineTransform::scale (sc));
        const float Rs = kSphereR * U;
        juce::ColourGradient body (juce::Colour (0xff111114), c.x, c.y, juce::Colour (0xff3b3a3e), c.x + Rs, c.y, true);
        body.addColour (0.55, juce::Colour (0xff161619));
        body.addColour (0.85, juce::Colour (0xff222224));
        g.setGradientFill (body);
        g.fillEllipse (c.x - Rs, c.y - Rs, 2 * Rs, 2 * Rs);
        // The broad smoky rim rolls into a thin, pearly reflection.
        juce::ColourGradient edge (juce::Colours::transparentWhite, c.x, c.y, juce::Colours::transparentWhite, c.x + Rs * 1.06f, c.y, true);
        edge.addColour (0.69, juce::Colour (0x00dfdce4));
        edge.addColour (0.84, juce::Colour (0x19dfdce4));
        edge.addColour (0.91, juce::Colour (0x61e1dfe7));
        edge.addColour (0.934, juce::Colour (0xc3eeecf3));
        edge.addColour (0.942, juce::Colour (0xeefbf7ff));
        edge.addColour (0.951, juce::Colour (0x30eeecf3));
        edge.addColour (0.968, juce::Colours::transparentWhite);
        g.setGradientFill (edge);
        g.fillEllipse (c.x - Rs * 1.06f, c.y - Rs * 1.06f, Rs * 2.12f, Rs * 2.12f);
        juce::Random rng (11);
        for (int i = 0; i < 28000; ++i) // smoke grain, denser toward the edge
        {
            const float a = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            const float r = Rs * std::pow (rng.nextFloat(), 0.4f);
            const float cloud = 0.60f + 0.24f * std::sin (11.0f * a + 16.0f * r / Rs) + 0.16f * std::sin (23.0f * a - 6.0f * r / Rs);
            g.setColour (juce::Colour (0xffe9e5ee).withAlpha (0.009f + 0.23f * cloud * std::pow (r / Rs, 6.0f) * rng.nextFloat()));
            g.fillRect (c.x + r * std::sin (a), c.y - r * std::cos (a), U, U);
        }
        juce::Image::BitmapData pixels (sphere, juce::Image::BitmapData::readWrite);
        const auto centrePx = c * sc;
        const float radiusPx = Rs * sc;
        const auto rim = juce::Rectangle<float> (2.0f * radiusPx, 2.0f * radiusPx).withCentre (centrePx)
                             .getSmallestIntegerContainer().getIntersection (sphere.getBounds());
        for (int y = rim.getY(); y < rim.getBottom(); ++y)
            for (int x = rim.getX(); x < rim.getRight(); ++x)
            {
                const auto offset = (juce::Point<float> (float (x), float (y)) - centrePx) / radiusPx;
                const float distance = offset.getDistanceFromOrigin();
                if (distance < 0.73f || distance > 1.025f) continue;
                const float rimWeight = juce::jlimit (0.0f, 1.0f, (distance - 0.73f) * 4.8f);
                const float noise = theme::surfaceNoise (float (x) / sc, float (y) / sc);
                const float angle = std::atan2 (offset.y, offset.x);
                const float reflection = 0.018f - 0.14f * offset.y - 0.072f * offset.x
                                         + 0.08f * std::cos (3.0f * angle + 1.1f);
                const float grain = 0.54f * noise + 0.32f * (rng.nextFloat() - 0.5f);
                const float crest = (distance - 1.0f) * 71.0f;
                const float strength = (reflection + grain - 0.06f * std::exp (-crest * crest)) * rimWeight;
                const auto shade = strength >= 0.0f ? juce::Colour (0xfff5f2fa) : juce::Colours::black;
                const auto base = pixels.getPixelColour (x, y);
                pixels.setPixelColour (x, y, base.interpolatedWith (shade.withAlpha (base.getFloatAlpha()), std::abs (strength)));
            }
    }

    // Cached energy field and particles that move together with the pitch visualisation.
    const float Rh = kHazeR * U;
    const int side = juce::jmax (1, int (Rh * 2.2f * sc));
    core = juce::Image (juce::Image::ARGB, side, side, true);
    {
        juce::Graphics g (core);
        g.addTransform (juce::AffineTransform::scale (sc));
        const juce::Point<float> k (Rh * 1.1f, Rh * 1.1f);
        const float r0 = kCoreR * U * 0.975f;
        const auto pinkDust = theme::orbTint (juce::Colour (0xffdd7cdb));
        const auto blueDust = theme::orbTint (juce::Colour (0xff42add8));
        const auto warmDust = theme::orbTint (juce::Colour (0xffffc2bd));
        juce::Random rng (5);
        for (int i = 0; i < 20500; ++i)
        {
            const float angle = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            const float radius = r0 * std::sqrt (rng.nextFloat());
            const auto point = k + juce::Point<float> (std::sin (angle), -std::cos (angle)) * radius;
            const auto offset = (point - k) / r0;
            // The warm face catches the light above the curved seam. The cyan underside stays in shadow.
            const float fold = offset.x - (0.18f + 0.38f * offset.y * offset.y - 0.21f * offset.y);
            const float seam = std::exp (-std::abs (fold) * 11.0f) * (0.81f - 0.19f * offset.y);
            const float texture = 0.68f + 0.52f * theme::surfaceNoise (point.x * 2.2f, point.y * 2.2f);
            const float warmth = juce::jlimit (0.0f, 1.0f, -offset.y * 0.79f + offset.x * 0.33f);
            const float cool = juce::jlimit (0.0f, 1.0f, 0.43f + offset.x * 0.57f + offset.y * 0.27f);
            const auto colour = pinkDust.interpolatedWith (blueDust, cool).interpolatedWith (warmDust, warmth);
            g.setColour (colour.withAlpha ((0.18f + 0.40f * seam + 0.23f * warmth) * texture * rng.nextFloat()));
            const float size = (0.35f + 0.83f * rng.nextFloat()) * U;
            g.fillRect (point.x, point.y, size, size);
        }
        for (int i = 0; i < 50000; ++i)
        {
            const float a = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            const float u = std::pow (rng.nextFloat(), 2.3f);
            const float wave = 1.0f + 0.024f * std::sin (3.0f * a + 0.4f)
                                   + 0.017f * std::sin (7.0f * a) + 0.009f * std::sin (13.0f * a + 1.1f);
            const float eddy = 0.50f + 0.34f * std::sin (4.0f * a + 0.6f)
                                    + 0.16f * std::sin (11.0f * a - 0.9f);
            const float width = 0.35f + 0.65f * eddy;
            const float scatter = rng.nextBool() ? 1.0f : -0.56f;
            const float r = r0 * wave + (Rh - r0) * width * u * scatter;
            const float density = 0.70f + 0.30f * eddy + 0.13f * std::cos (23.0f * a + u * 3.1f);
            const float alpha = (0.025f + 0.73f * std::pow (1.0f - u, 3.0f))
                                * (0.26f + 0.74f * rng.nextFloat()) * density;
            const float sz = (0.34f + rng.nextFloat() * 0.69f) * U;
            // The rim has a bright filament and coarser, saturated eddies beyond it.
            const float whiteMix = 0.33f + 0.60f * (1.0f - u) * rng.nextFloat();
            g.setColour (coreColourAt (a).interpolatedWith (juce::Colours::white, whiteMix).withAlpha (alpha));
            g.fillRect (k.x + r * std::sin (a), k.y - r * std::cos (a), sz, sz);
        }
        // A few bright embers scattered just inside the contour for sparkle.
        for (int i = 0; i < 3100; ++i)
        {
            const float a = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            const float r = r0 * (0.72f + 0.26f * rng.nextFloat());
            const float sz = (0.5f + rng.nextFloat() * 1.1f) * U;
            g.setColour (coreColourAt (a).interpolatedWith (juce::Colours::white, 0.7f).withAlpha (0.035f + 0.22f * rng.nextFloat()));
            g.fillEllipse (k.x + r * std::sin (a) - sz * 0.5f, k.y - r * std::cos (a) - sz * 0.5f, sz, sz);
        }
    }

    // Both palettes share the same grain positions, so changing Speed never resets the flow.
    plumCore = core.createCopy();
    juce::Image::BitmapData plumPixels (plumCore, juce::Image::BitmapData::readWrite);
    const auto plum = theme::orbTint (juce::Colour (0xffae7be8));
    for (int y = 0; y < plumPixels.height; ++y)
        for (int x = 0; x < plumPixels.width; ++x)
        {
            const auto source = plumPixels.getPixelColour (x, y);
            if (! source.isTransparent())
                plumPixels.setPixelColour (x, y, juce::Colour::fromHSV (plum.getHue(), source.getSaturation() * 0.71f,
                                                                       source.getBrightness(), source.getFloatAlpha()));
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
    const float breath = 0.5f + 0.5f * std::sin (t * 0.55f);
    const float hit = syllable;
    const float flow = spin + coreAngle;
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
    const float voiceHue = ease (0.48f + 0.66f * std::sin (2.7f * t + 1.3f * coreAngle));
    const float violetShell = stereo * atmosphere * voiceHue;
    const float pearl = atmosphere * (1.0f - 0.88f * violetShell) * (1.0f - 0.67f * bypassAmount);
    if (pearl > 0.004f)
    {
        g.setOpacity (pearl);
        g.drawImageTransformed (sphere, juce::AffineTransform::scale (float (getWidth()) / float (sphere.getWidth()),
                                                                       float (getHeight()) / float (sphere.getHeight()))
                                            .scaled (grow, grow, c.x, c.y));
        g.setOpacity (1.0f);
    }

    // Wet lets the shell take on the moving violet light, while quiet outlines stay pale.
    const float attackWash = juce::jlimit (0.0f, 0.72f, 0.55f * hit + a * (0.22f + 0.19f * std::sin (2.0f * spin)));
    const float glowWash = juce::jlimit (0.0f, 1.0f, 0.50f * (1.0f - atmosphere) + 0.74f * violetShell + 0.28f * attackWash);
    const float wash = glowWash * (1.0f - bypassAmount) + (0.44f + 0.43f * atmosphere) * bypassAmount;
    if (wash > 0.004f)
    {
        const float haloR = sphereRadius * 1.06f;
        const float colouredGlow = juce::jmax (violetShell, bypassAmount);
        const float hue = juce::jlimit (0.0f, 1.0f, 0.9f * violetShell + 0.65f * attackWash);
        const auto tint = juce::Colour (0xffdddbe0)
            .interpolatedWith (orbTint (juce::Colour (0xff9472ed)), hue)
            .interpolatedWith (bypassAccent, bypassAmount);
        juce::ColourGradient rim (tint.withAlpha (0.0f), c, tint.withAlpha (0.0f), c.translated (haloR, 0.0f), true);
        rim.addColour (0.36, tint.withAlpha (0.09f * colouredGlow * wash));
        rim.addColour (0.66, tint.withAlpha (0.22f * colouredGlow * wash));
        rim.addColour (0.85, tint.withAlpha (0.40f * colouredGlow * wash));
        rim.addColour (0.918, tint.withAlpha ((0.16f + 0.48f * colouredGlow) * wash));
        rim.addColour (0.936, tint.withAlpha (0.72f * wash));
        rim.addColour (0.943, tint.withAlpha (wash));
        rim.addColour (0.952, tint.withAlpha (0.33f * wash));
        rim.addColour (0.966, tint.withAlpha (0.03f * wash));
        g.setGradientFill (rim);
        g.fillEllipse (juce::Rectangle<float> (2.0f * haloR, 2.0f * haloR).withCentre (c));
    }

    // The middle ring holds its radius and fades into the stereo effect above about 60% Wet.
    const float rm = kFieldR * U * grow;
    const float fieldFade = ease ((wet - 0.58f) / 0.27f);
    const float pulse = (0.53f + 0.30f * atmosphere + 0.10f * hit + 0.04f * std::sin (t * 1.3f)) * (1.0f - fieldFade);
    const auto field = modeColour (0xff9f7bff, 0xff9f7bff, 0xffe97c7d);
    const float fieldExtent = rm + 14.0f * U;
    juce::ColourGradient profile (field.withAlpha (0.0f), c, field.withAlpha (0.0f),
                                  c.translated (fieldExtent, 0.0f), true);
    const std::pair<float, float> falloff[] { { -12.0f, 0.048f }, { -8.0f, 0.20f }, { -4.9f, 0.43f },
                                             { -2.0f, 0.59f }, { 0.0f, 0.675f }, { 2.0f, 0.60f },
                                             { 4.9f, 0.43f }, { 8.0f, 0.20f }, { 12.0f, 0.048f } };
    for (const auto [distance, opacity] : falloff)
        profile.addColour ((rm + distance * U) / fieldExtent, field.withAlpha (opacity * pulse));
    g.setGradientFill (profile);
    g.fillEllipse (juce::Rectangle<float> (2.0f * fieldExtent, 2.0f * fieldExtent).withCentre (c));


    const float swell = 1.0f + 0.044f * a + 0.024f * hit;
    const float rk = kCoreR * U * swell;
    const auto contour = coreContour (c, rk, flow);
    const float glow = 0.18f + 0.66f * atmosphere + 0.16f * hit;
    const float coreLight = 0.29f + 0.71f * atmosphere;
    const auto bounds = juce::Rectangle<float> (2.3f * rk, 2.3f * rk).withCentre (c);
    const auto rotation = juce::AffineTransform::scale (rk).rotated (flow).translated (c.x, c.y);
    const auto at = [&] (float x, float y) { return juce::Point<float> (x, y).transformedBy (rotation); };
    const auto magenta = modeColour (0xffa253c7, 0xffee46ce, 0xffad5057);
    const auto cyanFace = modeColour (0xff7864d8, 0xff32c9ed, 0xff7d394a);
    const auto blush = modeColour (0xffd8bdf6, 0xffefc4db, 0xfff3bdad);
    const auto warmShadow = modeColour (0xff1a1029, 0xff1a1029, 0xff271217);
    const auto warmEdge = modeColour (0xff17152e, 0xff17152e, 0xff200e13);
    const auto coolShadow = modeColour (0xff18122d, 0xff0e2032, 0xff251419);
    const auto coolEdge = modeColour (0xff21193d, 0xff18223b, 0xff34191e);

    // The two luminous faces share a curved fold. Pitch turns the whole surface,
    // while the vocal envelope changes the bend and the strength of its light.
    const float bend = 0.16f + a * 0.16f * std::sin (t * 4.9f + coreAngle);
    juce::Path seam;
    seam.startNewSubPath (at (-1.2f, -0.57f));
    seam.cubicTo (at (-0.55f, -0.67f), at (-0.48f, bend), at (-0.04f, bend));
    seam.cubicTo (at (0.43f, bend), at (0.65f, -0.55f), at (1.2f, -0.14f));
    auto coolSurface = seam;
    coolSurface.lineTo (at (1.24f, -1.24f));
    coolSurface.lineTo (at (-1.24f, -1.24f));
    coolSurface.closeSubPath();
    {
        juce::Graphics::ScopedSaveState clip (g);
        g.reduceClipRegion (contour);
        const auto warmLight = at (-0.33f, 0.49f);
        g.setGradientFill (juce::ColourGradient (warmShadow.interpolatedWith (magenta, glow),
                                                 warmLight, warmEdge, at (0.62f, -0.92f), true));
        g.fillRect (bounds);

        const auto coolLight = at (0.30f, -0.45f);
        juce::ColourGradient cool (coolShadow.interpolatedWith (cyanFace, glow), coolLight,
                                   coolEdge, at (-0.89f, 0.26f), true);
        g.setGradientFill (cool);
        g.fillPath (coolSurface);

        const auto sheen = at (0.38f, bend - 0.12f);
        g.setGradientFill (juce::ColourGradient (blush.withAlpha (0.26f * glow), sheen,
                                                 blush.withAlpha (0.0f), sheen.translated (rk * 0.9f, 0.0f), true));
        g.fillRect (bounds);
        g.setGradientFill (juce::ColourGradient (cyanFace.withAlpha (0.10f * glow), at (-0.95f, -0.2f),
                                                 blush.withAlpha (0.38f * glow), at (0.91f, -0.18f), false));
        g.strokePath (seam, juce::PathStrokeType (2.3f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    const auto filament = lavender.interpolatedWith (bypassAccent, bypassAmount);
    g.setColour (filament.withAlpha (0.13f * glow));
    g.strokePath (contour, juce::PathStrokeType (9.5f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (filament.withAlpha (0.19f * glow));
    g.strokePath (contour, juce::PathStrokeType (4.8f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // The cached grains follow the same sweep as the fluid surface.
    const float w = float (core.getWidth()), h = float (core.getHeight());
    const float sc = kHazeR * U * 2.2f / w;
    const auto grainTransform = juce::AffineTransform::translation (-w * 0.5f, -h * 0.5f)
                                    .scaled (sc * swell / 0.975f)
                                    .rotated (flow)
                                    .translated (c.x, c.y);
    const float grainOpacity = (0.08f + 0.78f * atmosphere) * (1.0f - 0.66f * bypassAmount);
    if (splitAmount < 1.0f)
    {
        g.setOpacity (grainOpacity * (1.0f - splitAmount));
        g.drawImageTransformed (plumCore, grainTransform);
    }
    if (splitAmount > 0.0f)
    {
        g.setOpacity (grainOpacity * splitAmount);
        g.drawImageTransformed (core, grainTransform);
    }
    g.setOpacity (1.0f);

    // A narrow luminous edge follows the deformation above the scattered grains.
    const auto edgeTop = cyanFace.interpolatedWith (juce::Colours::white, 0.63f).withAlpha ((0.38f + 0.42f * glow) * coreLight);
    const auto edgeBottom = magenta.interpolatedWith (juce::Colours::white, 0.70f).withAlpha ((0.36f + 0.42f * glow) * coreLight);
    g.setGradientFill (juce::ColourGradient (edgeTop, at (0.0f, -1.0f), edgeBottom, at (0.0f, 1.0f), false));
    g.strokePath (contour, juce::PathStrokeType (1.8f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

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
        const float boltScale = 0.89f + 0.11f * ba; // small pop as it lights up
        juce::Path bp;
        bp.startNewSubPath (0.57f, 0.0f);
        bp.lineTo (0.0f, 0.60f);
        bp.lineTo (0.49f, 0.60f);
        bp.lineTo (0.44f, 1.0f);
        bp.lineTo (1.0f, 0.39f);
        bp.lineTo (0.54f, 0.39f);
        bp.closeSubPath();
        bp.applyTransform (juce::AffineTransform::scale (25.0f * U * boltScale, 41.0f * U * boltScale)
                               .translated (c.x - 12.5f * U * boltScale, c.y - 20.5f * U * boltScale));
        g.beginTransparencyLayer (ba);
        juce::DropShadow (violet.withAlpha (0.20f), int (6 * U), {}).drawForPath (g, bp);
        juce::ColourGradient boltLight (orbTint (juce::Colour (0xffef76ed)), c.translated (-8.0f * U, -12.0f * U),
                                         orbTint (juce::Colour (0xff78a6f9)), c.translated (7.0f * U, 14.0f * U), false);
        boltLight.addColour (0.55f, orbTint (juce::Colour (0xffb688f4)));
        g.setGradientFill (boltLight);
        g.fillPath (bp);
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
