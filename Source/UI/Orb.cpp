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
    : xParam (x), yParam (y), xName (std::move (xn)), yName (std::move (yn)),
      xAttach (x, [this] (float v) { xNorm = xParam.convertTo0to1 (v); repaint(); }, um),
      // Only X opens an undo transaction: one XY drag = one undo step (Y joins it).
      yAttach (y, [this] (float v) { yNorm = yParam.convertTo0to1 (v); repaint(); }, nullptr)
{
    xAttach.sendInitialUpdate();
    yAttach.sendInitialUpdate();
    setRepaintsOnMouseActivity (false);
}

void Orb::setDisplay (const DisplayState& s, bool byp)
{
    state = s;
    bypassed = byp;
    const float target = byp ? 0.0f : s.presence * (0.3f + 0.7f * juce::jlimit (0.0f, 1.0f, s.level));
    act += (target - act) * 0.15f;
    // Bolt: on only while voiced, brightness following the raw level so it flickers syllable by syllable.
    const float boltTarget = (s.voiced && ! byp) ? juce::jlimit (0.35f, 1.0f, 0.35f + 1.3f * s.level) : 0.0f;
    boltAmt += (boltTarget - boltAmt) * (boltTarget > boltAmt ? 0.65f : 0.3f);
    if (boltAmt < 0.005f) boltAmt = 0.0f;
    t += 1.0f / 60.0f;
    if (t > 3600.0f) t -= 3600.0f;
    spin += 0.006f + 0.03f * act; // the haze keeps turning; singing speeds it up
    if (spin > juce::MathConstants<float>::twoPi) spin -= juce::MathConstants<float>::twoPi;
    if (s.voiced && ! byp)
    {
        // Ease toward the output pitch class along the shortest way round.
        float d = pcAngle (s.outputMidi) - coreAngle;
        d -= juce::MathConstants<float>::twoPi * std::round (d / juce::MathConstants<float>::twoPi);
        coreAngle += d * 0.18f;
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

bool Orb::hitTest (int x, int y)
{
    // The sphere, the handle and the two Doubler guide lines take the mouse; the rest of the stage lets clicks through.
    const juce::Point<float> p { float (x), float (y) };
    const float U = unit();
    if (p.getDistanceFrom (centre()) < (kSphereR + 4.0f) * U || p.getDistanceFrom (handlePos()) < 20.0f * U)
        return true;
    const auto gg = guideGeom();
    const float tol = 7.0f * U;
    const bool onV = std::abs (p.x - gg.cornerX) < tol && p.y > gg.topY && p.y < gg.baseY + tol;
    const bool onH = std::abs (p.y - gg.baseY) < tol && p.x > gg.cornerX - tol && p.x < gg.rightX;
    return onV || onH;
}

juce::Path Orb::coreContour (juce::Point<float> c, float radius) const
{
    // Organic outline: a few slow waves at unrelated rates never line up the same way twice;
    // the fastest, finest one only appears with the voice.
    juce::Path p;
    constexpr int n = 144;
    for (int i = 0; i <= n; ++i)
    {
        const float th = juce::MathConstants<float>::twoPi * float (i) / float (n);
        const float r = radius * (1.0f + 0.018f * std::sin (3.0f * th + 0.70f * t)
                                        + 0.013f * std::sin (5.0f * th - 1.10f * t + 1.0f)
                                        + 0.010f * std::sin (9.0f * th + 1.90f * t + 2.3f)
                                        + 0.016f * act * std::sin (11.0f * th + 3.2f * t));
        const juce::Point<float> pt (c.x + r * std::sin (th), c.y - r * std::cos (th));
        if (i == 0) p.startNewSubPath (pt); else p.lineTo (pt);
    }
    p.closeSubPath();
    return p;
}

void Orb::renderImages()
{
    const float sc = juce::Component::getApproximateScaleFactorForComponent (this);
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
}

void Orb::paint (juce::Graphics& g)
{
    using namespace theme;
    if (sphere.isNull()) renderImages();
    const float U = unit();
    const auto c = centre();
    const float a = bypassed ? 0.0f : act;
    const float breath = 0.5f + 0.5f * std::sin (t * 0.55f); // slow atmospheric pulse, 0..1

    // The rim stays clearly defined as the glass breathes with the voice.
    const float grow = 0.98f + 0.012f * breath + 0.025f * a;
    g.setOpacity (bypassed ? 0.52f : 0.9f + 0.1f * a);
    g.drawImageTransformed (sphere, juce::AffineTransform::scale (float (getWidth()) / float (sphere.getWidth()),
                                                                   float (getHeight()) / float (sphere.getHeight()))
                                        .scaled (grow, grow, c.x, c.y));
    g.setOpacity (1.0f);

    // A single radial profile keeps the luminous violet crest smooth at every editor scale.
    const float rm = kFieldR * U * grow;
    const float pulse = 0.87f + 0.07f * std::sin (t * 1.3f) + 0.09f * a;
    const auto field = orbTint (juce::Colour (0xff9f7bff));
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


    // Swirling plum and teal lighting beneath the particle contour.
    {
        const float rk = kCoreR * U * (1.0f + 0.05f * a);
        const auto contour = coreContour (c, rk);
        const float glow = 0.80f + 0.10f * std::sin (t * 2.1f) + 0.30f * a; // luminosity flicker
        {
            juce::Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (contour);
            g.setColour (juce::Colour (0xff160f20));
            g.fillEllipse (c.x - rk * 1.2f, c.y - rk * 1.2f, rk * 2.4f, rk * 2.4f);
            const float sw = -1.45f + 0.20f * std::sin (t * 0.40f + coreAngle); // the sheen swirls and leans toward the sung note
            auto blob = [&] (float ang, float dist, float r, juce::Colour col) {
                const juce::Point<float> at (c.x + rk * dist * std::sin (ang), c.y - rk * dist * std::cos (ang));
                g.setGradientFill (juce::ColourGradient (col.withMultipliedAlpha (glow), at.x, at.y, col.withAlpha (0.0f), at.x + r, at.y, true));
                g.fillEllipse (at.x - r, at.y - r, 2 * r, 2 * r);
            };
            blob (sw + 0.16f, 0.57f, rk * 1.24f, orbTint (juce::Colour (0xe8b43ac1)));
            blob (sw + 4.0f, 0.72f, rk * 1.12f, orbTint (juce::Colour (0xbf267da8)));
            blob (sw + 2.06f, 0.77f, rk * 0.90f, orbTint (juce::Colour (0xe4ffb5b9)));
            blob (sw + 3.0f, 0.53f, rk * 0.72f, orbTint (juce::Colour (0x8063c7e4)));

            const float drift = 0.08f * std::sin (0.7f * t + coreAngle);
            const auto point = [&] (float x, float y) { return c.translated (rk * (x + drift), rk * y); };
            juce::Path shade;
            shade.startNewSubPath (point (1.16f, -0.57f));
            shade.cubicTo (point (1.0f, -0.57f), point (0.54f, -0.28f), point (0.32f, -0.07f));
            shade.cubicTo (point (0.13f, 0.13f), point (0.24f, 0.59f), point (0.35f, 0.99f));
            shade.lineTo (point (1.28f, 1.11f));
            shade.closeSubPath();
            g.setGradientFill (juce::ColourGradient (orbTint (juce::Colour (0x10132738)), point (0.18f, -0.34f),
                                                     orbTint (juce::Colour (0xc9081c25)), point (0.83f, 0.46f), false));
            g.fillPath (shade);
            blob (sw + 3.35f, 0.75f, rk * 0.55f, orbTint (juce::Colour (0x7606131c)));
            // Glass depth: darker toward the rim, a faint sheen high on one side.
            juce::ColourGradient depth (juce::Colours::transparentBlack, c.x, c.y, juce::Colours::black.withAlpha (0.45f), c.x + rk, c.y, true);
            g.setGradientFill (depth);
            g.fillEllipse (c.x - rk * 1.2f, c.y - rk * 1.2f, rk * 2.4f, rk * 2.4f);
        }
        // Soft bloom beneath the particles.
        g.setColour (lavender.withAlpha (0.055f * glow));
        g.strokePath (contour, juce::PathStrokeType (16.0f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (lavender.withAlpha (0.10f * glow));
        g.strokePath (contour, juce::PathStrokeType (9.0f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (juce::Colour (0xffd4caff).withAlpha (0.12f * glow));
        g.strokePath (contour, juce::PathStrokeType (4.5f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Cached, uneven particle contour with a slow pitch-responsive turn.
    {
        const float w = float (core.getWidth()), h = float (core.getHeight());
        const float sc = kHazeR * U * 2.2f / w; // image pixels -> component units
        g.setOpacity (0.72f + 0.28f * a);
        g.drawImageTransformed (core, juce::AffineTransform::translation (-w * 0.5f, -h * 0.5f)
                                          .scaled (sc * (1.0f + 0.04f * a))
                                          .rotated (0.15f * std::sin (spin + coreAngle))
                                          .translated (c.x, c.y));
        g.setOpacity (1.0f);
    }

    // The coloured bolt lights up with the detected voice.
    if (const float ba = boltAmt; ba > 0.0f)
    {
        const float sc = 0.89f + 0.11f * ba; // small pop as it lights up
        juce::Path bp;
        bp.startNewSubPath (0.57f, 0.0f);
        bp.lineTo (0.0f, 0.60f);
        bp.lineTo (0.49f, 0.60f);
        bp.lineTo (0.44f, 1.0f);
        bp.lineTo (1.0f, 0.39f);
        bp.lineTo (0.54f, 0.39f);
        bp.closeSubPath();
        bp.applyTransform (juce::AffineTransform::scale (25.0f * U * sc, 41.0f * U * sc)
                               .translated (c.x - 12.5f * U * sc, c.y - 20.5f * U * sc));
        g.beginTransparencyLayer (ba);
        juce::DropShadow (violet.withAlpha (0.20f), int (6 * U), {}).drawForPath (g, bp);
        juce::ColourGradient boltLight (orbTint (juce::Colour (0xffef76ed)), c.translated (-8.0f * U, -12.0f * U),
                                         orbTint (juce::Colour (0xff78a6f9)), c.translated (7.0f * U, 14.0f * U), false);
        boltLight.addColour (0.55f, orbTint (juce::Colour (0xffb688f4)));
        g.setGradientFill (boltLight);
        g.fillPath (bp);
        g.endTransparencyLayer();
    }
    if (bypassed)
    {
        g.setFont (font (16.0f * U, true));
        shadowText (g, "BYPASSED", juce::Rectangle<float> (140.0f * U, 24.0f * U).withCentre (c.translated (0, (kCoreR + 30.0f) * U)), juce::Justification::centred, text.withAlpha (0.6f));
    }


    // Project the handle onto the two straight sides of the rounded guide.
    const auto hp = handlePos();
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
        g.setColour (guideColour.withAlpha ((hoverLine >= 0 ? 0.72f : 0.53f) * ga));
        g.strokePath (axes, juce::PathStrokeType (1.8f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        const float dot = 6.0f * U;
        g.setColour (guideColour.withAlpha (0.76f * ga));
        g.fillEllipse (juce::Rectangle<float> (dot, dot).withCentre ({ gg.cornerX, hp.y }));
        g.fillEllipse (juce::Rectangle<float> (dot, dot).withCentre ({ hp.x, gg.baseY }));

        g.setFont (font (12.0f * U, true));
        const auto labelColour = text.withAlpha (0.66f * ga);
        shadowFittedText (g, yName, juce::Rectangle<float> (56.0f * U, 31.0f * U).withCentre ({ gg.cornerX + 9.0f * U, gg.topY - 29.0f * U }),
                          juce::Justification::centred, 2, labelColour);
        shadowFittedText (g, xName, juce::Rectangle<float> (65.0f * U, 32.0f * U).withCentre ({ gg.rightX + 49.0f * U, gg.baseY }),
                          juce::Justification::centred, 2, labelColour);
    }

    // Keep the handle subdued while the guides are visible.
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
    g.setColour (juce::Colours::white.withAlpha (0.37f - 0.16f * ga));
    g.drawEllipse (handle, 1.2f * U);


    if (dragging)
    {
        g.setFont (font (12.0f, true));
        const auto label = yParam.getCurrentValueAsText() + "  /  " + xParam.getCurrentValueAsText();
        auto box = juce::Rectangle<float> (120.0f, 22.0f).withCentre (hp.translated (0, hp.y > c.y ? -32.0f : 32.0f));
        box = box.constrainedWithin (getLocalBounds().toFloat());
        g.setColour (bg3.withAlpha (0.92f));
        g.fillRoundedRectangle (box, 5.0f);
        shadowText (g, label, box, juce::Justification::centred, text);
    }
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
    setMouseCursor (h ? juce::MouseCursor::DraggingHandCursor : juce::MouseCursor::NormalCursor);
}

void Orb::mouseDown (const juce::MouseEvent& e)
{
    if (e.position.getDistanceFrom (handlePos()) > 20.0f * unit())
        return;
    dragging = true;
    lockFrom = e.position; lockX = xNorm; lockY = yNorm; // anchor for Alt axis-lock
    xAttach.beginGesture();
    yAttach.beginGesture();
    mouseDrag (e);
}

void Orb::mouseDrag (const juce::MouseEvent& e)
{
    if (! dragging) return;
    const auto t = trackArea();
    float nx = juce::jlimit (0.0f, 1.0f, (e.position.x - t.getX()) / t.getWidth());
    float ny = juce::jlimit (0.0f, 1.0f, (t.getBottom() - e.position.y) / t.getHeight());
    if (e.mods.isAltDown()) // lock to the axis with the larger movement from the grab point
    {
        if (std::abs (e.position.x - lockFrom.x) >= std::abs (e.position.y - lockFrom.y)) ny = lockY;
        else nx = lockX;
    }
    xNorm = nx; yNorm = ny;
    xAttach.setValueAsPartOfGesture (xParam.convertFrom0to1 (xNorm));
    yAttach.setValueAsPartOfGesture (yParam.convertFrom0to1 (yNorm));
    repaint();
}

void Orb::mouseUp (const juce::MouseEvent&)
{
    if (! dragging) return;
    dragging = false;
    xAttach.endGesture();
    yAttach.endGesture();
    repaint();
}

void Orb::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (e.position.getDistanceFrom (handlePos()) > 20.0f * unit()) return;
    xAttach.setValueAsCompleteGesture (xParam.convertFrom0to1 (xParam.getDefaultValue()));
    yAttach.setValueAsCompleteGesture (yParam.convertFrom0to1 (yParam.getDefaultValue()));
}
