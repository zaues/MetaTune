#include "Widgets.h"

namespace icons
{
static juce::Path undoShape()
{
    // Hooked arrow: arc over the top, arrowhead on the left end pointing down.
    juce::Path p;
    p.addCentredArc (12, 13, 7.5f, 6.5f, 0, -juce::MathConstants<float>::halfPi, 2.3f, true);
    const juce::Point<float> tip (4.5f, 13.0f);
    p.startNewSubPath (tip.translated (-3.2f, -3.0f));
    p.lineTo (tip.translated (0.0f, 0.8f));
    p.lineTo (tip.translated (3.2f, -3.0f));
    return p;
}
juce::Path undo() { return undoShape(); }
juce::Path redo() { auto p = undoShape(); p.applyTransform (juce::AffineTransform::scale (-1.0f, 1.0f, 12.0f, 12.0f)); return p; }
juce::Path reset()
{
    juce::Path p;
    p.addCentredArc (12, 12, 7, 7, 0, 0.6f, 5.6f, true);
    p.startNewSubPath (13.5f, 1.5f); p.lineTo (16.5f, 5.0f); p.lineTo (12.5f, 7.0f);
    return p;
}
juce::Path power()
{
    juce::Path p;
    p.addCentredArc (12, 13, 7, 7, 0, 0.65f, juce::MathConstants<float>::twoPi - 0.65f, true);
    p.startNewSubPath (12, 3); p.lineTo (12, 11);
    return p;
}
juce::Path eye()
{
    juce::Path p;
    p.startNewSubPath (3, 12); p.quadraticTo (12, 3, 21, 12); p.quadraticTo (12, 21, 3, 12); p.closeSubPath();
    p.addEllipse (9.5f, 9.5f, 5, 5);
    return p;
}
juce::Path notes()
{
    juce::Path p;
    for (int i = 0; i < 4; ++i) { p.startNewSubPath (4.0f + 5.0f * float (i), 18); p.lineTo (4.0f + 5.0f * float (i), 6.0f + float (i % 2) * 4.0f); }
    p.startNewSubPath (2, 20); p.lineTo (22, 20);
    return p;
}
juce::Path help()
{
    juce::Path p;
    p.addEllipse (3, 3, 18, 18);
    p.startNewSubPath (9.2f, 9.5f); p.cubicTo (9.2f, 6.0f, 15.0f, 6.0f, 15.0f, 9.5f); p.cubicTo (15.0f, 12.0f, 12.0f, 11.8f, 12.0f, 14.5f);
    p.addEllipse (11.2f, 16.6f, 1.6f, 1.6f);
    return p;
}
juce::Path menu()
{
    juce::Path p;
    for (int i = 0; i < 3; ++i) { p.startNewSubPath (5, 7.0f + 5.0f * float (i)); p.lineTo (19, 7.0f + 5.0f * float (i)); }
    return p;
}
juce::Path panel()
{
    juce::Path p;
    p.addRoundedRectangle (4.0f, 5.0f, 16.0f, 14.0f, 1.5f);
    p.startNewSubPath (4, 10); p.lineTo (20, 10);
    return p;
}
juce::Path copyArrow()
{
    juce::Path p;
    p.startNewSubPath (9, 5); p.lineTo (16, 12); p.lineTo (9, 19);
    return p;
}
juce::Path swap()
{
    juce::Path p;
    p.startNewSubPath (4, 9); p.lineTo (18, 9); p.startNewSubPath (14, 5); p.lineTo (18, 9); p.lineTo (14, 13);
    p.startNewSubPath (20, 15); p.lineTo (6, 15); p.startNewSubPath (10, 11); p.lineTo (6, 15); p.lineTo (10, 19);
    return p;
}
juce::Path loop()
{
    juce::Path p;
    p.startNewSubPath (5, 13); p.lineTo (5, 10); p.quadraticTo (5, 7, 8, 7); p.lineTo (18, 7);
    p.startNewSubPath (15, 4); p.lineTo (18, 7); p.lineTo (15, 10);
    p.startNewSubPath (19, 11); p.lineTo (19, 14); p.quadraticTo (19, 17, 16, 17); p.lineTo (6, 17);
    p.startNewSubPath (9, 14); p.lineTo (6, 17); p.lineTo (9, 20);
    return p;
}
juce::Path save()
{
    juce::Path p;
    p.startNewSubPath (5, 4); p.lineTo (16, 4); p.lineTo (20, 8); p.lineTo (20, 20); p.lineTo (4, 20); p.lineTo (4, 5); p.closeSubPath();
    p.addRectangle (8.0f, 4.0f, 7.0f, 5.0f);
    p.addRectangle (8.0f, 13.0f, 8.0f, 7.0f);
    return p;
}
juce::Path chat()
{
    juce::Path p;
    p.startNewSubPath (4, 5); p.lineTo (20, 5); p.lineTo (20, 16); p.lineTo (10, 16); p.lineTo (6, 20); p.lineTo (6, 16); p.lineTo (4, 16); p.closeSubPath();
    p.startNewSubPath (8, 9); p.lineTo (16, 9);
    p.startNewSubPath (8, 12); p.lineTo (14, 12);
    return p;
}
juce::Path grid()
{
    juce::Path p;
    p.addRectangle (4.0f, 4.0f, 6.5f, 6.5f); p.addRectangle (13.5f, 4.0f, 6.5f, 6.5f);
    p.addRectangle (4.0f, 13.5f, 6.5f, 6.5f); p.addRectangle (13.5f, 13.5f, 6.5f, 6.5f);
    return p;
}
juce::Path chevronUp() { juce::Path p; p.startNewSubPath (6, 15); p.lineTo (12, 9); p.lineTo (18, 15); return p; }
juce::Path chevronDown() { juce::Path p; p.startNewSubPath (6, 9); p.lineTo (12, 15); p.lineTo (18, 9); return p; }
} // namespace icons

static void strokeIcon (juce::Graphics& g, const juce::Path& icon, juce::Rectangle<float> area, float thickness = 1.7f)
{
    g.strokePath (icon, juce::PathStrokeType (thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded),
                  icon.getTransformToScaleToFit (area, true));
}

