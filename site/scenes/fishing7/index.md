---
layout: scene
title: FISHING 7 — Catches a starfish (right-side variant), throws it back
ads: FISHING
tag: 7
slug: fishing7
status: validated
description: "FISHING.ADS scene 7: Johnny fishes off the right side of the island, hooks a starfish, and throws it back. Validated 2026-05-08."
---

Validated on PS1 under the current scene bar. The high/low packs were
recaptured with the host island shifted far left (`x=-300,y=54`) so the
complete scene-relative foreground stayed inside the capture viewport.
That is a capture/test position only: production playback uses the
normal random island placement.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 7</code>
- Slug: <code>fishing7</code>

## What this scene is

Johnny casts a line off the right side of the island, hooks a starfish, and tosses it back into the water. A right-side variant of the FISHING 1 starfish gag. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "octopus chokes Johnny" caption-mapping guess was wrong.

### Validation Notes

A host-side Johnny Reborn capture/export pass produces a
scene-relative `.FG2` foreground pack and a JSONL of sound events. The
PS1 build replays that pack at native resolution through every variant
the original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). `FISHING 7` uses a full-frame
foreground-only keyed overlay during pack generation to avoid stale
full-host background pixels. A forced far-left PS1 stress run was clean,
so the old runtime island-position pin has been removed.
