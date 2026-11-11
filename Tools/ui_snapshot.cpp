// Renders the real editor to PNG for visual review (runs under Wine; no host needed).
// Feeds the processor a synthetic, slightly sharp harmonic tone so the visualisers show real DSP data.
// usage: ui_snapshot.exe <out-dir> [scale] [mode]
// Modes: major, bypass, silent, light, reference, reference-next, alternate, custom,
//        notes, doubler, speed-positive, speed-zero, speed-negative, bypass-video,
//        doubler-min, doubler-mid, doubler-max, doubler-idle, audition-major, audition-minor, presets, groups, sustain, controls.
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include "../Source/Parameters.h"
#include "../Source/UI/Theme.h"
#include <juce_gui_basics/juce_gui_basics.h>

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

struct MetaTuneSnapshotAccess
{
    static void begin (MetaTuneEditor& editor)
    {
        editor.stopTimer();
        editor.sizeRestored = true;
        editor.showNotesPending = false;
    }

    static void holdRequestedSize (MetaTuneEditor& editor) { editor.sizeRestored = true; }
    static void frame (MetaTuneEditor& editor) { editor.timerCallback(); }
    static void useReferencePalette (MetaTuneEditor& editor, bool light)
    {
        theme::setTheme (light);
        theme::setGradient (false, theme::gradA, theme::gradB);
        theme::setAccent (0.0f, 1.0f, 1.0f);
        theme::orbHue = theme::keyHue = theme::meterHue = 0.0f;
        editor.repaintAll();
    }

    static juce::MouseEvent mouse (juce::Component& component, juce::Point<float> point, int modifiers)
    {
        const auto source = juce::Desktop::getInstance().getMainMouseSource();
        const auto now = juce::Time::getCurrentTime();
        return { source, point, juce::ModifierKeys (modifiers), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                 &component, &component, now, point, now, 1, false };
    }

    static void showDoublerGuides (MetaTuneEditor& editor)
    {
        auto& orb = editor.orb;
        orb.mouseMove (mouse (orb, orb.handlePos(), 0));
        orb.guide.snapTo (1.0f);
    }

    static constexpr float labelTop = float (KeyboardPanel::kHeatTop + 2 * (KeyboardPanel::kHeatH + KeyboardPanel::kHeatGap));

    static juce::Point<float> noteLabel (const KeyboardPanel& keyboard, int pitchClass)
    {
        return { float (keyboard.getWidth()) * (float (pitchClass) + 0.5f) / 12.0f, labelTop + KeyboardPanel::kLabelH * 0.5f };
    }

    static void showChordAudition (MetaTuneEditor& editor, bool major)
    {
        (major ? editor.majorPill : editor.minorPill).onClick();
        auto& keyboard = editor.keyboard;
        keyboard.mouseDown (mouse (keyboard, noteLabel (keyboard, 4), juce::ModifierKeys::leftButtonModifier));
    }

    static bool closeTo (double actual, double expected) { return std::abs (actual - expected) < 0.2; }

    static void setFromHost (MetaTuneEditor& editor, const juce::String& id, float value)
    {
        auto& parameter = *editor.proc.apvts.getParameter (id);
        parameter.setValueNotifyingHost (parameter.convertTo0to1 (value));
    }

    static bool testControls (MetaTuneEditor& editor)
    {
        auto& groups = editor.groups;
        auto click = [&] (juce::Point<float> point)
        {
            groups.mouseDown (mouse (groups, point, juce::ModifierKeys::leftButtonModifier));
        };
        auto& processor = editor.proc;
        click ({ 28.0f, 80.0f });
        if (processor.getGroup() != 0) return false;
        click ({ 10.0f, float (groups.getHeight()) - 12.0f });
        if (processor.getGroup() != 1) return false;
        click ({ 16.0f, float (groups.getHeight()) - 41.0f });
        if (processor.getGroup() != 0) return false;
        click ({ 28.0f, 80.0f });
        if (processor.getGroup() != -1) return false;

        editor.speedKnob.setValue (25.0, juce::sendNotificationSync);
        editor.bButton.onClick();
        if (processor.getABSlot() != 1) return false;
        editor.speedKnob.setValue (70.0, juce::sendNotificationSync);
        editor.copyButton.onClick();
        editor.aButton.onClick();
        auto& state = processor.apvts;
        if (processor.getABSlot() != 0 || ! closeTo (state.getRawParameterValue (ParamID::speed)->load(), 70.0))
            return false;

        // Host parameter updates reach the existing attachments, including the non-default readouts.
        setFromHost (editor, ParamID::sustain, -30.0f);
        setFromHost (editor, ParamID::amount, 85.8f);
        setFromHost (editor, ParamID::referenceHz, 452.1f);
        if (! closeTo (editor.sustainKnob.getValue(), -30.0) || ! closeTo (editor.amountKnob.getValue(), 85.8)
            || ! closeTo (editor.refStrip.getValue(), 452.1))
            return false;
        return true;
    }

