---
layout: scene
title: ACTIVITY 4 — Reads, seagull on head
ads: ACTIVITY
tag: 4
slug: activity4
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 4: Reads, seagull on head. Validated 2026-05-05."
---

Validated 2026-05-05. Visual + audible signoff on the existing on-disc
`ACTIVITY4.FG2` / `ACTV4LOW.FG2` packs — no rework needed. High-tide
nighttime route, reads-with-seagull-on-head.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 4</code>
- Slug: <code>activity4</code>

## What this scene probably is

(Guess.) Johnny reads under the tree; a seagull lands on his head and Johnny tries to club it and misses.

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
