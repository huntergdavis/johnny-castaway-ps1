---
layout: scene
title: WALKSTUF 1 — Jogs around the island
ads: WALKSTUF
tag: 1
slug: walkstuf1
status: validated
last_verified: "2026-05-05"
description: "WALKSTUF.ADS scene 1: Jogs around the island. Validated 2026-05-05."
---

Validated 2026-05-05. Visual + audible signoff after re-exporting
high/low packs with -500/+300 stitch positions and a range-gated
Johnny-bbox hold (frames 63-165, glitch threshold 1000) so Johnny
stays drawn in his rest position while the boat + mermaid scene plays
out and the foreground-only diff drops him.

## Pack identifiers

- ADS dispatch: <code>WALKSTUF.ADS scene 1</code>
- Slug: <code>walkstuf1</code>

## What this scene probably is

(Guess.) Johnny jogs around the island in a grey jogging outfit.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **MED**.

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