IconButton::IconButton (const juce::String& name, juce::Path i, juce::String c, juce::Path i2)
    : juce::Button (name), icon (std::move (i)), icon2 (std::move (i2)), caption (std::move (c))
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void IconButton::paintButton (juce::Graphics& g, bool, bool down)
{
    using namespace theme;
    const float h = hover;
    auto r = getLocalBounds().toFloat();
    if (hoverFill && h > 0.0f)
    {
        g.setColour (juce::Colours::white.withAlpha ((down ? 0.10f : 0.06f) * h));
        g.fillRoundedRectangle (r.reduced (1.0f), 4.0f);
    }
    const bool on = getToggleState();
    const auto col = ! isEnabled() ? textFaint : (on ? onColour : textDim.interpolatedWith (theme::text, h));
    const float is = juce::jmin (r.getWidth(), r.getHeight());
    if (caption.isEmpty())
    {
        g.setColour (col);
        const float s = is * iconScale * (1.0f + 0.06f * h);
        strokeIcon (g, icon, juce::Rectangle<float> (s, s).withCentre (r.getCentre()));
        return;
    }
    // Icons on the left, caption on the right.
    const float s = juce::jmin (22.0f, is * 0.6f);
    auto left = r.removeFromLeft (icon2.isEmpty() ? s + 22.0f : 2.0f * s + 26.0f);
    g.setColour (col);
    strokeIcon (g, icon, juce::Rectangle<float> (s, s).withCentre ({ left.getX() + 8.0f + s * 0.5f, left.getCentreY() }));
    if (! icon2.isEmpty())
    {
        g.setColour (textDim.interpolatedWith (theme::text, h * 0.6f));
        strokeIcon (g, icon2, juce::Rectangle<float> (s * 0.85f, s * 0.85f).withCentre ({ left.getX() + 14.0f + s * 1.5f, left.getCentreY() }), 1.5f);
    }
    auto capCol = on ? theme::text : textDim.interpolatedWith (theme::text, h);
    if (on && onColour != theme::text) capCol = onColour;
    g.setColour (isEnabled() ? capCol : textFaint);
    g.setFont (font (12.5f, true));
    g.drawFittedText (caption, r.toNearestInt().withTrimmedRight (4), juce::Justification::centred, 2, 0.9f);
}

void PillButton::paintButton (juce::Graphics& g, bool, bool)
{
    using namespace theme;
    const auto r = getLocalBounds().toFloat();
    const bool on = getToggleState();
    g.setColour (on ? theme::text : textDim.interpolatedWith (theme::text, 0.7f * hover));
    g.setFont (font (20.0f, on));
    g.drawText (getButtonText(), r, juce::Justification::centred);
}

void TextPill::paintButton (juce::Graphics& g, bool, bool down)
{
    using namespace theme;
    const auto r = getLocalBounds().toFloat().reduced (2.0f);
    const bool on = getToggleState();
    lit.setTarget (on ? 1.0f : 0.0f);
    const float h = hover, l = lit;
    juce::Path p;
    p.addRoundedRectangle (r, r.getHeight() * 0.5f);
    if (kind == Kind::Primary)
    {
        if (l > 0.0f)
        {
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff42454f).withAlpha (l), r.getX(), r.getY(), juce::Colour (0xff2f323a).withAlpha (l), r.getX(), r.getBottom(), false));
            g.fillPath (p);
        }
    }
    else
    {
        // Recessed pill: dark fill with an inset shadow along the top.
        g.setColour (bg1.interpolatedWith (juce::Colour (0xff2a2b34), l));
        g.fillPath (p);
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (p);
        g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.45f), r.getX(), r.getY(), juce::Colours::transparentBlack, r.getX(), r.getY() + 7.0f, false));
        g.fillRect (r);
    }
    if (h > 0.0f)
    {
        g.setColour (juce::Colours::white.withAlpha ((down ? 0.12f : 0.06f) * h));
        g.fillPath (p);
    }
    g.setColour ((kind == Kind::Primary && l > 0.5f) ? juce::Colour (0xff6b6f7a) : line.brighter (0.35f * h));
    g.strokePath (p, juce::PathStrokeType (1.0f));
    g.setFont (font (14.0f, on && kind == Kind::Primary));
    shadowText (g, getButtonText(), r, juce::Justification::centred, textDim.interpolatedWith (theme::text, juce::jmax (l, 0.8f * h)));
}

