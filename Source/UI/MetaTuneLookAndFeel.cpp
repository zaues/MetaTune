#include "MetaTuneLookAndFeel.h"

MetaTuneLookAndFeel::MetaTuneLookAndFeel()
{
    using namespace theme;
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1c1c1e));
    setColour (juce::PopupMenu::textColourId, text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, violet.withAlpha (0.35f));
    setColour (juce::PopupMenu::highlightedTextColourId, text);
    setColour (juce::PopupMenu::headerTextColourId, textDim);
    setColour (juce::TextEditor::backgroundColourId, bg0);
    setColour (juce::TextEditor::textColourId, text);
    setColour (juce::TextEditor::highlightColourId, violet.withAlpha (0.4f));
    setColour (juce::TextEditor::outlineColourId, violet);
    setColour (juce::TextEditor::focusedOutlineColourId, violet);
    setColour (juce::CaretComponent::caretColourId, lavender);
    setColour (juce::TooltipWindow::backgroundColourId, bg3);
    setColour (juce::TooltipWindow::textColourId, text);
}

void MetaTuneLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    // Same card as the preset panel: near-black with a faint edge.
    g.fillAll (juce::Colour (0xff1c1c1e));
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRect (0, 0, w, h);
}

void MetaTuneLookAndFeel::getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int, int& w, int& h)
{
    if (isSeparator) { w = 60; h = 11; return; }
    w = juce::GlyphArrangement::getStringWidthInt (getPopupMenuFont(), text) + 64;
    h = 28;
}

void MetaTuneLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                                         bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                                         const juce::String&, const juce::Drawable*, const juce::Colour*)
{
    using namespace theme;
    const auto r = area.toFloat().reduced (3.0f, 1.5f);
    if (isSeparator)
    {
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.fillRect (r.getX() + 6.0f, r.getCentreY(), r.getWidth() - 12.0f, 1.0f);
        return;
    }
    if (isHighlighted && isActive) // hover box, like the preset panel rows
    {
        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
    }
    if (isTicked)
    {
        g.setColour (violet.brighter (0.3f));
        g.fillEllipse (r.getX() + 9.0f, r.getCentreY() - 3.0f, 6.0f, 6.0f);
    }
    g.setFont (getPopupMenuFont());
    theme::shadowText (g, text, r.withTrimmedLeft (22.0f).withTrimmedRight (22.0f), juce::Justification::centredLeft,
                       ! isActive ? textFaint : (isTicked || isHighlighted ? theme::text : textDim.brighter (0.2f)));
    if (hasSubMenu)
    {
        juce::Path arrow;
        const float x = r.getRight() - 12.0f, y = r.getCentreY();
        arrow.startNewSubPath (x - 3.0f, y - 4.5f);
        arrow.lineTo (x + 1.5f, y);
        arrow.lineTo (x - 3.0f, y + 4.5f);
        g.setColour (isHighlighted ? theme::text : textDim);
        g.strokePath (arrow, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

juce::Rectangle<int> MetaTuneLookAndFeel::getTooltipBounds (const juce::String& tip, juce::Point<int> pos, juce::Rectangle<int> parent)
{
    juce::AttributedString s;
    s.append (tip, theme::font (13.0f, true), theme::text);
    juce::TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, 230.0f);
    const int w = int (tl.getWidth()) + 20, h = int (tl.getHeight()) + 14;
    return juce::Rectangle<int> (pos.x > parent.getCentreX() ? pos.x - (w + 12) : pos.x + 16,
                                 pos.y > parent.getCentreY() ? pos.y - (h + 8) : pos.y + 18, w, h)
        .constrainedWithin (parent);
}

void MetaTuneLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& tip, int w, int h)
{
    const auto r = juce::Rectangle<float> (0, 0, float (w), float (h));
    g.setColour (juce::Colour (0xf20a0a0b));
    g.fillRoundedRectangle (r, 7.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawRoundedRectangle (r.reduced (0.5f), 7.0f, 1.0f);
    juce::AttributedString s;
    s.append (tip, theme::font (13.0f, true), theme::text);
    s.setJustification (juce::Justification::centred);
    juce::TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, 230.0f);
    tl.draw (g, r.reduced (10.0f, 7.0f));
}