    static bool testKeyboard (MetaTuneEditor& editor)
    {
        using metatune::ScaleType;
        auto& processor = editor.proc;
        auto& keyboard = editor.keyboard;
        const auto root = [&] { return int (processor.apvts.getRawParameterValue (ParamID::root)->load()); };
        const auto scale = [&] { return ScaleType (int (processor.apvts.getRawParameterValue (ParamID::scale)->load())); };
        const auto hasScale = [&] (ScaleType expected, int pitch)
        {
            return scale() == expected && root() == pitch && processor.currentMask() == metatune::scaleMask (expected, pitch);
        };

        const float width = float (keyboard.getWidth());
        const auto eLabel = noteLabel (keyboard, 4);
        const auto whiteKey = [&] (float x) { return juce::Point<float> { width * x, float (keyboard.getHeight()) - 12.0f }; };
        const juce::Point<float> cSharp { width * 0.125f, labelTop + KeyboardPanel::kLabelH + 20.0f };
        const auto press = [&] (juce::Point<float> point, int note, int chord = 0)
        {
            keyboard.mouseDown (mouse (keyboard, point, juce::ModifierKeys::leftButtonModifier));
            const bool playing = processor.toneNote.load() == note && (note < 0 || processor.toneChord.load() == chord);
            keyboard.mouseUp (mouse (keyboard, point, 0));
            return playing && processor.toneNote.load() == -1;
        };

        const auto initialMask = processor.currentMask();
        const int initialRoot = root();
        const auto initialScale = scale();
        if (! press (eLabel, 64, 0) || processor.currentMask() != initialMask || root() != initialRoot)
            return false;

        editor.majorPill.onClick();
        if (scale() != initialScale || ! press (eLabel, 64, 1) || processor.currentMask() != initialMask
            || root() != initialRoot || ! press (eLabel, 64, 0))
            return false;

        const auto toggledMask = metatune::PitchClassMask (initialMask ^ (1u << 1) ^ 1u);
        if (! press (cSharp, -1) || processor.currentMask() != (initialMask ^ (1u << 1))
            || ! press (whiteKey (0.05f), -1) || processor.currentMask() != toggledMask || scale() != ScaleType::Custom)
            return false;

        editor.chromaticButton.onClick();
        if (scale() != ScaleType::Chromatic || processor.currentMask() != metatune::kAllNotes)
            return false;
        juce::MemoryBlock savedProject;
        processor.getStateInformation (savedProject);
        processor.ui.scaleBeforeChromatic.reset();
        processor.setStateInformation (savedProject.getData(), int (savedProject.getSize()));
        editor.chromaticButton.onClick();
        if (scale() != ScaleType::Custom || processor.currentMask() != toggledMask || root() != initialRoot)
            return false;

        editor.majorPill.onClick();
        if (! press (whiteKey (0.20f), -1) || ! hasScale (ScaleType::Major, 2)
            || ! press (eLabel, 64, 0))
            return false;

        editor.majorHoldPill.onClick();
        if (! processor.ui.majorHold || processor.ui.minorHold || ! press (whiteKey (0.48f), -1)
            || ! hasScale (ScaleType::Major, 5) || ! press (eLabel, 64, 1) || ! hasScale (ScaleType::Major, 5))
            return false;

        editor.minorHoldPill.onClick();
        if (processor.ui.majorHold || ! processor.ui.minorHold || ! press (whiteKey (0.20f), -1)
            || ! hasScale (ScaleType::Minor, 2) || ! press (eLabel, 64, 2) || ! hasScale (ScaleType::Minor, 2))
            return false;

        editor.chromaticButton.onClick();
        if (processor.ui.majorHold || processor.ui.minorHold || processor.currentMask() != metatune::kAllNotes)
            return false;
        editor.chromaticButton.onClick();
        if (! hasScale (ScaleType::Minor, 2) || ! press (eLabel, 64, 0))
            return false;

        keyboard.mouseDown (mouse (keyboard, eLabel, juce::ModifierKeys::rightButtonModifier));
        keyboard.mouseUp (mouse (keyboard, eLabel, 0));
        return processor.toneNote.load() == -1 && hasScale (ScaleType::Minor, 4);
    }

