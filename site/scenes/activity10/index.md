---
layout: scene
title: ACTIVITY 10 — Reads; seagull steals book
ads: ACTIVITY
tag: 10
slug: activity10
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 10: Reads; seagull steals book. Validated 2026-05-05."
---

Validated 2026-05-05. Visual + audible signoff on the existing on-disc
`ACTIVITY10.FG2` / `ACTV10L.FG2` packs — high-tide nighttime route, no
rework needed.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 10</code>
- Slug: <code>activity10</code>

## What this scene probably is

(Guess.) Reads under the tree; a seagull lifts the book away from him.

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
