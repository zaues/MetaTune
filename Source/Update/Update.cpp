#include "Update.h"
#include <juce_data_structures/juce_data_structures.h>

#if JUCE_WINDOWS
extern "C" __declspec (dllimport) unsigned long __stdcall GetCurrentProcessId();
#endif

#ifndef METATUNE_UPDATE_REPO
 #define METATUNE_UPDATE_REPO ""
#endif

namespace update
{
static const juce::String kAsset { "MetaTune-Setup.exe" };

juce::String feedUrl()
{
    const auto env = juce::SystemStats::getEnvironmentVariable ("METATUNE_UPDATE_FEED", {});
    if (env.isNotEmpty()) return env;
    const juce::String repo (METATUNE_UPDATE_REPO);
    return repo.isEmpty() ? juce::String() : "https://api.github.com/repos/" + repo + "/releases/latest";
}

static juce::Array<int> versionParts (juce::String v)
{
    v = v.trim().trimCharactersAtStart ("vV").upToFirstOccurrenceOf ("-", false, false);
    juce::Array<int> parts;
    for (const auto& s : juce::StringArray::fromTokens (v, ".", {}))
        parts.add (s.getIntValue());
    while (parts.size() < 3) parts.add (0);
    return parts;
}

bool isNewer (const juce::String& a, const juce::String& b)
{
    const auto x = versionParts (a), y = versionParts (b);
    for (int i = 0; i < juce::jmax (x.size(), y.size()); ++i)
        if (x[i] != y[i]) return x[i] > y[i];
    return false;
}

static bool isSha256 (const juce::String& s) { return s.length() == 64 && s.containsOnly ("0123456789abcdef"); }

std::optional<Release> parseRelease (const juce::var& json)
{
    if (! json.isObject() || json["draft"] || json["prerelease"]) return std::nullopt;
    Release r;
    r.version = json["tag_name"].toString().trimCharactersAtStart ("vV");
    if (r.version.isEmpty()) return std::nullopt;
    if (const auto* assets = json["assets"].getArray())
        for (const auto& a : *assets)
        {
            const auto name = a["name"].toString();
            if (name == kAsset)
            {
                r.downloadUrl = a["browser_download_url"].toString();
                const auto digest = a["digest"].toString(); // "sha256:<hex>" on current GitHub
                if (digest.startsWithIgnoreCase ("sha256:")) r.sha256 = digest.fromFirstOccurrenceOf (":", false, false).toLowerCase();
            }
            else if (name == kAsset + ".sha256" && r.sha256.isEmpty())
                r.sha256 = "@" + a["browser_download_url"].toString(); // resolved by fetchLatest
        }
    // Only https (or a local file while testing) is accepted for the installer itself.
    if (! (r.downloadUrl.startsWith ("https://") || r.downloadUrl.startsWith ("file:"))) return std::nullopt;
    if (r.sha256.isEmpty()) return std::nullopt;
    return r;
}

static juce::String download (const juce::String& url)
{
    if (url.startsWith ("file:")) return juce::URL (url).getLocalFile().loadFileAsString();
    auto opts = juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                    .withConnectionTimeoutMs (5000)
                    .withExtraHeaders ("User-Agent: MetaTune-Updater\r\nAccept: application/vnd.github+json");
    if (auto in = juce::URL (url).createInputStream (opts)) return in->readEntireStreamAsString();
    return {};
}

std::optional<Release> fetchLatest()
{
    const auto url = feedUrl();
    if (url.isEmpty()) return std::nullopt;
    auto r = parseRelease (juce::JSON::parse (download (url)));
    if (r && r->sha256.startsWith ("@"))
        r->sha256 = download (r->sha256.substring (1)).trim().upToFirstOccurrenceOf (" ", false, false).toLowerCase();
    if (r && ! isSha256 (r->sha256)) return std::nullopt;
    return r;
}

// ---- settings
static juce::PropertiesFile& props()
{
    static juce::PropertiesFile file ([] {
        juce::PropertiesFile::Options o;
        o.applicationName = "Updates";
        o.folderName = "MetaTune";
        o.filenameSuffix = ".settings";
        o.osxLibrarySubFolder = "Application Support";
        o.processLock = nullptr;
        return o;
    }());
    return file;
}

bool Settings::autoCheck() { return props().getBoolValue ("autoCheck", true); }
void Settings::setAutoCheck (bool on) { props().setValue ("autoCheck", on); props().saveIfNeeded(); }

juce::String Settings::notesVersion() { return props().getValue ("notesVersion"); }
void Settings::setNotesVersion (const juce::String& v) { props().setValue ("notesVersion", v); props().saveIfNeeded(); }

float Settings::accentHue()    { return (float) props().getDoubleValue ("accentHue", 0.0); }
float Settings::accentSat()    { return (float) props().getDoubleValue ("accentSat", 1.0); }
float Settings::accentBright() { return (float) props().getDoubleValue ("accentBright", 1.0); }
void Settings::setAccentPrefs (float hue, float sat, float bright)
{
    props().setValue ("accentHue", hue);
    props().setValue ("accentSat", sat);
    props().setValue ("accentBright", bright);
    props().saveIfNeeded();
}

bool Settings::lightMode() { return props().getBoolValue ("lightMode", false); }
void Settings::setLightMode (bool on) { props().setValue ("lightMode", on); props().saveIfNeeded(); }

bool  Settings::gradientOn() { return props().getBoolValue ("gradientOn", false); }
float Settings::gradHueA()   { return (float) props().getDoubleValue ("gradHueA", 0.83); }
float Settings::gradHueB()   { return (float) props().getDoubleValue ("gradHueB", 0.58); }
void  Settings::setGradient (bool on, float hueA, float hueB)
{
    props().setValue ("gradientOn", on);
    props().setValue ("gradHueA", hueA);
    props().setValue ("gradHueB", hueB);
    props().saveIfNeeded();
}

float Settings::orbHue()   { return (float) props().getDoubleValue ("orbHue", 0.0); }
float Settings::keyHue()   { return (float) props().getDoubleValue ("keyHue", 0.0); }
float Settings::meterHue() { return (float) props().getDoubleValue ("meterHue", 0.0); }
void  Settings::setElementHues (float orb, float key, float meter)
{
    props().setValue ("orbHue", orb);
    props().setValue ("keyHue", key);
    props().setValue ("meterHue", meter);
    props().saveIfNeeded();
}

static juce::String favKey (const juce::String& folder, const juce::String& name) { return folder + "\t" + name; }

bool Settings::isFavorite (const juce::String& folder, const juce::String& name)
{
    return juce::StringArray::fromLines (props().getValue ("favorites")).contains (favKey (folder, name));
}

void Settings::setFavorite (const juce::String& folder, const juce::String& name, bool on)
{
    auto f = juce::StringArray::fromLines (props().getValue ("favorites"));
    f.removeEmptyStrings();
    f.removeString (favKey (folder, name));
    if (on) f.add (favKey (folder, name));
    props().setValue ("favorites", f.joinIntoString ("\n"));
    props().saveIfNeeded();
}

std::optional<Release> Settings::cached()
{
    Release r { props().getValue ("latestVersion"), props().getValue ("latestUrl"), props().getValue ("latestSha256") };
    if (r.version.isEmpty() || ! isNewer (r.version, JucePlugin_VersionString) || ! isSha256 (r.sha256)) return std::nullopt;
    return r;
}

void Settings::store (const std::optional<Release>& r)
{
    props().setValue ("lastCheck", juce::Time::currentTimeMillis());
    props().setValue ("latestVersion", r ? r->version : juce::String());
    props().setValue ("latestUrl", r ? r->downloadUrl : juce::String());
    props().setValue ("latestSha256", r ? r->sha256 : juce::String());
    props().saveIfNeeded();
}

bool Settings::checkDue()
{
    const auto last = props().getValue ("lastCheck", "0").getLargeIntValue();
    return juce::Time::currentTimeMillis() - last > juce::int64 (6) * 3600 * 1000;
}

// ---- Auto Key bridge ----
KeyStatus readMetaKeyStatus()
{
    KeyStatus s;
    const auto f = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("MetaTune").getChildFile ("metakey.status");
    if (! f.existsAsFile()) return s;
    juce::int64 time = 0;
    for (const auto& ln : juce::StringArray::fromLines (f.loadFileAsString()))
    {
        const auto k = ln.upToFirstOccurrenceOf ("=", false, false).trim();
        const auto v = ln.fromFirstOccurrenceOf ("=", false, false).trim();
        if (k == "root") s.root = v.getIntValue();
        else if (k == "minor") s.minor = v.getIntValue() != 0;
        else if (k == "conf") s.conf = v.getFloatValue();
        else if (k == "time") time = v.getLargeIntValue();
    }
    s.live = (juce::Time::currentTimeMillis() - time) < 5000; // written within 5s = a MetaKey is running
    return s;
}

static juce::File commonVst3Dir()
{
    const auto cpf = juce::SystemStats::getEnvironmentVariable ("CommonProgramFiles", "C:\\Program Files\\Common Files");
    return juce::File (cpf).getChildFile ("VST3");
}

bool metaKeyInstalled() { return commonVst3Dir().getChildFile ("MetaKey.vst3").exists(); }

juce::File metaKeyInstaller()
{
    // Bundled by MetaTune's installer next to the updater: C:\Program Files\MetaTune\MetaKey-Setup.exe.
    return juce::File::getSpecialLocation (juce::File::globalApplicationsDirectory)
        .getChildFile (JucePlugin_Name).getChildFile ("MetaKey-Setup.exe");
}

// ---- updater hand-off
static juce::File installedUpdater()
{
    // The installer puts it next to the licenses: C:\Program Files\MetaTune\MetaTune Updater.exe.
    return juce::File::getSpecialLocation (juce::File::globalApplicationsDirectory)
        .getChildFile (JucePlugin_Name).getChildFile (juce::String (JucePlugin_Name) + " Updater.exe");
}

bool launchUpdater (const Release& r)
{
    const auto src = installedUpdater();
    if (! src.existsAsFile()) return false;
    const auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("MetaTuneUpdate");
    dir.createDirectory();
    const auto exe = dir.getChildFile (src.getFileName());
    if (! src.copyFileTo (exe)) return false;
    const auto host = juce::File::getSpecialLocation (juce::File::hostApplicationPath);
    juce::String args;
    args << "--version " << r.version.quoted() << " --url " << r.downloadUrl.quoted() << " --sha256 " << r.sha256
         << " --host " << host.getFullPathName().quoted();
   #if JUCE_WINDOWS
    args << " --pid " << juce::String ((juce::int64) GetCurrentProcessId());
   #endif
    return exe.startAsProcess (args);
}

// ---- background check
void Checker::start (std::function<void (std::optional<Release>)> callback)
{
    if (isThreadRunning()) return;
    onResult = std::move (callback);
    startThread (juce::Thread::Priority::low);
}

void Checker::run()
{
    auto latest = fetchLatest();
    if (threadShouldExit()) return;
    if (latest && ! isNewer (latest->version, JucePlugin_VersionString)) latest.reset();
    Settings::store (latest);
    juce::MessageManager::callAsync ([cb = onResult, latest] { if (cb) cb (latest); });
}
} // namespace update
