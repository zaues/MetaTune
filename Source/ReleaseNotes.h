#pragma once
#include <juce_core/juce_core.h>

// Bundled changelog shown in the "What's New" popup (after an update) and in Settings > Release notes.
// Newest first. Keep entries short and user-facing; add a new one with every release.
struct ReleaseNote { const char* version; const char* body; };

inline const ReleaseNote kReleaseNotes[] = {
    { "0.5.4",
      "- The 'What's New' popup now fades out when you close it.\n"
      "- More space between the Doubler line and the 'Doubler Width' label.\n"
      "- The Doubler ball now stops just short of the top of each guide line instead of reaching the very end." },
    { "0.5.3",
      "- The 'Doubler Width' label now sits just past the end of its line, centred on it.\n"
      "- Tooltips follow the theme (light tooltips in light mode).\n"
      "- In the colour panel, 'Default' highlights when nothing is customised, and clicking it also returns you to the dark theme." },
    { "0.5.2",
      "- The Doubler guide is a single line with a rounded corner, hidden until you hover it (or the ball), then it lights up white.\n"
      "- The 'Doubler Width' label now shows in full, spaced below its line.\n"
      "- Light mode: the selected scale and Note Stabilizer buttons (Chromatic, Tone, etc.) and tooltips now follow the theme." },
    { "0.5.1",
      "- Light mode now covers the whole interface: the side columns, keyboard, note labels and bottom bars all go light.\n"
      "- The header preset box opens a presets-only list; the grid/box icon still opens the tabbed Presets + Groups browser.\n"
      "- The Doubler guide lines are rounded, stop short of the axis labels, and light up white when you hover the line itself (not just the handle).\n"
      "- More breathing room around the palette (colour) button icon." },
    { "0.5.0",
      "- Light mode: a full light theme. Toggle it from the colour panel (the palette button).\n"
      "- Gradient accent: pick two colours and MetaTune blends them across the orb, meters and keys.\n"
      "- Theme presets: one-click FL, Ocean, Green and Default looks in the colour panel.\n"
      "- Per-element colour: give the orb, keys and meters their own hue.\n"
      "- The Sustain knob now shows a slim, smooth bipolar arc that fills out from the centre.\n"
      "- The Doubler handle has a circular guide ring that lights up white on hover, with a white/black/white centre.\n"
      "- The preset/groups browser now slides in from the left." },
    { "0.4.2",
      "- The box icon slides its browser open and now greys out while the browser is showing, like a selected tab.\n"
      "- New preset-browser toolbar: New preset, New folder, Search (filter by name), Open folder and Favourites.\n"
      "- Star any preset to favourite it, then use the star in the toolbar to show only your favourites." },
    { "0.4.1",
      "- The box icon above the group tabs now opens a tabbed browser with both Presets and Groups, so you can reach your presets from there too.\n"
      "- More space between the Doubler Dry/Wet label and its guide line." },
    { "0.4.0",
      "- New: recolour MetaTune! The palette button (top-left) opens a colour panel with Hue, Saturation and Brightness sliders plus a grid of preset swatches, so you can switch the accent from purple to any colour. Your choice is saved." },
    { "0.3.14",
      "- New: a 'What's New' popup after updates, plus a Release notes entry in Settings to read it anytime." },
    { "0.3.13",
      "- The Doubler ball now passes BEHIND the Dry/Wet and Doubler labels, which stay fully legible with a brighter look and a stronger shadow.\n"
      "- The Sustain readout is centred, and the Speed value keeps its decimal (20.0)." },
    { "0.3.6",
      "- Soft drop shadows on all text across the whole interface for better depth and legibility." },
    { "0.3.1",
      "- Group Browser: open it from the grid icon above the group tabs to drag instances between named groups, plus an Ungrouped section." },
    { "0.3.0",
      "- Preset folders: create folders, save presets into them, drag presets between folders, and delete presets or folders (to the Recycle Bin)." },
    { "0.2.1",
      "- The tone generator can play major/minor chords (pick Major or Minor, then click a note).\n"
      "- Keep My Scale (Settings) controls whether loading a preset also loads its key/scale." },
    { "0.2.0",
      "- Sustain now goes negative (-100..100): negative makes held notes retune slower.\n"
      "- Hold Alt while dragging the Doubler dot to lock it to one axis.\n"
      "- Picking Major/Minor drops you into editable Note mode so you can fine-tune the scale." },
    { "0.1.9",
      "- Reworked the central orb: denser nebula, a brighter glowing ring and a richer outer halo.\n"
      "- The scale picker slides in smoothly when you hover the key cell." },
    { "0.1.0",
      "- First release: real-time pitch correction with Speed, Amount, Sustain and Note Transition, a live pitch orb, the Doubler, Groups and presets." },
};

inline constexpr int kNumReleaseNotes = int (sizeof (kReleaseNotes) / sizeof (kReleaseNotes[0]));
