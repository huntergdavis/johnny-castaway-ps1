---
layout: scene
title: FISHING 8 — Catches a fish (right-side variant)
ads: FISHING
tag: 8
slug: fishing8
status: validated
description: "FISHING.ADS scene 8: Johnny fishes off the right side of the island and reels in a fish. Validated 2026-05-08."
---

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
