#include "PluginProcessor.h"
#include "Parameters.h"
#include "PluginEditor.h"

namespace
{
// All plugin copies loaded in this process (a DAW normally hosts them in one process).
struct GroupHub
{
    juce::CriticalSection lock;
    juce::Array<MetaTuneProcessor*> members;
    juce::String names[4];                 // user-chosen group names (empty = default)
    static GroupHub& get() { static GroupHub hub; return hub; }
};
static std::atomic<int> sNextInstanceId { 1 };
} // namespace


MetaTuneProcessor::MetaTuneProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, &undoManager, "STATE", createParameterLayout())
{
    bypassParam = apvts.getParameter (ParamID::bypass);
    pBypass = apvts.getRawParameterValue (ParamID::bypass);
    pSpeed = apvts.getRawParameterValue (ParamID::speed);
    pAmount = apvts.getRawParameterValue (ParamID::amount);
    pRoot = apvts.getRawParameterValue (ParamID::root);
    pScale = apvts.getRawParameterValue (ParamID::scale);
    pRef = apvts.getRawParameterValue (ParamID::referenceHz);
    pRange = apvts.getRawParameterValue (ParamID::range);
    for (int pc = 0; pc < 12; ++pc)
        pNotes[pc] = apvts.getRawParameterValue (ParamID::note (pc));
    pSustain = apvts.getRawParameterValue (ParamID::sustain);
    pStabilizer = apvts.getRawParameterValue (ParamID::stabilizer);
    pToneVol = apvts.getRawParameterValue (ParamID::toneVolume);
    pDblMix = apvts.getRawParameterValue (ParamID::doublerMix);
    pDblWidth = apvts.getRawParameterValue (ParamID::doublerWidth);
    chain.setTelemetryQueue (&telemetry);
    {
        const juce::ScopedLock sl (GroupHub::get().lock);
        instanceId = sNextInstanceId++;
        GroupHub::get().members.add (this);
    }
    startTimerHz (20);
}

MetaTuneProcessor::~MetaTuneProcessor()
{
    stopTimer();
    const juce::ScopedLock sl (GroupHub::get().lock);
    GroupHub::get().members.removeAllInstancesOf (this);
}

int MetaTuneProcessor::groupMemberCount (int group)
{
    const juce::ScopedLock sl (GroupHub::get().lock);
    int n = 0;
    for (auto* m : GroupHub::get().members)
        n += m->ui.group == group;
    return n;
}

std::vector<float> MetaTuneProcessor::linkedValues() const
{
    std::vector<float> v;
    for (auto* p : getParameters())
        v.push_back (p == bypassParam ? 0.0f : p->getValue());
    return v;
}

void MetaTuneProcessor::receiveGroupValues (const std::vector<float>& v)
{
    const auto params = getParameters();
    for (int i = 0; i < params.size() && i < int (v.size()); ++i)
        if (params[i] != bypassParam && std::abs (params[i]->getValue() - v[size_t (i)]) > 1e-6f)
            params[i]->setValueNotifyingHost (v[size_t (i)]);
    // Record what we actually hold (parameters may snap the value): otherwise our own timer would see a
    // "change", send it back, and two copies could ping-pong forever.
    groupSynced = linkedValues();
}

void MetaTuneProcessor::joinGroup (int group)
{
    const juce::ScopedLock sl (GroupHub::get().lock);
    ui.group = group == ui.group ? -1 : juce::jlimit (-1, 3, group);
    if (ui.group < 0) return;
    for (auto* m : GroupHub::get().members)
        if (m != this && m->ui.group == ui.group)
        {
            receiveGroupValues (m->linkedValues()); // adopt the group's sound
            return;
        }
    groupSynced = linkedValues();
}

void MetaTuneProcessor::syncGroup()
{
    if (ui.group < 0) return;
    const auto now = linkedValues();
    if (now == groupSynced) return;
    groupSynced = now;
    const juce::ScopedLock sl (GroupHub::get().lock);
    for (auto* m : GroupHub::get().members)
        if (m != this && m->ui.group == ui.group)
            m->receiveGroupValues (now);
}

void MetaTuneProcessor::setGroup (int group)
{
    const juce::ScopedLock sl (GroupHub::get().lock);
    ui.group = juce::jlimit (-1, 3, group);
    if (ui.group < 0) { groupSynced = linkedValues(); return; }
    for (auto* m : GroupHub::get().members)
        if (m != this && m->ui.group == ui.group) { receiveGroupValues (m->linkedValues()); return; }
    groupSynced = linkedValues();
}

