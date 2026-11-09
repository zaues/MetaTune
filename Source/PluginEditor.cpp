#include "PluginEditor.h"
#include "Parameters.h"
#include <map>

namespace layout
{
// Design-canvas coordinates (1082 x 856).
constexpr int headerH = 52;
constexpr int rowY = 532, rowH = 48;          // mode row
constexpr int bottomY = rowY + rowH;          // heat rows + keyboard
constexpr int leftW = 130, rightX = 950;      // bottom side columns
const juce::Point<float> speedC { 845, 195 }, sustainC { 982, 232 }, amountC { 948, 382 };
constexpr float speedR = 100, sustainR = 58, amountR = 92;
constexpr float speedBand = 0.69f;            // Speed knob: proportion where the aggressive (< 0 ms) band starts
} // namespace layout

static juce::String formatSpeed (double v) { return std::abs (v) < 100.0 ? juce::String (v, 1) : juce::String (juce::roundToInt (v)); }

static juce::Rectangle<int> around (juce::Point<float> c, float r) { return juce::Rectangle<float> (2 * r, 2 * r).withCentre (c).toNearestInt(); }

MetaTuneEditor::MetaTuneEditor (MetaTuneProcessor& p)
    : AudioProcessorEditor (p), proc (p), state (p.apvts),
      speedKnob ("Speed", formatSpeed, { -135.0f, 60.0f, true, 3, layout::speedBand, "ms", 0.08f }),
      sustainKnob ("Sustain", [] (double v) { return juce::String (juce::roundToInt (v)); }, { -135.0f, 135.0f, false, 4, -1.0f, "%", 0.0f, true, true, true }),
      amountKnob ("Amount", [] (double v) { return juce::String (juce::roundToInt (v)); }, { -140.0f, 62.0f, true, 3, -1.0f, "%" }),
      orb (*state.getParameter (ParamID::doublerWidth), *state.getParameter (ParamID::doublerMix), "Doubler\nWidth", "Doubler\nDry / Wet", &p.undoManager),
      stabilizerSeg (*state.getParameter (ParamID::stabilizer), kStabilizerNames, &p.undoManager)
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
    addAndMakeVisible (canvas);

    for (auto* c : std::initializer_list<juce::Component*> { &orb, &speedKnob, &sustainKnob, &amountKnob, &keyboard, &refStrip, &toneSlider,
                                                            &stabilizerSeg, &groups, &presetBar, &notePill, &majorPill, &minorPill, &chromaticPill,
                                                            &scalePill, &lowLatency, &colorButton, &menuButton, &saveButton, &undoButton, &redoButton,
                                                            &resetButton, &aButton, &copyButton, &bButton, &helpButton, &heatButton,
                                                            &detectedButton, &bypassButton, &chromaticButton })
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
    if (update::Settings::notesVersion() != JucePlugin_VersionString)
        showNotesPending = true; // a version change (install/update): show "What's New" once the UI is laid out
    saveCard.onSave = [this] (const juce::String& n) {
        if (pendingSave == PendingSave::GroupName)
        {
            MetaTuneProcessor::setGroupName (pendingGroup, n.trim());
            saveCard.setVisible (false);
            return;
        }
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
    presetPanel.onHeightChanged = [this] {
        using namespace layout;
        const int x = 54, y = headerH + 2, w = 288, th = 26;
        const int maxH = rowY - (y + th) - 10;
        if (presetPanel.isVisible())
            presetPanel.setBounds (x, y + th, w, juce::jmin (presetPanel.preferredHeight(), maxH));
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
    presetPanel.onMouseLeave = nullptr; // the tabbed browser closes on a click outside, not on mouse-leave
    canvas.addMouseListener (this, true);
    rootPicker.onRoot = [this] (int pc) { pickerPinned = false; setRoot (pc); };
    rootPicker.onScale = [this] (int i) { pickerPinned = false; applyPickedScale (i); };
    rootPicker.onFlats = [this] (bool f) { proc.ui.useFlats = f; canvas.repaint(); };
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
    bypassButton.onColour = theme::pink;
    bypassAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (state, ParamID::bypass, bypassButton);
    speedKnob.setDoubleClickReturnValue (true, 20.0);
    sustainKnob.setDoubleClickReturnValue (true, 0.0);
    amountKnob.setDoubleClickReturnValue (true, 100.0);
    refStrip.setDoubleClickReturnValue (true, 440.0);
    toneSlider.setDoubleClickReturnValue (true, -18.0);

    keyboard.onToggleNote = [this] (int pc) { keyClicked (pc); };
    keyboard.onSetRoot = [this] (int pc) { setRoot (pc); };
    // Holding a key plays the reference tone (C4..B4) at Tone Volume.
    keyboard.onPress = [this] (int pc) { proc.toneChord = toneChordMode; proc.toneNote = 60 + pc; };
    keyboard.onRelease = [this] { proc.toneNote = -1; };

    undoButton.onClick = [this] { proc.undoManager.undo(); };
    redoButton.onClick = [this] { proc.undoManager.redo(); };
    resetButton.onClick = [this] { proc.resetAllParameters(); };
    aButton.onClick = [this] { if (proc.getABSlot() != 0) proc.switchAB(); };
    bButton.onClick = [this] { if (proc.getABSlot() != 1) proc.switchAB(); };
    copyButton.onClick = [this] { proc.copyToOtherAB(); };
    menuButton.onClick = [this] { showSettingsPanel (! settingsPanel.isVisible()); };
    colorButton.onClick = [this] { showColorPanel (! colorPanel.isVisible()); };
    colorButton.iconScale = 0.5f; // a touch smaller so the palette icon has margin inside its tile
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
    // The header preset box opens a presets-only list (no Groups tab); the grid/box icon opens the tabbed browser.
    presetBar.onClick = [this] { showPresetPanel (browserTabs.isVisible() || ! presetPanel.isVisible()); };
    presetBar.onPrev = [this] { stepPreset (-1); };
    presetBar.onNext = [this] { stepPreset (+1); };

    notePill.onClick = [this] { toneChordMode = 0; selectNoteMode(); };
    majorPill.onClick = [this] { toneChordMode = 1; selectKeyScale (metatune::ScaleType::Major); };
    minorPill.onClick = [this] { toneChordMode = 2; selectKeyScale (metatune::ScaleType::Minor); };
    chromaticPill.onClick = [this] { toneChordMode = 0; selectScale (metatune::ScaleType::Chromatic); };
    scalePill.onClick = [this] { pickerPinned = true; };
    chromaticButton.onClick = [this] { selectScale (metatune::ScaleType::Chromatic); };
    lowLatency.onClick = [this] { proc.undoManager.beginNewTransaction ("Low Latency"); setParam (ParamID::range, lowLatency.getToggleState() ? 2.0f : 0.0f); };

    groups.setSelected (proc.ui.group);
    groups.onSelect = [this] (int g) { proc.joinGroup (g); groups.setSelected (proc.getGroup()); };
    groups.onBrowse = [this] { showBrowser (! browserTabs.isVisible()); };
    groupBrowser.onAssign = [this] (int id, int grp) { MetaTuneProcessor::setInstanceGroup (id, grp); refreshGroupBrowser(); };
    groupBrowser.onRename = [this] (int grp) { showBrowser (false); pendingSave = PendingSave::GroupName; pendingGroup = grp; saveCard.open (MetaTuneProcessor::groupName (grp)); };
    groupBrowser.onMouseLeave = nullptr;

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
    chromaticPill.setTooltip ("Chromatic scale: all notes allowed.");
    scalePill.setTooltip ("Pick a scale and root.");
    stabilizerSeg.setTooltip ("Note Transition: how long a new note must hold before the tuner moves to it (Low 40 ms, Mid 80 ms, High 200 ms). Stops quick passing notes from being snapped.");
    refStrip.setTooltip ("Reference tuning for A4. Drag, Shift-drag for fine, double-click for 440 Hz.");
    toneSlider.setTooltip ("Volume of the tone generator: hold a key on the keyboard to hear that note.");
    groups.setTooltip ("Groups: copies of the plugin in the same group share their settings. The small number shows how many copies are in each group. Click your group again to leave it.");
    presetBar.setTooltip ("Presets: click for the list, arrows step through them. Presets keep your key and scale.");
    chromaticButton.setTooltip ("Switch to the chromatic scale (all notes allowed).");
    undoButton.setTooltip ("Undo");
    redoButton.setTooltip ("Redo");
    resetButton.setTooltip ("Reset all parameters to defaults (undoable)");
    aButton.setTooltip ("Snapshot A");
    bButton.setTooltip ("Snapshot B");
    copyButton.setTooltip ("Copy the current snapshot to the other one");
    helpButton.setTooltip ("Show/hide tooltips");
    heatButton.setTooltip ("Show/hide the input and output overlays");
    detectedButton.setTooltip ("Show/hide detected input note and output target on the keyboard");
    bypassButton.setTooltip ("Bypass all processing (latency stays compensated)");
    menuButton.setTooltip ("Settings");
    saveButton.setTooltip ("Save the current settings as a preset");

    syncModeButtons();
    updateTooltips();
    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio (double (kW) / kH);
    setResizeLimits (kW * 6 / 10, kH * 6 / 10, kW * 3 / 2, kH * 3 / 2);
    setSize (juce::roundToInt (kW * proc.ui.scale), juce::roundToInt (kH * proc.ui.scale));
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
    colorButton.setBounds (7, 7, 38, 38);
    menuButton.setBounds (60, 6, 46, 40);
    presetBar.setBounds (113, 0, 229, headerH);
    saveButton.setBounds (346, 6, 50, 40);
    undoButton.setBounds (776, 8, 40, 36);
    redoButton.setBounds (820, 8, 40, 36);
    resetButton.setBounds (864, 8, 40, 36);
    aButton.setBounds (910, 6, 36, 40);
    copyButton.setBounds (948, 8, 34, 36);
    bButton.setBounds (984, 6, 36, 40);
    helpButton.setBounds (1032, 6, 44, 40);
    updatePill.setBounds (650, 11, 116, 30);

    // Stage.
    groups.setBounds (0, 122, 52, 340);
    orb.setBounds (90, 62, 480, 460);
    speedKnob.setBounds (around (speedC, speedR));
    sustainKnob.setBounds (around (sustainC, sustainR));
    amountKnob.setBounds (around (amountC, amountR));
    speedKnob.setCutout (sustainC, sustainR);

    // Mode row.
    notePill.setBounds (133, rowY + 8, 84, 32);
    majorPill.setBounds (237, rowY + 8, 62, 32);
    minorPill.setBounds (299, rowY + 8, 58, 32);
    chromaticPill.setBounds (357, rowY + 8, 90, 32);
    scalePill.setBounds (447, rowY + 8, 62, 32);
    stabilizerSeg.setBounds (545, rowY + 7, 275, 34);
    lowLatency.setBounds (962, rowY + 13, 40, 22);

    // Bottom.
    keyboard.setBounds (leftW, bottomY, rightX - leftW, kH - bottomY);
    toneSlider.setBounds (8, 650, 116, 24);
    rootArea = { 0, 690, leftW, 132 };
    chromaticButton.setBounds (0, 826, leftW, 30);
    heatButton.setBounds (rightX + 2, 582, 130, 42);
    detectedButton.setBounds (rightX + 2, 626, 130, 62);
    bypassButton.setBounds (rightX + 2, 691, 130, 68);
    refStrip.setBounds (rightX + 8, 796, 118, 50);
    // (the picker's bounds are driven every frame by updatePicker)
}

void MetaTuneEditor::Canvas::paint (juce::Graphics& g)
{
    using namespace theme;
    using namespace layout;
    auto& e = owner;
    g.fillAll (bg1);

    // Header bar with darker cells.
    g.setColour (header);
    g.fillRect (0, 0, kW, headerH);
    g.setColour (headerBox);
    g.fillRect (342, 0, 58, headerH);
    {
        // Tile behind the colour (palette) button.
        const auto tile = juce::Rectangle<float> (6, 7, 40, 38);
        g.setGradientFill (lightMode
            ? juce::ColourGradient (bg3.brighter (0.25f), tile.getX(), tile.getY(), bg3.darker (0.06f), tile.getX(), tile.getBottom(), false)
            : juce::ColourGradient (juce::Colour (0xff4a4a4e), tile.getX(), tile.getY(), juce::Colour (0xff2c2c2f), tile.getX(), tile.getBottom(), false));
        g.fillRoundedRectangle (tile, 5.0f);
    }
    {
        // Title cell: dark nebula behind the wordmark (a light plate in light mode so the wordmark reads).
        const auto t = juce::Rectangle<float> (400, 0, 370, float (headerH));
        g.setColour (lightMode ? headerBox : juce::Colour (0xff1b1a1f));
        g.fillRect (t);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0x553d2f6b), t.getCentreX() - 60, t.getCentreY(), juce::Colour (0x003d2f6b), t.getRight(), t.getCentreY(), true));
        g.fillRect (t);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0x302a4a6b), t.getCentreX() + 90, t.getY(), juce::Colour (0x002a4a6b), t.getCentreX() + 90, t.getBottom() + 30, true));
        g.fillRect (t);
        juce::AttributedString shadow;
        shadow.append (wordmarkA, font (22.0f), juce::Colours::black.withAlpha (0.5f));
        shadow.append (wordmarkB, font (22.0f, true), juce::Colours::black.withAlpha (0.5f));
        shadow.setJustification (juce::Justification::centred);
        shadow.draw (g, t.translated (0.0f, 1.5f));
        juce::AttributedString word;
        word.append (wordmarkA, font (22.0f), text.withAlpha (0.85f));
        word.append (wordmarkB, font (22.0f, true), text);
        word.setJustification (juce::Justification::centred);
        word.draw (g, t);
    }
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    for (int x : { 52, 108, 113, 342, 400, 770, 906, 1028 })
        g.fillRect (x, 0, x == 108 ? 5 : 2, headerH);
    g.fillRect (0, headerH, kW, 2);

    // Stage: obsidian with soft radial highlights; brushed-metal panel on the right with a curved edge.
    {
        const juce::Rectangle<float> st (0.0f, float (headerH + 2), float (kW), float (rowY - headerH - 2));
        g.setColour (bg1);
        g.fillRect (st);
        auto glowAt = [&] (float x, float y, float r, float alpha) {
            g.setGradientFill (juce::ColourGradient (highlight.brighter (0.25f).withAlpha (alpha), x, y, highlight.withAlpha (0.0f), x + r, y, true));
            g.fillEllipse (x - r, y - r, 2 * r, 2 * r);
        };
        glowAt (330.0f, 290.0f, 330.0f, 0.9f);
        glowAt (40.0f, float (rowY), 260.0f, 0.6f);
    }
    {
        juce::Path panelPath;
        panelPath.startNewSubPath (585.0f, float (headerH));
        panelPath.cubicTo (638.0f, 52.0f, 664.0f, 92.0f, 680.0f, 132.0f);
        panelPath.lineTo (792.0f, 412.0f);
        panelPath.cubicTo (812.0f, 462.0f, 824.0f, 502.0f, 838.0f, float (rowY));
        panelPath.lineTo (float (kW), float (rowY));
        panelPath.lineTo (float (kW), float (headerH));
        panelPath.closeSubPath();
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (0, headerH + 2, kW, rowY - headerH - 2);
        g.reduceClipRegion (panelPath);
        if (e.brushed.isNull())
        {
            // Fine horizontal streaks over a dark gradient: brushed metal.
            e.brushed = juce::Image (juce::Image::ARGB, kW, rowY, true);
            juce::Graphics bg (e.brushed);
            bg.setGradientFill (juce::ColourGradient (panel.brighter (0.18f), 1000.0f, 70.0f, panel.darker (0.22f), 700.0f, float (rowY), true));
            bg.fillAll();
            juce::Random rng (3);
            for (int i = 0; i < 2600; ++i)
            {
                const float y = rng.nextFloat() * float (rowY), x = rng.nextFloat() * float (kW), len = 40.0f + rng.nextFloat() * 260.0f;
                bg.setColour ((rng.nextBool() ? juce::Colours::white : juce::Colours::black).withAlpha (0.012f + 0.03f * rng.nextFloat()));
                bg.fillRect (x, y, len, 1.0f);
            }
        }
        g.drawImageAt (e.brushed, 0, 0);
        // Diagonal shade line across the upper right.
        juce::Path shade;
        shade.startNewSubPath (760.0f, float (headerH));
        shade.lineTo (900.0f, float (headerH));
        shade.lineTo (float (kW), 150.0f);
        shade.lineTo (float (kW), 290.0f);
        shade.closeSubPath();
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.045f), 830.0f, float (headerH), juce::Colours::transparentWhite, 1000.0f, 260.0f, false));
        g.fillPath (shade);
        g.setColour (juce::Colours::white.withAlpha (0.07f));
        g.drawLine (900.0f, float (headerH), float (kW), 150.0f, 1.0f);
        // Soft shading where the knobs sit.
        for (auto [c, r] : { std::pair { speedC, speedR }, std::pair { amountC, amountR } })
        {
            g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.4f), c.x, c.y + 10, juce::Colours::transparentBlack, c.x, c.y + r * 1.35f, true));
            g.fillEllipse (c.x - r * 1.35f, c.y - r * 1.25f, r * 2.7f, r * 2.7f);
        }
    }
    {
        juce::Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (0, headerH + 2, kW, rowY - headerH - 2);
        juce::Path edge;
        edge.startNewSubPath (585.0f, float (headerH));
        edge.cubicTo (638.0f, 52.0f, 664.0f, 92.0f, 680.0f, 132.0f);
        edge.lineTo (792.0f, 412.0f);
        edge.cubicTo (812.0f, 462.0f, 824.0f, 502.0f, 838.0f, float (rowY));
        g.setColour (line.brighter (0.2f));
        g.strokePath (edge, juce::PathStrokeType (1.0f));
    }

    // Mode row.
    g.setColour (bg2);
    g.fillRect (0, rowY, kW, rowH);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRect (0, rowY, kW, 1);
    g.fillRect (0, rowY + rowH - 1, kW, 1);
    g.setFont (font (13.0f, true));
    shadowText (g, "Select", juce::Rectangle<float> (24.0f, float (rowY), 60.0f, float (rowH)), juce::Justification::centredLeft, textDim);
    g.setColour (textDim);
    g.fillRect (96, rowY + rowH / 2, 20, 1);
    g.setColour (bg3);
    g.fillRoundedRectangle (233.0f, float (rowY + 8), 278.0f, 32.0f, 16.0f);
    {
        const auto box = juce::Rectangle<float> (540.0f, float (rowY + 5), 406.0f, 38.0f);
        g.setColour (bg0);
        g.fillRoundedRectangle (box, 19.0f);
        g.setColour (juce::Colours::white.withAlpha (0.07f));
        g.drawRoundedRectangle (box.reduced (0.5f), 19.0f, 1.0f);
        const auto lbl = juce::Rectangle<float> (822.0f, float (rowY + 9), 120.0f, 30.0f);
        g.setColour (bg3);
        g.fillRoundedRectangle (lbl, 15.0f);
        g.setFont (font (14.0f, true));
        shadowText (g, "Note Transition", lbl, juce::Justification::centred, text.withAlpha (0.9f));
    }
    g.setFont (font (13.0f, true));
    shadowFittedText (g, "Low\nLatency", juce::Rectangle<int> (1010, rowY + 4, 68, rowH - 8), juce::Justification::centredLeft, 2, e.lowLatency.getToggleState() ? text : greyText);

    // Bottom side columns.
    g.setColour (bg2);
    g.fillRect (0, bottomY, leftW, kH - bottomY);
    g.fillRect (rightX, bottomY, kW - rightX, kH - bottomY);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRect (leftW - 1, bottomY, 1, kH - bottomY);
    g.fillRect (rightX, bottomY, 1, kH - bottomY);
    for (int y : { 624, 689 })
        g.fillRect (0, y, leftW, 1);
    for (int y : { 624, 689, 760 })
        g.fillRect (rightX, y, kW - rightX, 1);

    g.setFont (font (12.5f, true));
    if (e.proc.ui.heatmaps)
    {
        shadowText (g, "Input", juce::Rectangle<float> (24.0f, float (bottomY + e.keyboard.heatInputY()), 60.0f, float (KeyboardPanel::kHeatH)), juce::Justification::centredLeft, textDim);
        shadowText (g, "Output", juce::Rectangle<float> (24.0f, float (bottomY + e.keyboard.heatOutputY()), 60.0f, float (KeyboardPanel::kHeatH)), juce::Justification::centredLeft, textDim);
        g.setColour (textDim);
        g.fillRect (96, bottomY + e.keyboard.heatInputY() + KeyboardPanel::kHeatH / 2, 20, 1);
        g.fillRect (96, bottomY + e.keyboard.heatOutputY() + KeyboardPanel::kHeatH / 2, 20, 1);
    }
    shadowText (g, "Tone Volume", juce::Rectangle<float> (0.0f, 630.0f, float (leftW), 18.0f), juce::Justification::centred, text.withAlpha (0.9f));
    shadowText (g, "Pitch Reference", juce::Rectangle<float> (float (rightX), 772.0f, float (kW - rightX), 18.0f), juce::Justification::centred, text.withAlpha (0.9f));

    const int root = juce::jlimit (0, 11, int (e.state.getRawParameterValue (ParamID::root)->load()));
    static const char* flatNames[] { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
    const juce::String name = e.proc.ui.useFlats ? flatNames[root] : kNoteNames[root];
    g.setColour (text);
    if (! e.proc.ui.scaleEverSet)
    {
        // First run only (no key ever picked): a mouse cursor and a prompt instead of a key.
        const auto area = e.rootArea.withHeight (100).toFloat();
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
        const auto curBox = juce::Rectangle<float> (31.0f, 39.0f).withCentre ({ area.getCentreX(), float (e.rootArea.getY()) + 28.0f });
        const auto curXform = cur.getTransformToScaleToFit (curBox, true);
        g.setColour (juce::Colours::black.withAlpha (0.5f)); // shadow
        g.fillPath (cur, curXform.followedBy (juce::AffineTransform::translation (0.0f, 1.6f)));
        g.setColour (promptCol);
        g.fillPath (cur, curXform);
        g.setFont (font (18.0f, true));
        shadowFittedText (g, "Hover to\nset scale", e.rootArea.withTrimmedTop (66).withHeight (56), juce::Justification::centredTop, 2, promptCol);
    }
    else
    {
        // Letter large; an accidental sits raised and smaller to its right.
        auto makeLayout = [&] (juce::Colour col) {
            juce::AttributedString as;
            as.append (name.substring (0, 1), font (80.0f), col);
            if (name.length() > 1) as.append (name.substring (1), font (40.0f), col);
            juce::TextLayout tl; tl.createLayout (as, 200.0f); return tl;
        };
        auto tl = makeLayout (text);
        const auto area = e.rootArea.withHeight (100).toFloat();
        const auto target = juce::Rectangle<float> (tl.getWidth(), tl.getHeight()).withCentre (area.getCentre());
        makeLayout (juce::Colours::black.withAlpha (0.5f)).draw (g, target.translated (0.0f, 2.0f)); // shadow
        tl.draw (g, target);
        g.setFont (font (19.0f, true));
        shadowText (g, e.scaleDisplayName(), e.rootArea.withTrimmedTop (98).withHeight (26).toFloat(), juce::Justification::centred, text);
    }

    // Chromatic tab.
    juce::Path tab;
    tab.addRoundedRectangle (0.0f, 826.0f, float (leftW), 30.0f, 12.0f, 12.0f, false, true, false, false);
    g.setColour (bg3);
    g.fillPath (tab);
}

