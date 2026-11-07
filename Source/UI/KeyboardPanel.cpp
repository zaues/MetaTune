#include "KeyboardPanel.h"

static const char* kSharpNames[] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
static const char* kFlatNames[] { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
static bool isBlack (int pc) { return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10; }
static int whiteIndex (int pc) { static const int w[] { 0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6 }; return w[pc]; }

juce::Rectangle<float> KeyboardPanel::labelRow() const
{
    const float y = float (kHeatTop + 2 * kHeatH + 2 * kHeatGap);
    return { 0.0f, y, float (getWidth()), float (kLabelH) };
}

juce::Rectangle<float> KeyboardPanel::piano() const
{
    const auto l = labelRow();
    return { 0.0f, l.getBottom(), float (getWidth()), float (getHeight()) - l.getBottom() };
}

juce::Rectangle<float> KeyboardPanel::keyRect (int pc) const
{
    const auto p = piano();
    const float cell = p.getWidth() / 12.0f;
    if (isBlack (pc))
        return { cell * float (pc), p.getY(), cell, p.getHeight() * 0.60f };
    const float ww = p.getWidth() / 7.0f;
    return { ww * float (whiteIndex (pc)), p.getY(), ww, p.getHeight() };
}

int KeyboardPanel::pitchClassAt (juce::Point<float> pt) const
{
    if (labelRow().contains (pt))
        return juce::jlimit (0, 11, int (pt.x / (float (getWidth()) / 12.0f)));
    if (! piano().contains (pt))
        return -1;
    for (int pc = 0; pc < 12; ++pc) // black keys sit on top: test them first
        if (isBlack (pc) && keyRect (pc).contains (pt)) return pc;
    for (int pc = 0; pc < 12; ++pc)
        if (! isBlack (pc) && keyRect (pc).contains (pt)) return pc;
    return -1;
}

void KeyboardPanel::addHeat (float inMidi, float outMidi, float w)
{
    auto splat = [w] (std::array<float, kBins>& h, float midi) {
        // Bin 0 is half a semitone below C, so each note's cell centre matches the label row.
        const float pos = std::fmod (midi + 0.5f, 12.0f) / 12.0f * float (kBins);
        const int c = int (pos);
        for (int k = -4; k <= 4; ++k)
        {
            const float d = (float (c + k) + 0.5f - pos) / 1.6f;
            h[size_t ((c + k + kBins) % kBins)] += w * std::exp (-d * d);
        }
    };
    if (inMidi > 0) splat (heatIn, inMidi);
    if (outMidi > 0) splat (heatOut, outMidi);
}

void KeyboardPanel::decayHeat (float f)
{
    for (auto* h : { &heatIn, &heatOut })
        for (auto& v : *h) v = v * f < 0.002f ? 0.0f : juce::jmin (4.0f, v * f);
    shimmer += 1.0f / 60.0f;
    if (shimmer > 3600.0f) shimmer -= 3600.0f;
    if (showHeat) repaint (0, 0, getWidth(), kHeatTop + 2 * kHeatH + kHeatGap);
}

void KeyboardPanel::drawHeat (juce::Graphics& g, juce::Rectangle<float> r, const std::array<float, kBins>& h) const
{
    using namespace theme;
    g.setColour (bg0);
    g.fillRect (r);
    // Full-height violet streaks that turn white-hot where the voice dwells.
    const float bw = r.getWidth() / float (kBins);
    for (int i = 0; i < kBins; ++i)
    {
        // Idle shimmer: two slow interfering waves keep a very faint violet glow moving when nobody is singing.
        const float idle = juce::jmax (0.0f, 0.06f + 0.05f * std::sin (float (i) * 0.37f + shimmer * 1.7f) * std::sin (float (i) * 0.11f - shimmer * 0.6f));
        const float v = juce::jlimit (0.0f, 1.0f, juce::jmax (h[size_t (i)] * 0.3f, idle));
        if (v <= 0.0f) continue;
        const auto col = v < 0.6f ? violet.withAlpha (v / 0.6f) : violet.interpolatedWith (juce::Colours::white, (v - 0.6f) / 0.4f);
        g.setColour (col);
        g.fillRect (r.getX() + float (i) * bw, r.getY(), bw + 0.6f, r.getHeight());
    }
}

void KeyboardPanel::paint (juce::Graphics& g)
{
    using namespace theme;
    const float W = float (getWidth());
    g.fillAll (juce::Colour (0xff111112));

    if (showHeat)
    {
        drawHeat (g, { 0.0f, float (heatInputY()), W, float (kHeatH) }, heatIn);
        drawHeat (g, { 0.0f, float (heatOutputY()), W, float (kHeatH) }, heatOut);
    }

    const bool live = state.voiced && state.presence > 0.05f;
    const int detectedPc = live ? ((juce::roundToInt (state.inputMidi) % 12) + 12) % 12 : -1;
    const int targetPc = live && state.target >= 0 ? state.target % 12 : -1;

    // Label row: the top of each key, grey over white keys and black over black keys.
    const auto lr = labelRow();
    const float cell = W / 12.0f;
    for (int pc = 0; pc < 12; ++pc)
    {
        auto r = juce::Rectangle<float> (cell * float (pc), lr.getY(), cell, lr.getHeight()).reduced (1.0f, 0.0f);
        const bool on = metatune::isAllowed (mask, pc);
        // Allowed notes: lighter cell with a lavender line at the bottom; forbidden notes recede.
        if (isBlack (pc)) g.setColour (on ? juce::Colour (0xff232325) : labelBlack);
        else              g.setColour (on ? labelWhite.brighter (0.12f) : juce::Colour (0xff3a3a3d));
        g.fillRect (r);
        if (pc == detectedPc && showDetected) // detected input note: the whole label lights up
        {
            g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.55f * state.presence), r.getX(), r.getY(),
                                                     juce::Colours::white.withAlpha (0.15f * state.presence), r.getX(), r.getBottom(), false));
            g.fillRect (r);
        }
        if (const float h = keyHover[size_t (pc)]->get(); h > 0.0f)
        {
            g.setColour (juce::Colours::white.withAlpha (0.08f * h));
            g.fillRect (r);
        }
        g.setColour (on ? text : text.withAlpha (0.45f));
        g.setFont (font (14.0f, true));
        g.drawText ((useFlats ? kFlatNames : kSharpNames)[pc], r, juce::Justification::centred);
        if (on)
        {
            g.setColour (lavender.withAlpha (0.9f));
            g.fillRect (r.removeFromBottom (2.5f).reduced (3.0f, 0.0f));
        }
    }

    // Piano: white keys, then black keys on top.
    auto drawKey = [&] (int pc) {
        const bool black = isBlack (pc);
        const auto r = keyRect (pc).reduced (1.0f, 0.0f);
        const bool on = metatune::isAllowed (mask, pc);
        juce::Colour top, bottom;
        if (black) { top = on ? blackOnTop : blackOff; bottom = on ? blackOnBottom : blackOff.darker (0.2f); }
        else       { top = on ? keyOnTop : keyOffTop; bottom = on ? keyOnBottom : keyOffBottom; }
        if (const float h = keyHover[size_t (pc)]->get(); h > 0.0f) { top = top.brighter (0.12f * h); bottom = bottom.brighter (0.12f * h); }
        juce::Path p;
        p.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), 3.0f, 3.0f, false, false, true, true);
        if (black)
            juce::DropShadow (juce::Colours::black.withAlpha (0.45f), 8, { 0, 3 }).drawForPath (g, p);
        g.setGradientFill (juce::ColourGradient (top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false));
        g.fillPath (p);
        if (showDetected && pc == targetPc) // output target: glow + outline
        {
            g.setColour (juce::Colours::white.withAlpha (0.16f * state.presence));
            g.fillPath (p);
            g.setColour (lavender.withAlpha (state.presence));
            g.strokePath (p, juce::PathStrokeType (2.0f));
        }
        if (pc == rootPc) // root marker
        {
            const float d = 6.0f;
            g.setColour (black ? lavender : juce::Colours::white.withAlpha (0.85f));
            g.fillEllipse (r.getCentreX() - d * 0.5f, r.getBottom() - d - 9.0f, d, d);
        }
    };
    for (int pc = 0; pc < 12; ++pc) if (! isBlack (pc)) drawKey (pc);
    for (int pc = 0; pc < 12; ++pc) if (isBlack (pc)) drawKey (pc);

    if (mask == 0)
    {
        const auto p = piano();
        g.setColour (bg0.withAlpha (0.75f));
        g.fillRect (p);
        g.setColour (warn);
        g.setFont (font (15.0f, true));
        g.drawText ("No notes enabled - correction paused. Click a key to enable it.", p, juce::Justification::centred);
    }
}

void KeyboardPanel::mouseDown (const juce::MouseEvent& e)
{
    const int pc = pitchClassAt (e.position);
    if (pc < 0) return;
    if (e.mods.isPopupMenu()) { if (onSetRoot) onSetRoot (pc); return; }
    if (onPress) onPress (pc);
    if (onToggleNote) onToggleNote (pc);
}

void KeyboardPanel::mouseMove (const juce::MouseEvent& e)
{
    const int pc = pitchClassAt (e.position);
    if (pc == hoverPc) return;
    if (hoverPc >= 0) keyHover[size_t (hoverPc)]->setTarget (0.0f);
    hoverPc = pc;
    if (hoverPc >= 0) keyHover[size_t (hoverPc)]->setTarget (1.0f);
}
