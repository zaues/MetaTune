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
}

// Flat icon button; toggleable buttons light up in the accent colour when on.
// Optional second icon and a (two-line) caption to the right.
class IconButton : public juce::Button
{
public:
    IconButton (const juce::String& name, juce::Path icon, juce::String caption = {}, juce::Path icon2 = {});
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    juce::Colour onColour = theme::text;
    float iconScale = 0.62f;
    bool hoverFill = true;
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
    enum class Kind { Primary, Plain };
    TextPill (const juce::String& text, Kind k) : juce::Button (text), kind (k) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void buttonStateChanged() override { hover.setTarget (isOver() ? 1.0f : 0.0f); }

private:
    Kind kind;
    theme::Anim hover { *this }, lit { *this, 0.25f };
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
    ToggleSwitch() : juce::Button ("switch") { setClickingTogglesState (true); setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void buttonStateChanged() override { hover.setTarget (isOver() ? 1.0f : 0.0f); }

private:
    theme::Anim hover { *this }, pos { *this, 0.25f };
    bool first = true;
};

// Row of amber segments bound to a choice parameter (Note Stabilizer). Each segment maps to one choice index.
class SegmentedControl : public juce::Component, public juce::SettableTooltipClient
{
public:
    SegmentedControl (juce::RangedAudioParameter&, juce::StringArray labels, juce::UndoManager*);
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
    void setCount (int g, int n) { if (counts[g] != n) { counts[g] = n; repaint(); } }
    void setBrowseActive (bool a) { if (a != browseActive) { browseActive = a; repaint(); } } // grey the grid/box icon while its browser is open
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override { if (hover >= 0) hoverAmt[hover].setTarget (0.0f); hover = -1; }

private:
    static constexpr int kTop = 68; // grid icon cell height
    int indexAt (float y) const;
    theme::Anim hoverAmt[4] { theme::Anim { *this }, theme::Anim { *this }, theme::Anim { *this }, theme::Anim { *this } };
    theme::Anim selAmt[4] { theme::Anim { *this }, theme::Anim { *this }, theme::Anim { *this }, theme::Anim { *this } };
    int selected = -1, hover = -1;
    int counts[4] {};
    bool browseActive = false;
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

// Two-tab header ("Presets" / "Groups") for the browser opened from the group box icon.
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

// One preset as the panel sees it (folder "" = root / factory).
struct PresetItem { juce::String name, folder; bool factory = false; bool favorite = false; };

// Preset list that drops down from the header preset box, inside the plugin window. Factory presets,
// then user sub-folders (each with a + to save-into and a trash to delete), then root "My presets",
// then New preset / New folder / Open folder. User presets have a hover trash button and can be
// dragged onto a folder (or "My presets") to move them. Wheel scrolls when it overflows.
class PresetPanel : public juce::Component
{
public:
    PresetPanel();
    std::function<void (int index)> onLoad;                               // index into the flat preset list
    std::function<void()> onNewPreset, onNewFolder, onOpenFolder, onMouseLeave;
    std::function<void (const juce::String& folder)> onSaveToFolder;      // save current preset into a folder
    std::function<void (int index)> onDelete;                            // trash a user preset
    std::function<void (int index)> onToggleFavorite;                    // star / unstar a preset
    std::function<void()> onHeightChanged;                               // search / favourites changed the preferred height
    std::function<void (const juce::String& folder)> onDeleteFolder;     // trash a folder
    std::function<void (int index, const juce::String& folder)> onMove;  // move a preset to a folder ("" = root)

