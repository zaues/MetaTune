#include "PluginEditor.h"
#include "Parameters.h"

namespace layout
{
// Measured on the 1415 x 1056 reference, converted to the editor's logical units.
constexpr float unitsPerPixel = float (MetaTuneEditor::kW) / MetaTuneEditor::referenceW;
constexpr float referenceUnits (int px) { return float (px) * unitsPerPixel; }
constexpr int referencePixel (int px) { return int (referenceUnits (px) + 0.5f); }
constexpr int headerH = referencePixel (55);
// The recording places the stage divider about 63–64% down the window.
constexpr int stageBottom = 670, modeBottom = 715;
constexpr int rowY = referencePixel (stageBottom), bottomY = referencePixel (modeBottom);
constexpr int rowH = bottomY - rowY;
constexpr int leftW = referencePixel (182), rightX = referencePixel (1225);
constexpr int footerY = referencePixel (1033), chromaticY = referencePixel (988);
constexpr int keyboardLabelY = bottomY + KeyboardPanel::kHeatTop + 2 * (KeyboardPanel::kHeatH + KeyboardPanel::kHeatGap);
constexpr int pianoY = keyboardLabelY + KeyboardPanel::kLabelH;
constexpr int referenceY = referencePixel (930);
constexpr float stagePosition (int y)
{
    return referenceUnits (55) + referenceUnits (y - 55) * float (stageBottom - 55) / (602.0f - 55.0f);
}
juce::Point<float> stagePoint (int x, int y) { return { referenceUnits (x), stagePosition (y) }; }

const auto speedC = stagePoint (1075, 217);
const auto sustainC = stagePoint (1238, 255);
const auto amountC = stagePoint (1191, 442);
constexpr float speedR = referenceUnits (122), sustainR = referenceUnits (73), amountR = referenceUnits (110);
constexpr float speedBand = 0.78f;            // Speed knob: proportion where the aggressive (< 0 ms) band starts
juce::Rectangle<float> modeRect (int x, int y, int w, int h)
{
    constexpr float scale = float (modeBottom - stageBottom) / (655.0f - 602.0f);
    return { referenceUnits (x), referenceUnits (stageBottom) + referenceUnits (y - 602) * scale,
             referenceUnits (w), referenceUnits (h) * scale };
}

const auto noteBounds = modeRect (180, 610, 114, 34);
const auto majorBounds = modeRect (308, 610, 81, 34);
const auto majorHoldBounds = modeRect (389, 610, 73, 34);
const auto minorBounds = modeRect (475, 610, 81, 34);
const auto minorHoldBounds = modeRect (556, 610, 73, 34);
const auto timingBounds = modeRect (698, 605, 354, 43);
const auto stabilizerBounds = modeRect (1066, 610, 162, 34);
const auto latencyTrack = modeRect (1258, 616, 45, 21);
const auto latencyCaption = modeRect (1327, 609, 76, 39);

juce::Image makeConcretePanel()
{
    using namespace theme;
    // Low-frequency luminance sampled from exposed areas of the canonical panel.
    // The curved boundary and knobs were masked before sampling.
    static constexpr float luminance[9][11] {
        { 40, 48, 48, 47, 47, 48, 48, 51, 60, 68, 72 },
        { 43, 47, 47, 47, 48, 48, 49, 52, 60, 67, 70 },
        { 46, 46, 45, 46, 47, 47, 49, 52, 59, 66, 70 },
        { 44, 44, 44, 44, 45, 45, 45, 60, 60, 64, 68 },
        { 43, 43, 43, 44, 45, 46, 43, 47, 55, 60, 64 },
        { 44, 44, 44, 44, 44, 45, 46, 46, 54, 58, 64 },
        { 44, 44, 44, 44, 44, 45, 46, 46, 56, 59, 61 },
        { 45, 45, 45, 45, 45, 46, 46, 47, 54, 59, 61 },
        { 46, 46, 46, 46, 46, 46, 45, 46, 53, 56, 57 },
    };
    auto texture = juce::Image (juce::Image::RGB, MetaTuneEditor::kW, rowY, true);
    juce::Image::BitmapData pixels (texture, juce::Image::BitmapData::writeOnly);
    juce::Random grain (0x434f4e43);
    const auto origin = stagePoint (750, 60);
    const auto end = stagePoint (1407, 598);
    const auto shadow = [] (juce::Point<float> p, juce::Point<float> centre, float radius, float offset, float softness)
    {
        const auto delta = p - centre.translated (1.0f, offset);
        const float distance = std::sqrt (delta.x * delta.x + 1.12f * delta.y * delta.y);
        const float outside = juce::jmax (0.0f, distance - radius * 0.90f) / softness;
        return std::exp (-outside * outside);
    };
    for (int y = headerH; y < rowY; ++y)
        for (int x = referencePixel (735); x < MetaTuneEditor::kW; ++x)
        {
            const juce::Point<float> point { float (x), float (y) };
            const float u = juce::jlimit (0.0f, 10.0f, 10.0f * (point.x - origin.x) / (end.x - origin.x));
            const float v = juce::jlimit (0.0f, 8.0f, 8.0f * (point.y - origin.y) / (end.y - origin.y));
            const int col = juce::jmin (9, int (u)), row = juce::jmin (7, int (v));
            const float dx = u - float (col), dy = v - float (row);
            const float base = std::lerp (std::lerp (luminance[row][col], luminance[row][col + 1], dx),
                                          std::lerp (luminance[row + 1][col], luminance[row + 1][col + 1], dx), dy);
            const float cloud = surfaceNoise (point.x * 1.5f, point.y * 1.8f);
            const float grit = surfaceNoise (point.x * 6.4f + 19.0f, point.y * 5.7f + 53.0f);
            const float lower = juce::jlimit (0.0f, 1.0f, (v - 6.0f) * 0.5f);
            const float light = base + (9.0f - 4.0f * lower) * cloud + 2.8f * grit + 5.6f * (grain.nextFloat() - 0.5f);
            const float speedShadow = 17.0f * shadow (point, speedC, speedR, 5.0f, 10.0f);
            const float sustainShadow = 11.0f * shadow (point, sustainC, sustainR, 3.0f, 7.0f);
            const float amountShadow = 22.0f * shadow (point, amountC, amountR, 9.0f, 10.5f);
            const float level = light - juce::jmax (speedShadow, sustainShadow, amountShadow);
            const auto channel = [] (float n) { return static_cast<juce::uint8> (juce::jlimit (0, 255, juce::roundToInt (n))); };
            pixels.setPixelColour (x, y, lightMode ? panel.interpolatedWith (juce::Colours::black, juce::jlimit (0.0f, 0.19f, (70.0f - level) * 0.003f))
                                                  : juce::Colour (channel (level), channel (level), channel (level + 1.0f)));
        }
    return texture;
}

juce::Path panelEdge()
{
    juce::Path edge;
    edge.startNewSubPath (stagePoint (741, 55));
    edge.cubicTo (stagePoint (779, 55), stagePoint (789, 66), stagePoint (807, 100));
    edge.lineTo (stagePoint (1026, 500));
    edge.cubicTo (stagePoint (1061, 564), stagePoint (1104, 596), stagePoint (1154, 602));
    return edge;
}

const juce::Path& wordmark()
{
    static const auto outline = [] {
        auto path = juce::Drawable::parseSVGPath (
        "M 60.0 19.6 L 61.4 17.8 L 62.6 17.2 L 69.2 17.2 L 70.0 18.2 L 70.0 21.2 L 68.8 22.6 L 67.2 23.4 L 63.6 23.6 L 61.6 23.0 L 60.4 21.8 Z "
        "M 132.8 14.4 L 134.4 12.6 L 136.2 12.4 L 137.6 13.4 L 138.0 14.0 L 138.0 15.0 L 137.2 15.6 L 133.4 15.6 Z "
        "M 31.4 15.2 L 33.0 12.6 L 34.6 11.4 L 39.0 11.2 L 40.2 11.8 L 41.4 13.0 L 42.2 14.8 L 42.2 15.8 L 41.2 16.6 L 32.4 16.6 L 31.4 15.8 Z "
        "M 59.8 10.8 L 59.4 11.2 L 59.4 12.2 L 60.2 12.8 L 62.6 11.2 L 67.4 11.2 L 69.4 12.6 L 70.2 14.0 L 70.2 15.0 L 69.6 15.8 L 63.4 15.6 L 60.6 16.4 L 59.0 18.0 L 58.4 19.4 L 58.4 21.2 L 59.2 23.0 L 61.6 24.8 L 63.8 25.2 L 67.2 25.0 L 69.6 23.6 L 71.6 24.0 L 72.0 23.4 L 71.8 13.6 L 70.8 11.8 L 68.6 10.0 L 67.4 9.6 L 63.2 9.6 Z "
        "M 34.4 9.8 L 32.6 10.8 L 31.2 12.2 L 29.8 14.8 L 29.8 20.2 L 30.8 22.2 L 32.6 24.0 L 34.2 24.8 L 40.0 24.8 L 42.0 23.8 L 43.2 22.6 L 43.0 21.8 L 42.2 21.6 L 39.4 23.4 L 35.0 23.4 L 33.6 22.8 L 32.2 21.4 L 31.4 19.8 L 31.4 18.4 L 32.0 18.0 L 43.2 17.8 L 44.0 16.6 L 43.6 13.6 L 43.0 12.4 L 41.2 10.6 L 39.8 9.8 Z "
        "M 4.8 12.2 L 4.4 16.0 L 4.4 23.6 L 4.8 24.6 L 5.6 24.8 L 6.2 24.2 L 6.2 13.8 L 7.8 11.8 L 9.4 11.0 L 11.2 11.0 L 12.4 11.4 L 14.0 12.8 L 14.6 13.8 L 14.6 24.2 L 15.0 24.8 L 15.8 24.8 L 16.2 24.4 L 16.2 14.8 L 16.6 13.4 L 18.4 11.6 L 19.6 11.2 L 22.0 11.2 L 23.2 11.8 L 24.8 14.2 L 24.8 24.4 L 25.2 24.8 L 26.0 24.8 L 26.4 24.4 L 26.4 14.4 L 25.2 11.6 L 23.4 10.0 L 22.4 9.6 L 19.4 9.6 L 17.4 10.4 L 15.8 11.8 L 15.2 11.8 L 12.6 9.8 L 9.0 9.6 L 7.8 10.0 Z "
        "M 93.6 9.4 L 93.2 10.2 L 93.4 21.4 L 94.8 24.2 L 95.8 25.0 L 97.6 25.6 L 100.0 25.6 L 101.6 25.0 L 102.6 24.2 L 104.2 25.2 L 107.4 25.0 L 107.8 24.4 L 107.6 9.8 L 106.8 9.2 L 103.8 9.2 L 103.2 9.6 L 103.2 20.4 L 102.0 21.6 L 99.8 22.0 L 99.0 21.6 L 98.0 20.2 L 98.0 9.8 L 97.4 9.2 Z "
        "M 133.6 8.8 L 131.0 10.0 L 129.4 11.6 L 128.2 13.8 L 127.8 15.6 L 128.0 20.2 L 129.2 22.6 L 130.6 24.0 L 132.4 25.0 L 134.4 25.4 L 138.4 25.2 L 141.4 23.8 L 142.2 22.8 L 141.8 21.8 L 140.6 20.6 L 139.8 20.6 L 138.4 21.6 L 135.6 22.0 L 133.8 21.2 L 132.8 20.2 L 132.8 19.0 L 133.2 18.6 L 142.2 18.6 L 142.8 17.6 L 142.4 13.6 L 141.4 11.6 L 139.2 9.6 L 137.2 8.8 Z "
        "M 123.0 9.6 L 121.6 9.0 L 119.4 8.8 L 117.8 9.2 L 116.2 10.2 L 115.6 10.2 L 114.4 9.2 L 112.4 9.2 L 111.0 9.8 L 111.0 25.0 L 114.2 25.4 L 115.4 24.8 L 115.6 14.0 L 116.6 12.8 L 117.4 12.4 L 119.2 12.8 L 120.6 14.6 L 120.8 24.6 L 121.4 25.2 L 124.6 25.2 L 125.0 24.8 L 125.4 14.4 L 124.8 11.8 Z "
        "M 49.2 5.4 L 48.8 6.0 L 48.6 9.2 L 47.8 9.8 L 46.6 9.8 L 45.8 10.4 L 46.4 11.4 L 48.0 11.4 L 48.6 12.2 L 48.6 20.6 L 49.0 22.2 L 50.8 24.2 L 52.0 24.8 L 54.8 25.0 L 55.8 24.2 L 55.4 23.2 L 52.4 23.2 L 51.0 22.0 L 50.6 21.2 L 50.4 12.2 L 51.6 11.2 L 54.6 11.4 L 55.2 10.4 L 54.6 9.8 L 50.6 9.6 L 50.2 9.2 L 50.2 5.8 Z "
        "M 75.2 3.6 L 74.4 4.6 L 74.4 6.4 L 75.0 7.2 L 80.2 7.4 L 81.0 8.4 L 81.0 24.6 L 82.0 25.2 L 84.8 25.2 L 85.6 24.6 L 85.6 8.4 L 86.6 7.2 L 91.8 7.2 L 92.2 6.8 L 92.2 4.4 L 91.6 3.6 Z "
        );
        path.setUsingNonZeroWinding (false);
        path.applyTransform (juce::AffineTransform::scale (unitsPerPixel)
                                 .translated (referenceUnits (670), referenceUnits (13)));
        return path;
    }();
    return outline;
}

} // namespace layout

