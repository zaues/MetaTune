#pragma once
#include "Theme.h"
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>

namespace icons
{
juce::Path undo();
juce::Path redo();
juce::Path reset();
juce::Path power();
juce::Path eye();
juce::Path notes();
juce::Path help();
juce::Path menu();
juce::Path panel();     // window with a header bar (main menu)
juce::Path copyArrow(); // ">" chevron
juce::Path swap();      // two opposed arrows
juce::Path loop();      // two-arrow cycle (reset)
juce::Path save();      // floppy disk
juce::Path chat();      // speech bubble
juce::Path grid();      // 2x2 squares (groups)
juce::Path chevronUp();
juce::Path chevronDown();
juce::Path palette();   // three overlapping circles (colour / theme)
juce::Path key();       // a key (Auto Key)
}

// Flat icon button; toggleable buttons light up in the accent colour when on.
// Optional second icon and a (two-line) caption to the right.
class IconButton : public juce::Button
{
public:
    IconButton (const juce::String& name, juce::Path icon, juce::String caption = {}, juce::Path icon2 = {});
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void setIcon (juce::Path replacement) { icon = std::move (replacement); repaint(); }
    std::function<void (const juce::ModifierKeys&)> onModifiedClick;
    void clicked (const juce::ModifierKeys& modifiers) override { if (onModifiedClick) onModifiedClick (modifiers); }
    juce::Colour onColour = theme::text, idleColour;
    juce::Point<float> iconOffset;
    float iconScale = 0.62f, captionInset = -1.0f;
    bool hoverFill = true;
    bool useFilledIcons = false;
    void buttonStateChanged() override { hover.setTarget (isOver() || isDown() ? 1.0f : 0.0f); }

private:
    theme::Anim hover { *this };
    juce::Path icon, icon2;
    juce::String caption;
};

// Big text toggle (A/B slots): white when selected, grey otherwise.
class PillButton : public juce::Button
{
public:
    explicit PillButton (const juce::String& text) : juce::Button (text) {}
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void buttonStateChanged() override { hover.setTarget (isOver() ? 1.0f : 0.0f); }

private:
    theme::Anim hover { *this };
};

// Text button in the mode row. Primary = violet pill when on; Plain = lit segment inside a group.
class TextPill : public juce::Button
{
public:
    enum class Kind { Primary, Plain, SplitLeft, SplitRight };
    TextPill (const juce::String& text, Kind k, juce::Colour selected = {})
        : juce::Button (text), kind (k), selectedAccent (selected) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void buttonStateChanged() override { hover.setTarget (isOver() ? 1.0f : 0.0f); }

private:
    Kind kind;
    juce::Colour selectedAccent;
    theme::Anim hover { *this };
};

