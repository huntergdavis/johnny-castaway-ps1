---
layout: scene
title: STAND 8 — Tap, lift, look
ads: STAND
tag: 8
slug: stand8
status: pending
description: "STAND.ADS scene 8: Tap, lift, look. Not yet validated."
---

Not yet validated.

## Pack identifiers

- ADS dispatch: <code>STAND.ADS scene 8</code>
- Slug: <code>stand8</code>

## What this scene probably is

(Guess.) Idle at SPOT_C E: composite of two short stances.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **LOW**.

### How this scene gets validated

The same way every scene does: under the FISHING 1 bar.

A host-side Johnny Reborn capture/export pass produces a
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
