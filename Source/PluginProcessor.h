#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/TunerChain.h"
#include "DSP/Pitch/Scale.h"

class MetaTuneProcessor : public juce::AudioProcessor, private juce::Timer
{
public:
    MetaTuneProcessor();
    ~MetaTuneProcessor() override;

    void prepareToPlay (double sampleRate, int maxBlockSize) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // Allowed-note mask from root/scale/custom notes. Lock-free; callable from any thread.
    metatune::PitchClassMask currentMask() const noexcept;
    double latencyMsFor (int rangeIndex) const noexcept;

    // Editing workflow (message thread only).
    void resetAllParameters();
    int getABSlot() const noexcept { return abSlot; }
    void switchAB();      // store the current settings in this slot, recall the other one
    void copyToOtherAB(); // overwrite the other slot with the current settings

    // Presets: built-in ones first, then the user's files in presetFolder(). Loading a preset never
    // changes the key/scale or bypass, so a vocal chain can be swapped without re-entering the key.
    static juce::File presetFolder();
    juce::StringArray presetNames() const;
    int numFactoryPresets() const noexcept;
    void loadPreset (int index);
    bool savePreset (const juce::String& name);

    // Groups: copies of the plugin in the same group (0-3) share every setting except Bypass.
    // Message thread only. Joining adopts the group's current settings; joining the same group again leaves it.
    void joinGroup (int group);
    int getGroup() const noexcept { return ui.group; }
    static int groupMemberCount (int group);

    juce::UndoManager undoManager { 400, 20 }; // >= 20 user-visible transactions kept
    juce::AudioProcessorValueTreeState apvts;
    metatune::TelemetryQueue telemetry;            // audio -> editor, single consumer
    std::atomic<int> toneNote { -1 };           // editor -> audio: reference tone MIDI note, -1 = off

    // Display preferences: not parameters (not automatable, not part of A/B), saved with the project.
    struct UiSettings
    {
        bool heatmaps = true, detectedNotes = true, tooltips = true;
        bool majorHold = false, minorHold = false; // key click sets the root of a major/minor scale
        bool useFlats = false;                     // note names: Db instead of C#
        juce::String scaleLabel;                   // name of a picked Custom scale (e.g. "pent maj"); empty = hand-made
        juce::String presetName;                   // last loaded/saved preset; empty = none
        int group = -1;                            // -1 = not linked
        float scale = 0.85f;
    } ui;

private:
    void applyValues (const juce::StringPairArray& idToValue); // undoable, notifies the host
    std::vector<float> linkedValues() const;                  // normalised, in getParameters() order
    void receiveGroupValues (const std::vector<float>&);
    void syncGroup();                                         // timer: push local changes to the group
    std::vector<float> groupSynced;                           // values last exchanged with the group
    void process (juce::AudioBuffer<float>&, bool forceBypass) noexcept;
    void timerCallback() override; // message thread: keeps reported latency in step with Range

    metatune::TunerChain chain; // audio-thread owned after prepareToPlay()
    std::atomic<double> preparedRate { 0.0 };
    juce::ValueTree abSnapshot[2];
    int abSlot = 0;

    juce::AudioProcessorParameter* bypassParam = nullptr;
    std::atomic<float>* pBypass = nullptr;
    std::atomic<float>* pSpeed = nullptr;
    std::atomic<float>* pAmount = nullptr;
    std::atomic<float>* pRoot = nullptr;
    std::atomic<float>* pScale = nullptr;
    std::atomic<float>* pRef = nullptr;
    std::atomic<float>* pRange = nullptr;
    std::atomic<float>* pNotes[12] {};
    std::atomic<float>* pSustain = nullptr;
    std::atomic<float>* pStabilizer = nullptr;
    std::atomic<float>* pToneVol = nullptr;
    std::atomic<float>* pDblMix = nullptr;
    std::atomic<float>* pDblWidth = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetaTuneProcessor)
};
