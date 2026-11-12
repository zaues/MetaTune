#pragma once
#include <array>
#include <vector>
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

    void setDisplay (const DisplayState& s, float retuneSpeed, float bypassBlend, float seconds = 1.0f / 60.0f);
    void paint (juce::Graphics&) override;
    void resized() override { sphere = {}; grain = {}; fieldMask = {}; }
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
    static constexpr float kCoreR = 71.5f;                       // mean radius of the core contour
    float unit() const { return float (getHeight()) / 460.0f; } // layout unit: design height 460
    juce::Point<float> centre() const { return getLocalBounds().toFloat().getCentre(); }
    juce::Point<float> handlePos() const;
    juce::Rectangle<float> trackArea() const; // the reduced area the handle travels (stops short of the line ends)
    struct GuideGeom { float cornerX, baseY, topY, rightX, radius; }; // the visible L guide anchors
    enum class DragAxis { none, both, width, wet };
    GuideGeom guideGeom() const;
    DragAxis projectionAt (juce::Point<float>) const;
    void renderImages (float scale);
    static constexpr int contourSamples = 256;
    using Radii = std::array<float, contourSamples + 1>;
    Radii coreRadii (float radius) const;
    juce::Path coreContour (juce::Point<float>, const Radii&) const;
    void drawParticles (juce::Graphics&, juce::Point<float>, const Radii&, float flow, float colourFlow, float light);

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
    juce::Image sphere, fieldMask, grain, particleBuffer; // cached glass and neutral interior grain
    juce::Image coreField { juce::Image::ARGB, 112, 112, false, juce::SoftwareImageType {} };
    struct EdgePixel { int x, y, angle; float radius, fraction, grain; };
    std::vector<EdgePixel> edgePixels;
    float renderedScale = 0.0f;
    theme::Anim guide { *this, 0.2f }; // Doubler axis guides fade in on hover
    float coreAngle = 0;      // smoothed pitch class used for lighting and local deformation
    float spin = 0;           // independent particle drift
    float haloPhase = 0;      // rotation of the cached shell texture
    float contourPhase = 0;   // drift of the local contour waves
    float colourPhase = 0;    // flow of the interior lighting
    double t = 0;             // contour elapsed seconds
    double colourTime = 0;    // fluid interior motion
    float fieldPhase = 0;     // middle ring illumination phase
    float act = 0;            // smoothed voice activity 0..1
    float voicedSeconds = 0;
    float haloLight = 0;
    float voiceLevel = 0;     // slower envelope for identifying vocal attacks
    float syllable = 0;       // short-lived brightness lift on an attack
    float boltAmt = 0;        // bolt visibility 0..1: fast attack/release so it flickers with syllables
    float splitAmount = 0;    // fades between the plum and negative-Speed palettes
    float bypassAmount = 0;   // shared editor bypass transition, 0..1
};
