---
layout: scene
title: FISHING 7 — Octopus chokes Johnny
ads: FISHING
tag: 7
slug: fishing7
status: validated
description: "FISHING.ADS scene 7: Octopus chokes Johnny. Validated on PS1 with captured-position foreground replay."
---

Validated on PS1 under the current scene bar. This scene uses a
single-position FG2 pack pinned to the host-captured island position
`x=3,y=9`; at that placement the foreground fills the 640-pixel
reference viewport and the right-edge pole/line clipping matches the
original capture.

## Pack identifiers

- ADS dispatch: <code>FISHING.ADS scene 7</code>
- Slug: <code>fishing7</code>

## What this scene probably is

(Guess.) Johnny pulls in fish, then an angry octopus that chokes him; Johnny climbs the palm tree to escape.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

### Validation Notes

A pixel-perfect host capture (ScummVM via the export script) produces a
base-diff `.FG2` foreground pack and a JSONL of sound events. The PS1
build replays that pack at native resolution through every variant the
original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). `FISHING 7` is position-sensitive:
the current pack is pixel-perfect for its captured viewport, not for
arbitrary random island positions. The runtime therefore pins this scene
to the captured position unless an explicit debug boot overrides it.
