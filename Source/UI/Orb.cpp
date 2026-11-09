#include "Orb.h"


// Angle on the circle for a pitch class (C at 12 o'clock, clockwise).
static float pcAngle (float midi) { return juce::MathConstants<float>::twoPi * std::fmod (midi, 12.0f) / 12.0f; }

// Particle colour by angle around the core (0 = top, clockwise): violet -> pink -> blush -> cyan -> blue.
static juce::Colour coreColourAt (float a)
{
    static const std::pair<float, juce::Colour> stops[] {
        { 0.0f, juce::Colour (0xffa86bff) }, { 0.20f, juce::Colour (0xffff6fd8) }, { 0.39f, juce::Colour (0xffffc3dc) },
        { 0.56f, juce::Colour (0xff9fe9ff) }, { 0.67f, juce::Colour (0xff3fc8ff) }, { 0.83f, juce::Colour (0xff6e7dff) },
        { 1.0f, juce::Colour (0xffa86bff) } };
    const float u = std::fmod (a / juce::MathConstants<float>::twoPi + 1.0f, 1.0f);
    for (size_t i = 1; i < std::size (stops); ++i)
        if (u <= stops[i].first)
            return theme::recolour (stops[i - 1].second.interpolatedWith (stops[i].second, (u - stops[i - 1].first) / (stops[i].first - stops[i - 1].first)));
    return theme::recolour (stops[0].second);
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

juce::Rectangle<float> Orb::padArea() const
{
    return getLocalBounds().toFloat().reduced (18.0f * unit());
}

juce::Rectangle<float> Orb::trackArea() const
{
    // The handle travels the full L: vertical line top (room for the Dry/Wet label) down to the
    // baseline, and across to the horizontal line's end. The axis labels are drawn on top of the ball.
    const auto pad = padArea();
    const float top = pad.getY() + 36.0f, right = pad.getRight() - 30.0f;
    return { pad.getX(), top, right - pad.getX(), pad.getBottom() - top };
}

juce::Point<float> Orb::handlePos() const
{
    const auto t = trackArea();
    return { t.getX() + xNorm * t.getWidth(), t.getBottom() - yNorm * t.getHeight() };
}

bool Orb::hitTest (int x, int y)
{
    // Only the sphere and the handle take the mouse: the empty stage lets clicks through.
    const juce::Point<float> p { float (x), float (y) };
    return p.getDistanceFrom (centre()) < (kSphereR + 4.0f) * unit() || p.getDistanceFrom (handlePos()) < 20.0f * unit();
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
        const float r = radius * (1.0f + 0.030f * std::sin (3.0f * th + 0.70f * t)
                                        + 0.018f * std::sin (5.0f * th - 1.10f * t + 1.0f)
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
        juce::ColourGradient body (juce::Colour (0xff0f0e12), c.x, c.y, juce::Colour (0xff302c3a), c.x + Rs, c.y, true);
        body.addColour (0.55, juce::Colour (0xff17151c));
        body.addColour (0.85, juce::Colour (0xff24212c));
        g.setGradientFill (body);
        g.fillEllipse (c.x - Rs, c.y - Rs, 2 * Rs, 2 * Rs);
        // Lit edge: a feathered band, not a stroke.
        juce::ColourGradient edge (juce::Colours::transparentWhite, c.x, c.y, juce::Colours::transparentWhite, c.x + Rs * 1.06f, c.y, true);
        edge.addColour (0.82, juce::Colour (0x00c9bfe6));
        edge.addColour (0.92, juce::Colour (0x26c9bfe6));
        edge.addColour (0.945, juce::Colour (0x40ddd6f2));
        edge.addColour (0.985, juce::Colour (0x12c9bfe6));
        g.setGradientFill (edge);
        g.fillEllipse (c.x - Rs * 1.06f, c.y - Rs * 1.06f, Rs * 2.12f, Rs * 2.12f);
        juce::Random rng (11);
        for (int i = 0; i < 12000; ++i) // smoke grain, denser toward the edge
        {
            const float a = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            const float r = Rs * std::pow (rng.nextFloat(), 0.4f);
            g.setColour (juce::Colour (0xffd8d0f0).withAlpha (0.012f + 0.045f * std::pow (r / Rs, 3.0f) * rng.nextFloat()));
            g.fillRect (c.x + r * std::sin (a), c.y - r * std::cos (a), U, U);
        }
    }

    // Haze: a faint particle band hugging the core, drawn rotated each frame.
    const float Rh = kHazeR * U;
    const int side = juce::jmax (1, int (Rh * 2.2f * sc));
    core = juce::Image (juce::Image::ARGB, side, side, true);
    {
        juce::Graphics g (core);
        g.addTransform (juce::AffineTransform::scale (sc));
        const juce::Point<float> k (Rh * 1.1f, Rh * 1.1f);
        const float r0 = kCoreR * U * 0.98f;
        juce::Random rng (5);
        for (int i = 0; i < 44000; ++i)
        {
            const float a = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            const float u = std::pow (rng.nextFloat(), 1.9f); // most grains near the contour
            const float r = r0 + (Rh - r0) * u;
            const float alpha = (0.05f + 0.46f * (1.0f - u)) * (0.3f + 0.7f * rng.nextFloat());
            const float sz = (0.40f + rng.nextFloat() * 0.75f) * U;
            // Brighter, whiter grains hug the contour; the outer veil stays saturated and dim.
            const float whiteMix = 0.38f + 0.42f * (1.0f - u) * rng.nextFloat();
            g.setColour (coreColourAt (a).interpolatedWith (juce::Colours::white, whiteMix).withAlpha (alpha));
            g.fillRect (k.x + r * std::sin (a), k.y - r * std::cos (a), sz, sz);
        }
        // A few bright embers scattered just inside the contour for sparkle.
        for (int i = 0; i < 900; ++i)
        {
            const float a = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            const float r = r0 * (0.72f + 0.26f * rng.nextFloat());
            const float sz = (0.5f + rng.nextFloat() * 1.1f) * U;
            g.setColour (coreColourAt (a).interpolatedWith (juce::Colours::white, 0.7f).withAlpha (0.10f + 0.45f * rng.nextFloat()));
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

    // 1. Atmosphere: breathes at rest, swells and clears with the voice.
    const float grow = 0.90f + 0.025f * breath + 0.11f * a;
    {
        g.setOpacity (0.50f + 0.08f * breath + 0.42f * a);
        g.drawImageTransformed (sphere, juce::AffineTransform::scale (float (getWidth()) / float (sphere.getWidth()),
                                                                       float (getHeight()) / float (sphere.getHeight()))
                                            .scaled (grow, grow, c.x, c.y));
        g.setOpacity (1.0f);
        // Outer bloom: low and wide.
        // Outer bloom: a wide nebula haze with a long, soft falloff.
        const float rb = (kSphereR + 64.0f) * U * grow;
        juce::ColourGradient bloom (violet.withAlpha (0.0f), c.x, c.y, violet.withAlpha (0.0f), c.x + rb, c.y, true);
        bloom.addColour (0.50, theme::recolour (juce::Colour (0xff6f5eb0)).withAlpha (0.04f + 0.02f * breath + 0.08f * a));
        bloom.addColour (0.72, theme::recolour (juce::Colour (0xff9a86e8)).withAlpha (0.12f + 0.06f * breath + 0.20f * a));
        bloom.addColour (0.86, theme::recolour (juce::Colour (0xffb7a4ff)).withAlpha (0.07f + 0.04f * breath + 0.14f * a));
        g.setGradientFill (bloom);
        g.fillEllipse (c.x - rb, c.y - rb, 2 * rb, 2 * rb);
        // Reflection: a soft arc drifting over the upper edge, like light sliding across glass.
        const float rr = kSphereR * U * grow * 0.90f;
        const float mid = -0.55f + 0.35f * std::sin (t * 0.13f);
        for (int i = 0; i < 3; ++i)
        {
            juce::Path arc;
            const float span = 0.55f - 0.15f * float (i);
            arc.addCentredArc (c.x, c.y, rr, rr, 0.0f, mid - span, mid + span, true);
            g.setColour (juce::Colours::white.withAlpha (0.035f + 0.03f * float (i)));
            g.strokePath (arc, juce::PathStrokeType ((7.0f - 2.0f * float (i)) * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // 2. Orbit: fine ticks with a comet highlight (and its tail) running clockwise.
    {
        const float ro = kOrbitR * U * grow;
        constexpr int ticks = 90;
        const float comet = t * (0.45f + 0.9f * a);
        for (int i = 0; i < ticks; ++i)
        {
            const float th = juce::MathConstants<float>::twoPi * float (i) / float (ticks) + t * 0.04f;
            float behind = std::fmod (comet - th, juce::MathConstants<float>::twoPi);
            if (behind < 0) behind += juce::MathConstants<float>::twoPi;
            const float tail = std::exp (-behind * 2.6f); // 1 at the head, fading behind it
            const juce::Point<float> d (std::sin (th), -std::cos (th));
            g.setColour (lavender.interpolatedWith (cyan, tail * 0.5f).withAlpha (0.07f + 0.08f * a + 0.7f * tail));
            g.drawLine ({ c + d * (ro - 2.5f * U), c + d * (ro + 2.5f * U) }, (1.1f + 0.8f * tail) * U);
        }
    }

    // 3. Field: a bright violet ring with a wide luminous bloom, a crisp core and a glassy highlight.
    {
        const float rm = kFieldR * U * (0.97f + 0.03f * grow);
        const float pulse = 0.5f + 0.5f * std::sin (t * 1.3f + 0.8f);
        const float lum = 0.75f + 0.25f * pulse + 0.40f * a;
        const juce::Colour glowCol = theme::recolour (juce::Colour (0xff8f6bff));
        // Outer-to-inner halo: widening soft strokes stacked for an additive bloom around the ring.
        for (int i = 5; i >= 1; --i)
        {
            const float w = float (i) * 3.6f * U;
            g.setColour (glowCol.withAlpha ((0.055f + 0.03f * pulse + 0.10f * a) * (0.3f + 0.7f / float (i)) * lum));
            g.drawEllipse (c.x - rm, c.y - rm, 2 * rm, 2 * rm, w);
        }
        // Crisp ring: a bright gradient core, lit top-left to a deeper violet bottom-right.
        juce::ColourGradient ring (theme::recolour (juce::Colour (0xffeadfff)), c.x - rm * 0.7f, c.y - rm * 0.7f,
                                   theme::recolour (juce::Colour (0xff6f4fe6)), c.x + rm * 0.7f, c.y + rm * 0.85f, false);
        ring.addColour (0.5, theme::recolour (juce::Colour (0xffc3acff)));
        g.setGradientFill (ring);
        g.setOpacity (juce::jmin (1.0f, 0.82f + 0.18f * lum));
        g.drawEllipse (c.x - rm, c.y - rm, 2 * rm, 2 * rm, 2.8f * U);
        // Thin inner highlight: a glassy sheen on the ring's upper edge.
        g.setColour (juce::Colours::white.withAlpha (0.28f * lum));
        const float ri = rm - 1.6f * U;
        g.drawEllipse (c.x - ri, c.y - ri, 2 * ri, 2 * ri, 0.9f * U);
        g.setOpacity (1.0f);
    }

    // 4. Haze: spinning particle band around the core.
    {
        const float w = float (core.getWidth()), h = float (core.getHeight());
        const float sc = kHazeR * U * 2.2f / w; // image pixels -> component units
        g.setOpacity (0.55f + 0.45f * a);
        g.drawImageTransformed (core, juce::AffineTransform::translation (-w * 0.5f, -h * 0.5f)
                                          .scaled (sc * (1.0f + 0.04f * a))
                                          .rotated (coreAngle + spin)
                                          .translated (c.x, c.y));
        g.setOpacity (1.0f);
    }

    // 5. Core: swirling plum/teal sheen inside an organic, luminous contour.
    {
        const float rk = kCoreR * U * (1.0f + 0.05f * a);
        const auto contour = coreContour (c, rk);
        const float glow = 0.80f + 0.10f * std::sin (t * 2.1f) + 0.30f * a; // luminosity flicker
        {
            juce::Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (contour);
            g.setColour (juce::Colour (0xff160f20));
            g.fillEllipse (c.x - rk * 1.2f, c.y - rk * 1.2f, rk * 2.4f, rk * 2.4f);
            const float sw = t * 0.22f + coreAngle; // the sheen swirls and leans toward the sung note
            auto blob = [&] (float ang, float dist, float r, juce::Colour col) {
                const juce::Point<float> at (c.x + rk * dist * std::sin (ang), c.y - rk * dist * std::cos (ang));
                g.setGradientFill (juce::ColourGradient (col.withMultipliedAlpha (glow), at.x, at.y, col.withAlpha (0.0f), at.x + r, at.y, true));
                g.fillEllipse (at.x - r, at.y - r, 2 * r, 2 * r);
            };
            blob (sw, 0.42f, rk * 1.05f, theme::recolour (juce::Colour (0xb0802a9c)));         // plum
            blob (sw + 2.3f, 0.48f, rk * 0.95f, theme::recolour (juce::Colour (0x9a1c8c9c)));  // teal
            blob (sw + 4.3f, 0.35f, rk * 0.85f, theme::recolour (juce::Colour (0x8a4436d6)));  // violet
            blob (sw + 1.2f, 0.55f, rk * 0.62f, theme::recolour (juce::Colour (0x7aec4899)));  // hot-pink light leak
            blob (sw + 3.4f, 0.72f, rk * 0.70f, theme::recolour (juce::Colour (0x886fd0ff)));  // cool blue rim light
            // Bright nebula heart: a soft white-lavender glow blooming from the centre.
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffeae4ff).withMultipliedAlpha (0.42f * glow), c.x, c.y - rk * 0.08f,
                                                     juce::Colours::transparentWhite, c.x + rk * 0.9f, c.y, true));
            g.fillEllipse (c.x - rk, c.y - rk, 2 * rk, 2 * rk);
            // Glass depth: darker toward the rim, a faint sheen high on one side.
            juce::ColourGradient depth (juce::Colours::transparentBlack, c.x, c.y, juce::Colours::black.withAlpha (0.45f), c.x + rk, c.y, true);
            g.setGradientFill (depth);
            g.fillEllipse (c.x - rk * 1.2f, c.y - rk * 1.2f, rk * 2.4f, rk * 2.4f);
            g.setColour (juce::Colours::white.withAlpha (0.05f));
            g.fillEllipse (c.x - rk * 0.55f, c.y - rk * 0.85f, rk * 0.9f, rk * 0.45f);
        }
        // Contour: three soft glow passes (wide -> tight), then a crisp line shading icy cyan -> white-lavender.
        g.setColour (lavender.withAlpha (0.055f * glow));
        g.strokePath (contour, juce::PathStrokeType (16.0f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (lavender.withAlpha (0.10f * glow));
        g.strokePath (contour, juce::PathStrokeType (9.0f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (juce::Colour (0xffd4caff).withAlpha (0.20f * glow));
        g.strokePath (contour, juce::PathStrokeType (4.5f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        juce::ColourGradient line (juce::Colour (0xff8fe6ff), c.x - rk, c.y + rk, juce::Colour (0xfff4ecff), c.x + rk, c.y - rk, false);
        line.addColour (0.75, juce::Colour (0xffffd9f2));
        g.setGradientFill (line);
        g.setOpacity (juce::jmin (1.0f, 0.75f + 0.25f * glow));
        g.strokePath (contour, juce::PathStrokeType (1.8f * U, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setOpacity (1.0f);
    }

    // 6. Symbol: crisp white bolt, shown only while a voice is detected (flickers with the level).
    if (const float ba = boltAmt; ba > 0.0f)
    {
        const float sc = 0.85f + 0.15f * ba; // small pop as it lights up
        const auto bp = bolt (juce::Rectangle<float> (19.0f * U * sc, 27.0f * U * sc).withCentre (c));
        g.beginTransparencyLayer (ba);
        // Heavy bloom: a wide violet glow, then a tighter white one, then the crisp symbol on top.
        juce::DropShadow (violet.withAlpha (0.55f + 0.3f * a), int (18 * U), {}).drawForPath (g, bp);
        juce::DropShadow (juce::Colours::white.withAlpha (0.45f + 0.2f * a), int (8 * U), {}).drawForPath (g, bp);
        g.setColour (juce::Colours::white);
        g.fillPath (bp);
        g.endTransparencyLayer();
    }
    if (bypassed)
    {
        g.setFont (font (16.0f * U, true));
        shadowText (g, "BYPASSED", juce::Rectangle<float> (140.0f * U, 24.0f * U).withCentre (c.translated (0, (kCoreR + 30.0f) * U)), juce::Justification::centred, text.withAlpha (0.6f));
    }


    // Doubler guides: an L of axis lines with markers that follow the ball.
    const auto hp = handlePos();
    guide.setTarget (dragging || hoverHandle ? 1.0f : 0.0f);
    if (const float ga = guide; ga > 0.0f)
    {
        const auto pad = padArea();
        const float x0 = pad.getX(), y1 = pad.getBottom(), yTop = pad.getY() + 36.0f, xEnd = pad.getRight() - 30.0f;
        g.setColour (juce::Colours::white.withAlpha (0.35f * ga));
        g.drawLine (x0, yTop, x0, y1, 1.5f);
        g.drawLine (x0, y1, xEnd, y1, 1.5f);
        g.setColour (juce::Colours::white.withAlpha (0.8f * ga));
        g.fillEllipse (x0 - 3.5f, hp.y - 3.5f, 7.0f, 7.0f);
        g.fillEllipse (hp.x - 3.5f, y1 - 3.5f, 7.0f, 7.0f);
    }

    // XY handle: grey ball.
    const float hr = (15.0f + 1.5f * guide.get()) * U;
    juce::Path hpath;
    hpath.addEllipse (hp.x - hr, hp.y - hr, 2 * hr, 2 * hr);
    juce::DropShadow (juce::Colours::black.withAlpha (0.7f), int (10 * U), { 0, int (3 * U) }).drawForPath (g, hpath);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff8f8f94).brighter (0.15f * guide.get()), hp.x, hp.y - hr, juce::Colour (0xff47474b), hp.x, hp.y + hr, false));
    g.fillPath (hpath);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.strokePath (hpath, juce::PathStrokeType (1.0f));

    // Doubler axis labels, drawn on top of the ball so it passes behind the text instead of over it.
    if (const float ga = guide; ga > 0.0f)
    {
        const auto pad = padArea();
        const float x0 = pad.getX(), y1 = pad.getBottom(), yTop = pad.getY() + 36.0f, xEnd = pad.getRight() - 30.0f;
        g.setFont (font (12.0f, true));
        // Brighter text with a strong shadow so it stays legible when the grey ball slides behind it.
        shadowFittedText (g, yName, juce::Rectangle<float> (juce::jmax (2.0f, x0 - 10.0f), yTop - 46.0f, 110.0f, 30.0f),
                          juce::Justification::centredLeft, 2, text.withAlpha (ga), 1.0f);
        shadowFittedText (g, xName, juce::Rectangle<float> (xEnd - 45.0f, y1 + 2.0f, 90.0f, 30.0f), juce::Justification::centred, 2, text.withAlpha (ga), 1.0f);
    }

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
    const bool h = e.position.getDistanceFrom (handlePos()) < 20.0f * unit();
    if (h != hoverHandle) { hoverHandle = h; repaint(); }
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
