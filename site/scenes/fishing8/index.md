---
layout: scene
title: FISHING 8 — Another boot
ads: FISHING
tag: 8
slug: fishing8
status: validated
description: "FISHING.ADS scene 8: Another boot. Validated on PS1 with captured-position foreground replay."
---

Validated on PS1 under the current scene bar. This scene uses a
single-position FG2 pack pinned to the host-captured island position
`x=3,y=9`, matching the full-width reference viewport used by the
host capture.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 8</code>
- Slug: <code>fishing8</code>

## What this scene probably is

(Guess.) A second 'caught a boot' variant; near-duplicate of FISHING 2.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### Validation Notes

A pixel-perfect host capture (ScummVM via the export script) produces a
base-diff `.FG2` foreground pack and a JSONL of sound events. The PS1
build replays that pack at native resolution through every variant the
original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). `FISHING 8` is position-sensitive:
the current pack is pixel-perfect for its captured viewport, not for
arbitrary random island positions. The runtime therefore pins this scene
to the captured position unless an explicit debug boot overrides it.
