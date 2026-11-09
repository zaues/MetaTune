#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "KeyDetector.h"
#include <array>
#include <atomic>

// MetaKey: a key-detection effect. Audio passes through untouched; a background thread analyses it
// (Listen) or an imported file (Use File), or the user sets the key by hand (Manual). The detected key
// is published atomically for the editor.
class MetaKeyProcessor : public juce::AudioProcessor, private juce::Thread
{
public:
    enum class Mode { Listen, File, Manual };

    MetaKeyProcessor();
    ~MetaKeyProcessor() override;

    void prepareToPlay (double sampleRate, int maxBlockSize) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    // --- UI control (message thread) ---
    Mode mode() const noexcept { return currentMode; }
    void setMode (Mode m);
    void restartListen();                      // clear the accumulated chroma and detect fresh
    void analyseFile (const juce::File&);       // Use File: analyse the whole file off-thread
    void setManualKey (int root, bool minor);   // Manual
    bool fileBusy() const noexcept { return analysing.load(); }
    juce::String fileName() const;

    // --- Results (any thread) ---
    KeyDetector::Result result() const;         // the current detected / manual key
    std::array<float, 12> chroma() const;       // for the display bars
    bool hasSignal() const noexcept { return signalSeen.load(); }

private:
    void run() override;                        // analysis thread
    void publish (const KeyDetector::Result&);
    bool readFileMono (const juce::File&, juce::AudioBuffer<float>& out, double& fileSr);

    KeyDetector detector;
    double sr = 44100.0;

    // Lock-free capture: processBlock writes mono samples, the analysis thread reads them.
    juce::AbstractFifo fifo { 1 << 16 };
    std::vector<float> capture;
    std::atomic<bool> signalSeen { false };

    std::atomic<Mode> currentMode { Mode::Listen };
    std::atomic<bool> restartFlag { false };
    std::atomic<bool> analysing { false };

    juce::CriticalSection fileLock;
    juce::File pendingFile, loadedFile;

    // Published result.
    std::atomic<int> outRoot { -1 };
    std::atomic<bool> outMinor { false };
    std::atomic<float> outConf { 0.0f };
    std::atomic<int> manualRoot { 0 };
    std::atomic<bool> manualMinor { false };

    juce::CriticalSection chromaLock;
    std::array<float, 12> chromaOut {};

    juce::AudioFormatManager formats;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetaKeyProcessor)
};
