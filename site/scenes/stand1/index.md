---
layout: scene
title: STAND 1 — Edge of the island
ads: STAND
tag: 1
slug: stand1
status: pending
description: "STAND.ADS scene 1: Edge of the island. Not yet validated."
---

Not yet validated.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 1</code>
- Slug: <code>stand1</code>

## What this scene probably is

(Guess.) Idle stance at SPOT_A SW: taps a foot, tips his hat. Stand-style filler.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

### How this scene gets validated

The same way every scene does: under the FISHING 1 bar.

A pixel-perfect host capture (ScummVM via the export script) produces a
base-diff `.FG2` foreground pack and a JSONL of sound events. The PS1
build replays that pack at native resolution through every variant the
original game randomized between (night, low-tide, holiday overlays,
raft-stage progress where applicable). The
[regtest harness]({{ '/docs/regtest/' | relative_url }}) checks that the
visuals come out frame-identical and the SFX cues land on the same
ticks. Once that holds across all applicable variants, the scene moves
to `validated` and a row turns green in the
[ledger]({{ '/scenes/' | relative_url }}).

Until then, this page exists so the catalogue is complete — not because
the scene is finished. See [the method]({{ '/about/method/' | relative_url }})
for the longer version.