void MetaTuneEditor::syncModeButtons()
{
    const auto scale = metatune::ScaleType (juce::jlimit (0, 3, int (state.getRawParameterValue (ParamID::scale)->load())));
    notePill.setToggleState (scale == metatune::ScaleType::Custom && proc.ui.scaleLabel.isEmpty(), juce::dontSendNotification);
    majorPill.setToggleState (scale == metatune::ScaleType::Major, juce::dontSendNotification);
    minorPill.setToggleState (scale == metatune::ScaleType::Minor, juce::dontSendNotification);
    chromaticPill.setToggleState (scale == metatune::ScaleType::Chromatic, juce::dontSendNotification);
    scalePill.setToggleState (scale == metatune::ScaleType::Custom && proc.ui.scaleLabel.isNotEmpty(), juce::dontSendNotification);
    lowLatency.setToggleState (int (state.getRawParameterValue (ParamID::range)->load()) == 2, juce::dontSendNotification);
}

void MetaTuneEditor::timerCallback()
{
    // Drain DSP telemetry: every sample feeds the HeatMaps, the newest drives the live displays.
    const double now = juce::Time::getMillisecondCounterHiRes();

    // Once, shortly after opening: if the host resized us away from the saved size, put it back.
    if (! sizeRestored && now - openedMs > 250.0)
    {
        sizeRestored = true;
        if (std::abs (getWidth() - juce::roundToInt (kW * openScale)) > 2)
            setSize (juce::roundToInt (kW * openScale), juce::roundToInt (kH * openScale));
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
        if (t.inputMidi > 0 && t.confidence >= 0.7f)
        {
            const float out = bypassed ? t.inputMidi : t.outputMidi;
            keyboard.addHeat (t.inputMidi, out, t.confidence * juce::jmin (1.0f, t.rms * 8.0f) * 0.35f);
            display.inputMidi = t.inputMidi;
            display.outputMidi = out;
            display.target = bypassed ? -1 : t.target;
            lastVoicedMs = now;
        }
    }
    keyboard.decayHeat (0.93f);
    display.voiced = now - lastVoicedMs < 80.0;
    display.presence += display.voiced ? (1.0f - display.presence) * 0.35f : -display.presence * 0.10f;
    const float lvl = juce::jlimit (0.0f, 1.0f, (juce::Decibels::gainToDecibels (latestRms, -100.0f) + 50.0f) / 40.0f);
    display.level += (lvl - display.level) * 0.25f;

    orb.setDisplay (display, bypassed);
    speedKnob.setActivity (bypassed ? 0.0f : display.presence * (0.35f + 0.65f * display.level));
    keyboard.setDisplay (display);
    keyboard.setMask (proc.currentMask(), juce::jlimit (0, 11, int (state.getRawParameterValue (ParamID::root)->load())));
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
    syncModeButtons();
    if (rootPicker.isVisible())
        syncRootPickerState();

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
    canvas.repaint (1008, layout::rowY, 74, layout::rowH); // Low Latency caption colour
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

void MetaTuneEditor::selectKeyScale (metatune::ScaleType type)
{
    // Apply the major/minor scale's notes on the current root, then switch to Note mode so the
    // individual notes can be edited (matches the guide: the selector returns to Note after a key pick).
    const int root = juce::jlimit (0, 11, int (state.getRawParameterValue (ParamID::root)->load()));
    const auto mask = metatune::scaleMask (type, root, 0);
    proc.ui.scaleLabel = {};
    proc.undoManager.beginNewTransaction ("Select key");
    for (int i = 0; i < 12; ++i)
        setParam (ParamID::note (i), metatune::isAllowed (mask, i) ? 1.0f : 0.0f);
    setParam (ParamID::scale, float (metatune::ScaleType::Custom));
}

void MetaTuneEditor::selectNoteMode()
{
    // Note mode = pick notes by hand: start a custom scale from what is currently audible.
    const auto scale = metatune::ScaleType (int (state.getRawParameterValue (ParamID::scale)->load()));
    if (scale == metatune::ScaleType::Major || scale == metatune::ScaleType::Minor)
    {
        const auto mask = proc.currentMask();
        proc.ui.scaleLabel = {};
        proc.undoManager.beginNewTransaction ("Note mode");
        for (int i = 0; i < 12; ++i)
            setParam (ParamID::note (i), metatune::isAllowed (mask, i) ? 1.0f : 0.0f);
        setParam (ParamID::scale, float (metatune::ScaleType::Custom));
    }
}

void MetaTuneEditor::keyClicked (int pc)
{
    if (proc.ui.majorHold || proc.ui.minorHold)
    {
        proc.undoManager.beginNewTransaction ("Set key");
        proc.ui.scaleLabel = {};
        setParam (ParamID::scale, float (proc.ui.majorHold ? metatune::ScaleType::Major : metatune::ScaleType::Minor));
        setParam (ParamID::root, float (pc));
        return;
    }
    toggleNote (pc);
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
    if (sc == metatune::ScaleType::Chromatic) idx = RootPicker::scaleNames().indexOf ("Chromatic");
    if (sc == metatune::ScaleType::Custom && proc.ui.scaleLabel.isEmpty()) idx = RootPicker::scaleNames().indexOf ("Custom");
    rootPicker.setState (int (state.getRawParameterValue (ParamID::root)->load()), idx, proc.ui.useFlats);
}

// Frame-driven scale picker: eased entirely here off the 60 Hz timer so the slide is identical and
// reliable every time (no ComponentAnimator proxy/visibility races, no dependence on mouse enter/exit).
void MetaTuneEditor::updatePicker()
{
    using namespace layout;
    const juce::Rectangle<int> area (0, bottomY, leftW - 1, 826 - bottomY);
    const int offX = -area.getWidth(); // fully tucked off the left edge at slide 0

    // Target: open while the pointer is over the root cell or the (visible) panel, or while pinned.
    const auto screen = juce::Desktop::getInstance().getMainMouseSource().getScreenPosition();
    const auto mp = canvas.getLocalPoint (nullptr, screen).roundToInt();
    const bool over = rootArea.contains (mp) || (rootPicker.isVisible() && rootPicker.getBounds().contains (mp));
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
        rootPicker.setBounds (area.withX (juce::roundToInt (float (offX) * (1.0f - pickerSlide))));
}

void MetaTuneEditor::mouseDown (const juce::MouseEvent& e)
{
    if (browserTabs.isVisible() && e.eventComponent != &presetPanel && e.eventComponent != &groupBrowser
        && e.eventComponent != &browserTabs && e.eventComponent != &presetBar && e.eventComponent != &groups
        && ! presetPanel.isParentOf (e.eventComponent) && ! groupBrowser.isParentOf (e.eventComponent))
        showBrowser (false);
    // Standalone presets-only panel (opened from the header preset box): close it on an outside click.
    if (presetPanel.isVisible() && ! browserTabs.isVisible()
        && e.eventComponent != &presetPanel && e.eventComponent != &presetBar
        && ! presetPanel.isParentOf (e.eventComponent))
        showPresetPanel (false);
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

void MetaTuneEditor::applyPickedScale (int index)
{
    const auto name = RootPicker::scaleNames()[index];
    if (name == "Major") { selectScale (metatune::ScaleType::Major); return; }
    if (name == "Minor") { selectScale (metatune::ScaleType::Minor); return; }
    if (name == "Custom") { selectNoteMode(); return; }
    if (name == "Chromatic") { selectScale (metatune::ScaleType::Chromatic); return; }
    // Other scales become a Custom note set on the current root.
    static const std::map<juce::String, std::vector<int>> intervals {
        { "Pent Maj", { 0, 2, 4, 7, 9 } }, { "Pent Min", { 0, 3, 5, 7, 10 } },
        { "Harm Maj", { 0, 2, 4, 5, 7, 8, 11 } }, { "Harm Min", { 0, 2, 3, 5, 7, 8, 11 } } };
    const auto it = intervals.find (name);
    if (it == intervals.end()) return;
    const int root = int (state.getRawParameterValue (ParamID::root)->load());
    bool on[12] {};
    for (int iv : it->second) on[(root + iv) % 12] = true;
    proc.undoManager.beginNewTransaction ("Scale");
    for (int i = 0; i < 12; ++i)
        setParam (ParamID::note (i), on[i] ? 1.0f : 0.0f);
    setParam (ParamID::scale, float (metatune::ScaleType::Custom));
    proc.ui.scaleLabel = name;
}

void MetaTuneEditor::showBrowser (bool show, int tab)
{
    using namespace layout;
    auto& animator = juce::Desktop::getInstance().getAnimator();
    if (tab >= 0) browserTabs.setTab (tab);
    const int x = 54, y = headerH + 2, w = 288, th = 26;

    if (! show)
    {
        if (browserTabs.isVisible())
        {
            animator.animateComponent (&browserTabs, browserTabs.getBounds().translated (-22, 0), 0.0f, 130, true, 1.0, 1.0);
            if (presetPanel.isVisible()) animator.animateComponent (&presetPanel, presetPanel.getBounds().translated (-22, 0), 0.0f, 130, true, 1.0, 1.0);
            if (groupBrowser.isVisible()) animator.animateComponent (&groupBrowser, groupBrowser.getBounds().translated (-22, 0), 0.0f, 130, true, 1.0, 1.0);
        }
        browserTabs.setVisible (false);
        presetPanel.setVisible (false);
        groupBrowser.setVisible (false);
        groups.setBrowseActive (false);
        return;
    }
    showSettingsPanel (false);
    showColorPanel (false);
    groups.setBrowseActive (true);

    const bool wasOpen = browserTabs.isVisible();
    const juce::Rectangle<int> tabsOpen (x, y, w, th);
    if (! wasOpen)
    {
        browserTabs.setBounds (tabsOpen.translated (-28, 0));
        browserTabs.setAlpha (0.0f);
        browserTabs.setVisible (true);
        browserTabs.toFront (false);
        animator.animateComponent (&browserTabs, tabsOpen, 1.0f, 190, false, 0.0, 0.0);
    }
    else
    {
        animator.cancelAnimation (&browserTabs, false);
        browserTabs.setBounds (tabsOpen);
        browserTabs.setAlpha (1.0f);
        browserTabs.setVisible (true);
        browserTabs.toFront (false);
    }

    const int maxH = rowY - (y + th) - 10;
    auto slideIn = [&] (juce::Component& c, int ph)
    {
        const juce::Rectangle<int> open (x, y + th, w, juce::jmin (ph, maxH));
        animator.cancelAnimation (&c, false);
        c.setBounds (open.translated (-28, 0));
        c.setAlpha (0.0f);
        c.setVisible (true);
        c.toFront (false);
        animator.animateComponent (&c, open, 1.0f, 190, false, 0.0, 0.0);
    };

    if (browserTabs.getTab() == 0)
    {
        refreshPresetPanel();
        slideIn (presetPanel, presetPanel.preferredHeight());
        groupBrowser.setVisible (false);
    }
    else
    {
        refreshGroupBrowser();
        slideIn (groupBrowser, groupBrowser.preferredHeight());
        presetPanel.setVisible (false);
    }
}

void MetaTuneEditor::repaintAll()
{
    brushed = juce::Image(); // rebuilt on the next paint in the current theme
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
        showPresetPanel (false);
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
        showPresetPanel (false);
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
    auto& animator = juce::Desktop::getInstance().getAnimator();
    if (show == presetPanel.isVisible() && ! animator.isAnimating (&presetPanel))
        return;
    refreshPresetPanel();
    const int h = juce::jmin (presetPanel.preferredHeight(), layout::rowY - layout::headerH - 12);
    const juce::Rectangle<int> open (113, layout::headerH + 2, 287, h), tucked = open.translated (0, -12);
    if (show)
    {
        showSettingsPanel (false);
        showColorPanel (false);
        browserTabs.setVisible (false);   // presets-only: no Groups tab
        groupBrowser.setVisible (false);
        groups.setBrowseActive (false);
        presetPanel.setBounds (tucked);
        presetPanel.setAlpha (0.0f);
        presetPanel.setVisible (true);
        presetPanel.toFront (false);
        animator.animateComponent (&presetPanel, open, 1.0f, 170, false, 0.0, 0.0);
    }
    else
    {
        animator.animateComponent (&presetPanel, tucked, 0.0f, 130, true, 1.0, 1.0);
        presetPanel.setVisible (false);
        browserTabs.setVisible (false);
    }
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
    std::vector<PresetItem> items;
    for (const auto& e : proc.presetList())
        items.push_back ({ e.name, e.folder, e.factory, update::Settings::isFavorite (e.folder, e.name) });
    presetPanel.setItems (items, proc.ui.presetName);
}

void MetaTuneEditor::refreshGroupBrowser()
{
    juce::StringArray names;
    for (int g = 0; g < 4; ++g) names.add (MetaTuneProcessor::groupName (g));
    std::vector<GroupInstanceRow> rows;
    for (const auto& inf : proc.groupInstances())
        rows.push_back ({ inf.id, inf.group,
                          inf.preset.isNotEmpty() ? inf.preset : "Instance " + juce::String (inf.id),
                          inf.id == proc.instanceId });
    groupBrowser.setData (names, rows);
}

void MetaTuneEditor::showNotesCard()
{
    update::Settings::setNotesVersion (JucePlugin_VersionString);
    showPresetPanel (false);
    showSettingsPanel (false);
    showGroupBrowser (false);
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
    auto& animator = juce::Desktop::getInstance().getAnimator();
    if (show == groupBrowser.isVisible() && ! animator.isAnimating (&groupBrowser)) return;
    if (show)
    {
        refreshGroupBrowser();
        const int h = juce::jmin (groupBrowser.preferredHeight(), layout::rowY - 134);
        const juce::Rectangle<int> open (54, 124, 272, h), tucked = open.translated (-12, 0);
        showPresetPanel (false);
        showSettingsPanel (false);
        groupBrowser.setBounds (tucked);
        groupBrowser.setAlpha (0.0f);
        groupBrowser.setVisible (true);
        groupBrowser.toFront (false);
        animator.animateComponent (&groupBrowser, open, 1.0f, 170, false, 0.0, 0.0);
    }
    else
    {
        animator.animateComponent (&groupBrowser, groupBrowser.getBounds().translated (-12, 0), 0.0f, 130, true, 1.0, 1.0);
        groupBrowser.setVisible (false);
        browserTabs.setVisible (false);
    }
}

void MetaTuneEditor::setScale (float s)
{
    proc.ui.scale = s;
    setSize (juce::roundToInt (kW * s), juce::roundToInt (kH * s));
}