void UpdatePill::paintButton (juce::Graphics& g, bool, bool down)
{
    using namespace theme;
    const auto r = getLocalBounds().toFloat().reduced (3.0f);
    const float h = hover, gl = glow();
    if (gl > 0.0f)
    {
        g.setColour (violet.withAlpha (0.18f * gl));
        g.fillRoundedRectangle (r.expanded (2.5f), r.getHeight() * 0.5f + 2.5f);
    }
    g.setGradientFill (juce::ColourGradient (accentAt (0.0f), r.getX(), r.getCentreY(), accentAt (1.0f), r.getRight(), r.getCentreY(), false));
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    g.setColour (juce::Colours::white.withAlpha ((down ? 0.22f : 0.12f) * h));
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    // Down-arrow glyph, then the label.
    const auto icon = r.withWidth (r.getHeight()).reduced (r.getHeight() * 0.3f).translated (r.getHeight() * 0.18f, 0);
    juce::Path a;
    a.startNewSubPath (icon.getCentreX(), icon.getY());
    a.lineTo (icon.getCentreX(), icon.getBottom() - 1.5f);
    a.startNewSubPath (icon.getX() + 1.0f, icon.getCentreY() + 1.0f);
    a.lineTo (icon.getCentreX(), icon.getBottom() - 1.5f);
    a.lineTo (icon.getRight() - 1.0f, icon.getCentreY() + 1.0f);
    g.setColour (juce::Colours::white);
    g.strokePath (a, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setFont (font (12.5f, true));
    g.drawText (getButtonText(), r.withTrimmedLeft (r.getHeight() * 0.95f).withTrimmedRight (8.0f), juce::Justification::centred);
}

void ToggleSwitch::paintButton (juce::Graphics& g, bool, bool)
{
    using namespace theme;
    const bool on = getToggleState();
    if (first) { pos.snapTo (on ? 1.0f : 0.0f); first = false; }
    pos.setTarget (on ? 1.0f : 0.0f);
    const float t = pos, h = hover;
    const auto r = getLocalBounds().toFloat().withSizeKeepingCentre (float (getWidth()), 18.0f);
    // Track brightens from dark to mid grey while the knob slides across.
    g.setColour (juce::Colour (0xff2a2c33).brighter (0.15f * h).interpolatedWith (juce::Colour (0xff5a5e69), t));
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    const float d = r.getHeight() - 4.0f;
    const float x = r.getX() + 2.0f + t * (r.getWidth() - d - 4.0f);
    juce::Path knob;
    knob.addEllipse (x, r.getY() + 2.0f, d, d);
    juce::DropShadow (juce::Colours::black.withAlpha (0.5f), 6, { 0, 1 }).drawForPath (g, knob);
    g.setColour (juce::Colour (0xff9c9ba3).interpolatedWith (juce::Colours::white, juce::jmax (t, 0.4f * h)));
    g.fillPath (knob);
}

SegmentedControl::SegmentedControl (juce::RangedAudioParameter& p, juce::StringArray l, juce::UndoManager* um)
    : labels (std::move (l)),
      attach (p, [this] (float v) { selected = juce::roundToInt (v); slide.setTarget (float (selected)); repaint(); }, um)
{
    for (auto& a : hoverAmt) a = std::make_unique<theme::Anim> (*this);
    attach.sendInitialUpdate();
    slide.snapTo (float (selected));
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void SegmentedControl::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto r = getLocalBounds().toFloat();
    const float w = r.getWidth() / float (labels.size());
    // Grey highlight glides to the selected segment.
    const auto sel = juce::Rectangle<float> (r.getX() + w * slide.get(), r.getY(), w, r.getHeight()).reduced (3.0f);
    g.setColour (juce::Colour (0xff2a2b34));
    g.fillRoundedRectangle (sel, sel.getHeight() * 0.5f);
    g.setColour (line.brighter (0.25f));
    g.drawRoundedRectangle (sel, sel.getHeight() * 0.5f, 1.0f);
    for (int i = 0; i < labels.size(); ++i)
    {
        const auto cellR = juce::Rectangle<float> (r.getX() + w * float (i), r.getY(), w, r.getHeight()).reduced (3.0f);
        const float h = i < int (hoverAmt.size()) ? hoverAmt[size_t (i)]->get() : 0.0f;
        if (h > 0.0f && i != selected)
        {
            g.setColour (juce::Colours::white.withAlpha (0.05f * h));
            g.fillRoundedRectangle (cellR, cellR.getHeight() * 0.5f);
        }
        const float near = juce::jmax (0.0f, 1.0f - std::abs (slide.get() - float (i)));
        g.setColour (textDim.brighter (0.15f).interpolatedWith (text, juce::jmax (near, 0.7f * h)));
        g.setFont (font (14.0f));
        g.drawText (labels[i], cellR, juce::Justification::centred);
    }
}

void SegmentedControl::mouseDown (const juce::MouseEvent& e)
{
    const int i = indexAt (e.position.x);
    if (i != selected)
        attach.setValueAsCompleteGesture (float (i));
}

void SegmentedControl::mouseMove (const juce::MouseEvent& e)
{
    const int i = indexAt (e.position.x);
    if (i == hover) return;
    if (juce::isPositiveAndBelow (hover, int (hoverAmt.size()))) hoverAmt[size_t (hover)]->setTarget (0.0f);
    hover = i;
    if (juce::isPositiveAndBelow (hover, int (hoverAmt.size()))) hoverAmt[size_t (hover)]->setTarget (1.0f);
}

int GroupTabs::indexAt (float y) const
{
    if (y < float (kTop)) return -1;
    return juce::jlimit (0, 3, int ((y - float (kTop)) / ((float (getHeight()) - float (kTop)) / 4.0f)));
}

void GroupTabs::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto b = getLocalBounds().toFloat();
    juce::Path bg;
    bg.addRoundedRectangle (b.getX() - 12.0f, b.getY(), b.getWidth() + 12.0f, b.getHeight(), 12.0f);
    g.setColour (juce::Colour (0xff121213));
    g.fillPath (bg);
    g.setColour (textDim);
    juce::Path gi = icons::grid();
    g.fillPath (gi, gi.getTransformToScaleToFit (juce::Rectangle<float> (20.0f, 20.0f).withCentre ({ b.getCentreX(), float (kTop) * 0.55f }), true));

    const float rowH = (b.getHeight() - float (kTop)) / 4.0f;
    for (int i = 0; i < 4; ++i)
    {
        const auto r = juce::Rectangle<float> (b.getX(), float (kTop) + rowH * float (i), b.getWidth(), rowH);
        selAmt[i].setTarget (i == selected ? 1.0f : 0.0f);
        const float sa = selAmt[i], ha = hoverAmt[i];
        if (sa > 0.0f || ha > 0.0f)
        {
            g.setColour (juce::Colour (0xff121213).interpolatedWith (juce::Colour (0xff1c1c1e), ha).interpolatedWith (juce::Colour (0xff252528), sa));
            g.fillRect (r.reduced (0.0f, 1.0f));
            g.setColour (juce::Colour (0xffc9ccd4).withAlpha (0.8f * sa));
            g.fillRect (r.getX(), r.getY() + 6.0f, 2.0f, r.getHeight() - 12.0f);
        }
        // How many plugin copies are in this group.
        g.setColour (counts[i] > 0 ? violet.brighter (0.4f) : violet.withAlpha (0.8f));
        g.setFont (font (12.0f, true));
        g.drawText (juce::String (counts[i]), juce::Rectangle<float> (r.getRight() - 24.0f, r.getY() + 4.0f, 18.0f, 14.0f), juce::Justification::centred);
        g.setColour (textDim.interpolatedWith (text, juce::jmax (sa, 0.7f * ha)));
        g.setFont (font (17.0f, true));
        g.drawText (juce::String (i + 1), juce::Rectangle<float> (r.getX() + 8.0f, r.getY() + 14.0f, 18.0f, r.getHeight() - 14.0f), juce::Justification::centred);
    }
}

void GroupTabs::mouseDown (const juce::MouseEvent& e)
{
    const int i = indexAt (e.position.y);
    if (i >= 0) { if (onSelect) onSelect (i); }
    else if (onBrowse) onBrowse(); // the grid icon at the top opens the Group Browser
}

void GroupTabs::mouseMove (const juce::MouseEvent& e)
{
    const int i = indexAt (e.position.y);
    if (i == hover) return;
    if (hover >= 0) hoverAmt[hover].setTarget (0.0f);
    hover = i;
    if (hover >= 0) hoverAmt[hover].setTarget (1.0f);
}

void PresetBar::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto b = getLocalBounds().toFloat();
    const auto mouse = getMouseXYRelative().toFloat();
    const bool over = isMouseOver();
    auto arrows = b.withLeft (b.getRight() - 36.0f);
    auto main = b.withRight (arrows.getX());

    g.setColour (headerBox);
    g.fillRect (b);
    if (over && main.contains (mouse))
    {
        g.setColour (juce::Colours::white.withAlpha (0.04f));
        g.fillRect (main);
    }
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRect (arrows.getX(), b.getY(), 1.0f, b.getHeight());

    // Name in an oblique style.
    {
        juce::Graphics::ScopedSaveState ss (g);
        const auto tr = main.withTrimmedLeft (18.0f).withTrimmedRight (60.0f);
        g.addTransform (juce::AffineTransform::shear (-0.18f, 0.0f).translated (0.18f * tr.getCentreY(), 0.0f));
        g.setColour (name.isEmpty() ? textDim : text);
        g.setFont (font (14.0f));
        g.drawText (name.isEmpty() ? juce::String ("No preset selected") : name, tr, juce::Justification::centredLeft);
    }

    // Preset icon: bracketed bars.
    {
        const auto ic = juce::Rectangle<float> (22.0f, 14.0f).withCentre ({ main.getRight() - 30.0f, main.getCentreY() });
        g.setColour (lavender);
        juce::Path p;
        p.startNewSubPath (ic.getX() + 3, ic.getY()); p.lineTo (ic.getX(), ic.getY()); p.lineTo (ic.getX(), ic.getBottom()); p.lineTo (ic.getX() + 3, ic.getBottom());
        p.startNewSubPath (ic.getRight() - 3, ic.getY()); p.lineTo (ic.getRight(), ic.getY()); p.lineTo (ic.getRight(), ic.getBottom()); p.lineTo (ic.getRight() - 3, ic.getBottom());
        for (int i = 0; i < 3; ++i)
        {
            const float x = ic.getX() + 6.0f + 5.0f * float (i);
            p.startNewSubPath (x, ic.getBottom() - 2.0f); p.lineTo (x, ic.getY() + (i == 1 ? 3.0f : 6.0f));
        }
        g.strokePath (p, juce::PathStrokeType (1.6f));
    }

    const auto up = arrows.withHeight (arrows.getHeight() * 0.5f), down = arrows.withTrimmedTop (arrows.getHeight() * 0.5f);
    for (auto [r, ic] : { std::pair { up, icons::chevronUp() }, std::pair { down, icons::chevronDown() } })
    {
        g.setColour (over && r.contains (mouse) ? text : textFaint);
        strokeIcon (g, ic, juce::Rectangle<float> (12.0f, 12.0f).withCentre (r.getCentre()), 1.4f);
    }
}

