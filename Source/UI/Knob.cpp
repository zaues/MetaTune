#include "Knob.h"

static float rad (float deg) { return juce::degreesToRadians (deg); }

// Traced in the canonical reference's knob close-up. Origin at the readout centre.
static juce::AffineTransform speedTransform (juce::Point<float> centre, float radius)
{
    return juce::AffineTransform::translation (-135.0f, -142.0f)
        .scaled (radius / 121.0f).translated (centre.x, centre.y);
}

static const juce::Path& speedOutline()
{
    static const auto outline = juce::Drawable::parseSVGPath (
        "M 247 99 C 229 54 195 27 153 21 C 87 12 29 53 18 117 "
        "C 5 180 39 235 91 254 C 139 273 185 260 217 230 "
        "C 220 228 217 225 216 222 C 195 180 198 137 227 113 "
        "C 232 109 239 105 245 103 C 248 102 248 100 247 99 Z");
    return outline;
}

Knob::Knob (juce::String l, std::function<juce::String (double)> f, Style s)
    : label (std::move (l)), format (std::move (f)), style (s)
{
    setSliderStyle (RotaryHorizontalVerticalDrag);
    setTextBoxStyle (NoTextBox, true, 0, 0);
    setRotaryParameters (rad (s.startDeg) + juce::MathConstants<float>::twoPi, rad (s.endDeg) + juce::MathConstants<float>::twoPi, true);
    setMouseDragSensitivity (220);
    // Holding Shift switches to velocity mode with low sensitivity: fine adjustment.
    setVelocityModeParameters (0.25, 1, 0.0, true, juce::ModifierKeys::shiftModifier);
    setWantsKeyboardFocus (true);
}

bool Knob::hitTest (int x, int y)
{
    const auto b = getLocalBounds().toFloat();
    const juce::Point<float> p { float (x), float (y) };
    if (p.getDistanceFrom (b.getCentre()) > juce::jmin (b.getWidth(), b.getHeight()) * 0.5f)
        return false;
    if (style.bandFrom < 0.0f) return true;
    const float radius = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f - 1.0f;
    return speedOutline().contains (p.transformedBy (speedTransform (b.getCentre(), radius).inverted()));
}

void Knob::setSustainDisplay (float amount, float timeRatio)
{
    if (amount == sustainAmount && std::abs (timeRatio - sustainTimeRatio) < 0.001f)
        return;

    sustainAmount = amount;
    sustainTimeRatio = timeRatio;
    repaint();
}

