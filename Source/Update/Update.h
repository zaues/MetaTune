#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <optional>

// Update check: the editor asks GitHub Releases (in the background) whether a newer MetaTune exists and,
// on request, hands the download to "MetaTune Updater.exe", which closes the host, installs and reopens it.
// Never touches the audio thread.
namespace update
{
struct Release
{
    juce::String version;     // "0.2.0" (tag without the leading v)
    juce::String downloadUrl; // https URL of MetaTune-Setup.exe
    juce::String sha256;      // lowercase hex of that file; the updater refuses anything else
};

// Feed URL: the GitHub "latest release" API for METATUNE_UPDATE_REPO, or $METATUNE_UPDATE_FEED (testing).
// Empty when no repository is configured: the whole feature then stays off.
juce::String feedUrl();

// True when version a is newer than b ("1.10.0" > "1.9.3"; a leading "v" is ignored).
bool isNewer (const juce::String& a, const juce::String& b);

// Reads a GitHub release JSON object. Needs an asset named MetaTune-Setup.exe and its sha256
// (GitHub's asset "digest" field, or a MetaTune-Setup.exe.sha256 asset whose text is fetched by fetchLatest).
std::optional<Release> parseRelease (const juce::var& json);

// Blocking: downloads the feed and returns the latest release. Call from a background thread only.
std::optional<Release> fetchLatest();

// Per-user settings shared by every instance: %APPDATA%\MetaTune\Updates.settings.
struct Settings
{
    static bool autoCheck();
    static void setAutoCheck (bool);
    // The last release seen, so the button shows immediately on the next open without a network call.
    static std::optional<Release> cached();
    static void store (const std::optional<Release>&);
    static bool checkDue(); // more than 6 hours since the last check
};

// Copies the updater to %TEMP% (the installer replaces the original while it runs) and starts it
// with the release and this host's exe/pid. Returns false if the updater is missing.
bool launchUpdater (const Release&);

// Background check owned by the editor; calls onResult on the message thread (nullopt = up to date or failed).
class Checker : private juce::Thread
{
public:
    Checker() : juce::Thread ("MetaTune update check") {}
    ~Checker() override { stopThread (6000); }
    void start (std::function<void (std::optional<Release>)> callback);
    bool isChecking() const { return isThreadRunning(); }

private:
    void run() override;
    std::function<void (std::optional<Release>)> onResult;
};
} // namespace update
