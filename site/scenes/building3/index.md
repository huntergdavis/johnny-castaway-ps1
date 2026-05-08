---
layout: scene
title: BUILDING 3 — Builds a fire
ads: BUILDING
tag: 3
slug: building3
status: validated
description: "BUILDING.ADS scene 3: Builds a fire. Validated on PS1 after generic multi-view capture."
---

Validated on 2026-05-05 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}). The high/low packs were
regenerated through the generic normal/far-left/far-right foreground-only
multi-view stitch, and the long sleeping/idle beat played cleanly on the
normal low-tide/night validation route.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 3</code>
- Slug: <code>building3</code>

## What this scene probably is

(Guess.) Johnny gathers sticks, builds a fire, and warms his hands.

Caption mapping confidence in the [audit]({{ '/docs/captions/' | relative_url }}): **HIGH**.

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