void PresetBar::mouseUp (const juce::MouseEvent& e)
{
    const float ax = float (getWidth()) - 36.0f;
    if (e.position.x < ax) { if (onClick) onClick(); }
    else if (e.position.y < float (getHeight()) * 0.5f) { if (onPrev) onPrev(); }
    else if (onNext) onNext();
}

ToneSlider::ToneSlider()
{
    setSliderStyle (LinearHorizontal);
    setTextBoxStyle (NoTextBox, true, 0, 0);
    setVelocityModeParameters (0.2, 1, 0.0, true, juce::ModifierKeys::shiftModifier);
    setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
}

void ToneSlider::paint (juce::Graphics& g)
{
    const auto track = getLocalBounds().toFloat().withSizeKeepingCentre (float (getWidth()), 14.0f);
    const float x = track.getX() + 3.0f + float (valueToProportionOfLength (getValue())) * (track.getWidth() - 6.0f);
    // Empty dark track; the red fill grows from the left up to the handle, so it follows the volume.
    g.setColour (juce::Colour (0xff141416));
    g.fillRoundedRectangle (track, 3.0f);
    {
        juce::Graphics::ScopedSaveState s (g);
        g.reduceClipRegion (track.withRight (x).getSmallestIntegerContainer());
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a1012), track.getX(), 0, juce::Colour (0xff7b2c35), track.getRight(), 0, false));
        g.fillRoundedRectangle (track, 3.0f);
    }
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawRoundedRectangle (track, 3.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (isMouseOverOrDragging() ? 1.0f : 0.9f));
    g.fillRect (x - 1.0f, track.getY() + 1.0f, 2.0f, track.getHeight() - 2.0f);
}

ValueStrip::ValueStrip()
{
    setSliderStyle (LinearHorizontal);
    setTextBoxStyle (NoTextBox, true, 0, 0);
    setMouseDragSensitivity (300);
    setVelocityModeParameters (0.2, 1, 0.0, true, juce::ModifierKeys::shiftModifier);
    setSliderSnapsToMousePosition (false);
    setMouseCursor (juce::MouseCursor::LeftRightResizeCursor);
}

void ValueStrip::paint (juce::Graphics& g)
{
    using namespace theme;
    auto r = getLocalBounds().toFloat();
    const auto track = r.removeFromTop (16.0f);
    g.setColour (bg0);
    g.fillRoundedRectangle (track, 4.0f);
    const float x = track.getX() + 3.0f + float (valueToProportionOfLength (getValue())) * (track.getWidth() - 6.0f);
    g.setColour (juce::Colours::white.withAlpha (isMouseOverOrDragging() ? 1.0f : 0.9f));
    g.fillRect (x - 1.0f, track.getY() + 2.0f, 2.0f, track.getHeight() - 4.0f);
    const double hz = getValue();
    const int cents = juce::roundToInt (1200.0 * std::log2 (hz / 440.0));
    g.setColour (text);
    g.setFont (font (13.5f, true));
    g.drawText (juce::String (cents) + "cts " + juce::String (hz, 1) + "Hz", r.withTrimmedTop (6.0f), juce::Justification::centred);
}

const juce::StringArray& RootPicker::scaleNames()
{
    static const juce::StringArray names { "Major", "Minor", "Pent Maj", "Pent Min", "Harm Maj", "Harm Min", "Custom", "Chromatic" };
    return names;
}

theme::Anim& RootPicker::cellAnim (int i)
{
    while (int (cellHover.size()) <= i) cellHover.push_back (std::make_unique<theme::Anim> (*this, 0.3f));
    return *cellHover[size_t (i)];
}

void RootPicker::setState (int r, int s, bool f)
{
    if (r != root || s != scale || f != flats) { root = r; scale = s; flats = f; repaint(); }
}

juce::Rectangle<float> RootPicker::tabRect (int i) const
{
    const float w = (float (getWidth()) - 8.0f) * 0.5f;
    return { 4.0f + w * float (i), 3.0f, w, 22.0f };
}

juce::Rectangle<float> RootPicker::noteRect (int pc) const
{
    const float w = (float (getWidth()) - 8.0f) / 3.0f, h = 27.0f;
    return { 4.0f + w * float (pc % 3), 30.0f + h * float (pc / 3), w, h };
}

juce::Rectangle<float> RootPicker::scaleRect (int i) const
{
    const float w = (float (getWidth()) - 8.0f) * 0.5f, h = 25.0f, top = 30.0f + 27.0f * 4.0f + 6.0f;
    return { 4.0f + w * float (i % 2), top + h * float (i / 2), w, h };
}

void RootPicker::paint (juce::Graphics& g)
{
    using namespace theme;
    static const char* letters[] { "C", "C", "D", "D", "E", "F", "F", "G", "G", "A", "A", "B" };
    static const char* flatLetters[] { "C", "D", "D", "E", "E", "F", "G", "G", "A", "A", "B", "B" };
    static const bool accidental[] { false, true, false, true, false, false, true, false, true, false, true, false };
    const auto b = getLocalBounds().toFloat();
    const auto mouse = getMouseXYRelative().toFloat();
    const bool over = isMouseOver (true);

    // Structured drawer: charcoal block with a 1 px frame.
    g.setColour (bg1);
    g.fillRect (b);
    g.setColour (line);
    g.drawRect (b, 1.0f);

    int cell = 0;
    auto hoverBox = [&] (juce::Rectangle<float> r, bool selected) {
        auto& a = cellAnim (cell++);
        a.setTarget (over && r.contains (mouse) ? 1.0f : 0.0f);
        const auto rr = r.reduced (2.0f, 1.5f);
        // Active item: thin light-grey box (no fill). Hover: a fainter box fading in.
        const float k = juce::jmax (selected ? 1.0f : 0.0f, 0.6f * a.get());
        if (k > 0.0f)
        {
            g.setColour (juce::Colour (0xffc9ccd4).withAlpha (0.85f * k));
            g.drawRoundedRectangle (rr, 3.0f, 1.0f);
        }
    };

    // Flats / Sharps tabs.
    for (int i = 0; i < 2; ++i)
    {
        const auto r = tabRect (i);
        const bool sel = (i == 0) == flats;
        auto& a = cellAnim (cell++);
        a.setTarget (over && r.contains (mouse) ? 1.0f : 0.0f);
        if (sel || a.get() > 0.0f)
        {
            g.setColour (juce::Colour (0xffc9ccd4).withAlpha (sel ? 0.85f : 0.5f * a.get()));
            g.drawRoundedRectangle (r.reduced (1.5f), 4.0f, 1.0f);
        }
        g.setColour (sel ? text : greyText.interpolatedWith (text, a.get()));
        g.setFont (font (12.5f, sel));
        g.drawText (i == 0 ? "Flats" : "Sharps", r, juce::Justification::centred);
    }

    // Root grid: letter plus a raised accidental.
    for (int pc = 0; pc < 12; ++pc)
    {
        const auto r = noteRect (pc);
        hoverBox (r, pc == root);
        juce::AttributedString as;
        const auto col = pc == root ? text : greyText;
        as.append (flats ? flatLetters[pc] : letters[pc], font (15.0f, pc == root), col);
        if (accidental[pc])
            as.append (flats ? "b" : "#", font (9.5f, true), col);
        juce::TextLayout tl;
        tl.createLayout (as, r.getWidth());
        tl.draw (g, juce::Rectangle<float> (tl.getWidth(), tl.getHeight()).withCentre (r.getCentre()));
    }

    // Scales.
    for (int i = 0; i < scaleNames().size(); ++i)
    {
        const auto r = scaleRect (i);
        hoverBox (r, false);
        const bool sel = i == scale;
        g.setColour (sel ? text : greyText);
        g.setFont (font (sel ? 12.5f : 11.5f, sel));
        g.drawText (scaleNames()[i], r, juce::Justification::centred);
    }
}

