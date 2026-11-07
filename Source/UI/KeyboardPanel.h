#pragma once
#include "DisplayState.h"
#include "Theme.h"
#include "../DSP/Pitch/Scale.h"
#include <array>
#include <functional>

// Input/Output HeatMaps + note label row + one-octave piano, all sharing one pitch-class x axis
// (12 equal cells, cell centre = note). Shows the allowed-note mask, root, detected input note and
// output target. Click a note to toggle it; right-click a note to make it the root.
class KeyboardPanel : public juce::Component, public juce::SettableTooltipClient
{
public:
    static constexpr int kHeatTop = 3, kHeatH = 17, kHeatGap = 4, kLabelH = 64;
    static constexpr int kBins = 240; // heat resolution: 5 cents per bin

    std::function<void (int pitchClass)> onToggleNote, onSetRoot;
    std::function<void (int pitchClass)> onPress; // left button down on a key (reference tone on)
    std::function<void()> onRelease;              // button up (reference tone off)

    void setMask (metatune::PitchClassMask m, int root) { if (m != mask || root != rootPc) { mask = m; rootPc = root; repaint(); } }
    void setShowHeat (bool b) { showHeat = b; repaint(); }
    void setShowDetected (bool b) { showDetected = b; repaint(); }
    void setUseFlats (bool b) { if (b != useFlats) { useFlats = b; repaint(); } }
    void addHeat (float inputMidi, float outputMidi, float weight); // per telemetry sample
    void decayHeat (float factor);                                  // per frame
    void setDisplay (const DisplayState& s) { state = s; repaint(); }

    int heatInputY() const noexcept { return kHeatTop; }
    int heatOutputY() const noexcept { return kHeatTop + kHeatH + kHeatGap; }

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override { if (onRelease) onRelease(); }
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override { if (hoverPc >= 0) keyHover[size_t (hoverPc)]->setTarget (0.0f); hoverPc = -1; }

private:
    int pitchClassAt (juce::Point<float>) const;
    juce::Rectangle<float> labelRow() const;
    juce::Rectangle<float> piano() const;
    juce::Rectangle<float> keyRect (int pc) const;
    void drawHeat (juce::Graphics&, juce::Rectangle<float>, const std::array<float, kBins>&) const;

    metatune::PitchClassMask mask = metatune::kAllNotes;
    int rootPc = 0, hoverPc = -1;
    bool showHeat = true, showDetected = true, useFlats = false;
    float shimmer = 0; // idle animation clock for the light bars (advanced in decayHeat)
    std::array<float, kBins> heatIn {}, heatOut {};
    std::array<std::unique_ptr<theme::Anim>, 12> keyHover { makeAnims() };
    std::array<std::unique_ptr<theme::Anim>, 12> makeAnims()
    {
        std::array<std::unique_ptr<theme::Anim>, 12> a;
        for (auto& x : a) x = std::make_unique<theme::Anim> (*this, 0.25f);
        return a;
    }
    DisplayState state;
};
