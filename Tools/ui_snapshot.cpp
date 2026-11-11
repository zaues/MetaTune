// Renders the real editor to PNG for visual review (runs under Wine; no host needed).
// Feeds the processor a synthetic, slightly sharp harmonic tone so the visualisers show real DSP data.
// usage: ui_snapshot.exe <out-dir> [scale] [major|bypass|silent|reference|reference-next|alternate|custom|notes|doubler|controls]
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

    static void frame (MetaTuneEditor& editor) { editor.timerCallback(); }

    static bool testControls (MetaTuneEditor& editor)
    {
        const auto mouse = [] (juce::Component& component, juce::Point<float> point, int button)
        {
            const auto source = juce::Desktop::getInstance().getMainMouseSource();
            const auto now = juce::Time::getCurrentTime();
            return juce::MouseEvent (source, point, juce::ModifierKeys (button), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                     &component, &component, now, point, now, 1, false);
        };
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
        const auto closeTo = [] (double actual, double expected) { return std::abs (actual - expected) < 0.2; };
        if (processor.getABSlot() != 0 || ! closeTo (state.getRawParameterValue (ParamID::speed)->load(), 70.0))
            return false;

        // Host parameter updates reach the existing attachments, including the non-default readouts.
        const auto hostSet = [&] (const juce::String& id, float value)
        {
            auto& parameter = *state.getParameter (id);
            parameter.setValueNotifyingHost (parameter.convertTo0to1 (value));
        };
        hostSet (ParamID::sustain, -30.0f);
        hostSet (ParamID::amount, 85.8f);
        hostSet (ParamID::referenceHz, 452.1f);
        if (! closeTo (editor.sustainKnob.getValue(), -30.0) || ! closeTo (editor.amountKnob.getValue(), 85.8)
            || ! closeTo (editor.refStrip.getValue(), 452.1))
            return false;
        editor.bypassButton.setToggleState (true, juce::sendNotificationSync);
        if (state.getRawParameterValue (ParamID::bypass)->load() < 0.5f)
            return false;

        // Exercise both the piano's flared key shape and the note-label hit areas.
        auto& keyboard = editor.keyboard;
        const auto blackKey = juce::Point<float> (float (keyboard.getWidth()) * 1.5f / 12.0f,
                                                  float (keyboard.getHeight()) * 0.60f);
        const bool wasAllowed = metatune::isAllowed (processor.currentMask(), 1);
        keyboard.mouseDown (mouse (keyboard, blackKey, juce::ModifierKeys::leftButtonModifier));
        if (processor.toneNote.load() != 61 || metatune::isAllowed (processor.currentMask(), 1) == wasAllowed)
            return false;
        keyboard.mouseUp (mouse (keyboard, blackKey, 0));
        const auto eLabel = juce::Point<float> (float (keyboard.getWidth()) * 4.5f / 12.0f, 65.0f);
        keyboard.mouseDown (mouse (keyboard, eLabel, juce::ModifierKeys::rightButtonModifier));
        return processor.toneNote.load() == -1 && closeTo (state.getRawParameterValue (ParamID::root)->load(), 4.0);
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
    const bool fixedFrame = mode == "reference" || tailBlocks > 0 || alternate || custom;
    if (fixedFrame)
    {
        set ("root", custom ? 4.0f : (alternate ? 6.0f : 1.0f));
        set ("scale", custom ? 3.0f : (alternate ? 2.0f : 1.0f));
        set ("range", alternate || custom ? 0.0f : 2.0f);
        set ("speed", alternate ? 25.0f : -3.0f);
        set ("sustain", alternate ? -30.0f : 0.0f);
        set ("amount", 100);
        set ("stabilizer", 1);
        set ("refHz", alternate || custom ? 440.0f : 452.1f);
        if (custom)
        {
            constexpr bool allowed[] { true, false, true, false, false, true, true, true, true, true, false, false };
            for (int note = 0; note < 12; ++note)
                set (ParamID::note (note).toRawUTF8(), allowed[note] ? 1.0f : 0.0f);
        }
        auto& ui = static_cast<MetaTuneProcessor&> (*proc).ui;
        ui.useFlats = ! custom;
        ui.scaleEverSet = true;
        ui.scale = float (MetaTuneEditor::referenceW) / MetaTuneEditor::kW;
    }
    if (mode == "bypass") set ("bypass", 1);

    std::unique_ptr<juce::AudioProcessorEditor> ed (proc->createEditorAndMakeActive());
    const int width = fixedFrame ? MetaTuneEditor::referenceW : juce::roundToInt (MetaTuneEditor::kW * scale);
    ed->setSize (width, MetaTuneEditor::heightForWidth (float (width)));
    auto& editor = static_cast<MetaTuneEditor&> (*ed);
    if (mode == "controls")
    {
        const bool passed = MetaTuneSnapshotAccess::testControls (editor);
        std::puts (passed ? "PASS group navigation, A/B copy, knob and reference attachments, bypass, and piano input"
                          : "FAIL editor control interactions");
        return passed ? 0 : 1;
    }
    if (fixedFrame) MetaTuneSnapshotAccess::begin (editor);
    if (mode == "light") theme::setTheme (true); // forced for visual review (snapshot ignores saved prefs)

    const double amp = mode == "silent" ? 0.0 : 0.2; // "silent": no input, so voice-driven visuals stay off
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
        const double freq = fixedFrame ? 440.0 * std::pow (2.0, (referencePhrase[phraseIndex] + bend - 69.0) / 12.0) : f;
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

    if (mode == "notes" || mode == "doubler")
    {
        auto* canvas = ed->getChildComponent (0);
        RootPicker* picker = nullptr;
        KeyboardPanel* keyboard = nullptr;
        juce::Component* chromatic = nullptr;
        Orb* orb = nullptr;
        for (auto* child : canvas->getChildren())
        {
            if (auto* p = dynamic_cast<RootPicker*> (child)) picker = p;
            if (auto* k = dynamic_cast<KeyboardPanel*> (child)) keyboard = k;
            if (auto* o = dynamic_cast<Orb*> (child)) orb = o;
            if (child->getName() == "chromatic") chromatic = child;
        }

        if (mode == "notes" && picker != nullptr && keyboard != nullptr && chromatic != nullptr)
        {
            picker->setState (0, 0, false);
            const auto keys = keyboard->getBounds();
            picker->setBounds (0, keys.getY(), keys.getX() - 1, chromatic->getY() - keys.getY());
            picker->setVisible (true);
            picker->toFront (false);
        }

        if (mode == "doubler" && orb != nullptr)
        {
            const int y = orb->getHeight() / 4;
            for (int x = 0; x < orb->getWidth(); ++x)
            {
                if (! orb->hitTest (x, y)) continue;
                const juce::Point<float> point { float (x), float (y) };
                const auto now = juce::Time::getCurrentTime();
                const auto source = juce::Desktop::getInstance().getMainMouseSource();
                orb->mouseMove ({ source, point, {}, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, orb, orb, now, point, now, 0, false });
                break;
            }
            // The first paint starts the guide fade, then the timer advances it.
            orb->createComponentSnapshot (orb->getLocalBounds());
            for (int i = 0; i < 30; ++i)
            {
                juce::Thread::sleep (17);
                juce::Timer::callPendingTimersSynchronously();
            }
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
