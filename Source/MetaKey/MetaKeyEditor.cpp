#include "MetaKeyEditor.h"

using namespace theme;

static const char* kNoteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static int scaleMaskFor (int root, bool minor)
{
    static const int majI[7] = { 0, 2, 4, 5, 7, 9, 11 };
    static const int minI[7] = { 0, 2, 3, 5, 7, 8, 10 };
    int m = 0;
    for (int i = 0; i < 7; ++i) m |= 1 << ((root + (minor ? minI[i] : majI[i])) % 12);
    return m;
}

static juce::String keyName (int root, bool minor)
{
    if (root < 0) return "--";
    return juce::String (kNoteNames[root % 12]) + (minor ? " Minor" : " Major");
}

MetaKeyEditor::MetaKeyEditor (MetaKeyProcessor& p) : juce::AudioProcessorEditor (p), proc (p)
{
    setSize (kW, kH);
    startTimerHz (20);
}

MetaKeyEditor::~MetaKeyEditor() { stopTimer(); }

juce::Rectangle<float> MetaKeyEditor::methodTile (int i) const
{
    const float x0 = 20.0f, gap = 16.0f, w = (float (kW) - 40.0f - 2.0f * gap) / 3.0f;
    return { x0 + float (i) * (w + gap), 88.0f, w, 92.0f };
}

juce::Rectangle<float> MetaKeyEditor::minorToggle() const { return { float (kW) / 2.0f - 70.0f, 256.0f, 140.0f, 26.0f }; }
juce::Rectangle<float> MetaKeyEditor::restartBtn() const { return { float (kW) - 112.0f, 202.0f, 92.0f, 24.0f }; }

