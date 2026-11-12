#include "Widgets.h"
#include "../ReleaseNotes.h"

namespace icons
{
static juce::Path tracedIcon (const char* outline)
{
    auto path = juce::Drawable::parseSVGPath (outline);
    path.setUsingNonZeroWinding (false);
    return path;
}
juce::Path undo() { return tracedIcon ("M 6.2 10.8 L 6.2 11.8 L 11.8 16.6 L 12.8 16.6 L 13.0 15.4 L 9.8 12.4 L 10.4 11.8 L 21.2 12.0 L 23.4 12.8 L 24.6 13.8 L 25.8 15.8 L 25.8 18.8 L 25.4 19.6 L 23.0 22.0 L 21.4 22.6 L 18.8 22.8 L 18.4 23.2 L 18.6 24.0 L 20.4 24.2 L 22.8 23.8 L 24.6 22.8 L 27.0 19.8 L 27.4 18.2 L 27.0 14.6 L 25.8 12.8 L 23.8 11.2 L 22.4 10.6 L 19.0 10.2 L 10.6 10.2 L 10.2 9.4 L 13.0 7.0 L 12.6 5.8 L 11.4 6.2 Z"); }
juce::Path redo() { return tracedIcon ("M 28.0 10.6 L 22.8 6.0 L 21.4 5.8 L 21.2 7.0 L 24.0 9.4 L 23.6 10.2 L 12.6 10.2 L 9.6 11.2 L 7.6 13.0 L 6.4 15.4 L 6.2 17.2 L 6.8 20.0 L 7.6 21.2 L 10.4 23.6 L 12.4 24.2 L 14.6 24.2 L 15.2 23.8 L 15.0 22.8 L 11.6 22.2 L 10.4 21.6 L 8.6 19.6 L 8.0 16.8 L 9.0 14.2 L 10.6 12.8 L 12.2 12.2 L 23.6 12.0 L 24.0 12.4 L 23.6 13.4 L 21.2 15.4 L 21.2 16.6 L 22.0 16.8 L 28.0 11.8 Z"); }
juce::Path copyArrow() { return tracedIcon ("M 9.6 7.4 L 8.8 7.4 L 8.2 8.0 L 8.2 8.8 L 14.4 14.8 L 14.6 15.6 L 8.0 22.2 L 8.2 23.2 L 9.0 23.6 L 10.0 23.2 L 17.0 15.8 L 17.0 14.8 Z"); }
juce::Path save() { return tracedIcon ("M 15.4 16.0 L 16.6 15.2 L 26.2 15.6 L 27.0 16.6 L 27.2 25.6 L 26.0 26.6 L 16.6 26.6 L 15.6 26.2 L 15.2 25.6 Z M 15.0 8.4 L 16.0 7.2 L 23.4 7.2 L 24.0 8.0 L 24.0 10.4 L 23.2 11.6 L 16.2 12.0 L 15.2 11.2 Z M 26.0 5.6 L 24.8 5.0 L 18.4 5.4 L 10.4 5.2 L 9.6 5.8 L 9.4 26.6 L 10.2 27.8 L 11.8 28.2 L 30.2 28.2 L 31.8 27.0 L 32.2 22.6 L 32.0 12.2 L 31.0 10.6 Z"); }
juce::Path menu() { return tracedIcon ("M 33.2 24.6 L 32.6 24.0 L 12.0 24.2 L 11.2 24.8 L 11.0 27.4 L 11.4 28.4 L 12.4 29.0 L 32.2 29.0 L 33.2 28.2 Z M 11.0 19.8 L 11.8 21.4 L 32.2 21.6 L 33.2 21.0 L 33.4 20.2 L 33.2 18.0 L 32.6 17.2 L 12.0 17.2 L 11.2 17.8 Z M 11.0 9.6 L 11.0 12.4 L 12.0 13.6 L 32.2 13.8 L 33.2 12.8 L 33.2 9.6 L 32.8 8.8 L 31.2 8.4 L 11.6 8.6 Z"); }
juce::Path palette() { return tracedIcon ("M 14.4 9.6 L 12.6 11.0 L 12.2 12.2 L 12.4 18.8 L 14.0 21.0 L 20.2 25.6 L 21.0 27.0 L 21.2 29.6 L 20.4 31.0 L 19.0 31.6 L 17.8 31.4 L 17.4 31.0 L 17.2 26.0 L 16.2 25.2 L 14.6 24.8 L 12.6 25.0 L 12.0 26.4 L 12.2 33.2 L 12.6 33.8 L 14.6 34.2 L 25.4 34.2 L 26.2 33.8 L 27.2 32.4 L 27.0 24.6 L 24.8 22.0 L 19.6 18.6 L 18.0 17.0 L 17.2 15.4 L 17.2 13.4 L 17.8 12.4 L 18.8 12.0 L 20.4 12.6 L 21.2 13.8 L 21.2 16.2 L 21.6 17.2 L 22.6 18.0 L 23.8 18.0 L 25.0 19.0 L 26.4 19.2 L 27.2 17.8 L 27.0 10.8 L 25.6 9.6 Z M 29.8 7.0 L 30.6 8.8 L 30.4 36.4 L 29.4 37.4 L 9.8 37.2 L 9.0 36.4 L 8.8 8.8 L 9.2 7.6 L 10.0 7.0 Z M 8.8 5.8 L 8.0 6.6 L 7.4 8.4 L 7.8 37.6 L 9.6 38.8 L 29.4 38.8 L 30.6 38.4 L 32.0 36.4 L 31.8 6.6 L 30.6 5.6 L 18.6 5.8 L 10.6 5.4 Z"); }
juce::Path chat() { return tracedIcon ("M 23.6 15.2 L 23.0 14.4 L 21.6 14.2 L 20.2 14.6 L 17.4 16.6 L 16.6 16.6 L 15.2 15.0 L 12.8 14.6 L 11.2 15.0 L 11.0 16.0 L 12.2 16.8 L 14.4 16.8 L 15.2 17.2 L 16.6 19.0 L 17.8 19.0 L 20.6 16.8 L 23.4 16.0 Z M 8.0 10.4 L 8.4 10.0 L 27.4 10.2 L 27.8 10.8 L 27.8 22.6 L 27.0 23.2 L 21.8 23.4 L 17.6 26.8 L 16.8 26.2 L 16.6 24.2 L 15.6 23.2 L 8.6 23.2 L 7.8 22.4 Z M 6.4 9.2 L 6.0 10.8 L 6.2 23.4 L 7.4 24.8 L 14.4 25.0 L 15.0 25.6 L 15.6 28.0 L 18.4 28.2 L 22.8 25.0 L 27.2 25.0 L 29.2 24.0 L 29.6 23.2 L 29.6 9.6 L 28.8 8.6 L 27.0 8.2 L 8.2 8.2 Z"); }
juce::Path grid() { return tracedIcon ("M 20.4 22.4 L 21.0 23.4 L 21.0 28.8 L 20.2 30.0 L 13.4 30.2 L 12.4 29.4 L 12.0 23.6 L 13.4 22.0 L 18.8 21.8 Z M 32.6 21.8 L 33.6 23.4 L 31.6 24.4 L 30.2 26.0 L 28.0 27.2 L 27.4 28.0 L 27.4 29.8 L 26.6 30.6 L 25.8 30.4 L 25.0 29.4 L 25.0 23.6 L 25.6 22.6 L 27.4 21.6 Z M 24.6 21.4 L 23.8 22.6 L 24.0 30.4 L 25.4 31.4 L 29.4 31.2 L 31.6 33.4 L 34.8 33.0 L 36.2 31.8 L 36.6 26.4 L 36.2 25.4 L 34.8 24.4 L 34.6 22.6 L 33.2 20.8 L 26.0 20.6 Z M 20.8 20.8 L 12.8 20.8 L 11.6 21.8 L 10.8 23.2 L 11.0 29.6 L 12.2 31.0 L 13.6 31.6 L 19.8 31.6 L 21.8 30.2 L 22.0 22.4 Z M 26.2 8.8 L 32.0 8.6 L 33.0 9.2 L 33.2 16.4 L 32.2 17.6 L 27.0 17.6 L 25.6 16.6 L 25.2 15.6 L 25.2 10.2 Z M 13.6 8.6 L 19.4 8.6 L 20.6 9.4 L 21.0 10.4 L 20.8 16.0 L 19.4 17.6 L 14.2 17.6 L 12.4 16.4 L 12.2 9.6 Z M 25.4 7.6 L 24.0 9.4 L 24.2 17.0 L 25.6 18.4 L 26.6 18.8 L 32.8 18.6 L 34.6 16.6 L 34.6 9.2 L 34.0 8.2 L 32.4 7.2 L 26.2 7.2 Z M 12.2 7.6 L 10.8 9.4 L 10.8 16.0 L 11.2 17.2 L 12.6 18.6 L 13.4 18.8 L 20.6 18.6 L 22.2 16.4 L 22.0 9.4 L 21.6 8.6 L 19.6 7.2 L 13.0 7.2 Z"); }
juce::Path eye() { return tracedIcon ("M 17.2 15.4 L 18.0 15.6 L 18.8 16.6 L 18.8 17.6 L 17.8 18.6 L 17.2 18.6 L 16.2 17.4 L 16.2 16.4 Z M 16.6 13.6 L 15.0 14.4 L 14.2 15.4 L 14.0 18.2 L 15.6 20.2 L 16.4 20.6 L 19.2 20.4 L 20.8 18.4 L 20.8 16.0 L 19.4 14.0 Z M 8.4 16.8 L 11.6 13.6 L 13.4 12.4 L 15.6 11.6 L 18.2 11.4 L 20.8 12.0 L 23.2 13.2 L 26.8 17.2 L 22.8 21.4 L 20.2 22.6 L 17.6 23.0 L 14.8 22.6 L 12.4 21.4 L 8.4 17.6 Z M 6.6 16.8 L 7.0 18.2 L 10.8 22.2 L 14.0 24.0 L 16.2 24.4 L 19.2 24.4 L 23.2 23.0 L 26.0 20.6 L 28.4 17.4 L 28.2 16.4 L 25.0 12.8 L 22.6 11.2 L 19.4 10.0 L 15.6 10.0 L 12.8 11.0 L 10.6 12.4 Z"); }
juce::Path power() { return tracedIcon ("M 17.8 15.6 L 15.2 18.6 L 14.0 22.4 L 14.2 26.0 L 15.0 28.4 L 17.0 31.4 L 20.0 33.6 L 23.6 34.8 L 28.4 34.6 L 32.8 32.4 L 35.6 29.0 L 36.8 25.4 L 36.8 21.8 L 36.4 20.4 L 34.2 16.6 L 33.0 15.6 L 31.8 15.6 L 31.0 16.6 L 34.2 21.6 L 34.2 25.2 L 32.2 29.2 L 30.0 31.0 L 28.2 31.8 L 23.6 32.2 L 22.4 32.0 L 19.6 30.4 L 17.8 28.4 L 16.6 26.0 L 16.4 23.4 L 17.0 20.8 L 19.6 17.4 L 19.4 16.4 L 18.6 15.6 Z M 25.0 10.0 L 24.2 10.8 L 24.2 22.6 L 24.8 23.2 L 26.0 23.2 L 26.8 22.4 L 26.8 11.4 L 26.0 10.0 Z"); }
juce::Path swap() { return tracedIcon ("M 24.8 19.2 L 23.8 19.0 L 22.6 21.6 L 20.8 22.8 L 12.6 22.6 L 12.4 22.0 L 13.2 20.2 L 12.2 19.8 L 8.6 22.8 L 8.4 23.8 L 12.4 27.2 L 12.8 27.0 L 12.4 25.6 L 12.8 24.6 L 19.6 24.4 L 22.4 23.8 L 24.0 22.2 L 24.8 20.6 Z M 25.0 13.4 L 21.2 9.8 L 20.2 9.8 L 20.0 10.6 L 21.2 12.2 L 20.8 12.8 L 11.6 13.0 L 9.2 14.8 L 8.2 17.2 L 8.8 18.2 L 9.4 18.2 L 10.4 15.8 L 11.2 15.0 L 13.4 14.4 L 21.0 14.4 L 21.2 15.4 L 20.2 16.2 L 20.2 17.0 L 21.4 17.2 L 24.8 14.6 Z"); }

juce::Path reset()
{
    juce::Path p;
    p.addCentredArc (12, 12, 7, 7, 0, 0.6f, 5.6f, true);
    p.startNewSubPath (13.5f, 1.5f); p.lineTo (16.5f, 5.0f); p.lineTo (12.5f, 7.0f);
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
juce::Path panel()
{
    juce::Path p;
    p.addRoundedRectangle (4.0f, 5.0f, 16.0f, 14.0f, 1.5f);
    p.startNewSubPath (4, 10); p.lineTo (20, 10);
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
juce::Path chevronUp() { return tracedIcon ("M 12.7 13.0 L 13.0 14.0 L 14.0 14.0 L 18.3 9.8 L 22.3 13.8 L 23.2 13.8 L 23.5 13.5 L 23.5 12.5 L 18.7 7.5 L 17.7 7.5 L 13.0 12.3 Z"); }
juce::Path chevronDown() { return tracedIcon ("M 23.5 8.5 L 22.5 8.5 L 18.3 12.8 L 17.7 12.7 L 14.3 9.0 L 13.5 8.5 L 13.0 9.2 L 17.3 13.5 L 17.7 14.2 L 18.5 14.3 L 19.5 13.0 L 23.2 9.5 Z"); }
juce::Path key()
{
    juce::Path p;
    p.addEllipse (3.0f, 7.0f, 9.0f, 9.0f);                 // bow (ring)
    p.startNewSubPath (11.5f, 11.5f); p.lineTo (21.0f, 11.5f); // shaft
    p.startNewSubPath (17.5f, 11.5f); p.lineTo (17.5f, 15.0f); // teeth
    p.startNewSubPath (20.5f, 11.5f); p.lineTo (20.5f, 14.0f);
    return p;
}
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
    auto drawIcon = [&] (const juce::Path& path, juce::Rectangle<float> bounds, float width = 1.7f)
    {
        if (useFilledIcons)
            g.fillPath (path, path.getTransformToScaleToFit (bounds, true));
        else
            strokeIcon (g, path, bounds, width);
    };
    const float h = hover;
    auto r = getLocalBounds().toFloat();
    if (hoverFill && h > 0.0f)
    {
        g.setColour (juce::Colours::white.withAlpha ((down ? 0.10f : 0.06f) * h));
        g.fillRoundedRectangle (r.reduced (1.0f), 4.0f);
    }
    const bool on = getToggleState();
    const auto rest = lightMode || idleColour.isTransparent() ? textDim : idleColour;
    const auto disabled = lightMode || idleColour.isTransparent() ? textFaint.interpolatedWith (textDim, 0.18f)
                                                                  : rest.interpolatedWith (headerBox, 0.54f);
    const auto col = isEnabled() ? (on ? onColour : rest.interpolatedWith (theme::text, h)) : disabled;
    const float is = juce::jmin (r.getWidth(), r.getHeight());
    if (caption.isEmpty())
    {
        g.setColour (col);
        const float s = is * iconScale * (1.0f + 0.06f * h);
        drawIcon (icon, juce::Rectangle<float> (s, s).withCentre (r.getCentre() + iconOffset));
        return;
    }
    // Icons on the left, caption on the right.
    const float s = juce::jmin (icon2.isEmpty() ? 19.0f : 17.0f, is * 0.6f);
    const auto centre = juce::Point<float> (r.getX() + 8.0f + s * 0.5f, r.getCentreY()) + iconOffset;
    g.setColour (col);
    drawIcon (icon, juce::Rectangle<float> (s, s).withCentre (centre));
    if (! icon2.isEmpty())
    {
        g.setColour ((lightMode ? textDim : juce::Colour (0xffd3d3d3)).interpolatedWith (theme::text, h * 0.6f));
        drawIcon (icon2, juce::Rectangle<float> (s * 0.86f, s * 0.86f).withCentre (centre.translated (28.0f, 0.0f)), 1.5f);
    }
    auto capCol = (lightMode ? textDim : juce::Colour (0xffd7d7d7)).interpolatedWith (theme::text, h * 0.55f);
    if (on && onColour != theme::text) capCol = onColour;
    const float inset = captionInset >= 0.0f ? captionInset : s + 22.0f;
    g.setFont (font (16.0f));
    shadowFittedText (g, caption, r.withTrimmedLeft (inset).withTrimmedRight (2.0f), juce::Justification::centredLeft,
                      2, isEnabled() ? capCol : textFaint, 0.33f);
}

void PillButton::paintButton (juce::Graphics& g, bool, bool)
{
    using namespace theme;
    const auto r = getLocalBounds().toFloat();
    const bool on = getToggleState();
    g.setFont (font (22.7f).withHorizontalScale (0.97f));
    const auto rest = lightMode ? textDim : juce::Colour (0xffc8c8c8);
    shadowText (g, getButtonText(), r, juce::Justification::centred,
                on ? rest.interpolatedWith (theme::text, 0.28f) : rest.interpolatedWith (theme::text, 0.45f * hover), 0.7f, 0.42f);
}

void TextPill::paintButton (juce::Graphics& g, bool, bool down)
{
    using namespace theme;
    auto r = getLocalBounds().toFloat().reduced (2.0f);
    const bool splitLeft = kind == Kind::SplitLeft, splitRight = kind == Kind::SplitRight;
    if (splitLeft) r = r.withRight (float (getWidth()));
    if (splitRight) r = r.withX (0.0f);
    const bool on = getToggleState();

    const float h = hover, l = on ? 1.0f : 0.0f;
    juce::Path p;
    const float radius = r.getHeight() * 0.5f;
    p.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), radius, radius,
                           ! splitRight, ! splitLeft, ! splitRight, ! splitLeft);
    if (kind == Kind::Primary)
    {
        if (l > 0.0f)
        {
            g.setColour (juce::Colours::black.withAlpha (0.34f));
            g.fillPath (p, juce::AffineTransform::translation (0.0f, 1.6f));
            const auto centre = r.getCentre().translated (-r.getWidth() * 0.11f, -r.getHeight() * 0.05f);
            juce::ColourGradient selected (recolour (dark::notePill).brighter (0.18f), centre,
                                           recolour (dark::noteShade), centre.translated (r.getWidth() * 0.63f, 0.0f), true);
            selected.addColour (0.52, recolour (dark::notePill));
            g.setGradientFill (selected);
            g.fillPath (p);
        }
    }
    else
    {
        g.setColour (juce::Colours::black.withAlpha (0.28f));
        g.fillPath (p, juce::AffineTransform::translation (0.0f, 1.35f));
        const auto base = lightMode ? bg3 : dark::control;
        const auto accent = selectedAccent.isTransparent() ? recolour (dark::notePill) : selectedAccent;
        const auto selected = base.interpolatedWith (accent, 0.64f * l);
        g.setGradientFill (juce::ColourGradient (selected.brighter (0.048f), r.getTopLeft(),
                                                 selected.darker (0.22f), r.getBottomLeft(), false));
        g.fillPath (p);
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (p);
        g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.16f), r.getX(), r.getY(), juce::Colours::transparentBlack, r.getX(), r.getY() + 7.0f, false));
        g.fillRect (r);
    }
    if (h > 0.0f)
    {
        g.setColour (juce::Colours::white.withAlpha ((down ? 0.12f : 0.06f) * h));
        g.fillPath (p);
    }
    g.setGradientFill (juce::ColourGradient (lightMode ? line : theme::text.withAlpha (0.17f + 0.10f * h), r.getTopLeft(),
                                             lightMode ? line : juce::Colours::black.withAlpha (0.48f), r.getBottomLeft(), false));
    g.strokePath (p, juce::PathStrokeType (0.85f));
    g.setFont (font (16.0f, on && kind == Kind::Primary));
    const auto idleText = lightMode ? textDim : juce::Colour (0xffc6c6c6);
    const float emphasis = kind == Kind::Primary ? l : 0.52f * l;
    shadowText (g, getButtonText(), r, juce::Justification::centred,
                idleText.interpolatedWith (theme::text, juce::jmax (emphasis, 0.7f * h)), 0.75f, 0.38f);
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
    shadowText (g, getButtonText(), r.withTrimmedLeft (r.getHeight() * 0.95f).withTrimmedRight (8.0f), juce::Justification::centred, juce::Colours::white);
}

