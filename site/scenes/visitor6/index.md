---
layout: scene
title: VISITOR 6 — Coconut tree impact
ads: VISITOR
tag: 6
slug: visitor6
status: validated
description: "VISITOR.ADS scene 6: Coconut tree impact. Validated on PS1/DuckStation with full-host impact-delta capture."
---

Validated on 2026-05-04 under the FISHING 1 bar.

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 6</code>
- Slug: <code>visitor6</code>

## What This Scene Is

Johnny shakes the palm tree; the coconut/tree impact is partly owned by
background-visible host layers instead of foreground-only actor draws.

The old catalogue label on this page described the schooner/partygoer scene.
Live validation showed that label belongs elsewhere in the current mapping.
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

VISITOR 6 uses the generic normal/far-left/far-right foreground-only
multi-view stitch for random-position safety. Its coconut/tree impact also
needs a narrow full-host delta injection over source frames 120:141, because
foreground-only capture keeps Johnny and the coconut clean but omits
background-owned tree shake/strike pixels.

See [the method]({{ '/about/method/' | relative_url }}) for the longer version.
