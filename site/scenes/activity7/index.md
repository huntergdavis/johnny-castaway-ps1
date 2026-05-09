---
layout: scene
title: ACTIVITY 7 — Reads a book upside-down
ads: ACTIVITY
tag: 7
slug: activity7
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 7: Johnny sits reading a book, but the book is upside-down. Validated 2026-05-08."
---

Validated 2026-05-05. Visual + audible signoff after re-exporting
high/low packs through the no-stitch fast path with frame-wide keyed
overlay — fixes ghosted Johnny pose residue on the right side of the
island from base-diff against the static-Johnny base.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 7</code>
- Slug: <code>activity7</code>

## What this scene is

Johnny sits and reads a book — except the book is upside-down. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "bathes / seagull steals clothes" caption-mapping guess was wrong.


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
