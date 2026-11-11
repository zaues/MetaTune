#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Visual tokens shared by every component. Change colours here, never inline.
namespace theme
{

// Neutral surface palette. MUTABLE: setTheme() swaps the whole set between the dark and light
// themes at runtime. Draw surfaces with these tokens (never inline hex) so light mode works.
inline juce::Colour bg0 { 0xff0d0e10 };   // deepest (strips, wells, heat rows)
inline juce::Colour bg1 { 0xff121316 };   // window: obsidian
inline juce::Colour bg2 { 0xff17181c };   // raised panels, mode row
inline juce::Colour bg3 { 0xff24262d };   // controls, hover
inline juce::Colour line { 0xff2e3038 };  // 1 px borders
inline juce::Colour text { 0xfff2f1f7 };
inline juce::Colour textDim { 0xffa09eac };
inline juce::Colour textFaint { 0xff5f5d6b };

inline juce::Colour header { 0xff17181c };    // header bar
inline juce::Colour headerBox { 0xff121316 }; // header cells (preset box, title)
inline juce::Colour stage { 0xff121316 };     // stage behind the orb
inline juce::Colour highlight { 0xff1e2026 }; // soft radial highlights on the background
inline juce::Colour panel { 0xff1e2026 };     // brushed-metal panel behind the knobs
inline juce::Colour knobBody { 0xff18191c };
inline juce::Colour menuBg { 0xff1c1c1e };    // dropdown / browser panel surface
inline juce::Colour menuBg2 { 0xff121213 };   // group-tab rail, deepest menu surface
inline juce::Colour menuSel { 0xff252528 };   // selected / highlighted menu row
inline juce::Colour menuTab { 0xff232327 };   // browser tab strip
inline juce::Colour well { 0xff141416 };      // text fields, slider tracks

// Dark face and material samples from the normalized reference.
namespace dark
{
inline const juce::Colour toolbarTop { 0xff2d2d2d }, toolbarBottom { 0xff232323 };
inline const juce::Colour toolbarCell { 0xff252525 }, inset { 0xff090909 };
inline const juce::Colour stage { 0xff0d0d0e }, graphite { 0xff3a393b };
inline const juce::Colour rail { 0xff121212 }, railCap { 0xff2d2d2e }, railRow { 0xff151515 };
inline const juce::Colour knobFace { 0xff1d1d1d }, knobEdge { 0xff2c2c2c };
inline const juce::Colour modeStrip { 0xff262626 }, control { 0xff2c2c2c };
inline const juce::Colour notePill { 0xff655385 }, noteShade { 0xff544676 };
inline const juce::Colour scalePanel { 0xff2e2e2e }, utilityPanel { 0xff2a2a2a };
inline const juce::Colour timingAmber { 0xfffecc8e }, timingTrack { 0xffa68765 };
inline const juce::Colour heatTrack { 0xff101011 }, labelWhite { 0xff656565 }, labelBlack { 0xff303030 };
}

inline bool lightMode = false;
void setTheme (bool light); // swap the neutral palette (keeps the current accent)

// The signature accent palette. These are MUTABLE: setAccent() recolours the whole family at runtime
// (hue rotation + saturation/brightness) so the user can change the plugin's colour. Their originals
// live in `base::` below and are never modified.
inline juce::Colour violet { 0xffa855f7 };     // neon accent
inline juce::Colour violetDeep { 0xff7c3aed }; // active toggles
inline juce::Colour pink { 0xffec4899 };       // hot magenta accent
inline juce::Colour coral { 0xfffb7185 };
inline const juce::Colour bypassAccent { 0xffdf7779 };
inline const juce::Colour majorAudition { 0xff87caff }, minorAudition { 0xff81e6d3 };
inline juce::Colour blue { 0xff6aa8ff };
inline juce::Colour cyan { 0xff06b6d4 };
inline juce::Colour lavender { 0xffc4b5fd };
inline juce::Colour rose { 0xffb35c87 };
inline juce::Colour lilac { 0xffeccdf1 };
inline juce::Colour amber { 0xfff4d09c };
inline juce::Colour warn { 0xffffb35c };

// Keyboard. Allowed keys glow in the accent colour; forbidden keys are plain grey/black.
inline juce::Colour keyOnTop { 0xff9277fb }, keyOnBottom { 0xff897ea8 };
inline juce::Colour keyOffTop { 0xff646467 }, keyOffBottom { 0xff535355 };   // mutable: lighten in light mode
inline juce::Colour blackOnTop { 0xff503e8b }, blackOnBottom { 0xff4b445e };
inline juce::Colour blackOff { 0xff262628 };
inline juce::Colour labelWhite { 0xff5d5d60 }, labelBlack { 0xff19191a };
inline juce::Colour greyText { 0xff7d808a };                                // inactive text in the key drawer

namespace base // immutable originals the accent palette is derived from
{
inline constexpr juce::uint32 violet = 0xffa855f7, violetDeep = 0xff7c3aed, pink = 0xffec4899,
    coral = 0xfffb7185, blue = 0xff6aa8ff, cyan = 0xff06b6d4, lavender = 0xffc4b5fd, rose = 0xffb35c87,
    lilac = 0xffeccdf1, amber = 0xfff4d09c, warn = 0xffffb35c, keyOnTop = 0xff9277fb,
    keyOnBottom = 0xff897ea8, blackOnTop = 0xff503e8b, blackOnBottom = 0xff4b445e;
}

// Current recolouring, so hard-coded accent colours elsewhere (e.g. the orb) can match.
inline float accentHue = 0.0f, accentSat = 1.0f, accentBright = 1.0f;
inline juce::Colour recolour (juce::Colour c)
{
    return c.withRotatedHue (accentHue).withMultipliedSaturation (accentSat).withMultipliedBrightness (accentBright);
}
// hue 0..1 (fraction of a full turn), saturation/brightness are multipliers around 1.
void setAccent (float hue, float saturation, float brightness);

// Two-colour accent gradient override. When on, the whole accent family and accentAt()/tickColourAt()
// are built from these two endpoint colours instead of the hue-rotation palette.
inline bool gradientOn = false;
inline juce::Colour gradA { 0xffec4899 }, gradB { 0xff6aa8ff };
void setGradient (bool on, juce::Colour a, juce::Colour b);

// Per-element extra hue rotation (orb / keyboard / meters), applied on top of the accent (0 = none).
inline float orbHue = 0.0f, keyHue = 0.0f, meterHue = 0.0f;
inline juce::Colour tint (juce::Colour c, float extraHue) { return extraHue == 0.0f ? c : c.withRotatedHue (extraHue); }
inline juce::Colour orbTint (juce::Colour c) { return tint (recolour (c), orbHue); } // recolour + the orb's own hue offset

// Meter tracks: hot pink/coral -> bright cyan -> deep violet along the sweep (u = 0..1).
inline juce::Colour tickColourAt (float u)
{
    u = juce::jlimit (0.0f, 1.0f, u);
    const juce::Colour c = gradientOn ? gradA.interpolatedWith (gradB, u)
        : (u < 0.5f ? coral.interpolatedWith (cyan, u * 2.0f) : cyan.interpolatedWith (violetDeep.brighter (0.2f), (u - 0.5f) * 2.0f));
    return tint (c, meterHue);
}

// Position 0..1 along the signature pink -> violet -> blue gradient.
inline juce::Colour accentAt (float t)
{
    t = juce::jlimit (0.0f, 1.0f, t);
    if (gradientOn) return gradA.interpolatedWith (gradB, t);
    return t < 0.5f ? pink.interpolatedWith (violet, t * 2.0f) : violet.interpolatedWith (blue, (t - 0.5f) * 2.0f);
}

float surfaceNoise (float x, float y) noexcept;
juce::Font font (float height, bool semibold = false);

// Text with a soft drop shadow. The font must already be set on g; pass the text colour.
// Templated on the rectangle type so any Rectangle<int>/<float> works at the call site.
template <typename RectType>
inline void shadowText (juce::Graphics& g, const juce::String& t, RectType area,
                        juce::Justification j, juce::Colour colour, float dy = 1.6f, float alpha = 0.7f)
{
    const auto a = area.toFloat();
    const float sh = alpha * colour.getFloatAlpha() * (lightMode ? 0.3f : 1.0f); // shadow fades with the text; lighter in light mode
    // Two passes: a softer, lower one for spread, then the main shadow, then the text.
    g.setColour (juce::Colours::black.withAlpha (sh * 0.5f));
    g.drawText (t, a.translated (0.0f, dy + 1.0f), j);
    g.setColour (juce::Colours::black.withAlpha (sh));
    g.drawText (t, a.translated (0.0f, dy), j);
    g.setColour (colour);
    g.drawText (t, a, j);
}
template <typename RectType>
inline void shadowFittedText (juce::Graphics& g, const juce::String& t, RectType area,
                              juce::Justification j, int maxLines, juce::Colour colour, float alpha = 0.7f)
{
    const auto a = area.toFloat().toNearestInt();
    const float sh = alpha * colour.getFloatAlpha() * (lightMode ? 0.3f : 1.0f);
    g.setColour (juce::Colours::black.withAlpha (sh * 0.5f));
    g.drawFittedText (t, a.translated (0, 3), j, maxLines);
    g.setColour (juce::Colours::black.withAlpha (sh));
    g.drawFittedText (t, a.translated (0, 2), j, maxLines);
    g.setColour (colour);
    g.drawFittedText (t, a, j, maxLines);
}

// Eases a value toward a target at 60 Hz and repaints the owner while moving (hover fades, slides).
class Anim : private juce::Timer
{
public:
    explicit Anim (juce::Component& c, float rate = 0.22f) : owner (c), k (rate) {}
    void setTarget (float t)
    {
        target = t;
        if (std::abs (target - value) > 0.001f && ! isTimerRunning()) startTimerHz (60);
    }
    void snapTo (float t) { target = value = t; stopTimer(); }
    float get() const noexcept { return value; }
    operator float() const noexcept { return value; }

private:
    void timerCallback() override
    {
        value += (target - value) * k;
        if (std::abs (target - value) < 0.002f) { value = target; stopTimer(); }
        owner.repaint();
    }
    juce::Component& owner;
    float k, value = 0, target = 0;
};

// Seven-segment readout, centred in `area` with `cells` digit cells (value right-aligned); unlit
// segments show as a faint ghost "8". Understands 0-9, '-', ' ' and '.' (the dot sits between cells).
void drawSevenSeg (juce::Graphics&, const juce::String& value, juce::Rectangle<float> area, int cells,
                   juce::Colour on, juce::Colour ghost, bool centre = false);

// Lightning bolt fitted into `area`.
juce::Path bolt (juce::Rectangle<float> area);
} // namespace theme
