---
layout: scene
title: VISITOR 6 — Shakes tree, cracks coconut on it, eats
ads: VISITOR
tag: 6
slug: visitor6
status: validated
description: "VISITOR.ADS scene 6: Johnny shakes the palm, drops a coconut, cracks it on the trunk, and eats it. Validated 2026-05-08."
---

Validated on 2026-05-04 under the [FISHING 1 bar]({{ '/docs/glossary/#fishing1-bar' | relative_url }}).

## Pack identifiers

- ADS dispatch: <code>VISITOR.ADS scene 6</code>
- Slug: <code>visitor6</code>

## What this scene is

Johnny shakes the palm tree until a coconut drops. He picks it up, cracks it open against the trunk, and eats the coconut meat. Confirmed by direct on-PS1 playback observation while capturing the chapter-select thumbnail; sharpens the prior "coconut tree impact" caption-mapping with the full shake-crack-eat sequence made explicit.

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