void ToggleSwitch::paintButton (juce::Graphics& g, bool, bool)
{
    using namespace theme;
    const bool on = getToggleState();
    if (first) { pos.snapTo (on ? 1.0f : 0.0f); first = false; }
    pos.setTarget (on ? 1.0f : 0.0f);
    const float t = pos, h = hover;
    const auto r = getLocalBounds().toFloat().withSizeKeepingCentre (float (getWidth()), 16.0f);
    const auto off = warmAccent ? dark::timingTrack : (lightMode ? bg3 : dark::control);
    const auto active = recolour (juce::Colour (0xff6e6094));
    const auto top = off.brighter (0.06f * h).interpolatedWith (active.brighter (0.13f), t);
    const auto bottom = off.darker (0.20f).interpolatedWith (active.darker (0.25f), t);
    g.setGradientFill (juce::ColourGradient (top, r.getTopLeft(), bottom, r.getBottomLeft(), false));
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    g.setColour (juce::Colours::white.withAlpha (0.105f * t));
    g.drawRoundedRectangle (r.reduced (0.5f), r.getHeight() * 0.5f - 0.5f, 0.8f);
    const float d = r.getHeight() - 4.8f;
    const float x = r.getX() + 2.4f + t * (r.getWidth() - d - 4.8f);
    juce::Path knob;
    knob.addEllipse (x, r.getY() + 2.4f, d, d);
    juce::DropShadow (juce::Colours::black.withAlpha (0.39f), 4, { 0, 1 }).drawForPath (g, knob);
    const auto thumb = (warmAccent ? dark::timingAmber : juce::Colour (0xff9c9ba3)).interpolatedWith (lavender, juce::jmax (t, 0.4f * h));
    g.setGradientFill (juce::ColourGradient (thumb.brighter (0.18f), x + d * 0.35f, r.getY() + 3.0f,
                                             thumb.darker (0.09f), x + d * 0.7f, r.getBottom() - 2.4f, false));
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
    const auto sel = juce::Rectangle<float> (r.getX() + w * slide.get(), r.getY(), w, r.getHeight()).reduced (4.0f, 3.0f);
    const auto centre = sel.getCentre();
    const auto accent = warmAccent ? dark::timingAmber : text;
    juce::ColourGradient glow (accent.withAlpha (warmAccent ? 0.23f : 0.16f), centre,
                                accent.withAlpha (0.0f), centre.translated (sel.getWidth() * 0.51f, 0.0f), true);
    glow.addColour (0.50, accent.withAlpha (0.075f));
    g.setGradientFill (glow);
    g.fillRoundedRectangle (sel, sel.getHeight() * 0.5f);
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
        g.setFont (font (15.2f));
        const auto idle = lightMode ? textDim : juce::Colour (0xffcecece);
        shadowText (g, labels[i], cellR, juce::Justification::centred,
                    idle.interpolatedWith (accent, juce::jmax (near, 0.7f * h)), 0.75f, 0.43f);
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

float GroupTabs::rowHeight() const
{
    return float (getHeight() - kTop - arrowRailHeight) / 4.0f;
}

int GroupTabs::indexAt (juce::Point<float> p) const
{
    const float y = p.y - float (kTop);
    return y >= 0.0f && y < rowHeight() * 4.0f ? int (y / rowHeight()) : -1;
}

void GroupTabs::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto b = getLocalBounds().toFloat();
    const auto rail = b.withTrimmedBottom (float (arrowRailHeight));
    juce::Path shell;
    shell.addRoundedRectangle (rail.getX() - 13.0f, rail.getY(), rail.getWidth() + 13.0f, rail.getHeight(), 21.0f);
    g.setColour (lightMode ? menuBg2 : dark::rail);
    g.fillPath (shell);
    {
        juce::Graphics::ScopedSaveState clip (g);
        g.reduceClipRegion (shell);
        const auto cap = rail.withHeight (float (kTop));
        const auto capTop = (lightMode ? bg3 : dark::railCap).interpolatedWith (bypassAccent, 0.19f * bypassBlend);
        g.setGradientFill (juce::ColourGradient (capTop, cap.getTopLeft(),
                                                 lightMode ? menuBg2 : dark::railRow, cap.getBottomLeft(), false));
        g.fillRect (cap);
        if (browseActive)
        {
            g.setColour (menuSel);
            g.fillRect (cap.reduced (3.0f));
        }
        g.setColour ((browseActive ? text : textDim).interpolatedWith (bypassAccent, bypassBlend));
        static const auto grid = icons::grid();
        g.fillPath (grid, grid.getTransformToScaleToFit (juce::Rectangle<float> (19.8f, 19.8f)
                                                          .withCentre ({ 31.0f, 32.0f }), true));
        if (! lightMode)
        {
            const auto spine = rail.withTop (float (kTop)).withWidth (22.0f);
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff232323), spine.getTopLeft(),
                                                     dark::railRow, spine.getTopRight(), false));
            g.fillRect (spine);
        }

        for (int i = 0; i < 4; ++i)
        {
            const auto row = rail.withY (float (kTop) + rowHeight() * float (i)).withHeight (rowHeight());
            selAmt[i].setTarget (i == selected ? 1.0f : 0.0f);
            const float selectedAmount = selAmt[i], hovered = hoverAmt[i];
            const auto base = lightMode ? menuBg2 : dark::railRow;
            g.setColour (base.interpolatedWith (menuSel, juce::jmax (selectedAmount, 0.6f * hovered)));
            const auto cell = row.withTrimmedLeft (lightMode ? 0.0f : 22.0f);
            g.fillRect (cell);
            g.setColour (lightMode ? line : juce::Colour (0xff303030));
            g.fillRect (cell.withHeight (0.65f));
            g.setFont (font (14.8f, true));
            shadowText (g, juce::String (counts[i]), row.withTrimmedLeft (36.0f).withTrimmedRight (2.0f).withHeight (18.0f),
                        juce::Justification::centred, counts[i] > 0 ? lavender : lavender.withAlpha (0.93f), 0.7f, 0.5f);
            g.setFont (font (20.7f).withHorizontalScale (0.98f));
            shadowText (g, juce::String (i + 1), row.withTrimmedLeft (26.0f).withTrimmedRight (5.0f).withTrimmedTop (18.0f).withHeight (28.0f),
                        juce::Justification::centred, text.withAlpha (0.94f + 0.06f * selectedAmount), 0.8f, 0.45f);
        }
    }

    const auto arrows = b.withTop (rail.getBottom() - 16.0f).withWidth (29.0f);
    juce::Path lip;
    lip.addRoundedRectangle (arrows.getX() - 6.0f, arrows.getY(), arrows.getWidth() + 6.0f, arrows.getHeight(), 13.0f);
    g.setGradientFill (juce::ColourGradient (lightMode ? menuBg2 : dark::railCap, arrows.getTopLeft(),
                                             lightMode ? bg3 : dark::railRow, arrows.getBottomLeft(), false));
    g.fillPath (lip);
    for (int i = 0; i < 2; ++i)
    {
        const float y = arrows.getY() + 8.0f + float (i) * 23.0f;
        juce::Path arrow;
        arrow.addTriangle (10.0f, y - 1.3f, 23.0f, y - 1.3f, 16.5f, y - 7.4f);
        arrow.addTriangle (10.0f, y + 1.3f, 23.0f, y + 1.3f, 16.5f, y + 7.4f);
        g.setColour (text);
        g.fillPath (arrow);
    }
}