void MetaTuneProcessor::updateTrackProperties (const TrackProperties& properties)
{
    const juce::ScopedLock sl (GroupHub::get().lock);
    trackName = properties.name.value_or (juce::String {});
}

std::vector<MetaTuneProcessor::GroupInstanceInfo> MetaTuneProcessor::groupInstances() const
{
    const juce::ScopedLock sl (GroupHub::get().lock);
    std::vector<GroupInstanceInfo> out;
    for (auto* m : GroupHub::get().members)
        out.push_back ({ m->instanceId, m->ui.group, m->trackName.isNotEmpty() ? m->trackName : m->ui.presetName });
    std::sort (out.begin(), out.end(), [] (const GroupInstanceInfo& a, const GroupInstanceInfo& b) { return a.id < b.id; });
    return out;
}

void MetaTuneProcessor::setInstanceGroup (int id, int group)
{
    const juce::ScopedLock sl (GroupHub::get().lock);
    for (auto* m : GroupHub::get().members)
        if (m->instanceId == id) { m->setGroup (group); return; }
}

juce::String MetaTuneProcessor::groupName (int group)
{
    if (! juce::isPositiveAndBelow (group, 4)) return {};
    const juce::ScopedLock sl (GroupHub::get().lock);
    const auto n = GroupHub::get().names[group];
    return n.isNotEmpty() ? n : "Group " + juce::String (group + 1);
}

void MetaTuneProcessor::setGroupName (int group, const juce::String& name)
{
    if (! juce::isPositiveAndBelow (group, 4)) return;
    const juce::ScopedLock sl (GroupHub::get().lock);
    GroupHub::get().names[group] = name;
}

void MetaTuneProcessor::prepareToPlay (double sr, int)
{
    // Allocate for the lowest range so switching Range never allocates on the audio thread.
    chain.prepare (sr, kRangeMinHz[0]);
    setLatencySamples (metatune::TunerChain::latencyFor (sr, rangeMinHz (pRange->load())));
    preparedRate = sr;
}

void MetaTuneProcessor::timerCallback()
{
    syncGroup();
    // shortcut: latency is re-reported up to 100 ms after a Range change; DSP switches immediately.
    if (const double sr = preparedRate.load(); sr > 0)
        if (const int lat = metatune::TunerChain::latencyFor (sr, rangeMinHz (pRange->load())); lat != getLatencySamples())
            setLatencySamples (lat);
}

bool MetaTuneProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
        && layouts.getMainInputChannelSet() == out;
}

void MetaTuneProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) { process (buffer, false); }
void MetaTuneProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) { process (buffer, true); }

void MetaTuneProcessor::process (juce::AudioBuffer<float>& buffer, bool forceBypass) noexcept
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh = std::min (metatune::TunerChain::kMaxChannels, getTotalNumInputChannels());
    for (auto ch = numCh; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    metatune::TunerChain::Settings s;
    s.correction.speedMs = pSpeed->load();
    s.correction.amount = pAmount->load() / 100.0;
    s.correction.referenceHz = pRef->load();
    s.correction.mask = currentMask();
    s.minFreq = rangeMinHz (pRange->load());
    s.bypass = forceBypass || pBypass->load() > 0.5f;
    s.correction.sustain = pSustain->load() / 100.0;
    s.correction.stabilizerMs = kStabilizerMs[juce::jlimit (0, 3, int (pStabilizer->load()))];
    s.doublerMix = pDblMix->load() / 100.0;
    s.doublerWidth = pDblWidth->load() / 100.0;
    s.toneNote = toneNote.load();
    s.toneChord = toneChord.load();
    s.toneGain = juce::Decibels::decibelsToGain (double (pToneVol->load()), -60.0);
    chain.setSettings (s);
    chain.process (buffer.getArrayOfWritePointers(), numCh, buffer.getNumSamples());
}

metatune::PitchClassMask MetaTuneProcessor::currentMask() const noexcept
{
    const auto type = metatune::ScaleType (juce::jlimit (0, 3, int (pScale->load())));
    metatune::PitchClassMask custom = 0;
    for (int pc = 0; pc < 12; ++pc)
        custom |= metatune::PitchClassMask ((pNotes[pc]->load() > 0.5f ? 1u : 0u) << pc);
    return metatune::scaleMask (type, juce::jlimit (0, 11, int (pRoot->load())), custom);
}

