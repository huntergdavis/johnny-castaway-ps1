---
title: Play
eyebrow: Download · Run
subtitle: Latest build, quickstart, controls.
description: Download the latest Johnny Castaway PS1 fan port — .bin / .cue pair, DuckStation quickstart, controller map, freeplay controls, and the smoke-test path on real PS1 hardware via TonyHax.
---

## Latest build

**{{ site.release.tag }}** — short notes on this and earlier
versions are at [/releases/]({{ '/releases/' | relative_url }});
the GitHub release pages are at
[{{ site.github_url }}/releases/tag/{{ site.release.tag }}]({{ site.github_url }}/releases/tag/{{ site.release.tag }}).

<p class="hero-cta" markdown="0">
  <a class="btn btn--primary" href="{{ site.github_url }}/releases/download/{{ site.release.tag }}/jcreborn.bin">Download <code>.bin</code> <small>(~72&nbsp;MiB)</small></a>
  <a class="btn"              href="{{ site.github_url }}/releases/download/{{ site.release.tag }}/jcreborn.cue">Download <code>.cue</code> <small>(71&nbsp;B)</small></a>
  <a class="btn btn--small"   href="{{ '/releases/' | relative_url }}">Release notes</a>
</p>

The CD image ships as a **.bin / .cue** pair. Both files belong in
the same directory. Sizes shown are for the
[`{{ site.release.tag }}`]({{ site.github_url }}/releases/tag/{{ site.release.tag }})
upload — the `.bin` is ~76&nbsp;MB (~72&nbsp;MiB), mostly FG2 pack
payload routed onto the disc.

## Quickstart (DuckStation) {#emulator}

1. Install [DuckStation](https://www.duckstation.org/) on your
   platform of choice.
2. Drop `jcreborn.cue` and `jcreborn.bin` into a folder.
3. In DuckStation: *File → Start File…* → pick `jcreborn.cue`.
4. Hit start. The game runs as a screensaver — let it idle and the
   scenes will cycle on their own.

There is no menu to "begin." The game *is* the screensaver.

## Original Sierra data files

This port ships only the code that drives playback. The original
Sierra data files are **not** included for license reasons.

For development builds (host capture mode), you'll need:

| File          | Original-Sierra source         |
|---------------|--------------------------------|
| `RESOURCE.MAP`| Johnny Castaway 1.0 install    |
| `RESOURCE.001`| Johnny Castaway 1.0 install    |

For the PS1 build, **everything you need is on the disc image** —
the host pipeline pre-bakes the playback packs. End users do not
need any Sierra files to play the .bin/.cue.

## Controls {#controls}

The game runs without any input by default. The pause menu is
optional and reachable with **Start**.

| Button            | Action                                           |
|-------------------|--------------------------------------------------|
| Start             | Open pause menu / resume                         |
| D-pad / left analog | Move cursor or adjust values                   |
| Cross             | Confirm / select                                 |
| Circle            | Back from any menu or submenu                    |

Inside the pause menu you can: enter or exit Freeplay, mute sound,
toggle closed captions, force day/night, tide, raft, and holidays,
advance to the next scene, set the in-game date, move the island
anchor, set the RNG seed, and open the sound test.

## Freeplay controls

Freeplay launches from the pause menu. It is the direct-control Johnny
mode added in [`v0.5.0-ps1`]({{ '/releases/#v050-ps1--freeplay-and-debug-mode' | relative_url }}).

| Button | Action |
|---|---|
| D-pad / left analog | Walk Johnny. Movement cancels the current action immediately. |
| L2 held | Slow walk. |
| R2 held | Fast walk. |
| Circle | Fish from the nearest side of the island. |
| Select | Clear the screen, cancel transient actions, and rebuild the island. |
| R1 + Up | Toggle day/night. |
| R1 + Down | Toggle high/low tide. |
| R1 + Left | Cycle raft stage. |
| R1 + Right | Cycle holiday overlay. |
| Start | Open pause menu. |

The full implementation notes live at
[Menu help guide]({{ '/help/menu/' | relative_url }}),
[Freeplay and debug mode]({{ '/docs/freeplay/' | relative_url }}), and
[Pause menu]({{ '/docs/pause-menu/' | relative_url }}).

## Real PS1 hardware

It runs on real hardware — the build has been smoke-tested on a
SCPH-7501 via the [TonyHax](https://github.com/socram8888/tonyhax)
softmod path. No officially-licensed disc, no modchip required.
Your mileage may vary; treat any boot success as a small miracle.

## Where to file a bug

The issue tracker is at [{{ site.repo }}/issues]({{ site.github_url }}/issues).
Bugs are tolerated, not invited — there's no contributor onboarding,
just one author and a tip jar that doesn't exist. See the [FAQ]({{ '/faq/' | relative_url }}).

## Related pages

- [FAQ]({{ '/faq/' | relative_url }}) — author-written answers to
  the common "what is this?" / "do I need Sierra files?" / "does
  it work on real hardware?" questions.
- [Releases]({{ '/releases/' | relative_url }}) — short notes on
  every tagged version, with links to the full release notes.
- [Help: menu screenshots]({{ '/help/menu/' | relative_url }}) —
  every pause-menu sub-screen captured from the PS1 build.
- [Freeplay reference]({{ '/docs/freeplay/' | relative_url }})
  and [Pause menu reference]({{ '/docs/pause-menu/' | relative_url }})
  — the full controls and sub-screen surface.
- [Scene ledger]({{ '/scenes/' | relative_url }}) — every scene
  the original game had, validation status, last-verified release tag.