void GroupTabs::mouseDown (const juce::MouseEvent& e)
{
    const float arrowsTop = float (getHeight() - arrowRailHeight);
    if (e.position.y >= arrowsTop - 16.0f && e.position.x < 29.0f)
    {
        const bool up = e.position.y < arrowsTop + 4.0f;
        const int next = selected < 0 ? (up ? 3 : 0) : juce::jlimit (0, 3, selected + (up ? -1 : 1));
        if (onSelect && next != selected) onSelect (next);
        return;
    }

    const int i = indexAt (e.position);
    if (i >= 0) { if (onSelect) onSelect (i); }
    else if (e.position.y < float (kTop) && onBrowse) onBrowse();
}

void GroupTabs::mouseMove (const juce::MouseEvent& e)
{
    const int i = indexAt (e.position);
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
    g.setColour (lightMode ? headerBox : juce::Colour (0xff1e1c1d));
    g.fillRect (main);
    if (over && main.contains (mouse))
    {
        g.setColour (juce::Colours::white.withAlpha (0.04f));
        g.fillRect (main);
    }
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRect (arrows.getX(), b.getY(), 1.0f, b.getHeight());
    g.fillRect (arrows.getX(), b.getCentreY(), arrows.getWidth(), 0.7f);

    // Name in an oblique style.
    {
        juce::Graphics::ScopedSaveState ss (g);
        const auto tr = main.withTrimmedLeft (18.0f).withTrimmedRight (47.0f);
        g.addTransform (juce::AffineTransform::shear (-0.18f, 0.0f).translated (0.18f * tr.getCentreY(), 0.0f));
        g.setFont (font (16.0f));
        shadowText (g, name.isEmpty() ? juce::String ("No preset selected") : name, tr, juce::Justification::centredLeft, name.isEmpty() ? text.withAlpha (0.9f) : text);
    }

    // Traced preset emblem, independent of the preset name and its hit area.
    {
        static const auto emblem = icons::tracedIcon ("M 32.6 21.8 L 31.6 21.6 L 30.4 23.4 L 29.6 23.4 L 28.2 22.0 L 27.6 22.0 L 27.2 23.4 L 28.6 24.6 L 30.2 24.8 L 32.0 24.0 L 32.8 23.0 Z M 8.8 22.2 L 9.0 23.2 L 9.6 23.8 L 11.6 24.6 L 14.4 23.8 L 15.0 22.8 L 15.0 22.2 L 14.0 21.8 L 12.6 23.2 L 11.4 23.2 L 10.6 22.4 L 10.4 21.2 L 9.2 21.6 Z M 9.4 10.0 L 9.6 10.8 L 10.6 11.2 L 11.2 12.0 L 11.4 19.0 L 17.2 20.8 L 18.4 20.6 L 19.6 21.0 L 21.4 20.4 L 28.6 20.2 L 30.4 18.0 L 30.6 11.6 L 32.0 10.4 L 31.8 9.6 L 30.4 8.8 L 28.6 9.0 L 27.6 9.8 L 27.6 11.0 L 28.6 12.2 L 28.6 16.8 L 27.2 18.4 L 25.4 18.4 L 25.0 18.0 L 24.8 13.4 L 24.0 12.0 L 23.0 12.0 L 22.6 12.4 L 22.2 17.4 L 21.2 18.4 L 19.4 18.4 L 18.8 17.6 L 18.6 12.4 L 18.0 11.6 L 17.2 11.6 L 16.6 12.6 L 16.6 17.6 L 16.0 18.4 L 14.8 18.6 L 14.0 18.4 L 13.0 17.4 L 13.0 11.8 L 14.4 10.2 L 14.2 9.2 L 11.8 8.6 L 10.0 9.2 Z");
        const auto bounds = juce::Rectangle<float> (18.6f, 13.2f).withCentre ({ main.getRight() - 23.0f, main.getCentreY() + 0.4f });
        g.setGradientFill (juce::ColourGradient (lavender, bounds.getTopLeft(), text, bounds.getBottomLeft(), false));
        g.fillPath (emblem, emblem.getTransformToScaleToFit (bounds, true));
    }

    const auto up = arrows.withHeight (arrows.getHeight() * 0.5f), down = arrows.withTrimmedTop (arrows.getHeight() * 0.5f);
    for (auto [r, ic] : { std::pair { up, icons::chevronUp() }, std::pair { down, icons::chevronDown() } })
    {
        const bool top = r.getY() == up.getY();
        const auto arrowColour = lightMode ? textDim : juce::Colour (top ? 0xffb9b9b9 : 0xff838383);
        const auto centre = r.getCentre().translated (-1.0f, top ? 3.4f : -0.6f);
        g.setColour (over && r.contains (mouse) ? text : arrowColour);
        g.fillPath (ic, ic.getTransformToScaleToFit (juce::Rectangle<float> (9.0f, 9.0f).withCentre (centre), true));
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
    using namespace theme;
    const auto track = getLocalBounds().toFloat().withSizeKeepingCentre (float (getWidth()), 17.0f);
    const auto inner = track.reduced (3.0f, 3.0f);
    const float x = inner.getX() + float (valueToProportionOfLength (getValue())) * inner.getWidth();
    g.setColour (lightMode ? well : juce::Colours::black);
    g.fillRoundedRectangle (track, track.getHeight() * 0.5f);
    {
        juce::Graphics::ScopedSaveState clip (g);
        g.reduceClipRegion (inner.withRight (x).getSmallestIntegerContainer());
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff64141c), inner.getTopLeft(),
                                                 juce::Colour (0xffba8991), inner.getTopLeft().withX (x), false));
        g.fillRoundedRectangle (inner, inner.getHeight() * 0.5f);
    }
    g.setColour (text.withAlpha (0.14f));
    g.drawRoundedRectangle (track.reduced (0.4f), track.getHeight() * 0.5f - 0.4f, 0.8f);
    g.setColour (text.withAlpha (isMouseOverOrDragging() ? 1.0f : 0.86f));
    g.fillRect (x - 0.8f, track.getY() + 1.8f, 1.6f, track.getHeight() - 3.6f);
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
    g.setColour (lightMode ? bg0 : juce::Colours::black);
    g.fillRoundedRectangle (track, track.getHeight() * 0.5f);
    g.setColour (text.withAlpha (0.13f));
    g.drawRoundedRectangle (track.reduced (0.4f), track.getHeight() * 0.5f - 0.4f, 0.8f);
    const float usable = track.getWidth() - 6.0f;
    const float x  = track.getX() + 3.0f + float (valueToProportionOfLength (getValue())) * usable;
    const float cx = track.getX() + 3.0f + float (valueToProportionOfLength (440.0)) * usable; // centre = 440 Hz / 0 cts
    // Red fill grows out from the centre toward the handle, either direction (bright at the handle).
    if (std::abs (x - cx) > 0.5f)
    {
        juce::Graphics::ScopedSaveState s (g);
        const auto fill = juce::Rectangle<float> (juce::jmin (cx, x), track.getY() + 3.0f, std::abs (x - cx), track.getHeight() - 6.0f);
        g.reduceClipRegion (fill.getSmallestIntegerContainer());
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff280c10), cx, 0.0f, juce::Colour (0xffae737d), x, 0.0f, false));
        g.fillRoundedRectangle (track, track.getHeight() * 0.5f);
    }
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillRect (cx - 0.5f, track.getY() + 3.0f, 1.0f, track.getHeight() - 6.0f); // faint centre tick
    g.setColour (juce::Colours::white.withAlpha (isMouseOverOrDragging() ? 1.0f : 0.9f));
    g.fillRect (x - 1.0f, track.getY() + 2.0f, 2.0f, track.getHeight() - 4.0f);
    const double hz = getValue();
    const int cents = juce::roundToInt (1200.0 * std::log2 (hz / 440.0));
    const auto offset = (cents > 0 ? "+" : "") + juce::String (cents) + "cts ";
    const auto value = juce::String (hz, 1) + "Hz";
    const auto offsetFont = font (14.1f), valueFont = font (14.1f, true);
    const float leftWidth = juce::GlyphArrangement::getStringWidth (offsetFont, offset);
    const float width = leftWidth + juce::GlyphArrangement::getStringWidth (valueFont, value);
    auto caption = r.withTrimmedTop (1.0f).withSizeKeepingCentre (width + 1.0f, r.getHeight() - 1.0f);
    g.setFont (offsetFont);
    shadowText (g, offset, caption.removeFromLeft (leftWidth), juce::Justification::centredLeft, text.withAlpha (0.92f), 0.7f, 0.35f);
    g.setFont (valueFont);
    shadowText (g, value, caption, juce::Justification::centredLeft, text, 0.7f, 0.35f);
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
    const float w = float (getWidth()) * 0.5f;
    return { w * float (i), 0.0f, w, float (getHeight()) * 0.10f };
}