static juce::String formatSpeed (double v) { return std::abs (v) < 100.0 ? juce::String (v, 1) : juce::String (juce::roundToInt (v)); }

static juce::Rectangle<int> around (juce::Point<float> c, float r) { return juce::Rectangle<float> (2 * r, 2 * r).withCentre (c).toNearestInt(); }

MetaTuneEditor::MetaTuneEditor (MetaTuneProcessor& p)
    : AudioProcessorEditor (p), proc (p), state (p.apvts),
      speedKnob ("Speed", formatSpeed, { -210.0f, 60.0f, true, 3, layout::speedBand, "ms", 0.077f }),
      sustainKnob ("Sustain", [] (double v) { return juce::String (juce::roundToInt (v)); }, { -135.0f, 135.0f, false, 4, -1.0f, "%", 0.0f, true, true, true }),
      amountKnob ("Amount", [] (double v) { return v >= 99.95 ? juce::String ("100") : juce::String (v, 1); }, { -225.0f, 49.0f, true, 3, -1.0f, "%" }),
      orb (*state.getParameter (ParamID::doublerWidth), *state.getParameter (ParamID::doublerMix), "Doubler\nWidth", "Doubler\nDry / Wet", &p.undoManager),
      stabilizerSeg (*state.getParameter (ParamID::stabilizer), { "None", "Short", "Mid", "Long" }, &p.undoManager)
{
    theme::setTheme (update::Settings::lightMode());
    theme::setAccent (update::Settings::accentHue(), update::Settings::accentSat(), update::Settings::accentBright());
    theme::orbHue = update::Settings::orbHue();
    theme::keyHue = update::Settings::keyHue();
    theme::meterHue = update::Settings::meterHue();
    if (update::Settings::gradientOn())
        theme::setGradient (true, juce::Colour::fromHSV (update::Settings::gradHueA(), 0.85f, 1.0f, 1.0f),
                                  juce::Colour::fromHSV (update::Settings::gradHueB(), 0.85f, 1.0f, 1.0f));
    setLookAndFeel (&lnf);
    lnf.setColour (juce::TooltipWindow::backgroundColourId, theme::bg3);   // theme was applied above; refresh
    lnf.setColour (juce::TooltipWindow::textColourId, theme::text);
    lnf.setColour (juce::PopupMenu::backgroundColourId, theme::menuBg);
    lnf.setColour (juce::PopupMenu::textColourId, theme::text);
    addAndMakeVisible (canvas);

    for (auto* c : std::initializer_list<juce::Component*> { &orb, &speedKnob, &sustainKnob, &amountKnob, &keyboard, &refStrip, &toneSlider,
                                                            &stabilizerSeg, &groups, &presetBar, &notePill, &majorPill, &minorPill, &majorHoldPill,
                                                            &minorHoldPill, &lowLatency, &colorButton, &menuButton, &saveButton, &undoButton, &redoButton,
                                                            &aButton, &copyButton, &bButton, &helpButton, &heatButton,
                                                            &detectedButton, &heatResetButton, &detectedResetButton,
                                                            &bypassButton, &chromaticButton })
        canvas.addAndMakeVisible (c);
    canvas.addChildComponent (rootPicker);
    canvas.addChildComponent (presetPanel);
    canvas.addChildComponent (groupBrowser);
    canvas.addChildComponent (browserTabs);
    canvas.addChildComponent (colorPanel);
    browserTabs.onSelect = [this] (int t) { showBrowser (true, t); };
    canvas.addChildComponent (settingsPanel);
    settingsPanel.onPick = [this] (int id) {
        showSettingsPanel (false);
        if (id >= 60 && id <= 150) setScale (float (id) / 100.0f);
        if (id >= 500 && id < 503) { proc.undoManager.beginNewTransaction ("Range"); setParam (ParamID::range, float (id - 500)); }
        if (id == 600) update::Settings::setAutoCheck (! update::Settings::autoCheck());
        if (id == 601) startUpdateCheck();
        if (id == 602) installUpdate();
        if (id == 603) proc.ui.keepScale = ! proc.ui.keepScale;
        if (id == 604) showNotesCard();
        if (id == 605) showAutoKey();
        if (id == 606) proc.resetAllParameters();
        if (id == 607) pickerPinned = true;
    };
    settingsPanel.onMouseLeave = [this] {
        if (! settingsPanel.getLocalBounds().contains (settingsPanel.getMouseXYRelative()))
            showSettingsPanel (false);
    };
    canvas.addChildComponent (saveCard);
    saveCard.setBounds (0, 0, kW, kH);
    saveCard.onCancel = [this] { saveCard.setVisible (false); };
    canvas.addChildComponent (notesCard);
    notesCard.setBounds (0, 0, kW, kH);
    canvas.addChildComponent (autoKeyCard);
    autoKeyCard.setBounds (0, 0, kW, kH);
    autoKeyCard.onClose = [this] { autoKeyCard.setVisible (false); };
    autoKeyCard.onApply = [this] (int r, bool m) { autoKeyCard.setVisible (false); applyAutoKey (r, m); };
    autoKeyCard.onInstall = [this] {
        const auto inst = update::metaKeyInstaller();
        if (inst.existsAsFile()) inst.startAsProcess();
        else juce::URL ("https://github.com/zaues/MetaTune/releases/latest").launchInDefaultBrowser();
    };
    if (update::Settings::notesVersion() != JucePlugin_VersionString)
        showNotesPending = true; // a version change (install/update): show "What's New" once the UI is laid out
    saveCard.onSave = [this] (const juce::String& n) {
        const bool folder = pendingSave == PendingSave::Folder;
        if (n.trim().isEmpty()) { saveCard.open ({}, folder ? "Type a folder name" : "Type a name first"); return; }
        const bool ok = folder ? proc.createFolder (n) : proc.savePreset (n, pendingFolder);
        if (ok) saveCard.setVisible (false);
        else saveCard.open ({}, folder ? "Could not create the folder" : "Could not save the preset");
    };
    presetPanel.onLoad = [this] (int i) { proc.loadPreset (i); showBrowser (false); };
    presetPanel.onNewPreset = [this] { showBrowser (false); pendingSave = PendingSave::Preset; pendingFolder = {}; saveCard.open (proc.ui.presetName); };
    presetPanel.onSaveToFolder = [this] (const juce::String& f) { showBrowser (false); pendingSave = PendingSave::Preset; pendingFolder = f; saveCard.open (proc.ui.presetName); };
    presetPanel.onNewFolder = [this] { showBrowser (false); pendingSave = PendingSave::Folder; pendingFolder = {}; saveCard.open ({}); };
    presetPanel.onDelete = [this] (int i) { proc.deletePreset (i); refreshPresetPanel(); };
    presetPanel.onToggleKeepScale = [this]
    {
        proc.ui.keepScale = ! proc.ui.keepScale;
        syncModeButtons();
    };
    presetPanel.onToggleTiming = [this]
    {
        lowLatency.setToggleState (! lowLatency.getToggleState(), juce::sendNotificationSync);
        syncModeButtons();
    };

    presetPanel.onToggleFavorite = [this] (int i) {
        const auto list = proc.presetList();
        if (juce::isPositiveAndBelow (i, int (list.size())))
        {
            const auto& e = list[size_t (i)];
            update::Settings::setFavorite (e.folder, e.name, ! update::Settings::isFavorite (e.folder, e.name));
            refreshPresetPanel();
        }
    };
    presetPanel.onDeleteFolder = [this] (const juce::String& f) { proc.deleteFolder (f); refreshPresetPanel(); };
    presetPanel.onMove = [this] (int i, const juce::String& f) { proc.movePreset (i, f); refreshPresetPanel(); };
    presetPanel.onOpenFolder = [this] {
        showBrowser (false);
        const auto folder = MetaTuneProcessor::presetFolder();
        folder.createDirectory();
        folder.startAsProcess();
    };

    canvas.addMouseListener (this, true);
    rootPicker.onRoot = [this] (int pc) {
        pickerPinned = false;
        if (state.getRawParameterValue (ParamID::scale)->load() == float (metatune::ScaleType::Chromatic))
        {
            proc.ui.scaleLabel = {};
            proc.undoManager.beginNewTransaction ("Set key");
            setParam (ParamID::root, float (pc));
            setParam (ParamID::scale, float (metatune::ScaleType::Major));
        }
        else
            setRoot (pc);
        setPianoMode (0);
    };
    rootPicker.onScale = [this] (int i) { pickerPinned = false; applyPickedScale (i); setPianoMode (0); };
    rootPicker.onFlats = [this] (bool f) {
        proc.ui.useFlats = f;
        keyboard.setUseFlats (f);
        syncRootPickerState();
        canvas.repaint();
    };
    rootPicker.onClose = [this] { pickerPinned = false; };
    // Open/close is driven by the pointer poll in timerCallback (transform-safe, unlike mouse enter/exit
    // under a scaled canvas), so the panel's own leave handler stays out of it.
    rootPicker.onMouseLeave = nullptr;

    // Speed: 400 ms .. 0 over the ticks (skewed like the parameter), 0 .. -3 ms over the gradient band.
    {
        auto natural = std::make_shared<juce::NormalisableRange<double>> (0.0, 400.0);
        natural->setSkewForCentre (40.0);
        const double band = layout::speedBand;
        speedKnob.toValue = [natural, band] (double p) {
            return p <= band ? natural->convertFrom0to1 (1.0 - p / band) : -3.0 * (p - band) / (1.0 - band);
        };
        speedKnob.toProportion = [natural, band] (double v) {
            return v >= 0.0 ? band * (1.0 - natural->convertTo0to1 (juce::jmin (400.0, v))) : band + (1.0 - band) * juce::jmin (1.0, -v / 3.0);
        };
    }

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    speedAttach = std::make_unique<SA> (state, ParamID::speed, speedKnob);
    sustainAttach = std::make_unique<SA> (state, ParamID::sustain, sustainKnob);
    amountAttach = std::make_unique<SA> (state, ParamID::amount, amountKnob);
    refAttach = std::make_unique<SA> (state, ParamID::referenceHz, refStrip);
    toneAttach = std::make_unique<SA> (state, ParamID::toneVolume, toneSlider);
    bypassButton.setClickingTogglesState (true);
    bypassButton.onColour = theme::bypassAccent;
    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, ParamID::bypass, bypassButton);
    speedKnob.setDoubleClickReturnValue (true, 20.0);
    sustainKnob.setDoubleClickReturnValue (true, 0.0);
    amountKnob.setDoubleClickReturnValue (true, 100.0);
    refStrip.setDoubleClickReturnValue (true, 440.0);
    toneSlider.setDoubleClickReturnValue (true, -18.0);

    keyboard.onToggleNote = [this] (int pc) { keyClicked (pc); };
    keyboard.onSetRoot = [this] (int pc) { setRoot (pc); };
    // Holding a note label plays its reference tone or chord at Tone Volume.
    keyboard.onPress = [this] (int pc) { proc.toneChord = pianoMode(); proc.toneNote = 60 + pc; };
    keyboard.onRelease = [this] { proc.toneNote = -1; finishPianoChoice(); };

    undoButton.onClick = [this] { proc.undoManager.undo(); };
    redoButton.onClick = [this] { proc.undoManager.redo(); };
    aButton.onClick = [this] { if (proc.getABSlot() != 0) proc.switchAB(); };
    bButton.onClick = [this] { if (proc.getABSlot() != 1) proc.switchAB(); };
    copyButton.onClick = [this] { proc.copyToOtherAB(); };
    menuButton.onModifiedClick = [this] (const juce::ModifierKeys& modifiers)
    {
        if (modifiers.isPopupMenu() || modifiers.isShiftDown())
            showSettingsPanel (! settingsPanel.isVisible());
        else
            showBrowser (! browserTabs.isVisible());
    };
    colorButton.onClick = [this] { showColorPanel (! colorPanel.isVisible()); };
    colorButton.iconScale = 0.66f;
    menuButton.iconScale = 0.44f;
    saveButton.iconScale = 0.46f;
    undoButton.iconScale = redoButton.iconScale = 0.47f;
    copyButton.iconScale = 0.35f;
    copyButton.setToggleState (true, juce::dontSendNotification);
    helpButton.iconScale = 0.48f;
    menuButton.iconOffset.y = 1.45f;
    menuButton.idleColour = juce::Colour (0xffc8c8c8);
    colorButton.idleColour = juce::Colour (0xffc5c5c5);
    saveButton.iconOffset.y = -0.6f;
    saveButton.idleColour = juce::Colour (0xff808080);
    undoButton.idleColour = redoButton.idleColour = juce::Colour (0xffc3c3c3);
    helpButton.iconOffset.y = -0.7f;
    helpButton.idleColour = juce::Colour (0xffcccccc);
    heatButton.captionInset = detectedButton.captionInset = 67.0f;
    heatButton.iconOffset.x = detectedButton.iconOffset.x = -1.0f;
    heatResetButton.iconScale = detectedResetButton.iconScale = 0.52f;
    heatResetButton.idleColour = detectedResetButton.idleColour = juce::Colour (0xffd3d3d3);
    heatButton.setTitle ("Show Heat Maps");
    detectedButton.setTitle ("Show Detected Notes");
    bypassButton.captionInset = 61.0f;
    bypassButton.iconOffset.x = 6.7f;
    bypassButton.idleColour = juce::Colour (0xfff1f0f0);
    chromaticButton.captionInset = 55.0f;
    chromaticButton.iconOffset.x = 22.5f;
    chromaticButton.idleColour = juce::Colour (0xffd5d5d5);
    for (auto* button : { &colorButton, &menuButton, &saveButton, &undoButton, &redoButton, &copyButton,
                          &helpButton, &heatButton, &detectedButton, &heatResetButton, &detectedResetButton,
                          &bypassButton, &chromaticButton })
        button->useFilledIcons = true;
    colorPanel.onChange = [this] (float h, float s, float b) {
        theme::setGradient (false, theme::gradA, theme::gradB); // a solid accent cancels the gradient
        theme::setAccent (h, s, b);
        update::Settings::setGradient (false, update::Settings::gradHueA(), update::Settings::gradHueB());
        update::Settings::setAccentPrefs (h, s, b);
        repaintAll();
    };
    colorPanel.onLight = [this] (bool light) {
        theme::setTheme (light);
        update::Settings::setLightMode (light);
        repaintAll();
    };
    colorPanel.onGradient = [this] (bool on, float gA, float gB) {
        theme::setGradient (on, juce::Colour::fromHSV (gA, 0.85f, 1.0f, 1.0f), juce::Colour::fromHSV (gB, 0.85f, 1.0f, 1.0f));
        update::Settings::setGradient (on, gA, gB);
        repaintAll();
    };
    colorPanel.onElements = [this] (float orb, float key, float meter) {
        theme::orbHue = orb; theme::keyHue = key; theme::meterHue = meter;
        update::Settings::setElementHues (orb, key, meter);
        repaintAll();
    };
    colorPanel.onMouseLeave = [this] {
        if (! colorPanel.getLocalBounds().contains (colorPanel.getMouseXYRelative()))
            showColorPanel (false);
    };
    colorButton.setTooltip ("Colour: recolour MetaTune's accent (hue, saturation, brightness) or pick a preset swatch.");
    saveButton.onClick = [this] { savePresetDialog(); };
    presetBar.onClick = [this] { showPresetPanel (! browserTabs.isVisible() || browserTabs.getTab() != 0); };
    presetBar.onPrev = [this] { stepPreset (-1); };
    presetBar.onNext = [this] { stepPreset (+1); };

    notePill.onClick = [this] { setPianoMode (0); };
    majorPill.onClick = [this] { setPianoMode (1, proc.ui.majorHold); };
    minorPill.onClick = [this] { setPianoMode (2, proc.ui.minorHold); };
    majorHoldPill.onClick = [this] { setPianoMode (proc.ui.majorHold ? 0 : 1, ! proc.ui.majorHold); };
    minorHoldPill.onClick = [this] { setPianoMode (proc.ui.minorHold ? 0 : 2, ! proc.ui.minorHold); };
    chromaticButton.onClick = [this] { toggleChromatic(); };
    lowLatency.onClick = [this] { proc.undoManager.beginNewTransaction ("Low Latency"); setParam (ParamID::range, lowLatency.getToggleState() ? 2.0f : 0.0f); };

    groups.setSelected (proc.ui.group);
    groups.onSelect = [this] (int group)
    {
        proc.joinGroup (group);
        groups.setSelected (proc.getGroup());
        if (groupBrowser.isVisible()) refreshGroupBrowser();
    };
    groups.onBrowse = [this] { showGroupBrowser (! browserTabs.isVisible() || browserTabs.getTab() != 1); };
    groupBrowser.onAssign = [this] (int id, int group)
    {
        MetaTuneProcessor::setInstanceGroup (id, group);
        refreshGroupBrowser();
    };
    groupBrowser.onRename = [this] (int group, const juce::String& name)
    {
        MetaTuneProcessor::setGroupName (group, name);
        refreshGroupBrowser();
    };

    for (auto* b : { &helpButton, &heatButton, &detectedButton })
        b->setClickingTogglesState (true);
    for (auto* b : { &heatButton, &detectedButton, &bypassButton, &chromaticButton })
        b->hoverFill = false;
    helpButton.setToggleState (proc.ui.tooltips, juce::dontSendNotification);
    heatButton.setToggleState (proc.ui.heatmaps, juce::dontSendNotification);
    detectedButton.setToggleState (proc.ui.detectedNotes, juce::dontSendNotification);
    helpButton.onClick = [this] { proc.ui.tooltips = helpButton.getToggleState(); updateTooltips(); };
    heatButton.onClick = [this] { proc.ui.heatmaps = heatButton.getToggleState(); keyboard.setShowHeat (proc.ui.heatmaps); canvas.repaint(); };
    detectedButton.onClick = [this] { proc.ui.detectedNotes = detectedButton.getToggleState(); keyboard.setShowDetected (proc.ui.detectedNotes); };
    heatResetButton.onClick = [this] { keyboard.resetHeat(); };
    detectedResetButton.onClick = [this] { keyboard.resetDetected(); };
    keyboard.setShowHeat (proc.ui.heatmaps);
    keyboard.setShowDetected (proc.ui.detectedNotes);

    // Updates: show what the last check found right away, re-check in the background at most every 6 hours.
    canvas.addChildComponent (updatePill);
    updatePill.onClick = [this] { installUpdate(); };
    if (update::feedUrl().isNotEmpty())
    {
        showUpdate (update::Settings::cached());
        if (update::Settings::autoCheck() && update::Settings::checkDue()) startUpdateCheck();
    }

    speedKnob.setTooltip ("How quickly the voice snaps to the right note. Lower = harder; the coloured band is the robotic zone.");
    sustainKnob.setTooltip ("Tightens long held notes while the start of each note stays natural.");
    amountKnob.setTooltip ("How much of the tuning you hear. 0 leaves the voice untouched.");
    orb.setTooltip ("Live pitch view. Arc on the ring = correction from detected to tuned pitch. Grey ball = Doubler: up = more doubled voices, right = wider (Alt-drag locks an axis).");
    keyboard.setTooltip ("Click a note to allow/forbid it (switches to Custom). Right-click a note to make it the root. With Hold on, a click sets the key.");
    notePill.setTooltip ("Note: pick allowed notes yourself by clicking keys.");
    majorPill.setTooltip ("Major scale on the current root.");
    minorPill.setTooltip ("Minor scale on the current root.");
    majorHoldPill.setTooltip ("Set the root of a major scale by clicking a piano key.");
    minorHoldPill.setTooltip ("Set the root of a minor scale by clicking a piano key.");
    stabilizerSeg.setTooltip ("Note Stabilizer: how long a new note must hold before the tuner moves to it (Short 40 ms, Mid 80 ms, Long 200 ms). Stops quick passing notes from being snapped.");
    refStrip.setTooltip ("Reference tuning for A4. Drag, Shift-drag for fine, double-click for 440 Hz.");
    toneSlider.setTooltip ("Volume of the tone generator: hold a note label to audition a note or chord.");
    groups.setTooltip ("Groups: copies of the plugin in the same group share their settings. The small number shows how many copies are in each group. Click your group again to leave it.");
    presetBar.setTooltip ("Presets: click for the list, arrows step through them. Presets keep your key and scale.");
    chromaticButton.setTooltip ("Switch to the chromatic scale (all notes allowed).");
    undoButton.setTooltip ("Undo");
    redoButton.setTooltip ("Redo");
    aButton.setTooltip ("Snapshot A");
    bButton.setTooltip ("Snapshot B");
    copyButton.setTooltip ("Copy the current snapshot to the other one");
    helpButton.setTooltip ("Show/hide tooltips");
    heatButton.setTooltip ("Show/hide the input and output overlays");
    detectedButton.setTooltip ("Show/hide detected output pitches on the keyboard");
    heatResetButton.setTooltip ("Clear the input and output Heat Maps");
    detectedResetButton.setTooltip ("Clear the detected output notes");
    bypassButton.setTooltip ("Bypass all processing (latency stays compensated)");
    menuButton.setTooltip ("Presets and groups. Shift-click or right-click for settings.");
    saveButton.setTooltip ("Save the current settings as a preset");

    syncModeButtons();
    updateTooltips();
    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio (double (referenceW) / referenceH);
    setResizeLimits (kW * 6 / 10, kH * 6 / 10, kW * 3 / 2, kH * 3 / 2);
    setSize (juce::roundToInt (kW * proc.ui.scale), heightForWidth (kW * proc.ui.scale));
    updateBypassAppearance (bypassButton.getToggleState(), juce::Time::getMillisecondCounterHiRes());
    startTimerHz (60);
}

