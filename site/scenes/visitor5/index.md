---
layout: scene
title: VISITOR 5 — Throws a coconut at a plane; it crashes
ads: VISITOR
tag: 5
slug: visitor5
status: validated
description: "VISITOR.ADS scene 5: Johnny sees a plane overhead, throws a coconut at it, and the plane crashes. Validated 2026-05-08."
---

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 5</code>
- Slug: <code>visitor5</code>

## What this scene is

A plane flies overhead. Johnny grabs a coconut and throws it at the plane — and the coconut hits hard enough that the plane crashes. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; the earlier "visitor takes Johnny" caption-mapping guess was wrong (this scene is also where the audit's "coconut plane hit" caption actually belongs — VISITOR 4 was misattributed).

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

VISITOR 5 uses the generic normal/far-left/far-right foreground-only
multi-view stitch so island-relative action is complete across production
placements. Its high/low packs also use scene-specific hold redistribution
so the coconut impact and downed-plane motion stay readable without
changing total scene duration.

See [the method]({{ '/about/method/' | relative_url }}) for the longer version.