juce::Rectangle<float> RootPicker::noteRect (int pc) const
{
    const float w = float (getWidth()) / 3.0f;
    const float tabs = tabRect (0).getBottom(), h = (float (getHeight()) - tabs) * 0.125f;
    return { w * float (pc % 3), tabs + h * float (pc / 3), w, h };
}

juce::Rectangle<float> RootPicker::scaleRect (int i) const
{
    const float w = float (getWidth()) * 0.5f, top = noteRect (11).getBottom();
    const float h = (float (getHeight()) - top) * 0.25f;
    if (i == 6) return { 0.0f, top + 3.0f * h, 2.0f * w, h };
    return { w * float (i % 2), top + h * float (i / 2), w, h };
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

    const auto activeInk = lightMode ? text : juce::Colour (0xffeae8e6);
    const auto inactiveInk = lightMode ? textDim : juce::Colour (0xff868686);
    if (lightMode)
    {
        g.setColour (bg3);
        g.fillRect (b);
    }
    else
    {
        const auto rootBounds = b.withTop (tabRect (0).getBottom()).withBottom (noteRect (11).getBottom());
        const auto scaleBounds = b.withTop (rootBounds.getBottom());
        const auto fill = [&] (juce::Rectangle<float> area, juce::uint32 left, juce::uint32 right)
        {
            g.setGradientFill (juce::ColourGradient (juce::Colour (left), area.getTopLeft(),
                                                     juce::Colour (right), area.getTopRight(), false));
            g.fillRect (area);
        };
        fill (b.withBottom (rootBounds.getY()), 0xff2f2f2f, 0xff202020);
        fill (rootBounds, 0xff393939, 0xff272727);
        fill (scaleBounds, 0xff373737, 0xff212121);
    }
    g.setColour (lightMode ? bg0 : juce::Colour (0xff151515));
    g.fillRect (b.withTrimmedLeft (b.getWidth() - 1.0f));
    g.setColour (activeInk.withAlpha (0.08f));
    g.fillRect (b.withY (tabRect (0).getBottom() - 1.0f).withHeight (1.0f));

    int cell = 0;
    const auto hoverAt = [&] (juce::Rectangle<float> r)
    {
        auto& animation = cellAnim (cell++);
        animation.setTarget (over && r.contains (mouse) ? 1.0f : 0.0f);
        return animation.get();
    };
    const auto highlight = [&] (juce::Rectangle<float> r, bool selected, float hover)
    {
        if (selected || hover > 0.0f)
        {
            g.setColour (activeInk.withAlpha (0.033f * float (selected) + 0.035f * hover));
            g.fillRect (r);
        }
        if (hover > 0.0f)
        {
            g.setColour (activeInk.withAlpha (0.60f * hover));
            g.drawRect (r.reduced (1.0f, 0.75f), 1.0f);
        }
    };

    for (int i = 0; i < 2; ++i)
    {
        const auto r = tabRect (i);
        const bool selected = (i == 0) == flats;
        const float hover = hoverAt (r);
        if (selected || hover > 0.0f)
        {
            const auto fill = lightMode ? text.withAlpha (0.12f) : juce::Colour (0xff3a3a3a);
            g.setColour (fill.withMultipliedAlpha (selected ? 1.0f : 0.65f * hover));
            juce::Path tab;
            tab.addRoundedRectangle (r.getX() + 0.5f, r.getY() + 1.0f, r.getWidth() - 1.0f, r.getHeight() - 1.0f,
                                     6.0f, 6.0f, true, true, false, false);
            g.fillPath (tab);
        }
        g.setFont (font (15.0f, selected));
        shadowText (g, i == 0 ? juce::String ("Flats") : juce::String ("Sharps"), r,
                    juce::Justification::centred, inactiveInk.interpolatedWith (activeInk, selected ? 1.0f : hover));
    }

    // The small accidental sits high beside the letter.
    const auto noteFont = font (20.0f, true);
    const auto accidentalFont = font (12.0f, true);
    const juce::String symbol (flats ? "b" : "#");
    const float symbolWidth = juce::GlyphArrangement::getStringWidth (accidentalFont, symbol);
    for (int pc = 0; pc < 12; ++pc)
    {
        const auto r = noteRect (pc);
        const bool selected = pc == root;
        const float hover = hoverAt (r);
        highlight (r, selected, hover);

        const juce::String letter (flats ? flatLetters[pc] : letters[pc]);
        const auto ink = inactiveInk.interpolatedWith (activeInk, selected ? 1.0f : 0.82f * hover);
        const float letterWidth = juce::GlyphArrangement::getStringWidth (noteFont, letter);
        const float x = r.getCentreX() - (letterWidth + (accidental[pc] ? symbolWidth : 0.0f)) * 0.5f;
        g.setColour (ink);
        g.setFont (noteFont);
        g.drawText (letter, r.withX (x).withWidth (letterWidth + 1.0f), juce::Justification::centredLeft, false);
        if (accidental[pc])
        {
            g.setFont (accidentalFont);
            g.drawText (symbol, r.withX (x + letterWidth).withWidth (symbolWidth + 1.0f).translated (0.0f, -3.0f),
                        juce::Justification::centredLeft, false);
        }
    }

    // Chromatic has its own button below the drawer.
    for (int i = 0; i < scaleNames().size() - 1; ++i)
    {
        const auto r = scaleRect (i);
        const bool selected = i == scale;
        const float hover = hoverAt (r);
        highlight (r, selected, hover);
        g.setFont (font (14.0f, selected));
        shadowText (g, scaleNames()[i], r, juce::Justification::centred,
                    inactiveInk.interpolatedWith (activeInk, selected ? 1.0f : 0.82f * hover));
    }
}

int RootPicker::rootAt (juce::Point<float> point) const
{
    for (int pc = 0; pc < 12; ++pc)
        if (noteRect (pc).contains (point)) return pc;
    return -1;
}

int RootPicker::scaleAt (juce::Point<float> point) const
{
    for (int i = 0; i < scaleNames().size() - 1; ++i)
        if (scaleRect (i).contains (point)) return i;
    return -1;
}

void RootPicker::mouseUp (const juce::MouseEvent& e)
{
    for (int i = 0; i < 2; ++i)
        if (tabRect (i).contains (e.position)) { if (onFlats) onFlats (i == 0); return; }
    if (const int pc = rootAt (e.position); pc >= 0) { if (onRoot) onRoot (pc); return; }
    if (const int i = scaleAt (e.position); i >= 0 && onScale) onScale (i);
}

namespace
{
const auto browserMuted = juce::Colour (0xff888888);
const auto browserBright = juce::Colour (0xffe5e5e5);
const auto browserHover = juce::Colour (0xff333333);
const auto browserLilac = juce::Colour (0xffaa98ed);
const auto browserGold = juce::Colour (0xffdfbd8c);

void browserSurface (juce::Graphics& g, juce::Rectangle<float> area, juce::Rectangle<float> panel)
{
    if (theme::lightMode)
    {
        g.setColour (theme::menuBg);
        g.fillRect (area);
        return;
    }

    // The translucent sheet follows the stage lighting and becomes fainter down and to the right.
    const juce::Point<float> slope { 0.039f / juce::jmax (1.0f, panel.getWidth()),
                                     0.029f / juce::jmax (1.0f, panel.getHeight()) };
    const auto end = panel.getTopLeft() + slope * (0.068f / slope.getDistanceSquaredFromOrigin());
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.092f), panel.getTopLeft(),
                                             juce::Colours::white.withAlpha (0.024f), end, false));
    g.fillRect (area);
}

void browserDrawerBody (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (theme::lightMode)
    {
        g.setColour (theme::menuBg);
        g.fillRect (bounds);
        return;
    }

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff505050), bounds.getTopLeft(),
                                             juce::Colour (0xff2d2d2d), bounds.getTopRight(), false));
    g.fillRect (bounds);

    // Reproduce the stage illumination inside the opaque drawer; the orb is occluded by the sheet.
    juce::ColourGradient shade (juce::Colours::transparentBlack, bounds.getTopLeft(),
                                juce::Colours::black.withAlpha (0.46f), bounds.getBottomLeft(), false);
    shade.addColour (0.12, juce::Colours::black.withAlpha (0.11f));
    shade.addColour (0.25, juce::Colours::black.withAlpha (0.25f));
    shade.addColour (0.45, juce::Colours::black.withAlpha (0.40f));
    shade.addColour (0.65, juce::Colours::black.withAlpha (0.46f));
    shade.addColour (0.83, juce::Colours::black.withAlpha (0.47f));
    shade.addColour (0.96, juce::Colours::black.withAlpha (0.44f));
    g.setGradientFill (shade);
    g.fillRect (bounds);
}

void browserLabel (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> r,
                   juce::Colour colour, bool bold = false, bool italic = false)
{
    auto font = theme::font (13.1f, bold);
    if (italic) font = font.italicised();
    g.setFont (font);
    g.setColour (theme::lightMode ? theme::text : colour);
    g.drawText (text, r, juce::Justification::centredLeft, true);
}

