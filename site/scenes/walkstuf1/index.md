---
layout: scene
title: WALKSTUF 1 — Parties on a yacht, comes back drunk, passes out
ads: WALKSTUF
tag: 1
slug: walkstuf1
status: validated
last_verified: "2026-05-05"
description: "WALKSTUF.ADS scene 1: Johnny parties on a passing yacht, returns to the island drunk, and passes out. Validated 2026-05-08."
---

Validated 2026-05-05. Visual + audible signoff after re-exporting
high/low packs with -500/+300 stitch positions and a range-gated
Johnny-bbox hold (frames 63-165, glitch threshold 1000) so Johnny
stays drawn in his rest position while the boat + mermaid scene plays
out and the foreground-only diff drops him.

## Pack identifiers

- ADS dispatch: <code>WALKSTUF.ADS scene 1</code>
- Slug: <code>walkstuf1</code>

## What this scene is

A yacht pulls up; Johnny boards and parties with the crowd on board. He comes back to the island visibly drunk, staggers a few steps, and passes out on the sand. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "jogs around the island" caption-mapping guess was wrong (no jogging in the on-PS1 pack).

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
