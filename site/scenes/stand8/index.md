---
layout: scene
title: STAND 8 — Right side of island, looks around, scratches head
ads: STAND
tag: 8
slug: stand8
status: validated
description: "STAND.ADS scene 8: Johnny stands on the right side of the island, looks around, and scratches his head. Validated 2026-05-08."
image: /assets/img/stand8-ps1-right-scratch.png
image_alt: "STAND 8 on PS1 at night: Johnny stands on the right side of the island under the palm tree in the look-around-and-scratch-head idle."
image_width: 961
image_height: 720
---

<figure class="scene-hero">
  <img src="{{ '/assets/img/stand8-ps1-right-scratch.png' | relative_url }}"
       width="961" height="720"
       loading="lazy"
       decoding="async"
       alt="STAND 8 on PS1 at night: Johnny stands on the right side of the island under the palm tree in the look-around-and-scratch-head idle." />
  <figcaption>
    STAND 8 on PS1, captured during the v0.8.4-ps1
    <a href="{{ '/lab/chapter-select-grind/' | relative_url }}">chapter-select grind</a>.
    The right-side scratch-head idle. Position-variant pair with
    <a href="{{ '/scenes/stand6/' | relative_url }}">STAND 6</a>'s
    front-of-island scratch-head. Engineering footnote: STAND 8
    is what surfaced the static-ocean bug that had silently
    affected STAND 5/6/7 — the no-stitch foreground-only pack
    carries Johnny pixels but no captured water frames, so
    <code>OCEAN00.SCR</code> never advanced. The fix adds a
    per-frame wave tick to the FG2 runtime that mirrors
    <code>adsPilotTickBackgroundWaves</code>' timer pattern,
    advancing one wave frame every
    <code>gFgBackdropThread.delay</code> VBlanks and redrawing
    the last wave on intermediate frames so
    <code>grRestoreBgFromRects</code> doesn't leave a gap.
  </figcaption>
</figure>

Validated on 2026-05-04.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 8</code>
- Slug: <code>stand8</code>

## What this scene is

Johnny stands on the right side of the island, looks around, and scratches his head — a confused-looking idle pose, right-side variant (compare STAND 6, the front-of-island looks-out-and-scratches variant). Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "tap, lift, look" caption-mapping guess was wrong.

## Validation notes

Visual signoff passed after regenerating high and low tide packs through the
STAND no-stitch fast path AND adding a per-frame wave tick to the FG2
runtime. The earlier STAND validations (5/6/7) had silently lost ocean
animation: the no-stitch foreground-only pack carries Johnny pixels but no
captured water frames, so the static `OCEAN00.SCR` bg never advanced. The
new tick mirrors `adsPilotTickBackgroundWaves`' timer pattern, advancing
one wave frame every `gFgBackdropThread.delay` vblanks and redrawing the
last wave on intermediate frames so `grRestoreBgFromRects` doesn't leave a
gap. Scenes whose pack carries its own water frames are unaffected — the
foreground compose still draws on top.

Boot route:
`fgpilot stand8 lowtide 0 night 1 holiday 0 raft-stage 4 island-pos -154 54 loop seed 1`.