    void setItems (const std::vector<PresetItem>& items, const juce::String& current);
    int preferredHeight() const;
    void resized() override;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); if (! dragging && onMouseLeave) onMouseLeave(); }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    enum class Kind { Section, Folder, Preset, Toolbar };
    struct Row { Kind kind; juce::String text; juce::String folder; int index = -1; bool factory = false; bool favorite = false; };
    static constexpr float kRowH = 26.0f, kHeaderH = 24.0f, kToolbarH = 42.0f;
    float rowHeight (const Row& r) const
    {
        if (r.kind == Kind::Toolbar) return kToolbarH;
        return (r.kind == Kind::Section || r.kind == Kind::Folder) ? kHeaderH : kRowH;
    }
    float topInset() const { return searchActive ? 34.0f : 0.0f; }
    void rebuild(); // apply the search / favourites filter to allItems and lay out the rows
    float contentHeight() const;
    int rowAt (juce::Point<float>) const;
    juce::Rectangle<float> rowBounds (int i) const;
    juce::Rectangle<float> trashBounds (juce::Rectangle<float> row) const;
    juce::Rectangle<float> plusBounds (juce::Rectangle<float> row) const;
    juce::Rectangle<float> starBounds (juce::Rectangle<float> row) const;
    juce::Rectangle<float> toolBounds (juce::Rectangle<float> row, int i) const; // 0..4 footer icons
    int toolAt (juce::Rectangle<float> row, juce::Point<float> p) const;
    theme::Anim& rowAnim (int i);

    std::vector<PresetItem> allItems;
    std::vector<Row> rows;
    juce::String current, filter;
    bool searchActive = false, favOnly = false;
    juce::TextEditor searchBox;
    float scroll = 0;
    int dragIndex = -1, dropRow = -1; // dragIndex = preset being dragged; dropRow = folder/section row under cursor
    bool dragging = false;
    juce::Point<float> downPos;
    std::vector<std::unique_ptr<theme::Anim>> hover;
};

// One live instance as the group browser sees it.
struct GroupInstanceRow { int id = 0, group = -1; juce::String label; bool self = false; };

// Group Browser: lists the four groups (named, click to rename) with their member instances, plus an
// "Ungrouped" section. Drag an instance onto a group (or Ungrouped) to move it. Drops from the header
// preset bar's "Groups" affordance.
class GroupBrowser : public juce::Component
{
public:
    std::function<void (int id, int group)> onAssign; // group -1 = ungroup
    std::function<void (int group)> onRename;
    std::function<void()> onMouseLeave;

    void setData (const juce::StringArray& groupNames, const std::vector<GroupInstanceRow>& instances);
    int preferredHeight() const;
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); if (! dragging && onMouseLeave) onMouseLeave(); }

private:
    enum class Kind { GroupHeader, Instance, Empty };
    struct Row { Kind kind; int group; int id; juce::String text; bool self; };
    static constexpr float kRowH = 26.0f, kHeaderH = 26.0f;
    float rowHeight (const Row& r) const { return r.kind == Kind::GroupHeader ? kHeaderH : kRowH; }
    float contentHeight() const;
    juce::Rectangle<float> rowBounds (int i) const;
    int rowAt (juce::Point<float>) const;
    theme::Anim& rowAnim (int i);

    std::vector<Row> rows;
    int dragId = -1, dropRow = -1; bool dragging = false;
    juce::Point<float> downPos;
    std::vector<std::unique_ptr<theme::Anim>> hover;
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
    std::function<void (float hue, float sat, float bright)> onChange;
    std::function<void()> onMouseLeave;

    void setValues (float h, float s, float b) { hue = h; sat = s; bright = b; repaint(); }
    int preferredHeight() const { return 214; }
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); if (drag < 0 && onMouseLeave) onMouseLeave(); }

    static constexpr int kSwatches = 16;

private:
    juce::Rectangle<float> barFor (int i) const;      // 0 Hue, 1 Saturation, 2 Brightness
    juce::Rectangle<float> swatchAt (int i) const;
    static float swatchHue (int i) { return float (i) / float (kSwatches); }
    void applyDrag (juce::Point<float>);

    float hue = 0.0f, sat = 1.0f, bright = 1.0f;
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