double MetaTuneProcessor::latencyMsFor (int rangeIndex) const noexcept
{
    const double sr = preparedRate.load() > 0 ? preparedRate.load() : 48000.0;
    return 1000.0 * metatune::TunerChain::latencyFor (sr, rangeMinHz (float (rangeIndex))) / sr;
}

void MetaTuneProcessor::resetAllParameters()
{
    ui.scaleBeforeChromatic.reset();
    undoManager.beginNewTransaction ("Reset All");
    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p); rp != nullptr && p != bypassParam)
        {
            rp->beginChangeGesture();
            rp->setValueNotifyingHost (rp->getDefaultValue());
            rp->endChangeGesture();
        }
}

void MetaTuneProcessor::switchAB()
{
    abSnapshot[abSlot] = apvts.copyState();
    abSlot ^= 1;
    if (abSnapshot[abSlot].isValid())
        apvts.replaceState (abSnapshot[abSlot].createCopy());
}

void MetaTuneProcessor::copyToOtherAB()
{
    abSnapshot[abSlot ^ 1] = apvts.copyState();
}


namespace
{
struct FactoryPreset { const char* name; std::initializer_list<std::pair<const char*, float>> values; };
// Speed, Amount, Sustain, Note Stabilizer, Doubler. Key and scale are left as they are.
const FactoryPreset kFactory[] {
    { "Natural Pop",   { { "speed", 40 }, { "amount", 85 }, { "sustain", 30 }, { "stabilizer", 1 }, { "dblMix", 0 }, { "dblWidth", 0 } } },
    { "Hard Tune",     { { "speed", -3 }, { "amount", 100 }, { "sustain", 0 }, { "stabilizer", 0 }, { "dblMix", 0 }, { "dblWidth", 0 } } },
    { "Smooth Ballad", { { "speed", 120 }, { "amount", 70 }, { "sustain", 50 }, { "stabilizer", 2 }, { "dblMix", 0 }, { "dblWidth", 0 } } },
    { "Trap Lead",     { { "speed", 0 }, { "amount", 100 }, { "sustain", 20 }, { "stabilizer", 1 }, { "dblMix", 25 }, { "dblWidth", 60 } } },
    { "Wide Doubles",  { { "speed", 20 }, { "amount", 100 }, { "sustain", 30 }, { "stabilizer", 1 }, { "dblMix", 60 }, { "dblWidth", 100 } } },
    { "Subtle Polish", { { "speed", 200 }, { "amount", 50 }, { "sustain", 40 }, { "stabilizer", 3 }, { "dblMix", 0 }, { "dblWidth", 0 } } },
    { "Robot",         { { "speed", -3 }, { "amount", 100 }, { "sustain", 100 }, { "stabilizer", 0 }, { "dblMix", 15 }, { "dblWidth", 30 } } },
};
// Always left as-is when loading a preset (not musical content / latency-affecting).
bool alwaysKeeps (const juce::String& id) { return id == "bypass" || id == "range"; }
// Key/scale content: kept only while "Keep My Scale" is on.
bool isScaleParam (const juce::String& id) { return id == "root" || id == "scale" || id.startsWith ("note"); }
} // namespace

juce::File MetaTuneProcessor::presetFolder()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile (juce::String (JucePlugin_Name) + " Presets");
}

int MetaTuneProcessor::numFactoryPresets() const noexcept { return int (std::size (kFactory)); }

juce::StringArray MetaTuneProcessor::presetFolders() const
{
    juce::StringArray out;
    for (const auto& d : presetFolder().findChildFiles (juce::File::findDirectories, false))
        out.add (d.getFileName());
    out.sortNatural();
    return out;
}

static juce::StringArray xmlNamesIn (const juce::File& dir)
{
    juce::StringArray names;
    for (const auto& f : dir.findChildFiles (juce::File::findFiles, false, "*.xml"))
        names.add (f.getFileNameWithoutExtension());
    names.sortNatural();
    return names;
}

std::vector<MetaTuneProcessor::PresetEntry> MetaTuneProcessor::presetList() const
{
    std::vector<PresetEntry> list;
    for (const auto& f : kFactory) list.push_back ({ f.name, {}, true });
    const auto root = presetFolder();
    for (const auto& fld : presetFolders())
        for (const auto& n : xmlNamesIn (root.getChildFile (fld)))
            list.push_back ({ n, fld, false });
    for (const auto& n : xmlNamesIn (root))
        list.push_back ({ n, {}, false });
    return list;
}