MetaTuneEditor::~MetaTuneEditor()
{
    // The desktop animator outlives the editor: stop any fade still pointing at our panels.
    auto& animator = juce::Desktop::getInstance().getAnimator();
    animator.cancelAnimation (&rootPicker, false);
    animator.cancelAnimation (&presetPanel, false);
    animator.cancelAnimation (&groupBrowser, false);
    animator.cancelAnimation (&colorPanel, false);
    animator.cancelAnimation (&settingsPanel, false);
    canvas.removeMouseListener (this);
    proc.toneNote = -1; // closing the window while a key is held must not leave the tone on
    tooltipWindow.reset();
    setLookAndFeel (nullptr);
}

void MetaTuneEditor::updateTooltips()
{
    if (proc.ui.tooltips && tooltipWindow == nullptr)
    {
        tooltipWindow = std::make_unique<juce::TooltipWindow> (this, 350);
        tooltipWindow->setLookAndFeel (&lnf);
    }
    else if (! proc.ui.tooltips)
        tooltipWindow.reset();
}

void MetaTuneEditor::resized()
{
    const float s = float (getWidth()) / float (kW);
    // While the window is opening, the host may impose its own first size (clamped to our 60 % minimum):
    // don't let that overwrite the saved size. timerCallback() re-applies openScale shortly after.
    if (juce::Time::getMillisecondCounterHiRes() - openedMs > 1000.0)
        proc.ui.scale = s;
    canvas.setBounds (0, 0, kW, kH);
    canvas.setTransform (juce::AffineTransform::scale (s));
    layout();
}

