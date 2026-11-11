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

juce::Rectangle<float> KeyboardPanel::labelCell (int pc) const
{
    // Seams measured along the label row, relative to the keyboard's left edge.
    static constexpr int edges[] { 0, 88, 174, 263, 349, 435, 522, 609, 695, 780, 871, 957, 1043 };
    const auto row = labelRow();
    const float scale = row.getWidth() / float (edges[12]);
    return row.withLeft (float (edges[pc]) * scale).withRight (float (edges[pc + 1]) * scale);
}

juce::Rectangle<float> KeyboardPanel::piano() const
{
    const auto l = labelRow();
    return { 0.0f, l.getBottom(), float (getWidth()), float (getHeight()) - l.getBottom() };
}

juce::Rectangle<float> KeyboardPanel::keyRect (int pc) const
{
    const auto p = piano();
    if (isBlack (pc))
        return labelCell (pc).withY (p.getY()).withHeight (p.getHeight() * 0.58f);
    // Lower white-key seams measured at y=1015 on the 1415 px reference.
    static constexpr int whiteEdges[] { 182, 332, 475, 617, 767, 924, 1078, 1225 };
    const int i = whiteIndex (pc);
    const float scale = p.getWidth() / float (whiteEdges[7] - whiteEdges[0]);
    const float left = float (whiteEdges[i] - whiteEdges[0]) * scale;
    const float width = float (whiteEdges[i + 1] - whiteEdges[i]) * scale;
    return { left, p.getY(), width, p.getHeight() };
}

juce::Path KeyboardPanel::blackKeyShape (juce::Rectangle<float> r) const
{
    const float flare = r.getWidth() * 0.058f;
    const float corner = r.getWidth() * 0.18f;
    const float left = r.getX() - flare, right = r.getRight() + flare;
    const float bottom = r.getBottom();
    juce::Path p;
    p.startNewSubPath (r.getX(), r.getY());
    p.lineTo (r.getRight(), r.getY());
    p.lineTo (right, bottom - corner);
    p.quadraticTo (right + 0.6f, bottom, right - corner, bottom);
    p.lineTo (left + corner, bottom);
    p.quadraticTo (left - 0.6f, bottom, left, bottom - corner);
    p.closeSubPath();
    return p;
}

int KeyboardPanel::pitchClassAt (juce::Point<float> pt) const
{
    if (labelRow().contains (pt))
    {
        for (int pc = 0; pc < 12; ++pc)
            if (labelCell (pc).contains (pt)) return pc;
    }
    if (! piano().contains (pt))
        return -1;
    for (int pc = 0; pc < 12; ++pc) // black keys sit on top: test them first
        if (isBlack (pc) && blackKeyShape (keyRect (pc).reduced (1.0f, 0.0f)).contains (pt)) return pc;
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
        for (int k = -3; k <= 3; ++k)
        {
            const float d = (float (c + k) + 0.5f - pos) / 1.13f;
            h[size_t ((c + k + kBins) % kBins)] += w * std::exp (-d * d);
        }
    };
    if (inMidi > 0) splat (heatIn, inMidi);
    if (outMidi > 0) splat (heatOut, outMidi);
}

void KeyboardPanel::decayHeat (float f)
{
    for (auto* h : { &heatIn, &heatOut })
    {
        const float decay = h == &heatIn ? std::sqrt (f) : std::pow (f, 0.82f);
        for (auto& v : *h) v = v * decay < 0.002f ? 0.0f : juce::jmin (4.0f, v * decay);
    }
    shimmer += 1.0f / 60.0f;
    if (shimmer > 3600.0f) shimmer -= 3600.0f;
    if (showHeat) repaint (0, 0, getWidth(), kHeatTop + 2 * kHeatH + kHeatGap);
}

