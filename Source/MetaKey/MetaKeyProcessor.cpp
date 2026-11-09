#include "MetaKeyProcessor.h"
#include "MetaKeyEditor.h"

MetaKeyProcessor::MetaKeyProcessor()
    : juce::AudioProcessor (BusesProperties()
          .withInput ("In", juce::AudioChannelSet::stereo(), true)
          .withOutput ("Out", juce::AudioChannelSet::stereo(), true)),
      juce::Thread ("MetaKey analysis")
{
    formats.registerBasicFormats();
    capture.assign ((size_t) fifo.getTotalSize(), 0.0f);
    startThread (juce::Thread::Priority::low);
}

MetaKeyProcessor::~MetaKeyProcessor()
{
    stopThread (2000);
}

void MetaKeyProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    detector.prepare (sr);
    fifo.reset();
}

bool MetaKeyProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    if (in.isDisabled() || out.isDisabled()) return false;
    return in == out && (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo());
}

void MetaKeyProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();
    if (n <= 0 || ch <= 0) return; // audio passes through untouched (no clearing needed)

    // Mix to mono and push into the capture FIFO for the analysis thread.
    float peak = 0.0f;
    int s1, sz1, s2, sz2;
    fifo.prepareToWrite (n, s1, sz1, s2, sz2);
    auto writeOne = [&] (int dst, int src)
    {
        float m = 0.0f;
        for (int c = 0; c < ch; ++c) m += buffer.getSample (c, src);
        m /= float (ch);
        peak = juce::jmax (peak, std::abs (m));
        capture[(size_t) dst] = m;
    };
    for (int i = 0; i < sz1; ++i) writeOne (s1 + i, i);
    for (int i = 0; i < sz2; ++i) writeOne (s2 + i, sz1 + i);
    fifo.finishedWrite (sz1 + sz2);
    if (peak > 0.0015f) signalSeen.store (true);
}

void MetaKeyProcessor::setMode (Mode m)
{
    currentMode.store (m);
    if (m == Mode::Listen) restartFlag.store (true);
}

void MetaKeyProcessor::restartListen()
{
    signalSeen.store (false);
    restartFlag.store (true);
}

void MetaKeyProcessor::analyseFile (const juce::File& f)
{
    const juce::ScopedLock sl (fileLock);
    pendingFile = f;
}

void MetaKeyProcessor::setManualKey (int root, bool minor)
{
    manualRoot.store (juce::jlimit (0, 11, root));
    manualMinor.store (minor);
}

juce::String MetaKeyProcessor::fileName() const
{
    const juce::ScopedLock sl (fileLock);
    return loadedFile.getFileName();
}

KeyDetector::Result MetaKeyProcessor::result() const
{
    if (currentMode.load() == Mode::Manual)
        return { manualRoot.load(), manualMinor.load(), 1.0f };
    return { outRoot.load(), outMinor.load(), outConf.load() };
}

std::array<float, 12> MetaKeyProcessor::chroma() const
{
    const juce::ScopedLock sl (chromaLock);
    return chromaOut;
}

void MetaKeyProcessor::publish (const KeyDetector::Result& r)
{
    outRoot.store (r.root);
    outMinor.store (r.minor);
    outConf.store (r.confidence);
}

// The shared hand-off to MetaTune: a tiny file in %APPDATA%\MetaTune that MetaTune's Auto Key panel reads.
// A fresh timestamp also tells MetaTune that a MetaKey instance is live.
void MetaKeyProcessor::writeStatus()
{
    const auto r = result();
    auto f = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                 .getChildFile ("MetaTune").getChildFile ("metakey.status");
    f.getParentDirectory().createDirectory();
    juce::String s;
    s << "root=" << r.root << "\n"
      << "minor=" << (r.minor ? 1 : 0) << "\n"
      << "conf=" << juce::String (r.confidence, 3) << "\n"
      << "mode=" << (int) currentMode.load() << "\n"
      << "time=" << juce::Time::currentTimeMillis() << "\n";
    f.replaceWithText (s);
}