void RootPicker::mouseUp (const juce::MouseEvent& e)
{
    for (int i = 0; i < 2; ++i)
        if (tabRect (i).contains (e.position)) { if (onFlats) onFlats (i == 0); return; }
    for (int pc = 0; pc < 12; ++pc)
        if (noteRect (pc).contains (e.position)) { if (onRoot) onRoot (pc); return; }
    for (int i = 0; i < scaleNames().size(); ++i)
        if (scaleRect (i).contains (e.position)) { if (onScale) onScale (i); return; } // stays open; hovering out closes it
}

void PresetPanel::setItems (const std::vector<PresetItem>& items, const juce::String& cur)
{
    rows.clear();
    int idx = 0;
    rows.push_back ({ Kind::Section, "Factory", {}, -1, false });
    for (; idx < int (items.size()) && items[size_t (idx)].factory; ++idx)
        rows.push_back ({ Kind::Preset, items[size_t (idx)].name, {}, idx, true });

    juce::String curFolder;
    for (; idx < int (items.size()) && items[size_t (idx)].folder.isNotEmpty(); ++idx)
    {
        const auto& it = items[size_t (idx)];
        if (it.folder != curFolder) { rows.push_back ({ Kind::Folder, it.folder, it.folder, -1, false }); curFolder = it.folder; }
        rows.push_back ({ Kind::Preset, it.name, it.folder, idx, false });
    }
    bool rootHeader = false;
    for (; idx < int (items.size()); ++idx)
    {
        if (! rootHeader) { rows.push_back ({ Kind::Section, "My presets", {}, -1, false }); rootHeader = true; }
        rows.push_back ({ Kind::Preset, items[size_t (idx)].name, {}, idx, false });
    }
    rows.push_back ({ Kind::Section, {}, {}, -1, false });
    rows.push_back ({ Kind::NewPreset, "New preset...", {}, -1, false });
    rows.push_back ({ Kind::NewFolder, "New folder...", {}, -1, false });
    rows.push_back ({ Kind::OpenFolder, "Open presets folder", {}, -1, false });
    current = cur;
    scroll = juce::jlimit (0.0f, juce::jmax (0.0f, contentHeight() - float (getHeight())), scroll);
    repaint();
}

float PresetPanel::contentHeight() const
{
    float h = 6.0f;
    for (const auto& r : rows) h += rowHeight (r);
    return h + 6.0f;
}

int PresetPanel::preferredHeight() const { return int (std::ceil (contentHeight())); }

theme::Anim& PresetPanel::rowAnim (int i)
{
    while (int (hover.size()) <= i) hover.push_back (std::make_unique<theme::Anim> (*this, 0.3f));
    return *hover[size_t (i)];
}

juce::Rectangle<float> PresetPanel::rowBounds (int i) const
{
    float y = 6.0f - scroll;
    for (int j = 0; j < i && j < int (rows.size()); ++j) y += rowHeight (rows[size_t (j)]);
    const float h = juce::isPositiveAndBelow (i, int (rows.size())) ? rowHeight (rows[size_t (i)]) : kRowH;
    return { 6.0f, y, float (getWidth()) - 12.0f, h };
}

juce::Rectangle<float> PresetPanel::trashBounds (juce::Rectangle<float> r) const { return { r.getRight() - 23.0f, r.getCentreY() - 7.0f, 15.0f, 15.0f }; }
juce::Rectangle<float> PresetPanel::plusBounds (juce::Rectangle<float> r) const { return { r.getRight() - 44.0f, r.getCentreY() - 7.0f, 15.0f, 15.0f }; }

int PresetPanel::rowAt (juce::Point<float> p) const
{
    float y = 6.0f - scroll;
    for (int i = 0; i < int (rows.size()); ++i)
    {
        const float h = rowHeight (rows[size_t (i)]);
        if (p.y >= y && p.y < y + h) return i;
        y += h;
    }
    return -1;
}

static void drawTrash (juce::Graphics& g, juce::Rectangle<float> t)
{
    g.setColour (juce::Colour (0xffe06a6a));
    g.drawRect (t.reduced (3.0f, 1.5f).withTrimmedTop (3.0f), 1.2f);     // can body
    g.drawLine (t.getX() + 1.0f, t.getY() + 3.0f, t.getRight() - 1.0f, t.getY() + 3.0f, 1.3f); // lid
    g.drawLine (t.getCentreX() - 1.5f, t.getY() + 1.0f, t.getCentreX() + 1.5f, t.getY() + 1.0f, 1.3f); // handle
}