juce::StringArray MetaTuneProcessor::presetNames() const
{
    juce::StringArray names;
    for (const auto& e : presetList()) names.add (e.name);
    return names;
}

// Full path of a user preset entry; invalid for factory entries.
static juce::File presetFile (const MetaTuneProcessor::PresetEntry& e)
{
    const auto base = e.folder.isEmpty() ? MetaTuneProcessor::presetFolder()
                                         : MetaTuneProcessor::presetFolder().getChildFile (e.folder);
    return base.getChildFile (e.name + ".xml");
}

void MetaTuneProcessor::applyValues (const juce::StringPairArray& values)
{
    for (const auto& id : values.getAllKeys())
        if (! (alwaysKeeps (id) || (ui.keepScale && isScaleParam (id))))
            if (auto* p = apvts.getParameter (id))
            {
                p->beginChangeGesture();
                p->setValueNotifyingHost (p->convertTo0to1 (values[id].getFloatValue()));
                p->endChangeGesture();
            }
}

void MetaTuneProcessor::loadPreset (int index)
{
    const auto list = presetList();
    if (! juce::isPositiveAndBelow (index, int (list.size()))) return;
    const auto& e = list[size_t (index)];
    juce::StringPairArray values;
    if (e.factory)
    {
        for (const auto& [id, v] : kFactory[index].values) values.set (id, juce::String (v));
    }
    else
    {
        const auto xml = juce::XmlDocument::parse (presetFile (e));
        if (xml == nullptr) return;
        for (const auto* param : xml->getChildWithTagNameIterator ("PARAM"))
            values.set (param->getStringAttribute ("id"), param->getStringAttribute ("value"));
    }
    undoManager.beginNewTransaction ("Load preset");
    applyValues (values);
    ui.presetName = e.name;
}

bool MetaTuneProcessor::savePreset (const juce::String& rawName, const juce::String& rawFolder)
{
    const auto name = juce::File::createLegalFileName (rawName.trim());
    if (name.isEmpty()) return false;
    auto dir = presetFolder();
    if (rawFolder.isNotEmpty()) dir = dir.getChildFile (juce::File::createLegalFileName (rawFolder.trim()));
    if (! dir.createDirectory()) return false;
    const auto xml = apvts.copyState().createXml();
    if (xml == nullptr || ! xml->writeTo (dir.getChildFile (name + ".xml"))) return false;
    ui.presetName = name;
    return true;
}

bool MetaTuneProcessor::createFolder (const juce::String& rawName)
{
    const auto name = juce::File::createLegalFileName (rawName.trim());
    if (name.isEmpty()) return false;
    return presetFolder().getChildFile (name).createDirectory();
}

void MetaTuneProcessor::deletePreset (int index)
{
    const auto list = presetList();
    if (! juce::isPositiveAndBelow (index, int (list.size()))) return;
    const auto& e = list[size_t (index)];
    if (e.factory) return;
    presetFile (e).moveToTrash();
    if (ui.presetName == e.name) ui.presetName = {};
}

void MetaTuneProcessor::deleteFolder (const juce::String& folder)
{
    if (folder.isNotEmpty())
        presetFolder().getChildFile (folder).moveToTrash();
}

void MetaTuneProcessor::movePreset (int index, const juce::String& toFolder)
{
    const auto list = presetList();
    if (! juce::isPositiveAndBelow (index, int (list.size()))) return;
    const auto& e = list[size_t (index)];
    if (e.factory || e.folder == toFolder) return;
    auto dstDir = toFolder.isEmpty() ? presetFolder() : presetFolder().getChildFile (toFolder);
    dstDir.createDirectory();
    presetFile (e).moveFileTo (dstDir.getChildFile (e.name + ".xml"));
}

