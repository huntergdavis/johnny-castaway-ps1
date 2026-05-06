---
layout: scene
title: ACTIVITY 1 — Climbs the palm and dives
ads: ACTIVITY
tag: 1
slug: activity1
status: validated
description: "ACTIVITY.ADS scene 1: Climbs the palm and dives. Validated on PS1."
---

Validated on 2026-05-05 after rebuilding high/low packs from a capped
two-beat story capture (`FG_EXPORT_ACTIVITY1_CAPTURE_FRAMES=400`).
Source frames `148` and `348` hold the animal scorecards, and
`patch-activity1-tree-foreground.py` keys foreground-only tree-band
contamination against the full-host composite so the pre-pop hat/white
pixels and tree-occlusion ghosts are gone in both loops.

## Pack identifiers

- ADS dispatch: <code>ACTIVITY.ADS scene 1</code>
- Slug: <code>activity1</code>

## What this scene probably is

(Guess.) Johnny climbs the palm tree and does a dive off it — the perfect-flip variant.

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

See [the method]({{ '/about/method/' | relative_url }}) for the longer
version of the validation process.
