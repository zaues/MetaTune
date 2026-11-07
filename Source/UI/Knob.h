#pragma once
#include "Theme.h"
#include <juce_audio_processors/juce_audio_processors.h>

// Custom rotary control with a tick ring and a seven-segment readout. Drag vertically/horizontally;
// Shift = fine; double-click = default; right-click = type a value.
// Attach with AudioProcessorValueTreeState::SliderAttachment.
class Knob : public juce::Slider
{
public:
    struct Style
    {
        float startDeg = -135.0f, endDeg = 135.0f; // sweep, 0 = 12 o'clock, clockwise
        bool ticks = true;
        int cells = 3;                             // seven-segment digit cells
        float bandFrom = -1.0f;                    // >= 0: proportion where the gradient "aggressive" band starts
        juce::String unit;                         // shown in place of the label while hovering/dragging
    };

    Knob (juce::String label, std::function<juce::String (double)> format, Style);

    // Optional custom proportion <-> value mapping (e.g. Speed: clockwise = harder, aggressive band at the end).
    std::function<double (double)> toValue, toProportion;
    double proportionOfLengthToValue (double p) override { return toValue ? toValue (p) : Slider::proportionOfLengthToValue (p); }
    double valueToProportionOfLength (double v) override { return toProportion ? toProportion (v) : Slider::valueToProportionOfLength (v); }

    // A neighbouring knob nests into this one: its disc (plus a gap) is cut out of this knob.
    void setCutout (juce::Point<float> centreInParent, float radius) { cutCentre = centreInParent; cutRadius = radius; repaint(); }
    // 0..1 voice activity: the aggressive band glows brighter while someone is singing.
    void setActivity (float a) { if (std::abs (a - activity) > 0.01f) { activity = a; repaint(); } }

    void paint (juce::Graphics&) override;
    bool hitTest (int x, int y) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override { hover.setTarget (1.0f); }
    void mouseExit (const juce::MouseEvent&) override { if (! isMouseButtonDown()) hover.setTarget (0.0f); }
    void mouseUp (const juce::MouseEvent& e) override
    {
        Slider::mouseUp (e);
        turning.setTarget (0.0f);
        if (! isMouseOver()) hover.setTarget (0.0f);
    }

private:
    void showValueEditor();

    juce::String label;
    std::function<juce::String (double)> format;
    Style style;
    theme::Anim hover { *this };
    theme::Anim turning { *this, 0.3f }; // readout tint while the knob is being turned
    float activity = 0;
    juce::Point<float> cutCentre;
    float cutRadius = 0;
    std::unique_ptr<juce::TextEditor> editor;
};