void MetaTuneEditor::layout()
{
    using namespace layout;
    // Header.
    colorButton.setBounds (6, 2, 43, 38);
    menuButton.setBounds (54, 2, 56, 38);
    presetBar.setBounds (111, 0, 222, headerH);
    saveButton.setBounds (334, 3, 55, 38);
    undoButton.setBounds (790, 4, 48, 36);
    redoButton.setBounds (838, 4, 48, 36);
    aButton.setBounds (889, 3, 48, 38);
    copyButton.setBounds (937, 4, 40, 36);
    bButton.setBounds (977, 3, 46, 38);
    helpButton.setBounds (1024, 3, 57, 38);
    updatePill.setBounds (669, 7, 112, 30);

    // Stage.
    groups.setBounds (0, referencePixel (123), referencePixel (74), referencePixel (440));
    orb.setBounds (65, 32, 606, 466);
    speedKnob.setBounds (around (speedC, speedR));
    sustainKnob.setBounds (around (sustainC, sustainR));
    amountKnob.setBounds (around (amountC, amountR));

    // Mode row.
    notePill.setBounds (noteBounds.expanded (2.0f).toNearestInt());
    majorPill.setBounds (majorBounds.expanded (2.0f).withRight (majorBounds.getRight()).toNearestInt());
    majorHoldPill.setBounds (majorHoldBounds.expanded (2.0f).withLeft (majorHoldBounds.getX()).toNearestInt());
    minorPill.setBounds (minorBounds.expanded (2.0f).withRight (minorBounds.getRight()).toNearestInt());
    minorHoldPill.setBounds (minorHoldBounds.expanded (2.0f).withLeft (minorHoldBounds.getX()).toNearestInt());
    stabilizerSeg.setBounds (timingBounds.reduced (6.0f, 2.0f).toNearestInt());
    lowLatency.setBounds (latencyTrack.expanded (0.0f, 3.0f).toNearestInt());

    // Bottom.
    keyboard.setBounds (leftW, bottomY, rightX - leftW, footerY - bottomY);
    toneSlider.setBounds (12, keyboardLabelY + 29, leftW - 19, 25);
    rootArea = { 0, pianoY, leftW, chromaticY - pianoY };
    chromaticButton.setBounds (0, chromaticY, leftW, footerY - chromaticY);
    heatButton.setBounds (rightX + 6, bottomY, kW - rightX - 10, keyboardLabelY - bottomY);
    detectedButton.setBounds (rightX + 6, keyboardLabelY, kW - rightX - 10, pianoY - keyboardLabelY);
    heatResetButton.setBounds (heatButton.getBounds().withX (heatButton.getX() + 30).withWidth (28));
    detectedResetButton.setBounds (detectedButton.getBounds().withX (detectedButton.getX() + 30).withWidth (28));
    bypassButton.setBounds (rightX + 12, pianoY, kW - rightX - 18, referenceY - pianoY);
    refStrip.setBounds (rightX + 13, referenceY + 29, kW - rightX - 22, 45);
    // (the picker's bounds are driven every frame by updatePicker)
}

