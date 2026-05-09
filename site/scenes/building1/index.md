---
layout: scene
title: BUILDING 1 — Sandcastle slumps, Johnny stomps it
ads: BUILDING
tag: 1
slug: building1
status: validated
description: "BUILDING.ADS scene 1: Johnny tries to build a sandcastle; it slumps mid-build, and he stomps on it. Validated 2026-05-08."
---

Validated on 2026-05-05 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}). The high/low packs were
regenerated through the generic normal/far-left/far-right foreground-only
multi-view stitch and passed visual + audible signoff on the normal
high-tide/night validation route.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 1</code>
- Slug: <code>building1</code>

## What this scene is

Johnny tries to build a sandcastle on the beach. It slumps mid-build, and he stomps on it in frustration. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; matches and sharpens the prior caption-mapping guess ("sand castle crumbles").

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
