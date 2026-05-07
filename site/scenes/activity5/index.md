---
layout: scene
title: ACTIVITY 5 — Climb / look / dive
ads: ACTIVITY
tag: 5
slug: activity5
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 5: Climb / look / dive. Validated 2026-05-05."
---

Validated 2026-05-05. Visual + audible signoff on the climb/look/dive
gag. High/low packs use the JOHNNY 2-style split — upper thought-bubble
lane on full base-diff so the storm-cloud bubble + connector dots
survive, and lower third on keyed overlay so the post-dive splash band
cleans up without ghost trails. Source frame 46 (the storm-cloud
bubble) gets +30 vblanks of hold so the gag is readable.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 5</code>
- Slug: <code>activity5</code>

## What this scene probably is

(Guess.) A composite climb-look-dive sequence; the SPOT_E variant.

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