void MetaTuneEditor::Canvas::paint (juce::Graphics& g)
{
    using namespace theme;
    using namespace layout;
    auto& e = owner;
    g.fillAll (bg1);

    // Header bar with darker cells.
    g.setGradientFill (juce::ColourGradient ((lightMode ? header.brighter (0.10f) : dark::toolbarTop), 0.0f, 0.0f,
                                             (lightMode ? header : dark::toolbarBottom), 0.0f, float (headerH), false));
    g.fillRect (0, 0, kW, headerH);
    if (! lightMode)
    {
        g.setColour (juce::Colour (0xff2e2e2f));
        g.fillRect (0, 0, 111, headerH);
        g.fillRect (1024, 0, kW - 1024, headerH);
        g.setColour (juce::Colour (0xff232323));
        g.fillRect (789, 0, 98, headerH);
        g.setColour (juce::Colour (0xff262626));
        g.fillRect (887, 0, 137, headerH);
    }
    g.setColour (headerBox);
    g.fillRect (333, 0, 56, headerH);
    {
        // Tile behind the colour (palette) button.
        const auto tile = juce::Rectangle<float> (11, 4, 32, 34);
        g.setGradientFill (juce::ColourGradient (lightMode ? bg3 : juce::Colour (0xff303031), tile.getTopLeft(),
                                                 lightMode ? bg2 : juce::Colour (0xff2d2d2d), tile.getBottomLeft(), false));
        g.fillRoundedRectangle (tile, 2.0f);
    }
    {
        const auto title = juce::Rectangle<int> (389, 0, 400, headerH);
        if (e.titleTexture.isNull())
        {
            e.titleTexture = juce::Image (juce::Image::RGB, title.getWidth(), title.getHeight(), false);
            e.bypassTitleTexture = juce::Image (juce::Image::RGB, title.getWidth(), title.getHeight(), false);
            juce::Image::BitmapData pixels (e.titleTexture, juce::Image::BitmapData::writeOnly);
            juce::Image::BitmapData bypassPixels (e.bypassTitleTexture, juce::Image::BitmapData::writeOnly);
            const auto titleBase = lightMode ? headerBox : juce::Colour (0xff242426);
            const auto titleAccent = lightMode ? bg3 : juce::Colour (0xff4d4072);
            const auto bypassTitleAccent = lightMode ? juce::Colour (0xfff2c6c4) : juce::Colour (0xff42292b);
            for (int y = 0; y < pixels.height; ++y)
                for (int x = 0; x < pixels.width; ++x)
                {
                    const float u = float (x) / float (pixels.width);
                    const float cloud = surfaceNoise (float (x), float (y));
                    const float wave = std::sin (float (x) * 0.041f + float (y) * 0.12f + 0.85f * cloud);
                    const float envelope = juce::jmin (1.0f, 6.2f * juce::jmin (u, 1.0f - u));
                    const float level = envelope * (0.64f + 0.13f * cloud + 0.14f * wave);
                    pixels.setPixelColour (x, y, titleBase.interpolatedWith (titleAccent, level));
                    bypassPixels.setPixelColour (x, y, titleBase.interpolatedWith (bypassTitleAccent, level));
                }
        }
        g.drawImageAt (e.titleTexture, title.getX(), title.getY());
        if (e.bypassBlend > 0.0f)
        {
            g.setOpacity (e.bypassBlend);
            g.drawImageAt (e.bypassTitleTexture, title.getX(), title.getY());
            g.setOpacity (1.0f);
        }
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillPath (wordmark(), juce::AffineTransform::translation (0.0f, 0.7f));
        const auto normalWordmark = lightMode ? text : juce::Colour (0xfffaf8fc);
        const auto bypassWordmark = lightMode ? juce::Colour (0xffa44747) : theme::bypassAccent;
        g.setColour (normalWordmark.interpolatedWith (bypassWordmark, e.bypassBlend));
        g.fillPath (wordmark());
    }
    g.setColour (juce::Colours::black.withAlpha (0.72f));
    for (int x : { 53, 111, 334, 389, 789, 887, 1024 })
        g.fillRect (x, 0, 1, headerH);
    g.fillRect (0, headerH, kW, 2);
    g.setColour (text.withAlpha (0.09f));
    g.fillRect (1, 2, kW - 2, 1);

    // Stage: obsidian with soft radial highlights; graphite panel on the right with a curved edge.
    {
        const juce::Rectangle<float> st (0.0f, float (headerH + 2), float (kW), float (rowY - headerH - 2));
        g.setColour (stage);
        g.fillRect (st);
        const auto stageLight = lightMode ? highlight : juce::Colour (0xff1d1d1e);
        auto glowAt = [&] (float x, float y, float r, float alpha) {
            g.setGradientFill (juce::ColourGradient (stageLight.brighter (0.25f).withAlpha (alpha), x, y, stageLight.withAlpha (0.0f), x + r, y, true));
            g.fillEllipse (x - r, y - r, 2 * r, 2 * r);
        };
        glowAt (65.0f, 84.0f, 380.0f, 0.8f);
        glowAt (40.0f, float (rowY), 260.0f, 0.6f);
    }
    {
        auto panelPath = panelEdge();
        panelPath.lineTo (float (kW), float (rowY));
        panelPath.lineTo (float (kW), float (headerH));
        panelPath.closeSubPath();
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (0, headerH + 2, kW, rowY - headerH - 2);
        g.reduceClipRegion (panelPath);
        if (e.brushed.isNull()) e.brushed = makeConcretePanel();
        g.drawImageAt (e.brushed, 0, 0);
    }
    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (0, headerH + 2, kW, rowY - headerH - 2);
        g.setColour (juce::Colours::black.withAlpha (0.38f));
        g.strokePath (panelEdge(), juce::PathStrokeType (1.0f));
    }

    // Mode row.
    juce::ColourGradient modeLight (lightMode ? bg3 : dark::modeStrip, 0.0f, float (rowY),
                                    lightMode ? bg2 : juce::Colours::black, float (kW), float (rowY + rowH), false);
    if (! lightMode)
    {
        modeLight.addColour (0.31, juce::Colour (0xff1c1c1c));
        modeLight.addColour (0.49, juce::Colour (0xff1e1e1e));
        modeLight.addColour (0.76, juce::Colour (0xff151515));
        modeLight.addColour (0.92, juce::Colours::black);
    }
    g.setGradientFill (modeLight);
    g.fillRect (0, rowY, kW, rowH);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRect (0, rowY, kW, 1);
    g.fillRect (0, rowY + rowH - 1, kW, 1);
    g.setColour (text.withAlpha (0.13f));
    g.fillRect (0, rowY + 1, kW, 1);
    g.setFont (font (16.0f));
    const auto legend = lightMode ? textDim : juce::Colour (0xffd6d6d6);
    shadowText (g, "Select", juce::Rectangle<float> (28.0f, float (rowY), 60.0f, float (rowH)), juce::Justification::centredLeft, legend, 0.8f, 0.44f);
    g.setColour (legend);
    g.fillRect (103, rowY + rowH / 2, 19, 1);
    {
        const auto timing = timingBounds;
        g.setColour (lightMode ? bg0 : juce::Colours::black);
        g.fillRoundedRectangle (timing, timing.getHeight() * 0.5f);
        const auto inset = timing.reduced (4.0f, 2.8f);
        g.setGradientFill (juce::ColourGradient (text.withAlpha (0.13f), inset.getTopLeft(),
                                                 text.withAlpha (0.03f), inset.getBottomLeft(), false));
        g.drawRoundedRectangle (inset, inset.getHeight() * 0.5f, 1.0f);

        const auto lbl = stabilizerBounds;
        const auto base = lightMode ? bg3 : dark::control;
        g.setGradientFill (juce::ColourGradient (base.brighter (0.032f), lbl.getTopLeft(),
                                                 base.darker (0.23f), lbl.getBottomLeft(), false));
        g.fillRoundedRectangle (lbl, lbl.getHeight() * 0.5f);
        g.setColour (text.withAlpha (0.08f));
        g.drawRoundedRectangle (lbl, lbl.getHeight() * 0.5f, 0.8f);
        g.setFont (font (16.0f));
        shadowText (g, "Note Stabilizer", lbl, juce::Justification::centred,
                    e.stabilizerSeg.usesWarmAccent() ? dark::timingAmber : text.withAlpha (0.9f));
    }
    g.setFont (font (16.0f, true));
    shadowFittedText (g, "Low\nLatency", latencyCaption.toNearestInt(), juce::Justification::centredLeft, 2, e.lowLatency.getToggleState() ? lavender : textDim);

    // Bottom side columns.
    for (const auto column : { juce::Rectangle<int> (0, bottomY, leftW, footerY - bottomY),
                               juce::Rectangle<int> (rightX, bottomY, kW - rightX, footerY - bottomY) })
    {
        const bool left = column.getX() == 0;
        const auto top = lightMode ? bg3 : (left ? dark::scalePanel : juce::Colour (0xff242424));
        const auto end = lightMode ? bg2 : (left ? juce::Colour (0xff212121) : juce::Colour (0xff363636));
        g.setGradientFill (juce::ColourGradient (top, column.getTopLeft().toFloat(), end,
                                                 lightMode ? column.getBottomLeft().toFloat() : column.getTopRight().toFloat(), false));
        g.fillRect (column);
        if (! lightMode && ! left)
        {
            const auto lower = column.withTop (referenceY).toFloat();
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff212121), lower.getTopLeft(),
                                                     dark::utilityPanel, lower.getTopRight(), false));
            g.fillRect (lower);
        }
    }
    g.setColour (header);
    g.fillRect (0, footerY, kW, kH - footerY);
    g.setColour (line);
    g.fillRect (0, footerY, kW, 1);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRect (leftW - 1, bottomY, 1, kH - bottomY);
    g.fillRect (rightX, bottomY, 1, kH - bottomY);
    for (int y : { bottomY + e.keyboard.heatOutputY() - 1, keyboardLabelY, pianoY })
        g.fillRect (0, y, leftW, 1);
    for (int y : { keyboardLabelY, pianoY, referenceY })
        g.fillRect (rightX, y, kW - rightX, 1);

    g.setFont (font (15.0f));
    if (e.proc.ui.heatmaps)
    {
        shadowText (g, "Input", juce::Rectangle<float> (28.0f, float (bottomY + e.keyboard.heatInputY()), 60.0f, float (KeyboardPanel::kHeatH)), juce::Justification::centredLeft, legend, 0.8f, 0.44f);
        shadowText (g, "Output", juce::Rectangle<float> (28.0f, float (bottomY + e.keyboard.heatOutputY()), 60.0f, float (KeyboardPanel::kHeatH)), juce::Justification::centredLeft, legend, 0.8f, 0.44f);
        g.setColour (legend);
        g.fillRect (103, bottomY + e.keyboard.heatInputY() + KeyboardPanel::kHeatH / 2, 19, 1);
        g.fillRect (103, bottomY + e.keyboard.heatOutputY() + KeyboardPanel::kHeatH / 2, 19, 1);
    }
    shadowText (g, "Tone Volume", juce::Rectangle<float> (0.0f, float (keyboardLabelY + 8), float (leftW), 22.0f), juce::Justification::centred, text.withAlpha (0.9f));
    shadowText (g, "Pitch Reference", juce::Rectangle<float> (float (rightX), float (referenceY + 3), float (kW - rightX), 24.0f), juce::Justification::centred, text.withAlpha (0.9f));

    const int root = juce::jlimit (0, 11, int (e.state.getRawParameterValue (ParamID::root)->load()));
    static const char* flatNames[] { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
    const juce::String name = e.proc.ui.useFlats ? flatNames[root] : kNoteNames[root];
    g.setColour (text);
    const float rootScale = float (e.rootArea.getHeight()) / 136.0f;
    if (! e.proc.ui.scaleEverSet)
    {
        // First run only (no key ever picked): a mouse cursor and a prompt instead of a key.
        const auto area = e.rootArea.toFloat();
        const juce::Colour promptCol = textDim.interpolatedWith (text, 0.4f); // adapts to the theme
        juce::Path cur;
        cur.startNewSubPath (0.0f, 0.0f);
        cur.lineTo (0.0f, 16.0f);
        cur.lineTo (4.2f, 11.8f);
        cur.lineTo (7.0f, 18.0f);
        cur.lineTo (9.4f, 17.0f);
        cur.lineTo (6.7f, 11.0f);
        cur.lineTo (12.5f, 11.0f);
        cur.closeSubPath();
        const auto curBox = juce::Rectangle<float> (31.0f * rootScale, 39.0f * rootScale).withCentre ({ area.getCentreX(), area.getY() + 32.0f * rootScale });
        const auto curXform = cur.getTransformToScaleToFit (curBox, true);
        g.setColour (juce::Colours::black.withAlpha (0.5f)); // shadow
        g.fillPath (cur, curXform.followedBy (juce::AffineTransform::translation (0.0f, 1.6f)));
        g.setColour (promptCol);
        g.fillPath (cur, curXform);
        g.setFont (font (18.0f, true));
        shadowFittedText (g, "Hover to\nset scale", e.rootArea.withTrimmedTop (juce::roundToInt (58.0f * rootScale)), juce::Justification::centredTop, 2, promptCol);
    }
    else
    {
        const auto letter = name.substring (0, 1);
        const auto accidental = name.endsWithChar ('b') ? juce::String::charToString (0x266d) : name.substring (1);
        const auto letterFont = font (83.5f * rootScale, true).withHorizontalScale (0.96f), accidentalFont = font (63.0f * rootScale);
        const float letterWidth = juce::GlyphArrangement::getStringWidth (letterFont, letter);
        const float accidentalWidth = juce::GlyphArrangement::getStringWidth (accidentalFont, accidental);
        const auto area = e.rootArea.toFloat().withTrimmedTop (12.0f * rootScale).withHeight (83.0f * rootScale);
        const float x = area.getCentreX() + 2.8f - (letterWidth + accidentalWidth) * 0.5f;
        g.setFont (letterFont);
        shadowText (g, letter, area.withX (x).withWidth (letterWidth + 1.0f), juce::Justification::centredLeft, text);
        if (accidental.isNotEmpty())
        {
            g.setFont (accidentalFont);
            shadowText (g, accidental, area.withX (x + letterWidth).withWidth (accidentalWidth + 2.0f).translated (0.0f, -12.8f * rootScale),
                        juce::Justification::centredLeft, text);
        }
        g.setFont (font (23.5f * rootScale, true));
        shadowText (g, e.scaleDisplayName(), e.rootArea.toFloat().withTrimmedTop (94.0f * rootScale).withHeight (26.0f * rootScale).translated (2.8f, 0.0f),
                    juce::Justification::centred, text, 0.9f, 0.48f);
    }

    // Chromatic tab.
    juce::Path tab;
    tab.addRoundedRectangle (0.0f, float (chromaticY), float (leftW), float (footerY - chromaticY), 28.0f, 28.0f, true, false, false, false);
    g.setGradientFill (juce::ColourGradient (lightMode ? bg3 : juce::Colour (0xff272727), 0.0f, float (chromaticY),
                                             lightMode ? bg2 : juce::Colour (0xff202020), float (leftW), float (footerY), false));
    g.fillPath (tab);
    g.setColour (text.withAlpha (0.09f));
    g.strokePath (tab, juce::PathStrokeType (0.75f));

    g.setColour (lightMode ? line : juce::Colour (0xff4b4b4b));
    g.drawRect (getLocalBounds(), 1);
}

