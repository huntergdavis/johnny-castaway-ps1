---
layout: scene
title: ACTIVITY 8 — Bathes, then walks behind tree to dress
ads: ACTIVITY
tag: 8
slug: activity8
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 8: Johnny takes a bath, then walks behind the palm tree to hide his nakedness. Validated 2026-05-08."
---

Validated 2026-05-05. Visual + audible signoff after re-exporting
high/low packs through the no-stitch fast path with frame-wide keyed
overlay — fixes ghosted Johnny pose residue from base-diff against the
static-Johnny base.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 8</code>
- Slug: <code>activity8</code>

## What this scene is

Johnny takes a bath in the surf, then walks behind the palm tree, hiding his nakedness as he goes. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "reads book upside down" caption-mapping guess for this slot was wrong (that gag is ACTIVITY 7).


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
