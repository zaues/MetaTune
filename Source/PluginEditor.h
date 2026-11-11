#pragma once
#include "PluginProcessor.h"
#include "UI/KeyboardPanel.h"
#include "UI/Knob.h"
#include "UI/MetaTuneLookAndFeel.h"
#include "UI/Orb.h"
#include "UI/Widgets.h"
#include "Update/Update.h"

// Fixed 1082x807 design canvas; the editor scales it uniformly (60-150 %) so the layout is designed
// once, never squeezed. All visual data comes from the processor's telemetry queue.
class MetaTuneEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    static constexpr int referenceW = 1415, referenceH = 1056;
    static constexpr int kW = 1082, kH = 807; // Logical JUCE units, fitted to the reference aspect ratio.
    static int heightForWidth (float width) { return juce::roundToInt (width * referenceH / referenceW); }

    explicit MetaTuneEditor (MetaTuneProcessor&);
    ~MetaTuneEditor() override;
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (theme::bg1); }

private:
    friend struct MetaTuneSnapshotAccess;
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
    void selectNoteMode();
    void updatePicker();        // per-frame: eases the picker's slide toward the pointer/pin state
    void syncRootPickerState(); // push current root/scale into the picker
    void applyPickedScale (int index); // RootPicker::scaleNames() index
    juce::String scaleDisplayName() const;
    void showPresetPanel (bool show);
    void refreshPresetPanel();
    void showGroupBrowser (bool show);
    void refreshGroupBrowser();
    void showBrowser (bool show, int tab = -1); // tabbed Presets/Groups browser from the box icon
    void showColorPanel (bool show);
    void showAutoKey();            // open the Auto Key card (install prompt or MetaKey status + apply)
    void applyAutoKey (int root, bool minor);
    void repaintAll(); // after a theme / colour change: drop cached textures and repaint everything
    void showNotesCard(); // "What's New" overlay
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
    GroupBrowser groupBrowser;
    BrowserTabs browserTabs;
    ColorPanel colorPanel;
    SettingsPanel settingsPanel;
    SavePresetCard saveCard;
    ReleaseNotesCard notesCard;
    AutoKeyCard autoKeyCard;

    TextPill notePill { "Note", TextPill::Kind::Primary };
    TextPill majorPill { "Major", TextPill::Kind::SplitLeft }, minorPill { "Minor", TextPill::Kind::SplitLeft };
    TextPill majorHoldPill { "Hold", TextPill::Kind::SplitRight }, minorHoldPill { "Hold", TextPill::Kind::SplitRight };
    ToggleSwitch lowLatency;

    IconButton colorButton { "color", icons::palette() };
    IconButton menuButton { "menu", icons::menu() };
    IconButton saveButton { "save", icons::save() };
    IconButton undoButton { "undo", icons::undo() }, redoButton { "redo", icons::redo() };
    PillButton aButton { "A" }, bButton { "B" };
    IconButton copyButton { "copy", icons::copyArrow() };
    IconButton helpButton { "tips", icons::chat() };
    IconButton heatButton { "heat", icons::eye(), "In/Out\nHeatMaps", icons::swap() };
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
    int toneChordMode = 0;     // 0 single, 1 major, 2 minor: what the tone generator plays on a key press
    enum class PendingSave { Preset, Folder, GroupName } pendingSave = PendingSave::Preset;
    juce::String pendingFolder; // target folder for the next preset save ("" = root)
    int pendingGroup = 0;       // group being renamed via the save card
    std::unique_ptr<juce::TooltipWindow> tooltipWindow;

    DisplayState display;
    double lastVoicedMs = -1e9;
    float latestRms = 0;
    int shownRange = -1;
    int presetCheckTicks = 0; // throttles the "current preset still exists on disk?" check
    juce::Image brushed, titleTexture; // Cached chrome textures.
    double openedMs = juce::Time::getMillisecondCounterHiRes();
    float openScale = proc.ui.scale; // saved size at open; the host's first resize must not replace it
    bool sizeRestored = false;
    bool showNotesPending = false; // show the "What's New" overlay once, after the UI is first laid out

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetaTuneEditor)
};