void MetaTuneEditor::syncModeButtons()
{
    const auto scale = metatune::ScaleType (juce::jlimit (0, 3, int (state.getRawParameterValue (ParamID::scale)->load())));
    chromaticButton.onColour = theme::lavender;
    chromaticButton.setToggleState (scale == metatune::ScaleType::Chromatic, juce::dontSendNotification);
    const int mode = pianoMode();
    keyboard.setAuditionMode (mode);
    notePill.setToggleState (mode == 0, juce::dontSendNotification);
    majorPill.setToggleState (mode == 1, juce::dontSendNotification);
    minorPill.setToggleState (mode == 2, juce::dontSendNotification);
    majorHoldPill.setToggleState (proc.ui.majorHold, juce::dontSendNotification);
    minorHoldPill.setToggleState (proc.ui.minorHold, juce::dontSendNotification);
    const bool lowLatencyOn = int (state.getRawParameterValue (ParamID::range)->load()) == 2;
    const bool warmTiming = ! lowLatencyOn && state.getRawParameterValue (ParamID::stabilizer)->load() > 0.0f;
    lowLatency.setToggleState (lowLatencyOn, juce::dontSendNotification);
    lowLatency.setWarmAccent (warmTiming);
    presetPanel.setModeOptions (proc.ui.keepScale, lowLatencyOn, warmTiming);
    if (stabilizerSeg.usesWarmAccent() != warmTiming)
        canvas.repaint (layout::stabilizerBounds.getSmallestIntegerContainer());
    stabilizerSeg.setWarmAccent (warmTiming);
}

void MetaTuneEditor::updateBypassAppearance (bool bypassed, double now)
{
    constexpr double fadeMs = 180.0;
    const float step = lastBypassFrameMs < 0.0 ? 1.0f
        : static_cast<float> (juce::jlimit (0.0, 1.0, (now - lastBypassFrameMs) / fadeMs));
    lastBypassFrameMs = now;
    const float next = juce::jlimit (0.0f, 1.0f, bypassBlend + (bypassed ? step : -step));
    if (next == bypassBlend)
        return;

    bypassBlend = next;
    speedKnob.setBypassBlend (next);
    sustainKnob.setBypassBlend (next);
    amountKnob.setBypassBlend (next);
    groups.setBypassBlend (next);
    keyboard.setBypassBlend (next);
    canvas.repaint (juce::Rectangle<int> (389, 0, 400, layout::headerH));
}

void MetaTuneEditor::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes();
    const float elapsed = lastFrameMs < 0.0 ? 1.0f / 60.0f : float ((now - lastFrameMs) * 0.001);
    lastFrameMs = now;
    updateFrame (now, elapsed);
}

void MetaTuneEditor::updateFrame (double now, float elapsedSeconds)
{
    // Drain DSP telemetry: every sample feeds the HeatMaps, the newest drives the live displays.

    // Once, shortly after opening: if the host resized us away from the saved size, put it back.
    if (! sizeRestored && now - openedMs > 250.0)
    {
        sizeRestored = true;
        if (std::abs (getWidth() - juce::roundToInt (kW * openScale)) > 2)
            setSize (juce::roundToInt (kW * openScale), heightForWidth (kW * openScale));
    }
    if (showNotesPending && sizeRestored)
    {
        showNotesPending = false;
        showNotesCard();
    }
    // While bypassed nothing is corrected: show output = input and no target.
    const bool bypassed = state.getRawParameterValue (ParamID::bypass)->load() > 0.5f;
    metatune::Telemetry t;
    while (proc.telemetry.pop (t))
    {
        latestRms = t.rms;
        latestSustainRatio = t.sustainTimeRatio;
        if (t.inputMidi > 0 && t.confidence >= 0.7f)
        {
            const float out = bypassed ? t.inputMidi : t.outputMidi;
            keyboard.addHeat (t.inputMidi, out, t.confidence * juce::jmin (1.0f, t.rms * 8.0f) * 0.35f);
            display.inputMidi = t.inputMidi;
            display.outputMidi = t.outputMidi;
            display.target = t.target;
            lastVoicedMs = now;
        }
    }
    if (bypassed)
    {
        display.outputMidi = display.inputMidi;
        display.target = -1;
    }
    keyboard.decayHeat (0.91f);
    display.voiced = now - lastVoicedMs < 80.0;
    display.presence += display.voiced ? (1.0f - display.presence) * 0.35f : -display.presence * 0.10f;
    const float lvl = juce::jlimit (0.0f, 1.0f, (juce::Decibels::gainToDecibels (latestRms, -100.0f) + 50.0f) / 40.0f);
    display.level += (lvl - display.level) * 0.25f;

    updateBypassAppearance (bypassed, now);
    orb.setDisplay (display, state.getRawParameterValue (ParamID::speed)->load(), bypassBlend, elapsedSeconds);
    speedKnob.setActivity (display.presence * (0.35f + 0.65f * display.level));
    const float sustain = state.getRawParameterValue (ParamID::sustain)->load() * 0.01f;
    speedKnob.setSustainDisplay (sustain, display.voiced && display.target >= 0 ? latestSustainRatio : 1.0f);
    heatButton.setToggleState (proc.ui.heatmaps, juce::dontSendNotification);
    detectedButton.setToggleState (proc.ui.detectedNotes, juce::dontSendNotification);
    keyboard.setShowHeat (proc.ui.heatmaps);
    keyboard.setShowDetected (proc.ui.detectedNotes);
    keyboard.setDisplay (display);
    keyboard.setMask (proc.currentMask());
    keyboard.setUseFlats (proc.ui.useFlats);
    groups.setSelected (proc.getGroup());
    for (int gi = 0; gi < 4; ++gi)
        groups.setCount (gi, MetaTuneProcessor::groupMemberCount (gi));
    // If the current preset's file was deleted on disk (in Explorer or elsewhere), stop showing its name.
    if (++presetCheckTicks >= 45)
    {
        presetCheckTicks = 0;
        if (proc.ui.presetName.isNotEmpty() && ! proc.presetNames().contains (proc.ui.presetName))
            proc.ui.presetName = {};
    }
    if (presetBar.name != proc.ui.presetName) { presetBar.name = proc.ui.presetName; presetBar.repaint(); }
    if (autoKeyCard.isVisible() && (presetCheckTicks % 12) == 0) // refresh MetaKey's live key a few times a second
    {
        const bool inst = update::metaKeyInstalled();
        const auto st = update::readMetaKeyStatus();
        autoKeyCard.setState (inst, inst && st.live && st.valid(), st.root, st.minor);
    }
    syncModeButtons();

    undoButton.setEnabled (proc.undoManager.canUndo());
    redoButton.setEnabled (proc.undoManager.canRedo());
    aButton.setToggleState (proc.getABSlot() == 0, juce::dontSendNotification);
    bButton.setToggleState (proc.getABSlot() == 1, juce::dontSendNotification);
    copyButton.setTooltip (proc.getABSlot() == 0 ? "Copy A to B" : "Copy B to A");

    const int range = juce::jlimit (0, 2, int (state.getRawParameterValue (ParamID::range)->load()));
    if (range != shownRange)
    {
        shownRange = range;
        lowLatency.setTooltip ("Low Latency: track from 150 Hz up for the smallest delay. Now: " + kRangeNames[range] + ", "
                               + juce::String (proc.latencyMsFor (range), 1) + " ms added latency. Change while stopped.");
    }
    updatePicker();

    canvas.repaint (rootArea);                              // root letter / scale name
    canvas.repaint (1007, layout::rowY, 75, layout::rowH); // Low Latency caption colour
}

void MetaTuneEditor::setParam (const juce::String& id, float value)
{
    // Any deliberate key/scale edit retires the first-run "Hover to set scale" prompt for good.
    if (id == ParamID::scale || id == ParamID::root || id.startsWith ("note"))
        proc.ui.scaleEverSet = true;
    if (auto* p = state.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (value));
        p->endChangeGesture();
    }
}

void MetaTuneEditor::selectScale (metatune::ScaleType type)
{
    proc.ui.scaleLabel = {};
    proc.undoManager.beginNewTransaction ("Scale");
    setParam (ParamID::scale, float (type));
}

void MetaTuneEditor::selectNoteMode()
{
    // Note mode = pick notes by hand: start a custom scale from what is currently audible.
    const auto scale = metatune::ScaleType (int (state.getRawParameterValue (ParamID::scale)->load()));
    if (scale != metatune::ScaleType::Custom)
    {
        const auto mask = proc.currentMask();
        proc.ui.scaleLabel = {};
        proc.undoManager.beginNewTransaction ("Note mode");
        for (int i = 0; i < 12; ++i)
            setParam (ParamID::note (i), metatune::isAllowed (mask, i) ? 1.0f : 0.0f);
        setParam (ParamID::scale, float (metatune::ScaleType::Custom));
    }
}

int MetaTuneEditor::pianoMode() const
{
    if (proc.ui.majorHold) return 1;
    if (proc.ui.minorHold) return 2;
    return toneChordMode;
}

void MetaTuneEditor::setPianoMode (int mode, bool hold)
{
    toneChordMode = hold ? 0 : mode;
    proc.ui.majorHold = hold && mode == 1;
    proc.ui.minorHold = hold && mode == 2;
    syncModeButtons();
}

void MetaTuneEditor::finishPianoChoice()
{
    if (! proc.ui.majorHold && ! proc.ui.minorHold)
        setPianoMode (0);
}

