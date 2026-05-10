---
layout: scene
title: FISHING 8 — Catches a fish (right-side variant)
ads: FISHING
tag: 8
slug: fishing8
status: validated
description: "FISHING.ADS scene 8: Johnny fishes off the right side of the island and reels in a fish. Validated 2026-05-08."
image: /assets/img/fishing8-ps1-fish-right.png
image_alt: "FISHING 8 on PS1 at night: Johnny stands on the right-side dock holding a caught fish on the line, fishing rod arched out toward the water."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/fishing8-ps1-fish-right.png' | relative_url }}"
       width="961" height="720"
       fetchpriority="high"
       decoding="async"
       alt="FISHING 8 on PS1 at night: Johnny stands on the right-side dock holding a caught fish on the line, fishing rod arched out toward the water." />
  <figcaption>
    FISHING 8 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The reel-in beat: Johnny is on the right-side dock with a small
    fish on the line. (Compare
    <a href="{{ '/scenes/fishing7/' | relative_url }}">FISHING 7</a>,
    the right-side starfish variant of FISHING 1, also on this dock.)
    The pack uses the same full-frame foreground-only keyed overlay
    pattern as FISHING 7; a forced far-left PS1 stress run was
    clean, so production playback stays random-position safe.
  </figcaption>
</figure>

Validated on PS1 under the current scene bar. The high/low packs were
recaptured with the host island shifted far left (`x=-300,y=54`) so the
complete scene-relative foreground stayed inside the capture viewport.
That is a capture/test position only: production playback uses the
normal random island placement.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 8</code>
- Slug: <code>fishing8</code>

## What this scene is

Johnny casts a line off the right side of the island and reels in a fish. A right-side fishing variant. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "another boot" caption-mapping guess was wrong (the boot gag isn't this scene).

### Validation Notes

A host-side Johnny Reborn capture/export pass produces a
scene-relative `.FG2` foreground pack and a JSONL of sound events. The
PS1 build replays that pack at native resolution through every variant
the original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). `FISHING 8` uses the same
full-frame foreground-only keyed overlay pattern as `FISHING 7`. A forced
far-left PS1 stress run was clean, so production playback stays
random-position safe.
