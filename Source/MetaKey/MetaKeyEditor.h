#pragma once
#include "MetaKeyProcessor.h"
#include "../UI/Theme.h"

// MetaKey's window: a detection-method row (Listen / Use File / Manual), a big key readout with a
// confidence bar, and a keyboard that highlights the detected key. Everything is drawn directly and
// hit-tested in mouseDown; a timer repaints so the live key updates.
class MetaKeyEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    static constexpr int kW = 560, kH = 500;
    explicit MetaKeyEditor (MetaKeyProcessor&);
    ~MetaKeyEditor() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }

private:
    void timerCallback() override { repaint(); }
    juce::Rectangle<float> methodTile (int i) const;   // 0 Listen, 1 Use File, 2 Manual
    juce::Rectangle<float> minorToggle() const;
    juce::Rectangle<float> restartBtn() const;
    juce::Rectangle<float> keyRect (int pc) const;     // white/black key in the keyboard
    void openFileChooser();

    MetaKeyProcessor& proc;
    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetaKeyEditor)
};