void MetaTuneEditor::toggleChromatic()
{
    using metatune::ScaleType;
    setPianoMode (0);
    const auto type = ScaleType (int (state.getRawParameterValue (ParamID::scale)->load()));
    auto& scaleBeforeChromatic = proc.ui.scaleBeforeChromatic;
    if (type != ScaleType::Chromatic)
    {
        const auto root = int (state.getRawParameterValue (ParamID::root)->load());
        scaleBeforeChromatic = MetaTuneProcessor::UiSettings::SavedScale { type, root, proc.currentMask(), proc.ui.scaleLabel };
        selectScale (ScaleType::Chromatic);
    }
    else if (scaleBeforeChromatic)
    {
        const auto& saved = *scaleBeforeChromatic;
        proc.undoManager.beginNewTransaction ("Restore scale");
        setParam (ParamID::root, float (saved.root));
        if (saved.type == ScaleType::Custom)
            for (int pc = 0; pc < 12; ++pc)
                setParam (ParamID::note (pc), metatune::isAllowed (saved.mask, pc) ? 1.0f : 0.0f);
        proc.ui.scaleLabel = saved.label;
        setParam (ParamID::scale, float (saved.type));
        scaleBeforeChromatic.reset();
    }
    syncModeButtons();
}

void MetaTuneEditor::keyClicked (int pc)
{
    const int mode = pianoMode();
    if (mode == 0)
    {
        toggleNote (pc);
        return;
    }
    proc.undoManager.beginNewTransaction ("Set key");
    proc.ui.scaleLabel = {};
    setParam (ParamID::root, float (pc));
    setParam (ParamID::scale, float (mode == 1 ? metatune::ScaleType::Major : metatune::ScaleType::Minor));
    finishPianoChoice();
}

void MetaTuneEditor::toggleNote (int pc)
{
    const auto mask = proc.currentMask();
    proc.ui.scaleLabel = {}; // hand-edited: no longer a named scale
    proc.undoManager.beginNewTransaction ("Edit scale");
    if (int (state.getRawParameterValue (ParamID::scale)->load()) != int (metatune::ScaleType::Custom))
    {
        // Start the custom scale from what is currently audible.
        for (int i = 0; i < 12; ++i)
            setParam (ParamID::note (i), metatune::isAllowed (mask, i) ? 1.0f : 0.0f);
        setParam (ParamID::scale, float (metatune::ScaleType::Custom));
    }
    setParam (ParamID::note (pc), metatune::isAllowed (mask, pc) ? 0.0f : 1.0f);
}

void MetaTuneEditor::setRoot (int pc)
{
    proc.undoManager.beginNewTransaction ("Set root");
    const int scale = int (state.getRawParameterValue (ParamID::scale)->load());
    if (scale == int (metatune::ScaleType::Custom))
    {
        // Moving the root of a custom scale transposes it.
        const int shift = pc - int (state.getRawParameterValue (ParamID::root)->load());
        const auto moved = metatune::transposeMask (proc.currentMask(), shift);
        for (int i = 0; i < 12; ++i)
            setParam (ParamID::note (i), metatune::isAllowed (moved, i) ? 1.0f : 0.0f);
    }
    setParam (ParamID::root, float (pc));
}

void MetaTuneEditor::syncRootPickerState()
{
    const auto sc = metatune::ScaleType (int (state.getRawParameterValue (ParamID::scale)->load()));
    int idx = RootPicker::scaleNames().indexOf (proc.ui.scaleLabel, true);
    if (sc == metatune::ScaleType::Major) idx = 0;
    if (sc == metatune::ScaleType::Minor) idx = 1;
    if (sc == metatune::ScaleType::Chromatic) idx = 0;
    if (sc == metatune::ScaleType::Custom && proc.ui.scaleLabel.isEmpty()) idx = RootPicker::scaleNames().indexOf ("Custom");
    rootPicker.setState (int (state.getRawParameterValue (ParamID::root)->load()), idx, proc.ui.useFlats);
}

// Frame-driven scale picker: eased entirely here off the 60 Hz timer so the slide is identical and
// reliable every time (no ComponentAnimator proxy/visibility races, no dependence on mouse enter/exit).
void MetaTuneEditor::updatePicker()
{
    using namespace layout;
    const juce::Rectangle<int> area (0, bottomY, leftW - 1, chromaticY - bottomY);
    const int offX = -area.getWidth(); // fully tucked off the left edge at slide 0

    // Target: open while the pointer is over the root cell or the (visible) panel, or while pinned.
    const auto screen = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition();
    const auto mp = canvas.getLocalPoint (nullptr, screen).roundToInt();
    const bool over = rootArea.contains (mp) || (rootPicker.isVisible() && area.contains (mp));
    const float target = (over || pickerPinned) ? 1.0f : 0.0f;

    // Smooth, symmetric exponential ease toward the target (fps-independent enough at a steady 60 Hz).
    pickerSlide += (target - pickerSlide) * 0.20f;
    if (std::abs (target - pickerSlide) < 0.0015f) pickerSlide = target;

    const bool shouldShow = pickerSlide > 0.0015f;
    if (shouldShow && ! rootPicker.isVisible())
    {
        syncRootPickerState();
        rootPicker.setAlpha (1.0f);
        rootPicker.setVisible (true);
        rootPicker.toFront (false);
    }
    else if (! shouldShow && rootPicker.isVisible())
        rootPicker.setVisible (false);

    if (rootPicker.isVisible())
    {
        rootPicker.setBounds (area.withX (juce::roundToInt (float (offX) * (1.0f - pickerSlide))));
        syncRootPickerState();
    }
    const auto point = rootPicker.getLocalPoint (&canvas, mp.toFloat());
    keyboard.setPreviewMask (over && rootPicker.isVisible() && rootPicker.getLocalBounds().toFloat().contains (point)
                                 ? pickerPreview (point) : std::nullopt);
}

void MetaTuneEditor::mouseDown (const juce::MouseEvent& e)
{
    if (browserTabs.isVisible() && e.eventComponent != &presetPanel && e.eventComponent != &groupBrowser
        && e.eventComponent != &browserTabs && e.eventComponent != &presetBar && e.eventComponent != &groups
        && e.eventComponent != &menuButton
        && ! presetPanel.isParentOf (e.eventComponent) && ! groupBrowser.isParentOf (e.eventComponent))
        showBrowser (false);

    if (settingsPanel.isVisible() && e.eventComponent != &settingsPanel && e.eventComponent != &menuButton)
        showSettingsPanel (false);
    if (colorPanel.isVisible() && e.eventComponent != &colorPanel && e.eventComponent != &colorButton)
        showColorPanel (false);
    if (! rootPicker.isVisible() || e.eventComponent == &rootPicker || rootPicker.isParentOf (e.eventComponent))
        return;
    const auto p = canvas.getLocalPoint (e.eventComponent, e.getPosition());
    if (! rootArea.contains (p)) // a click outside the root cell / picker dismisses it
        pickerPinned = false;      // updatePicker then eases it closed (pointer is already away)
}

juce::String MetaTuneEditor::scaleDisplayName() const
{
    const int scale = juce::jlimit (0, 3, int (state.getRawParameterValue (ParamID::scale)->load()));
    if (scale == int (metatune::ScaleType::Custom) && proc.ui.scaleLabel.isNotEmpty())
        return proc.ui.scaleLabel.toLowerCase();
    return juce::String (kScaleNames[scale]).toLowerCase();
}

static metatune::PitchClassMask drawerScaleMask (int index, int root)
{
    using namespace metatune;
    constexpr PitchClassMask masks[] {
        kMajorFromC, kMinorFromC,
        0x0295, 0x04a9, // major and minor pentatonic
        0x09b5, 0x09ad  // harmonic major and harmonic minor
    };
    return transposeMask (masks[index], root);
}

std::optional<metatune::PitchClassMask> MetaTuneEditor::pickerPreview (juce::Point<float> point) const
{
    using metatune::ScaleType;
    const int root = int (state.getRawParameterValue (ParamID::root)->load());
    const auto type = ScaleType (int (state.getRawParameterValue (ParamID::scale)->load()));
    if (const int pitch = rootPicker.rootAt (point); pitch >= 0)
        return type == ScaleType::Chromatic ? drawerScaleMask (0, pitch)
                                            : metatune::transposeMask (proc.currentMask(), pitch - root);
    if (const int index = rootPicker.scaleAt (point); index >= 0 && index < 6)
        return drawerScaleMask (index, root);
    return std::nullopt;
}

void MetaTuneEditor::applyPickedScale (int index)
{
    using metatune::ScaleType;
    if (index == 0) { selectScale (ScaleType::Major); return; }
    if (index == 1) { selectScale (ScaleType::Minor); return; }
    if (index == 6) { selectNoteMode(); return; }
    if (! juce::isPositiveAndBelow (index, 6)) return;

    const auto mask = drawerScaleMask (index, int (state.getRawParameterValue (ParamID::root)->load()));
    proc.undoManager.beginNewTransaction ("Scale");
    for (int pc = 0; pc < 12; ++pc)
        setParam (ParamID::note (pc), metatune::isAllowed (mask, pc) ? 1.0f : 0.0f);
    setParam (ParamID::scale, float (ScaleType::Custom));
    proc.ui.scaleLabel = RootPicker::scaleNames()[index];
}

void MetaTuneEditor::showBrowser (bool show, int tab)
{
    constexpr int x = 53, y = 56, width = 188, tabHeight = 32;
    constexpr int tuckedOffset = -(x + width);
    const juce::Rectangle<int> tabsOpen (x, y, width, tabHeight);
    const juce::Rectangle<int> bodyOpen (x, y + tabHeight, width, layout::rowY - y - tabHeight);
    auto& animator = juce::Desktop::getInstance().getAnimator();

    if (! show)
    {
        if (! browserTabs.isVisible()) return;
        for (auto* component : std::array<juce::Component*, 3> { &browserTabs, &presetPanel, &groupBrowser })
        {
            if (! component->isVisible()) continue;
            animator.cancelAnimation (component, false);
            animator.animateComponent (component, component->getBounds().withX (-width), 0.0f, 170, true, 1.0, 1.0);
            component->setVisible (false);
        }
        groups.setBrowseActive (false);
        menuButton.setIcon (icons::menu());
        return;
    }

    showSettingsPanel (false);
    showColorPanel (false);
    const bool wasOpen = browserTabs.isVisible();
    if (tab >= 0) browserTabs.setTab (tab);
    const bool showingGroups = browserTabs.getTab() == 1;
    auto& body = showingGroups ? static_cast<juce::Component&> (groupBrowser) : static_cast<juce::Component&> (presetPanel);
    auto& other = showingGroups ? static_cast<juce::Component&> (presetPanel) : static_cast<juce::Component&> (groupBrowser);
    animator.cancelAnimation (&other, false);
    other.setVisible (false);

    if (showingGroups) refreshGroupBrowser();
    else refreshPresetPanel();
    groups.setBrowseActive (showingGroups);
    juce::Path drawerIcon;
    drawerIcon.addRoundedRectangle (0.0f, 0.0f, 20.0f, 5.0f, 1.0f);
    drawerIcon.addRoundedRectangle (0.0f, 7.0f, 8.0f, 12.0f, 0.9f);
    drawerIcon.addRoundedRectangle (10.0f, 7.0f, 10.0f, 12.0f, 0.9f);
    menuButton.setIcon (std::move (drawerIcon));

    const auto reveal = [&] (juce::Component& component, juce::Rectangle<int> open)
    {
        animator.cancelAnimation (&component, false);
        component.setBounds (wasOpen ? open : open.translated (tuckedOffset, 0));
        component.setAlpha (1.0f);
        component.setVisible (true);
        component.toFront (false);
        if (! wasOpen)
            animator.animateComponent (&component, open, 1.0f, 200, false, 1.0, 0.0);
    };
    reveal (browserTabs, tabsOpen);
    reveal (body, bodyOpen);
    groups.toFront (false);
}

