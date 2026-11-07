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
    void setSelected (int g) { if (g != selected) { selected = g; repaint(); } }
    void setCount (int g, int n) { if (counts[g] != n) { counts[g] = n; repaint(); } }
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

// Preset list that drops down from the header preset box, inside the plugin window.
// Factory presets, the user's presets, then Save / Open folder. Wheel scrolls when it overflows.
class PresetPanel : public juce::Component
{
public:
    std::function<void (int index)> onLoad;
    std::function<void()> onSave, onOpenFolder, onMouseLeave;

    void setItems (const juce::StringArray& names, int numFactory, const juce::String& current);
    int preferredHeight() const;
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); if (onMouseLeave) onMouseLeave(); }
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    enum class Kind { Header, Preset, Save, Folder };
    struct Row { Kind kind; juce::String text; int index; };
    static constexpr float kRowH = 26.0f, kHeaderH = 24.0f;
    float rowHeight (const Row& r) const { return r.kind == Kind::Header ? kHeaderH : kRowH; }
    float contentHeight() const;
    int rowAt (juce::Point<float>) const;
    theme::Anim& rowAnim (int i);

    std::vector<Row> rows;
    juce::String current;
    float scroll = 0;
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