void browserFolder (juce::Graphics& g, juce::Point<float> c, juce::Colour colour)
{
    juce::Path p;
    p.startNewSubPath (c.x - 6.5f, c.y + 4.8f);
    p.lineTo (c.x - 6.5f, c.y - 5.1f);
    p.lineTo (c.x - 1.7f, c.y - 5.1f);
    p.lineTo (c.x + 0.3f, c.y - 3.1f);
    p.lineTo (c.x + 6.5f, c.y - 3.1f);
    p.lineTo (c.x + 6.5f, c.y + 4.8f);
    p.closeSubPath();
    p.startNewSubPath (c.x - 6.5f, c.y - 1.0f);
    p.lineTo (c.x + 6.5f, c.y - 1.0f);
    g.setColour (colour);
    g.strokePath (p, juce::PathStrokeType (1.1f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void browserPlus (juce::Graphics& g, juce::Point<float> c, juce::Colour colour)
{
    g.setColour (colour);
    g.drawLine (c.x - 3.5f, c.y, c.x + 3.5f, c.y, 1.3f);
    g.drawLine (c.x, c.y - 3.5f, c.x, c.y + 3.5f, 1.3f);
}

void browserChevron (juce::Graphics& g, juce::Point<float> c, bool open)
{
    g.setColour (browserMuted);
    juce::Path p;
    if (open)
    {
        p.startNewSubPath (c.x - 2.6f, c.y - 1.2f);
        p.lineTo (c.x, c.y + 1.5f);
        p.lineTo (c.x + 2.6f, c.y - 1.2f);
    }
    else
    {
        p.startNewSubPath (c.x - 1.2f, c.y - 2.7f);
        p.lineTo (c.x + 1.5f, c.y);
        p.lineTo (c.x - 1.2f, c.y + 2.7f);
    }
    g.strokePath (p, juce::PathStrokeType (1.05f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void browserTrash (juce::Graphics& g, juce::Rectangle<float> r)
{
    const auto c = r.getCentre();
    g.setColour (browserMuted);
    g.drawRoundedRectangle (c.x - 2.9f, c.y - 2.9f, 5.8f, 7.1f, 1.15f, 1.0f);
    g.drawLine (c.x - 4.0f, c.y - 3.5f, c.x + 4.0f, c.y - 3.5f, 1.1f);
    g.drawLine (c.x - 1.5f, c.y - 5.0f, c.x + 1.5f, c.y - 5.0f, 1.1f);
}

void drawStar (juce::Graphics& g, juce::Rectangle<float> b, bool filled, juce::Colour colour)
{
    juce::Path star;
    star.addStar (b.getCentre(), 5, b.getWidth() * 0.21f, b.getWidth() * 0.5f);
    g.setColour (colour);
    if (filled) g.fillPath (star);
    else g.strokePath (star, juce::PathStrokeType (1.1f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}
} // namespace

PresetPanel::PresetPanel()
{
    addChildComponent (searchBox);
    searchBox.setTextToShowWhenEmpty ("Search presets", browserMuted);
    searchBox.setColour (juce::TextEditor::backgroundColourId, theme::well);
    searchBox.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    searchBox.setColour (juce::TextEditor::focusedOutlineColourId, browserLilac);
    searchBox.setColour (juce::TextEditor::textColourId, browserBright);
    searchBox.setFont (theme::font (13.0f));
    searchBox.onTextChange = [this] { filter = searchBox.getText().trim(); rebuild(); };
    searchBox.onEscapeKey = [this]
    {
        searchBox.setText ({}, false);
        filter = {};
        searchActive = false;
        searchBox.setVisible (false);
        rebuild();
    };
}

void PresetPanel::resized()
{
    searchBox.setBounds (8, 3, juce::jmax (0, getWidth() - 16), 24);
}

void PresetPanel::setModeOptions (bool keep, bool low, bool stabilizing)
{
    if (keep == keepScale && low == lowLatency && stabilizing == stabilizer) return;
    keepScale = keep;
    lowLatency = low;
    stabilizer = stabilizing;
    repaint();
}

void PresetPanel::setItems (const std::vector<PresetItem>& items, const juce::StringArray& folders, const juce::String& selected)
{
    allItems = items;
    userFolders = folders;
    current = selected;
    rebuild();
}

float PresetPanel::rowHeight (Kind kind)
{
    if (kind == Kind::Timing) return 31.0f;
    if (kind == Kind::Divider) return 10.0f;
    return kRowH;
}

void PresetPanel::rebuild()
{
    rows = { { Kind::KeepScale, "Keep My Scale", {} }, { Kind::Timing, {}, {} },
             { Kind::NewPreset, "New preset...", {} }, { Kind::NewFolder, "New folder...", {} },
             { Kind::Divider, {}, {} } };
    const auto matches = [this] (const PresetItem& item)
    {
        return (! favOnly || item.favorite) && (filter.isEmpty() || item.name.containsIgnoreCase (filter));
    };
    const auto addPresets = [&] (const juce::String& folder)
    {
        for (int i = 0; i < int (allItems.size()); ++i)
        {
            const auto& item = allItems[size_t (i)];
            if (item.folder == folder && matches (item))
                rows.push_back ({ Kind::Preset, item.name, folder, i, item.factory, item.favorite });
        }
    };

    juce::StringArray folders;
    for (const auto& item : allItems)
        if (item.factory && item.folder.isNotEmpty()) folders.addIfNotAlreadyThere (item.folder);
    folders.sortNatural();
    folders.addArray (userFolders);
    folders.removeDuplicates (false);
    const bool searching = favOnly || filter.isNotEmpty();
    bool anyFound = false;
    for (const auto& folder : folders)
    {
        const bool hasMatch = std::any_of (allItems.begin(), allItems.end(), [&] (const PresetItem& item)
        {
            return item.folder == folder && matches (item);
        });
        if (searching && ! hasMatch) continue;
        const bool factoryFolder = std::any_of (allItems.begin(), allItems.end(), [&] (const PresetItem& item)
        {
            return item.factory && item.folder == folder;
        });
        rows.push_back ({ Kind::Folder, folder, folder, -1, factoryFolder, false });
        if (searching || openFolders.contains (folder))
            addPresets (folder);
        anyFound = anyFound || hasMatch;
    }

    const bool hasRootPresets = std::any_of (allItems.begin(), allItems.end(), [&] (const PresetItem& item)
    {
        return item.folder.isEmpty() && matches (item);
    });
    if (hasRootPresets)
    {
        rows.push_back ({ Kind::Section, "My presets", {} });
        addPresets ({});
    }
    else if (searching && ! anyFound)
        rows.push_back ({ Kind::Section, favOnly ? "No favourites yet" : "No matches", {} });
    scroll = juce::jlimit (0.0f, juce::jmax (0.0f, contentHeight() - float (getHeight())), scroll);
    repaint();
}

float PresetPanel::contentHeight() const
{
    float height = topInset();
    for (const auto& row : rows)
        height += rowHeight (row.kind);
    return height;
}

juce::Rectangle<float> PresetPanel::rowBounds (int i) const
{
    float y = topInset() - scroll;
    for (int j = 0; j < i; ++j)
        y += rowHeight (rows[size_t (j)].kind);
    return { 0.0f, y, float (getWidth()), rowHeight (rows[size_t (i)].kind) };
}

int PresetPanel::rowAt (juce::Point<float> point) const
{
    if (! getLocalBounds().toFloat().contains (point) || point.y < topInset()) return -1;
    float y = topInset() - scroll;
    for (int i = 0; i < int (rows.size()); ++i)
    {
        const float height = rowHeight (rows[size_t (i)].kind);
        if (point.y >= y && point.y < y + height) return i;
        y += height;
    }
    return -1;
}

juce::Rectangle<float> PresetPanel::trashBounds (juce::Rectangle<float> row) const
{
    return { row.getRight() - 28.0f, row.getCentreY() - 8.0f, 16.0f, 16.0f };
}

juce::Rectangle<float> PresetPanel::plusBounds (juce::Rectangle<float> row) const
{
    return trashBounds (row).translated (-17.0f, 0.0f);
}

void PresetPanel::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    browserDrawerBody (g, bounds);
    juce::Graphics::ScopedSaveState clip (g);
    g.reduceClipRegion (getLocalBounds().withTrimmedTop (int (topInset())));
    const auto mouse = getMouseXYRelative().toFloat();
    const int hot = isMouseOver (true) ? rowAt (mouse) : -1;

    for (int i = 0; i < int (rows.size()); ++i)
    {
        const auto& row = rows[size_t (i)];
        const auto r = rowBounds (i);
        if (! r.intersects (bounds)) continue;
        const auto icon = juce::Point<float> (32.0f, r.getCentreY());
        auto text = r.withTrimmedLeft (49.0f).withTrimmedRight (7.0f);
        const bool active = (i == hot && ! dragging) || (dragging && i == dropRow);
        const auto colour = active ? browserBright : browserMuted;
        if (active && row.kind != Kind::Divider && row.kind != Kind::Section)
        {
            g.setColour (browserHover);
            g.fillRect (r);
        }

        switch (row.kind)
        {
            case Kind::KeepScale:
            {
                g.setColour (keepScale ? browserLilac : browserMuted);
                for (int key = 0; key < 4; ++key)
                {
                    const float x = icon.x - 7.0f + 4.7f * float (key);
                    const float top = icon.y - 4.7f + 2.0f * float (key % 2);
                    g.drawLine (x, top, x, icon.y + 3.5f, 1.25f);
                    g.fillEllipse (x - 1.3f, top - 0.8f, 2.6f, 1.8f);
                }
                g.drawLine (icon.x - 7.0f, icon.y + 3.5f, icon.x + 7.0f, icon.y + 3.5f, 1.25f);
                browserLabel (g, row.text, text, keepScale ? browserLilac : colour, true);
                break;
            }
            case Kind::Timing:
            {
                const auto top = juce::Rectangle<float> (11.0f, 25.0f).withCentre (icon);
                g.setColour (browserMuted);
                g.drawRoundedRectangle (top, 5.5f, 1.0f);
                g.setColour (lowLatency ? browserLilac : (stabilizer ? browserGold : browserMuted));
                g.fillEllipse (juce::Rectangle<float> (6.3f, 6.3f).withCentre (icon.translated (0.0f, lowLatency ? 6.0f : -6.0f)));
                browserLabel (g, "Note Stabilizer", text.removeFromTop (15.5f), ! lowLatency && stabilizer ? browserGold : colour, ! lowLatency && stabilizer);
                browserLabel (g, "Low Latency", text, lowLatency ? browserLilac : colour, lowLatency);
                break;
            }
            case Kind::NewPreset:
                browserPlus (g, icon, colour);
                browserLabel (g, row.text, text, colour, false, true);
                break;
            case Kind::NewFolder:
                browserFolder (g, icon.translated (-1.0f, 0.0f), colour);
                browserPlus (g, icon.translated (5.0f, 1.0f), colour);
                browserLabel (g, row.text, text, colour, false, true);
                break;
            case Kind::Divider:
                g.setColour (browserMuted);
                g.fillRect (25.0f, r.getCentreY(), r.getWidth() - 48.0f, 1.0f);
                break;
            case Kind::Folder:
                browserChevron (g, { 11.0f, icon.y }, openFolders.contains (row.folder) || favOnly || filter.isNotEmpty());
                browserFolder (g, icon, colour);
                if (active && ! dragging)
                {
                    browserPlus (g, plusBounds (r).getCentre(), browserMuted);
                    if (! row.factory) browserTrash (g, trashBounds (r));
                    text.setRight (plusBounds (r).getX() - 1.0f);
                }
                browserLabel (g, row.text, text, colour, true);
                break;
            case Kind::Preset:
            {
                const bool selected = row.text == current;
                if (active || row.favorite)
                {
                    const auto star = plusBounds (r).reduced (2.0f);
                    drawStar (g, star, row.favorite, row.favorite ? browserGold : browserMuted);
                    if (active && ! row.factory) browserTrash (g, trashBounds (r));
                    text.setRight (star.getX() - 2.0f);
                }
                browserLabel (g, row.text, text, selected ? browserLilac : colour, selected);
                break;
            }
            case Kind::Section:
                browserLabel (g, row.text, text, colour, true);
                break;
        }
    }

    if (dragging)
        for (const auto& row : rows)
            if (row.index == dragIndex)
            {
                const auto ghost = juce::Rectangle<float> (110.0f, kRowH).withCentre (mouse.translated (0.0f, -13.0f));
                browserLabel (g, row.text, ghost, browserBright, true);
                break;
            }
}

void PresetPanel::mouseDown (const juce::MouseEvent& e)
{
    downPos = e.position;
    dragging = false;
    dragIndex = dropRow = -1;
    const int i = rowAt (e.position);
    if (! e.mods.isLeftButtonDown() || i < 0) return;
    const auto& row = rows[size_t (i)];
    const auto r = rowBounds (i);
    if (row.kind == Kind::Preset && ! row.factory && ! trashBounds (r).contains (e.position)
        && ! plusBounds (r).contains (e.position))
        dragIndex = row.index;
}

void PresetPanel::mouseDrag (const juce::MouseEvent& e)
{
    if (dragIndex < 0) return;
    if (! dragging && e.position.getDistanceFrom (downPos) > 5.0f) dragging = true;
    if (! dragging) return;
    const int i = rowAt (e.position);
    dropRow = i >= 0 && (rows[size_t (i)].kind == Kind::Folder || rows[size_t (i)].text == "My presets") ? i : -1;
    repaint();
}

void PresetPanel::mouseUp (const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        showActions (e);
        return;
    }
    if (dragging)
    {
        if (dragIndex >= 0 && dropRow >= 0 && onMove)
            onMove (dragIndex, rows[size_t (dropRow)].folder);
        dragging = false;
        dragIndex = dropRow = -1;
        repaint();
        return;
    }
    dragIndex = -1;
    const int i = rowAt (e.position);
    if (i < 0) return;
    const auto& row = rows[size_t (i)];
    const auto r = rowBounds (i);
    switch (row.kind)
    {
        case Kind::KeepScale: if (onToggleKeepScale) onToggleKeepScale(); break;
        case Kind::Timing: if (onToggleTiming) onToggleTiming(); break;
        case Kind::NewPreset: if (onNewPreset) onNewPreset(); break;
        case Kind::NewFolder: if (onNewFolder) onNewFolder(); break;
        case Kind::Preset:
            if (! row.factory && trashBounds (r).contains (e.position)) { if (onDelete) onDelete (row.index); }
            else if (plusBounds (r).contains (e.position)) { if (onToggleFavorite) onToggleFavorite (row.index); }
            else if (onLoad) onLoad (row.index);
            break;
        case Kind::Folder:
            if (plusBounds (r).contains (e.position)) { if (onSaveToFolder) onSaveToFolder (row.folder); }
            else if (! row.factory && trashBounds (r).contains (e.position)) { if (onDeleteFolder) onDeleteFolder (row.folder); }
            else
            {
                const auto folder = row.folder;
                if (openFolders.contains (folder)) openFolders.removeString (folder);
                else openFolders.add (folder);
                rebuild();
            }
            break;
        default: break;
    }
}

void PresetPanel::showActions (const juce::MouseEvent& e)
{
    juce::PopupMenu menu;
    menu.addItem (1, "Search presets", true, searchActive);
    menu.addItem (2, "Favourites only", true, favOnly);
    menu.addItem (3, "Open preset folder");
    const auto screen = e.getScreenPosition();
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this)
                           .withTargetScreenArea ({ screen.x, screen.y, 1, 1 }),
                        [self = juce::Component::SafePointer<PresetPanel> (this)] (int result)
    {
        if (self == nullptr) return;
        if (result == 1)
        {
            self->searchActive = ! self->searchActive;
            self->searchBox.setVisible (self->searchActive);
            if (self->searchActive) self->searchBox.grabKeyboardFocus();
            else
            {
                self->searchBox.setText ({}, false);
                self->filter = {};
            }
            self->rebuild();
        }
        else if (result == 2)
        {
            self->favOnly = ! self->favOnly;
            self->rebuild();
        }
        else if (result == 3 && self->onOpenFolder) self->onOpenFolder();
    });
}

void PresetPanel::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    const float maxScroll = juce::jmax (0.0f, contentHeight() - float (getHeight()));
    scroll = juce::jlimit (0.0f, maxScroll, scroll - wheel.deltaY * 120.0f);
    repaint();
}

GroupBrowser::GroupBrowser()
{
    addChildComponent (nameEditor);
    nameEditor.setFont (theme::font (13.1f, true));
    nameEditor.setIndents (0, 0);
    nameEditor.setJustification (juce::Justification::centredLeft);
    nameEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    nameEditor.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    nameEditor.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    nameEditor.setColour (juce::TextEditor::textColourId, browserBright);
    nameEditor.setColour (juce::TextEditor::highlightColourId, browserLilac.withAlpha (0.38f));
    nameEditor.onReturnKey = [this] { endRename (true); };
    nameEditor.onEscapeKey = [this] { endRename (false); };
    nameEditor.onFocusLost = [this] { endRename (true); };
}

void GroupBrowser::setData (const juce::StringArray& groupNames, const std::vector<GroupInstanceRow>& entries)
{
    names = groupNames;
    instances = entries;
    rebuild();
}

void GroupBrowser::rebuild()
{
    rows.clear();
    groupSizes.fill (0);
    selectedGroup = -2;
    for (const auto& item : instances)
    {
        if (juce::isPositiveAndBelow (item.group + 1, int (groupSizes.size())))
            ++groupSizes[size_t (item.group + 1)];
        if (item.self) selectedGroup = item.group;
    }

    for (int group = -1; group < 4; ++group)
    {
        const auto label = group < 0 ? juce::String ("No Group") : names[group];
        rows.push_back ({ group, -1, label, true, false });
        if (openGroups[size_t (group + 1)])
            for (const auto& item : instances)
                if (item.group == group)
                    rows.push_back ({ group, item.id, item.label, false, item.self });
    }

    const auto maxScroll = juce::jmax (0.0f, float (rows.size()) * kRowH - float (getHeight()));
    scroll = juce::jlimit (0.0f, maxScroll, scroll);
    repaint();
}

juce::Rectangle<float> GroupBrowser::rowBounds (int i) const
{
    return { 0.0f, float (i) * kRowH - scroll, float (getWidth()), kRowH };
}

int GroupBrowser::rowAt (juce::Point<float> point) const
{
    if (! getLocalBounds().toFloat().contains (point)) return -1;
    const int i = int ((point.y + scroll) / kRowH);
    return juce::isPositiveAndBelow (i, int (rows.size())) ? i : -1;
}

void GroupBrowser::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    browserDrawerBody (g, bounds);
    const auto mouse = getMouseXYRelative().toFloat();
    const int hot = isMouseOver (true) ? rowAt (mouse) : -1;
    const auto grid = icons::grid();

    for (int i = 0; i < int (rows.size()); ++i)
    {
        const auto& row = rows[size_t (i)];
        const auto r = rowBounds (i);
        if (! r.intersects (bounds)) continue;
        const bool renaming = row.header && renameGroup >= 0 && row.group == renameGroup;
        const bool active = renaming || (! dragging && i == hot) || (dragging && i == dropRow);
        if (active)
        {
            g.setColour (browserHover);
            g.fillRect (r);
        }
        const auto colour = row.self || (row.header && row.group >= 0 && row.group == selectedGroup)
                          ? browserLilac : (active ? browserBright : browserMuted);
        const float y = r.getCentreY();
        if (row.header)
        {
            const auto groupIndex = size_t (row.group + 1);
            if (groupSizes[groupIndex] > 0)
                browserChevron (g, { 11.0f, y }, openGroups[groupIndex]);
            g.setColour (colour);
            g.fillPath (grid, grid.getTransformToScaleToFit ({ 24.0f, y - 7.5f, 15.0f, 15.0f }, true));
        }
        else if (row.self)
        {
            g.setColour (browserLilac);
            g.drawVerticalLine (26, y - 3.0f, y + 3.0f);
            g.drawVerticalLine (29, y - 5.2f, y + 5.2f);
            g.drawVerticalLine (32, y - 3.4f, y + 3.4f);
        }
        if (! row.header && i == hot && ! dragging)
        {
            g.setColour (browserMuted);
            g.drawVerticalLine (10, y - 4.0f, y + 4.0f);
            g.drawVerticalLine (14, y - 4.0f, y + 4.0f);
        }
        if (! renaming)
            browserLabel (g, row.text, r.withTrimmedLeft (50.0f).withTrimmedRight (7.0f), colour, row.header || row.self);
    }

    if (dragging)
        for (const auto& item : instances)
            if (item.id == dragId)
            {
                const auto ghost = juce::Rectangle<float> (95.0f, kRowH).withCentre (mouse.translated (0.0f, -12.0f));
                browserLabel (g, item.label, ghost, browserBright, true);
                break;
            }
}

void GroupBrowser::mouseDown (const juce::MouseEvent& e)
{
    downPos = e.position;
    dragging = false;
    dragId = dropRow = -1;
    const int i = rowAt (e.position);
    if (i >= 0 && ! rows[size_t (i)].header && e.mods.isLeftButtonDown())
        dragId = rows[size_t (i)].id;
}

void GroupBrowser::mouseDrag (const juce::MouseEvent& e)
{
    if (dragId < 0) return;
    if (! dragging && e.position.getDistanceFrom (downPos) > 5.0f) dragging = true;
    if (! dragging) return;
    const int i = rowAt (e.position);
    dropRow = i >= 0 && rows[size_t (i)].header ? i : -1;
    repaint();
}

void GroupBrowser::mouseUp (const juce::MouseEvent& e)
{
    if (dragging)
    {
        if (dropRow >= 0 && onAssign) onAssign (dragId, rows[size_t (dropRow)].group);
        dragging = false;
        dragId = dropRow = -1;
        repaint();
        return;
    }
    dragId = -1;
    const int i = rowAt (e.position);
    if (i < 0 || ! rows[size_t (i)].header) return;
    const auto group = rows[size_t (i)].group;
    const auto index = size_t (group + 1);
    if (e.position.x < 44.0f && groupSizes[index] > 0)
    {
        openGroups[index] = ! openGroups[index];
        rebuild();
    }
    else if (group >= 0 && e.position.x >= 44.0f)
        beginRename (i);
}

void GroupBrowser::beginRename (int i)
{
    endRename (true);
    const auto& row = rows[size_t (i)];
    renameGroup = row.group;
    nameEditor.setBounds (rowBounds (i).withTrimmedLeft (50.0f).withTrimmedRight (6.0f).toNearestInt());
    nameEditor.setText (row.text, false);
    nameEditor.setVisible (true);
    nameEditor.grabKeyboardFocus();
    nameEditor.selectAll();
    repaint();
}

void GroupBrowser::endRename (bool commit)
{
    if (renameGroup < 0) return;
    const int group = renameGroup;
    renameGroup = -1;
    const auto name = nameEditor.getText().trim();
    nameEditor.setVisible (false);
    if (commit && onRename) onRename (group, name);
    repaint();
}

void GroupBrowser::visibilityChanged()
{
    if (! isVisible()) endRename (true);
}

void GroupBrowser::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    endRename (true);
    const auto maxScroll = juce::jmax (0.0f, float (rows.size()) * kRowH - float (getHeight()));
    scroll = juce::jlimit (0.0f, maxScroll, scroll - wheel.deltaY * 120.0f);
    repaint();
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
    g.setColour (theme::menuBg);
    g.fillPath (p);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.strokePath (p, juce::PathStrokeType (1.0f));
    g.setFont (font (17.0f, true));
    shadowText (g, "Save preset", c.withHeight (52.0f).withTrimmedTop (14.0f), juce::Justification::centred, text);
    g.setFont (font (12.5f));
    shadowText (g, message.isEmpty() ? juce::String ("Name for this preset") : message,
                c.withTrimmedTop (42.0f).withHeight (20.0f), juce::Justification::centred, message.isEmpty() ? textDim : warn);
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
    g.setColour (theme::menuBg);
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
                g.setFont (font (11.5f, true));
                shadowText (g, it.text.toUpperCase(), r.withTrimmedLeft (10.0f), juce::Justification::centredLeft, textFaint.brighter (0.2f));
            }
            continue;
        }
        if (it.kind == Item::Kind::Info)
        {
            g.setFont (font (12.0f));
            shadowText (g, it.text, r.withTrimmedLeft (22.0f), juce::Justification::centredLeft, textFaint);
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
        g.setFont (font (13.5f, it.current));
        shadowText (g, it.text, r.withTrimmedLeft (22.0f), juce::Justification::centredLeft, it.current ? text : textDim.interpolatedWith (text, a.get()));
        if (it.detail.isNotEmpty())
        {
            g.setFont (font (12.0f));
            shadowText (g, it.detail, r.withTrimmedRight (10.0f), juce::Justification::centredRight, textFaint.brighter (0.3f));
        }
    }
}

