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
