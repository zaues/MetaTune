#pragma once
#include "DisplayState.h"
#include "Theme.h"
#include <juce_audio_processors/juce_audio_processors.h>

// Central visualiser, built from independent layers (outside in):
//  - atmosphere: a smoky glass sphere with a pearlescent rim;
//  - field: a thin violet ring with a slow pulse;
//  - haze: a faint spinning particle band around the core;
//  - core: a pitch-driven fold with a plum palette or a vivid negative-Speed split;
//  - symbol: a pink-to-cyan bolt that reacts to the voice only at negative Speed.
// Voice activity speeds things up, brightens the core and makes its contour ripple harder.
// The grey ball is an XY pad over the whole stage (Doubler): right = wider, up = more of the doubled voices.
class Orb : public juce::Component, public juce::SettableTooltipClient
{
public:
    Orb (juce::RangedAudioParameter& xParam, juce::RangedAudioParameter& yParam, juce::String xName, juce::String yName, juce::UndoManager*);

    void setDisplay (const DisplayState& s, float retuneSpeed, float bypassBlend);
    void paint (juce::Graphics&) override;
    void resized() override { sphere = {}; core = {}; plumCore = {}; }
    bool hitTest (int x, int y) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override { hoverHandle = false; hoverLine = -1; repaint(); }
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    friend struct MetaTuneSnapshotAccess;

    static constexpr float kSphereR = 172.0f, kFieldR = 110.8f; // layer radii (design units)
    static constexpr float kCoreR = 72.0f;                       // mean radius of the core contour
    static constexpr float kHazeR = 96.0f;                       // outer radius of the particle haze
    float unit() const { return float (getHeight()) / 460.0f; } // layout unit: design height 460
    juce::Point<float> centre() const { return getLocalBounds().toFloat().getCentre(); }
    juce::Point<float> handlePos() const;
    juce::Rectangle<float> trackArea() const; // the reduced area the handle travels (stops short of the line ends)
    struct GuideGeom { float cornerX, baseY, topY, rightX, radius; }; // the visible L guide anchors
    enum class DragAxis { none, both, width, wet };
    GuideGeom guideGeom() const;
    DragAxis projectionAt (juce::Point<float>) const;
    void renderImages (float scale);

    juce::RangedAudioParameter& xParam;
    juce::RangedAudioParameter& yParam;
    juce::String xName, yName;
    juce::UndoManager* undoManager;
    juce::ParameterAttachment xAttach, yAttach;
    float xNorm = 0, yNorm = 0;
    DragAxis dragAxis = DragAxis::none;
    bool hoverHandle = false, negativeSpeed = false;
    int hoverLine = -1; // which Doubler guide line the pointer is over: -1 none, 0 vertical (Dry/Wet), 1 horizontal (Width)
    juce::Point<float> lockFrom; float lockX = 0, lockY = 0; // Alt-drag: anchor to lock one Doubler axis
    juce::Image sphere, core, plumCore; // cached glass and mode-specific particles
    float renderedScale = 0.0f;
    theme::Anim guide { *this, 0.2f }; // Doubler axis guides fade in on hover
    float coreAngle = 0;      // smoothed rotation of the core (follows the output pitch class)
    float spin = 0;           // continuous rotation of the particle haze
    float t = 0;              // animation clock, seconds (advanced by setDisplay at ~60 Hz)
    float act = 0;            // smoothed voice activity 0..1
    float voiceLevel = 0;     // slower envelope for identifying vocal attacks
    float syllable = 0;       // short-lived brightness lift on an attack
    float boltAmt = 0;        // bolt visibility 0..1: fast attack/release so it flickers with syllables
    float splitAmount = 0;    // fades between the plum and negative-Speed palettes
    float bypassAmount = 0;   // shared editor bypass transition, 0..1
    juce::Path coreContour (juce::Point<float> c, float radius, float flow) const;
};
