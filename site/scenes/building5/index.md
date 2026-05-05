---
layout: scene
title: BUILDING 5 — Fixes the raft (mermaid)
ads: BUILDING
tag: 5
slug: building5
status: validated
description: "BUILDING.ADS scene 5: Fixes the raft (mermaid). Validated on PS1."
---

Validated on 2026-05-05 after regenerating high/low packs through the
generic normal/far-left/far-right foreground-only multi-view stitch. The
raft/mermaid repair scene played cleanly on the normal high-tide/night
validation route.

## Pack identifiers

- ADS dispatch: <code>BUILDING.ADS scene 5</code>
- Slug: <code>building5</code>

## What this scene probably is

(Guess.) Johnny works on the raft; the mermaid surfaces and asks what he's doing.

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

This scene is now in the validated set. See
[the method]({{ '/about/method/' | relative_url }}) for the longer version.