void SettingsPanel::mouseUp (const juce::MouseEvent& e)
{
    const int i = rowAt (e.position);
    if (i >= 0 && onPick) onPick (items[size_t (i)].id);
}

// ---- ReleaseNotesCard

void ReleaseNotesCard::open (juce::Image blurredBackdrop)
{
    backdrop = blurredBackdrop;
    scroll = 0;
    lastWidth = 0; // force the layouts to rebuild for the current width
    juce::Desktop::getInstance().getAnimator().cancelAnimation (this, false);
    setAlpha (1.0f);
    setVisible (true);
    toFront (false);
    repaint();
}

juce::Rectangle<int> ReleaseNotesCard::cardBounds() const
{
    const int w = juce::jmin (540, getWidth() - 60);
    const int h = juce::jmin (getHeight() - 70, 600);
    return juce::Rectangle<int> (w, h).withCentre (getLocalBounds().getCentre());
}

juce::Rectangle<float> ReleaseNotesCard::closeBounds() const
{
    const auto c = cardBounds();
    return { float (c.getRight() - 42), float (c.getY() + 14), 26.0f, 26.0f };
}

void ReleaseNotesCard::buildLayouts (float width)
{
    using namespace theme;
    bodyLayouts.clear();
    for (int i = 0; i < kNumReleaseNotes; ++i)
    {
        juce::AttributedString as;
        as.append (juce::String::fromUTF8 (kReleaseNotes[i].body), font (13.0f), textDim);
        as.setLineSpacing (3.0f);
        juce::TextLayout tl;
        tl.createLayout (as, width);
        bodyLayouts.push_back (std::move (tl));
    }
    lastWidth = width;
}