juce::Rectangle<float> MetaKeyEditor::keyRect (int pc) const
{
    const float x0 = 20.0f, y0 = 306.0f, w = float (kW) - 40.0f, h = 120.0f;
    const float ww = w / 7.0f;
    static const int whiteIdx[12] = { 0, -1, 1, -1, 2, 3, -1, 4, -1, 5, -1, 6 };
    static const int isBlack[12]  = { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
    static const int afterWhite[12] = { -1, 0, -1, 1, -1, -1, 3, -1, 4, -1, 5, -1 };
    if (! isBlack[pc])
        return { x0 + float (whiteIdx[pc]) * ww, y0, ww, h };
    const float bw = ww * 0.62f;
    return { x0 + float (afterWhite[pc] + 1) * ww - bw * 0.5f, y0, bw, h * 0.62f };
}

static void drawMethodIcon (juce::Graphics& g, int i, juce::Rectangle<float> a, juce::Colour c)
{
    g.setColour (c);
    const auto ctr = a.getCentre();
    if (i == 0) // Listen: an ear-like arc + lobe
    {
        juce::Path p;
        p.addArc (ctr.x - 13.0f, ctr.y - 15.0f, 26.0f, 26.0f, juce::MathConstants<float>::pi * 1.15f, juce::MathConstants<float>::pi * 2.7f, true);
        g.strokePath (p, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.drawEllipse (ctr.x - 4.0f, ctr.y + 2.0f, 8.0f, 8.0f, 2.2f);
    }
    else if (i == 1) // Use File: a document with a waveform
    {
        const auto r = juce::Rectangle<float> (24.0f, 30.0f).withCentre (ctr);
        g.drawRoundedRectangle (r, 3.0f, 2.0f);
        for (int k = 0; k < 5; ++k)
        {
            const float bx = r.getX() + 5.0f + float (k) * 3.5f;
            const float bh = 4.0f + 8.0f * std::abs (std::sin (float (k) * 1.3f));
            g.fillRect (bx, ctr.y - bh * 0.5f, 2.0f, bh);
        }
    }
    else // Manual: small piano keys
    {
        const auto r = juce::Rectangle<float> (30.0f, 24.0f).withCentre (ctr);
        g.drawRoundedRectangle (r, 2.0f, 2.0f);
        for (int k = 1; k < 4; ++k) g.drawLine (r.getX() + float (k) * r.getWidth() / 4.0f, r.getY(), r.getX() + float (k) * r.getWidth() / 4.0f, r.getBottom(), 1.4f);
        for (int k : { 0, 2 }) g.fillRect (r.getX() + (float (k) + 0.72f) * r.getWidth() / 4.0f, r.getY(), r.getWidth() / 7.0f, r.getHeight() * 0.6f);
    }
}

void MetaKeyEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg1);
    const auto mouse = getMouseXYRelative().toFloat();
    const auto res = proc.result();
    const auto mode = proc.mode();

    // Header
    g.setColour (header);
    g.fillRect (0, 0, kW, 56);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillRect (0, 56, kW, 2);
    {
        juce::AttributedString word;
        word.append ("Meta", font (24.0f), text.withAlpha (0.9f));
        word.append ("Key", font (24.0f, true), violet.brighter (0.15f));
        word.setJustification (juce::Justification::centredLeft);
        word.draw (g, juce::Rectangle<float> (22.0f, 0.0f, 240.0f, 56.0f));
    }
    g.setFont (font (12.0f, true));
    shadowText (g, "KEY DETECTION", juce::Rectangle<float> (float (kW) - 182.0f, 0.0f, 160.0f, 56.0f), juce::Justification::centredRight, textFaint.brighter (0.2f));

    g.setFont (font (12.0f, true));
    shadowText (g, "SELECT DETECTION METHOD", juce::Rectangle<float> (0.0f, 64.0f, float (kW), 18.0f), juce::Justification::centred, textDim);

    // Method tiles
    const char* labels[3] = { "Listen", "Use File", "Manual" };
    for (int i = 0; i < 3; ++i)
    {
        const auto t = methodTile (i);
        const bool active = (int) mode == i;
        const bool hot = t.contains (mouse);
        g.setColour (active ? violet.withAlpha (0.16f) : bg2);
        g.fillRoundedRectangle (t, 8.0f);
        g.setColour (active ? violet.brighter (0.1f) : juce::Colours::white.withAlpha (hot ? 0.22f : 0.08f));
        g.drawRoundedRectangle (t.reduced (0.5f), 8.0f, active ? 1.8f : 1.0f);
        const juce::Colour ic = active ? violet.brighter (0.2f) : textDim.interpolatedWith (text, hot ? 1.0f : 0.3f);
        drawMethodIcon (g, i, t.withTrimmedBottom (26.0f), ic);
        g.setFont (font (14.0f, active));
        shadowText (g, labels[i], t.withTop (t.getBottom() - 28.0f), juce::Justification::centred, active ? text : textDim.interpolatedWith (text, hot ? 1.0f : 0.5f));
    }

    // Status box
    const auto box = juce::Rectangle<float> (20.0f, 192.0f, float (kW) - 40.0f, 98.0f);
    g.setColour (bg0);
    g.fillRoundedRectangle (box, 8.0f);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawRoundedRectangle (box.reduced (0.5f), 8.0f, 1.0f);

    juce::String status;
    bool showKey = res.valid();
    if (mode == MetaKeyProcessor::Mode::Manual) status = "Manual key";
    else if (mode == MetaKeyProcessor::Mode::File)
        status = proc.fileBusy() ? "Analysing " + proc.fileName() + "..."
               : (res.valid() ? proc.fileName() : "Load an audio file to detect its key");
    else // Listen
        status = ! proc.hasSignal() ? "Waiting for audio - press play"
               : (res.valid() ? "Listening" : "Listening...");

    g.setFont (font (12.5f, true));
    shadowText (g, status, box.withHeight (22.0f).translated (0.0f, 8.0f), juce::Justification::centred, textDim);

    g.setFont (font (34.0f, true));
    shadowText (g, showKey ? keyName (res.root, res.minor) : juce::String ("--"),
                box.withTrimmedTop (24.0f).withTrimmedBottom (20.0f), juce::Justification::centred,
                showKey ? text : textFaint);

    // Confidence bar (not for manual)
    if (mode != MetaKeyProcessor::Mode::Manual)
    {
        const auto bar = juce::Rectangle<float> (box.getCentreX() - 110.0f, box.getBottom() - 16.0f, 220.0f, 5.0f);
        g.setColour (well);
        g.fillRoundedRectangle (bar, 2.5f);
        if (showKey)
        {
            g.setColour (violet.brighter (0.1f).withAlpha (0.9f));
            g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * juce::jlimit (0.05f, 1.0f, res.confidence)), 2.5f);
        }
    }

    // Restart (Listen) / Major-Minor toggle (Manual)
    if (mode == MetaKeyProcessor::Mode::Listen)
    {
        const auto rb = restartBtn();
        const bool hot = rb.contains (mouse);
        g.setColour (bg2.brighter (hot ? 0.3f : 0.1f));
        g.fillRoundedRectangle (rb, 12.0f);
        g.setColour (juce::Colours::white.withAlpha (hot ? 0.3f : 0.12f));
        g.drawRoundedRectangle (rb.reduced (0.5f), 12.0f, 1.0f);
        g.setFont (font (12.0f, true));
        shadowText (g, "Restart", rb, juce::Justification::centred, textDim.interpolatedWith (text, hot ? 1.0f : 0.6f));
    }
    else if (mode == MetaKeyProcessor::Mode::Manual)
    {
        const auto mt = minorToggle();
        const bool minor = res.minor;
        for (int i = 0; i < 2; ++i)
        {
            const auto half = juce::Rectangle<float> (mt.getX() + float (i) * mt.getWidth() / 2.0f, mt.getY(), mt.getWidth() / 2.0f, mt.getHeight());
            const bool on = (i == 1) == minor;
            g.setColour (on ? violet.withAlpha (0.9f) : bg2);
            g.fillRoundedRectangle (half.reduced (1.0f), 5.0f);
            g.setFont (font (12.5f, true));
            shadowText (g, i == 0 ? "Major" : "Minor", half, juce::Justification::centred, on ? juce::Colours::white : textDim);
        }
    }

    // Keyboard: highlight the detected key's scale, emphasise the tonic.
    const int mask = showKey ? scaleMaskFor (res.root, res.minor) : 0;
    for (int pc = 0; pc < 12; ++pc) // white keys first
    {
        static const int isBlack[12] = { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
        if (isBlack[pc]) continue;
        const auto r = keyRect (pc).reduced (1.0f, 0.0f);
        const bool inKey = (mask >> pc) & 1, tonic = showKey && pc == res.root;
        g.setColour (tonic ? violet.brighter (0.2f) : (inKey ? violet.withAlpha (0.45f) : juce::Colour (0xffe9e9ee)));
        g.fillRoundedRectangle (r.withTrimmedTop (0.0f), 3.0f);
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawRoundedRectangle (r, 3.0f, 1.0f);
        g.setFont (font (11.5f, tonic));
        g.setColour (tonic || inKey ? juce::Colours::white : juce::Colour (0xff6a6a72));
        g.drawText (kNoteNames[pc], r.withTrimmedBottom (6.0f), juce::Justification::centredBottom);
    }
    for (int pc = 0; pc < 12; ++pc) // black keys on top
    {
        static const int isBlack[12] = { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
        if (! isBlack[pc]) continue;
        const auto r = keyRect (pc);
        const bool inKey = (mask >> pc) & 1, tonic = showKey && pc == res.root;
        juce::DropShadow (juce::Colours::black.withAlpha (0.5f), 6, { 0, 2 }).drawForRectangle (g, r.getSmallestIntegerContainer());
        g.setColour (tonic ? violet.brighter (0.2f) : (inKey ? violet.withAlpha (0.8f) : juce::Colour (0xff17171b)));
        g.fillRoundedRectangle (r, 2.5f);
    }

    // Send-to-MetaTune placeholder (the link arrives in a later build).
    const auto send = juce::Rectangle<float> (float (kW) / 2.0f - 120.0f, 442.0f, 240.0f, 34.0f);
    g.setColour (bg2);
    g.fillRoundedRectangle (send, 17.0f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (send.reduced (0.5f), 17.0f, 1.0f);
    g.setFont (font (13.0f, true));
    shadowText (g, "Send to MetaTune  (soon)", send, juce::Justification::centred, textFaint.brighter (0.2f));
}

void MetaKeyEditor::mouseDown (const juce::MouseEvent& e)
{
    const auto p = e.position;
    for (int i = 0; i < 3; ++i)
    {
        if (methodTile (i).contains (p))
        {
            if (i == 1) { proc.setMode (MetaKeyProcessor::Mode::File); openFileChooser(); }
            else proc.setMode (i == 0 ? MetaKeyProcessor::Mode::Listen : MetaKeyProcessor::Mode::Manual);
            repaint();
            return;
        }
    }
    if (proc.mode() == MetaKeyProcessor::Mode::Listen && restartBtn().contains (p)) { proc.restartListen(); return; }
    if (proc.mode() == MetaKeyProcessor::Mode::Manual)
    {
        const auto mt = minorToggle();
        if (mt.contains (p)) { proc.setManualKey (proc.result().root < 0 ? 0 : proc.result().root, p.x > mt.getCentreX()); repaint(); return; }
        for (int pc = 0; pc < 12; ++pc) // black keys take priority
        {
            static const int isBlack[12] = { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
            if (isBlack[pc] && keyRect (pc).contains (p)) { proc.setManualKey (pc, proc.result().minor); repaint(); return; }
        }
        for (int pc = 0; pc < 12; ++pc)
        {
            static const int isBlack[12] = { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
            if (! isBlack[pc] && keyRect (pc).contains (p)) { proc.setManualKey (pc, proc.result().minor); repaint(); return; }
        }
    }
}

void MetaKeyEditor::openFileChooser()
{
    chooser = std::make_unique<juce::FileChooser> ("Pick an audio file to detect its key",
                                                   juce::File(), "*.wav;*.aiff;*.aif;*.flac;*.ogg;*.mp3");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this] (const juce::FileChooser& fc)
                          {
                              const auto f = fc.getResult();
                              if (f.existsAsFile()) proc.analyseFile (f);
                          });
}
