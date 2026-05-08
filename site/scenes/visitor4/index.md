---
layout: scene
title: VISITOR 4 — Coconut plane hit
ads: VISITOR
tag: 4
slug: visitor4
status: validated
description: "VISITOR.ADS scene 4: Coconut plane gag. Validated on PS1/DuckStation."
---

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 4</code>
- Slug: <code>visitor4</code>

## What This Scene Is

Johnny knocks down the plane/visitor with a coconut.

The old catalogue label on this page described a red-boat scene; live
validation proved VISITOR 4 is the coconut/plane gag in the current PS1
scene mapping.

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

VISITOR 4 uses the generic normal/far-left/far-right foreground-only
multi-view stitch so island-relative action is complete across production
placements.

See [the method]({{ '/about/method/' | relative_url }}) for the longer version.