    static bool testBypass (MetaTuneEditor& editor)
    {
        constexpr metatune::Telemetry signal { 63.8f, 64.0f, 0.99f, 0.12f, 64 };
        auto& processor = editor.proc;
        const auto feed = [&]
        {
            processor.telemetry.push (signal);
            frame (editor);
        };
        feed();
        if (editor.display.target != signal.target || editor.display.outputMidi != signal.outputMidi)
            return false;

        editor.bypassButton.setToggleState (true, juce::sendNotificationSync);
        frame (editor); // A previously detected target should clear without waiting for another audio block.
        if (processor.apvts.getRawParameterValue (ParamID::bypass)->load() < 0.5f
            || editor.display.target != -1 || editor.display.outputMidi != signal.inputMidi)
            return false;
        feed();
        if (editor.display.target != -1 || editor.display.outputMidi != signal.inputMidi)
            return false;
        juce::Thread::sleep (200);
        frame (editor);
        if (editor.bypassBlend < 0.99f)
            return false;

        editor.bypassButton.setToggleState (false, juce::sendNotificationSync);
        feed();
        juce::Thread::sleep (200);
        feed();
        return editor.bypassBlend < 0.01f && editor.display.target == signal.target && editor.display.outputMidi == signal.outputMidi;
    }

    static bool testHeatHistory (MetaTuneEditor& editor)
    {
        auto& keyboard = editor.keyboard;
        editor.heatButton.setToggleState (true, juce::sendNotificationSync);
        editor.detectedButton.setToggleState (true, juce::sendNotificationSync);
        editor.heatResetButton.onClick();
        editor.detectedResetButton.onClick();
        const auto capture = [&] { return keyboard.createComponentSnapshot (keyboard.getLocalBounds()); };
        const int e = juce::roundToInt (keyboard.getWidth() * 4.5f / 12.0f);
        const int heatY = keyboard.heatOutputY() + 5;
        const int seamY = keyboard.heatOutputY() + KeyboardPanel::kHeatH + KeyboardPanel::kHeatGap
                          + KeyboardPanel::kLabelH - 1;
        const auto idle = capture();
        for (int sample = 0; sample < 20; ++sample)
            keyboard.addHeat (63.8f, 64.0f, 0.25f);
        for (int tick = 0; tick < 150; ++tick)
            keyboard.decayHeat (0.9f);
        const auto history = capture();
        if (history.getPixelAt (e, heatY) == idle.getPixelAt (e, heatY)
            || history.getPixelAt (e, seamY) == idle.getPixelAt (e, seamY))
            return false;

        editor.heatButton.setToggleState (false, juce::sendNotificationSync);
        const auto hidden = capture();
        if (hidden.getPixelAt (e, heatY) == history.getPixelAt (e, heatY)
            || hidden.getPixelAt (e, seamY) != history.getPixelAt (e, seamY))
            return false;
        editor.heatButton.setToggleState (true, juce::sendNotificationSync);
        if (capture().getPixelAt (e, heatY) != history.getPixelAt (e, heatY))
            return false;

        editor.heatResetButton.onClick();
        const auto heatCleared = capture();
        if (heatCleared.getPixelAt (e, heatY) != idle.getPixelAt (e, heatY)
            || heatCleared.getPixelAt (e, seamY) != history.getPixelAt (e, seamY))
            return false;
        editor.detectedResetButton.onClick();
        return capture().getPixelAt (e, seamY) == idle.getPixelAt (e, seamY);
    }

