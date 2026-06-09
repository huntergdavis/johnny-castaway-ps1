---
layout: page
title: Freeplay and debug mode
eyebrow: Reference
subtitle: Direct-control Johnny, plus the runtime debug menus that make it safe to test.
description: "Freeplay and debug mode in the Johnny Castaway PS1 port: controls, pause-menu catalogs, world toggles, loading transitions, and long-run memory rules."
---

Freeplay is the first mode in the PS1 port where Johnny is not just being
replayed from a captured foreground pack. The player walks him around the
island directly. The rest of the world still uses the same hard-won PS1
runtime pieces: the ocean [backdrop]({{ '/docs/glossary/#backdrop' | relative_url }}), sparse clean-rect restoration, holiday
overlays, SPU sounds, captions, and the pause-menu UI.

The important distinction is architectural. `fishing1`, `fishing2`, and
the other story scenes are `.FG2` playback. Freeplay is live C code. Every
frame reads the controller, updates a small state machine, restores the
background from clean rects, draws Johnny and any active prop, and presents
the frame. That makes it useful as a play mode and as a debugging cockpit.

<details class="page-toc" markdown="1">
<summary>On this page</summary>

* TOC
{:toc}
</details>

## How to enter and leave

The normal boot still starts in screensaver mode. Press **Start**, choose
**Freeplay: OFF**, and the game tears the current scene down, shows the
`MEANWHIL.BMP` [frog clock]({{ '/docs/glossary/#frog-clock' | relative_url }}), builds the freeplay island, and hands control
to the player.

Inside freeplay, press **Start** again to open the same pause menu. Choose
**Freeplay: ON** to exit back to the screensaver loop. Exit also shows the
frog clock while freeplay releases sprite slots, clean-rect memory, captions,
and its live island backdrop.

[`BOOTMODE.TXT`]({{ '/docs/glossary/#bootmode' | relative_url }}) token entry remains available for test discs:

```text
fgpilot freeplay
```

The default release path is menu-driven because that is the real player
flow we need to test.

## Final controller map

Freeplay deliberately keeps live controls small. Anything uncertain or
catalog-like belongs in the pause-menu debug pages, where it can be named,
described, and selected without memorizing button chords.

| Control | Action |
|---|---|
| D-pad / left analog | Walk Johnny 4-way around the island. Any movement cancels the current gag/action immediately. |
| L2 held | Slow walk. Useful for lining Johnny up by the water or tree. |
| R2 held | Fast walk. |
| Select | Clear the freeplay screen, rebuild the clean background, and return to idle. This is both a player escape hatch and a heap/fragmentation safety valve. |
| R1 + Up | Toggle day/night immediately. |
| R1 + Down | Toggle high tide/low tide immediately. |
| R1 + Left | Cycle raft stage 0..5 immediately. |
| R1 + Right | Cycle holiday overlay immediately. |
| Start | Open pause menu. |

In menus, **Circle** is always Back. **Cross** is Select. That rule is now
global; no freeplay submenu is allowed to invent a second meaning for Circle.

## Pause-menu debug pages

The pause menu carries the rest of freeplay/debug mode:

| Menu | Purpose |
|---|---|
| Freeplay Options | Entry point for gag catalog, visitor catalog, controls, and clear screen. |
| Freeplay Gags | Named one-shot Johnny actions with source BMP, frame count, and rough memory cost. |
| Freeplay Visitors | Named outside-world events with source BMP, frame count, and rough memory cost. |
| World Options | Day/night, tide, raft, holidays, and island position. In freeplay, changing a value closes the menu, shows the frog clock, rebuilds the island, and applies immediately. |
| Sound Test | Selector-style SPU test page for individual sound effects. |
| Accessibility | Closed captions, sound mute, [ocean ambience]({{ '/docs/glossary/#ocean-ambience' | relative_url }}), and the Sound Test selector. |
| Controls | On-device reference for the freeplay controls above. |

This split is intentional. The joypad is for walking, clearing, and the
four immediate world toggles. Everything with a catalog or metadata
lives in a menu — including fishing, which moved off Circle to the
Scene Set selector during the Scene Set rewrite (see the "Fishing"
section below).

## What the debug catalogs expose

Gags and visitors are fail-soft. If an optional BMP cannot load, freeplay
prints the skip banner instead of crashing. The catalog pages make that
visible by showing the asset filename and frame count before you spawn it.

The current gag catalog includes simple Johnny actions such as eating,
wiping his brow, getting angry, bonking his head, strutting, and running.
The visitor catalog includes seagulls, the tiny parade, biplane, canoe,
boat, King Kong, Mary, pirate/fisherman cameo, flock, clouds, and the
meanwhile panel. These are debug-selectable first; we can tune frame ranges
and coordinates without changing the player-facing control scheme.

## Fishing