void PresetPanel::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto b = getLocalBounds().toFloat();
    juce::Path bg;
    bg.addRoundedRectangle (b.getX(), b.getY(), b.getWidth(), b.getHeight(), 8.0f, 8.0f, false, false, true, true);
    juce::DropShadow (juce::Colours::black.withAlpha (0.6f), 16, { 0, 4 }).drawForPath (g, bg);
    g.setColour (juce::Colour (0xff1c1c1e));
    g.fillPath (bg);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.strokePath (bg, juce::PathStrokeType (1.0f));

    juce::Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (getLocalBounds());
    const auto mouse = getMouseXYRelative().toFloat();
    const int hot = isMouseOver (true) ? rowAt (mouse) : -1;

    for (int i = 0; i < int (rows.size()); ++i)
    {
        const auto& row = rows[size_t (i)];
        const auto r = rowBounds (i);
        if (r.getBottom() < 0.0f || r.getY() > b.getHeight()) continue;

        if (row.kind == Kind::Section)
        {
            if (row.text.isEmpty()) { g.setColour (juce::Colours::white.withAlpha (0.08f)); g.fillRect (r.getX() + 6.0f, r.getCentreY(), r.getWidth() - 12.0f, 1.0f); }
            else { g.setColour (textFaint.brighter (0.2f)); g.setFont (font (11.5f, true)); g.drawText (row.text.toUpperCase(), r.withTrimmedLeft (10.0f), juce::Justification::centredLeft); }
            if (dragging && dropRow == i) { g.setColour (violet.withAlpha (0.5f)); g.drawRoundedRectangle (r.reduced (3.0f, 1.0f), 4.0f, 1.4f); }
            continue;
        }
        if (row.kind == Kind::Folder)
        {
            if (dragging && dropRow == i) { g.setColour (violet.withAlpha (0.22f)); g.fillRoundedRectangle (r.reduced (2.0f, 1.0f), 4.0f); }
            g.setColour (textDim.interpolatedWith (text, i == hot ? 1.0f : 0.35f));
            g.setFont (font (12.0f, true));
            g.drawText (juce::String::fromUTF8 ("\xF0\x9F\x93\x81  ") + row.text.toUpperCase(), r.withTrimmedLeft (10.0f).withTrimmedRight (50.0f), juce::Justification::centredLeft);
            if (i == hot && ! dragging)
            {
                const auto pb = plusBounds (r);
                g.setColour (text.withAlpha (0.85f));
                g.drawLine (pb.getCentreX() - 4.0f, pb.getCentreY(), pb.getCentreX() + 4.0f, pb.getCentreY(), 1.6f);
                g.drawLine (pb.getCentreX(), pb.getCentreY() - 4.0f, pb.getCentreX(), pb.getCentreY() + 4.0f, 1.6f);
                drawTrash (g, trashBounds (r));
            }
            continue;
        }
        if (row.kind == Kind::NewPreset || row.kind == Kind::NewFolder || row.kind == Kind::OpenFolder)
        {
            auto& a = rowAnim (i); a.setTarget (i == hot ? 1.0f : 0.0f);
            if (a.get() > 0.0f) { g.setColour (juce::Colours::white.withAlpha (0.06f * a.get())); g.fillRoundedRectangle (r.reduced (0.0f, 1.5f), 4.0f); }
            g.setColour (textDim.interpolatedWith (text, juce::jmax (0.55f, a.get())));
            g.setFont (font (13.0f, true));
            g.drawText (row.text, r.withTrimmedLeft (22.0f), juce::Justification::centredLeft);
            continue;
        }

        auto& a = rowAnim (i); a.setTarget (i == hot ? 1.0f : 0.0f);
        const bool isCurrent = row.index >= 0 && row.text == current;
        if (a.get() > 0.0f)
        {
            const auto rr = r.reduced (0.0f, 1.5f);
            g.setColour (juce::Colours::white.withAlpha (0.06f * a.get()));
            g.fillRoundedRectangle (rr, 4.0f);
            g.setColour (juce::Colours::white.withAlpha (0.3f * a.get()));
            g.drawRoundedRectangle (rr, 4.0f, 1.0f);
        }
        if (isCurrent) { g.setColour (violet.brighter (0.3f)); g.fillEllipse (r.getX() + 9.0f, r.getCentreY() - 3.0f, 6.0f, 6.0f); }
        g.setColour (isCurrent ? text : textDim.interpolatedWith (text, a.get()));
        g.setFont (font (13.5f, isCurrent));
        g.drawText (row.text, r.withTrimmedLeft (22.0f).withTrimmedRight (26.0f), juce::Justification::centredLeft);
        if (i == hot && ! row.factory && ! dragging) drawTrash (g, trashBounds (r));
    }

    if (dragging && dragIndex >= 0)
    {
        const auto box = juce::Rectangle<float> (150.0f, 22.0f).withCentre (mouse.translated (0.0f, -14.0f));
        g.setColour (bg3.withAlpha (0.95f));
        g.fillRoundedRectangle (box, 5.0f);
        g.setColour (violet.withAlpha (0.6f));
        g.drawRoundedRectangle (box, 5.0f, 1.0f);
        juce::String nm;
        for (const auto& rr : rows) if (rr.index == dragIndex) nm = rr.text;
        g.setColour (text);
        g.setFont (font (12.5f, true));
        g.drawText (nm, box, juce::Justification::centred);
    }
}

void PresetPanel::mouseDown (const juce::MouseEvent& e)
{
    downPos = e.position; dragging = false; dragIndex = -1; dropRow = -1;
    const int i = rowAt (e.position);
    if (i >= 0 && rows[size_t (i)].kind == Kind::Preset && ! rows[size_t (i)].factory
        && ! trashBounds (rowBounds (i)).contains (e.position))
        dragIndex = rows[size_t (i)].index;
}

void PresetPanel::mouseDrag (const juce::MouseEvent& e)
{
    if (dragIndex < 0) return;
    if (! dragging && e.position.getDistanceFrom (downPos) > 5.0f) dragging = true;
    if (dragging)
    {
        const int i = rowAt (e.position);
        const bool isTarget = i >= 0 && (rows[size_t (i)].kind == Kind::Folder
                                         || (rows[size_t (i)].kind == Kind::Section && rows[size_t (i)].text == "My presets"));
        dropRow = isTarget ? i : -1;
        repaint();
    }
}

void PresetPanel::mouseUp (const juce::MouseEvent& e)
{
    if (dragging)
    {
        if (dragIndex >= 0 && dropRow >= 0 && onMove)
        {
            const auto& dr = rows[size_t (dropRow)];
            onMove (dragIndex, dr.kind == Kind::Folder ? dr.folder : juce::String());
        }
        dragging = false; dragIndex = -1; dropRow = -1; repaint();
        return;
    }
    dragIndex = -1;
    const int i = rowAt (e.position);
    if (i < 0) return;
    const auto& row = rows[size_t (i)];
    const auto r = rowBounds (i);
    switch (row.kind)
    {
        case Kind::Preset:
            if (! row.factory && trashBounds (r).contains (e.position)) { if (onDelete) onDelete (row.index); }
            else if (onLoad) onLoad (row.index);
            break;
        case Kind::Folder:
            if (plusBounds (r).contains (e.position)) { if (onSaveToFolder) onSaveToFolder (row.folder); }
            else if (trashBounds (r).contains (e.position)) { if (onDeleteFolder) onDeleteFolder (row.folder); }
            break;
        case Kind::NewPreset:  if (onNewPreset) onNewPreset(); break;
        case Kind::NewFolder:  if (onNewFolder) onNewFolder(); break;
        case Kind::OpenFolder: if (onOpenFolder) onOpenFolder(); break;
        default: break;
    }
}

void PresetPanel::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    const float maxScroll = juce::jmax (0.0f, contentHeight() - float (getHeight()));
    scroll = juce::jlimit (0.0f, maxScroll, scroll - w.deltaY * 120.0f);
    repaint();
}

