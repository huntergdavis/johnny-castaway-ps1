---
layout: scene
title: WALKSTUF 2 — Works on the raft
ads: WALKSTUF
tag: 2
slug: walkstuf2
status: validated
last_verified: "2026-05-04"
description: "WALKSTUF.ADS scene 2: Johnny works on his raft. Validated 2026-05-08."
---

Validated 2026-05-04. Visual + audible signoff on the existing on-disc
`WALK2.FG2` / `WALK2LOW.FG2` packs (no rework needed) — high-tide
nighttime route.

## Pack identifiers

- ADS dispatch: <code>WALKSTUF.ADS scene 2</code>
- Slug: <code>walkstuf2</code>

## What this scene is

Johnny works on his raft, building it up — one of the visible-progress beats that drives the raft-stage variant flag. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "walk transition" caption-mapping was vague (this scene is specifically the raft-build work, not a generic walk loop).

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
