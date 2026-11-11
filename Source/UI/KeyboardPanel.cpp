#include "KeyboardPanel.h"

static const char* kSharpNames[] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
static const char* kFlatNames[] { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
// Measured seams along the label row, shared with both heatmaps.
static constexpr int pitchEdges[] { 0, 88, 174, 263, 349, 435, 522, 609, 695, 780, 871, 957, 1043 };
static bool isBlack (int pc) { return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10; }
static int whiteIndex (int pc) { static const int w[] { 0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6 }; return w[pc]; }

static juce::Colour auditionColour (int mode)
{
    if (mode == 1) return theme::majorAudition;
    if (mode == 2) return theme::minorAudition;
    return theme::tint (theme::lavender, theme::keyHue);
}

static void drawSpeaker (juce::Graphics& g, juce::Point<float> centre)
{
    juce::Path speaker;
    speaker.startNewSubPath (-7.0f, -2.3f);
    speaker.lineTo (-4.0f, -2.3f);
    speaker.lineTo (0.0f, -5.3f);
    speaker.lineTo (0.0f, 5.3f);
    speaker.lineTo (-4.0f, 2.3f);
    speaker.lineTo (-7.0f, 2.3f);
    speaker.closeSubPath();
    const auto atCentre = juce::AffineTransform::translation (centre.x, centre.y);
    g.fillPath (speaker, atCentre);

    juce::Path waves;
    waves.startNewSubPath (2.7f, -2.7f);
    waves.quadraticTo (5.6f, 0.0f, 2.7f, 2.7f);
    waves.startNewSubPath (5.0f, -4.5f);
    waves.quadraticTo (9.5f, 0.0f, 5.0f, 4.5f);
    g.strokePath (waves, juce::PathStrokeType (1.25f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded), atCentre);
}

void KeyboardPanel::setPreviewMask (std::optional<metatune::PitchClassMask> next)
{
    if (previewMask == next) return;
    previewMask = next;
    repaint();
}

void KeyboardPanel::setBypassBlend (float blend)
{
    blend = juce::jlimit (0.0f, 1.0f, blend);
    if (blend == bypassBlend) return;
    bypassBlend = blend;
    repaint();
}

juce::Rectangle<float> KeyboardPanel::labelRow() const
{
    const float y = float (kHeatTop + 2 * kHeatH + 2 * kHeatGap);
    return { 0.0f, y, float (getWidth()), float (kLabelH) };
}

juce::Rectangle<float> KeyboardPanel::labelCell (int pc) const
{
    const auto row = labelRow();
    const float scale = row.getWidth() / float (pitchEdges[12]);
    return row.withLeft (float (pitchEdges[pc]) * scale).withRight (float (pitchEdges[pc + 1]) * scale);
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

void KeyboardPanel::HeatHistory::add (float position, float weight, float width)
{
    const int bin = int (position);
    for (int offset = -6; offset <= 6; ++offset)
    {
        const float d = (float (bin + offset) + 0.5f - position) / width;
        const float energy = weight * (0.78f * std::exp (-d * d) + 0.22f * std::exp (-0.23f * d * d));
        const size_t i = size_t ((bin + offset + kBins) % kBins);
        total[i] += energy;
        recent[i] = juce::jmax (recent[i], energy);
    }
    // Preserve relative pitch density while bounding the accumulation over a long session.
    if (total[size_t (bin % kBins)] > 1024.0f)
        for (auto& binWeight : total)
            binWeight *= 0.5f;
}

void KeyboardPanel::addHeat (float inputMidi, float outputMidi, float weight)
{
    const auto addPitch = [weight] (HeatHistory& history, float midi, float width)
    {
        const float pitch = std::fmod (midi + 0.5f, 12.0f);
        const int pc = int (pitch);
        const float x = std::lerp (float (pitchEdges[pc]), float (pitchEdges[pc + 1]), pitch - float (pc));
        history.add (x * float (kBins) / float (pitchEdges[12]), weight, width);
    };
    if (inputMidi > 0.0f && std::isfinite (inputMidi))
        addPitch (heatIn, inputMidi, 1.30f);
    if (outputMidi > 0.0f && std::isfinite (outputMidi))
    {
        addPitch (heatOut, outputMidi, 0.90f);
        auto& detected = detectedHistory[size_t (juce::roundToInt (outputMidi) % 12)];
        detected += weight;
        if (detected > 1024.0f)
            for (auto& count : detectedHistory)
                count *= 0.5f;
    }
}

void KeyboardPanel::decayHeat (float factor)
{
    // Retain the recorded pitch distribution; only the white live crests fade.
    for (auto* history : { &heatIn, &heatOut })
    {
        const float decay = history == &heatIn ? factor : factor * factor;
        for (auto& value : history->recent)
        {
            value *= decay;
            if (value < 0.002f) value = 0.0f;
        }
    }
    if (showHeat) repaint (0, 0, getWidth(), kHeatTop + 2 * kHeatH + kHeatGap);
}

void KeyboardPanel::resetHeat()
{
    heatIn = {};
    heatOut = {};
    repaint (0, 0, getWidth(), kHeatTop + 2 * kHeatH + kHeatGap);
}

void KeyboardPanel::resetDetected()
{
    detectedHistory.fill (0.0f);
    repaint (labelRow().getSmallestIntegerContainer());
}

void KeyboardPanel::drawHeat (juce::Graphics& g, juce::Rectangle<int> bounds,
                              const HeatHistory& history, juce::Image& strip) const
{
    using namespace theme;
    const auto ground = lightMode ? bg0 : dark::heatTrack;
    const auto heatViolet = tint (recolour (juce::Colour (0xff5230d8)), meterHue);
    const auto lavenderCore = tint (recolour (juce::Colour (0xffb6acff)), meterHue);
    const auto whiteHot = juce::Colour (0xfffbf9ff);
    const float peak = *std::max_element (history.total.begin(), history.total.end());
    const float normalise = 1.0f / juce::jmax (0.70f, peak);
    if (peak <= 0.0f)
    {
        g.setColour (ground);
        g.fillRect (bounds);
        return;
    }

    // Draw a sub-bin gradient at each fractional pitch. Historical density stays violet;
    // new strong pitches add narrow white crests without changing the surrounding floor.
    {
        juce::Image::BitmapData pixels (strip, juce::Image::BitmapData::writeOnly);
        for (int i = 0; i < kBins; ++i)
        {
            for (int sample = 0; sample < samplesPerBin; ++sample)
            {
                const float offset = float (sample - 1) / float (samplesPerBin);
                const int next = (i + (offset < 0.0f ? -1 : 1) + kBins) % kBins;
                const float mix = std::abs (offset);
                const float total = std::lerp (history.total[size_t (i)], history.total[size_t (next)], mix);
                const float strength = std::sqrt (total * normalise);
                auto colour = strength < 0.61f ? ground.interpolatedWith (heatViolet, strength / 0.61f)
                                               : heatViolet.interpolatedWith (lavenderCore, (strength - 0.61f) / 0.39f);
                const float recent = std::lerp (history.recent[size_t (i)], history.recent[size_t (next)], mix);
                const float live = juce::jlimit (0.0f, 1.0f, (recent - 0.04f) / 0.26f);
                colour = colour.interpolatedWith (whiteHot, live * live * live * 0.94f);
                pixels.setPixelColour (i * samplesPerBin + sample, 0, colour);
            }
        }
    }
    g.drawImage (strip, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                 0, 0, strip.getWidth(), 1);
}

void KeyboardPanel::drawKeyOutline (juce::Graphics& g, const juce::Path& shape, int pc, juce::Colour colour) const
{
    juce::Graphics::ScopedSaveState clip (g);
    g.reduceClipRegion (shape);
    g.setColour (colour.withMultipliedAlpha (0.24f));
    g.strokePath (shape, juce::PathStrokeType (6.0f));
    g.setColour (colour);
    g.strokePath (shape, juce::PathStrokeType (2.7f));

    if (isBlack (pc)) return;

    // Paint the visible notch in a white key where a raised black key overlaps it.
    for (const int adjacent : { pc - 1, pc + 1 })
    {
        if (adjacent < 0 || adjacent >= 12 || ! isBlack (adjacent)) continue;
        const auto edge = blackKeyShape (keyRect (adjacent).reduced (0.68f, 0.0f));
        g.setColour (colour.withMultipliedAlpha (0.24f));
        g.strokePath (edge, juce::PathStrokeType (9.0f));
        g.setColour (colour);
        g.strokePath (edge, juce::PathStrokeType (5.0f));
    }
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

    const int outputPc = state.voiced && state.outputMidi > 0.0f ? juce::roundToInt (state.outputMidi) % 12 : -1;
    const float detectionPeak = juce::jmax (1.0f, *std::max_element (detectedHistory.begin(), detectedHistory.end()));
    const auto noteHighlight = tint (lavender, keyHue).interpolatedWith (bypassAccent, bypassBlend);
    const float playing = auditionGlow.get();
    const auto playingColour = auditionColour (pressedMode);

    // Label row: the top of each key, grey over white keys and black over black keys.
    for (int pc = 0; pc < 12; ++pc)
    {
        const auto r = labelCell (pc).reduced (0.65f, 0.0f);
        const auto base = isBlack (pc) ? labelBlack : labelWhite;
        const auto top = base.brighter (isBlack (pc) ? 0.025f : 0.065f);
        g.setGradientFill (juce::ColourGradient (top, r.getTopLeft(), base, r.getBottomLeft(), false));
        g.fillRect (r);
        if (hoverLabel && hoverPc == pc)
        {
            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.fillRect (r);
        }
        if (isBlack (pc) && metatune::isAllowed (mask, pc))
        {
            g.setColour (noteHighlight.withAlpha (0.45f));
            g.fillRect (r.withTop (r.getBottom() - 1.15f).reduced (2.6f, 0.0f));
        }
        const float history = detectedHistory[size_t (pc)];
        if (showDetected && history > 0.04f)
        {
            const float sustained = 0.73f * std::sqrt (history / detectionPeak) * juce::jmin (1.0f, history);
            const float live = pc == outputPc ? 0.94f * state.presence : 0.0f;
            g.setColour (juce::Colours::white.withAlpha (juce::jmax (sustained, live)));
            g.fillRect (r.withTop (r.getBottom() - 1.25f).reduced (1.2f, 0.0f));
        }
        const float feedback = metatune::isAllowed (auditionMask, pc) ? playing : 0.0f;
        const auto nameColour = lightMode ? text : juce::Colour (0xffeeeeee);
        if (feedback < 0.995f)
        {
            g.setFont (font (17.2f).withExtraKerningFactor (0.08f));
            shadowText (g, (useFlats ? kFlatNames : kSharpNames)[pc], r, juce::Justification::centred,
                        nameColour.withMultipliedAlpha (1.0f - feedback), 0.55f, 0.22f);
        }
        if (feedback > 0.0f)
        {
            const auto box = r.reduced (0.7f, 1.0f);
            g.setColour (playingColour.withAlpha (0.20f * feedback));
            g.fillRect (box);
            g.setColour (playingColour.withAlpha (0.22f * feedback));
            g.drawRect (box.reduced (1.5f), 3.0f);
            g.setColour (playingColour.withAlpha (0.95f * feedback));
            g.drawRect (box, 1.65f);
            g.setColour (playingColour.interpolatedWith (juce::Colours::white, 0.51f).withAlpha (feedback));
            drawSpeaker (g, box.getCentre());
        }
    }

    // Piano: white keys, then black keys on top.
    auto drawKey = [&] (int pc) {
        const bool black = isBlack (pc);
        const auto r = keyRect (pc).reduced (0.68f, 0.0f);
        const bool on = metatune::isAllowed (mask, pc);
        const float hover = keyHover[size_t (pc)]->get();
        const bool previewed = previewMask.has_value() && metatune::isAllowed (*previewMask, pc);
        auto top = black ? blackOff : keyOffTop;
        auto bottom = black ? blackOff.darker (0.16f) : keyOffBottom;
        juce::Colour middle;
        if (on)
        {
            top = tint (black ? blackOnTop : keyOnTop, keyHue);
            bottom = tint (black ? blackOnBottom : keyOnBottom, keyHue);
            if (! black && ! gradientOn) top = top.withMultipliedSaturation (1.055f);
            middle = gradientOn ? top.interpolatedWith (bottom, 0.25f)
                                : tint (recolour (juce::Colour (black ? 0xff57458d : 0xffa080fa)), keyHue);
            top = top.interpolatedWith (juce::Colour (black ? 0xff843e41 : 0xffed7074), bypassBlend);
            middle = middle.interpolatedWith (juce::Colour (black ? 0xff853f42 : 0xfff07476), bypassBlend);
            bottom = bottom.interpolatedWith (juce::Colour (black ? 0xff604344 : 0xffaa7678), bypassBlend);
        }
        else
        {
            if (! black && ! lightMode)
            {
                // The far right edge of the keyboard catches more light from the panel.
                const float light = 0.11f * (float (pc) / 11.0f) * (float (pc) / 11.0f);
                top = top.interpolatedWith (juce::Colours::white, light);
                bottom = juce::Colour (0xff646464).interpolatedWith (juce::Colours::white, light * 0.48f);
            }
            if (previewed)
            {
                top = top.interpolatedWith (noteHighlight, 0.18f);
                bottom = bottom.interpolatedWith (noteHighlight, 0.12f);
            }
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
            surface.addColour (black ? 0.34 : 0.45, middle.brighter (0.12f * hover));
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
                const auto edge = tint (recolour (juce::Colour (0xfff6edff)), keyHue)
                                      .interpolatedWith (juce::Colour (0xffffd6d6), bypassBlend);
                juce::ColourGradient glow (edge.withAlpha (black ? 1.0f : 0.74f), seam.getTopLeft(),
                                           edge.withAlpha (0.0f), seam.getBottomLeft(), false);
                glow.addColour (0.36, black ? noteHighlight.withAlpha (0.66f) : edge.withAlpha (0.24f));
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
        const float outline = juce::jmax (previewed ? 0.81f : 0.0f, 0.96f * hover);
        if (outline > 0.0f)
            drawKeyOutline (g, p, pc, noteHighlight.interpolatedWith (juce::Colours::white, 0.47f).withAlpha (outline));
    };
    for (int pc = 0; pc < 12; ++pc) if (! isBlack (pc)) drawKey (pc);
    for (int pc = 0; pc < 12; ++pc) if (isBlack (pc)) drawKey (pc);
}

void KeyboardPanel::mouseDown (const juce::MouseEvent& e)
{
    const int pc = pitchClassAt (e.position);
    if (pc < 0) return;
    if (e.mods.isPopupMenu()) { if (onSetRoot) onSetRoot (pc); return; }
    if (! e.mods.isLeftButtonDown()) return;
    if (! labelRow().contains (e.position))
    {
        if (onToggleNote) onToggleNote (pc);
        return;
    }

    const auto note = [] (int pitch) { return metatune::PitchClassMask (1u << (pitch % 12)); };
    pressedMode = auditionMode;
    auditionMask = note (pc);
    if (pressedMode != 0)
        auditionMask |= note (pc + (pressedMode == 1 ? 4 : 3)) | note (pc + 7);
    auditionPressed = true;
    auditionGlow.snapTo (1.0f);
    repaint();
    if (onPress) onPress (pc);
}

void KeyboardPanel::mouseUp (const juce::MouseEvent&)
{
    if (! auditionPressed) return;
    auditionPressed = false;
    auditionGlow.setTarget (0.0f);
    if (onRelease) onRelease();
}

void KeyboardPanel::setHoveredKey (int pc, bool label)
{
    if (pc == hoverPc && label == hoverLabel) return;
    if (hoverPc >= 0 && ! hoverLabel) keyHover[size_t (hoverPc)]->setTarget (0.0f);
    hoverPc = pc;
    hoverLabel = label;
    if (hoverPc >= 0 && ! hoverLabel) keyHover[size_t (hoverPc)]->setTarget (1.0f);
    repaint();
}

void KeyboardPanel::mouseMove (const juce::MouseEvent& e)
{
    setHoveredKey (pitchClassAt (e.position), labelRow().contains (e.position));
}

void KeyboardPanel::mouseExit (const juce::MouseEvent&)
{
    setHoveredKey (-1, hoverLabel);
}
