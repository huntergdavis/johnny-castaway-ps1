---
title: Play
eyebrow: Download · Run
subtitle: Latest build, quickstart, controls.
---

## Latest build

**{{ site.release.tag }}** — see [release notes]({{ site.github_url }}/releases/tag/{{ site.release.tag }}).

<p class="hero-cta" markdown="0">
  <a class="btn btn--primary" href="{{ site.github_url }}/releases/download/{{ site.release.tag }}/jcreborn.bin">Download .bin</a>
  <a class="btn"              href="{{ site.github_url }}/releases/download/{{ site.release.tag }}/jcreborn.cue">Download .cue</a>
  <a class="btn btn--small"   href="{{ site.github_url }}/releases">All releases</a>
</p>

The CD image ships as a **.bin / .cue** pair. Both files belong in
the same directory.

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

## Controls

The game runs without any input by default. The pause menu is
optional and reachable with **Start**.

| Button            | Action                                           |
|-------------------|--------------------------------------------------|
| Start             | Open / close pause menu                          |
| D-Pad ↑/↓         | Move cursor in menu                              |
| D-Pad ←/→         | Cycle a toggle (sound, captions, holiday, …)    |
| X                 | Confirm / select                                 |
| Triangle          | Back / cancel                                    |

Inside the pause menu you can: mute sound, toggle closed captions,
force a holiday, advance to the next scene, set the in-game date,
move the island anchor, set the RNG seed, and read the credits.
The full controller map and pause-menu walkthrough lands at
`/play/controls/` in P2.

## Real PS1 hardware

It runs on real hardware — the build has been smoke-tested on a
SCPH-7501 via the [TonyHax](https://github.com/socram8888/tonyhax)
softmod path. No officially-licensed disc, no modchip required.
Your mileage may vary; treat any boot success as a small miracle.

## Where to file a bug

The issue tracker is at [{{ site.repo }}/issues]({{ site.github_url }}/issues).
Bugs are tolerated, not invited — there's no contributor onboarding,
just one author and a tip jar that doesn't exist. See the [FAQ]({{ '/faq/' | relative_url }}).
