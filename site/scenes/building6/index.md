---
layout: scene
title: BUILDING 6 — Lilliputians tie Johnny up (no-bird variant)
ads: BUILDING
tag: 6
slug: building6
status: validated
description: "BUILDING.ADS scene 6: Lilliputians tie Johnny down while he sleeps — the no-bird variant of the BUILDING 4 gag. Validated 2026-05-08."
---

Validated on 2026-05-05 after regenerating high/low packs through the
generic normal/far-left/far-right foreground-only multi-view stitch. The
edge-sleep scene played cleanly on the normal high-tide/night validation
route.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 6</code>
- Slug: <code>building6</code>

## What this scene is

Same Gulliver gag as BUILDING 4: Johnny sleeps on the sand and tiny lilliputians swarm in and tie him down with ropes. This time no bird beat — just the lilliputians and the ropes. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "edge sleep" caption-mapping guess was wrong.

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

This scene is now in the validated set. See
[the method]({{ '/about/method/' | relative_url }}) for the longer version.