float ReleaseNotesCard::contentHeight() const
{
    float h = 0.0f;
    for (const auto& tl : bodyLayouts) h += 26.0f + tl.getHeight() + 18.0f;
    return h;
}

void ReleaseNotesCard::paint (juce::Graphics& g)
{
    using namespace theme;
    if (backdrop.isValid())
        g.drawImage (backdrop, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
    g.fillAll (juce::Colours::black.withAlpha (0.5f));

    const auto cb = cardBounds();
    const auto cf = cb.toFloat();
    juce::DropShadow (juce::Colours::black.withAlpha (0.7f), 28, { 0, 10 }).drawForRectangle (g, cb);
    g.setColour (bg2);
    g.fillRoundedRectangle (cf, 12.0f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (cf.reduced (0.5f), 12.0f, 1.0f);

    g.setFont (font (20.0f, true));
    shadowText (g, "What's New", juce::Rectangle<float> (cf.getX() + 22.0f, cf.getY() + 15.0f, cf.getWidth() - 90.0f, 28.0f),
                juce::Justification::centredLeft, text);

    // Close (X).
    const auto xb = closeBounds();
    const bool xHot = xb.contains (getMouseXYRelative().toFloat());
    if (xHot) { g.setColour (juce::Colours::white.withAlpha (0.12f)); g.fillRoundedRectangle (xb, 6.0f); }
    g.setColour (text.withAlpha (xHot ? 1.0f : 0.7f));
    const auto xi = xb.reduced (8.0f);
    g.drawLine (xi.getX(), xi.getY(), xi.getRight(), xi.getBottom(), 1.8f);
    g.drawLine (xi.getX(), xi.getBottom(), xi.getRight(), xi.getY(), 1.8f);

    const float top = cf.getY() + 52.0f;
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.fillRect (cf.getX() + 16.0f, top, cf.getWidth() - 32.0f, 1.0f);

    const float padX = 24.0f, width = cf.getWidth() - padX * 2.0f;
    if (std::abs (lastWidth - width) > 0.5f) buildLayouts (width);
    const juce::Rectangle<float> content (cf.getX() + padX, top + 12.0f, width, cf.getBottom() - top - 24.0f);
    juce::Graphics::ScopedSaveState ss (g);
    g.reduceClipRegion (content.toNearestInt());
    float y = content.getY() - scroll;
    for (int i = 0; i < kNumReleaseNotes; ++i)
    {
        g.setFont (font (15.0f, true));
        shadowText (g, "Version " + juce::String (kReleaseNotes[i].version),
                    juce::Rectangle<float> (content.getX(), y, width, 22.0f), juce::Justification::centredLeft, violet.brighter (0.3f));
        y += 26.0f;
        bodyLayouts[size_t (i)].draw (g, juce::Rectangle<float> (content.getX(), y, width, bodyLayouts[size_t (i)].getHeight()));
        y += bodyLayouts[size_t (i)].getHeight() + 10.0f;
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.fillRect (content.getX(), y, width, 1.0f);
        y += 8.0f;
    }
}

void ReleaseNotesCard::mouseUp (const juce::MouseEvent& e)
{
    if (closeBounds().contains (e.position) || ! cardBounds().contains (e.getPosition()))
    {
        // Fade the whole overlay out (a proxy image animates while the real component hides).
        juce::Desktop::getInstance().getAnimator().animateComponent (this, getBounds(), 0.0f, 220, true, 1.0, 1.0);
        setVisible (false);
        if (onClose) onClose();
    }
}

void ReleaseNotesCard::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w)
{
    if (! cardBounds().contains (e.getPosition())) return;
    const float maxS = juce::jmax (0.0f, contentHeight() - (cardBounds().toFloat().getHeight() - 76.0f));
    scroll = juce::jlimit (0.0f, maxS, scroll - w.deltaY * 120.0f);
    repaint();
}

// ---- ColorPanel

static constexpr float kSatMin = 0.4f, kSatMax = 1.6f, kBriMin = 0.6f, kBriMax = 1.3f;

// Strip Y positions by id: 0 hue 1 sat 2 bright | 3 gradA 4 gradB | 5 orb 6 key 7 meter.
static float stripY (int id)
{
    switch (id) { case 0: return 14; case 1: return 40; case 2: return 66;
                  case 3: return 226; case 4: return 250;
                  case 5: return 284; case 6: return 308; default: return 332; }
}

juce::Rectangle<float> ColorPanel::stripRect (int id) const
{
    return { 58.0f, stripY (id), float (getWidth()) - 58.0f - 14.0f, 10.0f };
}

juce::Rectangle<float> ColorPanel::swatchAt (int i) const
{
    const int cols = 8;
    const float gap = 6.0f, sw = (float (getWidth()) - 28.0f - gap * float (cols - 1)) / float (cols);
    const int col = i % cols, row = i / cols;
    return { 14.0f + float (col) * (sw + gap), 92.0f + float (row) * (sw + gap), sw, sw };
}

juce::Rectangle<float> ColorPanel::chipRect (int i) const
{
    const float gap = 8.0f, x0 = 14.0f, w = (float (getWidth()) - 28.0f - gap * 2.0f) / 3.0f, h = 22.0f;
    const int col = i % 3, row = i / 3;
    return { x0 + float (col) * (w + gap), 156.0f + float (row) * (h + 6.0f), w, h };
}

// A horizontal strip: full-spectrum hue ramp with a white handle at value (0..1). dim = inactive.
static void drawHueStrip (juce::Graphics& g, juce::Rectangle<float> bar, float value, bool dim)
{
    juce::ColourGradient grad (juce::Colour::fromHSV (0.0f, 0.8f, 1.0f, 1.0f), bar.getX(), 0.0f,
                               juce::Colour::fromHSV (1.0f, 0.8f, 1.0f, 1.0f), bar.getRight(), 0.0f, false);
    for (float t = 0.125f; t < 1.0f; t += 0.125f) grad.addColour (t, juce::Colour::fromHSV (t, 0.8f, 1.0f, 1.0f));
    g.setGradientFill (grad);
    g.setOpacity (dim ? 0.4f : 1.0f);
    g.fillRoundedRectangle (bar, bar.getHeight() * 0.5f);
    g.setOpacity (1.0f);
    const float hx = bar.getX() + juce::jlimit (0.0f, 1.0f, value) * bar.getWidth();
    g.setColour (juce::Colours::white.withAlpha (dim ? 0.6f : 1.0f));
    g.fillEllipse (hx - 6.0f, bar.getCentreY() - 6.0f, 12.0f, 12.0f);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.drawEllipse (hx - 6.0f, bar.getCentreY() - 6.0f, 12.0f, 12.0f, 1.0f);
}

void ColorPanel::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto b = getLocalBounds().toFloat();
    juce::Path bg;
    bg.addRoundedRectangle (b.getX(), b.getY(), b.getWidth(), b.getHeight(), 8.0f, 8.0f, false, false, true, true);
    juce::DropShadow (juce::Colours::black.withAlpha (0.6f), 16, { 0, 4 }).drawForPath (g, bg);
    g.setColour (theme::menuBg); g.fillPath (bg);
    g.setColour (juce::Colours::white.withAlpha (0.07f)); g.strokePath (bg, juce::PathStrokeType (1.0f));

    const auto mouse = getMouseXYRelative().toFloat();
    auto label = [&] (const juce::String& t, float y, juce::Colour c)
    {
        g.setFont (font (12.0f, true));
        shadowText (g, t, juce::Rectangle<float> (12.0f, y - 9.0f, 46.0f, 18.0f), juce::Justification::centredLeft, c);
    };

    // --- Accent: Hue / Saturation / Bright. Dimmed while a gradient drives the accent.
    const char* anames[] = { "Hue", "Sat", "Bright" };
    const float frac[] = { hue, (sat - kSatMin) / (kSatMax - kSatMin), (bright - kBriMin) / (kBriMax - kBriMin) };
    for (int i = 0; i < 3; ++i)
    {
        const auto bar = stripRect (i);
        label (anames[i], bar.getCentreY(), gradOn ? textFaint : textDim);
        if (i == 0) { drawHueStrip (g, bar, frac[0], gradOn); }
        else
        {
            if (i == 1) g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6b6b70), bar.getX(), 0.0f,
                                                                 juce::Colour (theme::base::violet).withRotatedHue (hue), bar.getRight(), 0.0f, false));
            else        g.setGradientFill (juce::ColourGradient (juce::Colours::black, bar.getX(), 0.0f, juce::Colours::white, bar.getRight(), 0.0f, false));
            g.setOpacity (gradOn ? 0.4f : 1.0f);
            g.fillRoundedRectangle (bar, bar.getHeight() * 0.5f);
            g.setOpacity (1.0f);
            const float hx = bar.getX() + juce::jlimit (0.0f, 1.0f, frac[i]) * bar.getWidth();
            g.setColour (juce::Colours::white.withAlpha (gradOn ? 0.6f : 1.0f));
            g.fillEllipse (hx - 6.0f, bar.getCentreY() - 6.0f, 12.0f, 12.0f);
            g.setColour (juce::Colours::black.withAlpha (0.35f));
            g.drawEllipse (hx - 6.0f, bar.getCentreY() - 6.0f, 12.0f, 12.0f, 1.0f);
        }
    }

    // --- Swatches
    for (int i = 0; i < kSwatches; ++i)
    {
        const auto s = swatchAt (i);
        g.setColour (juce::Colour (theme::base::violet).withRotatedHue (swatchHue (i)));
        g.fillRoundedRectangle (s, 5.0f);
        const bool sel = ! gradOn && std::abs (hue - swatchHue (i)) < 0.5f / float (kSwatches);
        g.setColour (juce::Colours::white.withAlpha (sel ? 0.95f : (s.contains (mouse) ? 0.55f : 0.14f)));
        g.drawRoundedRectangle (s.reduced (0.5f), 5.0f, sel ? 2.0f : 1.0f);
    }

    // --- Chips: Light / Gradient toggles, then Default / FL / Ocean / Green presets.
    // "Default" lights up (grey) when nothing is customised: default accent, no gradient, dark theme.
    const bool isDefault = ! gradOn && ! lightOn && std::abs (hue) < 0.01f
                           && std::abs (sat - 1.0f) < 0.03f && std::abs (bright - 1.0f) < 0.03f;
    const char* clabels[] = { "Light", "Gradient", "Default", "FL", "Ocean", "Green" };
    const bool active[] = { lightOn, gradOn, isDefault, false, false, false };
    for (int i = 0; i < 6; ++i)
    {
        const auto c = chipRect (i);
        const bool hot = c.contains (mouse);
        const bool greySel = i == 2; // Default uses a neutral grey "selected" fill, not the accent
        g.setColour (active[i] ? (greySel ? bg3.brighter (0.18f) : violet.withAlpha (0.92f))
                               : menuSel.withAlpha (hot ? 1.0f : 0.65f));
        g.fillRoundedRectangle (c, 5.0f);
        if (! active[i]) { g.setColour (juce::Colours::white.withAlpha (hot ? 0.3f : 0.1f)); g.drawRoundedRectangle (c.reduced (0.5f), 5.0f, 1.0f); }
        g.setFont (font (11.5f, true));
        const juce::Colour lblCol = active[i] ? (greySel ? text : juce::Colours::white)
                                              : textDim.interpolatedWith (text, hot ? 1.0f : 0.5f);
        shadowText (g, clabels[i], c, juce::Justification::centred, lblCol);
    }

    // --- Gradient endpoints
    g.setFont (font (10.5f, true));
    shadowText (g, "GRADIENT", juce::Rectangle<float> (12.0f, 206.0f, 160.0f, 14.0f), juce::Justification::centredLeft, textFaint.brighter (0.15f));
    label ("A", stripRect (3).getCentreY(), gradOn ? textDim : textFaint);
    drawHueStrip (g, stripRect (3), gradA, ! gradOn);
    label ("B", stripRect (4).getCentreY(), gradOn ? textDim : textFaint);
    drawHueStrip (g, stripRect (4), gradB, ! gradOn);
    { // a little preview of the current gradient
        const auto pv = juce::Rectangle<float> (float (getWidth()) - 52.0f, 206.0f, 38.0f, 14.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour::fromHSV (gradA, 0.85f, 1.0f, 1.0f), pv.getX(), 0.0f,
                                                 juce::Colour::fromHSV (gradB, 0.85f, 1.0f, 1.0f), pv.getRight(), 0.0f, false));
        g.fillRoundedRectangle (pv, 3.0f);
    }

    // --- Per-element hue offsets
    g.setFont (font (10.5f, true));
    shadowText (g, "PER-ELEMENT", juce::Rectangle<float> (12.0f, 272.0f - 10.0f, 160.0f, 14.0f), juce::Justification::centredLeft, textFaint.brighter (0.15f));
    label ("Orb", stripRect (5).getCentreY(), textDim);
    drawHueStrip (g, stripRect (5), orbH, false);
    label ("Keys", stripRect (6).getCentreY(), textDim);
    drawHueStrip (g, stripRect (6), keyH, false);
    label ("Mtr", stripRect (7).getCentreY(), textDim);
    drawHueStrip (g, stripRect (7), meterH, false);
}