void GroupBrowser::setData (const juce::StringArray& groupNames, const std::vector<GroupInstanceRow>& instances)
{
    rows.clear();
    for (int g = 0; g < 4; ++g)
    {
        rows.push_back ({ Kind::GroupHeader, g, 0, g < groupNames.size() ? groupNames[g] : "Group " + juce::String (g + 1), false });
        int count = 0;
        for (const auto& it : instances)
            if (it.group == g) { rows.push_back ({ Kind::Instance, g, it.id, it.label, it.self }); ++count; }
        if (count == 0) rows.push_back ({ Kind::Empty, g, 0, {}, false });
    }
    rows.push_back ({ Kind::GroupHeader, -1, 0, "Ungrouped", false });
    for (const auto& it : instances)
        if (it.group < 0) rows.push_back ({ Kind::Instance, -1, it.id, it.label, it.self });
    repaint();
}

float GroupBrowser::contentHeight() const
{
    float h = 8.0f;
    for (const auto& r : rows) h += rowHeight (r);
    return h + 8.0f;
}
int GroupBrowser::preferredHeight() const { return int (std::ceil (contentHeight())); }

theme::Anim& GroupBrowser::rowAnim (int i)
{
    while (int (hover.size()) <= i) hover.push_back (std::make_unique<theme::Anim> (*this, 0.3f));
    return *hover[size_t (i)];
}

juce::Rectangle<float> GroupBrowser::rowBounds (int i) const
{
    float y = 8.0f;
    for (int j = 0; j < i && j < int (rows.size()); ++j) y += rowHeight (rows[size_t (j)]);
    const float h = juce::isPositiveAndBelow (i, int (rows.size())) ? rowHeight (rows[size_t (i)]) : kRowH;
    return { 8.0f, y, float (getWidth()) - 16.0f, h };
}

int GroupBrowser::rowAt (juce::Point<float> p) const
{
    float y = 8.0f;
    for (int i = 0; i < int (rows.size()); ++i)
    {
        const float h = rowHeight (rows[size_t (i)]);
        if (p.y >= y && p.y < y + h) return i;
        y += h;
    }
    return -1;
}

void GroupBrowser::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto b = getLocalBounds().toFloat();
    juce::Path bg;
    bg.addRoundedRectangle (b.getX(), b.getY(), b.getWidth(), b.getHeight(), 8.0f, 8.0f);
    juce::DropShadow (juce::Colours::black.withAlpha (0.6f), 18, { 0, 4 }).drawForPath (g, bg);
    g.setColour (juce::Colour (0xff1c1c1e));
    g.fillPath (bg);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.strokePath (bg, juce::PathStrokeType (1.0f));

    juce::Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (getLocalBounds());
    const auto mouse = getMouseXYRelative().toFloat();
    const int hot = isMouseOver (true) ? rowAt (mouse) : -1;

    for (int i = 0; i < int (rows.size()); ++i)
    {
        const auto& row = rows[size_t (i)];
        const auto r = rowBounds (i);
        if (row.kind == Kind::GroupHeader)
        {
            if (dragging && dropRow == i) { g.setColour (violet.withAlpha (0.22f)); g.fillRoundedRectangle (r.reduced (1.0f), 4.0f); }
            g.setColour (row.group < 0 ? textFaint.brighter (0.2f) : (i == hot && ! dragging ? violet.brighter (0.4f) : text));
            g.setFont (font (12.5f, true));
            g.drawText (row.text.toUpperCase(), r.withTrimmedLeft (10.0f).withTrimmedRight (54.0f), juce::Justification::centredLeft);
            if (row.group >= 0 && i == hot && ! dragging)
            {
                g.setColour (textDim);
                g.setFont (font (10.5f, true));
                g.drawText ("rename", r.withTrimmedRight (10.0f), juce::Justification::centredRight);
            }
            continue;
        }
        if (row.kind == Kind::Empty)
        {
            g.setColour (textFaint.withAlpha (0.5f));
            g.setFont (font (11.5f, false));
            g.drawText (juce::String::fromUTF8 ("\xE2\x80\x94 empty \xE2\x80\x94"), r.withTrimmedLeft (26.0f), juce::Justification::centredLeft);
            continue;
        }
        auto& a = rowAnim (i); a.setTarget (i == hot ? 1.0f : 0.0f);
        if (a.get() > 0.0f) { g.setColour (juce::Colours::white.withAlpha (0.06f * a.get())); g.fillRoundedRectangle (r.reduced (0.0f, 1.5f), 4.0f); }
        if (row.self) { g.setColour (violet.brighter (0.3f)); g.fillEllipse (r.getX() + 12.0f, r.getCentreY() - 3.0f, 6.0f, 6.0f); }
        g.setColour (textDim.interpolatedWith (text, juce::jmax (row.self ? 0.9f : 0.5f, a.get())));
        g.setFont (font (13.0f, row.self));
        g.drawText (row.text + (row.self ? juce::String ("  (this)") : juce::String()), r.withTrimmedLeft (26.0f), juce::Justification::centredLeft);
    }

    if (dragging && dragId >= 0)
    {
        juce::String nm;
        for (const auto& rr : rows) if (rr.kind == Kind::Instance && rr.id == dragId) nm = rr.text;
        const auto box = juce::Rectangle<float> (150.0f, 22.0f).withCentre (mouse.translated (0.0f, -14.0f));
        g.setColour (bg3.withAlpha (0.95f));
        g.fillRoundedRectangle (box, 5.0f);
        g.setColour (violet.withAlpha (0.6f));
        g.drawRoundedRectangle (box, 5.0f, 1.0f);
        g.setColour (text); g.setFont (font (12.5f, true));
        g.drawText (nm, box, juce::Justification::centred);
    }
}

void GroupBrowser::mouseDown (const juce::MouseEvent& e)
{
    downPos = e.position; dragging = false; dragId = -1; dropRow = -1;
    const int i = rowAt (e.position);
    if (i >= 0 && rows[size_t (i)].kind == Kind::Instance) dragId = rows[size_t (i)].id;
}

void GroupBrowser::mouseDrag (const juce::MouseEvent& e)
{
    if (dragId < 0) return;
    if (! dragging && e.position.getDistanceFrom (downPos) > 5.0f) dragging = true;
    if (dragging)
    {
        const int i = rowAt (e.position);
        dropRow = (i >= 0 && rows[size_t (i)].kind == Kind::GroupHeader) ? i : -1;
        repaint();
    }
}

void GroupBrowser::mouseUp (const juce::MouseEvent& e)
{
    if (dragging)
    {
        if (dragId >= 0 && dropRow >= 0 && onAssign) onAssign (dragId, rows[size_t (dropRow)].group);
        dragging = false; dragId = -1; dropRow = -1; repaint();
        return;
    }
    dragId = -1;
    const int i = rowAt (e.position);
    if (i < 0) return;
    const auto& row = rows[size_t (i)];
    if (row.kind == Kind::GroupHeader && row.group >= 0 && onRename) onRename (row.group);
}

