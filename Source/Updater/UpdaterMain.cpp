// MetaTune Updater: started by the plugin's "Update" button (from a %TEMP% copy, so the installer can replace the
// original). Downloads the new installer, checks its SHA-256, asks the host to close (it offers to save), installs
// silently, then offers to reopen the host. One branded window, no installer wizard.
//
// Args: --version 0.2.0 --url <https://.../MetaTune-Setup.exe> --sha256 <hex> [--host <exe>] [--pid <n>]
// Dev:  --snapshot <out.png> <state>   renders the window in a state and exits (states: download, close, closing,
//                                      install, done, failed)
//       --no-elevate                   runs the installer without the UAC prompt (testing with a dummy installer)
//       --auto-result <file>           clicks through by itself and writes "done" or "failed: ..." to <file> (tests)
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_cryptography/juce_cryptography.h>
#include "../UI/Theme.h"
#include "HostProcess.h"

namespace
{
enum class Stage { Download, Verify, NeedClose, Closing, Install, Done, Failed };

struct Job
{
    juce::String version, url, sha256, hostExe;
    unsigned long pid = 0;
    bool elevate = true;
    juce::File autoResult;
    juce::String hostName() const
    {
        const auto n = juce::File (hostExe).getFileNameWithoutExtension();
        return n.startsWithIgnoreCase ("FL") ? juce::String ("FL Studio") : (n.isNotEmpty() ? n : juce::String ("your DAW"));
    }
};

// Flat pill button in the plugin's style: primary = signature gradient, secondary = dark outline.
class Btn : public juce::Button
{
public:
    Btn (const juce::String& t, bool primary) : juce::Button (t), primary (primary) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics& g, bool over, bool down) override
    {
        using namespace theme;
        const auto r = getLocalBounds().toFloat().reduced (1.0f);
        const float rad = r.getHeight() * 0.5f;
        if (primary)
        {
            g.setGradientFill (juce::ColourGradient (accentAt (0.0f), r.getX(), r.getCentreY(), accentAt (1.0f), r.getRight(), r.getCentreY(), false));
            g.fillRoundedRectangle (r, rad);
        }
        else
        {
            g.setColour (bg3);
            g.fillRoundedRectangle (r, rad);
            g.setColour (line);
            g.drawRoundedRectangle (r, rad, 1.0f);
        }
        if (over || down)
        {
            g.setColour (juce::Colours::white.withAlpha (down ? 0.16f : 0.08f));
            g.fillRoundedRectangle (r, rad);
        }
        g.setColour (primary ? juce::Colours::white : theme::text);
        g.setFont (font (13.5f, true));
        g.drawText (getButtonText(), r, juce::Justification::centred);
    }

private:
    bool primary;
};

class UpdaterView : public juce::Component, private juce::Timer, private juce::Thread
{
public:
    static constexpr int W = 460, H = 300;

    explicit UpdaterView (Job j) : juce::Thread ("MetaTune update"), job (std::move (j))
    {
        setSize (W, H);
        for (auto* b : std::initializer_list<juce::Component*> { &primary, &secondary, &closeX }) addChildComponent (b);
        closeX.setVisible (true);
        closeX.onClick = [this] { quit(); };
        startTimerHz (60);
    }
    ~UpdaterView() override { stopThread (4000); }

    void begin() { set (Stage::Download); startThread(); }
    void showStage (Stage s, float progress) { stage = s; downloaded = progress; layoutButtons(); repaint(); } // snapshots

