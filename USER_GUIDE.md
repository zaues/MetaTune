# MetaTune — User Guide

MetaTune is a real-time automatic pitch corrector. It tunes the pitch of an incoming
signal either live while you track or after you've recorded. It is built for
**monophonic** sources — a voice or an instrument playing one note at a time. (With the
**Groups** feature you can run several instances that share one set of settings — more on
that below.)

MetaTune is a **VST3** plugin for **Windows (x64)**. Download `MetaTune-Setup.exe` from the
[Releases](https://github.com/zaues/MetaTune/releases) page and run it; once installed, the
plugin tells you when a new version is out and can update itself.

> 📸 _Screenshot: MetaTune loaded on a track_

---

## Contents

- [Quick start](#quick-start)
- [Finding and setting your key](#finding-and-setting-your-key)
- [Tone generator](#tone-generator)
- [Chromatic toggle](#chromatic-toggle)
- [The main controls](#the-main-controls) — Speed · Amount · Sustain · Note Transition
- [A word about latency](#a-word-about-latency)
- [The keyboard](#the-keyboard)
- [Detected Notes & HeatMaps](#detected-notes--heatmaps)
- [Pitch Reference](#pitch-reference)
- [The Doubler](#the-doubler)
- [Groups](#groups)
- [Automation](#automation)
- [The preset browser](#the-preset-browser)
- [What the top bar does](#what-the-top-bar-does)
- [Keyboard & mouse shortcuts](#keyboard--mouse-shortcuts)

---

## Quick start

1. Load MetaTune as an **insert effect** on your audio track.
2. **Set the key** you want the audio tuned to (see below).
3. Play the track and adjust the **Speed** and **Amount** knobs until it sounds the way you want.

That's it — for most sources that's all you need. The rest of this guide covers the features
that let you dial in exactly the effect you're after.

> 📸 _Screenshot: Speed and Amount knobs_

---

## Finding and setting your key

The key is the target that the notes in your audio get corrected to. When you first open
MetaTune the key cell on the left shows a cursor and **"Hover to set scale"** — it stays
until you pick a key for the first time.

> 📸 _Screenshot: the "Hover to set scale" prompt_

There are two ways to set the key.

**1. The chord selector above the keyboard.**
Click **Major** or **Minor**, then click the piano key for your root note. (Click the actual
white/black **key**, not the grey strip above it that shows the note names.) After you pick a
key, the selector drops into **Note** mode, where clicking keys turns individual notes of
that scale on and off — handy when you only want to correct to certain notes.

> 📸 _Screenshot: the Major · Minor · Chromatic · Scale selector_

**2. The scale menu.**
**Hover the key cell** on the left of the keyboard to slide out the scale menu, then choose a
**root note** and a **scale**. Use this to set a key directly, or to transpose the current
scale to a new root.

> 📸 _Screenshot: the scale menu_

**Not sure of the key?** Play the track back and click the **note names** above the keys —
each plays a reference tone so you can find your root by ear. You can also watch **Detected
Notes** (the white lines above the keys) to see which notes MetaTune is hearing in your
source.

---

## Tone generator

A built-in tone generator helps you find your key. Click a **note name** above any key to
hear a plain tone at that pitch. If you've selected **Major** or **Minor** in the chord
selector, clicking a note name plays that **chord** (root + third + fifth) instead of a single
note. Set the tone's level with the **Tone Volume** control on the left.

---

## Chromatic toggle

The **Chromatic** button lets your audio correct to the nearest note *anywhere* on the
keyboard, ignoring the scale you set. Click it again to go back to your scale. It's a quick
way to hear which notes your source is actually closest to.

---

## The main controls

MetaTune has four controls that shape the sound: **Speed**, **Amount**, **Sustain**, and
**Note Transition**.

### Speed

How fast correction is applied, in milliseconds:

| Speed | Result |
|---|---|
| 200 ms → 20 ms | Natural correction that isn't meant to be noticed |
| 20 ms → 0 ms | The classic robotic hard-tuned vocal sound |
| 0 ms → −3 ms | An even harder, more immediate effect — pairs well with Note Transition for a locked sound without flutter |

### Amount

How strongly the pitch is corrected. **0%** leaves the pitch untouched; **100%** always pulls
fully to the target note. Around **80%** leaves a little slack for a more natural, transparent
result.

### Sustain

Changes the retune speed based on how long a note is held — the start of each note stays
natural while longer notes behave differently. The Sustain ring around the Speed knob moves
as you play to show how much is being applied.

| Sustain | Held notes retune |
|---|---|
| Positive (0 → +100) | **Faster** — tighter, more locked long notes |
| Negative (0 → −100) | **Slower** — preserves vibrato and prevents over-tuning long notes |

### Note Transition

If you've used auto-tuners before, you've met **flutter** — at fast speeds the tuner darts
onto every tiny wobble (vibrato, little dips between words, breath at the end of a phrase),
producing a glitchy warble. **Note Transition** fixes this: it ignores short, passing notes
and only retunes to notes that are actually held.

| Setting | Ignores notes shorter than |
|---|---|
| Low | 40 ms |
| Mid | 80 ms |
| High | 200 ms |

So if you turn Speed down for a tight modern sound and hear flutter, switch Note Transition
to **Low** and the fluttering on short notes disappears. Set it to **High** for a heavily
locked, creative effect popular in hip-hop and rap.

> **Note:** Unlike some tuners, MetaTune's Note Transition adds **no extra latency** — you can
> use it together with Low Latency.

---

## A word about latency

MetaTune uses an efficient low-latency algorithm, which makes it good for tracking and live
monitoring. The amount of latency depends on the **Range** (the lowest pitch it tracks),
chosen in **Settings → Range**:

| Range | Added latency @ 48 kHz |
|---|---|
| High (150 Hz) | ~3.6 ms |
| Mid (100 Hz) | ~5.3 ms |
| Low (70 Hz) | ~7.4 ms |

The **Low Latency** toggle in the mode row selects the lowest-latency range for tracking.
MetaTune reports its latency to your DAW so plugin delay compensation works correctly.

---

## The keyboard

The keyboard has two parts: the **note names / tone generator** along the top, and the
**piano keys** below.

- **Tone generator** — click a note name to hear a tone (or a chord, if Major/Minor is
  selected). Volume is set by the control on the left.
- **Piano keys** — click keys to set your key (as above), or, in **Note** mode, click keys to
  turn individual notes of the scale on and off. **Right-click** a key to make it the root.

---

## Detected Notes & HeatMaps

These two overlays let you *see* what MetaTune is doing.

**Detected Notes** draws white lines above the piano keys that match the notes in your audio —
useful for spotting the key. By default it shows the notes *after* correction; use the
**Chromatic toggle** if you want to see what the raw source is closest to.

**HeatMaps** (above the keyboard) show the precise pitch of the input and output — not just
which note, but exactly where within the note the pitch landed. The top strip is the input,
the bottom is the corrected output.

Toggle each overlay with the eye buttons on the right.

> 📸 _Screenshot: Detected Notes and HeatMaps_

---

## Pitch Reference

By default MetaTune uses standard concert pitch, **A = 440 Hz**. You can retune this to match
sources at a different reference (e.g. A = 432 Hz). Drag the **Pitch Reference** control at the
bottom right; values can go up to one semitone in either direction. Shift-drag for fine
control, double-click to return to 440 Hz.

---

## The Doubler

The Doubler copies your tuned voice into two extra voices for a thicker, wider sound. Drag the
**grey dot** on the orb:

- **Horizontally → Width.** At 100% the two extra voices sit hard left and right.
- **Vertically → Dry/Wet.** How much of the doubled voices you hear.

| Dry/Wet | You hear |
|---|---|
| 0% | Only your original voice (centre) |
| 50% | All three voices at equal level (left, centre, right) |
| 100% | Only the two new voices (left, right) |

Hold **Alt** while dragging to lock the dot to one axis.

> 📸 _Screenshot: the Doubler dot and its guides_

---

## Groups

Groups let several instances of MetaTune **share one set of settings** — set it once and every
instance in the group follows. Great for a stack of backing vocals or ad-libs. Change any
instance and the rest update instantly.

**Shortcuts.** Click one of the numbered tabs **1–4** down the left edge to join that group.
The small number on each tab shows how many instances are in it. Click your group again to
leave it.

> 📸 _Screenshot: the group tabs_

**Group Browser.** Click the **grid icon** above the tabs to open the browser. It lists every
open MetaTune grouped by group (with an **Ungrouped** section). **Drag** an instance onto a
group to move it, and **click a group's name** to rename it. The instance whose window you
have open is marked **(this)**.

> 📸 _Screenshot: the Group Browser_

How groups behave:

- **Ungrouped** (default) — the instance uses its own settings.
- **Joining an empty group** — it keeps its settings and shares them with anyone who joins later.
- **Joining a group that already has members** — it adopts the group's settings. (Use **Undo**
  if you joined by mistake and want your old settings back.)
- **Leaving a group** — it keeps the settings it had but becomes independent again.

MetaTune supports up to **4 groups**.

---

## Automation

You can automate MetaTune's controls from your DAW like any plugin. Every member of a group
follows automation written on any track in that group. To avoid conflicts, write a group's
automation on a **single** track rather than automating the same control on two tracks at once.

---

## The preset browser

Open it from the **preset box** in the top bar. It's hidden by default.

**Load** — click any preset. Factory presets are built in; your own appear under **My
presets** and inside any folders you make.

**Save** — the **New preset…** row (or the **floppy** button in the top bar) saves the current
settings. Only your own presets can be overwritten; factory presets can't.

**Folders** — use **New folder…** to make a folder. Each folder header has a **+** to save the
current preset straight into it, and a **trash** to delete the folder. **Drag** a preset onto a
folder (or onto **My presets**) to move it. Hover a preset and click its **trash** to delete
it. Deletions go to the Recycle Bin, so nothing is lost by accident.

> 📸 _Screenshot: the preset browser with folders_

**Keep My Scale** — in **Settings**. When **on** (the default), loading a preset keeps your
current key and scale. Turn it **off** if you want presets to load their own scale too.

---

## What the top bar does

| Button | What it does |
|---|---|
| **≡** (menu) | Opens **Settings**: window size, Range, Keep My Scale, and updates |
| **Preset box** | Shows the current preset; click to open the browser, arrows step through presets |
| **💾** (floppy) | Save the current settings as a preset |
| **↶ / ↷** | Undo / Redo (last 20 actions) |
| **⟲** | Reset all controls to defaults |
| **A  ›  B** | Two snapshot slots for comparing settings; **›** copies the current slot to the other |
| **💬** | Show / hide tooltips |

---

## Keyboard & mouse shortcuts

| Action | Windows |
|---|---|
| Change a value | Mouse wheel |
| Fine adjust | Ctrl + drag, or right-click + drag |
| Reset to default | Alt + click, or double-click |
| Type a value | Double-click, then type |
| Undo / Redo | Ctrl + Z / Ctrl + Y |
| Lock the Doubler dot to one axis | Alt + drag |

---

*MetaTune is free and open source (AGPL-3.0). It is an independent project and is not affiliated
with, endorsed by, or connected to any commercial plugin vendor.*