    static bool showBrowser (MetaTuneEditor& editor, int tab)
    {
        editor.showBrowser (true, tab);
        for (int frame = 0; frame < 15; ++frame)
        {
            juce::Thread::sleep (16);
            juce::Timer::callPendingTimersSynchronously();
        }
        return editor.browserTabs.isVisible() && editor.browserTabs.getTab() == tab
            && (tab == 0 ? editor.presetPanel.isVisible() : editor.groupBrowser.isVisible());
    }

    static bool testBrowser (MetaTuneEditor& editor)
    {
        editor.menuButton.onModifiedClick ({});
        if (! editor.browserTabs.isVisible() || ! editor.presetPanel.isVisible()
            || editor.browserTabs.getTab() != 0 || editor.presetPanel.getX() >= 0)
            return false;

        editor.groups.onBrowse();
        if (editor.browserTabs.getTab() != 1 || ! editor.groupBrowser.isVisible() || editor.presetPanel.isVisible())
            return false;

        editor.presetBar.onClick();
        if (editor.browserTabs.getTab() != 0 || ! editor.presetPanel.isVisible() || editor.groupBrowser.isVisible())
            return false;

        const bool keep = editor.proc.ui.keepScale;
        editor.presetPanel.onToggleKeepScale();
        if (editor.proc.ui.keepScale == keep)
            return false;
        editor.presetPanel.onToggleKeepScale();

        const auto* range = editor.proc.apvts.getRawParameterValue (ParamID::range);
        const float prior = range->load();
        editor.presetPanel.onToggleTiming();
        if (range->load() == prior)
            return false;
        setFromHost (editor, ParamID::range, prior);

        editor.menuButton.onModifiedClick (juce::ModifierKeys::shiftModifier);
        return editor.settingsPanel.isVisible() && ! editor.browserTabs.isVisible();
    }

    static bool testOrb (MetaTuneEditor& editor)
    {
        auto& orb = editor.orb;
        DisplayState voiced;
        voiced.voiced = true;
        voiced.inputMidi = 63.8f;
        voiced.outputMidi = 64.0f;
        voiced.level = 0.6f;
        voiced.presence = 1.0f;
        for (int tick = 0; tick < 20; ++tick)
            orb.setDisplay (voiced, -0.8f, 0.0f);
        const float shallow = orb.boltAmt;
        for (int tick = 0; tick < 20; ++tick)
            orb.setDisplay (voiced, -3.0f, 0.0f);
        if (shallow <= 0.0f || orb.boltAmt <= shallow)
            return false;
        orb.setDisplay (voiced, 0.0f, 0.0f);
        if (orb.boltAmt != 0.0f || orb.negativeSpeed)
            return false;
        const float before = orb.spin;
        orb.setDisplay (voiced, 25.0f, 1.0f);
        return orb.boltAmt == 0.0f && orb.bypassAmount == 1.0f && orb.act > 0.0f && orb.spin != before;
    }

