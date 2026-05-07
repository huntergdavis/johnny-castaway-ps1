---
layout: scene
title: ACTIVITY 11 — Rain dance
ads: ACTIVITY
tag: 11
slug: activity11
status: validated
last_verified: "2026-05-05"
description: "ACTIVITY.ADS scene 11: Rain dance. Validated 2026-05-05."
---

Validated 2026-05-05. Visual + audible signoff after re-exporting
high/low packs with a frame-wide keyed overlay
(`KEYED_OVERLAY_RECT="0,0,640,480"`); fixes stale Johnny / bird-outline
residue that the foreground-only diff carried against the moving rain
backdrop.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 11</code>
- Slug: <code>activity11</code>

## What this scene probably is

(Guess.) Johnny does a long rain-dance / heat-relief sequence starting from the spawn spot.

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