// "Update available" pill in the header: signature gradient, a soft pulse until hovered once.
class UpdatePill : public juce::Button, private juce::Timer
{
public:
    UpdatePill() : juce::Button ("update") { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void setVersion (const juce::String& v) { setButtonText ("Update  v" + v); startTimerHz (30); repaint(); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void buttonStateChanged() override { hover.setTarget (isOver() ? 1.0f : 0.0f); if (isOver()) seen = true; }

private:
    void timerCallback() override { phase += 1.0f / 30.0f; if (seen && glow() < 0.01f) stopTimer(); repaint(); }
    float glow() const { return seen ? 0.0f : 0.5f + 0.5f * std::sin (phase * 2.6f); }
    theme::Anim hover { *this };
    float phase = 0;
    bool seen = false;
};

// Small pill switch (Low Latency).
class ToggleSwitch : public juce::Button
{
public:
    void setWarmAccent (bool warm) { if (warmAccent != warm) { warmAccent = warm; repaint(); } }
    ToggleSwitch() : juce::Button ("switch") { setClickingTogglesState (true); setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void buttonStateChanged() override { hover.setTarget (isOver() ? 1.0f : 0.0f); }

private:
    theme::Anim hover { *this }, pos { *this, 0.25f };
    bool first = true, warmAccent = false;
};

// Row of amber segments bound to a choice parameter (Note Stabilizer). Each segment maps to one choice index.
class SegmentedControl : public juce::Component, public juce::SettableTooltipClient
{
public:
    SegmentedControl (juce::RangedAudioParameter&, juce::StringArray labels, juce::UndoManager*);
    void setWarmAccent (bool warm) { if (warmAccent != warm) { warmAccent = warm; repaint(); } }
    bool usesWarmAccent() const noexcept { return warmAccent; }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override
    {
        if (juce::isPositiveAndBelow (hover, int (hoverAmt.size()))) hoverAmt[size_t (hover)]->setTarget (0.0f);
        hover = -1;
    }

private:
    int indexAt (float x) const { return juce::jlimit (0, labels.size() - 1, int (x / (float (getWidth()) / float (labels.size())))); }
    juce::StringArray labels;
    juce::ParameterAttachment attach;
    int selected = 0, hover = -1;
    bool warmAccent = false;
    theme::Anim slide { *this, 0.25f };              // highlight position, in segments
    std::array<std::unique_ptr<theme::Anim>, 8> hoverAmt;
};

// Vertical tab strip on the left edge: a grid icon over four numbered groups.
class GroupTabs : public juce::Component, public juce::SettableTooltipClient
{
public:
    std::function<void (int)> onSelect;
    std::function<void()> onBrowse; // the grid icon opens the Group Browser
    void setSelected (int g) { if (g != selected) { selected = g; repaint(); } }
    void setBypassBlend (float amount) { bypassBlend = amount; repaint(); }
    void setCount (int g, int n) { if (counts[g] != n) { counts[g] = n; repaint(); } }
    void setBrowseActive (bool a) { if (a != browseActive) { browseActive = a; repaint(); } } // grey the grid/box icon while its browser is open
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override { if (hover >= 0) hoverAmt[hover].setTarget (0.0f); hover = -1; }

private:
    static constexpr int kTop = 60, arrowRailHeight = 35;
    float rowHeight() const;
    int indexAt (juce::Point<float>) const;
    theme::Anim hoverAmt[4] { theme::Anim { *this }, theme::Anim { *this }, theme::Anim { *this }, theme::Anim { *this } };
    theme::Anim selAmt[4] { theme::Anim { *this }, theme::Anim { *this }, theme::Anim { *this }, theme::Anim { *this } };
    int selected = -1, hover = -1;
    int counts[4] {};
    bool browseActive = false;
    float bypassBlend = 0.0f;
};

// Header preset box: "No preset selected", a preset icon and up/down arrows.
class PresetBar : public juce::Component, public juce::SettableTooltipClient
{
public:
    std::function<void()> onClick, onPrev, onNext;
    juce::String name;
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
};

// Thin horizontal slider with a dark red track and a white handle (Tone Volume).
class ToneSlider : public juce::Slider
{
public:
    ToneSlider();
    void paint (juce::Graphics&) override;
};

// Pitch Reference: dark track with a white marker, "0cts 440.0Hz" below. Drag, Shift = fine, double-click = default.
class ValueStrip : public juce::Slider
{
public:
    ValueStrip();
    void paint (juce::Graphics&) override;
};

// Key picker: replaces the bottom-left column while the root letter is hovered.
// Flats/Sharps naming, a 3x4 grid of roots (hover = boxed), and a list of scales.
class RootPicker : public juce::Component
{
public:
    static const juce::StringArray& scaleNames(); // order = index passed to onScale

    std::function<void (int pitchClass)> onRoot;
    std::function<void (int scaleIndex)> onScale;
    std::function<void (bool flats)> onFlats;
    std::function<void()> onClose;
    std::function<void()> onMouseLeave;

    void setState (int root, int scaleIndex, bool flats);
    int rootAt (juce::Point<float>) const;
    int scaleAt (juce::Point<float>) const;
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); if (onMouseLeave) onMouseLeave(); }

private:
    juce::Rectangle<float> tabRect (int i) const;
    juce::Rectangle<float> noteRect (int pc) const;
    juce::Rectangle<float> scaleRect (int i) const;
    int root = 0, scale = 0;
    bool flats = false;
    std::vector<std::unique_ptr<theme::Anim>> cellHover; // tabs, notes, scales
    theme::Anim& cellAnim (int i);
};

// Presets / Groups tabs for the drawer anchored to the left group rail.
class BrowserTabs : public juce::Component, public juce::SettableTooltipClient
{
public:
    std::function<void (int)> onSelect;
    void setTab (int t) { if (t != tab) { tab = t; repaint(); } }
    int getTab() const noexcept { return tab; }
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }

private:
    juce::Rectangle<float> tabRect (int i) const;
    int tab = 0;
};