    static bool testDoubler (MetaTuneEditor& editor)
    {
        auto& orb = editor.orb;
        auto& state = editor.proc.apvts;
        auto* mix = state.getRawParameterValue (ParamID::doublerMix);
        auto* width = state.getRawParameterValue (ParamID::doublerWidth);
        setFromHost (editor, ParamID::doublerMix, 0.0f);
        setFromHost (editor, ParamID::doublerWidth, 0.0f);

        constexpr int left = juce::ModifierKeys::leftButtonModifier;
        constexpr int alt = juce::ModifierKeys::altModifier;
        const float pastRight = float (orb.getWidth()) + 200.0f;
        const auto drag = [&] (juce::Point<float> end, int modifiers)
        {
            const auto start = orb.handlePos();
            orb.mouseDown (mouse (orb, start, modifiers));
            orb.mouseDrag (mouse (orb, end, modifiers));
            orb.mouseUp (mouse (orb, end, 0));
        };
        drag ({ pastRight, orb.handlePos().y - 30.0f }, left | alt);
        if (! closeTo (width->load(), 100.0f) || ! closeTo (mix->load(), 0.0f))
            return false;
        drag ({ pastRight, -200.0f }, left);
        if (! closeTo (width->load(), 100.0f) || ! closeTo (mix->load(), 100.0f))
            return false;
        const auto guide = orb.guideGeom();
        const auto wetMarker = juce::Point<float> (guide.cornerX, orb.handlePos().y);
        const auto wetEnd = wetMarker.translated (0.0f, orb.trackArea().getHeight() * 0.75f);
        orb.mouseDown (mouse (orb, wetMarker, left));
        orb.mouseDrag (mouse (orb, wetEnd, left));
        orb.mouseUp (mouse (orb, wetEnd, 0));
        if (! closeTo (width->load(), 100.0f) || ! closeTo (mix->load(), 25.0f))
            return false;

        const auto widthMarker = juce::Point<float> (orb.handlePos().x, guide.baseY);
        const auto widthEnd = widthMarker.translated (-orb.trackArea().getWidth() * 0.50f, 0.0f);
        orb.mouseDown (mouse (orb, widthMarker, left));
        orb.mouseDrag (mouse (orb, widthEnd, left));
        orb.mouseUp (mouse (orb, widthEnd, 0));
        if (! closeTo (width->load(), 50.0f) || ! closeTo (mix->load(), 25.0f))
            return false;

        orb.mouseDoubleClick (mouse (orb, orb.handlePos(), left));
        return closeTo (width->load(), 0.0f) && closeTo (mix->load(), 0.0f);
    }
};

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File outDir (argc > 1 ? juce::String (argv[1]) : juce::File::getCurrentWorkingDirectory().getFullPathName());
    const float scale = argc > 2 ? juce::String (argv[2]).getFloatValue() : 1.0f;

    std::unique_ptr<juce::AudioProcessor> proc (createPluginFilter());
    const double sr = 48000;
    proc->setPlayConfigDetails (2, 2, sr, 128);
    proc->prepareToPlay (sr, 128);

    const juce::String mode = argc > 3 ? juce::String (argv[3]) : juce::String();
    auto set = [&] (const char* id, float v) {
        for (auto* p : proc->getParameters())
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p); rp && rp->getParameterID() == id)
                rp->setValueNotifyingHost (rp->convertTo0to1 (v));
    };
    if (mode == "major") { set ("root", 4); set ("scale", 1); set ("range", 2); set ("speed", -3); set ("amount", 72); set ("stabilizer", 1); }
    const bool alternate = mode == "alternate";
    const bool custom = mode == "custom";
    const int tailBlocks = mode == "reference-next" ? 96 : 0;
    const bool speedReference = mode == "speed-positive" || mode == "speed-zero" || mode == "speed-negative";
    const bool doublerIdle = mode == "doubler-idle";
    const bool doublerReference = mode == "doubler-min" || mode == "doubler-mid" || mode == "doubler-max" || doublerIdle;
    const bool videoBypass = mode == "bypass-video";
    const bool sustainReference = mode == "sustain";
    const bool browserReference = mode == "presets" || mode == "groups";
    const bool chordAudition = mode == "audition-major" || mode == "audition-minor";
    const bool fixedFrame = mode == "reference" || tailBlocks > 0 || alternate || custom
                            || speedReference || doublerReference || videoBypass || chordAudition
                            || sustainReference || browserReference;
    if (fixedFrame)
    {
        const bool gbMinor = alternate || browserReference || videoBypass || doublerReference || sustainReference;
        set ("root", custom ? 4.0f : (gbMinor ? 6.0f : 1.0f));
        set ("scale", custom ? 3.0f : (gbMinor ? 2.0f : 1.0f));
        set ("range", gbMinor || custom ? 0.0f : 2.0f);
        float speed = -3.0f;
        if (alternate || mode == "speed-positive" || videoBypass || sustainReference || doublerReference) speed = 25.0f;
        if (browserReference) speed = 21.3f;
        if (mode == "speed-zero") speed = 0.0f;
        if (mode == "speed-negative") speed = -0.8f;
        set (ParamID::speed, speed);
        set ("sustain", sustainReference ? -73.0f : (browserReference ? -35.0f : (alternate ? -30.0f : 0.0f)));
        set ("amount", 100);
        set ("stabilizer", 1);
        set ("refHz", gbMinor || custom ? 440.0f : 452.1f);
        if (custom)
        {
            constexpr bool allowed[] { true, false, true, false, false, true, true, true, true, true, false, false };
            for (int note = 0; note < 12; ++note)
                set (ParamID::note (note).toRawUTF8(), allowed[note] ? 1.0f : 0.0f);
        }
        if (doublerReference)
        {
            const float value = mode == "doubler-max" || doublerIdle ? 100.0f : (mode == "doubler-mid" ? 50.0f : 0.0f);
            set (ParamID::doublerMix, value);
            set (ParamID::doublerWidth, value);
        }
        auto& ui = static_cast<MetaTuneProcessor&> (*proc).ui;
        ui.useFlats = ! custom;
        ui.scaleEverSet = true;
        ui.scale = float (MetaTuneEditor::referenceW) / MetaTuneEditor::kW;
    }
    if (mode == "bypass" || videoBypass) set (ParamID::bypass, 1.0f);

    std::vector<std::unique_ptr<MetaTuneProcessor>> groupMembers;
    if (mode == "groups")
    {
        static_cast<MetaTuneProcessor&> (*proc).joinGroup (2);
        proc->updateTrackProperties ({ juce::String ("Harmony 1"), {} });
        const std::pair<int, const char*> tracks[] {
            { -1, "Lead Vocal" }, { 0, "Bridge Vocal" }, { 1, "Double" },
            { 2, "Harmony 2" }, { 2, "Harmony 3" }, { 3, "Ad-libs" }
        };
        for (const auto& [group, name] : tracks)
        {
            auto member = std::make_unique<MetaTuneProcessor>();
            member->updateTrackProperties ({ juce::String (name), {} });
            member->joinGroup (group);
            groupMembers.push_back (std::move (member));
        }
    }

    std::unique_ptr<juce::AudioProcessorEditor> ed (proc->createEditorAndMakeActive());
    const int width = fixedFrame ? MetaTuneEditor::referenceW : juce::roundToInt (MetaTuneEditor::kW * scale);
    ed->setSize (width, MetaTuneEditor::heightForWidth (float (width)));
    auto& editor = static_cast<MetaTuneEditor&> (*ed);
    MetaTuneSnapshotAccess::holdRequestedSize (editor);
    if (mode == "controls")
    {
        MetaTuneSnapshotAccess::begin (editor);
        const bool controls = MetaTuneSnapshotAccess::testControls (editor);
        const bool keyboard = MetaTuneSnapshotAccess::testKeyboard (editor);
        const bool bypass = MetaTuneSnapshotAccess::testBypass (editor);
        const bool doubler = MetaTuneSnapshotAccess::testDoubler (editor);
        const bool heat = MetaTuneSnapshotAccess::testHeatHistory (editor);
        const bool orb = MetaTuneSnapshotAccess::testOrb (editor);
        const bool browser = MetaTuneSnapshotAccess::testBrowser (editor);
        std::printf ("%s group navigation, A/B copy, knob and reference attachments\n", controls ? "PASS" : "FAIL");
        std::printf ("%s label audition, piano editing, Hold, and Chromatic restoration\n", keyboard ? "PASS" : "FAIL");
        std::printf ("%s bypass attachment, display transition, and telemetry\n", bypass ? "PASS" : "FAIL");
        std::printf ("%s Doubler XY drag, direct projections, Alt lock, and reset\n", doubler ? "PASS" : "FAIL");
        std::printf ("%s heatmap history, visibility, and independent clears\n", heat ? "PASS" : "FAIL");
        std::printf ("%s bolt threshold and continued orb motion in bypass\n", orb ? "PASS" : "FAIL");
        std::printf ("%s header, preset/group tabs, timing toggle, and Settings shortcut\n", browser ? "PASS" : "FAIL");
        return controls && keyboard && bypass && doubler && heat && orb && browser ? 0 : 1;
    }
    if (fixedFrame) MetaTuneSnapshotAccess::begin (editor);
    MetaTuneSnapshotAccess::useReferencePalette (editor, mode == "light");

    const double amp = mode == "silent" || doublerIdle ? 0.0 : 0.2;
    // E4 + 28 cents with vibrato-free harmonics, ~1 s, interleaved with message-loop time so timers run.
    juce::AudioBuffer<float> buf (2, 128);
    juce::MidiBuffer midi;
    const double f = 440.0 * std::pow (2.0, (64.28 - 69.0) / 12.0);
    static constexpr float referencePhrase[] { 61.24f, 63.08f, 64.95f, 65.18f, 67.14f, 68.05f, 70.14f, 72.16f,
                                               73.06f, 74.89f, 76.16f, 77.23f, 79.72f, 80.05f, 82.29f, 77.22f };
    double phase = 0.0;
    constexpr int phraseNotes = int (std::size (referencePhrase));
    for (int block = 0; block < (fixedFrame ? phraseNotes * 80 + tailBlocks : 375); ++block)
    {
        const float bend = 0.22f * std::sin (float (block % 80) * 0.077f);
        const int phraseIndex = juce::jmin (block / 80, phraseNotes - 1);
        const double freq = fixedFrame && ! sustainReference ? 440.0 * std::pow (2.0, (referencePhrase[phraseIndex] + bend - 69.0) / 12.0) : f;
        const double step = juce::MathConstants<double>::twoPi * freq / sr;
        for (int i = 0; i < 128; ++i)
        {
            double s = 0;
            for (int h = 1; h <= 10; ++h) s += std::sin (phase * h + h) / h;
            phase += step;
            buf.setSample (0, i, float (amp * s));
            buf.setSample (1, i, float (amp * s));
        }
        proc->processBlock (buf, midi);
        if (block % 4 == 0)
        {
            if (fixedFrame)
                MetaTuneSnapshotAccess::frame (editor);
            else
            {
                juce::Thread::sleep (10);
                juce::Timer::callPendingTimersSynchronously();
            }
        }
    }
    if (fixedFrame) MetaTuneSnapshotAccess::frame (editor);
    else for (int i = 0; i < 3; ++i) { juce::Thread::sleep (20); juce::Timer::callPendingTimersSynchronously(); }

    if (browserReference && ! MetaTuneSnapshotAccess::showBrowser (editor, mode == "groups" ? 1 : 0))
    {
        std::puts ("FAIL opening the requested browser tab");
        return 1;
    }

    if (mode == "doubler" || doublerReference)
        MetaTuneSnapshotAccess::showDoublerGuides (editor);
    if (chordAudition)
        MetaTuneSnapshotAccess::showChordAudition (editor, mode == "audition-major");

    if (mode == "notes")
    {
        auto* canvas = ed->getChildComponent (0);
        RootPicker* picker = nullptr;
        KeyboardPanel* keyboard = nullptr;
        juce::Component* chromatic = nullptr;
        for (auto* child : canvas->getChildren())
        {
            if (auto* p = dynamic_cast<RootPicker*> (child)) picker = p;
            if (auto* k = dynamic_cast<KeyboardPanel*> (child)) keyboard = k;
            if (child->getName() == "chromatic") chromatic = child;
        }

        if (picker != nullptr && keyboard != nullptr && chromatic != nullptr)
        {
            picker->setState (0, 0, false);
            const auto keys = keyboard->getBounds();
            picker->setBounds (0, keys.getY(), keys.getX() - 1, chromatic->getY() - keys.getY());
            picker->setVisible (true);
            picker->toFront (false);
        }
    }

    const auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
    const auto file = outDir.getChildFile ("ui_" + juce::String (juce::roundToInt (scale * 100)) + mode + ".png");
    file.deleteFile();
    juce::FileOutputStream os (file);
    juce::PNGImageFormat().writeImageToStream (img, os);
    std::printf ("wrote %s (%dx%d)\n", file.getFullPathName().toRawUTF8(), img.getWidth(), img.getHeight());
    ed.reset();
    return 0;
}