// State format v1: <METATUNE version="1"><STATE .../><AB slot=".."><A/><B/></AB><UI .../></METATUNE>.
// Also reads the older root tag and the pre-v1 format (a bare <STATE>), so existing projects still load.
// Unknown/missing pieces fall back to defaults.
static constexpr const char* kStateTag = "METATUNE";
static constexpr const char* kLegacyStateTag = "LYRA";
void MetaTuneProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::XmlElement root (kStateTag);
    root.setAttribute ("version", 1);
    if (auto st = apvts.copyState().createXml())
        root.addChildElement (st.release());

    auto* ab = root.createNewChildElement ("AB");
    ab->setAttribute ("slot", abSlot);
    for (int i = 0; i < 2; ++i)
        if (abSnapshot[i].isValid())
            if (auto x = abSnapshot[i].createXml())
                ab->createNewChildElement (i == 0 ? "A" : "B")->addChildElement (x.release());

    auto* u = root.createNewChildElement ("UI");
    u->setAttribute ("heatmaps", ui.heatmaps);
    u->setAttribute ("detected", ui.detectedNotes);
    u->setAttribute ("tooltips", ui.tooltips);
    u->setAttribute ("scale", ui.scale);
    u->setAttribute ("majorHold", ui.majorHold);
    u->setAttribute ("minorHold", ui.minorHold);
    u->setAttribute ("group", ui.group);
    u->setAttribute ("flats", ui.useFlats);
    u->setAttribute ("scaleLabel", ui.scaleLabel);
    u->setAttribute ("preset", ui.presetName);
    u->setAttribute ("keepScale", ui.keepScale);
    u->setAttribute ("scaleSet", ui.scaleEverSet);
    if (ui.scaleBeforeChromatic && pScale->load() == float (metatune::ScaleType::Chromatic))
    {
        const auto& saved = *ui.scaleBeforeChromatic;
        auto* previous = u->createNewChildElement ("BeforeChromatic");
        previous->setAttribute ("type", int (saved.type));
        previous->setAttribute ("root", saved.root);
        previous->setAttribute ("mask", int (saved.mask));
        previous->setAttribute ("label", saved.label);
    }
    copyXmlToBinary (root, dest);
}

void MetaTuneProcessor::setStateInformation (const void* data, int size)
{
    const auto xml = getXmlFromBinary (data, size);
    if (xml == nullptr)
        return;
    const bool wrapped = xml->hasTagName (kStateTag) || xml->hasTagName (kLegacyStateTag);
    const auto* st = wrapped ? xml->getChildByName (apvts.state.getType()) : xml.get();
    if (st != nullptr && st->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*st));

    if (const auto* ab = xml->getChildByName ("AB"))
    {
        abSlot = juce::jlimit (0, 1, ab->getIntAttribute ("slot"));
        for (int i = 0; i < 2; ++i)
        {
            abSnapshot[i] = {};
            if (const auto* holder = ab->getChildByName (i == 0 ? "A" : "B"))
                if (const auto* snap = holder->getChildByName (apvts.state.getType()))
                    abSnapshot[i] = juce::ValueTree::fromXml (*snap);
        }
    }
    ui.scaleBeforeChromatic.reset();
    if (const auto* u = xml->getChildByName ("UI"))
    {
        ui.heatmaps = u->getBoolAttribute ("heatmaps", true);
        ui.detectedNotes = u->getBoolAttribute ("detected", true);
        ui.tooltips = u->getBoolAttribute ("tooltips", true);
        ui.scale = juce::jlimit (0.6f, 1.5f, float (u->getDoubleAttribute ("scale", 0.85)));
        ui.useFlats = u->getBoolAttribute ("flats", false);
        ui.scaleLabel = u->getStringAttribute ("scaleLabel");
        ui.presetName = u->getStringAttribute ("preset");
        ui.keepScale = u->getBoolAttribute ("keepScale", true);
        ui.scaleEverSet = u->getBoolAttribute ("scaleSet", false);
        ui.majorHold = u->getBoolAttribute ("majorHold", false);
        ui.minorHold = u->getBoolAttribute ("minorHold", false);
        ui.group = juce::jlimit (-1, 3, u->getIntAttribute ("group", -1));
        if (const auto* previous = u->getChildByName ("BeforeChromatic"))
        {
            const int type = previous->getIntAttribute ("type");
            if (type >= int (metatune::ScaleType::Major) && type <= int (metatune::ScaleType::Custom))
                ui.scaleBeforeChromatic = UiSettings::SavedScale {
                    metatune::ScaleType (type), juce::jlimit (0, 11, previous->getIntAttribute ("root")),
                    metatune::PitchClassMask (previous->getIntAttribute ("mask") & metatune::kAllNotes),
                    previous->getStringAttribute ("label")
                };
        }
        groupSynced = linkedValues(); // a loaded project keeps its own settings
    }
    // Migration / safety: a project that already has a non-default key or scale has clearly been used,
    // so never show the first-run prompt over it even if the flag predates this build.
    if (! ui.scaleEverSet && (pScale->load() != 0.0f || pRoot->load() != 0.0f))
        ui.scaleEverSet = true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MetaTuneProcessor(); }

juce::AudioProcessorEditor* MetaTuneProcessor::createEditor() { return new MetaTuneEditor (*this); }