void KeyboardPanel::drawHeat (juce::Graphics& g, juce::Rectangle<int> r,
                              const std::array<float, kBins>& h, juce::Image& strip) const
{
    using namespace theme;
    const auto ground = lightMode ? bg0 : dark::heatTrack;
    const auto violetGlow = tint (recolour (juce::Colour (0xff750bff)), meterHue);
    const auto whiteHot = juce::Colour (0xfffcf7ff);
    const auto energyAt = [&] (int i) { return h[size_t ((i + kBins) % kBins)]; };

    std::array<float, kBins> crests;
    for (int i = 0; i < kBins; ++i)
    {
        const float energy = energyAt (i);
        const float shoulder = (energyAt (i - 3) + energyAt (i + 3)) * 0.5f;
        const float contrast = juce::jmax (0.0f, energy - shoulder);
        crests[size_t (i)] = juce::jlimit (0.0f, 1.0f, (1.32f * contrast) / (contrast + 0.38f));
    }
    const auto crestAt = [&] (int i) { return crests[size_t ((i + kBins) % kBins)]; };

    // A one-pixel-high image gives every streak the full row height with sub-bin precision.
    // Interpolate the live histogram and concentrate the white light at its local peaks.
    {
        juce::Image::BitmapData pixels (strip, juce::Image::BitmapData::writeOnly);
        for (int i = 0; i < kBins; ++i)
        {
            const float energy = energyAt (i);
            const float left = energyAt (i - 1), right = energyAt (i + 1);
            const float peak = crestAt (i);
            const float idle = 0.014f * (1.0f + std::sin (float (i) * 0.37f + shimmer * 1.7f)
                                               * std::sin (float (i) * 0.11f - shimmer * 0.6f));
            for (int s = 0; s < samplesPerBin; ++s)
            {
                const float offset = float (s - 1) / float (samplesPerBin);
                const int side = offset < 0.0f ? -1 : 1;
                const float fraction = std::abs (offset);
                const float sample = std::lerp (energy, side < 0 ? left : right, fraction);
                const float strength = sample / (sample + 0.43f);
                const float core = std::lerp (peak, crestAt (i + side), fraction);
                const float grain = 0.91f + 0.09f * std::sin ((float (i) + offset) * 2.38f);
                const float brightness = juce::jlimit (0.0f, 1.0f, idle + 0.79f * strength * grain + 0.52f * core);
                const float white = juce::jlimit (0.0f, 1.0f, (core - 0.32f) * 1.54f);
                const auto light = violetGlow.interpolatedWith (whiteHot, white * white);
                pixels.setPixelColour (i * samplesPerBin + s, 0, ground.interpolatedWith (light, brightness));
            }
        }
    }
    g.drawImage (strip, r.getX(), r.getY(), r.getWidth(), r.getHeight(), 0, 0, strip.getWidth(), 1);
}