void MetaTuneEditor::showAutoKey()
{
    showSettingsPanel (false);
    showColorPanel (false);
    showBrowser (false);
    const bool inst = update::metaKeyInstalled();
    const auto st = update::readMetaKeyStatus();
    autoKeyCard.open (inst, inst && st.live && st.valid(), st.root, st.minor);
    autoKeyCard.toFront (false);
}

void MetaTuneEditor::applyAutoKey (int root, bool minor)
{
    proc.undoManager.beginNewTransaction ("Auto Key");
    setRoot (juce::jlimit (0, 11, root));
    selectScale (minor ? metatune::ScaleType::Minor : metatune::ScaleType::Major);
}

void MetaTuneEditor::repaintAll()
{
    // Tooltip / popup colours are baked into the look-and-feel, so refresh them for the current theme.
    lnf.setColour (juce::TooltipWindow::backgroundColourId, theme::bg3);
    lnf.setColour (juce::TooltipWindow::textColourId, theme::text);
    lnf.setColour (juce::PopupMenu::backgroundColourId, theme::menuBg);
    lnf.setColour (juce::PopupMenu::textColourId, theme::text);
    lnf.setColour (juce::PopupMenu::highlightedBackgroundColourId, theme::violet.withAlpha (0.35f));
    lnf.setColour (juce::PopupMenu::highlightedTextColourId, theme::text);
    brushed = {};
    titleTexture = {};
    bypassTitleTexture = {};
    canvas.repaint();
}

void MetaTuneEditor::showColorPanel (bool show)
{
    auto& animator = juce::Desktop::getInstance().getAnimator();
    if (show == colorPanel.isVisible() && ! animator.isAnimating (&colorPanel))
        return;
    const int w = 268, h = colorPanel.preferredHeight();
    const juce::Rectangle<int> open (6, layout::headerH + 2, w, h), tucked = open.translated (0, -12);
    if (show)
    {
        colorPanel.setValues (update::Settings::accentHue(), update::Settings::accentSat(), update::Settings::accentBright());
        colorPanel.setExtras (update::Settings::lightMode(), update::Settings::gradientOn(),
                              update::Settings::gradHueA(), update::Settings::gradHueB(),
                              update::Settings::orbHue(), update::Settings::keyHue(), update::Settings::meterHue());
        showSettingsPanel (false);
        showBrowser (false);
        colorPanel.setBounds (tucked);
        colorPanel.setAlpha (0.0f);
        colorPanel.setVisible (true);
        colorPanel.toFront (false);
        animator.animateComponent (&colorPanel, open, 1.0f, 170, false, 0.0, 0.0);
    }
    else
    {
        animator.animateComponent (&colorPanel, tucked, 0.0f, 130, true, 1.0, 1.0);
        colorPanel.setVisible (false);
    }
}

void MetaTuneEditor::showSettingsPanel (bool show)
{
    auto& animator = juce::Desktop::getInstance().getAnimator();
    if (show == settingsPanel.isVisible() && ! animator.isAnimating (&settingsPanel))
        return;
    if (show)
    {
        using Item = SettingsPanel::Item;
        std::vector<Item> items;
        items.push_back ({ Item::Kind::Header, "Window size" });
        for (int pct : { 60, 75, 85, 100, 125, 150 })
            items.push_back ({ Item::Kind::Choice, juce::String (pct) + " %", {}, pct, std::abs (proc.ui.scale * 100.0f - float (pct)) < 1.0f });
        items.push_back ({ Item::Kind::Header, "Controls" });
        items.push_back ({ Item::Kind::Choice, "Auto Key", {}, 605, false });
        items.push_back ({ Item::Kind::Choice, "Choose key and scale", {}, 607, false });
        items.push_back ({ Item::Kind::Choice, "Reset parameters", {}, 606, false });
        items.push_back ({ Item::Kind::Header, "Presets" });
        items.push_back ({ Item::Kind::Choice, "Keep My Scale", {}, 603, proc.ui.keepScale });
        items.push_back ({ Item::Kind::Header, "Range (lowest note)" });
        const int range = int (state.getRawParameterValue (ParamID::range)->load());
        for (int i = 0; i < 3; ++i)
            items.push_back ({ Item::Kind::Choice, kRangeNames[i], juce::String (proc.latencyMsFor (i), 1) + " ms", 500 + i, i == range });
        if (update::feedUrl().isNotEmpty())
        {
            items.push_back ({ Item::Kind::Header, "Updates" });
            if (pendingUpdate) items.push_back ({ Item::Kind::Choice, "Install v" + pendingUpdate->version, {}, 602, false });
            items.push_back ({ Item::Kind::Choice, "Check now", updateChecker.isChecking() ? "checking..." : updateStatus, 601, false });
            items.push_back ({ Item::Kind::Choice, "Check automatically", {}, 600, update::Settings::autoCheck() });
        }
        items.push_back ({ Item::Kind::Header, {} });
        items.push_back ({ Item::Kind::Choice, "Release notes", {}, 604, false });
        items.push_back ({ Item::Kind::Info, juce::String (JucePlugin_Name) + " " + JucePlugin_VersionString });
        settingsPanel.setItems (std::move (items));
    }
    const juce::Rectangle<int> open (54, layout::headerH + 2, 240, settingsPanel.preferredHeight()), tucked = open.translated (0, -12);
    if (show)
    {
        showBrowser (false);
        settingsPanel.setBounds (tucked);
        settingsPanel.setAlpha (0.0f);
        settingsPanel.setVisible (true);
        settingsPanel.toFront (false);
        animator.animateComponent (&settingsPanel, open, 1.0f, 170, false, 0.0, 0.0);
    }
    else
    {
        animator.animateComponent (&settingsPanel, tucked, 0.0f, 130, true, 1.0, 1.0);
        settingsPanel.setVisible (false);
    }
}

void MetaTuneEditor::startUpdateCheck()
{
    updateStatus = {};
    updateChecker.start ([safe = juce::Component::SafePointer<MetaTuneEditor> (this)] (std::optional<update::Release> r) {
        if (safe == nullptr) return;
        safe->updateStatus = r ? "v" + r->version + " out" : "up to date";
        safe->showUpdate (r);
    });
}

void MetaTuneEditor::showUpdate (std::optional<update::Release> r)
{
    pendingUpdate = r;
    if (r) updatePill.setVersion (r->version);
    updatePill.setTooltip (r ? "MetaTune " + r->version + " is available. Click to update: FL Studio closes (it asks to save first), "
                               "the update installs, then you can reopen it." : juce::String());
    updatePill.setVisible (r.has_value());
}

void MetaTuneEditor::installUpdate()
{
    if (pendingUpdate && ! update::launchUpdater (*pendingUpdate))
        updateStatus = "updater missing - reinstall";
}

void MetaTuneEditor::showPresetPanel (bool show)
{
    showBrowser (show, show ? 0 : -1);
}

void MetaTuneEditor::stepPreset (int delta)
{
    const auto names = proc.presetNames();
    if (names.isEmpty()) return;
    const int cur = names.indexOf (proc.ui.presetName);
    const int next = cur < 0 ? (delta > 0 ? 0 : names.size() - 1) : (cur + delta + names.size()) % names.size();
    proc.loadPreset (next);
}

void MetaTuneEditor::savePresetDialog()
{
    showPresetPanel (false);
    pendingSave = PendingSave::Preset;
    pendingFolder = {};
    saveCard.open (proc.ui.presetName);
}

void MetaTuneEditor::refreshPresetPanel()
{
    const auto factoryFolder = [] (const juce::String& name)
    {
        if (name == "Hard Tune" || name == "Trap Lead") return "01. Hip-Hop";
        if (name == "Natural Pop" || name == "Wide Doubles") return "02. Pop & Electronic";
        if (name == "Smooth Ballad" || name == "Subtle Polish") return "03. Natural Tuning";
        return "04. Signature";
    };
    std::vector<PresetItem> items;
    for (const auto& entry : proc.presetList())
        items.push_back ({ entry.name, entry.factory ? juce::String (factoryFolder (entry.name)) : entry.folder,
                           entry.factory, update::Settings::isFavorite (entry.folder, entry.name) });
    presetPanel.setItems (items, proc.presetFolders(), proc.ui.presetName);
    syncModeButtons();
}

void MetaTuneEditor::refreshGroupBrowser()
{
    juce::StringArray names;
    for (int g = 0; g < 4; ++g) names.add (MetaTuneProcessor::groupName (g));
    std::vector<GroupInstanceRow> rows;
    for (const auto& inf : proc.groupInstances())
        rows.push_back ({ inf.id, inf.group,
                          inf.label.isNotEmpty() ? inf.label : "Instance " + juce::String (inf.id),
                          inf.id == proc.instanceId });
    groupBrowser.setData (names, rows);
}

void MetaTuneEditor::showNotesCard()
{
    update::Settings::setNotesVersion (JucePlugin_VersionString);
    showBrowser (false);
    showSettingsPanel (false);
    notesCard.setVisible (false); // keep the overlay out of its own backdrop snapshot
    auto snap = canvas.createComponentSnapshot (canvas.getLocalBounds());
    if (snap.isValid())
    {
        auto small = snap.rescaled (juce::jmax (1, snap.getWidth() / 3), juce::jmax (1, snap.getHeight() / 3));
        juce::ImageConvolutionKernel blur (9);
        blur.createGaussianBlur (3.0f);
        blur.applyToImage (small, small, small.getBounds());
        notesCard.open (small);
    }
    else
        notesCard.open ({});
    notesCard.toFront (false);
}

void MetaTuneEditor::showGroupBrowser (bool show)
{
    showBrowser (show, show ? 1 : -1);
}

void MetaTuneEditor::setScale (float s)
{
    proc.ui.scale = s;
    setSize (juce::roundToInt (kW * s), heightForWidth (kW * s));
}