void Knob::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto bounds = getLocalBounds().toFloat();
    const float R = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 1.0f;
    const auto c = bounds.getCentre();
    const double setting = getValue();
    const bool speed = style.bandFrom >= 0.0f;
    const double displayedValue = speed && setting > 0.0 && ! isMouseButtonDown()
        ? setting * sustainTimeRatio : setting;
    const float t = float (valueToProportionOfLength (displayedValue));
    const auto bypassColour = [this] (juce::Colour normal, juce::Colour warm)
    {
        return normal.interpolatedWith (warm.withAlpha (normal.getFloatAlpha()), bypassBlend);
    };
    const float hv = hover;
    const bool hot = hv > 0.5f || hasKeyboardFocus (false);

    juce::Graphics::ScopedSaveState ss (g);
    juce::Path housing;
    if (speed)
    {
        housing = speedOutline();
        housing.applyTransform (speedTransform (c, R));
        g.reduceClipRegion (housing);
    }
    const bool sustain = style.smoothArc;
    const auto face = lightMode ? knobBody : dark::knobFace;
    const auto bezel = lightMode ? line : dark::knobEdge;
    // Keep the open Speed outline, with a lit shoulder inside its silhouette.
    const float rim = speed ? 0.959f : (sustain ? 0.895f : 0.982f);
    const float rb = R * rim;
    const auto lipTop = sustain ? face.darker (0.25f) : bezel;
    const auto lipBottom = sustain ? face.darker (0.32f) : face.darker (0.17f);
    g.setGradientFill (juce::ColourGradient (lipTop, c.translated (-R * 0.42f, -R),
                                             lipBottom, c.translated (R * 0.52f, R), false));
    if (speed) g.fillPath (housing);
    else g.fillEllipse (c.x - R, c.y - R, 2 * R, 2 * R);

    const auto faceTop = sustain ? face.brighter (0.038f) : face;
    const auto faceBottom = sustain ? face.darker (0.16f) : face.darker (0.025f);
    g.setGradientFill (juce::ColourGradient (faceTop, c.translated (0.0f, -rb),
                                             faceBottom, c.translated (0.0f, rb), false));
    if (speed)
    {
        const auto inset = juce::AffineTransform::scale (rim, rim, c.x, c.y);
        g.fillPath (housing, inset);
        g.setColour (juce::Colours::black.withAlpha (0.24f));
        g.strokePath (housing, juce::PathStrokeType (2.0f));
        g.setGradientFill (juce::ColourGradient (text.withAlpha (0.10f), c.translated (-R, -R),
                                                 text.withAlpha (0.045f), c.translated (R, R), false));
        g.strokePath (housing, juce::PathStrokeType (0.95f), inset);
    }
    else
    {
        g.fillEllipse (c.x - rb, c.y - rb, 2 * rb, 2 * rb);
        g.setGradientFill (juce::ColourGradient (text.withAlpha (sustain ? 0.02f : 0.05f), c.translated (0.0f, -rb),
                                                 text.withAlpha (sustain ? 0.035f : 0.10f), c.translated (0.0f, rb), false));
        g.drawEllipse (c.x - rb, c.y - rb, 2 * rb, 2 * rb, sustain ? 0.75f : 1.2f);
    }

    if (style.ticks)
    {
        const float r1 = R * (speed ? 0.665f : 0.689f), r2 = R * (speed ? 0.799f : 0.806f);
        const float tickW = juce::jmax (1.1f, R * (speed ? 0.0174f : 0.019f));
        const bool band = style.bandFrom >= 0.0f;
        const float tickEnd = band ? style.bandFrom : 1.0f; // proportion where the ticks stop
        const float a0 = rad (speed ? -216.0f : -228.0f), a1 = rad (speed ? 54.0f : 45.0f);
        const float crest = rad (0.5f);
        auto angleAt = [&] (float p)
        {
            if (! speed) return std::lerp (a0, a1, p);
            if (p <= tickEnd) return std::lerp (a0, crest, p / tickEnd);
            return std::lerp (crest, a1, (p - tickEnd) / (1.0f - tickEnd));
        };
        if (speed && setting > 0.0 && std::abs (sustainAmount) > 0.002f)
        {
            const float start = angleAt (float (valueToProportionOfLength (setting)));
            const float end = juce::jlimit (a0, a1, start + rad (108.0f) * sustainAmount);
            const float radius = R * 0.969f;
            const auto ink = bypassColour (tint (juce::Colour (0xffad96e6), meterHue), bypassAccent);
            const float response = juce::jlimit (0.0f, 1.0f,
                std::abs (1.0f - sustainTimeRatio) / (0.97f * std::abs (sustainAmount)));
            juce::Path gauge;
            gauge.addCentredArc (c.x, c.y, radius, radius, 0.0f, juce::jmin (start, end), juce::jmax (start, end), true);
            const juce::PathStrokeType soft (R * 0.034f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
            const juce::PathStrokeType fine (R * 0.015f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
            g.setColour (ink.withAlpha (0.15f));
            g.strokePath (gauge, soft);
            g.setColour (ink.withAlpha (0.67f));
            g.strokePath (gauge, fine);
            if (response > 0.01f)
            {
                const float movingEnd = std::lerp (start, end, response);
                juce::Path activeGauge;
                activeGauge.addCentredArc (c.x, c.y, radius, radius, 0.0f,
                                           juce::jmin (start, movingEnd), juce::jmax (start, movingEnd), true);
                g.setColour (ink.withAlpha (0.16f));
                g.strokePath (activeGauge, soft);
                g.setColour (ink.interpolatedWith (juce::Colours::white, 0.43f).withAlpha (0.61f));
                g.strokePath (activeGauge, fine);
            }
        }
        const int n = speed ? 102 : 127;
        {
            // Glow under the lit ticks (the "box-shadow" of the arc).
            const float litTo = juce::jmin (t, tickEnd);
            if (litTo > 0.0f)
            {
                juce::Path glowArc;
                glowArc.addCentredArc (c.x, c.y, (r1 + r2) * 0.5f, (r1 + r2) * 0.5f, 0.0f, a0, angleAt (litTo), true);
                juce::ColourGradient gg (bypassColour (rose, bypassAccent).withAlpha (0.16f), c.x - R, c.y + R,
                                         bypassColour (lilac, bypassAccent).withAlpha (0.18f), c.x + R, c.y - R, false);
                g.setGradientFill (gg);
                g.strokePath (glowArc, juce::PathStrokeType ((r2 - r1) * 1.42f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
            }
        }
        const auto tickColour = [] (juce::uint32 colour) { return tint (recolour (juce::Colour (colour)), meterHue); };
        juce::ColourGradient tickPalette (tickColour (0xffaf5965), 0.0f, 0.0f,
                                           tickColour (0xfffff8ff), 1.0f, 0.0f, false);
        tickPalette.addColour (0.36, tickColour (0xffc17891));
        tickPalette.addColour (0.61, tickColour (0xffef9fc7));
        tickPalette.addColour (0.81, tickColour (0xfff3b8ef));
        for (int i = 0; i < n; ++i)
        {
            const float u = float (i) / float (n - 1), p = u * tickEnd;
            const float a = angleAt (p);
            const juce::Point<float> dir (std::sin (a), -std::cos (a));
            const auto active = gradientOn ? tint (rose.interpolatedWith (lilac, u).interpolatedWith (juce::Colours::white, 0.45f * u * u), meterHue)
                                           : tickPalette.getColourAtPosition (u);
            const auto base = bypassColour (active, juce::Colour (0xffa4474f).interpolatedWith (bypassAccent, u));
            const bool lit = p <= t + 1e-4f;
            g.setColour (lit ? base.brighter (0.12f * hv) : base.withAlpha (0.17f + 0.11f * hv));
            const float inner = r1 + (speed ? 0.0f : R * 0.021f * u);
            const float outer = r2 + (speed ? 0.0f : R * 0.045f * u);
            g.drawLine ({ c + dir * inner, c + dir * outer }, tickW);
        }
        if (band)
        {
            const float rm = (r1 + r2) * 0.5f, bw = (r2 - r1) * 0.91f;
            const float ab = angleAt (style.bandFrom);
            juce::Path full;
            full.addCentredArc (c.x, c.y, rm, rm, 0.0f, ab, a1, true);
            g.setColour (text.withAlpha (hot ? 0.15f : 0.085f));
            g.strokePath (full, juce::PathStrokeType (bw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour (face);
            g.strokePath (full, juce::PathStrokeType (bw - 1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            if (t > style.bandFrom)
            {
                juce::Path lit;
                lit.addCentredArc (c.x, c.y, rm, rm, 0.0f, ab, angleAt (t), true);
                const juce::Point<float> pa (c.x + rm * std::sin (ab), c.y - rm * std::cos (ab));
                const juce::Point<float> pb (c.x + rm * std::sin (a1), c.y - rm * std::cos (a1));
                juce::ColourGradient grad (bypassColour (tickColour (0xff41c1ff), juce::Colour (0xffb7595d)), pa,
                                           bypassColour (tickColour (0xffffcfda), juce::Colour (0xffffc2bb)), pb, false);
                grad.addColour (0.26, bypassColour (tickColour (0xffa684ff), bypassAccent));
                grad.addColour (0.56, bypassColour (tickColour (0xfffa64f2), juce::Colour (0xffee8987)));
                if (activity > 0.01f) // soft halo under the band while singing
                {
                    const auto glowAlpha = 0.18f * activity;
                    juce::ColourGradient glow (bypassColour (cyan, bypassAccent).withAlpha (glowAlpha), pa,
                                               bypassColour (pink, bypassAccent).withAlpha (glowAlpha), pb, false);
                    g.setGradientFill (glow);
                    g.strokePath (lit, juce::PathStrokeType (bw * (1.0f + 0.18f * activity), juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
                g.setGradientFill (grad);
                g.setOpacity (0.95f + 0.05f * activity);
                g.strokePath (lit, juce::PathStrokeType (bw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setOpacity (1.0f);
            }
            if (t > style.bandFrom) // position marker on the band
            {
                const float at = angleAt (t);
                const juce::Point<float> mp (c.x + rm * std::sin (at), c.y - rm * std::cos (at));
                const float md = bw * 0.62f;
                g.setColour (juce::Colours::white);
                if (hot) g.fillEllipse (mp.x - md, mp.y - md, 2 * md, 2 * md);
            }
            if (t > style.bandFrom)
            {
                const float am = angleAt (style.bandFrom + (1.0f - style.bandFrom) * 0.52f);
                const juce::Point<float> bp (c.x + rm * std::sin (am), c.y - rm * std::cos (am));
                g.setColour (juce::Colour (0xff21102d));
                g.fillPath (bolt (juce::Rectangle<float> (bw * 0.81f, bw * 0.81f).withCentre (bp)));
            }
        }
    }

    if (style.smoothArc)
    {
        // Slim, smooth progress arc (bipolar: fills out from the 12-o'clock centre both ways).
        const float rm = R * 0.822f, bw = juce::jmax (2.3f, R * 0.054f);
        const float a0 = rad (style.startDeg), a1 = rad (style.endDeg);
        auto ang = [&] (float p) { return a0 + (a1 - a0) * p; };
        juce::Path track;
        track.addCentredArc (c.x, c.y, rm, rm, 0.0f, a0, a1, true);
        g.setColour (juce::Colours::white.withAlpha (hot ? 0.10f : 0.043f));
        g.strokePath (track, juce::PathStrokeType (bw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float base = style.bipolar ? float (valueToProportionOfLength (0.0)) : 0.0f;
        if (std::abs (t - base) > 1.5e-3f)
        {
            const float lo = juce::jmin (base, t), hi = juce::jmax (base, t);
            juce::Path lit;
            lit.addCentredArc (c.x, c.y, rm, rm, 0.0f, ang (lo), ang (hi), true);
            const auto pointAt = [&] (float proportion)
            {
                const float angle = ang (proportion);
                return c.translated (rm * std::sin (angle), -rm * std::cos (angle));
            };
            const auto ink = bypassColour (tint (juce::Colour (0xffb59bed), meterHue), bypassAccent);
            g.setColour (ink.withAlpha (0.19f));
            g.strokePath (lit, juce::PathStrokeType (bw * 1.47f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setGradientFill (juce::ColourGradient (ink.withAlpha (0.88f), pointAt (base),
                                                     ink.withAlpha (0.48f), pointAt (t), false));
            g.strokePath (lit, juce::PathStrokeType (bw * 0.66f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    const float ri = R * (sustain ? 0.755f : 0.64f);
    if (sustain)
    {
        g.setGradientFill (juce::ColourGradient (face.darker (0.055f), c.translated (0.0f, -ri),
                                                 face, c.translated (0.0f, ri), false));
        g.fillEllipse (c.x - ri, c.y - ri, 2 * ri, 2 * ri);
    }

    const float lcdH = R * (speed ? 0.226f : (sustain ? 0.234f : 0.242f));
    const float lcdY = c.y - R * (speed ? 0.226f : (sustain ? 0.307f : 0.275f));
    const float lcdShift = R * (speed ? -0.021f : (sustain ? 0.014f : -0.033f));
    const auto readout = format (displayedValue);
    const juce::Rectangle<float> display (c.x - ri + lcdShift, lcdY, 2 * ri, lcdH);
    drawSevenSeg (g, readout, display.translated (0.0f, 0.9f), style.cells,
                  juce::Colours::black.withAlpha (0.46f), juce::Colours::transparentBlack, true);
    const auto digit = lightMode ? text : juce::Colour (0xfffcfaf9);
    drawSevenSeg (g, readout, display, style.cells,
                  digit.interpolatedWith (lavender.interpolatedWith (violet, 0.35f), turning.get()), juce::Colours::transparentBlack, true);
    g.setFont (font (sustain ? 16.0f : 17.0f));
    const float labelY = lcdY + lcdH + R * (speed ? 0.054f : (sustain ? 0.136f : 0.081f));
    shadowText (g, (hv > 0.5f || isMouseButtonDown()) && style.unit.isNotEmpty() ? style.unit : label,
                juce::Rectangle<float> (c.x - ri + style.labelDx * R, labelY, 2 * ri, R * 0.16f),
                juce::Justification::centred, text.withAlpha (0.96f), 0.8f, 0.46f);
}

void Knob::mouseDown (const juce::MouseEvent& e)
{
    if (! e.mods.isPopupMenu())
        turning.setTarget (1.0f);
    if (e.mods.isPopupMenu())
        showValueEditor();
    else
        Slider::mouseDown (e);
}

void Knob::showValueEditor()
{
    editor = std::make_unique<juce::TextEditor>();
    const int w = juce::jmax (60, getWidth() / 2), h = 26;
    editor->setBounds (getLocalBounds().withSizeKeepingCentre (w, h));
    editor->setFont (theme::font (15.0f, true));
    editor->setJustification (juce::Justification::centred);
    editor->setText (format (getValue()), false);
    editor->selectAll();
    editor->onReturnKey = [this] {
        const double v = editor->getText().retainCharacters ("-0123456789.").getDoubleValue();
        setValue (snapValue (juce::jlimit (getMinimum(), getMaximum(), v), notDragging), juce::sendNotificationSync);
        juce::MessageManager::callAsync ([sp = juce::Component::SafePointer<Knob> (this)] { if (sp) sp->editor.reset(); });
    };
    editor->onEscapeKey = editor->onFocusLost = [this] {
        juce::MessageManager::callAsync ([sp = juce::Component::SafePointer<Knob> (this)] { if (sp) sp->editor.reset(); });
    };
    addAndMakeVisible (*editor);
    editor->grabKeyboardFocus();
}