Fishing in freeplay is reached through the **Scene Set** menu rather
than a dedicated joypad button. Cycling Scene Set to *Fishing Only*
constrains the screensaver-loop random pool to the validated fishing
scenes (`fishing1`..`fishing8`) — the same captured `.FG2` packs the
normal rotation already plays, so the visuals match the source frames
exactly. The earlier Circle-as-fish freeplay action is gone; the live
control surface is reserved for walking, clearing, world toggles, and
the menu.

The Scene Set lineup also covers the rest of the catalog by family:
*Johnny Stories*, *Mary Visits*, *Visitors*, *Activities*, and *Misc &
Suzy*. At `{{ site.release.tag }}` every scene in every pool clears
the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }})
— the pools are reader-preference filters, not staging areas for
unvalidated content.

## Rendering and memory rules

Freeplay has to run forever. That means the frame loop is boring on
purpose:

- no malloc/free in the normal per-frame path;
- no BMP load in the normal per-frame path;
- clean-rect restore, sprite draw, overlay dirty marking, present;
- optional sprite loads only on menu/action boundaries;
- all optional action assets release their slot before replacement;
- freeplay exit hard-frees its clean-rect snapshot instead of leaving a
  dormant 300+ KB allocation behind;
- the frog loader uses a local `TTtmSlot` and releases `MEANWHIL.BMP`
  unconditionally after drawing the loading frame.

The overlay system has a separate dirty-rect hook for captions and banners.
That matters because those overlays draw straight to the framebuffer. When
they disappear, the underlying island rows must be marked dirty or the
bottom of a caption/banner can leave black pixels behind.

Wave animation is intentionally throttled in freeplay. The shared island
wave routine advances every fourth freeplay frame now, not every second
frame, because the live-control loop was making the surf read too fast
compared with the normal screensaver cadence.

## Telemetry

Freeplay has level-gated telemetry:

| Boot token | Effect |
|---|---|
| `freeplay-log` | periodic summaries: frame count, mode, action count, summon count, clean-rect count, failures, largest heap probe |
| `freeplay-detail` | asset load details and clean-rect rebuild detail |
| `freeplay-debug` | on-screen debug line with mode, position, rebuild count, and asset-failure count |

The default path keeps heap probes out of entry/exit and out of the frame
loop. Heap probing is still available when debugging a crash, but it is not
part of the forever-run player build.

## Related pages

- [Story-loop walks]({{ '/docs/walks/' | relative_url }}) — the
  other consumer of `walk_render`. Same draw kernel; story-loop
  walks consume the pre-baked Sierra path table while freeplay
  drives `(x, y)` from the D-pad per [VBlank]({{ '/docs/glossary/#vblank' | relative_url }}).
- [Pause menu]({{ '/docs/pause-menu/' | relative_url }}) — menu structure and persistence.
- [Holidays]({{ '/docs/holidays/' | relative_url }}) — holiday overlays used by the world toggle.
- [Audio pipeline]({{ '/docs/audio/' | relative_url }}) — the SPU
  voice rotation freeplay's Sound Test sub-screen drives.
- [API mapping (SDL2 → PSn00bSDK)]({{ '/docs/api/' | relative_url }})
  — freeplay is the largest live consumer of the `gr*` graphics
  surface.
- [Performance]({{ '/docs/performance/' | relative_url }}) — why heap shape and VBlank cadence matter.
- [v0.5.0-ps1 release entry]({{ '/releases/#v050-ps1--freeplay-and-debug-mode' | relative_url }})
  — the milestone freeplay shipped in.
- [Play]({{ '/play/' | relative_url }}) — the end-user flow:
  Pause → Freeplay: ON.
- [Devlog: freeplay debug mode]({{ '/devlog/freeplay-debug-mode/' | relative_url }}) — the implementation narrative.
- [Lab: the two-day SPI bug]({{ '/lab/two-day-spi-bug/' | relative_url }})
  — war story for the controller-poll fix without which
  freeplay's D-pad / analog / button reads were unreliable.

## Source

- [`src/scene_freeplay/scene_freeplay.c`]({{ site.github_url }}/blob/main/src/scene_freeplay/scene_freeplay.c)
- [`src/walk/walk_render.c`]({{ site.github_url }}/blob/main/src/walk/walk_render.c)
  · [`src/walk/walk_render.h`]({{ site.github_url }}/blob/main/src/walk/walk_render.h)
  — the shared draw kernel; story-loop walks consume it through
  `walk_pilot.c`, freeplay calls it directly with D-pad-driven coords.
- [`src/pause_menu/pause_menu.c`]({{ site.github_url }}/blob/main/src/pause_menu/pause_menu.c)
- [`docs/ps1/freeplay-mode-design.md`]({{ site.github_url }}/blob/main/docs/ps1/freeplay-mode-design.md)