void MetaKeyProcessor::run()
{
    std::vector<float> tmp;
    while (! threadShouldExit())
    {
        // --- Use File: analyse a pending import ---
        juce::File job;
        { const juce::ScopedLock sl (fileLock); job = pendingFile; pendingFile = juce::File(); }
        if (job.existsAsFile())
        {
            analysing.store (true);
            juce::AudioBuffer<float> mono;
            double fsr = 44100.0;
            if (readFileMono (job, mono, fsr))
            {
                KeyDetector fd;
                fd.prepare (fsr);
                const int block = 1 << 14;
                for (int p = 0; p < mono.getNumSamples() && ! threadShouldExit(); p += block)
                {
                    const int len = juce::jmin (block, mono.getNumSamples() - p);
                    fd.accumulate (mono.getReadPointer (0) + p, len);
                }
                { const juce::ScopedLock sl (chromaLock); chromaOut = fd.chroma(); }
                publish (fd.estimate());
                { const juce::ScopedLock sl (fileLock); loadedFile = job; }
            }
            analysing.store (false);
        }

        // --- Listen: drain captured audio into the live detector ---
        const int ready = fifo.getNumReady();
        if (ready > 0)
        {
            if (restartFlag.exchange (false)) detector.reset();
            int s1, sz1, s2, sz2;
            fifo.prepareToRead (ready, s1, sz1, s2, sz2);
            if (currentMode.load() == Mode::Listen)
            {
                tmp.resize ((size_t) ready);
                for (int i = 0; i < sz1; ++i) tmp[(size_t) i] = capture[(size_t) (s1 + i)];
                for (int i = 0; i < sz2; ++i) tmp[(size_t) (sz1 + i)] = capture[(size_t) (s2 + i)];
            }
            fifo.finishedRead (sz1 + sz2);
            if (currentMode.load() == Mode::Listen)
            {
                detector.accumulate (tmp.data(), ready);
                { const juce::ScopedLock sl (chromaLock); chromaOut = detector.chroma(); }
                publish (detector.estimate());
            }
        }
        else if (restartFlag.exchange (false))
        {
            detector.reset();
        }

        // Publish the current key to the shared file for MetaTune, at most twice a second.
        const juce::int64 now = juce::Time::currentTimeMillis();
        if (now - lastStatusMs > 500)
        {
            lastStatusMs = now;
            writeStatus();
        }

        wait (40);
    }
}

bool MetaKeyProcessor::readFileMono (const juce::File& f, juce::AudioBuffer<float>& out, double& fileSr)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (f));
    if (reader == nullptr) return false;
    fileSr = reader->sampleRate > 0 ? reader->sampleRate : 44100.0;
    const juce::int64 cap = juce::int64 (fileSr * 120.0); // analyse up to the first two minutes
    const int total = (int) juce::jmin ((juce::int64) reader->lengthInSamples, cap);
    if (total <= 0) return false;
    const int chans = (int) juce::jmax ((juce::uint32) 1, reader->numChannels);
    juce::AudioBuffer<float> raw (chans, total);
    reader->read (&raw, 0, total, 0, true, true);
    out.setSize (1, total);
    out.clear();
    for (int c = 0; c < chans; ++c) out.addFrom (0, 0, raw, c, 0, total, 1.0f / float (chans));
    return true;
}

void MetaKeyProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::ValueTree s ("MetaKey");
    s.setProperty ("mode", (int) currentMode.load(), nullptr);
    s.setProperty ("manualRoot", manualRoot.load(), nullptr);
    s.setProperty ("manualMinor", manualMinor.load(), nullptr);
    juce::MemoryOutputStream os (dest, false);
    s.writeToStream (os);
}

void MetaKeyProcessor::setStateInformation (const void* data, int size)
{
    auto s = juce::ValueTree::readFromData (data, (size_t) size);
    if (! s.isValid()) return;
    currentMode.store ((Mode) (int) s.getProperty ("mode", (int) Mode::Listen));
    manualRoot.store ((int) s.getProperty ("manualRoot", 0));
    manualMinor.store ((bool) s.getProperty ("manualMinor", false));
}

juce::AudioProcessorEditor* MetaKeyProcessor::createEditor() { return new MetaKeyEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MetaKeyProcessor(); }
