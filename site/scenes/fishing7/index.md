---
layout: scene
title: FISHING 7 — Octopus chokes Johnny
ads: FISHING
tag: 7
slug: fishing7
status: validated
description: "FISHING.ADS scene 7: Octopus chokes Johnny. Validated on PS1 with random-position-safe scene playback."
---

Validated on PS1 under the current scene bar. The high/low packs were
recaptured with the host island shifted far left (`x=-300,y=54`) so the
complete scene-relative foreground stayed inside the capture viewport.
That is a capture/test position only: production playback uses the
normal random island placement.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 7</code>
- Slug: <code>fishing7</code>

## What this scene probably is

(Guess.) Johnny pulls in fish, then an angry octopus that chokes him; Johnny climbs the palm tree to escape.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### Validation Notes

A pixel-perfect host capture (ScummVM via the export script) produces a
scene-relative `.FG2` foreground pack and a JSONL of sound events. The
PS1 build replays that pack at native resolution through every variant
the original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). `FISHING 7` uses a full-frame
foreground-only keyed overlay during pack generation to avoid stale
full-host background pixels. A forced far-left PS1 stress run was clean,
so the old runtime island-position pin has been removed.
