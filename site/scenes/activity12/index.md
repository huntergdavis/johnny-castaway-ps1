---
layout: scene
title: ACTIVITY 12 — Bird on head, Johnny clubs himself
ads: ACTIVITY
tag: 12
slug: activity12
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 12: A bird lands on Johnny's head and he clubs himself trying to hit the bird. Validated 2026-05-08."
---

Validated 2026-05-05. Visual + audible signoff on the existing on-disc
`ACTIVITY12.FG2` / `ACTV12L.FG2` packs — high-tide nighttime route, no
rework needed.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 12</code>
- Slug: <code>activity12</code>

## What this scene is

A bird lands on Johnny's head; he swings a club at it and ends up clobbering his own head instead. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "belly-flop dive" caption-mapping guess was wrong (that gag is ACTIVITY 1).

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