    void paint (juce::Graphics& g) override
    {
        using namespace theme;
        const auto b = getLocalBounds().toFloat();
        g.setColour (bg1);
        g.fillRoundedRectangle (b, 12.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0x553d2f6b), b.getCentreX(), 40.0f, juce::Colour (0x003d2f6b), b.getCentreX(), 260.0f, true));
        g.fillRoundedRectangle (b, 12.0f);
        g.setColour (line);
        g.drawRoundedRectangle (b.reduced (0.5f), 12.0f, 1.0f);

        // Header: wordmark + "Updater".
        juce::AttributedString word;
        word.append ("meta", font (20.0f), text.withAlpha (0.85f));
        word.append ("Tune", font (20.0f, true), text);
        word.append ("  Updater", font (13.0f), textFaint);
        word.setJustification (juce::Justification::centredLeft);
        word.draw (g, { 22.0f, 14.0f, 300.0f, 28.0f });

        // Centre visual: spinner while busy, ring + progress while downloading, check / cross at the end.
        const juce::Point<float> c (b.getCentreX(), 118.0f);
        const float r = 34.0f;
        g.setColour (bg3);
        g.drawEllipse (juce::Rectangle<float> (2 * r, 2 * r).withCentre (c), 5.0f);
        auto arc = [&] (float from, float to) {
            juce::Path p;
            p.addCentredArc (c.x, c.y, r, r, 0.0f, from, to, true);
            g.setGradientFill (juce::ColourGradient (accentAt (0.0f), c.x - r, c.y, accentAt (1.0f), c.x + r, c.y, false));
            g.strokePath (p, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        };
        if (stage == Stage::Done)
        {
            arc (0.0f, juce::MathConstants<float>::twoPi);
            juce::Path tick;
            tick.startNewSubPath (c.x - 13, c.y + 1);
            tick.lineTo (c.x - 4, c.y + 10);
            tick.lineTo (c.x + 14, c.y - 9);
            g.setColour (text);
            g.strokePath (tick, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        else if (stage == Stage::Failed)
        {
            g.setColour (coral);
            g.drawEllipse (juce::Rectangle<float> (2 * r, 2 * r).withCentre (c), 5.0f);
            g.drawLine (c.x - 11, c.y - 11, c.x + 11, c.y + 11, 4.0f);
            g.drawLine (c.x + 11, c.y - 11, c.x - 11, c.y + 11, 4.0f);
        }
        else if (stage == Stage::Download && downloaded >= 0.0f)
        {
            arc (0.0f, juce::MathConstants<float>::twoPi * juce::jmax (0.02f, downloaded.load()));
            g.setColour (text);
            g.setFont (font (15.0f, true));
            g.drawText (juce::String (juce::roundToInt (downloaded * 100.0f)) + "%", juce::Rectangle<float> (2 * r, 2 * r).withCentre (c), juce::Justification::centred);
        }
        else if (stage == Stage::NeedClose)
        {
            arc (0.0f, juce::MathConstants<float>::twoPi);
            g.setColour (text);
            g.setFont (font (22.0f, true));
            g.drawText ("v" + job.version, juce::Rectangle<float> (2 * r + 30, 2 * r).withCentre (c), juce::Justification::centred);
        }
        else
        {
            const float a = spin * juce::MathConstants<float>::twoPi;
            arc (a, a + 1.9f + 0.6f * std::sin (spin * 5.0f));
        }

        g.setColour (text);
        g.setFont (font (17.0f, true));
        g.drawText (title(), 20, 170, W - 40, 26, juce::Justification::centred);
        g.setColour (textDim);
        g.setFont (font (13.0f));
        g.drawFittedText (detail(), 30, 196, W - 60, 40, juce::Justification::centredTop, 2);

        // Close cross (top right).
        const auto x = closeX.getBounds().toFloat().reduced (11.0f);
        g.setColour (closeX.isOver() ? text : textFaint);
        g.drawLine (x.getX(), x.getY(), x.getRight(), x.getBottom(), 1.6f);
        g.drawLine (x.getRight(), x.getY(), x.getX(), x.getBottom(), 1.6f);
    }

    void resized() override { closeX.setBounds (W - 44, 8, 34, 34); layoutButtons(); }
    void mouseDown (const juce::MouseEvent& e) override { dragger.startDraggingComponent (getTopLevelComponent(), e.getEventRelativeTo (getTopLevelComponent())); }
    void mouseDrag (const juce::MouseEvent& e) override { dragger.dragComponent (getTopLevelComponent(), e.getEventRelativeTo (getTopLevelComponent()), nullptr); }

private:
    juce::String title() const
    {
        switch (stage)
        {
            case Stage::Download: return "Downloading MetaTune " + job.version;
            case Stage::Verify: return "Checking the download";
            case Stage::NeedClose: return "MetaTune " + job.version + " is ready";
            case Stage::Closing: return "Closing " + job.hostName();
            case Stage::Install: return "Installing";
            case Stage::Done: return "MetaTune is up to date";
            case Stage::Failed: return "Update didn't finish";
        }
        return {};
    }
    juce::String detail() const
    {
        switch (stage)
        {
            case Stage::Download: return "You can keep working, " + job.hostName() + " stays open for now.";
            case Stage::Verify: return "Making sure the file is the real MetaTune installer.";
            case Stage::NeedClose: return job.hostName() + " has to close to replace the plugin. It will ask you to save first.";
            case Stage::Closing: return "Save your project if " + job.hostName() + " asks. The update continues as soon as it's closed.";
            case Stage::Install: return "Windows may ask for permission. This only takes a few seconds.";
            case Stage::Done: return "Version " + job.version + " is installed." + (hostExists() ? " Reopen " + job.hostName() + " to use it." : juce::String());
            case Stage::Failed: return error;
        }
        return {};
    }
    bool hostExists() const { return job.hostExe.isNotEmpty() && juce::File (job.hostExe).existsAsFile(); }

    void layoutButtons()
    {
        primary.setVisible (false);
        secondary.setVisible (false);
        auto show = [this] (const juce::String& p, std::function<void()> pf, const juce::String& s, std::function<void()> sf) {
            const int y = H - 62, w = s.isEmpty() ? 220 : 196;
            primary.setButtonText (p);
            primary.onClick = std::move (pf);
            primary.setVisible (p.isNotEmpty());
            secondary.setButtonText (s);
            secondary.onClick = std::move (sf);
            secondary.setVisible (s.isNotEmpty());
            if (s.isEmpty()) primary.setBounds ((W - w) / 2, y, w, 38);
            else if (p.isEmpty()) secondary.setBounds ((W - w) / 2, y, w, 38);
            else { secondary.setBounds (W / 2 - w - 6, y, w, 38); primary.setBounds (W / 2 + 6, y, w, 38); }
        };
        switch (stage)
        {
            case Stage::NeedClose: show ("Close " + job.hostName() + " & update", [this] { askHostToClose(); set (Stage::Closing); }, "Later", [this] { quit(); }); break;
            case Stage::Closing: show ({}, {}, "Cancel", [this] { quit(); }); break;
            case Stage::Done:
                if (hostExists()) show ("Reopen " + job.hostName(), [this] { host::launch (job.hostExe.toWideCharPointer()); quit(); }, "Close", [this] { quit(); });
                else show ("Close", [this] { quit(); }, {}, {});
                break;
            case Stage::Failed: show ("Try again", [this] { begin(); }, "Close", [this] { quit(); }); break;
            default: break;
        }
    }

    void set (Stage s)
    {
        juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<UpdaterView> (this), s] {
            if (safe == nullptr) return;
            safe->stage = s;
            safe->layoutButtons();
            safe->repaint();
            if (s == Stage::Closing) safe->closingSince = juce::Time::getMillisecondCounter();
            if (safe->job.autoResult != juce::File())
            {
                if (s == Stage::NeedClose) safe->primary.triggerClick();
                if (s == Stage::Done || s == Stage::Failed)
                {
                    safe->job.autoResult.replaceWithText (s == Stage::Done ? juce::String ("done") : "failed: " + safe->error);
                    safe->quit();
                }
            }
        });
    }
    void fail (const juce::String& why) { error = why; set (Stage::Failed); }

    // Ask the host to close, remembering which of its windows already exist so the save prompt it opens can be spotted.
    void askHostToClose()
    {
        hostWindows = host::windowsOf (job.pid);
        host::requestClose (job.pid);
    }
    // The host's save prompt goes in front of this window (once per prompt, so the user can still click back to us).
    void bringPromptForward()
    {
        for (auto* w : host::windowsOf (job.pid))
            if (std::find (hostWindows.begin(), hostWindows.end(), w) == hostWindows.end())
            {
                hostWindows.push_back (w);
                host::raiseAbove (w, getWindowHandle());
                prompt = w;
                settle = 45; // the host raises its own main window as the prompt activates: re-tuck behind for ~0.75 s
            }
    }

    void timerCallback() override
    {
        spin = std::fmod (spin + 1.0f / 75.0f, 1000.0f);
        // Closing: as soon as the host is gone, install. Re-ask every 20 s in case the save dialog was dismissed.
        if (stage == Stage::Closing)
        {
            if (! host::isRunning (job.pid)) { stage = Stage::Install; layoutButtons(); startThread(); }
            else if (juce::Time::getMillisecondCounter() - closingSince > 20000) { closingSince = juce::Time::getMillisecondCounter(); askHostToClose(); }
            else if (++ticks % 6 == 0) bringPromptForward();
            else if (settle > 0 && --settle >= 0 && ! host::keepBehind (prompt, getWindowHandle())) settle = 0;
        }
        repaint();
    }

    // Background work: download + verify on the first run, install on the second (after the host has closed).
    void run() override
    {
        if (stage == Stage::Install) { install(); return; }
        downloaded = 0.0f;
        installer = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("MetaTuneUpdate").getChildFile ("MetaTune-Setup-" + job.version + ".exe");
        installer.getParentDirectory().createDirectory();
        installer.deleteFile();
        if (! fetch()) { if (! threadShouldExit()) fail ("The download failed. Check your internet connection and try again."); return; }
        set (Stage::Verify);
        juce::FileInputStream in (installer);
        const auto got = juce::SHA256 (in).toHexString().toLowerCase();
        if (got != job.sha256.toLowerCase())
        {
            installer.deleteFile();
            fail ("The downloaded file doesn't match the published checksum, so it wasn't installed.");
            return;
        }
        if (job.pid != 0 && host::isRunning (job.pid)) set (Stage::NeedClose);
        else { set (Stage::Install); install(); }
    }

    bool fetch()
    {
        std::unique_ptr<juce::InputStream> in;
        if (job.url.startsWith ("file:")) in = juce::URL (job.url).getLocalFile().createInputStream();
        else if (job.url.startsWith ("https://"))
            in = juce::URL (job.url).createInputStream (juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                                                            .withConnectionTimeoutMs (15000)
                                                            .withExtraHeaders ("User-Agent: MetaTune-Updater"));
        if (in == nullptr) return false;
        juce::FileOutputStream out (installer);
        if (! out.openedOk()) return false;
        const auto total = in->getTotalLength();
        juce::HeapBlock<char> buf (1 << 16);
        juce::int64 done = 0;
        while (! in->isExhausted() && ! threadShouldExit())
        {
            const int n = in->read (buf, 1 << 16);
            if (n <= 0) break;
            out.write (buf, size_t (n));
            done += n;
            if (total > 0) downloaded = float (double (done) / double (total));
        }
        out.flush();
        downloaded = 1.0f;
        return done > 0 && (total <= 0 || done == total) && ! threadShouldExit();
    }

    void install()
    {
        const std::wstring args = L"/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP-";
        const std::wstring exe = installer.getFullPathName().toWideCharPointer();
        int code = -1;
        if (job.elevate) code = host::runElevatedAndWait (exe, args);
        else
        {
            juce::ChildProcess cp;
            if (cp.start (juce::StringArray { installer.getFullPathName(), "/VERYSILENT" }) && cp.waitForProcessToFinish (120000))
                code = int (cp.getExitCode());
        }
        if (code == 0) { installer.deleteFile(); set (Stage::Done); }
        else if (code == -2) fail ("Windows permission was declined, so nothing was changed. Try again and click Yes.");
        else fail ("The installer stopped (code " + juce::String (code) + "). Nothing was changed, your old version still works.");
    }

    void quit() { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }

    Job job;
    std::atomic<Stage> stage { Stage::Download };
    std::atomic<float> downloaded { -1.0f };
    juce::File installer;
    juce::String error;
    float spin = 0;
    juce::uint32 closingSince = 0;
    std::vector<void*> hostWindows; // host windows open before we asked it to close
    int ticks = 0, settle = 0;
    void* prompt = nullptr; // the host's save prompt, once spotted
    Btn primary { {}, true }, secondary { {}, false };
    struct HitArea : public juce::Button { HitArea() : juce::Button ("close") {} void paintButton (juce::Graphics&, bool, bool) override {} };
    HitArea closeX; // the cross itself is painted by the view
    juce::ComponentDragger dragger;
};

class UpdaterApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "MetaTune Updater"; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise (const juce::String& cmd) override
    {
        const auto a = juce::StringArray::fromTokens (cmd, true);
        auto arg = [&] (const char* k) { const int i = a.indexOf (k); return i >= 0 ? a[i + 1].unquoted() : juce::String(); };
        Job job { arg ("--version"), arg ("--url"), arg ("--sha256"), arg ("--host"), (unsigned long) arg ("--pid").getLargeIntValue(), ! a.contains ("--no-elevate") };
        if (a.contains ("--auto-result")) job.autoResult = juce::File (arg ("--auto-result"));

        if (a.contains ("--snapshot")) // dev: render a state to PNG and exit
        {
            if (job.version.isEmpty()) job = { "0.2.0", {}, {}, "C:\\Program Files\\Image-Line\\FL Studio 2025\\FL64.exe", 1 };
            UpdaterView v (job);
            const auto st = arg ("--snapshot").isEmpty() ? juce::String() : a[a.indexOf ("--snapshot") + 2];
            const Stage s = st == "close" ? Stage::NeedClose : st == "closing" ? Stage::Closing : st == "install" ? Stage::Install
                          : st == "done" ? Stage::Done : st == "failed" ? Stage::Failed : Stage::Download;
            v.showStage (s, s == Stage::Download ? 0.62f : -1.0f);
            const auto img = v.createComponentSnapshot (v.getLocalBounds(), true, 2.0f);
            juce::FileOutputStream out (juce::File (arg ("--snapshot")));
            out.setPosition (0); out.truncate();
            juce::PNGImageFormat().writeImageToStream (img, out);
            quit();
            return;
        }
        if (job.version.isEmpty() || job.url.isEmpty() || job.sha256.length() != 64) { quit(); return; }

        view = std::make_unique<UpdaterView> (job);
        view->setOpaque (false);
        view->addToDesktop (juce::ComponentPeer::windowAppearsOnTaskbar | juce::ComponentPeer::windowHasDropShadow | juce::ComponentPeer::windowIsSemiTransparent);
        view->setName ("MetaTune Updater");
        view->centreWithSize (UpdaterView::W, UpdaterView::H);
        view->setVisible (true);
        view->toFront (true);
        view->begin();
    }
    void shutdown() override { view.reset(); }
    void systemRequestedQuit() override { quit(); }

private:
    std::unique_ptr<UpdaterView> view;
};
} // namespace

START_JUCE_APPLICATION (UpdaterApp)
