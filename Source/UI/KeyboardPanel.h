#pragma once
#include "DisplayState.h"
#include "Theme.h"
#include "../DSP/Pitch/Scale.h"
#include <array>
#include <functional>
#include <optional>

// Input/Output HeatMaps + note label row + one-octave piano, all sharing one pitch-class x axis
// Shows the allowed-note mask, detected input note and output target. The label row auditions
// pitches; the piano bodies edit the scale. Right-click either row to make a note the root.
class KeyboardPanel : public juce::Component, public juce::SettableTooltipClient
{
public:
    static constexpr int kHeatTop = 1, kHeatH = 21, kHeatGap = 2, kLabelH = 62;
    static constexpr int kBins = 480; // heat resolution: 2.5 cents per bin

    std::function<void (int pitchClass)> onToggleNote, onSetRoot;
    std::function<void (int pitchClass)> onPress; // press a label to audition
    std::function<void()> onRelease;              // release the audition

    void setMask (metatune::PitchClassMask m) { if (m != mask) { mask = m; repaint(); } }
    void setPreviewMask (std::optional<metatune::PitchClassMask>);
    void setAuditionMode (int mode) { auditionMode = mode; }
    void setBypassBlend (float blend);
    void setShowHeat (bool b) { if (b != showHeat) { showHeat = b; repaint(); } }
    void setShowDetected (bool b) { if (b != showDetected) { showDetected = b; repaint(); } }
    void resetHeat();
    void resetDetected();
    void setUseFlats (bool b) { if (b != useFlats) { useFlats = b; repaint(); } }
    void addHeat (float inputMidi, float outputMidi, float weight); // per telemetry sample
    void decayHeat (float factor);                                  // per frame
    void setDisplay (const DisplayState& s) { state = s; repaint(); }

    int heatInputY() const noexcept { return kHeatTop; }
    int heatOutputY() const noexcept { return kHeatTop + kHeatH + kHeatGap; }

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent& e) override { mouseMove (e); }
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    int pitchClassAt (juce::Point<float>) const;
    juce::Rectangle<float> labelRow() const;
    juce::Rectangle<float> labelCell (int pc) const;
    juce::Rectangle<float> piano() const;
    juce::Rectangle<float> keyRect (int pc) const;
    juce::Path blackKeyShape (juce::Rectangle<float>) const;
    void setHoveredKey (int pc, bool label);
    void drawKeyOutline (juce::Graphics&, const juce::Path&, int pc, juce::Colour) const;
    struct HeatHistory
    {
        std::array<float, kBins> total {}, recent {};
        void add (float position, float weight, float width);
    };
    void drawHeat (juce::Graphics&, juce::Rectangle<int>, const HeatHistory&, juce::Image&) const;

    metatune::PitchClassMask mask = metatune::kAllNotes;
    std::optional<metatune::PitchClassMask> previewMask;
    metatune::PitchClassMask auditionMask = 0;
    int hoverPc = -1, auditionMode = 0, pressedMode = 0;
    bool hoverLabel = false, auditionPressed = false;
    theme::Anim auditionGlow { *this, 0.25f };
    float bypassBlend = 0.0f;
    bool showHeat = true, showDetected = true, useFlats = false;
    HeatHistory heatIn, heatOut;
    std::array<float, 12> detectedHistory {};
    static constexpr int samplesPerBin = 3;
    juce::Image inputStrip { juce::Image::RGB, kBins * samplesPerBin, 1, false };
    juce::Image outputStrip { juce::Image::RGB, kBins * samplesPerBin, 1, false };
    std::array<std::unique_ptr<theme::Anim>, 12> keyHover { makeAnims() };
    std::array<std::unique_ptr<theme::Anim>, 12> makeAnims()
    {
        std::array<std::unique_ptr<theme::Anim>, 12> a;
        for (auto& x : a) x = std::make_unique<theme::Anim> (*this, 0.25f);
        return a;
    }
    DisplayState state;
};
