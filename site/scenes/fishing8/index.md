---
layout: scene
title: FISHING 8 — Another boot
ads: FISHING
tag: 8
slug: fishing8
status: validated
description: "FISHING.ADS scene 8: Another boot. Validated on PS1 with random-position-safe scene playback."
---

Validated on PS1 under the current scene bar. The high/low packs were
recaptured with the host island shifted far left (`x=-300,y=54`) so the
complete scene-relative foreground stayed inside the capture viewport.
That is a capture/test position only: production playback uses the
normal random island placement.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 8</code>
- Slug: <code>fishing8</code>

## What this scene probably is

(Guess.) A second 'caught a boot' variant; near-duplicate of FISHING 2.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### Validation Notes

A pixel-perfect host capture (ScummVM via the export script) produces a
scene-relative `.FG2` foreground pack and a JSONL of sound events. The
PS1 build replays that pack at native resolution through every variant
the original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). `FISHING 8` uses the same
full-frame foreground-only keyed overlay pattern as `FISHING 7`. A forced
far-left PS1 stress run was clean, so production playback stays
random-position safe.