SavePresetCard::SavePresetCard()
{
    using namespace theme;
    name.setFont (font (15.0f, true));
    name.setJustification (juce::Justification::centredLeft);
    name.setIndents (10, 0);
    name.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0e0e0f));
    name.setColour (juce::TextEditor::outlineColourId, juce::Colours::white.withAlpha (0.12f));
    name.setColour (juce::TextEditor::focusedOutlineColourId, violet);
    name.setColour (juce::TextEditor::textColourId, text);
    name.setTextToShowWhenEmpty ("Preset name", textFaint);
    name.onReturnKey = [this] { save(); };
    name.onEscapeKey = [this] { if (onCancel) onCancel(); };
    saveButton.setToggleState (true, juce::dontSendNotification); // primary: always lit
    saveButton.onClick = [this] { save(); };
    cancelButton.onClick = [this] { if (onCancel) onCancel(); };
    for (auto* c : std::initializer_list<juce::Component*> { &name, &saveButton, &cancelButton })
        addAndMakeVisible (c);
}

void SavePresetCard::open (const juce::String& suggested, juce::String error)
{
    message = std::move (error);
    if (message.isEmpty()) name.setText (suggested, false);
    setVisible (true);
    toFront (true);
    name.grabKeyboardFocus();
    name.selectAll();
    repaint();
}

void SavePresetCard::resized()
{
    const auto c = card();
    name.setBounds (c.getX() + 24, c.getY() + 66, c.getWidth() - 48, 34);
    saveButton.setBounds (c.getCentreX() - 112, c.getBottom() - 54, 104, 36);
    cancelButton.setBounds (c.getCentreX() + 8, c.getBottom() - 54, 104, 36);
}

void SavePresetCard::paint (juce::Graphics& g)
{
    using namespace theme;
    g.fillAll (juce::Colours::black.withAlpha (0.55f)); // dim the plugin behind the card
    const auto c = card().toFloat();
    juce::Path p;
    p.addRoundedRectangle (c, 10.0f);
    juce::DropShadow (juce::Colours::black.withAlpha (0.7f), 24, { 0, 6 }).drawForPath (g, p);
    g.setColour (juce::Colour (0xff1c1c1e));
    g.fillPath (p);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.strokePath (p, juce::PathStrokeType (1.0f));
    g.setColour (text);
    g.setFont (font (17.0f, true));
    g.drawText ("Save preset", c.withHeight (52.0f).withTrimmedTop (14.0f), juce::Justification::centred);
    g.setColour (message.isEmpty() ? textDim : warn);
    g.setFont (font (12.5f));
    g.drawText (message.isEmpty() ? juce::String ("Name for this preset") : message,
                c.withTrimmedTop (42.0f).withHeight (20.0f), juce::Justification::centred);
}

void SavePresetCard::mouseUp (const juce::MouseEvent& e)
{
    if (! card().contains (e.getPosition()) && onCancel) onCancel();
}

void SettingsPanel::setItems (std::vector<Item> v)
{
    items = std::move (v);
    repaint();
}

int SettingsPanel::preferredHeight() const
{
    float h = 12.0f;
    for (const auto& it : items) h += rowHeight (it);
    return int (std::ceil (h));
}

theme::Anim& SettingsPanel::rowAnim (int i)
{
    while (int (hover.size()) <= i) hover.push_back (std::make_unique<theme::Anim> (*this, 0.3f));
    return *hover[size_t (i)];
}

int SettingsPanel::rowAt (juce::Point<float> p) const
{
    float y = 6.0f;
    for (int i = 0; i < int (items.size()); ++i)
    {
        const float h = rowHeight (items[size_t (i)]);
        if (p.y >= y && p.y < y + h) return items[size_t (i)].kind == Item::Kind::Choice ? i : -1;
        y += h;
    }
    return -1;
}

void SettingsPanel::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto b = getLocalBounds().toFloat();
    juce::Path bg;
    bg.addRoundedRectangle (b.getX(), b.getY(), b.getWidth(), b.getHeight(), 8.0f, 8.0f, false, false, true, true);
    juce::DropShadow (juce::Colours::black.withAlpha (0.6f), 16, { 0, 4 }).drawForPath (g, bg);
    g.setColour (juce::Colour (0xff1c1c1e));
    g.fillPath (bg);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.strokePath (bg, juce::PathStrokeType (1.0f));

    const auto mouse = getMouseXYRelative().toFloat();
    const int hot = isMouseOver (true) ? rowAt (mouse) : -1;
    float y = 6.0f;
    for (int i = 0; i < int (items.size()); ++i)
    {
        const auto& it = items[size_t (i)];
        const float h = rowHeight (it);
        const auto r = juce::Rectangle<float> (6.0f, y, b.getWidth() - 12.0f, h);
        y += h;
        if (it.kind == Item::Kind::Header)
        {
            if (it.text.isEmpty())
            {
                g.setColour (juce::Colours::white.withAlpha (0.08f));
                g.fillRect (r.getX() + 6.0f, r.getCentreY(), r.getWidth() - 12.0f, 1.0f);
            }
            else
            {
                g.setColour (textFaint.brighter (0.2f));
                g.setFont (font (11.5f, true));
                g.drawText (it.text.toUpperCase(), r.withTrimmedLeft (10.0f), juce::Justification::centredLeft);
            }
            continue;
        }
        if (it.kind == Item::Kind::Info)
        {
            g.setColour (textFaint);
            g.setFont (font (12.0f));
            g.drawText (it.text, r.withTrimmedLeft (22.0f), juce::Justification::centredLeft);
            continue;
        }
        auto& a = rowAnim (i);
        a.setTarget (i == hot ? 1.0f : 0.0f);
        if (a.get() > 0.0f)
        {
            const auto rr = r.reduced (0.0f, 1.5f);
            g.setColour (juce::Colours::white.withAlpha (0.06f * a.get()));
            g.fillRoundedRectangle (rr, 4.0f);
            g.setColour (juce::Colours::white.withAlpha (0.35f * a.get()));
            g.drawRoundedRectangle (rr, 4.0f, 1.0f);
        }
        if (it.current)
        {
            g.setColour (violet.brighter (0.3f));
            g.fillEllipse (r.getX() + 9.0f, r.getCentreY() - 3.0f, 6.0f, 6.0f);
        }
        g.setColour (it.current ? text : textDim.interpolatedWith (text, a.get()));
        g.setFont (font (13.5f, it.current));
        g.drawText (it.text, r.withTrimmedLeft (22.0f), juce::Justification::centredLeft);
        if (it.detail.isNotEmpty())
        {
            g.setColour (textFaint.brighter (0.3f));
            g.setFont (font (12.0f));
            g.drawText (it.detail, r.withTrimmedRight (10.0f), juce::Justification::centredRight);
        }
    }
}

void SettingsPanel::mouseUp (const juce::MouseEvent& e)
{
    const int i = rowAt (e.position);
    if (i >= 0 && onPick) onPick (items[size_t (i)].id);
}
