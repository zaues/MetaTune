#include "Knob.h"

static float rad (float deg) { return juce::degreesToRadians (deg); }

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
    return cutRadius <= 0 || p.getDistanceFrom (cutCentre - getPosition().toFloat()) > cutRadius;
}

void Knob::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto bounds = getLocalBounds().toFloat();
    const float R = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 1.0f;
    const auto c = bounds.getCentre();
    const float t = float (valueToProportionOfLength (getValue()));
    const float hv = hover;
    const bool hot = hv > 0.5f || hasKeyboardFocus (false);

    juce::Graphics::ScopedSaveState ss (g);
    if (cutRadius > 0)
    {
        // Even-odd: bounds minus the neighbour's disc and a gap ring.
        juce::Path clip;
        clip.addRectangle (bounds.expanded (4.0f));
        const auto cc = cutCentre - getPosition().toFloat();
        const float cr = cutRadius + R * 0.05f;
        clip.addEllipse (cc.x - cr, cc.y - cr, 2 * cr, 2 * cr);
        clip.setUsingNonZeroWinding (false);
        g.reduceClipRegion (clip);
    }

    // Bezel and body.
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2e3038), c.x, c.y - R, juce::Colour (0xff0b0c0e), c.x, c.y + R, false));
    g.fillEllipse (c.x - R, c.y - R, 2 * R, 2 * R);
    const float rb = R * 0.965f;
    g.setGradientFill (juce::ColourGradient (knobBody.brighter (0.06f), c.x, c.y - rb, knobBody.darker (0.3f), c.x, c.y + rb, false));
    g.fillEllipse (c.x - rb, c.y - rb, 2 * rb, 2 * rb);

    if (style.ticks)
    {
        const float r1 = R * 0.70f, r2 = R * 0.90f, tickW = juce::jmax (1.4f, R * 0.021f);
        const bool band = style.bandFrom >= 0.0f;
        const float tickEnd = band ? style.bandFrom : 1.0f; // proportion where the ticks stop
        const float a0 = rad (style.startDeg), a1 = rad (style.endDeg);
        auto angleAt = [&] (float p) { return a0 + (a1 - a0) * p; };
        const int n = juce::jmax (8, juce::roundToInt ((style.endDeg - style.startDeg) * tickEnd / 2.4f));
        {
            // Glow under the lit ticks (the "box-shadow" of the arc).
            const float litTo = juce::jmin (t, tickEnd);
            if (litTo > 0.0f)
            {
                juce::Path glowArc;
                glowArc.addCentredArc (c.x, c.y, (r1 + r2) * 0.5f, (r1 + r2) * 0.5f, 0.0f, a0, angleAt (litTo), true);
                juce::ColourGradient gg (rose.withAlpha (0.20f), c.x - R, c.y + R, lilac.withAlpha (0.18f), c.x + R, c.y - R, false);
                g.setGradientFill (gg);
                g.strokePath (glowArc, juce::PathStrokeType ((r2 - r1) * 1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
            }
        }
        for (int i = 0; i < n; ++i)
        {
            const float u = float (i) / float (n - 1), p = u * tickEnd;
            const float a = angleAt (p);
            const juce::Point<float> dir (std::sin (a), -std::cos (a));
            const auto base = rose.interpolatedWith (lilac, u);
            const bool lit = p <= t + 1e-4f;
            g.setColour (lit ? base.brighter (0.12f * hv) : base.withAlpha (0.2f + 0.1f * hv));
            g.drawLine ({ c + dir * r1, c + dir * r2 }, tickW);
        }
        if (band)
        {
            const float rm = (r1 + r2) * 0.5f, bw = (r2 - r1) * 0.62f;
            const float ab = angleAt (style.bandFrom);
            juce::Path full;
            full.addCentredArc (c.x, c.y, rm, rm, 0.0f, ab, a1, true);
            g.setColour (juce::Colours::white.withAlpha (hot ? 0.10f : 0.07f));
            g.strokePath (full, juce::PathStrokeType (bw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            if (t > style.bandFrom)
            {
                juce::Path lit;
                lit.addCentredArc (c.x, c.y, rm, rm, 0.0f, ab, angleAt (t), true);
                const juce::Point<float> pa (c.x + rm * std::sin (ab), c.y - rm * std::cos (ab));
                const juce::Point<float> pb (c.x + rm * std::sin (a1), c.y - rm * std::cos (a1));
                juce::ColourGradient grad (cyan, pa, pink, pb, false);
                grad.addColour (0.45, violet.brighter (0.25f));
                if (activity > 0.01f) // soft halo under the band while singing
                {
                    juce::ColourGradient glow (cyan.withAlpha (0.45f * activity), pa, pink.withAlpha (0.45f * activity), pb, false);
                    g.setGradientFill (glow);
                    g.strokePath (lit, juce::PathStrokeType (bw * (1.0f + 1.4f * activity), juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
                g.setGradientFill (grad);
                g.setOpacity (0.75f + 0.25f * activity);
                g.strokePath (lit, juce::PathStrokeType (bw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                g.setOpacity (1.0f);
            }
            if (t > style.bandFrom) // position marker on the band
            {
                const float at = angleAt (t);
                const juce::Point<float> mp (c.x + rm * std::sin (at), c.y - rm * std::cos (at));
                const float md = bw * 0.62f;
                g.setColour (juce::Colours::white);
                g.fillEllipse (mp.x - md, mp.y - md, 2 * md, 2 * md);
            }
            const float am = angleAt (style.bandFrom + (1.0f - style.bandFrom) * 0.55f);
            const juce::Point<float> bp (c.x + rm * std::sin (am), c.y - rm * std::cos (am));
            g.setColour (t > style.bandFrom ? juce::Colour (0xff2a1450) : textDim);
            g.fillPath (bolt (juce::Rectangle<float> (bw * 0.9f, bw * 0.9f).withCentre (bp)));
        }
    }

    // Inner disc.
    const float ri = style.ticks ? R * 0.63f : R * 0.86f;
    g.setGradientFill (juce::ColourGradient (knobBody.brighter (0.10f), c.x, c.y - ri, juce::Colour (0xff0c0d0f), c.x, c.y + ri, false));
    g.fillEllipse (c.x - ri, c.y - ri, 2 * ri, 2 * ri);
    g.setColour (juce::Colours::white.withAlpha (0.05f + 0.07f * hv));
    g.drawEllipse (c.x - ri + 0.5f, c.y - ri + 0.5f, 2 * ri - 1, 2 * ri - 1, 1.0f);

    // Readout and label.
    const float lcdH = style.ticks ? R * 0.22f : R * 0.24f;
    const float lcdY = style.ticks ? c.y - R * 0.17f : c.y - R * 0.24f;
    drawSevenSeg (g, format (getValue()), { c.x - ri, lcdY, 2 * ri, lcdH }, style.cells,
                  text.interpolatedWith (lavender.interpolatedWith (violet, 0.35f), turning.get()), juce::Colours::white.withAlpha (0.05f));
    g.setColour (text.withAlpha (0.92f));
    g.setFont (font (juce::jmax (11.0f, R * (style.ticks ? 0.135f : 0.22f)), true));
    g.drawText (hot && style.unit.isNotEmpty() ? style.unit : label, juce::Rectangle<float> (c.x - ri, lcdY + lcdH + R * 0.08f, 2 * ri, R * (style.ticks ? 0.16f : 0.26f)),
                juce::Justification::centred);
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