struct PresetItem { juce::String name, folder; bool factory = false; bool favorite = false; };

class PresetPanel : public juce::Component
{
public:
    PresetPanel();
    std::function<void (int index)> onLoad, onDelete, onToggleFavorite;
    std::function<void()> onNewPreset, onNewFolder, onOpenFolder, onToggleKeepScale, onToggleTiming;
    std::function<void (const juce::String& folder)> onSaveToFolder, onDeleteFolder;
    std::function<void (int index, const juce::String& folder)> onMove;

    void setItems (const std::vector<PresetItem>& items, const juce::StringArray& folders, const juce::String& current);
    void setModeOptions (bool keepScale, bool lowLatency, bool stabilizer);
    void resized() override;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    enum class Kind { KeepScale, Timing, NewPreset, NewFolder, Divider, Folder, Preset, Section };
    struct Row { Kind kind; juce::String text, folder; int index = -1; bool factory = false, favorite = false; };
    static constexpr float kRowH = 24.0f;
    static float rowHeight (Kind kind);
    float topInset() const { return searchActive ? 30.0f : 0.0f; }
    void rebuild();
    float contentHeight() const;
    int rowAt (juce::Point<float>) const;
    juce::Rectangle<float> rowBounds (int i) const;
    juce::Rectangle<float> trashBounds (juce::Rectangle<float> row) const;
    juce::Rectangle<float> plusBounds (juce::Rectangle<float> row) const;
    void showActions (const juce::MouseEvent&);

    std::vector<PresetItem> allItems;
    std::vector<Row> rows;
    juce::StringArray userFolders, openFolders;
    juce::String current, filter;
    bool searchActive = false, favOnly = false;
    bool keepScale = true, lowLatency = false, stabilizer = false;
    juce::TextEditor searchBox;
    float scroll = 0;
    int dragIndex = -1, dropRow = -1;
    bool dragging = false;
    juce::Point<float> downPos;
};

struct GroupInstanceRow { int id = 0, group = -1; juce::String label; bool self = false; };

class GroupBrowser : public juce::Component
{
public:
    GroupBrowser();
    std::function<void (int id, int group)> onAssign;
    std::function<void (int group, const juce::String& name)> onRename;

    void setData (const juce::StringArray& groupNames, const std::vector<GroupInstanceRow>& instances);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void visibilityChanged() override;

private:
    struct Row { int group; int id; juce::String text; bool header, self; };
    static constexpr float kRowH = 24.0f;
    void rebuild();
    juce::Rectangle<float> rowBounds (int i) const;
    int rowAt (juce::Point<float>) const;
    void beginRename (int row);
    void endRename (bool commit);

    std::vector<Row> rows;
    std::vector<GroupInstanceRow> instances;
    juce::StringArray names;
    std::array<bool, 5> openGroups { true, true, true, true, true };
    std::array<int, 5> groupSizes {};
    int selectedGroup = -2;
    juce::TextEditor nameEditor;
    int renameGroup = -1;
    float scroll = 0;
    int dragId = -1, dropRow = -1;
    bool dragging = false;
    juce::Point<float> downPos;
};

// "Save preset" card drawn inside the plugin: dims the window, centred dark card with a name field.
// Enter saves, Escape or a click outside the card cancels.
class SavePresetCard : public juce::Component
{
public:
    SavePresetCard();
    std::function<void (const juce::String& name)> onSave;
    std::function<void()> onCancel;

    void open (const juce::String& suggestedName, juce::String error = {});
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    juce::Rectangle<int> card() const { return getLocalBounds().withSizeKeepingCentre (380, 190); }
    void save() { if (onSave) onSave (name.getText()); }
    juce::TextEditor name;
    TextPill saveButton { "Save", TextPill::Kind::Primary }, cancelButton { "Cancel", TextPill::Kind::Plain };
    juce::String message;
};

// Colour panel: Hue / Saturation / Brightness sliders plus a grid of preset swatches that recolour
// the plugin's accent palette. Drops down from the header palette button.
class ColorPanel : public juce::Component
{
public:
    std::function<void (float hue, float sat, float bright)> onChange;          // accent sliders / swatch / preset
    std::function<void (bool light)> onLight;                                   // light / dark theme
    std::function<void (bool on, float hueA, float hueB)> onGradient;           // two-colour accent gradient
    std::function<void (float orb, float key, float meter)> onElements;         // per-element hue offsets
    std::function<void()> onMouseLeave;