void KeyboardPanel::paint (juce::Graphics& g)
{
    using namespace theme;
    g.fillAll (lightMode ? bg1 : dark::inset);

    if (showHeat)
    {
        drawHeat (g, { 0, heatInputY(), getWidth(), kHeatH }, heatIn, inputStrip);
        drawHeat (g, { 0, heatOutputY(), getWidth(), kHeatH }, heatOut, outputStrip);
    }

    const bool live = state.voiced && state.presence > 0.05f;
    const int detectedPc = live ? ((juce::roundToInt (state.inputMidi) % 12) + 12) % 12 : -1;
    const int targetPc = live && state.target >= 0 ? state.target % 12 : -1;

    // Label row: the top of each key, grey over white keys and black over black keys.
    for (int pc = 0; pc < 12; ++pc)
    {
        const auto r = labelCell (pc).reduced (0.65f, 0.0f);
        const auto base = isBlack (pc) ? labelBlack : labelWhite;
        const auto top = base.brighter (isBlack (pc) ? 0.025f : 0.065f);
        g.setGradientFill (juce::ColourGradient (top, r.getTopLeft(), base, r.getBottomLeft(), false));
        g.fillRect (r);
        if (const float h = keyHover[size_t (pc)]->get(); h > 0.0f)
        {
            g.setColour (juce::Colours::white.withAlpha (0.08f * h));
            g.fillRect (r);
        }
        if (isBlack (pc) && metatune::isAllowed (mask, pc))
        {
            g.setColour (tint (lavender, keyHue).withAlpha (0.45f));
            g.fillRect (r.withTop (r.getBottom() - 1.15f).reduced (2.6f, 0.0f));
        }
        if (pc == detectedPc && showDetected)
        {
            g.setColour (juce::Colours::white.withAlpha (0.82f * state.presence));
            g.fillRect (r.withTop (r.getBottom() - 0.95f));
        }
        g.setFont (font (17.2f).withExtraKerningFactor (0.08f));
        shadowText (g, (useFlats ? kFlatNames : kSharpNames)[pc], r, juce::Justification::centred,
                    lightMode ? text : juce::Colour (0xffeeeeee), 0.55f, 0.22f);
    }

    // Piano: white keys, then black keys on top.
    auto drawKey = [&] (int pc) {
        const bool black = isBlack (pc);
        const auto r = keyRect (pc).reduced (0.68f, 0.0f);
        const bool on = metatune::isAllowed (mask, pc);
        const float hover = keyHover[size_t (pc)]->get();
        auto top = black ? blackOff : keyOffTop;
        auto bottom = black ? blackOff.darker (0.16f) : keyOffBottom;
        if (on)
        {
            top = tint (black ? blackOnTop : keyOnTop, keyHue);
            bottom = tint (black ? blackOnBottom : keyOnBottom, keyHue);
            if (! black && ! gradientOn) top = top.withMultipliedSaturation (1.055f);
        }
        else if (! black && ! lightMode)
        {
            // The far right edge of the keyboard catches more light from the panel.
            const float light = 0.11f * (float (pc) / 11.0f) * (float (pc) / 11.0f);
            top = top.interpolatedWith (juce::Colours::white, light);
            bottom = juce::Colour (0xff646464).interpolatedWith (juce::Colours::white, light * 0.48f);
        }

        juce::Path p;
        if (black)
        {
            p = blackKeyShape (r);
            juce::Graphics::ScopedSaveState clip (g);
            g.reduceClipRegion (piano().getSmallestIntegerContainer());
            const auto centre = juce::Point<float> (r.getRight() + 1.5f, r.getY() + r.getHeight() * 0.31f);
            const float radius = 17.0f;
            g.addTransform (juce::AffineTransform::scale (1.0f, 4.25f, centre.x, centre.y));
            g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (lightMode ? 0.16f : 0.31f), centre,
                                                     juce::Colours::transparentBlack, centre.translated (radius, 0.0f), true));
            g.fillEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre));
        }
        else if (pc == 0 || pc == 11)
            p.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                   pc == 0 ? 23.0f : 6.0f, pc == 0 ? 16.0f : 6.0f,
                                   false, false, pc == 0, pc == 11);
        else
            p.addRectangle (r);

        juce::ColourGradient surface (top.brighter (0.12f * hover), r.getTopLeft(),
                                       bottom.brighter (0.12f * hover), r.getBottomLeft(), false);
        if (on)
        {
            const auto bodyLight = tint (recolour (juce::Colour (black ? 0xff57458d : 0xffa080fa)), keyHue);
            surface.addColour (black ? 0.34 : 0.45, gradientOn ? top.interpolatedWith (bottom, 0.25f) : bodyLight);
        }
        g.setGradientFill (surface);
        g.fillPath (p);

        if (black)
        {
            // The key narrows into the label above, and has a single rounded rim below.
            g.setColour (juce::Colours::black.withAlpha (0.90f));
            g.strokePath (p, juce::PathStrokeType (2.15f));
        }
        {
            juce::Graphics::ScopedSaveState clip (g);
            g.reduceClipRegion (p);
            if (on)
            {
                auto seam = r.withHeight (black ? 4.6f : 5.2f).reduced (black ? 2.8f : 0.0f, 0.0f);
                const auto edge = tint (recolour (juce::Colour (0xfff6edff)), keyHue);
                juce::ColourGradient glow (edge.withAlpha (black ? 1.0f : 0.74f), seam.getTopLeft(),
                                           edge.withAlpha (0.0f), seam.getBottomLeft(), false);
                const auto violetEdge = tint (recolour (juce::Colour (0xffa67cfc)), keyHue);
                glow.addColour (0.36, black ? violetEdge.withAlpha (0.66f) : edge.withAlpha (0.24f));
                g.setGradientFill (glow);
                g.fillRect (seam);
            }
            else if (! black)
            {
                g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.10f), r.getTopLeft(),
                                                         juce::Colours::transparentWhite, r.getTopLeft().translated (0.0f, 2.2f), false));
                g.fillRect (r.withHeight (2.2f));
            }

            if (! black)
            {
                g.setColour (juce::Colours::white.withAlpha (lightMode ? 0.06f : 0.22f));
                g.fillRect (r.withTop (r.getBottom() - 0.85f));
            }
        }
        if (showDetected && pc == targetPc)
        {
            g.setColour (lavender.withAlpha (0.35f * state.presence));
            g.strokePath (p, juce::PathStrokeType (1.3f));
        }
    };
    for (int pc = 0; pc < 12; ++pc) if (! isBlack (pc)) drawKey (pc);
    for (int pc = 0; pc < 12; ++pc) if (isBlack (pc)) drawKey (pc);

    if (mask == 0)
    {
        const auto p = piano();
        g.setColour (bg0.withAlpha (0.75f));
        g.fillRect (p);
        g.setFont (font (15.0f, true));
        shadowText (g, "No notes enabled - correction paused. Click a key to enable it.", p, juce::Justification::centred, warn);
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
