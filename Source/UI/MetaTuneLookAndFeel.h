#pragma once
#include "Theme.h"

// Styles the stock JUCE pieces we still use: popup menus, tooltips, text editors.
class MetaTuneLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MetaTuneLookAndFeel();

    juce::Font getPopupMenuFont() override { return theme::font (13.5f, true); }
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
    void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                            bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* textColour) override;
    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                    int& idealWidth, int& idealHeight) override;
    int getPopupMenuBorderSize() override { return 6; }
    juce::Rectangle<int> getTooltipBounds (const juce::String&, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;
    void drawTooltip (juce::Graphics&, const juce::String&, int width, int height) override;
};
