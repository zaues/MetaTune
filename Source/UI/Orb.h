#pragma once
#include "DisplayState.h"
#include "Theme.h"
#include <juce_audio_processors/juce_audio_processors.h>

// Central visualiser, built from independent layers (outside in):
//  - atmosphere: a smoky glass sphere that breathes, with a reflection arc drifting over its upper edge;
//  - orbit: a ring of fine ticks with a comet highlight running around it;
//  - field: a thin violet ring with a slow pulse;
//  - haze: a faint spinning particle band around the core;
//  - core: an organic, wobbling contour (cyan -> white-lavender) around a swirling plum/teal sheen;
//  - symbol: a crisp white bolt that only shows while a voice is detected, flickering with its level.
// Voice activity speeds things up, brightens the core and makes its contour ripple harder.
// The grey ball is an XY pad over the whole stage (Doubler): right = wider, up = more of the doubled voices.
class Orb : public juce::Component, public juce::SettableTooltipClient
{
public:
    Orb (juce::RangedAudioParameter& xParam, juce::RangedAudioParameter& yParam, juce::String xName, juce::String yName, juce::UndoManager*);

    void setDisplay (const DisplayState& s, bool bypassed);
    void paint (juce::Graphics&) override;
    void resized() override { sphere = {}; core = {}; }
    bool hitTest (int x, int y) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override { hoverHandle = false; repaint(); }
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    static constexpr float kSphereR = 154.0f, kOrbitR = 140.0f, kFieldR = 124.0f; // layer radii (design units)
    static constexpr float kCoreR = 64.0f;                       // mean radius of the core contour
    static constexpr float kHazeR = 92.0f;                       // outer radius of the particle haze
    float unit() const { return float (getHeight()) / 460.0f; } // layout unit: design height 460
    juce::Point<float> centre() const { return getLocalBounds().toFloat().getCentre(); }
    juce::Point<float> handlePos() const;
    juce::Rectangle<float> padArea() const;
    juce::Rectangle<float> trackArea() const; // the L-shaped guide extents the handle travels within
    void renderImages();

    juce::RangedAudioParameter& xParam;
    juce::RangedAudioParameter& yParam;
    juce::String xName, yName;
    juce::ParameterAttachment xAttach, yAttach;
    float xNorm = 0, yNorm = 0;
    bool dragging = false, hoverHandle = false, bypassed = false;
    juce::Point<float> lockFrom; float lockX = 0, lockY = 0; // Alt-drag: anchor to lock one Doubler axis
    DisplayState state;
    juce::Image sphere, core; // static artwork, rendered once per size
    theme::Anim guide { *this, 0.2f }; // Doubler axis guides fade in on hover
    float coreAngle = 0;      // smoothed rotation of the core (follows the output pitch class)
    float spin = 0;           // continuous rotation of the particle haze
    float t = 0;              // animation clock, seconds (advanced by setDisplay at ~60 Hz)
    float act = 0;            // smoothed voice activity 0..1
    float boltAmt = 0;        // bolt visibility 0..1: fast attack/release so it flickers with syllables
    juce::Path coreContour (juce::Point<float> c, float radius) const;
};