    void setValues (float h, float s, float b) { hue = h; sat = s; bright = b; repaint(); }
    void setExtras (bool light, bool grad, float gA, float gB, float orb, float key, float meter)
    { lightOn = light; gradOn = grad; gradA = gA; gradB = gB; orbH = orb; keyH = key; meterH = meter; repaint(); }
    int preferredHeight() const { return 356; }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); if (drag < 0 && onMouseLeave) onMouseLeave(); }

    static constexpr int kSwatches = 16;

private:
    juce::Rectangle<float> stripRect (int id) const;  // 0 hue 1 sat 2 bright 3 gradA 4 gradB 5 orb 6 key 7 meter
    juce::Rectangle<float> swatchAt (int i) const;
    juce::Rectangle<float> chipRect (int i) const;    // 0 Light 1 Gradient 2 Default 3 FL 4 Ocean 5 Green
    static float swatchHue (int i) { return float (i) / float (kSwatches); }
    void applyDrag (juce::Point<float>);
    void applyPreset (float h, float s, float b);

    float hue = 0.0f, sat = 1.0f, bright = 1.0f;
    bool lightOn = false, gradOn = false;
    float gradA = 0.83f, gradB = 0.58f, orbH = 0.0f, keyH = 0.0f, meterH = 0.0f;
    int drag = -1;
};

// Full-window "What's New" / release-notes overlay: blurs the UI behind it and shows a scrollable
// list of version notes in a centred card with a close (X) button.
class ReleaseNotesCard : public juce::Component
{
public:
    ReleaseNotesCard() { setInterceptsMouseClicks (true, true); }
    std::function<void()> onClose;
    void open (juce::Image blurredBackdrop);
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    juce::Rectangle<int> cardBounds() const;
    juce::Rectangle<float> closeBounds() const;
    void buildLayouts (float width);
    float contentHeight() const;

    juce::Image backdrop;
    float scroll = 0, lastWidth = 0;
    std::vector<juce::TextLayout> bodyLayouts;
};

// Auto Key: a full-window modal card. If MetaKey isn't installed it offers to install it; otherwise it
// shows the key MetaKey is detecting with an Apply button (sends the key to MetaTune), or a hint to open
// MetaKey on the beat's mixer track. State is pushed in by the editor (which polls the shared file).
class AutoKeyCard : public juce::Component
{
public:
    AutoKeyCard() { setInterceptsMouseClicks (true, true); }
    std::function<void (int root, bool minor)> onApply;
    std::function<void()> onInstall, onClose;

    void open (bool metaKeyInstalled, bool haveKey, int root, bool minor);
    void setState (bool metaKeyInstalled, bool haveKey, int root, bool minor);
    void paint (juce::Graphics&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseUp (const juce::MouseEvent&) override;

private:
    juce::Rectangle<int> cardBounds() const { return juce::Rectangle<int> (380, 240).withCentre (getLocalBounds().getCentre()); }
    juce::Rectangle<float> actionButton() const; // Install / Apply
    juce::Rectangle<float> closeBounds() const;

    bool installed = false, haveKey = false, minor = false;
    int root = -1;
};

// Settings list that drops down from the header menu button, styled like the preset panel:
// section headers, choices with a hover box and a dot on the current one, and dim info lines.
class SettingsPanel : public juce::Component
{
public:
    struct Item
    {
        enum class Kind { Header, Choice, Info } kind;
        juce::String text, detail; // detail is drawn right-aligned (e.g. "3.6 ms")
        int id = 0;
        bool current = false;
    };
    std::function<void (int id)> onPick;
    std::function<void()> onMouseLeave;

    void setItems (std::vector<Item>);
    int preferredHeight() const;
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); if (onMouseLeave) onMouseLeave(); }

private:
    static constexpr float kRowH = 26.0f, kHeaderH = 24.0f;
    float rowHeight (const Item& it) const { return it.kind == Item::Kind::Header ? kHeaderH : kRowH; }
    int rowAt (juce::Point<float>) const;
    theme::Anim& rowAnim (int i);

    std::vector<Item> items;
    std::vector<std::unique_ptr<theme::Anim>> hover;
};
