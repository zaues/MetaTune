// Renders the MetaKey editor to PNG for visual review. usage: metakey_snapshot.exe <out-dir> [listen|file|manual]
#include "../Source/MetaKey/MetaKeyProcessor.h"
#include "../Source/MetaKey/MetaKeyEditor.h"
#include <juce_gui_basics/juce_gui_basics.h>

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File outDir (argc > 1 ? juce::String (argv[1]) : juce::File::getCurrentWorkingDirectory().getFullPathName());
    const juce::String mode = argc > 2 ? juce::String (argv[2]) : juce::String();

    MetaKeyProcessor proc;
    proc.setPlayConfigDetails (2, 2, 48000, 128);
    proc.prepareToPlay (48000, 128);
    if (mode == "manual") { proc.setMode (MetaKeyProcessor::Mode::Manual); proc.setManualKey (6, true); }
    else if (mode == "file") proc.setMode (MetaKeyProcessor::Mode::File);

    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditorAndMakeActive());
    for (int i = 0; i < 4; ++i) { juce::Thread::sleep (15); juce::Timer::callPendingTimersSynchronously(); }

    const auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
    const auto file = outDir.getChildFile ("metakey_" + (mode.isEmpty() ? juce::String ("listen") : mode) + ".png");
    file.deleteFile();
    juce::FileOutputStream os (file);
    juce::PNGImageFormat().writeImageToStream (img, os);
    std::printf ("wrote %s (%dx%d)\n", file.getFullPathName().toRawUTF8(), img.getWidth(), img.getHeight());
    ed.reset();
    return 0;
}