void ColorPanel::applyDrag (juce::Point<float> p)
{
    const auto bar = stripRect (drag);
    const float t = juce::jlimit (0.0f, 1.0f, (p.x - bar.getX()) / bar.getWidth());
    switch (drag)
    {
        case 0: hue = t; if (onChange) onChange (hue, sat, bright); break;
        case 1: sat = kSatMin + t * (kSatMax - kSatMin); if (onChange) onChange (hue, sat, bright); break;
        case 2: bright = kBriMin + t * (kBriMax - kBriMin); if (onChange) onChange (hue, sat, bright); break;
        case 3: gradA = t; gradOn = true; if (onGradient) onGradient (true, gradA, gradB); break;
        case 4: gradB = t; gradOn = true; if (onGradient) onGradient (true, gradA, gradB); break;
        case 5: orbH = t; if (onElements) onElements (orbH, keyH, meterH); break;
        case 6: keyH = t; if (onElements) onElements (orbH, keyH, meterH); break;
        case 7: meterH = t; if (onElements) onElements (orbH, keyH, meterH); break;
        default: break;
    }
    repaint();
}

void ColorPanel::applyPreset (float h, float s, float b)
{
    gradOn = false;
    if (onGradient) onGradient (false, gradA, gradB);
    hue = h; sat = s; bright = b;
    if (onChange) onChange (hue, sat, bright);
    repaint();
}

void ColorPanel::mouseDown (const juce::MouseEvent& e)
{
    drag = -1;
    for (int i = 0; i < 8; ++i)
        if (stripRect (i).expanded (6.0f, 9.0f).contains (e.position)) { drag = i; applyDrag (e.position); return; }
    for (int i = 0; i < kSwatches; ++i)
        if (swatchAt (i).contains (e.position)) { applyPreset (swatchHue (i), 1.0f, 1.0f); return; }
    for (int i = 0; i < 6; ++i)
        if (chipRect (i).contains (e.position))
        {
            switch (i)
            {
                case 0: lightOn = ! lightOn; if (onLight) onLight (lightOn); break;
                case 1: gradOn = ! gradOn; if (onGradient) onGradient (gradOn, gradA, gradB); break;
                case 2: // Default: reset accent, gradient AND the light theme
                    if (lightOn) { lightOn = false; if (onLight) onLight (false); }
                    applyPreset (0.0f, 1.0f, 1.0f);
                    break;
                case 3: applyPreset (0.31f, 1.3f, 1.12f); break; // FL orange
                case 4: applyPreset (0.82f, 1.0f, 1.0f); break;  // Ocean blue
                case 5: applyPreset (0.57f, 1.05f, 1.0f); break; // Green
                default: break;
            }
            repaint();
            return;
        }
}

void ColorPanel::mouseDrag (const juce::MouseEvent& e) { if (drag >= 0) applyDrag (e.position); }
void ColorPanel::mouseUp (const juce::MouseEvent&) { drag = -1; }

// ---- BrowserTabs

juce::Rectangle<float> BrowserTabs::tabRect (int i) const
{
    return { i == 0 ? 0.0f : float (getWidth()) - 86.0f, 0.0f, 86.0f, float (getHeight()) };
}

void BrowserTabs::paint (juce::Graphics& g)
{
    const char* names[] { "Presets", "Groups" };
    const auto mouse = getMouseXYRelative().toFloat();
    for (int i = 0; i < 2; ++i)
    {
        const auto r = tabRect (i);
        const bool selected = i == tab;
        if (selected)
        {
            juce::Path shape;
            shape.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), 7.0f, 7.0f,
                                       true, true, false, false);
            juce::Graphics::ScopedSaveState clip (g);
            g.reduceClipRegion (shape);
            browserSurface (g, r, { 0.0f, 0.0f, float (getWidth()), 462.0f });
        }
        g.setColour (theme::lightMode ? theme::text : (selected ? browserBright : browserMuted));
        if (! selected && r.contains (mouse)) g.setColour (theme::lightMode ? theme::text : browserMuted.brighter (0.3f));
        g.setFont (theme::font (13.1f, selected));
        g.drawText (names[i], r, juce::Justification::centred, true);
    }
}

void BrowserTabs::mouseUp (const juce::MouseEvent& e)
{
    for (int i = 0; i < 2; ++i)
        if (tabRect (i).contains (e.position) && i != tab && onSelect) { onSelect (i); return; }
}

// ---- AutoKeyCard

void AutoKeyCard::open (bool inst, bool have, int r, bool m) { setState (inst, have, r, m); setVisible (true); toFront (false); }
void AutoKeyCard::setState (bool inst, bool have, int r, bool m)
{
    if (inst == installed && have == haveKey && r == root && m == minor) return;
    installed = inst; haveKey = have; root = r; minor = m; repaint();
}
juce::Rectangle<float> AutoKeyCard::actionButton() const
{
    const auto c = cardBounds().toFloat();
    return { c.getCentreX() - 100.0f, c.getBottom() - 58.0f, 200.0f, 34.0f };
}
juce::Rectangle<float> AutoKeyCard::closeBounds() const
{
    const auto c = cardBounds().toFloat();
    return { c.getRight() - 30.0f, c.getY() + 12.0f, 18.0f, 18.0f };
}

void AutoKeyCard::paint (juce::Graphics& g)
{
    using namespace theme;
    const auto mouse = getMouseXYRelative().toFloat();
    g.fillAll (juce::Colours::black.withAlpha (0.55f));
    const auto c = cardBounds().toFloat();
    juce::DropShadow (juce::Colours::black.withAlpha (0.6f), 24, { 0, 8 }).drawForRectangle (g, c.getSmallestIntegerContainer());
    g.setColour (menuBg);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (c.reduced (0.5f), 10.0f, 1.0f);

    g.setColour (violet.brighter (0.15f));
    strokeIcon (g, icons::key(), juce::Rectangle<float> (22.0f, 22.0f).withCentre ({ c.getX() + 34.0f, c.getY() + 33.0f }), 2.0f);
    g.setFont (font (19.0f, true));
    shadowText (g, "Auto Key", juce::Rectangle<float> (c.getX() + 56.0f, c.getY() + 19.0f, 200.0f, 28.0f), juce::Justification::centredLeft, text);

    const auto x = closeBounds();
    g.setColour (x.expanded (4.0f).contains (mouse) ? text : textDim);
    g.drawLine (x.getX(), x.getY(), x.getRight(), x.getBottom(), 1.7f);
    g.drawLine (x.getX(), x.getBottom(), x.getRight(), x.getY(), 1.7f);

    static const char* N[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const auto body = juce::Rectangle<float> (c.getX() + 26.0f, c.getY() + 66.0f, c.getWidth() - 52.0f, c.getHeight() - 140.0f);
    juce::String btn;
    if (! installed)
    {
        g.setFont (font (15.0f, true));
        shadowText (g, "MetaKey isn't installed", body.withHeight (24.0f), juce::Justification::centredTop, text);
        g.setFont (font (13.0f));
        shadowFittedText (g, "It detects your beat's key and sends it straight to Auto Key. Install it to continue.",
                          body.withTrimmedTop (30.0f), juce::Justification::centredTop, 3, textDim);
        btn = "Install MetaKey";
    }
    else if (haveKey)
    {
        g.setFont (font (13.5f));
        shadowText (g, "MetaKey detects", body.withHeight (20.0f), juce::Justification::centred, textDim);
        g.setFont (font (32.0f, true));
        shadowText (g, juce::String (N[root % 12]) + (minor ? " Minor" : " Major"),
                    body.withTrimmedTop (22.0f).withHeight (42.0f), juce::Justification::centred, text);
        btn = "Apply to MetaTune";
    }
    else
    {
        g.setFont (font (13.5f));
        shadowFittedText (g, "Add MetaKey to the mixer track with your beat and press play, then this will light up.",
                          body, juce::Justification::centredTop, 3, textDim);
    }

    if (btn.isNotEmpty())
    {
        const auto b = actionButton();
        g.setColour (violet.withAlpha (b.contains (mouse) ? 1.0f : 0.9f));
        g.fillRoundedRectangle (b, b.getHeight() * 0.5f);
        g.setFont (font (14.0f, true));
        shadowText (g, btn, b, juce::Justification::centred, juce::Colours::white);
    }
}

void AutoKeyCard::mouseUp (const juce::MouseEvent& e)
{
    const auto p = e.position;
    if (closeBounds().expanded (6.0f).contains (p) || ! cardBounds().toFloat().contains (p)) { if (onClose) onClose(); return; }
    if ((! installed || haveKey) && actionButton().contains (p))
    {
        if (! installed) { if (onInstall) onInstall(); }
        else if (haveKey) { if (onApply) onApply (root, minor); }
    }
}
