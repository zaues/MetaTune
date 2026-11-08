#pragma once
#include "PluginProcessor.h"
#include "UI/KeyboardPanel.h"
#include "UI/Knob.h"
#include "UI/MetaTuneLookAndFeel.h"
#include "UI/Orb.h"
#include "UI/Widgets.h"
#include "Update/Update.h"

// Fixed 1082x856 design canvas; the editor scales it uniformly (60-150 %) so the layout is designed
// once, never squeezed. All visual data comes from the processor's telemetry queue.
class MetaTuneEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    static constexpr int kW = 1082, kH = 856;

    explicit MetaTuneEditor (MetaTuneProcessor&);
    ~MetaTuneEditor() override;
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (theme::bg1); }

private:
    struct Canvas : public juce::Component
    {
        explicit Canvas (MetaTuneEditor& e) : owner (e) {}
        void paint (juce::Graphics&) override;
        MetaTuneEditor& owner;
    };

    void timerCallback() override;
    void mouseDown (const juce::MouseEvent&) override; // global listener: a click outside the key picker closes it
    void layout();
    void keyClicked (int pc);
    void toggleNote (int pc);
    void setRoot (int pc);
    void setParam (const juce::String& id, float value);
    void selectScale (metatune::ScaleType);
    void selectKeyScale (metatune::ScaleType); // apply a major/minor key then drop into editable Note mode
    void selectNoteMode();
    void updatePicker();        // per-frame: eases the picker's slide toward the pointer/pin state
    void syncRootPickerState(); // push current root/scale into the picker
    void applyPickedScale (int index); // RootPicker::scaleNames() index
    juce::String scaleDisplayName() const;
    void showPresetPanel (bool show);
    void showSettingsPanel (bool show);
    void stepPreset (int delta);
    void savePresetDialog();
    void setScale (float s);
    void updateTooltips();
    void syncModeButtons();
    void startUpdateCheck();
    void showUpdate (std::optional<update::Release>);
    void installUpdate();

    MetaTuneProcessor& proc;
    juce::AudioProcessorValueTreeState& state;
    MetaTuneLookAndFeel lnf;
    Canvas canvas { *this };

    Knob speedKnob, sustainKnob, amountKnob;
    ValueStrip refStrip;
    ToneSlider toneSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAttach, sustainAttach, amountAttach, refAttach, toneAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;
    Orb orb;
    KeyboardPanel keyboard;
    SegmentedControl stabilizerSeg;
    GroupTabs groups;
    PresetBar presetBar;
    RootPicker rootPicker;
    PresetPanel presetPanel;
    SettingsPanel settingsPanel;
    SavePresetCard saveCard;

    TextPill notePill { "Note", TextPill::Kind::Primary };
    TextPill majorPill { "Major", TextPill::Kind::Plain }, minorPill { "Minor", TextPill::Kind::Plain };
    TextPill chromaticPill { "Chromatic", TextPill::Kind::Plain }, scalePill { "Scale", TextPill::Kind::Plain };
    ToggleSwitch lowLatency;

    IconButton menuButton { "menu", icons::menu() };
    IconButton saveButton { "save", icons::save() };
    IconButton undoButton { "undo", icons::undo() }, redoButton { "redo", icons::redo() }, resetButton { "reset", icons::loop() };
    PillButton aButton { "A" }, bButton { "B" };
    IconButton copyButton { "copy", icons::copyArrow() };
    IconButton helpButton { "tips", icons::chat() };
    IconButton heatButton { "heat", icons::eye(), "No Pitch\nReference", icons::swap() };
    IconButton detectedButton { "detected", icons::eye(), "Detected\nNotes", icons::swap() };
    IconButton bypassButton { "bypass", icons::power(), "Global\nBypass" };
    IconButton chromaticButton { "chromatic", icons::swap(), "Chromatic" };
    UpdatePill updatePill;
    update::Checker updateChecker;
    std::optional<update::Release> pendingUpdate;
    juce::String updateStatus; // shown in the settings menu
    juce::Rectangle<int> rootArea;
    bool pickerPinned = false; // true = picker opened by clicking "Scale"; stays until a click elsewhere
    float pickerSlide = 0.0f;  // 0 = tucked off the left edge / hidden, 1 = fully in; eased each frame
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;

    DisplayState display;
    double lastVoicedMs = -1e9;
    float latestRms = 0;
    int shownRange = -1;
    juce::Image brushed; // brushed-metal texture for the knob panel, rendered once
    double openedMs = juce::Time::getMillisecondCounterHiRes();
    float openScale = proc.ui.scale; // saved size at open; the host's first resize must not replace it
    bool sizeRestored = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetaTuneEditor)
};
